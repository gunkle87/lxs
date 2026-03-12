$ErrorActionPreference = "Stop"

$inputPath = if ($args.Length -gt 0) { $args[0] } else { throw "usage: gen_guard_chain4_macro.ps1 <input.bench> <output.bench>" }
$outputPath = if ($args.Length -gt 1) { $args[1] } else { throw "usage: gen_guard_chain4_macro.ps1 <input.bench> <output.bench>" }

$lines = Get-Content $inputPath
$nodes = New-Object System.Collections.Generic.List[object]
$byOutput = @{}
$byLineIndex = @{}
$useCount = @{}

for ($i = 0; $i -lt $lines.Count; ++$i)
	{
	$trim = $lines[$i].Trim()
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

		$node = [pscustomobject]@{
			LineIndex = $i
			Output = $out
			Gate = $gate
			Inputs = $ins
		}
		$nodes.Add($node)
		$byOutput[$out] = $node
		$byLineIndex[[string]$i] = $node
		}
	}

function Get-Node
	{
	param([string]$Name)

	if ($byOutput.ContainsKey($Name))
		{
		return $byOutput[$Name]
		}

	return $null
	}

$emitAt = @{}
$skipOutputs = @{}
$claimedOutputs = @{}
$claimedLines = @{}

foreach ($node in $nodes)
	{
	$and0 = $node
	if ($and0.Gate -ne "AND" -or $and0.Inputs.Count -ne 2)
		{
		continue
		}
	if ($claimedOutputs.ContainsKey($and0.Output))
		{
		continue
		}
	if ($claimedLines.ContainsKey([string]$and0.LineIndex))
		{
		continue
		}

	$not1 = if ($byLineIndex.ContainsKey([string]($and0.LineIndex + 1))) { $byLineIndex[[string]($and0.LineIndex + 1)] } else { $null }
	if (-not $not1 -or $not1.Gate -ne "NOT" -or $not1.Inputs.Count -ne 1 -or $not1.Inputs[0] -ne $and0.Output)
		{
		continue
		}
	if (-not $not1 -or $useCount[$not1.Output] -ne 1)
		{
		continue
		}

	$and1 = if ($byLineIndex.ContainsKey([string]($not1.LineIndex + 1))) { $byLineIndex[[string]($not1.LineIndex + 1)] } else { $null }
	if (-not $and1 -or $and1.Gate -ne "AND" -or $and1.Inputs.Count -ne 2 -or -not ($and1.Inputs -contains $not1.Output))
		{
		continue
		}

	$not2 = if ($byLineIndex.ContainsKey([string]($and1.LineIndex + 1))) { $byLineIndex[[string]($and1.LineIndex + 1)] } else { $null }
	if (-not $not2 -or $not2.Gate -ne "NOT" -or $not2.Inputs.Count -ne 1 -or $not2.Inputs[0] -ne $and1.Output)
		{
		continue
		}
	if (-not $not2 -or $useCount[$not2.Output] -ne 1)
		{
		continue
		}

	$and2 = if ($byLineIndex.ContainsKey([string]($not2.LineIndex + 1))) { $byLineIndex[[string]($not2.LineIndex + 1)] } else { $null }
	if (-not $and2 -or $and2.Gate -ne "AND" -or $and2.Inputs.Count -ne 2 -or -not ($and2.Inputs -contains $not2.Output))
		{
		continue
		}

	$not3 = if ($byLineIndex.ContainsKey([string]($and2.LineIndex + 1))) { $byLineIndex[[string]($and2.LineIndex + 1)] } else { $null }
	if (-not $not3 -or $not3.Gate -ne "NOT" -or $not3.Inputs.Count -ne 1 -or $not3.Inputs[0] -ne $and2.Output)
		{
		continue
		}
	if (-not $not3 -or $useCount[$not3.Output] -ne 1)
		{
		continue
		}

	$and3 = if ($byLineIndex.ContainsKey([string]($not3.LineIndex + 1))) { $byLineIndex[[string]($not3.LineIndex + 1)] } else { $null }
	if (-not $and3 -or $and3.Gate -ne "AND" -or $and3.Inputs.Count -ne 2 -or -not ($and3.Inputs -contains $not3.Output))
		{
		continue
		}

	$not4 = if ($byLineIndex.ContainsKey([string]($and3.LineIndex + 1))) { $byLineIndex[[string]($and3.LineIndex + 1)] } else { $null }
	if (-not $not4 -or $not4.Gate -ne "NOT" -or $not4.Inputs.Count -ne 1 -or $not4.Inputs[0] -ne $and3.Output)
		{
		continue
		}
	if ($claimedOutputs.ContainsKey($and1.Output) -or
		$claimedOutputs.ContainsKey($and2.Output) -or
		$claimedOutputs.ContainsKey($and3.Output) -or
		$claimedOutputs.ContainsKey($not1.Output) -or
		$claimedOutputs.ContainsKey($not2.Output) -or
		$claimedOutputs.ContainsKey($not3.Output) -or
		$claimedOutputs.ContainsKey($not4.Output))
		{
		continue
		}
	if ($claimedLines.ContainsKey([string]$not1.LineIndex) -or
		$claimedLines.ContainsKey([string]$and1.LineIndex) -or
		$claimedLines.ContainsKey([string]$not2.LineIndex) -or
		$claimedLines.ContainsKey([string]$and2.LineIndex) -or
		$claimedLines.ContainsKey([string]$not3.LineIndex) -or
		$claimedLines.ContainsKey([string]$and3.LineIndex) -or
		$claimedLines.ContainsKey([string]$not4.LineIndex))
		{
		continue
		}

	$invIn = if ($and0.Inputs[0] -eq $not1.Inputs[0]) { $and0.Inputs[1] } elseif ($and0.Inputs[1] -eq $not1.Inputs[0]) { $and0.Inputs[0] } else { $and0.Inputs[1] }
	$x0 = if ($and0.Inputs[0] -eq $invIn) { $and0.Inputs[1] } else { $and0.Inputs[0] }
	$x1 = if ($and1.Inputs[0] -eq $not1.Output) { $and1.Inputs[1] } else { $and1.Inputs[0] }
	$x2 = if ($and2.Inputs[0] -eq $not2.Output) { $and2.Inputs[1] } else { $and2.Inputs[0] }
	$x3 = if ($and3.Inputs[0] -eq $not3.Output) { $and3.Inputs[1] } else { $and3.Inputs[0] }

	$emitAt[[string]$and0.LineIndex] = "$($and0.Output), $($and1.Output), $($and2.Output), $($and3.Output), $($not4.Output) = GUARD_CHAIN4($invIn, $x0, $x1, $x2, $x3)"
	$skipOutputs[$not1.Output] = 1
	$skipOutputs[$and1.Output] = 1
	$skipOutputs[$not2.Output] = 1
	$skipOutputs[$and2.Output] = 1
	$skipOutputs[$not3.Output] = 1
	$skipOutputs[$and3.Output] = 1
	$skipOutputs[$not4.Output] = 1
	$skipOutputs.Remove($and1.Output) | Out-Null
	$skipOutputs.Remove($and2.Output) | Out-Null
	$skipOutputs.Remove($and3.Output) | Out-Null
	$skipOutputs.Remove($not4.Output) | Out-Null
	$claimedOutputs[$and0.Output] = 1
	$claimedOutputs[$and1.Output] = 1
	$claimedOutputs[$and2.Output] = 1
	$claimedOutputs[$and3.Output] = 1
	$claimedOutputs[$not1.Output] = 1
	$claimedOutputs[$not2.Output] = 1
	$claimedOutputs[$not3.Output] = 1
	$claimedOutputs[$not4.Output] = 1
	$claimedLines[[string]$and0.LineIndex] = 1
	$claimedLines[[string]$not1.LineIndex] = 1
	$claimedLines[[string]$and1.LineIndex] = 1
	$claimedLines[[string]$not2.LineIndex] = 1
	$claimedLines[[string]$and2.LineIndex] = 1
	$claimedLines[[string]$not3.LineIndex] = 1
	$claimedLines[[string]$and3.LineIndex] = 1
	$claimedLines[[string]$not4.LineIndex] = 1
	}

$rewritten = New-Object System.Collections.Generic.List[string]
for ($i = 0; $i -lt $lines.Count; ++$i)
	{
	$key = [string]$i
	if ($emitAt.ContainsKey($key))
		{
		$rewritten.Add($emitAt[$key])
		continue
		}

	$trim = $lines[$i].Trim()
	if ($trim -match '^([^=]+)=\s*([A-Z0-9_]+)\((.*)\)$')
		{
		$out = $Matches[1].Trim()
		if ($skipOutputs.ContainsKey($out))
			{
			continue
			}
		}

	$rewritten.Add($lines[$i])
	}

[System.IO.File]::WriteAllLines($outputPath, $rewritten)
Write-Host "Wrote $outputPath"
