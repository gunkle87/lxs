param(
	[string]$InputRoot,
	[string]$OutputRoot,
	[string]$Filter = "*.blif"
)

$ErrorActionPreference = "Stop"
if ($PSVersionTable.PSVersion.Major -ge 7)
	{
	$PSNativeCommandUseErrorActionPreference = $false
	}

if ([string]::IsNullOrWhiteSpace($InputRoot) -or [string]::IsNullOrWhiteSpace($OutputRoot))
	{
	throw "Usage: batch_net2bench.ps1 -InputRoot <source_dir> -OutputRoot <dest_dir> [-Filter *.blif]"
	}

$repoRoot = Split-Path -Parent $PSScriptRoot
$net2bench = Join-Path $repoRoot "blif2bench\\net2bench.exe"

if (-not (Test-Path $net2bench))
	{
	throw "net2bench.exe not found at $net2bench"
	}

$inputRootFull = (Resolve-Path $InputRoot).Path
$outputRootFull = $OutputRoot
if (-not (Test-Path $outputRootFull))
	{
	New-Item -ItemType Directory -Force -Path $outputRootFull | Out-Null
	}

$ok = 0
$fail = 0

Get-ChildItem $inputRootFull -Recurse -Filter $Filter -File | ForEach-Object {
	$relative = $_.FullName.Substring($inputRootFull.Length).TrimStart('\')
	$outPath = Join-Path $outputRootFull ([System.IO.Path]::ChangeExtension($relative, ".bench"))
	$outDir = Split-Path -Parent $outPath
	if (-not (Test-Path $outDir))
		{
		New-Item -ItemType Directory -Force -Path $outDir | Out-Null
		}

	try
		{
		& $net2bench --input $_.FullName --output $outPath --from auto --to bench --strict --overwrite replace --quiet 1> $null 2> $null
		}
	catch
		{
		$global:LASTEXITCODE = 1
		}
	if ($LASTEXITCODE -eq 0 -and (Test-Path $outPath))
		{
		$ok++
		}
	else
		{
		if (Test-Path $outPath)
			{
			Remove-Item $outPath -Force
			}
		$fail++
		}
}

Write-Output ("OK=" + $ok)
Write-Output ("FAIL=" + $fail)
