$ErrorActionPreference = "Stop"

function Initialize-LxsRewrite
	{
	param(
		[string]$Mode,
		[string]$OutputPath,
		[string]$Usage
	)

	if ($Mode -ne "packed" -and $Mode -ne "anchor" -and $Mode -ne "neighborhood")
		{
		throw $Usage
		}

	$outputDir = Split-Path -Parent $OutputPath
	if ($outputDir -and -not (Test-Path $outputDir))
		{
		New-Item -ItemType Directory -Path $outputDir | Out-Null
		}
	}

function Parse-BusIndex
	{
	param([string]$Name, [string]$Prefix)

	if ($Name -match ('^' + [regex]::Escape($Prefix) + '\[(\d+)\]$'))
		{
		return [int]$Matches[1]
		}

	return -1
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

function Get-BenchIo
	{
	param([string]$InputPath)

	$rawLines = Get-Content -Path $InputPath
	$inputs = New-Object System.Collections.Generic.List[string]
	$outputs = New-Object System.Collections.Generic.List[string]

	foreach ($line in $rawLines)
		{
		$trim = $line.Trim()
		if ($trim -match '^INPUT\((.+)\)$')
			{
			$inputs.Add($Matches[1])
			}
		elseif ($trim -match '^OUTPUT\((.+)\)$')
			{
			$outputs.Add($Matches[1])
			}
		}

	return @{
		inputs = $inputs
		outputs = $outputs
	}
	}

function Get-BusMap
	{
	param(
		[System.Collections.Generic.List[string]]$Names,
		[string]$Prefix
	)

	$map = @{}
	foreach ($name in $Names)
		{
		$index = Parse-BusIndex $name $Prefix
		if ($index -ge 0)
			{
			$map[$index] = $name
			}
		}

	return $map
	}

function Get-ContiguousBus
	{
	param(
		[hashtable]$Map,
		[string]$Label
	)

	if ($Map.Count -eq 0)
		{
		throw "missing $Label bus"
		}

	$bus = New-Object object[] $Map.Count
	for ($i = 0; $i -lt $Map.Count; ++$i)
		{
		if (-not $Map.ContainsKey($i))
			{
			throw "non-contiguous ${Label} bus at bit $i"
			}

		$bus[$i] = $Map[$i]
		}

	return $bus
	}

function New-BenchLines
	{
	param(
		[System.Collections.Generic.List[string]]$Inputs,
		[System.Collections.Generic.List[string]]$Outputs,
		[string]$ZeroDriver
	)

	$lines = New-Object System.Collections.Generic.List[string]
	foreach ($name in $Inputs)
		{
		$lines.Add("INPUT($name)")
		}
	$lines.Add("")
	foreach ($name in $Outputs)
		{
		$lines.Add("OUTPUT($name)")
		}
	$lines.Add("")

	$zero = "__lxs_zero"
	$lines.Add("$zero = XOR($ZeroDriver, $ZeroDriver)")
	$lines.Add("")

	return @{
		lines = $lines
		zero = $zero
	}
	}

function Write-BenchLines
	{
	param(
		[string]$OutputPath,
		[System.Collections.Generic.List[string]]$Lines,
		[string]$Summary
	)

	[System.IO.File]::WriteAllLines($OutputPath, $Lines)
	Write-Host "Wrote $OutputPath [$Summary]"
	}
