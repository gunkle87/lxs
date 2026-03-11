#include "lxs_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LXS_INITIAL_CAP 256U

static char* lxs_strdup_local(const char *text)
	{
	size_t length = strlen(text) + 1U;
	char *copy = (char*)malloc(length);

	if (!copy)
		{
		return NULL;
		}

	memcpy(copy, text, length);
	return copy;
	}

static char* lxs_trim(char *text)
	{
	char *end;

	while (*text == ' ' || *text == '\t' || *text == '\r' || *text == '\n')
		{
		text++;
		}

	end = text + strlen(text);
	while (end > text)
		{
		char c = end[-1];
		if (c != ' ' && c != '\t' && c != '\r' && c != '\n')
			{
			break;
			}
		end--;
		}

	*end = '\0';
	return text;
	}

static int lxs_reserve_names(char ***items, uint32_t *cap, uint32_t needed)
	{
	char **new_items;

	if (needed <= *cap)
		{
		return 1;
		}

	while (*cap < needed)
		{
		*cap = (*cap == 0U) ? LXS_INITIAL_CAP : (*cap * 2U);
		}

	new_items = (char**)realloc(*items, (size_t)(*cap) * sizeof(char*));
	if (!new_items)
		{
		return 0;
		}

	*items = new_items;
	return 1;
	}

static int lxs_reserve_u32(uint32_t **items, uint32_t *cap, uint32_t needed)
	{
	uint32_t *new_items;

	if (needed <= *cap)
		{
		return 1;
		}

	while (*cap < needed)
		{
		*cap = (*cap == 0U) ? LXS_INITIAL_CAP : (*cap * 2U);
		}

	new_items = (uint32_t*)realloc(*items, (size_t)(*cap) * sizeof(uint32_t));
	if (!new_items)
		{
		return 0;
		}

	*items = new_items;
	return 1;
	}

static int lxs_reserve_gates(lxs_gate_ir **items, uint32_t *cap, uint32_t needed)
	{
	lxs_gate_ir *new_items;

	if (needed <= *cap)
		{
		return 1;
		}

	while (*cap < needed)
		{
		*cap = (*cap == 0U) ? LXS_INITIAL_CAP : (*cap * 2U);
		}

	new_items = (lxs_gate_ir*)realloc(*items, (size_t)(*cap) * sizeof(lxs_gate_ir));
	if (!new_items)
		{
		return 0;
		}

	*items = new_items;
	return 1;
	}

static int lxs_reserve_u32_init_max(uint32_t **items, uint32_t *cap, uint32_t needed)
	{
	uint32_t old_cap = *cap;

	if (!lxs_reserve_u32(items, cap, needed))
		{
		return 0;
		}

	for (uint32_t i = old_cap; i < *cap; ++i)
		{
		(*items)[i] = UINT32_MAX;
		}

	return 1;
	}

static int lxs_push_u32(uint32_t **items, uint32_t *count, uint32_t *cap, uint32_t value)
	{
	if (!lxs_reserve_u32(items, cap, *count + 1U))
		{
		return 0;
		}

	(*items)[(*count)++] = value;
	return 1;
	}

static int lxs_push_gate(lxs_netlist *nl, const lxs_gate_ir *gate)
	{
	if (!lxs_reserve_gates(&nl->gates, &nl->gate_cap, nl->gate_count + 1U))
		{
		return 0;
		}

	nl->gates[nl->gate_count++] = *gate;
	return 1;
	}

static int lxs_push_line(char ***items, uint32_t *count, uint32_t *cap, const char *value)
	{
	if (!lxs_reserve_names(items, cap, *count + 1U))
		{
		return 0;
		}

	(*items)[*count] = lxs_strdup_local(value);
	if (!(*items)[*count])
		{
		return 0;
		}

	(*count)++;
	return 1;
	}

static void lxs_free_lines(char **lines, uint32_t count)
	{
	for (uint32_t i = 0; i < count; ++i)
		{
		free(lines[i]);
		}

	free(lines);
	}

