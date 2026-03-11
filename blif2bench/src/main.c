#include <stdio.h>
#include <string.h>

#include "bench_emit.h"
#include "blif_parser.h"
#include "cli.h"
#include "normalize.h"
#include "report.h"
#include "validate.h"
#include "verilog_parser.h"

static int n2b_file_exists(const char *path)
{
	FILE *stream;

	stream = fopen(path, "rb");
	if (stream == NULL)
	{
		return 0;
	}

	fclose(stream);
	return 1;
}

static int n2b_path_has_suffix(const char *path, const char *suffix)
{
	size_t path_len;
	size_t suffix_len;

	path_len = strlen(path);
	suffix_len = strlen(suffix);
	if (path_len < suffix_len)
	{
		return 0;
	}

	return strcmp(path + path_len - suffix_len, suffix) == 0;
}

static N2B_Format n2b_detect_format(const char *path)
{
	if (n2b_path_has_suffix(path, ".blif"))
	{
		return N2B_FORMAT_BLIF;
	}

	if (n2b_path_has_suffix(path, ".v") || n2b_path_has_suffix(path, ".verilog"))
	{
		return N2B_FORMAT_VERILOG;
	}

	return N2B_FORMAT_AUTO;
}

static int n2b_write_output_file(const char *path, const N2B_Options *options, const N2B_IR *ir, const N2B_Normalized *normalized, char *error_text, int error_size)
{
	FILE *stream;

	if (options->overwrite_mode == N2B_OVERWRITE_ERROR && n2b_file_exists(path))
	{
		snprintf(error_text, (size_t)error_size, "refusing to overwrite existing file: %s", path);
		return 0;
	}

	stream = fopen(path, "wb");
	if (stream == NULL)
	{
		snprintf(error_text, (size_t)error_size, "failed to open output file: %s", path);
		return 0;
	}

	if (!n2b_emit_bench(stream, ir, normalized, options->wide_gate_mode, error_text, error_size))
	{
		fclose(stream);
		return 0;
	}

	fclose(stream);
	return 1;
}

static int n2b_write_name_map_file(const char *path, const N2B_Options *options, const N2B_IR *ir, char *error_text, int error_size)
{
	FILE *stream;

	if (options->overwrite_mode == N2B_OVERWRITE_ERROR && n2b_file_exists(path))
	{
		snprintf(error_text, (size_t)error_size, "refusing to overwrite existing file: %s", path);
		return 0;
	}

	stream = fopen(path, "wb");
	if (stream == NULL)
	{
		snprintf(error_text, (size_t)error_size, "failed to open name-map file: %s", path);
		return 0;
	}

	if (!n2b_write_name_map(stream, ir, error_text, error_size))
	{
		fclose(stream);
		return 0;
	}

	fclose(stream);
	return 1;
}

int main(int argc, char **argv)
{
	N2B_Options options;
	N2B_IR ir;
	N2B_Normalized normalized;
	N2B_Format format;
	char error_text[512];
	int ok;

	memset(error_text, 0, sizeof(error_text));
	if (!n2b_parse_cli(argc, argv, &options, error_text, (int)sizeof(error_text)))
	{
		fprintf(stderr, "error: %s\n", error_text);
		n2b_print_usage();
		return 1;
	}

	format = (options.from_format == N2B_FORMAT_AUTO) ? n2b_detect_format(options.input_path) : options.from_format;
	if (format == N2B_FORMAT_AUTO)
	{
		fprintf(stderr, "error: unable to detect input format, use --from\n");
		return 1;
	}

	n2b_ir_init(&ir);
	n2b_normalized_init(&normalized);

	ok = (format == N2B_FORMAT_BLIF) ?
		n2b_parse_blif(options.input_path, &ir, error_text, (int)sizeof(error_text)) :
		n2b_parse_verilog(options.input_path, &ir, error_text, (int)sizeof(error_text));

	if (!ok)
	{
		fprintf(stderr, "error: %s\n", error_text);
		n2b_normalized_free(&normalized);
		n2b_ir_free(&ir);
		return 1;
	}

	if (!n2b_normalize_ir(&ir, &normalized, error_text, (int)sizeof(error_text)) ||
		!n2b_validate_normalized(&ir, &normalized, error_text, (int)sizeof(error_text)))
	{
		fprintf(stderr, "error: %s\n", error_text);
		n2b_normalized_free(&normalized);
		n2b_ir_free(&ir);
		return 1;
	}

	if (!options.quiet_mode)
	{
		n2b_print_summary(&ir, &normalized);
	}

	if (!options.validate_only)
	{
		if (options.stdout_mode)
		{
			if (!n2b_emit_bench(stdout, &ir, &normalized, options.wide_gate_mode, error_text, (int)sizeof(error_text)))
			{
				fprintf(stderr, "error: %s\n", error_text);
				n2b_normalized_free(&normalized);
				n2b_ir_free(&ir);
				return 1;
			}
		}
		else if (!n2b_write_output_file(options.output_path, &options, &ir, &normalized, error_text, (int)sizeof(error_text)))
		{
			fprintf(stderr, "error: %s\n", error_text);
			n2b_normalized_free(&normalized);
			n2b_ir_free(&ir);
			return 1;
		}
	}

	if (options.name_map_path != NULL && !n2b_write_name_map_file(options.name_map_path, &options, &ir, error_text, (int)sizeof(error_text)))
	{
		fprintf(stderr, "error: %s\n", error_text);
		n2b_normalized_free(&normalized);
		n2b_ir_free(&ir);
		return 1;
	}

	n2b_normalized_free(&normalized);
	n2b_ir_free(&ir);
	return 0;
}
