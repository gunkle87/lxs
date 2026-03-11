#include "verilog_parser.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N2B_TOKEN_CAPACITY 4096
#define N2B_TEXT_CAPACITY (1024 * 1024)

typedef struct N2B_Token
{
	char text[128];
	int is_identifier;
	int is_number;
	int line_number;
} N2B_Token;

typedef struct N2B_TokenStream
{
	N2B_Token tokens[N2B_TOKEN_CAPACITY];
	int count;
	int index;
} N2B_TokenStream;

static int n2b_set_error(char *error_text, int error_size, const char *message)
{
	if (error_text != NULL && error_size > 0)
	{
		snprintf(error_text, (size_t)error_size, "%s", message);
	}

	return 0;
}

static int n2b_is_gate_keyword(const char *text, N2B_Op *op)
{
	if (strcmp(text, "buf") == 0)
	{
		*op = N2B_OP_BUF;
		return 1;
	}

	if (strcmp(text, "not") == 0)
	{
		*op = N2B_OP_NOT;
		return 1;
	}

	if (strcmp(text, "and") == 0)
	{
		*op = N2B_OP_AND;
		return 1;
	}

	if (strcmp(text, "nand") == 0)
	{
		*op = N2B_OP_NAND;
		return 1;
	}

	if (strcmp(text, "or") == 0)
	{
		*op = N2B_OP_OR;
		return 1;
	}

	if (strcmp(text, "nor") == 0)
	{
		*op = N2B_OP_NOR;
		return 1;
	}

	if (strcmp(text, "xor") == 0)
	{
		*op = N2B_OP_XOR;
		return 1;
	}

	if (strcmp(text, "xnor") == 0)
	{
		*op = N2B_OP_XNOR;
		return 1;
	}

	return 0;
}

static int n2b_push_token(N2B_TokenStream *stream, const char *text, int is_identifier, int is_number, int line_number)
{
	if (stream->count >= N2B_TOKEN_CAPACITY)
	{
		return 0;
	}

	snprintf(stream->tokens[stream->count].text, sizeof(stream->tokens[stream->count].text), "%s", text);
	stream->tokens[stream->count].is_identifier = is_identifier;
	stream->tokens[stream->count].is_number = is_number;
	stream->tokens[stream->count].line_number = line_number;
	++stream->count;
	return 1;
}

static int n2b_tokenize_verilog(const char *path, N2B_TokenStream *stream, char *error_text, int error_size)
{
	FILE *file;
	char *text;
	size_t length;
	size_t read_count;
	size_t index;
	int line_number;

	file = fopen(path, "rb");
	if (file == NULL)
	{
		return n2b_set_error(error_text, error_size, "failed to open Verilog file");
	}

	text = (char *)malloc(N2B_TEXT_CAPACITY);
	if (text == NULL)
	{
		fclose(file);
		return n2b_set_error(error_text, error_size, "out of memory while reading Verilog file");
	}

	read_count = fread(text, 1U, N2B_TEXT_CAPACITY - 1U, file);
	fclose(file);
	text[read_count] = '\0';
	length = read_count;
	line_number = 1;

	for (index = 0U; index < length; )
	{
		if (isspace((unsigned char)text[index]))
		{
			if (text[index] == '\n')
			{
				++line_number;
			}

			++index;
			continue;
		}

		if (text[index] == '/' && index + 1U < length && text[index + 1U] == '/')
		{
			index += 2U;
			while (index < length && text[index] != '\n')
			{
				++index;
			}

			continue;
		}

		if (text[index] == '/' && index + 1U < length && text[index + 1U] == '*')
		{
			index += 2U;
			while (index + 1U < length && !(text[index] == '*' && text[index + 1U] == '/'))
			{
				if (text[index] == '\n')
				{
					++line_number;
				}

				++index;
			}

			index += 2U;
			continue;
		}

		if (text[index] == '\\')
		{
			free(text);
			return n2b_set_error(error_text, error_size, "escaped Verilog identifiers are unsupported in phase 1");
		}

		if (isalpha((unsigned char)text[index]) || text[index] == '_' || text[index] == '$')
		{
			char token[128];
			size_t out_index;

			out_index = 0U;
			while (index < length &&
				(isalnum((unsigned char)text[index]) || text[index] == '_' || text[index] == '$'))
			{
				if (out_index + 1U >= sizeof(token))
				{
					free(text);
					return n2b_set_error(error_text, error_size, "Verilog token is too long");
				}

				token[out_index++] = text[index++];
			}

			token[out_index] = '\0';
			if (!n2b_push_token(stream, token, 1, 0, line_number))
			{
				free(text);
				return n2b_set_error(error_text, error_size, "too many Verilog tokens");
			}

			continue;
		}

		if (isdigit((unsigned char)text[index]))
		{
			char token[128];
			size_t out_index;

			out_index = 0U;
			while (index < length && isdigit((unsigned char)text[index]))
			{
				if (out_index + 1U >= sizeof(token))
				{
					free(text);
					return n2b_set_error(error_text, error_size, "Verilog numeric token is too long");
				}

				token[out_index++] = text[index++];
			}

			token[out_index] = '\0';
			if (!n2b_push_token(stream, token, 0, 1, line_number))
			{
				free(text);
				return n2b_set_error(error_text, error_size, "too many Verilog tokens");
			}

			continue;
		}

		{
			char token[2];

			token[0] = text[index++];
			token[1] = '\0';
			if (!n2b_push_token(stream, token, 0, 0, line_number))
			{
				free(text);
				return n2b_set_error(error_text, error_size, "too many Verilog tokens");
			}
		}
	}

	free(text);
	return 1;
}

