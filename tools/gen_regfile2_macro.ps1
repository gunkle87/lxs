$ErrorActionPreference = "Stop"

if ($args.Length -lt 2)
	{
	throw "usage: gen_regfile2_macro.ps1 <input.bench> <output.bench>"
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

$removeNames = New-Object System.Collections.Generic.HashSet[string]
foreach ($name in @("nra","nwa","sr0","sr1","sw0","sw1","nsw0","nsw1"))
	{
	$removeNames.Add($name) | Out-Null
	}
foreach ($name in @("m0","m1","m2","m3"))
	{
	$removeNames.Add($name) | Out-Null
	}
for ($bit = 0; $bit -lt 4; ++$bit)
	{
	$removeNames.Add(("m{0}r0" -f $bit)) | Out-Null
	$removeNames.Add(("m{0}r1" -f $bit)) | Out-Null
	$removeNames.Add(("q0{0}" -f $bit)) | Out-Null
	$removeNames.Add(("q1{0}" -f $bit)) | Out-Null
	$removeNames.Add(("h0{0}" -f $bit)) | Out-Null
	$removeNames.Add(("l0{0}" -f $bit)) | Out-Null
	$removeNames.Add(("n0{0}" -f $bit)) | Out-Null
	$removeNames.Add(("h1{0}" -f $bit)) | Out-Null
	$removeNames.Add(("l1{0}" -f $bit)) | Out-Null
	$removeNames.Add(("n1{0}" -f $bit)) | Out-Null
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
$outLines.Add("m0,m1,m2,m3 = REGFILE2(ra ; wa ; d0,d1,d2,d3 ; we ; 0x0,0x0)")
foreach ($lhs in $assignmentOrder)
	{
	if ($removeNames.Contains($lhs) -or $lhs -match '^m[0-3]$')
		{
		continue
		}
	$outLines.Add(("{0} = {1}" -f $lhs, $assignments[$lhs]))
	}
Set-Content -Path $outputPath -Value $outLines
