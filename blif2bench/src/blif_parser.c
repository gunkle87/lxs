#include "blif_parser.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N2B_BLIF_MAX_TOKENS 128
#define N2B_BLIF_MAX_CUBES 128
#define N2B_BLIF_LINE_SIZE 1024

static int n2b_set_error(char *error_text, int error_size, const char *message)
{
	if (error_text != NULL && error_size > 0)
	{
		snprintf(error_text, (size_t)error_size, "%s", message);
	}

	return 0;
}

static int n2b_set_blif_error(const char *code, const char *path, int line_number, const char *message, char *error_text, int error_size)
{
	if (error_text != NULL && error_size > 0)
	{
		snprintf(
			error_text,
			(size_t)error_size,
			"error[%s]: %s:%d: %s",
			code,
			path,
			line_number,
			message);
	}

	return 0;
}

static char *n2b_trim(char *text)
{
	char *end;

	while (*text != '\0' && isspace((unsigned char)*text))
	{
		++text;
	}

	end = text + strlen(text);
	while (end > text && isspace((unsigned char)end[-1]))
	{
		--end;
	}

	*end = '\0';
	return text;
}

static int n2b_read_logical_line(FILE *stream, char *buffer, int buffer_size, int *physical_line_number, int *logical_line_number)
{
	char line[N2B_BLIF_LINE_SIZE];
	size_t length;
	int first_line;

	buffer[0] = '\0';
	first_line = 0;

	while (fgets(line, sizeof(line), stream) != NULL)
	{
		char *comment;
		size_t current_length;
		int trimmed_empty;

		++(*physical_line_number);
		if (first_line == 0)
		{
			first_line = *physical_line_number;
		}

		comment = strchr(line, '#');
		if (comment != NULL)
		{
			*comment = '\0';
		}

		current_length = strlen(buffer);
		length = strlen(line);
		if ((int)(current_length + length + 1U) >= buffer_size)
		{
			return -1;
		}

		memcpy(buffer + current_length, line, length + 1U);
		length = strlen(buffer);

		while (length > 0U && (buffer[length - 1U] == '\n' || buffer[length - 1U] == '\r'))
		{
			buffer[--length] = '\0';
		}

		if (length > 0U && buffer[length - 1U] == '\\')
		{
			buffer[length - 1U] = ' ';
			continue;
		}

		trimmed_empty = (n2b_trim(buffer)[0] == '\0');
		if (trimmed_empty)
		{
			buffer[0] = '\0';
			first_line = 0;
			continue;
		}

		*logical_line_number = first_line;
		return 1;
	}

	if (buffer[0] != '\0')
	{
		*logical_line_number = first_line;
		return 1;
	}

	return 0;
}

static int n2b_split_tokens(char *line, char **tokens, int max_tokens)
{
	int count;
	char *token;

	count = 0;
	token = strtok(line, " \t\r\n");

	while (token != NULL)
	{
		if (count >= max_tokens)
		{
			return -1;
		}

		tokens[count++] = token;
		token = strtok(NULL, " \t\r\n");
	}

	return count;
}

static int n2b_cube_all_ones(const char *cube)
{
	int index;

	for (index = 0; cube[index] != '\0'; ++index)
	{
		if (cube[index] != '1')
		{
			return 0;
		}
	}

	return 1;
}

static int n2b_cube_is_or_term(const char *cube, int width)
{
	int ones;
	int index;

	ones = 0;

	for (index = 0; index < width; ++index)
	{
		if (cube[index] == '1')
		{
			++ones;
			continue;
		}

		if (cube[index] != '-')
		{
			return 0;
		}
	}

	return ones == 1;
}