static N2B_Token *n2b_peek(N2B_TokenStream *stream)
{
	if (stream->index >= stream->count)
	{
		return NULL;
	}

	return &stream->tokens[stream->index];
}

static N2B_Token *n2b_next(N2B_TokenStream *stream)
{
	N2B_Token *token;

	token = n2b_peek(stream);
	if (token != NULL)
	{
		++stream->index;
	}

	return token;
}

static int n2b_expect_symbol(N2B_TokenStream *stream, const char *symbol, char *error_text, int error_size)
{
	N2B_Token *token;

	token = n2b_next(stream);
	if (token == NULL || strcmp(token->text, symbol) != 0)
	{
		return n2b_set_error(error_text, error_size, "unexpected Verilog token");
	}

	return 1;
}

static int n2b_parse_integer(N2B_TokenStream *stream, int *value, char *error_text, int error_size)
{
	N2B_Token *token;

	token = n2b_next(stream);
	if (token == NULL || !token->is_number)
	{
		return n2b_set_error(error_text, error_size, "expected integer");
	}

	*value = atoi(token->text);
	return 1;
}

static int n2b_parse_range(N2B_TokenStream *stream, int *has_range, int *left, int *right, char *error_text, int error_size)
{
	N2B_Token *token;

	token = n2b_peek(stream);
	if (token == NULL || strcmp(token->text, "[") != 0)
	{
		*has_range = 0;
		return 1;
	}

	*has_range = 1;

	if (!n2b_expect_symbol(stream, "[", error_text, error_size) ||
		!n2b_parse_integer(stream, left, error_text, error_size) ||
		!n2b_expect_symbol(stream, ":", error_text, error_size) ||
		!n2b_parse_integer(stream, right, error_text, error_size) ||
		!n2b_expect_symbol(stream, "]", error_text, error_size))
	{
		return 0;
	}

	return 1;
}

static int n2b_parse_net_ref(N2B_TokenStream *stream, char *out_name, int out_size, char *error_text, int error_size)
{
	N2B_Token *token;

	token = n2b_next(stream);
	if (token == NULL || !token->is_identifier)
	{
		return n2b_set_error(error_text, error_size, "expected net identifier");
	}

	snprintf(out_name, (size_t)out_size, "%s", token->text);

	token = n2b_peek(stream);
	if (token != NULL && strcmp(token->text, "[") == 0)
	{
		int index_value;
		char base_name[160];

		if (!n2b_expect_symbol(stream, "[", error_text, error_size) ||
			!n2b_parse_integer(stream, &index_value, error_text, error_size) ||
			!n2b_expect_symbol(stream, "]", error_text, error_size))
		{
			return 0;
		}

		snprintf(base_name, sizeof(base_name), "%s", out_name);
		if (strlen(base_name) + 16U >= (size_t)out_size)
		{
			return n2b_set_error(error_text, error_size, "indexed net name is too long");
		}

		snprintf(out_name, (size_t)out_size, "%s[%d]", base_name, index_value);
	}

	return 1;
}

