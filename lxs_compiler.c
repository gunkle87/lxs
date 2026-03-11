#include "lxs_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#define LXS_ALIGN_SIZE 64U
#define LXS_INITIAL_CAP 256U

static void* lxs_malloc_aligned(size_t size)
	{
	if (size == 0U)
		{
		return NULL;
		}

#ifdef _WIN32
	return _aligned_malloc(size, LXS_ALIGN_SIZE);
#else
	void *ptr = NULL;
	if (posix_memalign(&ptr, LXS_ALIGN_SIZE, size) != 0)
		{
		return NULL;
		}
	return ptr;
#endif
	}

static void* lxs_calloc_aligned(size_t count, size_t size)
	{
	void *ptr;
	size_t total_size = count * size;

	ptr = lxs_malloc_aligned(total_size);
	if (ptr)
		{
		memset(ptr, 0, total_size);
		}
	return ptr;
	}

static void lxs_free_aligned(void *ptr)
	{
	if (!ptr)
		{
		return;
		}

#ifdef _WIN32
	_aligned_free(ptr);
#else
	free(ptr);
#endif
	}

static void* lxs_realloc_aligned(void *ptr, size_t old_size, size_t new_size)
	{
	void *new_ptr;

	new_ptr = lxs_malloc_aligned(new_size);
	if (!new_ptr)
		{
		return NULL;
		}

	memset(new_ptr, 0, new_size);
	if (ptr && old_size > 0U)
		{
		size_t copy_size = old_size < new_size ? old_size : new_size;
		memcpy(new_ptr, ptr, copy_size);
		lxs_free_aligned(ptr);
		}
	return new_ptr;
	}

static char* lxs_strdup_local(const char *text)
	{
	size_t length = strlen(text) + 1U;
	char *copy = malloc(length);
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

static int lxs_has_suffix_ci(const char *path, const char *suffix)
	{
	size_t path_len = strlen(path);
	size_t suffix_len = strlen(suffix);

	if (path_len < suffix_len)
		{
		return 0;
		}

#ifdef _WIN32
	return _stricmp(path + path_len - suffix_len, suffix) == 0;
#else
	return strcasecmp(path + path_len - suffix_len, suffix) == 0;
#endif
	}

static int lxs_reserve_names(char ***items, uint32_t *cap, uint32_t needed)
	{
	size_t old_size;

	if (needed <= *cap)
		{
		return 1;
		}

	old_size = (size_t)(*cap) * sizeof(char*);
	while (*cap < needed)
		{
		*cap = (*cap == 0U) ? LXS_INITIAL_CAP : (*cap * 2U);
		}

	*items = lxs_realloc_aligned(*items, old_size, (size_t)(*cap) * sizeof(char*));
	return *items != NULL;
	}

static int lxs_reserve_u32(uint32_t **items, uint32_t *cap, uint32_t needed)
	{
	size_t old_size;

	if (needed <= *cap)
		{
		return 1;
		}

	old_size = (size_t)(*cap) * sizeof(uint32_t);
	while (*cap < needed)
		{
		*cap = (*cap == 0U) ? LXS_INITIAL_CAP : (*cap * 2U);
		}

	*items = lxs_realloc_aligned(*items, old_size, (size_t)(*cap) * sizeof(uint32_t));
	return *items != NULL;
	}

static int lxs_reserve_gates(lxs_gate_ir **items, uint32_t *cap, uint32_t needed)
	{
	size_t old_size;

	if (needed <= *cap)
		{
		return 1;
		}

	old_size = (size_t)(*cap) * sizeof(lxs_gate_ir);
	while (*cap < needed)
		{
		*cap = (*cap == 0U) ? LXS_INITIAL_CAP : (*cap * 2U);
		}

	*items = lxs_realloc_aligned(*items, old_size, (size_t)(*cap) * sizeof(lxs_gate_ir));
	return *items != NULL;
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
	if (!nl->net_names[id])
		{
		return UINT32_MAX;
		}

	return id;
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

	lxs_free_aligned(lines);
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

			new_buffer = realloc(buffer, new_cap);
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

static lxs_gate_type lxs_string_to_gate_type(const char *name)
	{
	if (strcmp(name, "AND") == 0)
		{
		return LXS_GATE_AND;
		}

	if (strcmp(name, "OR") == 0)
		{
		return LXS_GATE_OR;
		}

	if (strcmp(name, "XOR") == 0)
		{
		return LXS_GATE_XOR;
		}

	if (strcmp(name, "TRI") == 0)
		{
		return LXS_GATE_TRI;
		}

	if (strcmp(name, "NOT") == 0)
		{
		return LXS_GATE_NOT;
		}

	if (strcmp(name, "NAND") == 0)
		{
		return LXS_GATE_NAND;
		}

	if (strcmp(name, "NOR") == 0)
		{
		return LXS_GATE_NOR;
		}

	if (strcmp(name, "XNOR") == 0)
		{
		return LXS_GATE_XNOR;
		}

	if (strcmp(name, "BUF") == 0 || strcmp(name, "BUFF") == 0)
		{
		return LXS_GATE_BUF;
		}

	if (strcmp(name, "DFF") == 0)
		{
		return LXS_GATE_DFF;
		}

	return LXS_GATE_BUF;
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
			if (const1 == UINT32_MAX)
				{
				return 0;
				}
			return lxs_emit_unary_gate(nl, LXS_GATE_BUF, const1, output);
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
			if (const1 == UINT32_MAX)
				{
				return 0;
				}
			return lxs_emit_unary_gate(nl, LXS_GATE_BUF, const1, output);
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
			if (const1 == UINT32_MAX)
				{
				return 0;
				}
			return lxs_emit_unary_gate(nl, LXS_GATE_BUF, const1, output);
			}
		default:
			return 0;
		}
	}

static uint32_t lxs_gate_level(
	uint32_t gate_index,
	const lxs_netlist *nl,
	const int32_t *comb_driver,
	uint32_t *cache,
	uint8_t *visiting)
	{
	const lxs_gate_ir *gate = &nl->gates[gate_index];
	uint32_t max_level = 0U;

	if (cache[gate_index] != UINT32_MAX)
		{
		return cache[gate_index];
		}

	if (visiting[gate_index])
		{
		return 0U;
		}

	visiting[gate_index] = 1U;
	for (uint32_t i = 0; i < gate->input_count; ++i)
		{
		int32_t driver = comb_driver[gate->inputs[i]];
		if (driver >= 0)
			{
			uint32_t parent_level = lxs_gate_level((uint32_t)driver, nl, comb_driver, cache, visiting) + 1U;
			if (parent_level > max_level)
				{
				max_level = parent_level;
				}
			}
		}
	visiting[gate_index] = 0U;
	cache[gate_index] = max_level;
	return max_level;
	}