static int lxs_load_logical_lines(FILE *stream, char ***out_lines, uint32_t *out_count)
	{
	char raw[4096];
	char **lines = NULL;
	uint32_t line_count = 0U;
	uint32_t line_cap = 0U;
	char *buffer = NULL;
	size_t buffer_len = 0U;
	size_t buffer_cap = 0U;

	while (fgets(raw, sizeof(raw), stream))
		{
		char *trimmed = lxs_trim(raw);
		size_t segment_len = strlen(trimmed);
		int continued = segment_len > 0U && trimmed[segment_len - 1U] == '\\';

		if (continued)
			{
			trimmed[--segment_len] = '\0';
			}

		if (buffer_len + segment_len + 2U > buffer_cap)
			{
			size_t new_cap = buffer_cap == 0U ? 256U : buffer_cap;
			char *new_buffer;

			while (new_cap < buffer_len + segment_len + 2U)
				{
				new_cap *= 2U;
				}

			new_buffer = (char*)realloc(buffer, new_cap);
			if (!new_buffer)
				{
				free(buffer);
				lxs_free_lines(lines, line_count);
				return 0;
				}

			buffer = new_buffer;
			buffer_cap = new_cap;
			}

		memcpy(buffer + buffer_len, trimmed, segment_len);
		buffer_len += segment_len;
		buffer[buffer_len] = '\0';

		if (continued)
			{
			buffer[buffer_len++] = ' ';
			buffer[buffer_len] = '\0';
			continue;
			}

		if (!lxs_push_line(&lines, &line_count, &line_cap, lxs_trim(buffer)))
			{
			free(buffer);
			lxs_free_lines(lines, line_count);
			return 0;
			}

		buffer_len = 0U;
		if (buffer)
			{
			buffer[0] = '\0';
			}
		}

	if (buffer_len > 0U)
		{
		if (!lxs_push_line(&lines, &line_count, &line_cap, lxs_trim(buffer)))
			{
			free(buffer);
			lxs_free_lines(lines, line_count);
			return 0;
			}
		}

	free(buffer);
	*out_lines = lines;
	*out_count = line_count;
	return 1;
	}

static uint32_t lxs_find_net(const lxs_netlist *nl, const char *name)
	{
	for (uint32_t i = 0; i < nl->net_count; ++i)
		{
		if (strcmp(nl->net_names[i], name) == 0)
			{
			return i;
			}
		}

	return UINT32_MAX;
	}

static uint32_t lxs_intern_net(lxs_netlist *nl, const char *name)
	{
	uint32_t id = lxs_find_net(nl, name);

	if (id != UINT32_MAX)
		{
		return id;
		}

	if (!lxs_reserve_names(&nl->net_names, &nl->net_cap, nl->net_count + 1U))
		{
		return UINT32_MAX;
		}

	id = nl->net_count++;
	nl->net_names[id] = lxs_strdup_local(name);
	return nl->net_names[id] ? id : UINT32_MAX;
	}

static int lxs_emit_unary_gate(
	lxs_netlist *nl,
	uint32_t type,
	uint32_t input,
	uint32_t output)
	{
	lxs_gate_ir gate;

	memset(&gate, 0, sizeof(gate));
	gate.type = type;
	gate.input_count = 1U;
	gate.inputs[0] = input;
	gate.output = output;
	return lxs_push_gate(nl, &gate);
	}

static int lxs_emit_binary_gate(
	lxs_netlist *nl,
	uint32_t type,
	uint32_t input0,
	uint32_t input1,
	uint32_t output)
	{
	lxs_gate_ir gate;

	memset(&gate, 0, sizeof(gate));
	gate.type = type;
	gate.input_count = 2U;
	gate.inputs[0] = input0;
	gate.inputs[1] = input1;
	gate.output = output;
	return lxs_push_gate(nl, &gate);
	}

static uint32_t lxs_blif_const0(lxs_netlist *nl)
	{
	return lxs_intern_net(nl, "__lxs_const0");
	}

static uint32_t lxs_blif_const1(lxs_netlist *nl, uint32_t *const1_id)
	{
	uint32_t const0_id;

	if (*const1_id != UINT32_MAX)
		{
		return *const1_id;
		}

	const0_id = lxs_blif_const0(nl);
	if (const0_id == UINT32_MAX)
		{
		return UINT32_MAX;
		}

	*const1_id = lxs_intern_net(nl, "__lxs_const1");
	if (*const1_id == UINT32_MAX)
		{
		return UINT32_MAX;
		}

	if (!lxs_emit_binary_gate(nl, LXS_GATE_NOR, const0_id, const0_id, *const1_id))
		{
		return UINT32_MAX;
		}

	return *const1_id;
	}