static int n2b_add_declared_name(N2B_IR *ir, const char *path, int line_number, const char *keyword, const char *base_name, int has_range, int left, int right, char *error_text, int error_size)
{
	int bit;
	int step;

	if (!has_range)
	{
		if (strcmp(keyword, "input") == 0)
		{
			return n2b_ir_declare_input(ir, base_name, path, line_number, error_text, error_size) >= 0;
		}

		if (strcmp(keyword, "output") == 0)
		{
			return n2b_ir_declare_output(ir, base_name, path, line_number, error_text, error_size) >= 0;
		}

		return n2b_ir_declare_wire(ir, base_name, path, line_number, error_text, error_size) >= 0;
	}

	step = (left >= right) ? -1 : 1;
	for (bit = left; ; bit += step)
	{
		char indexed_name[160];

		snprintf(indexed_name, sizeof(indexed_name), "%s[%d]", base_name, bit);
		if (strcmp(keyword, "input") == 0)
		{
			if (n2b_ir_declare_input(ir, indexed_name, path, line_number, error_text, error_size) < 0)
			{
				return 0;
			}
		}
		else if (strcmp(keyword, "output") == 0)
		{
			if (n2b_ir_declare_output(ir, indexed_name, path, line_number, error_text, error_size) < 0)
			{
				return 0;
			}
		}
		else if (n2b_ir_declare_wire(ir, indexed_name, path, line_number, error_text, error_size) < 0)
		{
			return 0;
		}

		if (bit == right)
		{
			break;
		}
	}

	return 1;
}

static int n2b_parse_declaration(N2B_TokenStream *stream, const char *path, N2B_IR *ir, const char *keyword, int line_number, char *error_text, int error_size)
{
	int has_range;
	int left;
	int right;

	left = 0;
	right = 0;

	if (!n2b_parse_range(stream, &has_range, &left, &right, error_text, error_size))
	{
		return 0;
	}

	while (1)
	{
		N2B_Token *token;

		token = n2b_next(stream);
		if (token == NULL || !token->is_identifier)
		{
			return n2b_set_error(error_text, error_size, "expected identifier in declaration");
		}

		if (!n2b_add_declared_name(ir, path, line_number, keyword, token->text, has_range, left, right, error_text, error_size))
		{
			return 0;
		}

		token = n2b_next(stream);
		if (token == NULL)
		{
			return n2b_set_error(error_text, error_size, "unexpected end of declaration");
		}

		if (strcmp(token->text, ";") == 0)
		{
			return 1;
		}

		if (strcmp(token->text, ",") != 0)
		{
			return n2b_set_error(error_text, error_size, "expected ',' or ';' in declaration");
		}
	}
}

static int n2b_parse_gate_instance(N2B_TokenStream *stream, N2B_IR *ir, N2B_Op op, char *error_text, int error_size)
{
	char net_name[160];
	int nets[128];
	int net_count;
	N2B_Token *token;

	net_count = 0;
	token = n2b_peek(stream);

	if (token != NULL && token->is_identifier && stream->index + 1 < stream->count &&
		strcmp(stream->tokens[stream->index + 1].text, "(") == 0)
	{
		n2b_next(stream);
	}

	if (!n2b_expect_symbol(stream, "(", error_text, error_size))
	{
		return 0;
	}

	while (1)
	{
		if (!n2b_parse_net_ref(stream, net_name, (int)sizeof(net_name), error_text, error_size))
		{
			return 0;
		}

		nets[net_count] = n2b_ir_intern_net(ir, net_name, error_text, error_size);
		if (nets[net_count] < 0)
		{
			return 0;
		}

		++net_count;
		token = n2b_next(stream);
		if (token == NULL)
		{
			return n2b_set_error(error_text, error_size, "unexpected end of gate instance");
		}

		if (strcmp(token->text, ")") == 0)
		{
			break;
		}

		if (strcmp(token->text, ",") != 0)
		{
			return n2b_set_error(error_text, error_size, "expected ',' or ')' in gate instance");
		}
	}

	if (!n2b_expect_symbol(stream, ";", error_text, error_size))
	{
		return 0;
	}

	if (net_count < 2)
	{
		return n2b_set_error(error_text, error_size, "gate instance must include output and at least one input");
	}

	if ((op == N2B_OP_XOR || op == N2B_OP_XNOR) && net_count != 3)
	{
		return n2b_set_error(error_text, error_size, "phase 1 supports only 2-input xor/xnor");
	}

	return n2b_ir_add_node(ir, op, nets[0], &nets[1], net_count - 1, 0, error_text, error_size) >= 0;
}