static uint32_t lxs_count_bucket_gates(const lxs_netlist *nl, uint32_t level, uint32_t type)
	{
	uint32_t count = 0U;

	for (uint32_t i = 0; i < nl->gate_count; ++i)
		{
		if (nl->gates[i].type == type && nl->gates[i].level == level)
			{
			count++;
			}
		}

	return count;
	}

static uint32_t g_lxs_sort_type = 0U;

static int __cdecl lxs_compare_bucket_gates(const void *lhs, const void *rhs)
	{
	const lxs_gate_ir *a = (const lxs_gate_ir*)lhs;
	const lxs_gate_ir *b = (const lxs_gate_ir*)rhs;
	uint32_t a0 = a->input_count > 0U ? a->inputs[0] : 0U;
	uint32_t b0 = b->input_count > 0U ? b->inputs[0] : 0U;
	uint32_t a1 = a->input_count > 1U ? a->inputs[1] : 0U;
	uint32_t b1 = b->input_count > 1U ? b->inputs[1] : 0U;

	if (a0 != b0)
		{
		return a0 < b0 ? -1 : 1;
		}

	if (g_lxs_sort_type != LXS_GATE_NOT && g_lxs_sort_type != LXS_GATE_BUF && a1 != b1)
		{
		return a1 < b1 ? -1 : 1;
		}

	if (a->output != b->output)
		{
		return a->output < b->output ? -1 : 1;
		}

	return 1U;
	}

static void lxs_sort_bucket_gates(lxs_gate_ir *gates, uint32_t count, uint32_t type)
	{
	if (count < 2U)
		{
		g_lxs_sort_type = type;
		qsort(gates, count, sizeof(lxs_gate_ir), lxs_compare_bucket_gates);
		return;
		}

	g_lxs_sort_type = type;
	qsort(gates, count, sizeof(lxs_gate_ir), lxs_compare_bucket_gates);
	}

static void lxs_assign_net_group(
	uint32_t *remap,
	uint8_t *assigned,
	const uint32_t *items,
	uint32_t count,
	uint32_t *next_id)
	{
	for (uint32_t i = 0; i < count; ++i)
		{
		uint32_t net_id = items[i];
		if (!assigned[net_id])
			{
			assigned[net_id] = 1U;
			remap[net_id] = (*next_id)++;
			}
		}
	}

static void lxs_assign_remaining_nets(
	uint32_t *remap,
	uint8_t *assigned,
	uint32_t net_count,
	uint32_t *next_id)
	{
	for (uint32_t net_id = 0; net_id < net_count; ++net_id)
		{
		if (!assigned[net_id])
			{
			assigned[net_id] = 1U;
			remap[net_id] = (*next_id)++;
			}
		}
	}

static void lxs_apply_net_remap_to_array(uint32_t *items, uint32_t count, const uint32_t *remap)
	{
	for (uint32_t i = 0; i < count; ++i)
		{
		items[i] = remap[items[i]];
		}
	}

static void lxs_apply_net_remap_to_gates(lxs_gate_ir *gates, uint32_t count, const uint32_t *remap)
	{
	for (uint32_t i = 0; i < count; ++i)
		{
		for (uint32_t j = 0; j < gates[i].input_count; ++j)
			{
			gates[i].inputs[j] = remap[gates[i].inputs[j]];
			}
		gates[i].output = remap[gates[i].output];
		}
	}

static uint8_t lxs_find_contiguous_range(const uint32_t *items, uint32_t count, uint32_t *base)
	{
	if (count == 0U)
		{
		*base = 0U;
		return 1U;
		}

	*base = items[0];
	for (uint32_t i = 1; i < count; ++i)
		{
		if (items[i] != items[0] + i)
			{
			return 0U;
			}
		}

	return 1U;
	}

static uint32_t lxs_other_input(const lxs_gate_ir *gate, uint32_t known_input)
	{
	if (gate->input_count < 2U)
		{
		return UINT32_MAX;
		}

	if (gate->inputs[0] == known_input)
		{
		return gate->inputs[1];
		}

	if (gate->inputs[1] == known_input)
		{
		return gate->inputs[0];
		}

	return UINT32_MAX;
	}

static int lxs_gate_uses_input(const lxs_gate_ir *gate, uint32_t net_id)
	{
	for (uint32_t i = 0; i < gate->input_count; ++i)
		{
		if (gate->inputs[i] == net_id)
			{
			return 1;
			}
		}

	return 0;
	}

