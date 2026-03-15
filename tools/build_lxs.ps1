param(
	[string]$Configuration = "release",
	[switch]$Clean
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
$gcc = (Get-Command gcc -ErrorAction Stop).Source
$includeDir = Join-Path $repoRoot "include"
$coreDir = Join-Path $repoRoot "src\core"
$appDir = Join-Path $repoRoot "src\apps"
$outDir = Join-Path $repoRoot "build\bin"

if ($Clean -and (Test-Path $outDir))
	{
	Get-ChildItem $outDir -Filter "lxs_*.exe" -ErrorAction SilentlyContinue | Remove-Item -Force
	}

if (-not (Test-Path $outDir))
	{
	New-Item -ItemType Directory -Force -Path $outDir | Out-Null
	}

$commonFlags = @(
	"-I", $includeDir,
	"-std=c11",
	"-Wall",
	"-Wextra"
)

if ($Configuration -eq "debug")
	{
	$commonFlags += @("-O0", "-g")
	}
else
	{
	$commonFlags += @("-O3")
	}

$coreSources = @(
	(Join-Path $coreDir "lxs_api.c"),
	(Join-Path $coreDir "lxs_compiler.c"),
	(Join-Path $coreDir "lxs_engine.c")
)

$targets = @(
	@{
		Name = "lxs_bench.exe"
		Sources = @((Join-Path $appDir "lxs_bench.c")) + $coreSources
	},
	@{
		Name = "lxs_compare.exe"
		Sources = @((Join-Path $appDir "lxs_compare.c")) + $coreSources
	},
	@{
		Name = "lxs_test.exe"
		Sources = @((Join-Path $appDir "lxs_test.c")) + $coreSources
	}
)

foreach ($target in $targets)
	{
	$outPath = Join-Path $outDir $target.Name
	$args = @("-o", $outPath) + $commonFlags + $target.Sources
	Write-Host ("BUILD " + $target.Name)
	& $gcc @args
	if ($LASTEXITCODE -ne 0)
		{
		throw "build failed for $($target.Name)"
		}
	}

Write-Host "Build complete."
