$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot
$repo = Split-Path -Parent $root
$manifest = Join-Path $root "corpus\manifest.csv"
$exe = Join-Path $repo "net2bench.exe"
$rows = Import-Csv $manifest

if (!(Test-Path $exe))
{
	throw "missing executable: $exe"
}

New-Item -ItemType Directory -Force -Path (Join-Path $root "artifacts") | Out-Null

foreach ($row in $rows)
{
	$input = Join-Path $repo $row.input_path
	$flags = @()

	if ($row.flags)
	{
		$flags = $row.flags.Split(' ', [System.StringSplitOptions]::RemoveEmptyEntries)
	}

	if ($row.expect -eq "pass")
	{
		$output = Join-Path $repo $row.output_path
		if (Test-Path $output)
		{
			Remove-Item $output -Force
		}

		$args = @("--input", $input, "--from", $row.format, "--output", $output) + $flags
		if ($row.name_map_path)
		{
			$nameMap = Join-Path $repo $row.name_map_path
			if (Test-Path $nameMap)
			{
				Remove-Item $nameMap -Force
			}

			$args += @("--name-map", $nameMap)
		}

		& $exe @args 2>$null | Out-Null
		if ($LASTEXITCODE -ne 0)
		{
			throw "expected pass but failed: $($row.id)"
		}

		$firstRun = Get-Content -Raw $output
		$golden = Join-Path $root ("golden\" + $row.id + ".bench")
		if ($firstRun -ne (Get-Content -Raw $golden))
		{
			throw "golden mismatch: $($row.id)"
		}

		& $exe @args 2>$null | Out-Null
		if ($LASTEXITCODE -ne 0)
		{
			throw "determinism rerun failed: $($row.id)"
		}

		if ($firstRun -ne (Get-Content -Raw $output))
		{
			throw "determinism mismatch: $($row.id)"
		}

		if ($row.name_map_path)
		{
			$goldenMap = Join-Path $root ("golden\" + $row.id + ".csv")
			if ((Get-Content -Raw $nameMap) -ne (Get-Content -Raw $goldenMap))
			{
				throw "name-map mismatch: $($row.id)"
			}
		}

		continue
	}

	$failed = $false
	$args = @("--input", $input, "--from", $row.format, "--stdout") + $flags
	try
	{
		& $exe @args 2>$null | Out-Null
		$failed = ($LASTEXITCODE -ne 0)
	}
	catch
	{
		$failed = $true
	}

	if (-not $failed)
	{
		throw "expected failure but passed: $($row.id)"
	}
}

Write-Host "corpus passed"