static int lxs_try_match_mux2(
	const lxs_netlist *nl,
	const int32_t *comb_driver,
	const uint32_t *net_use_count,
	uint8_t *matched_gates,
	uint32_t gate_index,
	lxs_macro_plan *macro)
	{
	const lxs_gate_ir *or_gate;
	const lxs_gate_ir *and_a;
	const lxs_gate_ir *and_b;
	const lxs_gate_ir *not_gate;
	int32_t and_a_index;
	int32_t and_b_index;
	int32_t not_index;
	uint32_t data0;
	uint32_t data1;
	uint32_t sel;
	uint32_t nsel;
	int first_and_is_true;

	or_gate = &nl->gates[gate_index];
	if (or_gate->type != LXS_GATE_OR || or_gate->input_count != 2U || matched_gates[gate_index])
		{
		return 0;
		}

	and_a_index = comb_driver[or_gate->inputs[0]];
	and_b_index = comb_driver[or_gate->inputs[1]];
	if (and_a_index < 0 || and_b_index < 0)
		{
		return 0;
		}

	and_a = &nl->gates[and_a_index];
	and_b = &nl->gates[and_b_index];
	if (matched_gates[and_a_index] || matched_gates[and_b_index] ||
		and_a->type != LXS_GATE_AND || and_b->type != LXS_GATE_AND ||
		and_a->input_count != 2U || and_b->input_count != 2U)
		{
		return 0;
		}

	if (net_use_count[and_a->output] != 1U || net_use_count[and_b->output] != 1U)
		{
		return 0;
		}

	first_and_is_true = 1;
	not_index = comb_driver[and_a->inputs[0]];
	if (not_index >= 0 && nl->gates[not_index].type == LXS_GATE_NOT)
		{
		not_gate = &nl->gates[not_index];
		nsel = and_a->inputs[0];
		sel = not_gate->inputs[0];
		if (!lxs_gate_uses_input(and_b, sel))
			{
			first_and_is_true = 0;
			}
		}
	else
		{
		not_index = comb_driver[and_a->inputs[1]];
		if (not_index >= 0 && nl->gates[not_index].type == LXS_GATE_NOT)
			{
			not_gate = &nl->gates[not_index];
			nsel = and_a->inputs[1];
			sel = not_gate->inputs[0];
			if (!lxs_gate_uses_input(and_b, sel))
				{
				first_and_is_true = 0;
				}
			}
		else
			{
			first_and_is_true = 0;
			}
		}

	if (!first_and_is_true)
		{
		not_index = comb_driver[and_b->inputs[0]];
		if (not_index >= 0 && nl->gates[not_index].type == LXS_GATE_NOT)
			{
			not_gate = &nl->gates[not_index];
			nsel = and_b->inputs[0];
			sel = not_gate->inputs[0];
			if (!lxs_gate_uses_input(and_a, sel))
				{
				return 0;
				}
			data0 = lxs_other_input(and_b, nsel);
			data1 = lxs_other_input(and_a, sel);
			}
		else
			{
			not_index = comb_driver[and_b->inputs[1]];
			if (not_index < 0 || nl->gates[not_index].type != LXS_GATE_NOT)
				{
				return 0;
				}

			not_gate = &nl->gates[not_index];
			nsel = and_b->inputs[1];
			sel = not_gate->inputs[0];
			if (!lxs_gate_uses_input(and_a, sel))
				{
				return 0;
				}
			data0 = lxs_other_input(and_b, nsel);
			data1 = lxs_other_input(and_a, sel);
			}
		}
	else
		{
		data0 = lxs_other_input(and_a, nsel);
		data1 = lxs_other_input(and_b, sel);
		}

	if (not_gate->input_count != 1U || matched_gates[not_index] ||
		net_use_count[nsel] != 1U ||
		data0 == UINT32_MAX || data1 == UINT32_MAX || data0 == sel || data1 == nsel)
		{
		return 0;
		}

	matched_gates[gate_index] = 1U;
	matched_gates[(uint32_t)and_a_index] = 1U;
	matched_gates[(uint32_t)and_b_index] = 1U;
	matched_gates[(uint32_t)not_index] = 1U;

	memset(macro, 0, sizeof(*macro));
	macro->type = LXS_MACRO_MUX2;
	macro->level = or_gate->level;
	macro->inputs[0] = data0;
	macro->inputs[1] = data1;
	macro->inputs[2] = sel;
	macro->output = or_gate->output;
	macro->gate_equiv_count = 4U;
	return 1;
	}

static int lxs_extract_inverted_and_term(
	const lxs_netlist *nl,
	const int32_t *comb_driver,
	const uint32_t *net_use_count,
	const lxs_gate_ir *gate,
	uint32_t *base,
	uint32_t *other,
	uint32_t *not_gate_index)
	{
	int32_t not_index;

	if (gate->type != LXS_GATE_AND || gate->input_count != 2U)
		{
		return 0;
		}

	not_index = comb_driver[gate->inputs[0]];
	if (not_index >= 0 &&
		nl->gates[not_index].type == LXS_GATE_NOT &&
		net_use_count[gate->inputs[0]] == 1U)
		{
		*base = nl->gates[not_index].inputs[0];
		*other = gate->inputs[1];
		*not_gate_index = (uint32_t)not_index;
		return 1;
		}

	not_index = comb_driver[gate->inputs[1]];
	if (not_index >= 0 &&
		nl->gates[not_index].type == LXS_GATE_NOT &&
		net_use_count[gate->inputs[1]] == 1U)
		{
		*base = nl->gates[not_index].inputs[0];
		*other = gate->inputs[0];
		*not_gate_index = (uint32_t)not_index;
		return 1;
		}

	return 0;
	}

static int lxs_extract_double_inverted_and_term(
	const lxs_netlist *nl,
	const int32_t *comb_driver,
	const uint32_t *net_use_count,
	const lxs_gate_ir *gate,
	uint32_t *base_a,
	uint32_t *base_b,
	uint32_t *not_a_index,
	uint32_t *not_b_index)
	{
	int32_t first_not;
	int32_t second_not;

	if (gate->type != LXS_GATE_AND || gate->input_count != 2U)
		{
		return 0;
		}

	first_not = comb_driver[gate->inputs[0]];
	second_not = comb_driver[gate->inputs[1]];
	if (first_not < 0 || second_not < 0)
		{
		return 0;
		}

	if (nl->gates[first_not].type != LXS_GATE_NOT ||
		nl->gates[second_not].type != LXS_GATE_NOT ||
		nl->gates[first_not].input_count != 1U ||
		nl->gates[second_not].input_count != 1U ||
		net_use_count[gate->inputs[0]] != 1U ||
		net_use_count[gate->inputs[1]] != 1U)
		{
		return 0;
		}

	*base_a = nl->gates[first_not].inputs[0];
	*base_b = nl->gates[second_not].inputs[0];
	*not_a_index = (uint32_t)first_not;
	*not_b_index = (uint32_t)second_not;
	return 1;
	}

static int lxs_try_match_xor2(
	const lxs_netlist *nl,
	const int32_t *comb_driver,
	const uint32_t *net_use_count,
	uint8_t *matched_gates,
	uint32_t gate_index,
	lxs_macro_plan *macro)
	{
	const lxs_gate_ir *or_gate;
	const lxs_gate_ir *and_a;
	const lxs_gate_ir *and_b;
	int32_t and_a_index;
	int32_t and_b_index;
	uint32_t base_a;
	uint32_t base_b;
	uint32_t other_a;
	uint32_t other_b;
	uint32_t not_a_index;
	uint32_t not_b_index;

	or_gate = &nl->gates[gate_index];
	if (or_gate->type != LXS_GATE_OR || or_gate->input_count != 2U || matched_gates[gate_index])
		{
		return 0;
		}

	and_a_index = comb_driver[or_gate->inputs[0]];
	and_b_index = comb_driver[or_gate->inputs[1]];
	if (and_a_index < 0 || and_b_index < 0)
		{
		return 0;
		}

	and_a = &nl->gates[and_a_index];
	and_b = &nl->gates[and_b_index];
	if (matched_gates[and_a_index] || matched_gates[and_b_index] ||
		net_use_count[and_a->output] != 1U || net_use_count[and_b->output] != 1U)
		{
		return 0;
		}

	if (!lxs_extract_inverted_and_term(
			nl,
			comb_driver,
			net_use_count,
			and_a,
			&base_a,
			&other_a,
			&not_a_index) ||
		!lxs_extract_inverted_and_term(
			nl,
			comb_driver,
			net_use_count,
			and_b,
			&base_b,
			&other_b,
			&not_b_index))
		{
		return 0;
		}

	if (matched_gates[not_a_index] || matched_gates[not_b_index] ||
		base_a == other_a || base_b == other_b ||
		base_a != other_b || base_b != other_a)
		{
		return 0;
		}

	matched_gates[gate_index] = 1U;
	matched_gates[(uint32_t)and_a_index] = 1U;
	matched_gates[(uint32_t)and_b_index] = 1U;
	matched_gates[not_a_index] = 1U;
	matched_gates[not_b_index] = 1U;

	memset(macro, 0, sizeof(*macro));
	macro->type = LXS_MACRO_XOR2;
	macro->level = or_gate->level;
	macro->inputs[0] = base_a;
	macro->inputs[1] = base_b;
	macro->output = or_gate->output;
	macro->gate_equiv_count = 5U;
	return 1;
	}