static int n2b_lower_sop(N2B_IR *ir, int output_net, const int *input_nets, int input_count, char cubes[][N2B_BLIF_MAX_TOKENS], int cube_count, char *error_text, int error_size)
{
	int term_nets[N2B_BLIF_MAX_CUBES];
	int term_count;
	int cube_index;

	if (cube_count == 0)
	{
		return n2b_set_error(error_text, error_size, "BLIF constant 0 is unsupported in phase 1");
	}

	term_count = 0;

	for (cube_index = 0; cube_index < cube_count; ++cube_index)
	{
		int literal_nets[N2B_BLIF_MAX_TOKENS];
		int literal_count;
		int input_index;

		literal_count = 0;

		for (input_index = 0; input_index < input_count; ++input_index)
		{
			if (cubes[cube_index][input_index] == '1')
			{
				literal_nets[literal_count++] = input_nets[input_index];
				continue;
			}

			if (cubes[cube_index][input_index] == '0')
			{
				int temp_net;
				int temp_inputs[1];

				temp_net = n2b_ir_make_temp(ir, error_text, error_size);
				if (temp_net < 0)
				{
					return 0;
				}

				temp_inputs[0] = input_nets[input_index];
				if (n2b_ir_add_node(ir, N2B_OP_NOT, temp_net, temp_inputs, 1, 0, error_text, error_size) < 0)
				{
					return 0;
				}

				literal_nets[literal_count++] = temp_net;
				continue;
			}

			if (cubes[cube_index][input_index] != '-')
			{
				return n2b_set_error(error_text, error_size, "invalid BLIF cube character");
			}
		}

		if (literal_count == 0)
		{
			return n2b_set_error(error_text, error_size, "BLIF constant 1 is unsupported in phase 1");
		}

		if (literal_count == 1)
		{
			term_nets[term_count++] = literal_nets[0];
			continue;
		}

		{
			int temp_net;

			temp_net = n2b_ir_make_temp(ir, error_text, error_size);
			if (temp_net < 0)
			{
				return 0;
			}

			if (n2b_ir_add_node(ir, N2B_OP_AND, temp_net, literal_nets, literal_count, 0, error_text, error_size) < 0)
			{
				return 0;
			}

			term_nets[term_count++] = temp_net;
		}
	}

	if (term_count == 1)
	{
		int buf_input[1];

		buf_input[0] = term_nets[0];
		return n2b_ir_add_node(ir, N2B_OP_BUF, output_net, buf_input, 1, 0, error_text, error_size) >= 0;
	}

	return n2b_ir_add_node(ir, N2B_OP_OR, output_net, term_nets, term_count, 0, error_text, error_size) >= 0;
}

