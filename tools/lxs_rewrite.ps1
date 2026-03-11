$ErrorActionPreference = "Stop"

if ($args.Length -lt 4)
	{
	throw "usage: lxs_rewrite.ps1 <kind> <input.bench> <output.bench> <anchor|packed>"
	}

$kind = $args[0].ToLowerInvariant()
$inputPath = $args[1]
$outputPath = $args[2]
$mode = $args[3].ToLowerInvariant()
$toolDir = Split-Path -Parent $MyInvocation.MyCommand.Path

switch ($kind)
	{
	"vector-adder"
		{
		& (Join-Path $toolDir "gen_vector_adder_macro.ps1") $inputPath $outputPath $mode
		break
		}
	"vector-multiplier"
		{
		& (Join-Path $toolDir "gen_vector_multiplier_macro.ps1") $inputPath $outputPath $mode
		break
		}
	default
		{
		throw "unknown rewrite kind: $kind"
		}
	}
