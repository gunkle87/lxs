$ErrorActionPreference = "Stop"

$inputPath = if ($args.Length -gt 0) { $args[0] } else { throw "usage: gen_vector_multiplier_macro.ps1 <input.bench> <output.bench> [anchor|packed|neighborhood|neighborhood2|region1]" }
$outputPath = if ($args.Length -gt 1) { $args[1] } else { throw "usage: gen_vector_multiplier_macro.ps1 <input.bench> <output.bench> [anchor|packed|neighborhood|neighborhood2|region1]" }
$mode = if ($args.Length -gt 2) { $args[2].ToLowerInvariant() } else { "packed" }
$useSlices = $mode -eq "packed"
$useNeighborhood = $mode -eq "neighborhood"
$useNeighborhood2 = $mode -eq "neighborhood2"
$useRegion1 = $mode -eq "region1"
$toolDir = Split-Path -Parent $MyInvocation.MyCommand.Path
. (Join-Path $toolDir "LxsRewriteLib.ps1")
$usage = "usage: gen_vector_multiplier_macro.ps1 <input.bench> <output.bench> [anchor|packed|neighborhood|neighborhood2|region1]"

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

if ($useNeighborhood -or $useNeighborhood2 -or $useRegion1)
	{
	$columnCount = $productWidth + 4
	$columns = New-Object object[] $columnCount
	for ($bit = 0; $bit -lt $columnCount; ++$bit)
		{
		$columns[$bit] = New-Object 'System.Collections.Generic.List[string]'
		}

	for ($i = 0; $i -lt $aWidth; ++$i)
		{
		for ($j = 0; $j -lt $bWidth; ++$j)
			{
			$columns[$i + $j].Add($partialProducts["$i,$j"])
			}
		}

	$serial = 0
	$bit = 0
	while ($bit -le ($productWidth - 4))
		{
		if ($columns[$bit].Count -ge 3 -and
			$columns[$bit + 1].Count -ge 3 -and
			$columns[$bit + 2].Count -ge 3 -and
			$columns[$bit + 3].Count -ge 3)
			{
			$inputsAtWindow = New-Object System.Collections.Generic.List[string]
			for ($column = 0; $column -lt 4; ++$column)
				{
				for ($k = 0; $k -lt 3; ++$k)
					{
					$inputsAtWindow.Add($columns[$bit + $column][$k])
					}
				}

			for ($column = 0; $column -lt 4; ++$column)
				{
				$columns[$bit + $column].RemoveRange(0, 3)
				}

			$sum0 = "csr4_${serial}_s0"
			$sum1 = "csr4_${serial}_s1"
			$sum2 = "csr4_${serial}_s2"
			$sum3 = "csr4_${serial}_s3"
			$carry1 = "csr4_${serial}_c1"
			$carry2 = "csr4_${serial}_c2"
			$carry3 = "csr4_${serial}_c3"
			$carry4 = "csr4_${serial}_c4"
			$lines.Add("$sum0, $sum1, $sum2, $sum3, $carry1, $carry2, $carry3, $carry4 = CARRY_SAVE_ROW4($($inputsAtWindow[0]), $($inputsAtWindow[1]), $($inputsAtWindow[2]), $($inputsAtWindow[3]), $($inputsAtWindow[4]), $($inputsAtWindow[5]), $($inputsAtWindow[6]), $($inputsAtWindow[7]), $($inputsAtWindow[8]), $($inputsAtWindow[9]), $($inputsAtWindow[10]), $($inputsAtWindow[11]))")
			$columns[$bit].Add($sum0)
			$columns[$bit + 1].Add($sum1)
			$columns[$bit + 2].Add($sum2)
			$columns[$bit + 3].Add($sum3)
			$columns[$bit + 1].Add($carry1)
			$columns[$bit + 2].Add($carry2)
			$columns[$bit + 3].Add($carry3)
			$columns[$bit + 4].Add($carry4)
			$serial += 1
			$bit += 4
			continue
			}

		$bit += 1
		}

	if ($useNeighborhood2 -or $useRegion1)
		{
		for ($bit = 0; $bit -lt ($columnCount - 1); ++$bit)
			{
			while ($columns[$bit].Count -gt 3)
				{
				$a = $columns[$bit][0]
				$b = $columns[$bit][1]
				$c = $columns[$bit][2]
				$columns[$bit].RemoveRange(0, 3)
				$sum = "rp4_reduce_${serial}_s"
				$carryOut = "rp4_reduce_${serial}_c"
				$lines.Add("$sum, $carryOut = FULL_ADDER($a, $b, $c)")
				$columns[$bit].Add($sum)
				$columns[$bit + 1].Add($carryOut)
				$serial += 1
				}
			}

		$bit = 0
		while ($bit -lt $productWidth)
			{
			if ($bit -le ($productWidth - 4) -and
				$columns[$bit].Count -eq 3 -and
				$columns[$bit + 1].Count -eq 3 -and
				$columns[$bit + 2].Count -eq 3 -and
				$columns[$bit + 3].Count -eq 3)
				{
				$rpInputs = New-Object System.Collections.Generic.List[string]
				for ($column = 0; $column -lt 4; ++$column)
					{
					for ($k = 0; $k -lt 3; ++$k)
						{
						$rpInputs.Add($columns[$bit + $column][$k])
						}
					$columns[$bit + $column].RemoveRange(0, 3)
					}

				$spill0 = "rp4_${serial}_spill0"
				$spill1 = "rp4_${serial}_spill1"
				if ($useRegion1)
					{
					$lines.Add(
						"$($productOutputs[$bit]), $($productOutputs[$bit + 1]), $($productOutputs[$bit + 2]), $($productOutputs[$bit + 3]), $spill0, $spill1 = FUNC_MICRO($($rpInputs[0]), $($rpInputs[1]), $($rpInputs[2]), $($rpInputs[3]), $($rpInputs[4]), $($rpInputs[5]), $($rpInputs[6]), $($rpInputs[7]), $($rpInputs[8]), $($rpInputs[9]), $($rpInputs[10]), $($rpInputs[11]); o0, t0 = FA3(i0, i1, i2); t1, t3 = FA3(i3, i4, i5); o1, t2 = HA2(t1, t0); t4, t5 = FA3(i6, i7, i8); o2, t6 = FA3(t4, t3, t2); t7, t8 = FA3(i9, i10, i11); o3, t9 = FA3(t7, t5, t6); o4, o5 = HA2(t8, t9))")
					}
				else
					{
					$lines.Add(
						"$($productOutputs[$bit]), $($productOutputs[$bit + 1]), $($productOutputs[$bit + 2]), $($productOutputs[$bit + 3]), $spill0, $spill1 = REDUCE_PROPAGATE4($($rpInputs[0]), $($rpInputs[1]), $($rpInputs[2]), $($rpInputs[3]), $($rpInputs[4]), $($rpInputs[5]), $($rpInputs[6]), $($rpInputs[7]), $($rpInputs[8]), $($rpInputs[9]), $($rpInputs[10]), $($rpInputs[11]))")
					}
				$columns[$bit + 4].Add($spill0)
				if (($bit + 5) -lt $columnCount)
					{
					$columns[$bit + 5].Add($spill1)
					}
				$serial += 1
				$bit += 4
				continue
				}

			while ($columns[$bit].Count -gt 3)
				{
				$a = $columns[$bit][0]
				$b = $columns[$bit][1]
				$c = $columns[$bit][2]
				$columns[$bit].RemoveRange(0, 3)
				$sum = "rp4_local_${serial}_s"
				$carryOut = "rp4_local_${serial}_c"
				$lines.Add("$sum, $carryOut = FULL_ADDER($a, $b, $c)")
				$columns[$bit].Add($sum)
				$columns[$bit + 1].Add($carryOut)
				$serial += 1
				}

			$inputsAtBit = New-Object System.Collections.Generic.List[string]
			for ($k = 0; $k -lt $columns[$bit].Count; ++$k)
				{
				$inputsAtBit.Add([string]$columns[$bit][$k])
				}

			switch ($inputsAtBit.Count)
				{
				0
					{
					$lines.Add("$($productOutputs[$bit]) = BUF($zero)")
					}
				1
					{
					$lines.Add("$($productOutputs[$bit]) = BUF($($inputsAtBit[0]))")
					}
				2
					{
					$carryOut = "rp4_carry_${serial}_${bit}"
					$lines.Add("$($productOutputs[$bit]), $carryOut = HALF_ADDER($($inputsAtBit[0]), $($inputsAtBit[1]))")
					$columns[$bit + 1].Add($carryOut)
					}
				3
					{
					$carryOut = "rp4_carry_${serial}_${bit}"
					$lines.Add("$($productOutputs[$bit]), $carryOut = FULL_ADDER($($inputsAtBit[0]), $($inputsAtBit[1]), $($inputsAtBit[2]))")
					$columns[$bit + 1].Add($carryOut)
					}
				default
					{
					throw "unexpected unresolved column width $($inputsAtBit.Count) at bit $bit in $mode mode"
					}
				}

			$bit += 1
			}
		}
	else
		{
		for ($bit = 0; $bit -lt ($columnCount - 1); ++$bit)
			{
			while ($columns[$bit].Count -gt 2)
				{
				$a = $columns[$bit][0]
				$b = $columns[$bit][1]
				$c = $columns[$bit][2]
				$columns[$bit].RemoveRange(0, 3)
				$sum = "csa_reduce_${serial}_s"
				$carryOut = "csa_reduce_${serial}_c"
				$lines.Add("$sum, $carryOut = FULL_ADDER($a, $b, $c)")
				$columns[$bit].Add($sum)
				$columns[$bit + 1].Add($carryOut)
				$serial += 1
				}
			}

		$lhs = New-Object object[] $productWidth
		$rhs = New-Object object[] $productWidth
		for ($bit = 0; $bit -lt $productWidth; ++$bit)
			{
			$lhs[$bit] = if ($columns[$bit].Count -gt 0) { $columns[$bit][0] } else { $zero }
			$rhs[$bit] = if ($columns[$bit].Count -gt 1) { $columns[$bit][1] } else { $zero }
			}

		$carry = $zero
		$bit = 0
		while ($bit -lt $productWidth)
			{
			if ($bit -le ($productWidth - 4))
				{
				$carryOutName = "carry_final_$($bit + 3)"
				$lines.Add(
					"$($productOutputs[$bit]), $($productOutputs[$bit + 1]), $($productOutputs[$bit + 2]), $($productOutputs[$bit + 3]), $carryOutName = RIPPLE_ADD4($($lhs[$bit]), $($rhs[$bit]), $($lhs[$bit + 1]), $($rhs[$bit + 1]), $($lhs[$bit + 2]), $($rhs[$bit + 2]), $($lhs[$bit + 3]), $($rhs[$bit + 3]), $carry)")
				$carry = $carryOutName
				$bit += 4
				continue
				}

			if ($bit -le ($productWidth - 2))
				{
				$carryOutName = "carry_final_$($bit + 1)"
				$lines.Add(
					"$($productOutputs[$bit]), $($productOutputs[$bit + 1]), $carryOutName = RIPPLE_SLICE2($($lhs[$bit]), $($rhs[$bit]), $($lhs[$bit + 1]), $($rhs[$bit + 1]), $carry)")
				$carry = $carryOutName
				$bit += 2
				continue
				}

			$carryOutName = "carry_final_$bit"
			if ($carry -eq $zero)
				{
				$lines.Add("$($productOutputs[$bit]), $carryOutName = HALF_ADDER($($lhs[$bit]), $($rhs[$bit]))")
				}
			else
				{
				$lines.Add("$($productOutputs[$bit]), $carryOutName = FULL_ADDER($($lhs[$bit]), $($rhs[$bit]), $carry)")
				}
			$carry = $carryOutName
			$bit += 1
			}
		}
	}
