param(
	[string]$Family = "compare",
	[string]$CacheRoot = "build\recognition_stress",
	[int]$Samples = 1,
	[string]$SampleSelect = "median",
	[int]$Iterations = 1,
	[int]$Cycles = 1,
	[int]$TimeoutSeconds = 30
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
$benchExe = Join-Path $repoRoot "build\bin\lxs_bench.exe"

if (-not (Test-Path $benchExe))
	{
	throw "lxs_bench.exe not found at $benchExe"
	}

if (-not (Test-Path $CacheRoot))
	{
	New-Item -ItemType Directory -Force -Path $CacheRoot | Out-Null
	}

$families = @{
	"shared_xor" = "2"
	"shared_and" = "4"
	"compare" = "8"
	"register_en" = "16"
	"arithmetic" = "32"
	"control" = "64"
}

if (-not $families.ContainsKey($Family))
	{
	throw "Unknown family: $Family"
	}

$stressPack = @(
	"BenchSuites\EPFL\multiplier.bench",
	"BenchSuites\ISCAS89\s38417.bench",
	"BenchSuites\ISCAS89\s38584.bench",
	"BenchSuites\ITC99\b22_1.bench"
)

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

function Invoke-StressCase
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
		$stopwatch = [System.Diagnostics.Stopwatch]::StartNew()
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
				$stopwatch.Stop()
				Set-Content -Path $OutputPath -Value ("# stress_status,timed_out`n# elapsed_ms," + $stopwatch.ElapsedMilliseconds + "`n# target," + $TargetPath)
				return
				}
			$stopwatch.Stop()
			$stdout = Get-Content $stdoutPath -Raw
			$stderr = Get-Content $stderrPath -Raw
			if (($null -ne $proc.ExitCode) -and ($proc.ExitCode -ne 0))
				{
				Set-Content -Path $OutputPath -Value ("# stress_status,error`n# elapsed_ms," + $stopwatch.ElapsedMilliseconds + "`n# target," + $TargetPath + "`n# stderr," + $stderr.Trim())
				return
				}
			Set-Content -Path $OutputPath -Value ("# elapsed_ms," + $stopwatch.ElapsedMilliseconds + "`n" + $stdout)
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

foreach ($relativePath in $stressPack)
	{
	$targetPath = Join-Path $repoRoot $relativePath
	if (-not (Test-Path $targetPath))
		{
		Write-Host ("SKIP," + $relativePath + ",missing")
		continue
		}

	$stem = $relativePath -replace '[\\/:*?""<>|]', '_'
	$outputPath = Join-Path $CacheRoot ($Family + "_" + $stem + ".txt")
	Invoke-StressCase -Mask $families[$Family] -TargetPath $targetPath -OutputPath $outputPath

	$status = "ok"
	$elapsedMs = ""
	$recognitionLine = $null
	$telemetryLine = $null

	foreach ($line in (Get-Content $outputPath))
		{
		if ($line.StartsWith("# stress_status,"))
			{
			$status = $line.Substring(16).Trim()
			}
		elseif ($line.StartsWith("# elapsed_ms,"))
			{
			$elapsedMs = $line.Substring(13).Trim()
			}
		elseif ($line.StartsWith("# recognition,"))
			{
			$recognitionLine = Parse-KvLine $line
			}
		elseif ($line.StartsWith("# recognition_telemetry,") -and $line.Contains("family=" + $Family))
			{
			$telemetryLine = Parse-KvLine $line
			}
		}

	if ($status -ne "ok")
		{
		Write-Host ("RESULT," + $relativePath + ",status=" + $status + ",elapsed_ms=" + $elapsedMs)
		continue
		}

	if ($null -eq $recognitionLine -or $null -eq $telemetryLine)
		{
		Write-Host ("RESULT," + $relativePath + ",status=missing_profile,elapsed_ms=" + $elapsedMs)
		continue
		}

	$matchField = $Family + "_matches"
	if ($Family -eq "compare")
		{
		$matchField = "compare_matches"
		}
	elseif ($Family -eq "shared_xor")
		{
		$matchField = "shared_xor_matches"
		}
	elseif ($Family -eq "shared_and")
		{
		$matchField = "shared_and_matches"
		}
	elseif ($Family -eq "register_en")
		{
		$matchField = "register_en_matches"
		}
	elseif ($Family -eq "arithmetic")
		{
		$matchField = "arithmetic_matches"
		}
	elseif ($Family -eq "control")
		{
		$matchField = "control_matches"
		}

	Write-Host (
		"RESULT," + $relativePath +
		",status=ok" +
		",elapsed_ms=" + $elapsedMs +
		",matches=" + $recognitionLine.$matchField +
		",candidate_roots=" + $telemetryLine.candidate_roots +
		",nodes_visited=" + $telemetryLine.nodes_visited +
		",max_depth=" + $telemetryLine.max_depth +
		",abort_shape=" + $telemetryLine.abort_shape +
		",abort_fanout=" + $telemetryLine.abort_fanout +
		",abort_branch=" + $telemetryLine.abort_branch +
		",abort_depth=" + $telemetryLine.abort_depth +
		",abort_node_budget=" + $telemetryLine.abort_node_budget +
		",abort_overlap=" + $telemetryLine.abort_overlap +
		",time_us=" + $telemetryLine.time_us
	)
	}