static int n2b_handle_names(FILE *stream, const char *path, N2B_IR *ir, char **tokens, int token_count, int *physical_line_number, char *error_text, int error_size)
{
	char cube_lines[N2B_BLIF_MAX_CUBES][N2B_BLIF_MAX_TOKENS];
	char line[N2B_BLIF_LINE_SIZE * 2];
	int input_nets[N2B_BLIF_MAX_TOKENS];
	int input_count;
	int output_net;
	int cube_count;

	(void)path;

	if (token_count < 2)
	{
		return n2b_set_error(error_text, error_size, "BLIF .names requires at least one output");
	}

	input_count = token_count - 2;
	cube_count = 0;

	for (int index = 0; index < input_count; ++index)
	{
		input_nets[index] = n2b_ir_intern_net(ir, tokens[index + 1], error_text, error_size);
		if (input_nets[index] < 0)
		{
			return 0;
		}
	}

	output_net = n2b_ir_intern_net(ir, tokens[token_count - 1], error_text, error_size);
	if (output_net < 0)
	{
		return 0;
	}

	while (1)
	{
		long file_pos;
		char *row_tokens[N2B_BLIF_MAX_TOKENS];
		int row_count;
		char row_copy[N2B_BLIF_LINE_SIZE * 2];

		int logical_line_number;
		int read_status;

		file_pos = ftell(stream);
		read_status = n2b_read_logical_line(stream, line, (int)sizeof(line), physical_line_number, &logical_line_number);
		if (read_status <= 0)
		{
			break;
		}

		if (n2b_trim(line)[0] == '.')
		{
			fseek(stream, file_pos, SEEK_SET);
			break;
		}

		snprintf(row_copy, sizeof(row_copy), "%s", line);
		row_count = n2b_split_tokens(row_copy, row_tokens, N2B_BLIF_MAX_TOKENS);
		if (row_count < 1)
		{
			continue;
		}

		if (cube_count >= N2B_BLIF_MAX_CUBES)
		{
			return n2b_set_error(error_text, error_size, "too many BLIF cubes in one .names block");
		}

		if (input_count == 0)
		{
			return n2b_set_error(error_text, error_size, "BLIF constants are unsupported in phase 1");
		}

		if (row_count != 2 || strcmp(row_tokens[1], "1") != 0)
		{
			return n2b_set_error(error_text, error_size, "BLIF phase 1 accepts only ON-set rows ending in 1");
		}

		if ((int)strlen(row_tokens[0]) != input_count)
		{
			return n2b_set_error(error_text, error_size, "BLIF cube width does not match .names input count");
		}

		snprintf(cube_lines[cube_count++], sizeof(cube_lines[0]), "%s", row_tokens[0]);
	}

	if (input_count == 1 && cube_count == 1 && strcmp(cube_lines[0], "1") == 0)
	{
		return n2b_ir_add_node(ir, N2B_OP_BUF, output_net, input_nets, 1, 0, error_text, error_size) >= 0;
	}

	if (input_count == 1 && cube_count == 1 && strcmp(cube_lines[0], "0") == 0)
	{
		return n2b_ir_add_node(ir, N2B_OP_NOT, output_net, input_nets, 1, 0, error_text, error_size) >= 0;
	}

	if (input_count >= 2 && cube_count == 1 && n2b_cube_all_ones(cube_lines[0]))
	{
		return n2b_ir_add_node(ir, N2B_OP_AND, output_net, input_nets, input_count, 0, error_text, error_size) >= 0;
	}

	if (input_count >= 2 && cube_count == input_count)
	{
		int cube_index;
		int is_or;

		is_or = 1;

		for (cube_index = 0; cube_index < cube_count; ++cube_index)
		{
			if (!n2b_cube_is_or_term(cube_lines[cube_index], input_count))
			{
				is_or = 0;
				break;
			}
		}

		if (is_or)
		{
			return n2b_ir_add_node(ir, N2B_OP_OR, output_net, input_nets, input_count, 0, error_text, error_size) >= 0;
		}
	}

	if (input_count == 2 && cube_count == 2 &&
		((strcmp(cube_lines[0], "01") == 0 && strcmp(cube_lines[1], "10") == 0) ||
		 (strcmp(cube_lines[0], "10") == 0 && strcmp(cube_lines[1], "01") == 0)))
	{
		return n2b_ir_add_node(ir, N2B_OP_XOR, output_net, input_nets, 2, 0, error_text, error_size) >= 0;
	}

	if (input_count == 2 && cube_count == 2 &&
		((strcmp(cube_lines[0], "00") == 0 && strcmp(cube_lines[1], "11") == 0) ||
		 (strcmp(cube_lines[0], "11") == 0 && strcmp(cube_lines[1], "00") == 0)))
	{
		return n2b_ir_add_node(ir, N2B_OP_XNOR, output_net, input_nets, 2, 0, error_text, error_size) >= 0;
	}

	return n2b_lower_sop(ir, output_net, input_nets, input_count, cube_lines, cube_count, error_text, error_size);
}