static uint32_t lxs_blif_inverted_net(
	lxs_netlist *nl,
	uint32_t net_id,
	uint32_t **invert_cache,
	uint32_t *invert_cap)
	{
	char name[64];

	if (!lxs_reserve_u32_init_max(invert_cache, invert_cap, net_id + 1U))
		{
		return UINT32_MAX;
		}

	if ((*invert_cache)[net_id] != UINT32_MAX)
		{
		return (*invert_cache)[net_id];
		}

	snprintf(name, sizeof(name), "__lxs_not_%u", net_id);
	(*invert_cache)[net_id] = lxs_intern_net(nl, name);
	if ((*invert_cache)[net_id] == UINT32_MAX)
		{
		return UINT32_MAX;
		}

	if (!lxs_emit_unary_gate(nl, LXS_GATE_NOT, net_id, (*invert_cache)[net_id]))
		{
		return UINT32_MAX;
		}

	return (*invert_cache)[net_id];
	}

static int lxs_blif_match_cube(const char *pattern, uint32_t combo, uint32_t input_count)
	{
	for (uint32_t i = 0; i < input_count; ++i)
		{
		char bit = pattern[i];
		uint32_t combo_bit = (combo >> (input_count - 1U - i)) & 1U;

		if (bit == '-')
			{
			continue;
			}

		if ((bit == '0' && combo_bit != 0U) || (bit == '1' && combo_bit == 0U))
			{
			return 0;
			}
		}

	return 1;
	}

static int lxs_blif_truth_table(
	uint32_t input_count,
	char **covers,
	uint32_t cover_count,
	uint8_t *truth_bits)
	{
	uint32_t width = 1U << input_count;
	int default_value = 0;

	if (cover_count > 0U)
		{
		char *first = lxs_trim(covers[0]);
		size_t len = strlen(first);
		char out_bit = len > 0U ? first[len - 1U] : '0';

		if (out_bit != '0' && out_bit != '1')
			{
			out_bit = '1';
			}

		default_value = (out_bit == '1') ? 0 : 1;
		}

	for (uint32_t combo = 0; combo < width; ++combo)
		{
		truth_bits[combo] = (uint8_t)default_value;
		}

	for (uint32_t i = 0; i < cover_count; ++i)
		{
		char *cover = lxs_trim(covers[i]);
		char pattern[8];
		char out_text[8];
		int matched;
		int row_value = 1;

		pattern[0] = '\0';
		out_text[0] = '\0';
		matched = sscanf(cover, "%7s %7s", pattern, out_text);
		if (input_count == 0U)
			{
			if (matched == 1)
				{
				row_value = (pattern[0] == '0') ? 0 : 1;
				truth_bits[0] = (uint8_t)row_value;
				}
			continue;
			}

		if (matched <= 0 || strlen(pattern) != input_count)
			{
			return 0;
			}

		if (matched >= 2 && (out_text[0] == '0' || out_text[0] == '1'))
			{
			row_value = (out_text[0] == '1') ? 1 : 0;
			}

		for (uint32_t combo = 0; combo < width; ++combo)
			{
			if (lxs_blif_match_cube(pattern, combo, input_count))
				{
				truth_bits[combo] = (uint8_t)row_value;
				}
			}
		}

	return 1;
	}