static int lxs_try_match_xor2_nor_and(
	const lxs_netlist *nl,
	const int32_t *comb_driver,
	const uint32_t *net_use_count,
	uint8_t *matched_gates,
	uint32_t gate_index,
	lxs_macro_plan *macro)
	{
	const lxs_gate_ir *nor_gate;
	const lxs_gate_ir *a_gate;
	const lxs_gate_ir *b_gate;
	int32_t a_index;
	int32_t b_index;
	uint32_t a0;
	uint32_t a1;
	uint32_t b0;
	uint32_t b1;

	nor_gate = &nl->gates[gate_index];
	if (nor_gate->type != LXS_GATE_NOR || nor_gate->input_count != 2U || matched_gates[gate_index])
		{
		return 0;
		}

	a_index = comb_driver[nor_gate->inputs[0]];
	b_index = comb_driver[nor_gate->inputs[1]];
	if (a_index < 0 || b_index < 0)
		{
		return 0;
		}

	a_gate = &nl->gates[a_index];
	b_gate = &nl->gates[b_index];
	if (matched_gates[a_index] || matched_gates[b_index] ||
		a_gate->input_count != 2U || b_gate->input_count != 2U ||
		net_use_count[a_gate->output] != 1U || net_use_count[b_gate->output] != 1U)
		{
		return 0;
		}

	if (!((a_gate->type == LXS_GATE_NOR && b_gate->type == LXS_GATE_AND) ||
		(a_gate->type == LXS_GATE_AND && b_gate->type == LXS_GATE_NOR)))
		{
		return 0;
		}

	a0 = a_gate->inputs[0];
	a1 = a_gate->inputs[1];
	b0 = b_gate->inputs[0];
	b1 = b_gate->inputs[1];
	if (!((a0 == b0 && a1 == b1) || (a0 == b1 && a1 == b0)))
		{
		return 0;
		}

	matched_gates[gate_index] = 1U;
	matched_gates[(uint32_t)a_index] = 1U;
	matched_gates[(uint32_t)b_index] = 1U;

	memset(macro, 0, sizeof(*macro));
	macro->type = LXS_MACRO_XOR2;
	macro->level = nor_gate->level;
	macro->inputs[0] = a0;
	macro->inputs[1] = a1;
	macro->output = nor_gate->output;
	macro->gate_equiv_count = 3U;
	return 1;
	}

static int lxs_try_match_xnor2(
	const lxs_netlist *nl,
	const int32_t *comb_driver,
	const uint32_t *net_use_count,
	uint8_t *matched_gates,
	uint32_t gate_index,
	lxs_macro_plan *macro)
	{
	const lxs_gate_ir *or_gate;
	const lxs_gate_ir *a_gate;
	const lxs_gate_ir *b_gate;
	const lxs_gate_ir *plain_gate;
	int32_t a_index;
	int32_t b_index;
	int32_t plain_index;
	int32_t inverted_index;
	uint32_t plain_a;
	uint32_t plain_b;
	uint32_t inv_a;
	uint32_t inv_b;
	uint32_t not_a_index;
	uint32_t not_b_index;

	or_gate = &nl->gates[gate_index];
	if (or_gate->type != LXS_GATE_OR || or_gate->input_count != 2U || matched_gates[gate_index])
		{
		return 0;
		}

	a_index = comb_driver[or_gate->inputs[0]];
	b_index = comb_driver[or_gate->inputs[1]];
	if (a_index < 0 || b_index < 0)
		{
		return 0;
		}

	a_gate = &nl->gates[a_index];
	b_gate = &nl->gates[b_index];
	if (matched_gates[a_index] || matched_gates[b_index] ||
		a_gate->type != LXS_GATE_AND || b_gate->type != LXS_GATE_AND ||
		a_gate->input_count != 2U || b_gate->input_count != 2U ||
		net_use_count[a_gate->output] != 1U || net_use_count[b_gate->output] != 1U)
		{
		return 0;
		}

	plain_index = a_index;
	inverted_index = b_index;
	plain_gate = a_gate;
	plain_a = a_gate->inputs[0];
	plain_b = a_gate->inputs[1];
	if (!lxs_extract_double_inverted_and_term(
			nl,
			comb_driver,
			net_use_count,
			b_gate,
			&inv_a,
			&inv_b,
			&not_a_index,
			&not_b_index))
		{
		plain_index = b_index;
		inverted_index = a_index;
		plain_gate = b_gate;
		plain_a = b_gate->inputs[0];
		plain_b = b_gate->inputs[1];
		if (!lxs_extract_double_inverted_and_term(
				nl,
				comb_driver,
				net_use_count,
				a_gate,
				&inv_a,
				&inv_b,
				&not_a_index,
				&not_b_index))
			{
			return 0;
			}
		}

	if (matched_gates[not_a_index] || matched_gates[not_b_index] ||
		plain_a == plain_b ||
		!((plain_a == inv_a && plain_b == inv_b) || (plain_a == inv_b && plain_b == inv_a)))
		{
		return 0;
		}

	(void)plain_gate;
	(void)inverted_index;

	matched_gates[gate_index] = 1U;
	matched_gates[(uint32_t)plain_index] = 1U;
	matched_gates[(uint32_t)inverted_index] = 1U;
	matched_gates[not_a_index] = 1U;
	matched_gates[not_b_index] = 1U;

	memset(macro, 0, sizeof(*macro));
	macro->type = LXS_MACRO_XNOR2;
	macro->level = or_gate->level;
	macro->inputs[0] = plain_a;
	macro->inputs[1] = plain_b;
	macro->output = or_gate->output;
	macro->gate_equiv_count = 5U;
	return 1;
	}

