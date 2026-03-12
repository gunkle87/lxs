$ErrorActionPreference = "Stop"

$inputPath = if ($args.Length -gt 0) { $args[0] } else { throw "usage: gen_xnor_bank4_macro.ps1 <input.bench> <output.bench>" }
$outputPath = if ($args.Length -gt 1) { $args[1] } else { throw "usage: gen_xnor_bank4_macro.ps1 <input.bench> <output.bench>" }

$lines = Get-Content $inputPath
$nodes = @{}
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
		$nodes[$out] = $node
		$byLineIndex[[string]$i] = $node
		}
	}

$emitAt = @{}
$skipOutputs = @{}
$claimedLines = @{}

for ($i = 0; $i -lt $lines.Count; ++$i)
	{
	$key = [string]$i
	if ($claimedLines.ContainsKey($key))
		{
		continue
		}

	$group = @()
	for ($j = 0; $j -lt 4; ++$j)
		{
		$lineKey = [string]($i + $j)
		if (-not $byLineIndex.ContainsKey($lineKey))
			{
			$group = @()
			break
			}
		$node = $byLineIndex[$lineKey]
		if ($node.Gate -ne "NOT" -or $node.Inputs.Count -ne 1)
			{
			$group = @()
			break
			}
		if (-not $nodes.ContainsKey($node.Inputs[0]))
			{
			$group = @()
			break
			}
		$xorNode = $nodes[$node.Inputs[0]]
		if ($xorNode.Gate -ne "XOR" -or $xorNode.Inputs.Count -ne 2)
			{
			$group = @()
			break
			}
		$group += [pscustomobject]@{
			Not = $node
			Xor = $xorNode
		}
		}

	if ($group.Count -ne 4)
		{
		continue
		}

	$emitAt[$key] = "$($group[0].Not.Output), $($group[1].Not.Output), $($group[2].Not.Output), $($group[3].Not.Output) = XNOR_BANK4($($group[0].Xor.Inputs[0]), $($group[0].Xor.Inputs[1]), $($group[1].Xor.Inputs[0]), $($group[1].Xor.Inputs[1]), $($group[2].Xor.Inputs[0]), $($group[2].Xor.Inputs[1]), $($group[3].Xor.Inputs[0]), $($group[3].Xor.Inputs[1]))"
	for ($j = 0; $j -lt 4; ++$j)
		{
		$claimedLines[[string]($i + $j)] = 1
		$skipOutputs[$group[$j].Not.Output] = 1
		}
	$i += 3
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
