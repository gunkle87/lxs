$ErrorActionPreference = "Stop"

$samples = if ($args.Length -gt 0) { [int]$args[0] } else { 5 }
$sampleSelect = if ($args.Length -gt 1) { $args[1] } else { "median" }
$iterations = if ($args.Length -gt 2) { [int]$args[2] } else { 1 }
$cycles = if ($args.Length -gt 3) { [int]$args[3] } else { 10000 }
$trials = if ($args.Length -gt 4) { [int]$args[4] } else { 512 }

$root = Split-Path -Parent $MyInvocation.MyCommand.Path | Split-Path -Parent
$benchExe = Join-Path $root "build\bin\lxs_bench.exe"
$compareExe = Join-Path $root "build\bin\lxs_compare.exe"
$reportPath = Join-Path $root "Benchmarks\Generated\rewrite_report.csv"

$cases = @(
	@{
		Name = "c499_and_fan8"
		Class = "routing_fanout"
		Original = "Benchmarks\Benches\ISCAS85\c499.bench"
		Anchor = "Benchmarks\Generated\c499_and_fan8.bench"
		Packed = "Benchmarks\Generated\c499_and_fan8.bench"
		Trials = 2048
	},
	@{
		Name = "c499"
		Class = "parity"
		Original = "Benchmarks\Benches\ISCAS85\c499.bench"
		Anchor = "Benchmarks\Generated\c499_parity4.bench"
		Packed = "Benchmarks\Generated\c499_parity8.bench"
		Trials = 2048
	},
	@{
		Name = "c6288"
		Class = "multiplier"
		Original = "Benchmarks\Benches\ISCAS85\c6288.bench"
		Anchor = "Benchmarks\Generated\c6288_macro_anchor.bench"
		Packed = "Benchmarks\Generated\c6288_macro_packed.bench"
		Trials = 2048
	},
	@{
		Name = "epfl_adder"
		Class = "adder"
		Original = "Benchmarks\Benches\EPFL\adder.bench"
		Anchor = "Benchmarks\Generated\adder_macro_anchor.bench"
		Packed = "Benchmarks\Generated\adder_macro_packed.bench"
		Trials = $trials
	},
	@{
		Name = "epfl_multiplier"
		Class = "multiplier"
		Original = "Benchmarks\Benches\EPFL\multiplier.bench"
		Anchor = "Benchmarks\Generated\multiplier_macro_anchor.bench"
		Packed = "Benchmarks\Generated\multiplier_macro_packed.bench"
		Trials = $trials
	}
)

function Invoke-LxsBenchGeps
	{
	param([string]$BenchPath)

	$raw = & $benchExe $BenchPath --samples $samples --sample-select $sampleSelect --iterations $iterations --cycles $cycles
	if ($LASTEXITCODE -ne 0)
		{
		throw "bench failed for $BenchPath"
		}

	$line = ($raw | Select-String '^TOTAL,').Line
	if (-not $line)
		{
		throw "failed to parse TOTAL line for $BenchPath"
		}

	$fields = $line.Split(',')
	return [double]$fields[$fields.Length - 1]
	}

function Invoke-LxsCompareStatus
	{
	param(
		[string]$Lhs,
		[string]$Rhs,
		[int]$CaseTrials
	)

	$output = & $compareExe $Lhs $Rhs $CaseTrials 2>&1
	if ($LASTEXITCODE -eq 0)
		{
		return "equivalent"
		}

	return ($output | Out-String).Trim()
	}

$rows = New-Object System.Collections.Generic.List[object]
foreach ($case in $cases)
	{
	$originalGeps = Invoke-LxsBenchGeps $case.Original
	$anchorGeps = Invoke-LxsBenchGeps $case.Anchor
	$packedGeps = Invoke-LxsBenchGeps $case.Packed
	$anchorEq = Invoke-LxsCompareStatus $case.Original $case.Anchor $case.Trials
	$packedEq = Invoke-LxsCompareStatus $case.Original $case.Packed $case.Trials

	$rows.Add([pscustomobject]@{
		case_name = $case.Name
		class = $case.Class
		original = $case.Original
		anchor = $case.Anchor
		packed = $case.Packed
		anchor_equiv = $anchorEq
		packed_equiv = $packedEq
		original_geps = ('{0:F6}' -f $originalGeps)
		anchor_geps = ('{0:F6}' -f $anchorGeps)
		packed_geps = ('{0:F6}' -f $packedGeps)
		anchor_delta = ('{0:F6}' -f ($anchorGeps - $originalGeps))
		packed_delta = ('{0:F6}' -f ($packedGeps - $originalGeps))
	})
	}

$rows | Export-Csv -NoTypeInformation -Path $reportPath
$rows | Format-Table -AutoSize
Write-Host ""
Write-Host "Wrote $reportPath"
