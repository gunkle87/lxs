$ErrorActionPreference = "Stop"

$maskOff = if ($args.Length -gt 0) { $args[0] } else { "0" }
$maskOn = if ($args.Length -gt 1) { $args[1] } else { "1" }
$benchPath = if ($args.Length -gt 2) { $args[2] } else { "Benchmarks\\Benches" }
$samples = if ($args.Length -gt 3) { [int]$args[3] } else { 5 }
$sampleSelect = if ($args.Length -gt 4) { $args[4] } else { "median" }
$iterations = if ($args.Length -gt 5) { [int]$args[5] } else { 1 }
$cycles = if ($args.Length -gt 6) { [int]$args[6] } else { 10000 }

$root = Split-Path -Parent $MyInvocation.MyCommand.Path | Split-Path -Parent
$benchExe = Join-Path $root "build\\bin\\lxs_bench.exe"

function Invoke-LxsBenchTable
	{
	param([string]$Mask)

	$env:LXS_RECOGNITION_MASK = $Mask
	try
		{
		$raw = & $benchExe $benchPath --samples $samples --sample-select $sampleSelect --iterations $iterations --cycles $cycles
		if ($LASTEXITCODE -ne 0)
			{
			throw "bench failed for mask $Mask"
			}
		}
	finally
		{
		Remove-Item Env:LXS_RECOGNITION_MASK -ErrorAction SilentlyContinue
		}

	$rows = New-Object System.Collections.Generic.List[object]
	foreach ($line in $raw)
		{
		if ($line.StartsWith("#") -or [string]::IsNullOrWhiteSpace($line))
			{
			continue
			}
		if ($line.StartsWith("Circuit Name,") -or $line.StartsWith("AVERAGE,"))
			{
			continue
			}

		$fields = $line.Split(",")
		if ($fields.Length -lt 20)
			{
			continue
			}

		$rows.Add([pscustomobject]@{
			Circuit = $fields[0]
			GateEval = if ($fields[8] -eq "N/A") { 0.0 } else { [double]$fields[8] }
			TotalTime = if ($fields[18] -eq "N/A") { 0.0 } else { [double]$fields[18] }
			GEPS = if ($fields[19] -eq "N/A") { 0.0 } else { [double]$fields[19] }
		})
		}

	return $rows
	}

function Get-SuiteName
	{
	param([string]$Circuit)

	if ($Circuit -eq "TOTAL")
		{
		return "TOTAL"
		}
	$slash = $Circuit.IndexOf('\')
	if ($slash -lt 0)
		{
		return "MISC"
		}
	return $Circuit.Substring(0, $slash)
	}

function Get-WeightedGeps
	{
	param($Rows)

	$totalGateEval = ($Rows | Measure-Object -Property GateEval -Sum).Sum
	$totalTime = ($Rows | Measure-Object -Property TotalTime -Sum).Sum
	if ($totalTime -le 0.0)
		{
		return 0.0
		}
	return $totalGateEval / $totalTime
	}

$offRows = Invoke-LxsBenchTable $maskOff
$onRows = Invoke-LxsBenchTable $maskOn

$benchNames = New-Object System.Collections.Generic.HashSet[string]
foreach ($row in $offRows)
	{
	if ($row.Circuit -ne "TOTAL")
		{
		[void]$benchNames.Add($row.Circuit)
		}
	}
foreach ($row in $onRows)
	{
	if ($row.Circuit -ne "TOTAL")
		{
		[void]$benchNames.Add($row.Circuit)
		}
	}

$offByCircuit = @{}
$onByCircuit = @{}
foreach ($row in $offRows) { $offByCircuit[$row.Circuit] = $row }
foreach ($row in $onRows) { $onByCircuit[$row.Circuit] = $row }

$suiteNames = @("ISCAS85", "ISCAS89", "ITC99", "EPFL")

$overallOff = Get-WeightedGeps ($offRows | Where-Object { $_.Circuit -ne "TOTAL" })
$overallOn = Get-WeightedGeps ($onRows | Where-Object { $_.Circuit -ne "TOTAL" })

Write-Host "Recognition Delta"
Write-Host "  mask_off: $maskOff"
Write-Host "  mask_on : $maskOn"
Write-Host "  overall_off: $('{0:F6}' -f $overallOff)"
Write-Host "  overall_on : $('{0:F6}' -f $overallOn)"
Write-Host "  overall_delta: $('{0:F6}' -f ($overallOn - $overallOff))"
Write-Host ""

Write-Host "Per-Suite Weighted Delta"
foreach ($suite in $suiteNames)
	{
	$offSuite = $offRows | Where-Object { (Get-SuiteName $_.Circuit) -eq $suite }
	$onSuite = $onRows | Where-Object { (Get-SuiteName $_.Circuit) -eq $suite }
	$offSuiteGeps = Get-WeightedGeps $offSuite
	$onSuiteGeps = Get-WeightedGeps $onSuite
	Write-Host "  $suite off=$('{0:F6}' -f $offSuiteGeps) on=$('{0:F6}' -f $onSuiteGeps) delta=$('{0:F6}' -f ($onSuiteGeps - $offSuiteGeps))"
	}

Write-Host ""
Write-Host "Per-Benchmark Delta"
foreach ($name in ($benchNames | Sort-Object))
	{
	$offRow = $offByCircuit[$name]
	$onRow = $onByCircuit[$name]
	if (-not $offRow -or -not $onRow)
		{
		continue
		}
	Write-Host "  $name off=$('{0:F6}' -f $offRow.GEPS) on=$('{0:F6}' -f $onRow.GEPS) delta=$('{0:F6}' -f ($onRow.GEPS - $offRow.GEPS))"
	}