static int n2b_parse_assign(N2B_TokenStream *stream, N2B_IR *ir, char *error_text, int error_size)
{
	char left_name[160];
	char right_name[160];
	int output_net;
	int input_net;
	int input_nets[1];
	N2B_Op op;
	N2B_Token *token;

	if (!n2b_parse_net_ref(stream, left_name, (int)sizeof(left_name), error_text, error_size) ||
		!n2b_expect_symbol(stream, "=", error_text, error_size))
	{
		return 0;
	}

	op = N2B_OP_BUF;
	token = n2b_peek(stream);
	if (token != NULL && strcmp(token->text, "~") == 0)
	{
		n2b_next(stream);
		op = N2B_OP_NOT;
	}

	if (!n2b_parse_net_ref(stream, right_name, (int)sizeof(right_name), error_text, error_size) ||
		!n2b_expect_symbol(stream, ";", error_text, error_size))
	{
		return 0;
	}

	output_net = n2b_ir_intern_net(ir, left_name, error_text, error_size);
	input_net = n2b_ir_intern_net(ir, right_name, error_text, error_size);
	if (output_net < 0 || input_net < 0)
	{
		return 0;
	}

	input_nets[0] = input_net;
	return n2b_ir_add_node(ir, op, output_net, input_nets, 1, 0, error_text, error_size) >= 0;
}

int n2b_parse_verilog(const char *path, N2B_IR *ir, char *error_text, int error_size)
{
	N2B_TokenStream stream;
	int module_seen;

	memset(&stream, 0, sizeof(stream));
	if (!n2b_tokenize_verilog(path, &stream, error_text, error_size))
	{
		return 0;
	}

	module_seen = 0;
	while (n2b_peek(&stream) != NULL)
	{
		N2B_Token *token;
		N2B_Op op;

		token = n2b_next(&stream);
		if (strcmp(token->text, "module") == 0)
		{
			if (module_seen)
			{
				return n2b_set_error(error_text, error_size, "phase 1 supports only one Verilog module");
			}

			module_seen = 1;
			if (n2b_next(&stream) == NULL)
			{
				return n2b_set_error(error_text, error_size, "expected module name");
			}

			while ((token = n2b_next(&stream)) != NULL && strcmp(token->text, ";") != 0)
			{
			}

			if (token == NULL)
			{
				return n2b_set_error(error_text, error_size, "unterminated module header");
			}

			continue;
		}

		if (strcmp(token->text, "endmodule") == 0)
		{
			break;
		}

		if (strcmp(token->text, "input") == 0 || strcmp(token->text, "output") == 0 || strcmp(token->text, "wire") == 0)
		{
			if (!n2b_parse_declaration(&stream, path, ir, token->text, token->line_number, error_text, error_size))
			{
				return 0;
			}

			continue;
		}

		if (strcmp(token->text, "assign") == 0)
		{
			if (!n2b_parse_assign(&stream, ir, error_text, error_size))
			{
				return 0;
			}

			continue;
		}

		if (n2b_is_gate_keyword(token->text, &op))
		{
			if (!n2b_parse_gate_instance(&stream, ir, op, error_text, error_size))
			{
				return 0;
			}

			continue;
		}

		return n2b_set_error(error_text, error_size, "unsupported Verilog construct in phase 1");
	}

	if (!module_seen)
	{
		return n2b_set_error(error_text, error_size, "missing Verilog module");
	}

	return 1;
}