static int lxs_try_match_carry_inv2(
	const lxs_netlist *nl,
	const int32_t *comb_driver,
	const uint32_t *net_use_count,
	uint8_t *matched_gates,
	uint32_t gate_index,
	lxs_macro_plan *macro)
	{
	const lxs_gate_ir *outer_nor;
	const lxs_gate_ir *and_gate;
	const lxs_gate_ir *inner_nor;
	const lxs_gate_ir *nor_ab_gate;
	int32_t and_index;
	int32_t inner_index;
	int32_t nor_ab_index;
	uint32_t and_a;
	uint32_t and_b;
	uint32_t nor_a;
	uint32_t nor_b;
	uint32_t carry_inv_in;

	outer_nor = &nl->gates[gate_index];
	if (outer_nor->type != LXS_GATE_NOR || outer_nor->input_count != 2U || matched_gates[gate_index])
		{
		return 0;
		}

	and_index = comb_driver[outer_nor->inputs[0]];
	inner_index = comb_driver[outer_nor->inputs[1]];
	if (and_index < 0 || inner_index < 0 ||
		nl->gates[and_index].type != LXS_GATE_AND ||
		nl->gates[inner_index].type != LXS_GATE_NOR)
		{
		and_index = comb_driver[outer_nor->inputs[1]];
		inner_index = comb_driver[outer_nor->inputs[0]];
		if (and_index < 0 || inner_index < 0 ||
			nl->gates[and_index].type != LXS_GATE_AND ||
			nl->gates[inner_index].type != LXS_GATE_NOR)
			{
			return 0;
			}
		}

	and_gate = &nl->gates[and_index];
	inner_nor = &nl->gates[inner_index];
	if (matched_gates[and_index] || matched_gates[inner_index] ||
		and_gate->input_count != 2U || inner_nor->input_count != 2U ||
		net_use_count[inner_nor->output] != 1U)
		{
		return 0;
		}

	nor_ab_index = comb_driver[inner_nor->inputs[0]];
	carry_inv_in = inner_nor->inputs[1];
	if (nor_ab_index < 0 || nl->gates[nor_ab_index].type != LXS_GATE_NOR)
		{
		nor_ab_index = comb_driver[inner_nor->inputs[1]];
		carry_inv_in = inner_nor->inputs[0];
		if (nor_ab_index < 0 || nl->gates[nor_ab_index].type != LXS_GATE_NOR)
			{
			return 0;
			}
		}

	nor_ab_gate = &nl->gates[nor_ab_index];
	if (matched_gates[nor_ab_index] || nor_ab_gate->input_count != 2U)
		{
		return 0;
		}

	and_a = and_gate->inputs[0];
	and_b = and_gate->inputs[1];
	nor_a = nor_ab_gate->inputs[0];
	nor_b = nor_ab_gate->inputs[1];
	if (!((and_a == nor_a && and_b == nor_b) || (and_a == nor_b && and_b == nor_a)))
		{
		return 0;
		}

	matched_gates[gate_index] = 1U;
	matched_gates[(uint32_t)inner_index] = 1U;

	memset(macro, 0, sizeof(*macro));
	macro->type = LXS_MACRO_CARRY_INV2;
	macro->level = outer_nor->level;
	macro->inputs[0] = and_gate->output;
	macro->inputs[1] = nor_ab_gate->output;
	macro->inputs[2] = carry_inv_in;
	macro->output = outer_nor->output;
	macro->gate_equiv_count = 2U;
	return 1;
	}

static int lxs_try_match_sum_cinv2(
	const lxs_netlist *nl,
	const int32_t *comb_driver,
	const uint32_t *net_use_count,
	uint8_t *matched_gates,
	uint32_t gate_index,
	lxs_macro_plan *macro)
	{
	const lxs_gate_ir *sum_gate;
	const lxs_gate_ir *xor_gate;
	const lxs_gate_ir *and_gate;
	const lxs_gate_ir *nor_ab_gate;
	int32_t xor_index;
	int32_t and_index;
	int32_t nor_ab_index;
	uint32_t cin_n;
	uint32_t a;
	uint32_t b;

	sum_gate = &nl->gates[gate_index];
	if (sum_gate->type != LXS_GATE_XNOR || sum_gate->input_count != 2U || matched_gates[gate_index])
		{
		return 0;
		}

	xor_index = comb_driver[sum_gate->inputs[0]];
	cin_n = sum_gate->inputs[1];
	if (xor_index < 0 || nl->gates[xor_index].type != LXS_GATE_NOR)
		{
		xor_index = comb_driver[sum_gate->inputs[1]];
		cin_n = sum_gate->inputs[0];
		if (xor_index < 0 || nl->gates[xor_index].type != LXS_GATE_NOR)
			{
			return 0;
			}
		}

	xor_gate = &nl->gates[xor_index];
	if (matched_gates[xor_index] || xor_gate->input_count != 2U || net_use_count[xor_gate->output] != 1U)
		{
		return 0;
		}

	and_index = comb_driver[xor_gate->inputs[0]];
	nor_ab_index = comb_driver[xor_gate->inputs[1]];
	if (and_index < 0 || nor_ab_index < 0 ||
		nl->gates[and_index].type != LXS_GATE_AND ||
		nl->gates[nor_ab_index].type != LXS_GATE_NOR)
		{
		and_index = comb_driver[xor_gate->inputs[1]];
		nor_ab_index = comb_driver[xor_gate->inputs[0]];
		if (and_index < 0 || nor_ab_index < 0 ||
			nl->gates[and_index].type != LXS_GATE_AND ||
			nl->gates[nor_ab_index].type != LXS_GATE_NOR)
			{
			return 0;
			}
		}

	and_gate = &nl->gates[and_index];
	nor_ab_gate = &nl->gates[nor_ab_index];
	if (matched_gates[and_index] || matched_gates[nor_ab_index] ||
		and_gate->input_count != 2U || nor_ab_gate->input_count != 2U)
		{
		return 0;
		}

	a = and_gate->inputs[0];
	b = and_gate->inputs[1];
	if (!((nor_ab_gate->inputs[0] == a && nor_ab_gate->inputs[1] == b) ||
		(nor_ab_gate->inputs[0] == b && nor_ab_gate->inputs[1] == a)))
		{
		return 0;
		}

	matched_gates[gate_index] = 1U;
	matched_gates[(uint32_t)xor_index] = 1U;

	memset(macro, 0, sizeof(*macro));
	macro->type = LXS_MACRO_SUM_CINV2;
	macro->level = sum_gate->level;
	macro->inputs[0] = nor_ab_gate->output;
	macro->inputs[1] = and_gate->output;
	macro->inputs[2] = cin_n;
	macro->output = sum_gate->output;
	macro->gate_equiv_count = 2U;
	return 1;
	}