int n2b_parse_blif(const char *path, N2B_IR *ir, char *error_text, int error_size)
{
	FILE *stream;
	char line[N2B_BLIF_LINE_SIZE * 2];
	int physical_line_number;
	int model_count;
	int in_model;
	int saw_end;

	stream = fopen(path, "rb");
	if (stream == NULL)
	{
		return n2b_set_error(error_text, error_size, "failed to open BLIF file");
	}

	physical_line_number = 0;
	model_count = 0;
	in_model = 0;
	saw_end = 0;

	while (1)
	{
		char *tokens[N2B_BLIF_MAX_TOKENS];
		char line_copy[N2B_BLIF_LINE_SIZE * 2];
		int logical_line_number;
		int token_count;
		int read_status;

		read_status = n2b_read_logical_line(stream, line, (int)sizeof(line), &physical_line_number, &logical_line_number);
		if (read_status < 0)
		{
			fclose(stream);
			return n2b_set_error(error_text, error_size, "BLIF logical line is too long");
		}

		if (read_status == 0)
		{
			break;
		}

		snprintf(line_copy, sizeof(line_copy), "%s", line);
		token_count = n2b_split_tokens(line_copy, tokens, N2B_BLIF_MAX_TOKENS);
		if (token_count <= 0)
		{
			continue;
		}

		if (strcmp(tokens[0], ".model") == 0 || strcmp(tokens[0], ".end") == 0)
		{
			if (strcmp(tokens[0], ".model") == 0)
			{
				if (in_model || model_count > 0)
				{
					fclose(stream);
					return n2b_set_blif_error(
						"N2B_BLIF_MULTI_MODEL",
						path,
						logical_line_number,
						"multiple .model sections are not supported in phase-1",
						error_text,
						error_size);
				}

				in_model = 1;
				++model_count;
				continue;
			}

			if (!in_model)
			{
				fclose(stream);
				return n2b_set_blif_error(
					"N2B_BLIF_MODEL_STATE",
					path,
					logical_line_number,
					"encountered .end without an active .model",
					error_text,
					error_size);
			}

			in_model = 0;
			saw_end = 1;
			continue;
		}

		if (strcmp(tokens[0], ".inputs") == 0)
		{
			if (!in_model)
			{
				fclose(stream);
				return n2b_set_blif_error(
					"N2B_BLIF_MODEL_STATE",
					path,
					logical_line_number,
					"encountered declarations outside the single supported .model",
					error_text,
					error_size);
			}

			for (int index = 1; index < token_count; ++index)
			{
				if (n2b_ir_declare_input(ir, tokens[index], path, logical_line_number, error_text, error_size) < 0)
				{
					fclose(stream);
					return 0;
				}
			}

			continue;
		}

		if (strcmp(tokens[0], ".outputs") == 0)
		{
			if (!in_model)
			{
				fclose(stream);
				return n2b_set_blif_error(
					"N2B_BLIF_MODEL_STATE",
					path,
					logical_line_number,
					"encountered declarations outside the single supported .model",
					error_text,
					error_size);
			}

			for (int index = 1; index < token_count; ++index)
			{
				if (n2b_ir_declare_output(ir, tokens[index], path, logical_line_number, error_text, error_size) < 0)
				{
					fclose(stream);
					return 0;
				}
			}

			continue;
		}

		if (strcmp(tokens[0], ".names") == 0)
		{
			if (!in_model)
			{
				fclose(stream);
				return n2b_set_blif_error(
					"N2B_BLIF_MODEL_STATE",
					path,
					logical_line_number,
					"encountered logic outside the single supported .model",
					error_text,
					error_size);
			}

			if (!n2b_handle_names(stream, path, ir, tokens, token_count, &physical_line_number, error_text, error_size))
			{
				fclose(stream);
				return 0;
			}

			continue;
		}

		fclose(stream);
		return n2b_set_error(error_text, error_size, "unsupported BLIF directive in phase 1");
	}

	if (model_count == 0)
	{
		fclose(stream);
		return n2b_set_blif_error(
			"N2B_BLIF_MODEL_STATE",
			path,
			1,
			"missing required .model section",
			error_text,
			error_size);
	}

	if (in_model || !saw_end)
	{
		fclose(stream);
		return n2b_set_blif_error(
			"N2B_BLIF_MISSING_END",
			path,
			physical_line_number > 0 ? physical_line_number : 1,
			"missing .end for the single supported .model",
			error_text,
			error_size);
	}

	fclose(stream);
	return 1;
}