static int lxs_blif_emit_truth(
	lxs_netlist *nl,
	uint32_t output,
	uint32_t *inputs,
	uint32_t input_count,
	const uint8_t *truth_bits,
	uint32_t **invert_cache,
	uint32_t *invert_cap,
	uint32_t *const1_id)
	{
	uint32_t truth = 0U;

	for (uint32_t i = 0; i < (1U << input_count); ++i)
		{
		if (truth_bits[i])
			{
			truth |= (1U << i);
			}
		}

	if (input_count == 0U)
		{
		if (truth == 0U)
			{
			return 1;
			}

		if (truth == 1U)
			{
			uint32_t const1 = lxs_blif_const1(nl, const1_id);
			return const1 != UINT32_MAX && lxs_emit_unary_gate(nl, LXS_GATE_BUF, const1, output);
			}

		return 0;
		}

	if (input_count == 1U)
		{
		if (truth == 0x1U)
			{
			return lxs_emit_unary_gate(nl, LXS_GATE_NOT, inputs[0], output);
			}
		if (truth == 0x2U)
			{
			return lxs_emit_unary_gate(nl, LXS_GATE_BUF, inputs[0], output);
			}
		if (truth == 0x0U)
			{
			return 1;
			}
		if (truth == 0x3U)
			{
			uint32_t const1 = lxs_blif_const1(nl, const1_id);
			return const1 != UINT32_MAX && lxs_emit_unary_gate(nl, LXS_GATE_BUF, const1, output);
			}
		return 0;
		}

	if (input_count != 2U)
		{
		return 0;
		}

	switch (truth)
		{
		case 0x0U:
			return 1;
		case 0x1U:
			return lxs_emit_binary_gate(nl, LXS_GATE_NOR, inputs[0], inputs[1], output);
		case 0x2U:
			{
			uint32_t inv0 = lxs_blif_inverted_net(nl, inputs[0], invert_cache, invert_cap);
			return inv0 != UINT32_MAX && lxs_emit_binary_gate(nl, LXS_GATE_AND, inv0, inputs[1], output);
			}
		case 0x3U:
			return lxs_emit_unary_gate(nl, LXS_GATE_NOT, inputs[0], output);
		case 0x4U:
			{
			uint32_t inv1 = lxs_blif_inverted_net(nl, inputs[1], invert_cache, invert_cap);
			return inv1 != UINT32_MAX && lxs_emit_binary_gate(nl, LXS_GATE_AND, inputs[0], inv1, output);
			}
		case 0x5U:
			return lxs_emit_unary_gate(nl, LXS_GATE_NOT, inputs[1], output);
		case 0x6U:
			return lxs_emit_binary_gate(nl, LXS_GATE_XOR, inputs[0], inputs[1], output);
		case 0x7U:
			return lxs_emit_binary_gate(nl, LXS_GATE_NAND, inputs[0], inputs[1], output);
		case 0x8U:
			return lxs_emit_binary_gate(nl, LXS_GATE_AND, inputs[0], inputs[1], output);
		case 0x9U:
			return lxs_emit_binary_gate(nl, LXS_GATE_XNOR, inputs[0], inputs[1], output);
		case 0xAU:
			return lxs_emit_unary_gate(nl, LXS_GATE_BUF, inputs[1], output);
		case 0xCU:
			return lxs_emit_unary_gate(nl, LXS_GATE_BUF, inputs[0], output);
		case 0xEU:
			return lxs_emit_binary_gate(nl, LXS_GATE_OR, inputs[0], inputs[1], output);
		case 0xFU:
			{
			uint32_t const1 = lxs_blif_const1(nl, const1_id);
			return const1 != UINT32_MAX && lxs_emit_unary_gate(nl, LXS_GATE_BUF, const1, output);
			}
		default:
			return 0;
		}
	}

static lxs_netlist* lxs_load_blif(const char *path)
	{
	FILE *stream;
	char **lines = NULL;
	uint32_t line_count = 0U;
	lxs_netlist *nl = NULL;
	uint32_t *invert_cache = NULL;
	uint32_t invert_cap = 0U;
	uint32_t const1_id = UINT32_MAX;

	stream = fopen(path, "r");
	if (!stream)
		{
		return NULL;
		}

	if (!lxs_load_logical_lines(stream, &lines, &line_count))
		{
		fclose(stream);
		return NULL;
		}

	fclose(stream);
	nl = (lxs_netlist*)calloc(1U, sizeof(lxs_netlist));
	if (!nl)
		{
		lxs_free_lines(lines, line_count);
		return NULL;
		}

	for (uint32_t i = 0; i < line_count; ++i)
		{
		char *trimmed = lxs_trim(lines[i]);

		if (trimmed[0] == '\0' || trimmed[0] == '#')
			{
			continue;
			}

		if (strncmp(trimmed, ".model", 6) == 0 || strncmp(trimmed, ".end", 4) == 0)
			{
			continue;
			}

		if (strncmp(trimmed, ".inputs", 7) == 0 || strncmp(trimmed, ".outputs", 8) == 0)
			{
			char *ctx = NULL;
			char *token = strtok_s(trimmed, " \t", &ctx);
			int is_input = token && strcmp(token, ".inputs") == 0;

			token = strtok_s(NULL, " \t", &ctx);
			while (token)
				{
				uint32_t id = lxs_intern_net(nl, token);

				if (id == UINT32_MAX)
					{
					goto fail;
					}

				if (is_input)
					{
					if (!lxs_push_u32(&nl->inputs, &nl->input_count, &nl->input_cap, id))
						{
						goto fail;
						}
					}
				else
					{
					if (!lxs_push_u32(&nl->outputs, &nl->output_count, &nl->output_cap, id))
						{
						goto fail;
						}
					}

				token = strtok_s(NULL, " \t", &ctx);
				}
			continue;
			}

		if (strncmp(trimmed, ".names", 6) == 0)
			{
			char *ctx = NULL;
			char *token = strtok_s(trimmed, " \t", &ctx);
			char *tokens[4];
			uint32_t token_count = 0U;
			uint32_t input_ids[2];
			char *covers[16];
			uint32_t cover_count = 0U;
			uint32_t cover_end = i;
			uint8_t truth[4];
			uint32_t output_id;

			token = strtok_s(NULL, " \t", &ctx);
			while (token && token_count < 4U)
				{
				tokens[token_count++] = token;
				token = strtok_s(NULL, " \t", &ctx);
				}

			if (token_count == 0U || token_count > 3U)
				{
				goto fail;
				}

			for (uint32_t j = i + 1U; j < line_count; ++j)
				{
				char *cover = lxs_trim(lines[j]);
				if (cover[0] == '\0' || cover[0] == '#')
					{
					continue;
					}
				if (cover[0] == '.')
					{
					break;
					}
				if (cover_count < 16U)
					{
					covers[cover_count++] = cover;
					}
				cover_end = j;
				}

			output_id = lxs_intern_net(nl, tokens[token_count - 1U]);
			if (output_id == UINT32_MAX)
				{
				goto fail;
				}

			for (uint32_t j = 0; j + 1U < token_count; ++j)
				{
				input_ids[j] = lxs_intern_net(nl, tokens[j]);
				if (input_ids[j] == UINT32_MAX)
					{
					goto fail;
					}
				}

			if (!lxs_blif_truth_table(token_count - 1U, covers, cover_count, truth) ||
				!lxs_blif_emit_truth(nl, output_id, input_ids, token_count - 1U, truth, &invert_cache, &invert_cap, &const1_id))
				{
				goto fail;
				}

			i = cover_end;
			}
		}

	lxs_free_lines(lines, line_count);
	free(invert_cache);
	return nl;

fail:
	lxs_free_lines(lines, line_count);
	free(invert_cache);
	if (nl)
		{
		for (uint32_t i = 0; i < nl->net_count; ++i)
			{
			free(nl->net_names[i]);
			}
		free(nl->net_names);
		free(nl->inputs);
		free(nl->outputs);
		free(nl->gates);
		free(nl);
		}
	return NULL;
	}