static uint32_t lxs_collect_macros(
	const lxs_netlist *nl,
	const int32_t *comb_driver,
	uint8_t *matched_gates,
	lxs_macro_plan *macros)
	{
	uint32_t *net_use_count;
	uint32_t macro_count = 0U;

	net_use_count = lxs_calloc_aligned(nl->net_count, sizeof(uint32_t));
	if (!net_use_count)
		{
		return UINT32_MAX;
		}

	for (uint32_t i = 0; i < nl->gate_count; ++i)
		{
		for (uint32_t j = 0; j < nl->gates[i].input_count; ++j)
			{
			net_use_count[nl->gates[i].inputs[j]]++;
			}
		}

	for (uint32_t i = 0; i < nl->output_count; ++i)
		{
		net_use_count[nl->outputs[i]]++;
		}

	for (uint32_t i = 0; i < nl->gate_count; ++i)
		{
		lxs_macro_plan macro;

		if (lxs_try_match_sum_cinv2(nl, comb_driver, net_use_count, matched_gates, i, &macro) ||
			lxs_try_match_xnor2(nl, comb_driver, net_use_count, matched_gates, i, &macro) ||
			lxs_try_match_carry_inv2(nl, comb_driver, net_use_count, matched_gates, i, &macro) ||
			lxs_try_match_xor2_nor_and(nl, comb_driver, net_use_count, matched_gates, i, &macro) ||
			lxs_try_match_xor2(nl, comb_driver, net_use_count, matched_gates, i, &macro) ||
			lxs_try_match_mux2(nl, comb_driver, net_use_count, matched_gates, i, &macro))
			{
			if (macros)
				{
				macros[macro_count] = macro;
				}
			macro_count++;
			}
		}

	lxs_free_aligned(net_use_count);
	return macro_count;
	}

static lxs_netlist* lxs_load_bench(const char *path)
	{
	FILE *stream;
	char line[1024];
	lxs_netlist *nl;

	stream = fopen(path, "r");
	if (!stream)
		{
		return NULL;
		}

	nl = lxs_calloc_aligned(1U, sizeof(lxs_netlist));
	if (!nl)
		{
		fclose(stream);
		return NULL;
		}

	while (fgets(line, sizeof(line), stream))
		{
		char *trimmed = lxs_trim(line);
		char *equal_sign;

		if (trimmed[0] == '\0' || trimmed[0] == '#')
			{
			continue;
			}

		if (strncmp(trimmed, "INPUT(", 6) == 0)
			{
			char *name = trimmed + 6;
			char *end = strchr(name, ')');
			uint32_t id;

			if (!end)
				{
				continue;
				}

			*end = '\0';
			id = lxs_intern_net(nl, lxs_trim(name));
			if (id == UINT32_MAX || !lxs_push_u32(&nl->inputs, &nl->input_count, &nl->input_cap, id))
				{
				lxs_free_netlist(nl);
				fclose(stream);
				return NULL;
				}
			continue;
			}

		if (strncmp(trimmed, "OUTPUT(", 7) == 0)
			{
			char *name = trimmed + 7;
			char *end = strchr(name, ')');
			uint32_t id;

			if (!end)
				{
				continue;
				}

			*end = '\0';
			id = lxs_intern_net(nl, lxs_trim(name));
			if (id == UINT32_MAX || !lxs_push_u32(&nl->outputs, &nl->output_count, &nl->output_cap, id))
				{
				lxs_free_netlist(nl);
				fclose(stream);
				return NULL;
				}
			continue;
			}

		equal_sign = strchr(trimmed, '=');
		if (!equal_sign)
			{
			continue;
			}

		*equal_sign = '\0';
		{
		char *out_name = lxs_trim(trimmed);
		char *gate_part = lxs_trim(equal_sign + 1);
		char *open_paren = strchr(gate_part, '(');
		char *close_paren;
		lxs_gate_ir gate;

		if (!open_paren)
			{
			continue;
			}

		*open_paren = '\0';
		close_paren = strchr(open_paren + 1, ')');
		if (!close_paren)
			{
			continue;
			}

		*close_paren = '\0';
		memset(&gate, 0, sizeof(gate));
		gate.type = lxs_string_to_gate_type(lxs_trim(gate_part));
		gate.output = lxs_intern_net(nl, out_name);
		if (gate.output == UINT32_MAX)
			{
			lxs_free_netlist(nl);
			fclose(stream);
			return NULL;
			}

		{
		char *ctx = NULL;
		char *token = strtok_s(open_paren + 1, ",", &ctx);
		while (token && gate.input_count < 2U)
			{
			uint32_t id = lxs_intern_net(nl, lxs_trim(token));
			if (id == UINT32_MAX)
				{
				lxs_free_netlist(nl);
				fclose(stream);
				return NULL;
				}

			gate.inputs[gate.input_count++] = id;
			token = strtok_s(NULL, ",", &ctx);
			}
		}

		if (!lxs_push_gate(nl, &gate))
			{
			lxs_free_netlist(nl);
			fclose(stream);
			return NULL;
			}
		}
		}

	fclose(stream);
	return nl;
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
	nl = lxs_calloc_aligned(1U, sizeof(lxs_netlist));
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
					lxs_free_lines(lines, line_count);
					lxs_free_aligned(invert_cache);
					lxs_free_netlist(nl);
					return NULL;
					}

				if (is_input)
					{
					if (!lxs_push_u32(&nl->inputs, &nl->input_count, &nl->input_cap, id))
						{
						lxs_free_lines(lines, line_count);
						lxs_free_aligned(invert_cache);
						lxs_free_netlist(nl);
						return NULL;
						}
					}
				else
					{
					if (!lxs_push_u32(&nl->outputs, &nl->output_count, &nl->output_cap, id))
						{
						lxs_free_lines(lines, line_count);
						lxs_free_aligned(invert_cache);
						lxs_free_netlist(nl);
						return NULL;
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
				lxs_free_lines(lines, line_count);
				lxs_free_aligned(invert_cache);
				lxs_free_netlist(nl);
				return NULL;
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
				lxs_free_lines(lines, line_count);
				lxs_free_aligned(invert_cache);
				lxs_free_netlist(nl);
				return NULL;
				}

			for (uint32_t j = 0; j + 1U < token_count; ++j)
				{
				input_ids[j] = lxs_intern_net(nl, tokens[j]);
				if (input_ids[j] == UINT32_MAX)
					{
					lxs_free_lines(lines, line_count);
					lxs_free_aligned(invert_cache);
					lxs_free_netlist(nl);
					return NULL;
					}
				}

			if (!lxs_blif_truth_table(token_count - 1U, covers, cover_count, truth) ||
				!lxs_blif_emit_truth(
					nl,
					output_id,
					input_ids,
					token_count - 1U,
					truth,
					&invert_cache,
					&invert_cap,
					&const1_id))
				{
				lxs_free_lines(lines, line_count);
				lxs_free_aligned(invert_cache);
				lxs_free_netlist(nl);
				return NULL;
				}

			i = cover_end;
			}
		}

	lxs_free_lines(lines, line_count);
	lxs_free_aligned(invert_cache);
	return nl;
	}

