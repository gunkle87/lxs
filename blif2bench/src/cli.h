#ifndef N2B_CLI_H
#define N2B_CLI_H

typedef enum N2B_Format
{
	N2B_FORMAT_AUTO = 0,
	N2B_FORMAT_BLIF,
	N2B_FORMAT_VERILOG
} N2B_Format;

typedef enum N2B_Overwrite
{
	N2B_OVERWRITE_ERROR = 0,
	N2B_OVERWRITE_REPLACE
} N2B_Overwrite;

typedef enum N2B_WideGateMode
{
	N2B_WIDE_PRESERVE = 0,
	N2B_WIDE_LOWER_BINARY
} N2B_WideGateMode;

typedef struct N2B_Options
{
	const char *input_path;
	const char *output_path;
	const char *name_map_path;
	N2B_Format from_format;
	N2B_Overwrite overwrite_mode;
	N2B_WideGateMode wide_gate_mode;
	int strict_mode;
	int validate_only;
	int stdout_mode;
	int quiet_mode;
	int constants_reject;
} N2B_Options;

int n2b_parse_cli(int argc, char **argv, N2B_Options *options, char *error_text, int error_size);
void n2b_print_usage(void);

#endif
