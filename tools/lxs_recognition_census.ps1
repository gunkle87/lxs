param(
	[string]$BenchRoot = "BenchSuites",
	[string]$CacheRoot = "build\recognition_census",
	[string]$Family = "all",
	[int]$Samples = 1,
	[string]$SampleSelect = "median",
	[int]$Iterations = 1,
	[int]$Cycles = 1,
	[int]$TimeoutSeconds = 120,
	[switch]$Force
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
$benchExe = Join-Path $repoRoot "build\bin\lxs_bench.exe"

if (-not (Test-Path $benchExe))
	{
	throw "lxs_bench.exe not found at $benchExe"
	}

if (-not (Test-Path $BenchRoot))
	{
	throw "Bench root not found: $BenchRoot"
	}

if (-not (Test-Path $CacheRoot))
	{
	New-Item -ItemType Directory -Force -Path $CacheRoot | Out-Null
	}

$families = @(
	[pscustomobject]@{ Name = "shared_xor"; Label = "Shared XOR"; Mask = "2"; MatchField = "shared_xor_matches"; GateField = "shared_xor_gate_equiv"; ShareField = "shared_xor_absorbed_work_share" },
	[pscustomobject]@{ Name = "shared_and"; Label = "Shared AND"; Mask = "4"; MatchField = "shared_and_matches"; GateField = "shared_and_gate_equiv"; ShareField = "shared_and_absorbed_work_share" },
	[pscustomobject]@{ Name = "compare"; Label = "Compare/Equality"; Mask = "8"; MatchField = "compare_matches"; GateField = "compare_gate_equiv"; ShareField = "compare_absorbed_work_share" },
	[pscustomobject]@{ Name = "register_en"; Label = "Register Enable"; Mask = "16"; MatchField = "register_en_matches"; GateField = "register_en_gate_equiv"; ShareField = "register_en_absorbed_work_share" },
	[pscustomobject]@{ Name = "arithmetic"; Label = "Arithmetic"; Mask = "32"; MatchField = "arithmetic_matches"; GateField = "arithmetic_gate_equiv"; ShareField = "arithmetic_absorbed_work_share" },
	[pscustomobject]@{ Name = "control"; Label = "Control"; Mask = "64"; MatchField = "control_matches"; GateField = "control_gate_equiv"; ShareField = "control_absorbed_work_share" }
)

if ($Family -ne "all")
	{
	$families = @($families | Where-Object { $_.Name -eq $Family })
	if ($families.Count -eq 0)
		{
		throw "Unknown family: $Family"
		}
	}

function Parse-KvLine
	{
	param([string]$Line)

	$parts = $Line.Split(',')
	$obj = [ordered]@{
		tag = $parts[0].Substring(2).Trim()
		circuit = $parts[1]
	}
	for ($i = 2; $i -lt $parts.Length; ++$i)
		{
		$kv = $parts[$i].Split('=', 2)
		if ($kv.Length -eq 2)
			{
			$obj[$kv[0]] = $kv[1]
			}
		}
	return [pscustomobject]$obj
	}

function Get-SuiteName
	{
	param([string]$Circuit)

	$slash = $Circuit.IndexOf('\')
	if ($slash -lt 0)
		{
		return "MISC"
		}
	return $Circuit.Substring(0, $slash)
	}

function Get-SuitePaths
	{
	param([string]$RootPath)

	$rootItem = Get-Item $RootPath
	if ($rootItem.PSIsContainer)
		{
		$childDirs = @(Get-ChildItem $RootPath -Directory | Sort-Object Name)
		if ($childDirs.Count -gt 0)
			{
			return $childDirs
			}
		return @($rootItem)
		}
	return @($rootItem)
	}

function Get-BenchmarkTargets
	{
	param([string]$RootPath)

	$rootItem = Get-Item $RootPath
	if (-not $rootItem.PSIsContainer)
		{
		return @($rootItem)
		}

	$files = @(Get-ChildItem $RootPath -Recurse -File | Where-Object { $_.Extension -ieq ".bench" } | Sort-Object FullName)
	if ($files.Count -gt 0)
		{
		return $files
		}

	$dirs = @(Get-ChildItem $RootPath -Directory | Sort-Object FullName)
	if ($dirs.Count -gt 0)
		{
		return $dirs
		}

	return @($rootItem)
	}

function Get-CacheStem
	{
	param(
		[string]$RootPath,
		[string]$TargetPath
	)

	$rootResolved = (Resolve-Path $RootPath).Path
	$targetResolved = (Resolve-Path $TargetPath).Path
	if ($rootResolved -eq $targetResolved)
		{
		$relative = [System.IO.Path]::GetFileNameWithoutExtension($targetResolved)
		}
	else
		{
		$relative = $targetResolved.Substring($rootResolved.Length).TrimStart('\')
		$relative = [System.IO.Path]::Combine(
			[System.IO.Path]::GetDirectoryName($relative),
			[System.IO.Path]::GetFileNameWithoutExtension($relative)
		)
		}
	if ([string]::IsNullOrWhiteSpace($relative))
		{
		$relative = [System.IO.Path]::GetFileNameWithoutExtension($targetResolved)
		}
	$stem = $relative
	$stem = $stem -replace '[\\/:*?""<>|]', '_'
	return $stem
	}

function Invoke-StructProfileToFile
	{
	param(
		[string]$Mask,
		[string]$TargetPath,
		[string]$OutputPath
	)

	$env:LXS_RECOGNITION_MASK = $Mask
	$env:LXS_RECOGNITION_MODE = "report_only"
	try
		{
		$argList = @(
			$TargetPath,
			"--samples", $Samples,
			"--sample-select", $SampleSelect,
			"--iterations", $Iterations,
			"--cycles", $Cycles,
			"--struct-profile"
		)
		$stdoutPath = [System.IO.Path]::GetTempFileName()
		$stderrPath = [System.IO.Path]::GetTempFileName()
		try
			{
			$proc = Start-Process -FilePath $benchExe -ArgumentList $argList -PassThru -NoNewWindow -RedirectStandardOutput $stdoutPath -RedirectStandardError $stderrPath
			if (-not $proc.WaitForExit($TimeoutSeconds * 1000))
				{
				try
					{
					if (-not $proc.HasExited)
						{
						$proc.Kill()
						}
					}
				catch
					{
					}
				$proc.WaitForExit()
				Set-Content -Path $OutputPath -Value ("# census_status,timed_out`n# target," + $TargetPath)
				return
				}
			$stdout = Get-Content $stdoutPath -Raw
			$stderr = Get-Content $stderrPath -Raw
			if (($null -ne $proc.ExitCode) -and ($proc.ExitCode -ne 0))
				{
				if ($null -eq $stderr)
					{
					$stderr = ""
					}
				Set-Content -Path $OutputPath -Value ("# census_status,error`n# target," + $TargetPath + "`n# stderr," + $stderr.Trim())
				return
				}
			Set-Content -Path $OutputPath -Value $stdout
			}
		finally
			{
			Remove-Item $stdoutPath -ErrorAction SilentlyContinue
			Remove-Item $stderrPath -ErrorAction SilentlyContinue
			}
		}
	finally
		{
		Remove-Item Env:LXS_RECOGNITION_MASK -ErrorAction SilentlyContinue
		Remove-Item Env:LXS_RECOGNITION_MODE -ErrorAction SilentlyContinue
		}
	}

function Load-StructProfileFile
	{
	param([string]$Path)

	$struct = New-Object System.Collections.Generic.List[object]
	$recognition = New-Object System.Collections.Generic.List[object]
	$status = "ok"
	foreach ($line in (Get-Content $Path))
		{
		if ($line.StartsWith("# census_status,"))
			{
			$status = $line.Substring(16).Trim()
			}
		elseif ($line.StartsWith("# struct,"))
			{
			$struct.Add((Parse-KvLine $line))
			}
		elseif ($line.StartsWith("# recognition,"))
			{
			$recognition.Add((Parse-KvLine $line))
			}
		}

	return [pscustomobject]@{
		Status = $status
		Struct = $struct
		Recognition = $recognition
	}
	}

$targets = Get-BenchmarkTargets $BenchRoot
$rows = New-Object System.Collections.Generic.List[object]

foreach ($familyDef in $families)
	{
	$allStruct = New-Object System.Collections.Generic.List[object]
	$allRecognition = New-Object System.Collections.Generic.List[object]
	$timedOutTargets = New-Object System.Collections.Generic.List[string]
	$errorTargets = New-Object System.Collections.Generic.List[string]

	foreach ($target in $targets)
		{
		$cacheStem = Get-CacheStem $BenchRoot $target.FullName
		$cacheFile = Join-Path $CacheRoot ($familyDef.Name + "_report_only_" + $cacheStem + ".txt")
		if ($Force -or -not (Test-Path $cacheFile))
			{
			Invoke-StructProfileToFile $familyDef.Mask $target.FullName $cacheFile
			}
		$result = Load-StructProfileFile $cacheFile
		if ($result.Status -eq "timed_out")
			{
			$timedOutTargets.Add($target.FullName)
			continue
			}
		if ($result.Status -eq "error")
			{
			$errorTargets.Add($target.FullName)
			continue
			}
		foreach ($row in $result.Struct)
			{
			$allStruct.Add($row)
			}
		foreach ($row in $result.Recognition)
			{
			$allRecognition.Add($row)
			}
		}

	$totalSteps = ($allStruct | Measure-Object -Property total_steps -Sum).Sum
	$totalGateEquiv = ($allStruct | Measure-Object -Property total_gate_equiv -Sum).Sum
	$totalMatches = ($allRecognition | Measure-Object -Property $familyDef.MatchField -Sum).Sum
	$totalFamilyGateEquiv = ($allRecognition | Measure-Object -Property $familyDef.GateField -Sum).Sum
	$stepShare = if ($totalSteps -gt 0) { [double]$totalMatches / [double]$totalSteps } else { 0.0 }
	$workShare = if ($totalGateEquiv -gt 0) { [double]$totalFamilyGateEquiv / [double]$totalGateEquiv } else { 0.0 }

	$circuitsAffected = @(
		$allRecognition |
			Where-Object { [double]($_.$($familyDef.MatchField)) -gt 0 } |
			Select-Object -ExpandProperty circuit
	)

	$topSuites = @(
		$allRecognition |
			ForEach-Object {
					[pscustomobject]@{
						Suite = Get-SuiteName $_.circuit
						GateEquiv = [double]($_.$($familyDef.GateField))
					}
			} |
			Group-Object Suite |
			ForEach-Object {
				[pscustomobject]@{
					Suite = $_.Name
					GateEquiv = ($_.Group | Measure-Object -Property GateEquiv -Sum).Sum
				}
			} |
			Where-Object { $_.GateEquiv -gt 0 } |
			Sort-Object GateEquiv -Descending
	)

	$topCircuits = @(
		$allRecognition |
			ForEach-Object {
					[pscustomobject]@{
						Circuit = $_.circuit
						GateEquiv = [double]($_.$($familyDef.GateField))
						WorkShare = [double]($_.$($familyDef.ShareField))
					}
			} |
			Where-Object { $_.GateEquiv -gt 0 } |
			Sort-Object WorkShare -Descending
	)

		$rows.Add([pscustomobject]@{
			Name = $familyDef.Name
			Label = $familyDef.Label
			Mask = $familyDef.Mask
		MatchCount = $totalMatches
		GateEquiv = $totalFamilyGateEquiv
		StepShare = $stepShare
		WorkShare = $workShare
		CircuitsAffected = $circuitsAffected.Count
		TimedOut = $timedOutTargets.Count
		Errors = $errorTargets.Count
		TopSuite = if ($topSuites.Count -gt 0) { $topSuites[0].Suite } else { "" }
		TopSuiteGateEquiv = if ($topSuites.Count -gt 0) { $topSuites[0].GateEquiv } else { 0 }
		TopCircuit = if ($topCircuits.Count -gt 0) { $topCircuits[0].Circuit } else { "" }
		TopCircuitWorkShare = if ($topCircuits.Count -gt 0) { $topCircuits[0].WorkShare } else { 0.0 }
	})
	}

$rows | Sort-Object WorkShare -Descending | Format-Table -AutoSize
