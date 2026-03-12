$ErrorActionPreference = "Stop"

$inputPath = if ($args.Length -gt 0) { $args[0] } else { throw "usage: gen_parity4_macro.ps1 <input.bench> <output.bench>" }
$outputPath = if ($args.Length -gt 1) { $args[1] } else { throw "usage: gen_parity4_macro.ps1 <input.bench> <output.bench>" }

$lines = Get-Content $inputPath
$assignments = New-Object System.Collections.Generic.List[object]
$useCount = @{}

foreach ($line in $lines)
	{
	$trim = $line.Trim()
	if ($trim -match '^([^=]+)=\s*([A-Z0-9_]+)\((.*)\)$')
		{
		$out = $Matches[1].Trim()
		$gate = $Matches[2].Trim()
		$ins = @()
		foreach ($token in ($Matches[3] -split ','))
			{
			$name = $token.Trim()
			if ($name.Length -gt 0)
				{
				$ins += $name
				if (-not $useCount.ContainsKey($name))
					{
					$useCount[$name] = 0
					}
				$useCount[$name]++
				}
			}

		$assignments.Add([pscustomobject]@{
			Output = $out
			Gate = $gate
			Inputs = $ins
			Raw = $line
		})
		}
	}

$byOutput = @{}
foreach ($node in $assignments)
	{
	$byOutput[$node.Output] = $node
	}

function Get-ParityLeafSet
	{
	param(
		[string]$Net,
		[hashtable]$NodeMap,
		[hashtable]$UseMap,
		[int]$MaxLeaves,
		[hashtable]$Visited
	)

	if ($Visited.ContainsKey($Net))
		{
		return $null
		}

	if (-not $NodeMap.ContainsKey($Net))
		{
		return @{
			leaves = @($Net)
			internal = @()
		}
		}

	$node = $NodeMap[$Net]
	if ($node.Gate -eq "PARITY4" -and $node.Inputs.Count -eq 4)
		{
		return @{
			leaves = @($node.Inputs)
			internal = @($Net)
		}
		}

	if ($node.Gate -eq "PARITY8" -and $node.Inputs.Count -eq 8)
		{
		return @{
			leaves = @($node.Inputs)
			internal = @($Net)
		}
		}

	if ($node.Gate -ne "XOR" -or $node.Inputs.Count -ne 2)
		{
		return @{
			leaves = @($Net)
			internal = @()
		}
		}

	$Visited[$Net] = 1
	$lhs = Get-ParityLeafSet $node.Inputs[0] $NodeMap $UseMap $MaxLeaves $Visited
	$rhs = Get-ParityLeafSet $node.Inputs[1] $NodeMap $UseMap $MaxLeaves $Visited
	$Visited.Remove($Net) | Out-Null

	if (-not $lhs -or -not $rhs)
		{
		return $null
		}

	$leaves = @($lhs.leaves + $rhs.leaves)
	if ($leaves.Count -gt $MaxLeaves)
		{
		return $null
		}

	$internal = @($lhs.internal + $rhs.internal + $Net)
	return @{
		leaves = $leaves
		internal = $internal
	}
	}

$skip = @{}
$rewritten = New-Object System.Collections.Generic.List[string]

foreach ($line in $lines)
	{
	$trim = $line.Trim()

	if ($trim.Length -eq 0 -or $trim.StartsWith("#") -or $trim.StartsWith("INPUT(") -or $trim.StartsWith("OUTPUT("))
		{
		$rewritten.Add($line)
		continue
		}

	if ($trim -notmatch '^([^=]+)=\s*([A-Z0-9_]+)\((.*)\)$')
		{
		$rewritten.Add($line)
		continue
		}

	$out = $Matches[1].Trim()
	if ($skip.ContainsKey($out))
		{
		continue
		}

	$set8 = Get-ParityLeafSet $out $byOutput $useCount 8 @{}
	if ($set8 -and $set8.leaves.Count -eq 8)
		{
		$rewritten.Add("$out = PARITY8($($set8.leaves -join ', '))")
		foreach ($internal in $set8.internal)
			{
			if ($internal -ne $out -and $useCount.ContainsKey($internal) -and $useCount[$internal] -eq 1)
				{
				$skip[$internal] = 1
				}
			}
		continue
		}

	$set4 = Get-ParityLeafSet $out $byOutput $useCount 4 @{}
	if ($set4 -and $set4.leaves.Count -eq 4)
		{
		$rewritten.Add("$out = PARITY4($($set4.leaves -join ', '))")
		foreach ($internal in $set4.internal)
			{
			if ($internal -ne $out -and $useCount.ContainsKey($internal) -and $useCount[$internal] -eq 1)
				{
				$skip[$internal] = 1
				}
			}
		continue
		}

	$rewritten.Add($line)
	}

[System.IO.File]::WriteAllLines($outputPath, $rewritten)
Write-Host "Wrote $outputPath"
