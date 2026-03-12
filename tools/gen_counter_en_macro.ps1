$ErrorActionPreference = "Stop"

if ($args.Length -lt 2)
	{
	throw "usage: gen_counter_en_macro.ps1 <input.bench> <output.bench>"
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
		$outputs.Add($line.Substring(7, $line.Length - 8).Trim())
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

$registerOutputs = @("q0","q1","q2","q3")
$removeNames = New-Object System.Collections.Generic.HashSet[string]
foreach ($name in @("n_en","s0","c1","s1","c2","s2","c3","s3"))
	{
	$removeNames.Add($name) | Out-Null
	}
for ($bit = 0; $bit -lt 4; ++$bit)
	{
	$removeNames.Add(("h{0}" -f $bit)) | Out-Null
	$removeNames.Add(("l{0}" -f $bit)) | Out-Null
	$removeNames.Add(("n{0}" -f $bit)) | Out-Null
	$removeNames.Add(("q{0}" -f $bit)) | Out-Null
	}

$outLines = New-Object System.Collections.Generic.List[string]
foreach ($line in $inputs)
	{
	$outLines.Add($line)
	}
foreach ($name in $outputs)
	{
	$outLines.Add(("OUTPUT({0})" -f $name))
	}
$outLines.Add(("{0} = COUNTER_EN(en)" -f ($registerOutputs -join ",")))
foreach ($lhs in $assignmentOrder)
	{
	if ($removeNames.Contains($lhs))
		{
		continue
		}
	$outLines.Add(("{0} = {1}" -f $lhs, $assignments[$lhs]))
	}
Set-Content -Path $outputPath -Value $outLines
