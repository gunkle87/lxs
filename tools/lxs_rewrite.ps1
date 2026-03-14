$ErrorActionPreference = "Stop"

if ($args.Length -lt 4)
	{
	throw "usage: lxs_rewrite.ps1 <kind> <input.bench> <output.bench> <anchor|packed|neighborhood|neighborhood2|region1>"
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
	"parity4"
		{
		& (Join-Path $toolDir "gen_parity4_macro.ps1") $inputPath $outputPath
		break
		}
	"xor-fan8"
		{
		& (Join-Path $toolDir "gen_xor_fan8_macro.ps1") $inputPath $outputPath
		break
		}
	"and-fan8"
		{
		& (Join-Path $toolDir "gen_and_fan8_macro.ps1") $inputPath $outputPath
		break
		}
	"guard-chain4"
		{
		& (Join-Path $toolDir "gen_guard_chain4_macro.ps1") $inputPath $outputPath
		break
		}
	"register-en"
		{
		& (Join-Path $toolDir "gen_register_en_macro.ps1") $inputPath $outputPath
		break
		}
	"xnor-bank4"
		{
		& (Join-Path $toolDir "gen_xnor_bank4_macro.ps1") $inputPath $outputPath
		break
		}
	"counter-en"
		{
		& (Join-Path $toolDir "gen_counter_en_macro.ps1") $inputPath $outputPath
		break
		}
	"regfile2"
		{
		& (Join-Path $toolDir "gen_regfile2_macro.ps1") $inputPath $outputPath
		break
		}
	default
		{
		throw "unknown rewrite kind: $kind"
		}
	}