else
	{
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
		$isFinalRow = $rowIndex -eq ($aWidth - 1)
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
				$sum0 = if ($isFinalRow) { $productOutputs[$bit] } else { "add_${rowIndex}_sum_${bit}" }
				$sum1 = if ($isFinalRow) { $productOutputs[$bit + 1] } else { "add_${rowIndex}_sum_$($bit + 1)" }
				$carryOut =
					if ($isFinalRow -and (($bit + 1) -eq ($productWidth - 2)))
						{
						$productOutputs[$productWidth - 1]
						}
					elseif (($bit + 1) -lt ($productWidth - 1))
						{
						"add_${rowIndex}_carry_$($bit + 1)"
						}
					else
						{
						"add_${rowIndex}_carry_top"
						}
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
					$sum = if ($isFinalRow) { $productOutputs[$bit] } else { "add_${rowIndex}_sum_${bit}" }
					$carryOut =
						if ($isFinalRow -and ($bit -eq ($productWidth - 2)))
							{
							$productOutputs[$productWidth - 1]
							}
						elseif ($bit -lt ($productWidth - 1))
							{
							"add_${rowIndex}_carry_${bit}"
							}
						else
							{
							"add_${rowIndex}_carry_top"
							}
					$lines.Add("$sum, $carryOut = HALF_ADDER($($inputsAtBit[0]), $($inputsAtBit[1]))")
					$next[$bit] = $sum
					$carry = $carryOut
					Assert-BitReduction 2 $true $true "row $rowIndex bit $bit"
					}
				3
					{
					$sum = if ($isFinalRow) { $productOutputs[$bit] } else { "add_${rowIndex}_sum_${bit}" }
					$carryOut =
						if ($isFinalRow -and ($bit -eq ($productWidth - 2)))
							{
							$productOutputs[$productWidth - 1]
							}
						elseif ($bit -lt ($productWidth - 1))
							{
							"add_${rowIndex}_carry_${bit}"
							}
						else
							{
							"add_${rowIndex}_carry_top"
							}
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
		if ($source -ne $productOutputs[$bit])
			{
			$lines.Add("$($productOutputs[$bit]) = BUF($source)")
			}
		}
	}

Write-BenchLines $outputPath $lines "$mode aw=$aWidth bw=$bWidth ow=$productWidth"
