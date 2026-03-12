$ErrorActionPreference = "Stop"

$inputPath = if ($args.Length -gt 0) { $args[0] } else { throw "usage: gen_xor_fan8_macro.ps1 <input.bench> <output.bench>" }
$outputPath = if ($args.Length -gt 1) { $args[1] } else { throw "usage: gen_xor_fan8_macro.ps1 <input.bench> <output.bench>" }

$lines = Get-Content $inputPath
$xorNodes = New-Object System.Collections.Generic.List[object]
$sharedCounts = @{}

for ($i = 0; $i -lt $lines.Count; ++$i)
	{
	$trim = $lines[$i].Trim()
	if ($trim -match '^([^=]+)=\s*XOR\(([^,]+),\s*([^)]+)\)$')
		{
		$out = $Matches[1].Trim()
		$a = $Matches[2].Trim()
		$b = $Matches[3].Trim()
		$xorNodes.Add([pscustomobject]@{
			LineIndex = $i
			Output = $out
			A = $a
			B = $b
		})

		foreach ($candidate in @($a, $b))
			{
			if (-not $sharedCounts.ContainsKey($candidate))
				{
				$sharedCounts[$candidate] = 0
				}
			$sharedCounts[$candidate]++
			}
		}
	}

$groups = @{}
foreach ($node in $xorNodes)
	{
	$candidates = @()
	if ($sharedCounts[$node.A] -ge 8) { $candidates += $node.A }
	if ($sharedCounts[$node.B] -ge 8) { $candidates += $node.B }
	if ($candidates.Count -eq 0)
		{
		continue
		}

	$shared = $candidates |
		Sort-Object -Property @(
			@{ Expression = { -$sharedCounts[$_] } },
			@{ Expression = { $_ } }) |
		Select-Object -First 1
	$leaf = if ($node.A -eq $shared) { $node.B } else { $node.A }
	if (-not $groups.ContainsKey($shared))
		{
		$groups[$shared] = New-Object System.Collections.Generic.List[object]
		}
	$groups[$shared].Add([pscustomobject]@{
		LineIndex = $node.LineIndex
		Output = $node.Output
		Leaf = $leaf
	})
	}

$emitAt = @{}
$skipOutputs = @{}

foreach ($shared in $groups.Keys)
	{
	$entries = $groups[$shared] | Sort-Object LineIndex
	for ($i = 0; $i -le ($entries.Count - 8); $i += 8)
		{
		$chunk = $entries[$i..($i + 7)]
		$key = [string]$chunk[0].LineIndex
		$emitAt[$key] = "$(($chunk | ForEach-Object Output) -join ', ') = XOR_FAN8($shared, $(($chunk | ForEach-Object Leaf) -join ', '))"
		for ($j = 0; $j -lt $chunk.Count; ++$j)
			{
			$skipOutputs[$chunk[$j].Output] = 1
			}
		$skipOutputs.Remove($chunk[0].Output) | Out-Null
		}
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
	if ($trim -match '^([^=]+)=\s*XOR\(')
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
