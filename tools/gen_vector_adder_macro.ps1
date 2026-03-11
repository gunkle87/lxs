$ErrorActionPreference = "Stop"

$inputPath = if ($args.Length -gt 0) { $args[0] } else { throw "usage: gen_vector_adder_macro.ps1 <input.bench> <output.bench> [anchor|packed]" }
$outputPath = if ($args.Length -gt 1) { $args[1] } else { throw "usage: gen_vector_adder_macro.ps1 <input.bench> <output.bench> [anchor|packed]" }
$mode = if ($args.Length -gt 2) { $args[2].ToLowerInvariant() } else { "packed" }
$useSlices = $mode -eq "packed"
$toolDir = Split-Path -Parent $MyInvocation.MyCommand.Path
. (Join-Path $toolDir "LxsRewriteLib.ps1")
$usage = "usage: gen_vector_adder_macro.ps1 <input.bench> <output.bench> [anchor|packed]"

Initialize-LxsRewrite $mode $outputPath $usage
$io = Get-BenchIo $inputPath
$inputs = $io.inputs
$outputs = $io.outputs

$aMap = Get-BusMap $inputs "a"
$bMap = Get-BusMap $inputs "b"
$fMap = Get-BusMap $outputs "f"
$carryOut = $null

foreach ($name in $outputs)
	{
	if ($name -eq "cOut")
		{
		$carryOut = $name
		}
	}

if ($aMap.Count -eq 0 -or $aMap.Count -ne $bMap.Count -or $aMap.Count -ne $fMap.Count)
	{
	throw "failed to infer vector adder interface from $inputPath"
	}

$width = $aMap.Count
$aInputs = Get-ContiguousBus $aMap "a"
$bInputs = Get-ContiguousBus $bMap "b"
$sumOutputs = Get-ContiguousBus $fMap "f"
$bench = New-BenchLines $inputs $outputs $aInputs[0]
$lines = $bench.lines
$zero = $bench.zero

$carry = $zero
$bit = 0
while ($bit -lt $width)
	{
	if ($useSlices -and ($bit + 1) -lt $width)
		{
		$carryOutName = if (($bit + 1) -eq ($width - 1) -and $carryOut) { $carryOut } else { "carry_$($bit + 1)" }
		$lines.Add(
			"$($sumOutputs[$bit]), $($sumOutputs[$bit + 1]), $carryOutName = RIPPLE_SLICE2($($aInputs[$bit]), $($bInputs[$bit]), $($aInputs[$bit + 1]), $($bInputs[$bit + 1]), $carry)")
		$carry = $carryOutName
		$bit += 2
		continue
		}

	if ($carry -eq $zero)
		{
		$carryOutName = if ($bit -eq ($width - 1) -and $carryOut) { $carryOut } else { "carry_$bit" }
		$lines.Add("$($sumOutputs[$bit]), $carryOutName = HALF_ADDER($($aInputs[$bit]), $($bInputs[$bit]))")
		$carry = $carryOutName
		}
	else
		{
		$carryOutName = if ($bit -eq ($width - 1) -and $carryOut) { $carryOut } else { "carry_$bit" }
		$lines.Add("$($sumOutputs[$bit]), $carryOutName = FULL_ADDER($($aInputs[$bit]), $($bInputs[$bit]), $carry)")
		$carry = $carryOutName
		}

	$bit += 1
	}

if (-not $carryOut)
	{
		$lines.Add("")
	}

Write-BenchLines $outputPath $lines "$mode width=$width carryOut=$([bool]$carryOut)"