static const char* lxs_gate_name(uint32_t type)
	{
	switch (type)
		{
		case LXS_GATE_AND: return "AND";
		case LXS_GATE_OR: return "OR";
		case LXS_GATE_XOR: return "XOR";
		case LXS_GATE_TRI: return "TRI";
		case LXS_GATE_NOT: return "NOT";
		case LXS_GATE_NAND: return "NAND";
		case LXS_GATE_NOR: return "NOR";
		case LXS_GATE_XNOR: return "XNOR";
		case LXS_GATE_BUF: return "BUF";
		case LXS_GATE_DFF: return "DFF";
		default: return "BUF";
		}
	}

static int lxs_write_bench(const lxs_netlist *nl, const char *path)
	{
	FILE *stream = fopen(path, "w");

	if (!stream)
		{
		return 0;
		}

	for (uint32_t i = 0; i < nl->input_count; ++i)
		{
		fprintf(stream, "INPUT(%s)\n", nl->net_names[nl->inputs[i]]);
		}
	fprintf(stream, "\n");
	for (uint32_t i = 0; i < nl->output_count; ++i)
		{
		fprintf(stream, "OUTPUT(%s)\n", nl->net_names[nl->outputs[i]]);
		}
	fprintf(stream, "\n");
	for (uint32_t i = 0; i < nl->gate_count; ++i)
		{
		const lxs_gate_ir *gate = &nl->gates[i];
		fprintf(stream, "%s = %s(%s",
			nl->net_names[gate->output],
			lxs_gate_name(gate->type),
			nl->net_names[gate->inputs[0]]);
		if (gate->input_count > 1U)
			{
			fprintf(stream, ", %s", nl->net_names[gate->inputs[1]]);
			}
		fprintf(stream, ")\n");
		}

	fclose(stream);
	return 1;
	}

static void lxs_free_netlist_local(lxs_netlist *nl)
	{
	if (!nl)
		{
		return;
		}

	for (uint32_t i = 0; i < nl->net_count; ++i)
		{
		free(nl->net_names[i]);
		}
	free(nl->net_names);
	free(nl->inputs);
	free(nl->outputs);
	free(nl->gates);
	free(nl);
	}

int main(int argc, char **argv)
	{
	lxs_netlist *nl;

	if (argc != 3)
		{
		fprintf(stderr, "usage: lxs_blif2bench <input.blif> <output.bench>\n");
		return 1;
		}

	nl = lxs_load_blif(argv[1]);
	if (!nl)
		{
		fprintf(stderr, "conversion failed: %s\n", argv[1]);
		return 1;
		}

	if (!lxs_write_bench(nl, argv[2]))
		{
		fprintf(stderr, "write failed: %s\n", argv[2]);
		lxs_free_netlist_local(nl);
		return 1;
		}

	lxs_free_netlist_local(nl);
	return 0;
	}
