$ErrorActionPreference = "Stop"

$inputPath = if ($args.Length -gt 0) { $args[0] } else { throw "usage: gen_vector_multiplier_macro.ps1 <input.bench> <output.bench> [anchor|packed]" }
$outputPath = if ($args.Length -gt 1) { $args[1] } else { throw "usage: gen_vector_multiplier_macro.ps1 <input.bench> <output.bench> [anchor|packed]" }
$mode = if ($args.Length -gt 2) { $args[2].ToLowerInvariant() } else { "packed" }
$useSlices = $mode -eq "packed"
$toolDir = Split-Path -Parent $MyInvocation.MyCommand.Path
. (Join-Path $toolDir "LxsRewriteLib.ps1")
$usage = "usage: gen_vector_multiplier_macro.ps1 <input.bench> <output.bench> [anchor|packed]"

Initialize-LxsRewrite $mode $outputPath $usage
$io = Get-BenchIo $inputPath
$inputs = $io.inputs
$outputs = $io.outputs

$aMap = Get-BusMap $inputs "a"
$bMap = Get-BusMap $inputs "b"
$fMap = Get-BusMap $outputs "f"

if ($aMap.Count -eq 0 -or $bMap.Count -eq 0 -or $fMap.Count -eq 0)
	{
	throw "failed to infer vector multiplier interface from $inputPath"
	}

$aWidth = $aMap.Count
$bWidth = $bMap.Count
$productWidth = $fMap.Count

$aInputs = Get-ContiguousBus $aMap "a"
$bInputs = Get-ContiguousBus $bMap "b"
$productOutputs = Get-ContiguousBus $fMap "f"
$bench = New-BenchLines $inputs $outputs $aInputs[0]
$lines = $bench.lines
$zero = $bench.zero

$partialProducts = @{}
for ($i = 0; $i -lt $aWidth; ++$i)
	{
	for ($j = 0; $j -lt $bWidth; ++$j)
		{
		$name = "pp_${i}_${j}"
		$partialProducts["$i,$j"] = $name
		$lines.Add("$name = AND($($aInputs[$i]), $($bInputs[$j]))")
		}
	}

$lines.Add("")

$acc = New-Object object[] $productWidth
for ($j = 0; $j -lt $bWidth -and $j -lt $productWidth; ++$j)
	{
	$acc[$j] = $partialProducts["0,$j"]
	}

for ($rowIndex = 1; $rowIndex -lt $aWidth; ++$rowIndex)
	{
	$row = New-Object object[] $productWidth
	$next = New-Object object[] $productWidth
	$carry = $zero
	$bit = 0

	for ($j = 0; $j -lt $bWidth; ++$j)
		{
		$dst = $rowIndex + $j
		if ($dst -lt $productWidth)
			{
			$row[$dst] = $partialProducts["$rowIndex,$j"]
			}
		}

	while ($bit -lt $productWidth)
		{
		$hasCarryIn = $carry -ne $zero
		$inputCount0 = (Get-PresentBit $acc[$bit]) + (Get-PresentBit $row[$bit])
		if ($hasCarryIn)
			{
			$inputCount0 += 1
			}

		if ($useSlices -and $bit -le ($productWidth - 2) -and
			$acc[$bit] -and $row[$bit] -and $acc[$bit + 1] -and $row[$bit + 1] -and $hasCarryIn)
			{
			$sum0 = "add_${rowIndex}_sum_${bit}"
			$sum1 = "add_${rowIndex}_sum_$($bit + 1)"
			$carryOut = if (($bit + 1) -lt ($productWidth - 1)) { "add_${rowIndex}_carry_$($bit + 1)" } else { "add_${rowIndex}_carry_top" }
			$lines.Add("$sum0, $sum1, $carryOut = RIPPLE_SLICE2($($acc[$bit]), $($row[$bit]), $($acc[$bit + 1]), $($row[$bit + 1]), $carry)")
			$next[$bit] = $sum0
			$next[$bit + 1] = $sum1
			$carry = $carryOut
			Assert-SliceReduction $inputCount0 3 $true $true $true "row $rowIndex bits $bit-$($bit + 1)"
			$bit += 2
			continue
			}

		$inputsAtBit = New-Object System.Collections.Generic.List[string]
		if ($acc[$bit])
			{
			$inputsAtBit.Add([string]$acc[$bit])
			}
		if ($row[$bit])
			{
			$inputsAtBit.Add([string]$row[$bit])
			}
		if ($hasCarryIn)
			{
			$inputsAtBit.Add([string]$carry)
			}

		switch ($inputsAtBit.Count)
			{
			0
				{
				$next[$bit] = $null
				$carry = $zero
				Assert-BitReduction 0 $false $false "row $rowIndex bit $bit"
				}
			1
				{
				$next[$bit] = $inputsAtBit[0]
				$carry = $zero
				Assert-BitReduction 1 $true $false "row $rowIndex bit $bit"
				}
			2
				{
				$sum = "add_${rowIndex}_sum_${bit}"
				$carryOut = if ($bit -lt ($productWidth - 1)) { "add_${rowIndex}_carry_${bit}" } else { "add_${rowIndex}_carry_top" }
				$lines.Add("$sum, $carryOut = HALF_ADDER($($inputsAtBit[0]), $($inputsAtBit[1]))")
				$next[$bit] = $sum
				$carry = $carryOut
				Assert-BitReduction 2 $true $true "row $rowIndex bit $bit"
				}
			3
				{
				$sum = "add_${rowIndex}_sum_${bit}"
				$carryOut = if ($bit -lt ($productWidth - 1)) { "add_${rowIndex}_carry_${bit}" } else { "add_${rowIndex}_carry_top" }
				$lines.Add("$sum, $carryOut = FULL_ADDER($($inputsAtBit[0]), $($inputsAtBit[1]), $($inputsAtBit[2]))")
				$next[$bit] = $sum
				$carry = $carryOut
				Assert-BitReduction 3 $true $true "row $rowIndex bit $bit"
				}
			default
				{
				throw "Unexpected operand count at row $rowIndex bit $bit"
				}
			}

		$bit += 1
		}

	$acc = $next
	$lines.Add("")
	}

for ($bit = 0; $bit -lt $productWidth; ++$bit)
	{
	$source = if ($acc[$bit]) { $acc[$bit] } else { $zero }
	$lines.Add("$($productOutputs[$bit]) = BUF($source)")
	}

Write-BenchLines $outputPath $lines "$mode aw=$aWidth bw=$bWidth ow=$productWidth"
