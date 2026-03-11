$ErrorActionPreference = "Stop"

$outputPath = if ($args.Length -gt 0) { $args[0] } else { "Benchmarks\Generated\c6288_macro.bench" }
$mode = if ($args.Length -gt 1) { $args[1].ToLowerInvariant() } else { "packed" }
$useSlices = $mode -eq "packed"

if ($mode -ne "packed" -and $mode -ne "anchor")
	{
	throw "usage: gen_c6288_macro.ps1 <output.bench> [packed|anchor]"
	}

$outputDir = Split-Path -Parent $outputPath
if ($outputDir -and -not (Test-Path $outputDir))
	{
	New-Item -ItemType Directory -Path $outputDir | Out-Null
	}

function Assert-BitReduction
	{
	param(
		[int]$InputCount,
		[bool]$HasSum,
		[bool]$HasCarry,
		[string]$Label
	)

	$expectedSum = if ($InputCount -gt 0) { 1 } else { 0 }
	$expectedCarry = if ($InputCount -ge 2) { 1 } else { 0 }
	$actualSum = Get-BoolBit $HasSum
	$actualCarry = Get-BoolBit $HasCarry

	if ($actualSum -ne $expectedSum -or $actualCarry -ne $expectedCarry)
		{
		throw "Invariant failure at ${Label}: in=$InputCount sum=$actualSum carry=$actualCarry"
		}
	}

function Assert-SliceReduction
	{
	param(
		[int]$InputCount0,
		[int]$InputCount1,
		[bool]$HasSum0,
		[bool]$HasSum1,
		[bool]$HasCarryOut,
		[string]$Label
	)

	if (-not $HasSum0 -or -not $HasSum1 -or -not $HasCarryOut)
		{
		throw "Slice invariant failure at ${Label}: expected sum0,sum1,carry from dense 2-bit slice"
		}
	}

function Get-PresentBit
	{
	param([object]$Signal)

	if ($Signal)
		{
		return 1
		}

	return 0
	}

function Get-BoolBit
	{
	param([bool]$Value)

	if ($Value)
		{
		return 1
		}

	return 0
	}

$aInputs = @(
	"1", "18", "35", "52", "69", "86", "103", "120",
	"137", "154", "171", "188", "205", "222", "239", "256"
)

$bInputs = @(
	"273", "290", "307", "324", "341", "358", "375", "392",
	"409", "426", "443", "460", "477", "494", "511", "528"
)

$outputs = @(
	"545", "1581", "1901", "2223", "2548", "2877", "3211", "3552",
	"3895", "4241", "4591", "4946", "5308", "5672", "5971", "6123",
	"6150", "6160", "6170", "6180", "6190", "6200", "6210", "6220",
	"6230", "6240", "6250", "6260", "6270", "6280", "6287", "6288"
)

$lines = New-Object System.Collections.Generic.List[string]

foreach ($name in $aInputs + $bInputs)
	{
	$lines.Add("INPUT($name)")
	}

$lines.Add("")

foreach ($name in $outputs)
	{
	$lines.Add("OUTPUT($name)")
	}

$lines.Add("")

$zero = "__lxs_zero"
$lines.Add("$zero = XOR($($aInputs[0]), $($aInputs[0]))")
$lines.Add("")

$partialProducts = @{}
for ($i = 0; $i -lt 16; ++$i)
	{
	for ($j = 0; $j -lt 16; ++$j)
		{
		$name = "pp_${i}_${j}"
		$partialProducts["$i,$j"] = $name
		$lines.Add("$name = AND($($aInputs[$i]), $($bInputs[$j]))")
		}
	}

$lines.Add("")

$acc = New-Object object[] 32
for ($j = 0; $j -lt 16; ++$j)
	{
	$acc[$j] = $partialProducts["0,$j"]
	}

for ($rowIndex = 1; $rowIndex -lt 16; ++$rowIndex)
	{
	$row = New-Object object[] 32
	$next = New-Object object[] 32
	$carry = $zero
	$bit = 0

	for ($j = 0; $j -lt 16; ++$j)
		{
		$row[$rowIndex + $j] = $partialProducts["$rowIndex,$j"]
		}

	while ($bit -lt 32)
		{
		$hasCarryIn = $carry -ne $zero
		$inputCount0 = (Get-PresentBit $acc[$bit]) + (Get-PresentBit $row[$bit])
		if ($hasCarryIn)
			{
			$inputCount0 += 1
			}

		if ($useSlices -and $bit -le 30 -and $acc[$bit] -and $row[$bit] -and $acc[$bit + 1] -and $row[$bit + 1] -and $hasCarryIn)
			{
			$inputCount1 = 3
			$sum0 = "add_${rowIndex}_sum_${bit}"
			$sum1 = "add_${rowIndex}_sum_$($bit + 1)"
			$carryOut = if (($bit + 1) -lt 31) { "add_${rowIndex}_carry_$($bit + 1)" } else { "add_${rowIndex}_carry_top" }
			$lines.Add("$sum0, $sum1, $carryOut = RIPPLE_SLICE2($($acc[$bit]), $($row[$bit]), $($acc[$bit + 1]), $($row[$bit + 1]), $carry)")
			$next[$bit] = $sum0
			$next[$bit + 1] = $sum1
			$carry = $carryOut
			Assert-SliceReduction $inputCount0 $inputCount1 $true $true $true "row $rowIndex bits $bit-$($bit + 1)"
			$bit += 2
			continue
			}

		$inputs = New-Object System.Collections.Generic.List[string]
		if ($acc[$bit])
			{
			$inputs.Add([string]$acc[$bit])
			}
		if ($row[$bit])
			{
			$inputs.Add([string]$row[$bit])
			}
		if ($hasCarryIn)
			{
			$inputs.Add([string]$carry)
			}

		switch ($inputs.Count)
			{
			0
				{
				$next[$bit] = $null
				$carry = $zero
				Assert-BitReduction 0 $false $false "row $rowIndex bit $bit"
				}
			1
				{
				$next[$bit] = $inputs[0]
				$carry = $zero
				Assert-BitReduction 1 $true $false "row $rowIndex bit $bit"
				}
			2
				{
				$sum = "add_${rowIndex}_sum_${bit}"
				$carryOut = if ($bit -lt 31) { "add_${rowIndex}_carry_${bit}" } else { "add_${rowIndex}_carry_top" }
				$lines.Add("$sum, $carryOut = HALF_ADDER($($inputs[0]), $($inputs[1]))")
				$next[$bit] = $sum
				$carry = $carryOut
				Assert-BitReduction 2 $true $true "row $rowIndex bit $bit"
				}
			3
				{
				$sum = "add_${rowIndex}_sum_${bit}"
				$carryOut = if ($bit -lt 31) { "add_${rowIndex}_carry_${bit}" } else { "add_${rowIndex}_carry_top" }
				$lines.Add("$sum, $carryOut = FULL_ADDER($($inputs[0]), $($inputs[1]), $($inputs[2]))")
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

for ($bit = 0; $bit -lt 32; ++$bit)
	{
	$productBit = $bit
	if ($bit -eq 30)
		{
		$productBit = 31
		}
	elseif ($bit -eq 31)
		{
		$productBit = 30
		}

	$source = if ($acc[$productBit]) { $acc[$productBit] } else { $zero }
	$lines.Add("$($outputs[$bit]) = BUF($source)")
	}

[System.IO.File]::WriteAllLines($outputPath, $lines)
Write-Host "Wrote $outputPath [$mode]"