lxs_netlist* lxs_load_iscas(const char *path)
	{
	if (lxs_has_suffix_ci(path, ".blif"))
		{
		return lxs_load_blif(path);
		}

	return lxs_load_bench(path);
	}

void lxs_free_netlist(lxs_netlist *nl)
	{
	if (!nl)
		{
		return;
		}

	for (uint32_t i = 0; i < nl->net_count; ++i)
		{
		free(nl->net_names[i]);
		}

	lxs_free_aligned(nl->net_names);
	lxs_free_aligned(nl->inputs);
	lxs_free_aligned(nl->outputs);
	lxs_free_aligned(nl->gates);
	lxs_free_aligned(nl);
	}

lxs_plan* lxs_compile_to_plan(lxs_netlist *nl)
	{
	int32_t *comb_driver;
	uint32_t *level_cache;
	uint8_t *visiting;
	uint32_t *net_remap;
	uint8_t *net_assigned;
	uint8_t *matched_gates;
	lxs_plan *plan;
	uint32_t max_level = 0U;
	uint32_t chunk_fill = 0U;
	uint32_t gate_fill = 0U;
	uint32_t dff_fill = 0U;
	uint32_t next_net_id = 0U;
	uint32_t macro_fill = 0U;
	uint32_t macro_level_fill = 0U;

	if (!nl)
		{
		return NULL;
		}

	comb_driver = lxs_calloc_aligned(nl->net_count, sizeof(int32_t));
	level_cache = lxs_calloc_aligned(nl->gate_count, sizeof(uint32_t));
	visiting = lxs_calloc_aligned(nl->gate_count, sizeof(uint8_t));
	net_remap = lxs_calloc_aligned(nl->net_count, sizeof(uint32_t));
	net_assigned = lxs_calloc_aligned(nl->net_count, sizeof(uint8_t));
	matched_gates = lxs_calloc_aligned(nl->gate_count, sizeof(uint8_t));
	if (!comb_driver || !level_cache || !visiting || !net_remap || !net_assigned || !matched_gates)
		{
		lxs_free_aligned(comb_driver);
		lxs_free_aligned(level_cache);
		lxs_free_aligned(visiting);
		lxs_free_aligned(net_remap);
		lxs_free_aligned(net_assigned);
		lxs_free_aligned(matched_gates);
		return NULL;
		}

	for (uint32_t i = 0; i < nl->net_count; ++i)
		{
		comb_driver[i] = -1;
		}

	for (uint32_t i = 0; i < nl->gate_count; ++i)
		{
		level_cache[i] = UINT32_MAX;
		if (nl->gates[i].type != LXS_GATE_DFF)
			{
			comb_driver[nl->gates[i].output] = (int32_t)i;
			}
		}

	for (uint32_t i = 0; i < nl->gate_count; ++i)
		{
		if (nl->gates[i].type == LXS_GATE_DFF)
			{
			nl->gates[i].level = UINT32_MAX;
			continue;
			}

		nl->gates[i].level = lxs_gate_level(i, nl, comb_driver, level_cache, visiting);
		if (nl->gates[i].level > max_level)
			{
			max_level = nl->gates[i].level;
			}
		}

	lxs_assign_net_group(net_remap, net_assigned, nl->inputs, nl->input_count, &next_net_id);
	for (uint32_t i = 0; i < nl->gate_count; ++i)
		{
		if (nl->gates[i].type == LXS_GATE_DFF)
			{
			lxs_assign_net_group(net_remap, net_assigned, &nl->gates[i].output, 1U, &next_net_id);
			}
		}
	lxs_assign_net_group(net_remap, net_assigned, nl->outputs, nl->output_count, &next_net_id);
	for (uint32_t i = 0; i < nl->gate_count; ++i)
		{
		if (nl->gates[i].type == LXS_GATE_DFF && nl->gates[i].input_count > 0U)
			{
			lxs_assign_net_group(net_remap, net_assigned, &nl->gates[i].inputs[0], 1U, &next_net_id);
			}
		}
	lxs_assign_remaining_nets(net_remap, net_assigned, nl->net_count, &next_net_id);

	lxs_apply_net_remap_to_array(nl->inputs, nl->input_count, net_remap);
	lxs_apply_net_remap_to_array(nl->outputs, nl->output_count, net_remap);
	lxs_apply_net_remap_to_gates(nl->gates, nl->gate_count, net_remap);

	plan = lxs_calloc_aligned(1U, sizeof(lxs_plan));
	if (!plan)
		{
		lxs_free_aligned(comb_driver);
		lxs_free_aligned(level_cache);
		lxs_free_aligned(visiting);
		lxs_free_aligned(net_remap);
		lxs_free_aligned(net_assigned);
		lxs_free_aligned(matched_gates);
		return NULL;
		}

	plan->net_count = nl->net_count;
	plan->gate_count = nl->gate_count;
	plan->macro_count = lxs_collect_macros(nl, comb_driver, matched_gates, NULL);
	if (plan->macro_count == UINT32_MAX)
		{
		lxs_free_aligned(comb_driver);
		lxs_free_aligned(level_cache);
		lxs_free_aligned(visiting);
		lxs_free_aligned(net_remap);
		lxs_free_aligned(net_assigned);
		lxs_free_aligned(matched_gates);
		lxs_free_aligned(plan);
		return NULL;
		}
	plan->level_count = 0U;
	for (uint32_t i = 0; i < nl->gate_count; ++i)
		{
		if (nl->gates[i].type == LXS_GATE_DFF)
			{
			plan->state.count++;
			}
		else
			{
			if (!matched_gates[i])
				{
				plan->comb_gate_count++;
				}
			if ((nl->gates[i].level + 1U) > plan->level_count)
				{
				plan->level_count = nl->gates[i].level + 1U;
				}
			}
		}

	plan->inputs.count = nl->input_count;
	plan->outputs.count = nl->output_count;
	plan->inputs.net_ids = lxs_calloc_aligned(plan->inputs.count, sizeof(uint32_t));
	plan->outputs.net_ids = lxs_calloc_aligned(plan->outputs.count, sizeof(uint32_t));
	plan->state.d_inputs = lxs_calloc_aligned(plan->state.count, sizeof(uint32_t));
	plan->state.q_outputs = lxs_calloc_aligned(plan->state.count, sizeof(uint32_t));
	plan->levels = lxs_calloc_aligned(plan->level_count, sizeof(lxs_level_plan));
	plan->comb_gates = lxs_calloc_aligned(plan->comb_gate_count, sizeof(lxs_gate_ir));
	plan->chunks = lxs_calloc_aligned(plan->comb_gate_count, sizeof(lxs_chunk_plan));
	plan->macros = lxs_calloc_aligned(plan->macro_count, sizeof(lxs_macro_plan));

	if ((plan->inputs.count && !plan->inputs.net_ids) ||
		(plan->outputs.count && !plan->outputs.net_ids) ||
		(plan->state.count && (!plan->state.d_inputs || !plan->state.q_outputs)) ||
		(plan->level_count && !plan->levels) ||
		(plan->comb_gate_count && (!plan->comb_gates || !plan->chunks)) ||
		(plan->macro_count && !plan->macros))
		{
		lxs_free_plan(plan);
		lxs_free_aligned(comb_driver);
		lxs_free_aligned(level_cache);
		lxs_free_aligned(visiting);
		lxs_free_aligned(net_remap);
		lxs_free_aligned(net_assigned);
		lxs_free_aligned(matched_gates);
		return NULL;
		}

	memcpy(plan->inputs.net_ids, nl->inputs, (size_t)plan->inputs.count * sizeof(uint32_t));
	memcpy(plan->outputs.net_ids, nl->outputs, (size_t)plan->outputs.count * sizeof(uint32_t));
	plan->inputs.is_contiguous =
		lxs_find_contiguous_range(plan->inputs.net_ids, plan->inputs.count, &plan->inputs.contiguous_base);
	plan->outputs.is_contiguous =
		lxs_find_contiguous_range(plan->outputs.net_ids, plan->outputs.count, &plan->outputs.contiguous_base);

	for (uint32_t i = 0; i < nl->gate_count; ++i)
		{
		if (nl->gates[i].type == LXS_GATE_DFF)
			{
			plan->state.d_inputs[dff_fill] = nl->gates[i].input_count > 0U ? nl->gates[i].inputs[0] : 0U;
			plan->state.q_outputs[dff_fill] = nl->gates[i].output;
			dff_fill++;
			}
		}
	plan->state.d_is_contiguous =
		lxs_find_contiguous_range(plan->state.d_inputs, plan->state.count, &plan->state.d_contiguous_base);
	plan->state.q_is_contiguous =
		lxs_find_contiguous_range(plan->state.q_outputs, plan->state.count, &plan->state.q_contiguous_base);

	if (plan->macro_count > 0U)
		{
		memset(matched_gates, 0, (size_t)nl->gate_count * sizeof(uint8_t));
		macro_fill = lxs_collect_macros(nl, comb_driver, matched_gates, plan->macros);
		if (macro_fill == UINT32_MAX)
			{
			lxs_free_plan(plan);
			lxs_free_aligned(comb_driver);
			lxs_free_aligned(level_cache);
			lxs_free_aligned(visiting);
			lxs_free_aligned(net_remap);
			lxs_free_aligned(net_assigned);
			lxs_free_aligned(matched_gates);
			return NULL;
			}
		plan->macro_count = macro_fill;
		}

	for (uint32_t level = 0; level < plan->level_count; ++level)
		{
		uint32_t level_chunk_start = chunk_fill;
		uint32_t level_macro_start = macro_level_fill;

		for (uint32_t type = 0; type < (uint32_t)LXS_GATE_DFF; ++type)
			{
			uint32_t count = lxs_count_bucket_gates(nl, level, type);
			uint32_t primitive_count = 0U;
			if (count == 0U)
				{
				continue;
				}

			for (uint32_t i = 0; i < nl->gate_count; ++i)
				{
				if (nl->gates[i].type == type && nl->gates[i].level == level && !matched_gates[i])
					{
					plan->comb_gates[gate_fill + primitive_count] = nl->gates[i];
					primitive_count++;
					}
				}

			if (primitive_count == 0U)
				{
				continue;
				}

			plan->chunks[chunk_fill].level = level;
			plan->chunks[chunk_fill].type = type;
			plan->chunks[chunk_fill].start = gate_fill;
			plan->chunks[chunk_fill].count = primitive_count;
			chunk_fill++;

			lxs_sort_bucket_gates(plan->comb_gates + gate_fill, primitive_count, type);
			gate_fill += primitive_count;

			if (primitive_count > plan->max_span_count)
				{
				plan->max_span_count = primitive_count;
				}
			}

		plan->levels[level].chunk_start = level_chunk_start;
		plan->levels[level].chunk_count = chunk_fill - level_chunk_start;
		while (macro_level_fill < plan->macro_count && plan->macros[macro_level_fill].level == level)
			{
			macro_level_fill++;
			}
		plan->levels[level].macro_start = level_macro_start;
		plan->levels[level].macro_count = macro_level_fill - level_macro_start;
		}

	plan->span_count = chunk_fill;

	lxs_free_aligned(comb_driver);
	lxs_free_aligned(level_cache);
	lxs_free_aligned(visiting);
	lxs_free_aligned(net_remap);
	lxs_free_aligned(net_assigned);
	lxs_free_aligned(matched_gates);
	return plan;
	}

void lxs_free_plan(lxs_plan *plan)
	{
	if (!plan)
		{
		return;
		}

	lxs_free_aligned(plan->comb_gates);
	lxs_free_aligned(plan->chunks);
	lxs_free_aligned(plan->macros);
	lxs_free_aligned(plan->levels);
	lxs_free_aligned(plan->inputs.net_ids);
	lxs_free_aligned(plan->outputs.net_ids);
	lxs_free_aligned(plan->state.d_inputs);
	lxs_free_aligned(plan->state.q_outputs);
	lxs_free_aligned(plan);
	}
