#include "cli.h"

#include <stdio.h>
#include <string.h>

static int n2b_set_error(char *error_text, int error_size, const char *message)
{
	if (error_text != NULL && error_size > 0)
	{
		snprintf(error_text, (size_t)error_size, "%s", message);
	}

	return 0;
}

static int n2b_parse_format(const char *value, N2B_Format *format)
{
	if (strcmp(value, "auto") == 0)
	{
		*format = N2B_FORMAT_AUTO;
		return 1;
	}

	if (strcmp(value, "blif") == 0)
	{
		*format = N2B_FORMAT_BLIF;
		return 1;
	}

	if (strcmp(value, "verilog") == 0)
	{
		*format = N2B_FORMAT_VERILOG;
		return 1;
	}

	return 0;
}

int n2b_parse_cli(int argc, char **argv, N2B_Options *options, char *error_text, int error_size)
{
	int i;

	memset(options, 0, sizeof(*options));
	options->from_format = N2B_FORMAT_AUTO;
	options->overwrite_mode = N2B_OVERWRITE_ERROR;
	options->wide_gate_mode = N2B_WIDE_PRESERVE;
	options->strict_mode = 1;
	options->constants_reject = 1;

	for (i = 1; i < argc; ++i)
	{
		if (strcmp(argv[i], "--input") == 0)
		{
			if (i + 1 >= argc)
			{
				return n2b_set_error(error_text, error_size, "missing value for --input");
			}

			options->input_path = argv[++i];
			continue;
		}

		if (strcmp(argv[i], "--output") == 0)
		{
			if (i + 1 >= argc)
			{
				return n2b_set_error(error_text, error_size, "missing value for --output");
			}

			options->output_path = argv[++i];
			continue;
		}

		if (strcmp(argv[i], "--name-map") == 0)
		{
			if (i + 1 >= argc)
			{
				return n2b_set_error(error_text, error_size, "missing value for --name-map");
			}

			options->name_map_path = argv[++i];
			continue;
		}

		if (strcmp(argv[i], "--from") == 0)
		{
			if (i + 1 >= argc)
			{
				return n2b_set_error(error_text, error_size, "missing value for --from");
			}

			if (!n2b_parse_format(argv[++i], &options->from_format))
			{
				return n2b_set_error(error_text, error_size, "unsupported --from value");
			}

			continue;
		}

		if (strcmp(argv[i], "--to") == 0)
		{
			if (i + 1 >= argc)
			{
				return n2b_set_error(error_text, error_size, "missing value for --to");
			}

			++i;

			if (strcmp(argv[i], "bench") != 0)
			{
				return n2b_set_error(error_text, error_size, "phase 1 supports only --to bench");
			}

			continue;
		}

		if (strcmp(argv[i], "--strict") == 0)
		{
			options->strict_mode = 1;
			continue;
		}

		if (strcmp(argv[i], "--validate-only") == 0)
		{
			options->validate_only = 1;
			continue;
		}

		if (strcmp(argv[i], "--stdout") == 0)
		{
			options->stdout_mode = 1;
			continue;
		}

		if (strcmp(argv[i], "--quiet") == 0)
		{
			options->quiet_mode = 1;
			continue;
		}

		if (strcmp(argv[i], "--overwrite") == 0)
		{
			if (i + 1 >= argc)
			{
				return n2b_set_error(error_text, error_size, "missing value for --overwrite");
			}

			++i;

			if (strcmp(argv[i], "error") == 0)
			{
				options->overwrite_mode = N2B_OVERWRITE_ERROR;
				continue;
			}

			if (strcmp(argv[i], "replace") == 0)
			{
				options->overwrite_mode = N2B_OVERWRITE_REPLACE;
				continue;
			}

			return n2b_set_error(error_text, error_size, "unsupported --overwrite value");
		}

		if (strcmp(argv[i], "--wide-gates") == 0)
		{
			if (i + 1 >= argc)
			{
				return n2b_set_error(error_text, error_size, "missing value for --wide-gates");
			}

			++i;

			if (strcmp(argv[i], "preserve") == 0)
			{
				options->wide_gate_mode = N2B_WIDE_PRESERVE;
				continue;
			}

			if (strcmp(argv[i], "lower-binary") == 0)
			{
				options->wide_gate_mode = N2B_WIDE_LOWER_BINARY;
				continue;
			}

			return n2b_set_error(error_text, error_size, "unsupported --wide-gates value");
		}

		if (strcmp(argv[i], "--constants") == 0)
		{
			if (i + 1 >= argc)
			{
				return n2b_set_error(error_text, error_size, "missing value for --constants");
			}

			++i;

			if (strcmp(argv[i], "reject") != 0)
			{
				return n2b_set_error(error_text, error_size, "phase 1 supports only --constants reject");
			}

			continue;
		}

		return n2b_set_error(error_text, error_size, "unknown command-line flag");
	}

	if (options->input_path == NULL)
	{
		return n2b_set_error(error_text, error_size, "missing required --input");
	}

	if (!options->stdout_mode && !options->validate_only && options->output_path == NULL)
	{
		return n2b_set_error(error_text, error_size, "missing required --output");
	}

	if (options->stdout_mode && options->output_path != NULL)
	{
		return n2b_set_error(error_text, error_size, "use either --output or --stdout, not both");
	}

	return 1;
}

void n2b_print_usage(void)
{
	printf("usage: net2bench --input <path> --output <path> [--from auto|blif|verilog] [--to bench]\n");
	printf("                 [--strict] [--validate-only] [--overwrite error|replace] [--stdout]\n");
	printf("                 [--quiet] [--name-map <path>] [--wide-gates preserve|lower-binary]\n");
	printf("                 [--constants reject]\n");
}
