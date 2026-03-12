$ErrorActionPreference = "Stop"

if ($args.Length -lt 2)
	{
	throw "usage: gen_register_en_macro.ps1 <input.bench> <output.bench>"
	}

$inputPath = $args[0]
$outputPath = $args[1]

$lines = Get-Content $inputPath
$inputs = New-Object System.Collections.Generic.List[string]
$outputs = New-Object System.Collections.Generic.List[string]
$assignments = @{}
$assignmentOrder = New-Object System.Collections.Generic.List[string]

foreach ($rawLine in $lines)
	{
	$line = $rawLine.Trim()
	if ($line.Length -eq 0 -or $line.StartsWith("#"))
		{
		continue
		}

	if ($line.StartsWith("INPUT("))
		{
		$inputs.Add($line)
		continue
		}

	if ($line.StartsWith("OUTPUT("))
		{
		$name = $line.Substring(7, $line.Length - 8).Trim()
		$outputs.Add($name)
		continue
		}

	$parts = $line.Split("=", 2)
	if ($parts.Length -ne 2)
		{
		throw "malformed assignment: $line"
		}

	$lhs = $parts[0].Trim()
	$rhs = $parts[1].Trim()
	$assignments[$lhs] = $rhs
	$assignmentOrder.Add($lhs)
	}

$enableNet = $null
$invertedEnableNet = $null
foreach ($entry in $assignments.GetEnumerator())
	{
	$rhs = $entry.Value
	if ($rhs -match '^NOT\(([^)]+)\)$')
		{
		$enableNet = $matches[1].Trim()
		$invertedEnableNet = $entry.Key
		break
		}
	}

if (-not $enableNet -or -not $invertedEnableNet)
	{
	throw "failed to locate shared enable inverter"
	}

$registerOutputs = New-Object System.Collections.Generic.List[string]
foreach ($name in $outputs)
	{
	if ($assignments.ContainsKey($name) -and $assignments[$name] -match '^DFF\(([^)]+)\)$')
		{
		$registerOutputs.Add($name)
		}
	}

if ($registerOutputs.Count -eq 0)
	{
	throw "failed to locate DFF-driven register outputs"
	}

$dataInputs = New-Object System.Collections.Generic.List[string]
$removeNames = New-Object System.Collections.Generic.HashSet[string]
$removeNames.Add($invertedEnableNet) | Out-Null
foreach ($q in $registerOutputs)
	{
	if (-not $assignments.ContainsKey($q))
		{
		throw "missing DFF assignment for $q"
		}

	$dffRhs = $assignments[$q]
	if ($dffRhs -notmatch '^DFF\(([^)]+)\)$')
		{
		throw "output $q is not driven by DFF"
		}

	$dffInput = $matches[1].Trim()
	if (-not $assignments.ContainsKey($dffInput))
		{
		throw "missing OR stage for $q"
		}

	$orRhs = $assignments[$dffInput]
	if ($orRhs -notmatch '^OR\(([^,]+),([^)]+)\)$')
		{
		throw "D input $dffInput is not an OR stage"
		}

	$orA = $matches[1].Trim()
	$orB = $matches[2].Trim()
	$candidates = @($orA, $orB)
	$dataNet = $null

	foreach ($candidate in $candidates)
		{
		if (-not $assignments.ContainsKey($candidate))
			{
			continue
			}

		$andRhs = $assignments[$candidate]
		if ($andRhs -match '^AND\(([^,]+),([^)]+)\)$')
			{
			$a = $matches[1].Trim()
			$b = $matches[2].Trim()
			if (($a -eq $q -and $b -eq $invertedEnableNet) -or ($b -eq $q -and $a -eq $invertedEnableNet))
				{
				continue
				}
			if (($a -eq $enableNet) -or ($b -eq $enableNet))
				{
				$dataNet = if ($a -eq $enableNet) { $b } else { $a }
				break
				}
			}
		}

	if (-not $dataNet)
		{
		throw "failed to derive data input for $q"
		}

	$dataInputs.Add($dataNet)
	$removeNames.Add($q) | Out-Null
	$removeNames.Add($dffInput) | Out-Null
	$removeNames.Add($orA) | Out-Null
	$removeNames.Add($orB) | Out-Null
	}

$outLines = New-Object System.Collections.Generic.List[string]
foreach ($line in $inputs)
	{
	$outLines.Add($line)
	}
foreach ($name in $outputs)
	{
	$outLines.Add("OUTPUT($name)")
	}
$outLines.Add(("{0} = REGISTER_EN({1} ; {2})" -f ($registerOutputs -join ","), ($dataInputs -join ","), $enableNet))
foreach ($lhs in $assignmentOrder)
	{
	if ($removeNames.Contains($lhs))
		{
		continue
		}
	$outLines.Add(("{0} = {1}" -f $lhs, $assignments[$lhs]))
	}

$outDir = Split-Path -Parent $outputPath
if ($outDir -and -not (Test-Path $outDir))
	{
	New-Item -ItemType Directory -Path $outDir | Out-Null
	}

Set-Content -Path $outputPath -Value $outLines
