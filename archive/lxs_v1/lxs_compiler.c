#include "lxs_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#define ALIGN_SIZE 64U
#define LXS_INITIAL_NET_CAP 256U
#define LXS_INITIAL_GATE_CAP 256U
#define LXS_MIN_PACKED_SLICE 64U
#define LXS_PACKED_SPAN_PENALTY 128U
#define LXS_PACKED_MIN_SCORE 128

static void* lxs_malloc_aligned(size_t size)
	{
	if (size == 0)
		{
		return NULL;
		}

#ifdef _WIN32
	return _aligned_malloc(size, ALIGN_SIZE);
#else
	void *ptr = NULL;
	if (posix_memalign(&ptr, ALIGN_SIZE, size) != 0)
		{
		return NULL;
		}
	return ptr;
#endif
	}

static void* lxs_calloc_aligned(size_t count, size_t size)
	{
	size_t total_size = count * size;
	void *ptr = lxs_malloc_aligned(total_size);
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
	void *new_ptr = lxs_malloc_aligned(new_size);
	if (!new_ptr)
		{
		return NULL;
		}

	memset(new_ptr, 0, new_size);
	if (ptr && old_size > 0)
		{
		size_t copy_size = old_size < new_size ? old_size : new_size;
		memcpy(new_ptr, ptr, copy_size);
		lxs_free_aligned(ptr);
		}
	return new_ptr;
	}

static char* lxs_strdup_local(const char *s)
	{
	size_t len = strlen(s) + 1U;
	char *copy = malloc(len);
	if (!copy)
		{
		return NULL;
		}
	memcpy(copy, s, len);
	return copy;
	}

static char* lxs_trim(char *s)
	{
	char *end;
	while (*s == ' ' || *s == '\t' || *s == '\n' || *s == '\r')
		{
		s++;
		}

	end = s + strlen(s);
	while (end > s)
		{
		char c = end[-1];
		if (c != ' ' && c != '\t' && c != '\n' && c != '\r')
			{
			break;
			}
		end--;
		}
	*end = '\0';
	return s;
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
		*cap = *cap == 0 ? LXS_INITIAL_NET_CAP : (*cap * 2U);
		}

	*items = lxs_realloc_aligned(*items, old_size, (size_t)(*cap) * sizeof(char*));
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
		*cap = *cap == 0 ? LXS_INITIAL_GATE_CAP : (*cap * 2U);
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
	size_t old_size;

	if (*count >= *cap)
		{
		old_size = (size_t)(*cap) * sizeof(uint32_t);
		*cap = *cap == 0 ? LXS_INITIAL_NET_CAP : (*cap * 2U);
		*items = lxs_realloc_aligned(*items, old_size, (size_t)(*cap) * sizeof(uint32_t));
		if (!*items)
			{
			return 0;
			}
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

static lxs_gate_type lxs_string_to_gate_type(const char *s)
	{
	if (strcmp(s, "AND") == 0) return LXS_GATE_AND;
	if (strcmp(s, "OR") == 0) return LXS_GATE_OR;
	if (strcmp(s, "XOR") == 0) return LXS_GATE_XOR;
	if (strcmp(s, "TRI") == 0) return LXS_GATE_TRI;
	if (strcmp(s, "NOT") == 0) return LXS_GATE_NOT;
	if (strcmp(s, "NAND") == 0) return LXS_GATE_NAND;
	if (strcmp(s, "NOR") == 0) return LXS_GATE_NOR;
	if (strcmp(s, "XNOR") == 0) return LXS_GATE_XNOR;
	if (strcmp(s, "BUF") == 0 || strcmp(s, "BUFF") == 0) return LXS_GATE_BUF;
	if (strcmp(s, "DFF") == 0) return LXS_GATE_DFF;
	return LXS_GATE_BUF;
	}

static uint32_t lxs_gate_level(
	uint32_t gate_index,
	const lxs_netlist *nl,
	const int32_t *comb_driver,
	uint32_t *cache,
	uint8_t *visiting)
	{
	lxs_gate_ir *gate = &nl->gates[gate_index];
	uint32_t max_level = 0;

	if (cache[gate_index] != UINT32_MAX)
		{
		return cache[gate_index];
		}

	if (visiting[gate_index])
		{
		return 0;
		}

	visiting[gate_index] = 1U;
	for (uint32_t i = 0; i < gate->input_count; ++i)
		{
		int32_t driver = comb_driver[gate->inputs[i]];
		if (driver >= 0)
			{
			uint32_t level = lxs_gate_level((uint32_t)driver, nl, comb_driver, cache, visiting) + 1U;
			if (level > max_level)
				{
				max_level = level;
				}
			}
		}
	visiting[gate_index] = 0U;
	cache[gate_index] = max_level;
	return max_level;
	}

static const lxs_gate_ir *g_lxs_sort_gates = NULL;
static uint32_t g_lxs_sort_primary_input = 0U;
static uint32_t g_lxs_sort_secondary_input = 1U;

static uint32_t lxs_gate_input_key(const lxs_gate_ir *gate, uint32_t input_slot)
	{
	return input_slot < gate->input_count ? gate->inputs[input_slot] : 0U;
	}

static int __cdecl lxs_compare_gate_indices_by_inputs(const void *lhs, const void *rhs)
	{
	const uint32_t lhs_index = *(const uint32_t*)lhs;
	const uint32_t rhs_index = *(const uint32_t*)rhs;
	const lxs_gate_ir *lhs_gate = &g_lxs_sort_gates[lhs_index];
	const lxs_gate_ir *rhs_gate = &g_lxs_sort_gates[rhs_index];
	uint32_t lhs_primary = lxs_gate_input_key(lhs_gate, g_lxs_sort_primary_input);
	uint32_t rhs_primary = lxs_gate_input_key(rhs_gate, g_lxs_sort_primary_input);
	uint32_t lhs_secondary = lxs_gate_input_key(lhs_gate, g_lxs_sort_secondary_input);
	uint32_t rhs_secondary = lxs_gate_input_key(rhs_gate, g_lxs_sort_secondary_input);

	if (lhs_primary != rhs_primary)
		{
		return lhs_primary < rhs_primary ? -1 : 1;
		}

	if (lhs_secondary != rhs_secondary)
		{
		return lhs_secondary < rhs_secondary ? -1 : 1;
		}

	if (lhs_gate->output != rhs_gate->output)
		{
		return lhs_gate->output < rhs_gate->output ? -1 : 1;
		}

	if (lhs_index != rhs_index)
		{
		return lhs_index < rhs_index ? -1 : 1;
		}

	return 0;
	}

static uint32_t lxs_count_copy_runs_for_input(
	const lxs_gate_ir *gates,
	const uint32_t *indices,
	uint32_t count,
	uint32_t input_slot)
	{
	uint32_t runs = 0;
	uint32_t previous = 0U;

	for (uint32_t i = 0; i < count; ++i)
		{
		uint32_t current = lxs_gate_input_key(&gates[indices[i]], input_slot);
		if (i == 0 || current != (previous + 1U))
			{
			runs++;
			}
		previous = current;
		}

	return runs;
	}

static uint32_t lxs_score_gate_order(
	const lxs_gate_ir *gates,
	const uint32_t *indices,
	uint32_t count,
	uint32_t gate_type)
	{
	uint32_t score = lxs_count_copy_runs_for_input(gates, indices, count, 0U);
	if (gate_type != LXS_GATE_NOT && gate_type != LXS_GATE_BUF)
		{
		score += lxs_count_copy_runs_for_input(gates, indices, count, 1U);
		}
	return score;
	}

static int32_t lxs_score_packed_order(
	const lxs_gate_ir *gates,
	const uint32_t *indices,
	uint32_t count,
	uint32_t gate_type);

static int lxs_reorder_gates_for_execution(lxs_netlist *nl, uint32_t level_count)
	{
	lxs_gate_ir *reordered_gates;
	uint32_t next_index = 0;

	reordered_gates = lxs_calloc_aligned(nl->gate_count, sizeof(lxs_gate_ir));
	if (!reordered_gates)
		{
		return 0;
		}

	for (uint32_t level = 0; level < level_count; ++level)
		{
		for (uint32_t type = 0; type < (uint32_t)LXS_GATE_DFF; ++type)
			{
			uint32_t count = 0;
			uint32_t *original_indices;
			uint32_t *sorted_primary;
			uint32_t *sorted_secondary;
			const uint32_t *chosen_order;
			uint32_t original_score;
			uint32_t primary_score;
			uint32_t secondary_score;
			int32_t original_packed_score;
			int32_t primary_packed_score;
			int32_t secondary_packed_score;

			for (uint32_t i = 0; i < nl->gate_count; ++i)
				{
				if (nl->gates[i].type == type && nl->gates[i].level == level)
					{
					count++;
					}
				}

			if (count == 0)
				{
				continue;
				}

			original_indices = lxs_calloc_aligned(count, sizeof(uint32_t));
			sorted_primary = lxs_calloc_aligned(count, sizeof(uint32_t));
			sorted_secondary = lxs_calloc_aligned(count, sizeof(uint32_t));
			if (!original_indices || !sorted_primary || !sorted_secondary)
				{
				lxs_free_aligned(original_indices);
				lxs_free_aligned(sorted_primary);
				lxs_free_aligned(sorted_secondary);
				lxs_free_aligned(reordered_gates);
				return 0;
				}

			count = 0;
			for (uint32_t i = 0; i < nl->gate_count; ++i)
				{
				if (nl->gates[i].type == type && nl->gates[i].level == level)
					{
					original_indices[count] = i;
					sorted_primary[count] = i;
					sorted_secondary[count] = i;
					count++;
					}
				}

			original_score = lxs_score_gate_order(nl->gates, original_indices, count, type);
			original_packed_score = lxs_score_packed_order(nl->gates, original_indices, count, type);
			chosen_order = original_indices;

			g_lxs_sort_gates = nl->gates;
			g_lxs_sort_primary_input = 0U;
			g_lxs_sort_secondary_input = 1U;
			qsort(sorted_primary, count, sizeof(uint32_t), lxs_compare_gate_indices_by_inputs);
			primary_score = lxs_score_gate_order(nl->gates, sorted_primary, count, type);
			primary_packed_score = lxs_score_packed_order(nl->gates, sorted_primary, count, type);

			secondary_score = UINT32_MAX;
			secondary_packed_score = INT32_MIN;
			if (type != LXS_GATE_NOT && type != LXS_GATE_BUF)
				{
				g_lxs_sort_primary_input = 1U;
				g_lxs_sort_secondary_input = 0U;
				qsort(sorted_secondary, count, sizeof(uint32_t), lxs_compare_gate_indices_by_inputs);
				secondary_score = lxs_score_gate_order(nl->gates, sorted_secondary, count, type);
				secondary_packed_score = lxs_score_packed_order(nl->gates, sorted_secondary, count, type);
				}
			g_lxs_sort_gates = NULL;

			if (primary_packed_score > original_packed_score ||
				(primary_packed_score == original_packed_score && primary_score < original_score))
				{
				chosen_order = sorted_primary;
				original_score = primary_score;
				original_packed_score = primary_packed_score;
				}

			if (secondary_packed_score > original_packed_score ||
				(secondary_packed_score == original_packed_score && secondary_score < original_score))
				{
				chosen_order = sorted_secondary;
				}

			for (uint32_t i = 0; i < count; ++i)
				{
				reordered_gates[next_index++] = nl->gates[chosen_order[i]];
				}

			lxs_free_aligned(original_indices);
			lxs_free_aligned(sorted_primary);
			lxs_free_aligned(sorted_secondary);
			}
		}

	for (uint32_t i = 0; i < nl->gate_count; ++i)
		{
		if (nl->gates[i].type == LXS_GATE_DFF)
			{
			reordered_gates[next_index++] = nl->gates[i];
			}
		}

	memcpy(nl->gates, reordered_gates, (size_t)nl->gate_count * sizeof(lxs_gate_ir));
	lxs_free_aligned(reordered_gates);
	return 1;
	}

static int lxs_assign_remap_id(uint32_t *remap, uint32_t old_id, uint32_t *next_id)
	{
	if (remap[old_id] == UINT32_MAX)
		{
		remap[old_id] = (*next_id)++;
		}
	return 1;
	}

static int lxs_reorder_net_names(lxs_netlist *nl, const uint32_t *remap)
	{
	char **new_names = lxs_calloc_aligned(nl->net_count, sizeof(char*));
	if (!new_names)
		{
		return 0;
		}

	for (uint32_t old_id = 0; old_id < nl->net_count; ++old_id)
		{
		new_names[remap[old_id]] = nl->net_names[old_id];
		}

	lxs_free_aligned(nl->net_names);
	nl->net_names = new_names;
	return 1;
	}

static int lxs_remap_netlist_for_execution(lxs_netlist *nl)
	{
	uint32_t *remap = lxs_calloc_aligned(nl->net_count, sizeof(uint32_t));
	uint32_t next_id = 0;

	if (!remap)
		{
		return 0;
		}

	for (uint32_t i = 0; i < nl->net_count; ++i)
		{
		remap[i] = UINT32_MAX;
		}

	for (uint32_t i = 0; i < nl->input_count; ++i)
		{
		lxs_assign_remap_id(remap, nl->inputs[i], &next_id);
		}

	for (uint32_t i = 0; i < nl->gate_count; ++i)
		{
		if (nl->gates[i].type == LXS_GATE_DFF)
			{
			lxs_assign_remap_id(remap, nl->gates[i].output, &next_id);
			}
		}

	for (uint32_t level = 0; level < nl->gate_count; ++level)
		{
		for (uint32_t type = 0; type < (uint32_t)LXS_GATE_DFF; ++type)
			{
			for (uint32_t i = 0; i < nl->gate_count; ++i)
				{
				if (nl->gates[i].type == type && nl->gates[i].level == level)
					{
					lxs_assign_remap_id(remap, nl->gates[i].output, &next_id);
					}
				}
			}
		}

	for (uint32_t i = 0; i < nl->output_count; ++i)
		{
		lxs_assign_remap_id(remap, nl->outputs[i], &next_id);
		}

	for (uint32_t i = 0; i < nl->net_count; ++i)
		{
		lxs_assign_remap_id(remap, i, &next_id);
		}

	if (!lxs_reorder_net_names(nl, remap))
		{
		lxs_free_aligned(remap);
		return 0;
		}

	for (uint32_t i = 0; i < nl->input_count; ++i)
		{
		nl->inputs[i] = remap[nl->inputs[i]];
		}

	for (uint32_t i = 0; i < nl->output_count; ++i)
		{
		nl->outputs[i] = remap[nl->outputs[i]];
		}

	for (uint32_t i = 0; i < nl->gate_count; ++i)
		{
		for (uint32_t j = 0; j < nl->gates[i].input_count; ++j)
			{
			nl->gates[i].inputs[j] = remap[nl->gates[i].inputs[j]];
			}
		nl->gates[i].output = remap[nl->gates[i].output];
		}

	lxs_free_aligned(remap);
	return 1;
	}

static uint32_t lxs_detect_contiguous_base(const uint32_t *items, uint32_t count, uint32_t *base_out)
	{
	uint32_t base;

	if (count == 0)
		{
		*base_out = 0U;
		return 0U;
		}

	base = items[0];
	for (uint32_t i = 1; i < count; ++i)
		{
		if (items[i] != (base + i))
			{
			*base_out = 0U;
			return 0U;
			}
		}

	*base_out = base;
	return 1U;
	}

static uint32_t lxs_count_copy_runs_slice(
	const uint32_t *items,
	uint32_t start,
	uint32_t count)
	{
	uint32_t runs = 0;
	uint32_t previous = 0U;

	for (uint32_t i = 0; i < count; ++i)
		{
		uint32_t current = items[start + i];
		if (i == 0U || current != (previous + 1U))
			{
			runs++;
			}
		previous = current;
		}

	return runs;
	}

static uint32_t lxs_detect_contiguous_base_slice(
	const uint32_t *items,
	uint32_t start,
	uint32_t count,
	uint32_t *base_out)
	{
	return lxs_detect_contiguous_base(items + start, count, base_out);
	}

static uint32_t lxs_should_pack_slice(
	uint32_t type,
	const uint32_t *src_a,
	const uint32_t *src_b,
	uint32_t start,
	uint32_t count)
	{
	uint32_t unary = (type == LXS_GATE_NOT || type == LXS_GATE_BUF);
	uint32_t src_a_base = 0U;
	uint32_t src_b_base = 0U;
	uint32_t src_a_contig;
	uint32_t src_b_contig = 0U;
	uint32_t src_a_runs;
	uint32_t src_b_runs = 0U;

	if (count < 16U)
		{
		return 0U;
		}

	src_a_contig = lxs_detect_contiguous_base_slice(src_a, start, count, &src_a_base);
	src_a_runs = src_a_contig ? 0U : lxs_count_copy_runs_slice(src_a, start, count);

	if (unary)
		{
		return !src_a_contig && src_a_runs > 0U && (src_a_runs * 4U) <= count;
		}

	src_b_contig = lxs_detect_contiguous_base_slice(src_b, start, count, &src_b_base);
	src_b_runs = src_b_contig ? 0U : lxs_count_copy_runs_slice(src_b, start, count);

	if (!src_a_contig && (src_a_runs == 0U || (src_a_runs * 4U) > count))
		{
		return 0U;
		}

	if (!src_b_contig && (src_b_runs == 0U || (src_b_runs * 4U) > count))
		{
		return 0U;
		}

	return !(src_a_contig && src_b_contig);
	}

static uint32_t lxs_plan_span_slices(
	uint32_t type,
	const uint32_t *src_a,
	const uint32_t *src_b,
	uint32_t count,
	uint32_t *slice_starts,
	uint32_t *slice_counts)
	{
	uint32_t best_start = UINT32_MAX;
	uint32_t best_count = 0U;
	int32_t best_score = 0;
	uint32_t slice_count = 0U;

	for (uint32_t start = 0U; start < count; ++start)
		{
		uint32_t remaining = count - start;
		if (remaining < LXS_MIN_PACKED_SLICE)
			{
			break;
			}

		for (uint32_t length = remaining; length >= LXS_MIN_PACKED_SLICE; --length)
			{
			if (lxs_should_pack_slice(type, src_a, src_b, start, length))
				{
				uint32_t extra_spans = 0U;
				int32_t score;

				if (start > 0U)
					{
					extra_spans++;
					}

				if ((start + length) < count)
					{
					extra_spans++;
					}

				score = (int32_t)length - (int32_t)(extra_spans * LXS_PACKED_SPAN_PENALTY);
				if (score > best_score)
					{
					best_score = score;
					best_start = start;
					best_count = length;
					}
				}
			}
		}

	if (best_score < LXS_PACKED_MIN_SCORE || best_count == 0U)
		{
		slice_starts[0] = 0U;
		slice_counts[0] = count;
		return 1U;
		}

	if (best_start > 0U)
		{
		slice_starts[slice_count] = 0U;
		slice_counts[slice_count] = best_start;
		slice_count++;
		}

	slice_starts[slice_count] = best_start;
	slice_counts[slice_count] = best_count;
	slice_count++;

	if ((best_start + best_count) < count)
		{
		slice_starts[slice_count] = best_start + best_count;
		slice_counts[slice_count] = count - (best_start + best_count);
		slice_count++;
		}

	return slice_count;
	}

static int32_t lxs_score_packed_order(
	const lxs_gate_ir *gates,
	const uint32_t *indices,
	uint32_t count,
	uint32_t gate_type)
	{
	uint32_t *src_a;
	uint32_t *src_b;
	uint32_t *slice_starts;
	uint32_t *slice_counts;
	uint32_t slice_count;
	int32_t best_score = 0;

	src_a = lxs_calloc_aligned(count, sizeof(uint32_t));
	src_b = lxs_calloc_aligned(count, sizeof(uint32_t));
	slice_starts = lxs_calloc_aligned(count, sizeof(uint32_t));
	slice_counts = lxs_calloc_aligned(count, sizeof(uint32_t));
	if (!src_a || !src_b || !slice_starts || !slice_counts)
		{
		lxs_free_aligned(src_a);
		lxs_free_aligned(src_b);
		lxs_free_aligned(slice_starts);
		lxs_free_aligned(slice_counts);
		return INT32_MIN;
		}

	for (uint32_t i = 0; i < count; ++i)
		{
		const lxs_gate_ir *gate = &gates[indices[i]];
		src_a[i] = gate->input_count > 0U ? gate->inputs[0] : 0U;
		src_b[i] = gate->input_count > 1U ? gate->inputs[1] : 0U;
		}

	slice_count = lxs_plan_span_slices(gate_type, src_a, src_b, count, slice_starts, slice_counts);
	for (uint32_t i = 0; i < slice_count; ++i)
		{
		if (lxs_should_pack_slice(gate_type, src_a, src_b, slice_starts[i], slice_counts[i]))
			{
			uint32_t extra_spans = slice_count - 1U;
			int32_t score = (int32_t)slice_counts[i] - (int32_t)(extra_spans * LXS_PACKED_SPAN_PENALTY);
			if (score > best_score)
				{
				best_score = score;
				}
			}
		}

	lxs_free_aligned(src_a);
	lxs_free_aligned(src_b);
	lxs_free_aligned(slice_starts);
	lxs_free_aligned(slice_counts);
	return best_score;
	}

static int lxs_build_copy_runs(
	const uint32_t *ids,
	uint32_t count,
	uint32_t *run_count_out,
	lxs_copy_run *restrict *runs_out)
	{
	uint32_t run_count = 0;
	uint32_t span_base = 0;
	lxs_copy_run *runs = NULL;

	if (count == 0)
		{
		*run_count_out = 0;
		*runs_out = NULL;
		return 1;
		}

	while (span_base < count)
		{
		uint32_t length = 1;
		while ((span_base + length) < count &&
			ids[span_base + length] == (ids[span_base] + length))
			{
			length++;
			}
		run_count++;
		span_base += length;
		}

	runs = lxs_calloc_aligned(run_count, sizeof(lxs_copy_run));
	if (!runs)
		{
		return 0;
		}

	run_count = 0;
	span_base = 0;
	while (span_base < count)
		{
		uint32_t length = 1;
		while ((span_base + length) < count &&
			ids[span_base + length] == (ids[span_base] + length))
			{
			length++;
			}

		runs[run_count].net_base = ids[span_base];
		runs[run_count].span_base = span_base;
		runs[run_count].count = length;
		run_count++;
		span_base += length;
		}

	*run_count_out = run_count;
	*runs_out = runs;
	return 1;
	}

static uint32_t lxs_should_pack_span(const lxs_span_plan *span)
	{
	uint32_t unary = (span->type == LXS_GATE_NOT || span->type == LXS_GATE_BUF);
	uint32_t src_a_ready;
	uint32_t src_b_ready;

	if ((span->layout_flags & LXS_SPAN_DST_CONTIG) == 0U || span->count < 16U)
		{
		return 0U;
		}

	src_a_ready =
		((span->layout_flags & LXS_SPAN_SRC_A_CONTIG) != 0U) ||
		(span->src_a_run_count > 0U && (span->src_a_run_count * 4U) <= span->count);

	if (unary)
		{
		return src_a_ready && ((span->layout_flags & LXS_SPAN_SRC_A_CONTIG) == 0U);
		}

	src_b_ready =
		((span->layout_flags & LXS_SPAN_SRC_B_CONTIG) != 0U) ||
		(span->src_b_run_count > 0U && (span->src_b_run_count * 4U) <= span->count);

	if (!src_a_ready || !src_b_ready)
		{
		return 0U;
		}

	if ((span->layout_flags & LXS_SPAN_SRC_A_CONTIG) != 0U &&
		(span->layout_flags & LXS_SPAN_SRC_B_CONTIG) != 0U)
		{
		return 0U;
		}

	return 1U;
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

static void lxs_collect_bucket_signals(
	const lxs_netlist *nl,
	uint32_t level,
	uint32_t type,
	uint32_t *src_a,
	uint32_t *src_b,
	uint32_t *dst)
	{
	uint32_t fill = 0U;

	for (uint32_t i = 0; i < nl->gate_count; ++i)
		{
		const lxs_gate_ir *gate = &nl->gates[i];
		if (gate->type == type && gate->level == level)
			{
			src_a[fill] = gate->input_count > 0U ? gate->inputs[0] : 0U;
			src_b[fill] = gate->input_count > 1U ? gate->inputs[1] : 0U;
			dst[fill] = gate->output;
			fill++;
			}
		}
	}

static int lxs_init_span_from_slice(
	lxs_span_plan *span,
	uint32_t type,
	const uint32_t *src_a,
	const uint32_t *src_b,
	const uint32_t *dst,
	uint32_t start,
	uint32_t count)
	{
	memset(span, 0, sizeof(*span));
	span->type = type;
	span->count = count;
	span->src_a = lxs_calloc_aligned(count, sizeof(uint32_t));
	span->src_b = lxs_calloc_aligned(count, sizeof(uint32_t));
	span->dst = lxs_calloc_aligned(count, sizeof(uint32_t));
	if (!span->src_a || !span->src_b || !span->dst)
		{
		return 0;
		}

	memcpy(span->src_a, src_a + start, (size_t)count * sizeof(uint32_t));
	memcpy(span->src_b, src_b + start, (size_t)count * sizeof(uint32_t));
	memcpy(span->dst, dst + start, (size_t)count * sizeof(uint32_t));

	if (lxs_detect_contiguous_base(span->src_a, span->count, &span->src_a_base))
		{
		span->layout_flags |= LXS_SPAN_SRC_A_CONTIG;
		}

	if (lxs_detect_contiguous_base(span->dst, span->count, &span->dst_base))
		{
		span->layout_flags |= LXS_SPAN_DST_CONTIG;
		}

	if (span->type != LXS_GATE_NOT && span->type != LXS_GATE_BUF)
		{
		if (lxs_detect_contiguous_base(span->src_b, span->count, &span->src_b_base))
			{
			span->layout_flags |= LXS_SPAN_SRC_B_CONTIG;
			}
		}

	if ((span->layout_flags & LXS_SPAN_SRC_A_CONTIG) == 0U)
		{
		if (!lxs_build_copy_runs(span->src_a, span->count, &span->src_a_run_count, &span->src_a_runs))
			{
			return 0;
			}
		}

	if ((span->type != LXS_GATE_NOT && span->type != LXS_GATE_BUF) &&
		(span->layout_flags & LXS_SPAN_SRC_B_CONTIG) == 0U)
		{
		if (!lxs_build_copy_runs(span->src_b, span->count, &span->src_b_run_count, &span->src_b_runs))
			{
			return 0;
			}
		}

	if (lxs_should_pack_span(span))
		{
		span->layout_flags |= LXS_SPAN_PACKED;
		}

	return 1;
	}

static void lxs_free_span(lxs_span_plan *span)
	{
	lxs_free_aligned(span->src_a);
	lxs_free_aligned(span->src_b);
	lxs_free_aligned(span->dst);
	lxs_free_aligned(span->src_a_runs);
	lxs_free_aligned(span->src_b_runs);
	span->src_a = NULL;
	span->src_b = NULL;
	span->dst = NULL;
	span->src_a_runs = NULL;
	span->src_b_runs = NULL;
	span->count = 0;
	}

lxs_netlist* lxs_load_iscas(const char *path)
	{
	FILE *f = fopen(path, "r");
	char line[1024];
	lxs_netlist *nl;

	if (!f)
		{
		return NULL;
		}

	nl = lxs_calloc_aligned(1U, sizeof(lxs_netlist));
	if (!nl)
		{
		fclose(f);
		return NULL;
		}

	while (fgets(line, sizeof(line), f))
		{
		char *trimmed = lxs_trim(line);
		char *eq;

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
				fclose(f);
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
				fclose(f);
				return NULL;
				}
			continue;
			}

		eq = strchr(trimmed, '=');
		if (!eq)
			{
			continue;
			}

		*eq = '\0';
		{
		char *out_name = lxs_trim(trimmed);
		char *gate_part = lxs_trim(eq + 1);
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
			fclose(f);
			return NULL;
			}

		{
		char *ctx = NULL;
		char *tok = strtok_s(open_paren + 1, ",", &ctx);
		while (tok && gate.input_count < 2U)
			{
			uint32_t id = lxs_intern_net(nl, lxs_trim(tok));
			if (id == UINT32_MAX)
				{
				lxs_free_netlist(nl);
				fclose(f);
				return NULL;
				}
			gate.inputs[gate.input_count++] = id;
			tok = strtok_s(NULL, ",", &ctx);
			}
		}

		if (!lxs_push_gate(nl, &gate))
			{
			lxs_free_netlist(nl);
			fclose(f);
			return NULL;
			}
		}
		}

	fclose(f);
	return nl;
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
	lxs_plan *plan;
	uint32_t comb_gate_count = 0;
	uint32_t dff_count = 0;
	uint32_t max_level = 0;

	if (!nl)
		{
		return NULL;
		}

	comb_driver = lxs_calloc_aligned(nl->net_count, sizeof(int32_t));
	level_cache = lxs_calloc_aligned(nl->gate_count, sizeof(uint32_t));
	visiting = lxs_calloc_aligned(nl->gate_count, sizeof(uint8_t));
	if (!comb_driver || !level_cache || !visiting)
		{
		lxs_free_aligned(comb_driver);
		lxs_free_aligned(level_cache);
		lxs_free_aligned(visiting);
		return NULL;
		}

	for (uint32_t i = 0; i < nl->net_count; ++i)
		{
		comb_driver[i] = -1;
		}
	for (uint32_t i = 0; i < nl->gate_count; ++i)
		{
		level_cache[i] = UINT32_MAX;
		if (nl->gates[i].type == LXS_GATE_DFF)
			{
			dff_count++;
			}
		else
			{
			comb_driver[nl->gates[i].output] = (int32_t)i;
			comb_gate_count++;
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

	if (!lxs_reorder_gates_for_execution(nl, max_level + 1U))
		{
		lxs_free_aligned(comb_driver);
		lxs_free_aligned(level_cache);
		lxs_free_aligned(visiting);
		return NULL;
		}

	if (!lxs_remap_netlist_for_execution(nl))
		{
		lxs_free_aligned(comb_driver);
		lxs_free_aligned(level_cache);
		lxs_free_aligned(visiting);
		return NULL;
		}

	plan = lxs_calloc_aligned(1U, sizeof(lxs_plan));
	if (!plan)
		{
		lxs_free_aligned(comb_driver);
		lxs_free_aligned(level_cache);
		lxs_free_aligned(visiting);
		return NULL;
		}

	plan->net_count = nl->net_count;
	plan->gate_count = nl->gate_count;
	plan->comb_gate_count = comb_gate_count;
	plan->level_count = comb_gate_count > 0 ? (max_level + 1U) : 0U;
	plan->levels = lxs_calloc_aligned(plan->level_count, sizeof(lxs_level_plan));
	plan->inputs.count = nl->input_count;
	plan->inputs.net_ids = lxs_calloc_aligned(nl->input_count, sizeof(uint32_t));
	plan->outputs.count = nl->output_count;
	plan->outputs.net_ids = lxs_calloc_aligned(nl->output_count, sizeof(uint32_t));
	plan->state.count = dff_count;
	plan->state.d_inputs = lxs_calloc_aligned(dff_count, sizeof(uint32_t));
	plan->state.q_outputs = lxs_calloc_aligned(dff_count, sizeof(uint32_t));

	if ((nl->input_count && !plan->inputs.net_ids) ||
		(nl->output_count && !plan->outputs.net_ids) ||
		(dff_count && (!plan->state.d_inputs || !plan->state.q_outputs)))
		{
		lxs_free_plan(plan);
		lxs_free_aligned(comb_driver);
		lxs_free_aligned(level_cache);
		lxs_free_aligned(visiting);
		return NULL;
		}

	memcpy(plan->inputs.net_ids, nl->inputs, (size_t)nl->input_count * sizeof(uint32_t));
	memcpy(plan->outputs.net_ids, nl->outputs, (size_t)nl->output_count * sizeof(uint32_t));

	if (plan->level_count > 0)
		{
		uint32_t *span_counts = lxs_calloc_aligned(plan->level_count, sizeof(uint32_t));
		if (!span_counts)
			{
			lxs_free_plan(plan);
			lxs_free_aligned(comb_driver);
			lxs_free_aligned(level_cache);
			lxs_free_aligned(visiting);
			return NULL;
			}

		for (uint32_t level = 0; level < plan->level_count; ++level)
			{
			for (uint32_t type = 0; type < (uint32_t)LXS_GATE_DFF; ++type)
				{
				uint32_t count = lxs_count_bucket_gates(nl, level, type);

				if (count > 0)
					{
					uint32_t *src_a = lxs_calloc_aligned(count, sizeof(uint32_t));
					uint32_t *src_b = lxs_calloc_aligned(count, sizeof(uint32_t));
					uint32_t *dst = lxs_calloc_aligned(count, sizeof(uint32_t));
					uint32_t *slice_starts = lxs_calloc_aligned(count, sizeof(uint32_t));
					uint32_t *slice_counts = lxs_calloc_aligned(count, sizeof(uint32_t));
					uint32_t slice_count;

					if (!src_a || !src_b || !dst || !slice_starts || !slice_counts)
						{
						lxs_free_aligned(src_a);
						lxs_free_aligned(src_b);
						lxs_free_aligned(dst);
						lxs_free_aligned(slice_starts);
						lxs_free_aligned(slice_counts);
						lxs_free_aligned(span_counts);
						lxs_free_plan(plan);
						lxs_free_aligned(comb_driver);
						lxs_free_aligned(level_cache);
						lxs_free_aligned(visiting);
						return NULL;
						}

					lxs_collect_bucket_signals(nl, level, type, src_a, src_b, dst);
					slice_count = lxs_plan_span_slices(type, src_a, src_b, count, slice_starts, slice_counts);
					span_counts[level] += slice_count;
					plan->span_count += slice_count;
					for (uint32_t slice = 0; slice < slice_count; ++slice)
						{
						if (slice_counts[slice] > plan->max_span_count)
							{
							plan->max_span_count = slice_counts[slice];
							}
						}

					lxs_free_aligned(src_a);
					lxs_free_aligned(src_b);
					lxs_free_aligned(dst);
					lxs_free_aligned(slice_starts);
					lxs_free_aligned(slice_counts);
					}
				}

			plan->levels[level].span_count = span_counts[level];
			plan->levels[level].spans = lxs_calloc_aligned(span_counts[level], sizeof(lxs_span_plan));
			if (span_counts[level] > 0 && !plan->levels[level].spans)
				{
				lxs_free_aligned(span_counts);
				lxs_free_plan(plan);
				lxs_free_aligned(comb_driver);
				lxs_free_aligned(level_cache);
				lxs_free_aligned(visiting);
				return NULL;
				}
			}

		for (uint32_t level = 0; level < plan->level_count; ++level)
			{
			uint32_t span_index = 0;
			for (uint32_t type = 0; type < (uint32_t)LXS_GATE_DFF; ++type)
				{
				uint32_t count = lxs_count_bucket_gates(nl, level, type);
				uint32_t *src_a;
				uint32_t *src_b;
				uint32_t *dst;
				uint32_t *slice_starts;
				uint32_t *slice_counts;
				uint32_t slice_count;

				if (count == 0)
					{
					continue;
					}

				src_a = lxs_calloc_aligned(count, sizeof(uint32_t));
				src_b = lxs_calloc_aligned(count, sizeof(uint32_t));
				dst = lxs_calloc_aligned(count, sizeof(uint32_t));
				slice_starts = lxs_calloc_aligned(count, sizeof(uint32_t));
				slice_counts = lxs_calloc_aligned(count, sizeof(uint32_t));
				if (!src_a || !src_b || !dst || !slice_starts || !slice_counts)
					{
					lxs_free_aligned(src_a);
					lxs_free_aligned(src_b);
					lxs_free_aligned(dst);
					lxs_free_aligned(slice_starts);
					lxs_free_aligned(slice_counts);
					lxs_free_aligned(span_counts);
					lxs_free_plan(plan);
					lxs_free_aligned(comb_driver);
					lxs_free_aligned(level_cache);
					lxs_free_aligned(visiting);
					return NULL;
					}

				lxs_collect_bucket_signals(nl, level, type, src_a, src_b, dst);
				slice_count = lxs_plan_span_slices(type, src_a, src_b, count, slice_starts, slice_counts);

				for (uint32_t slice = 0; slice < slice_count; ++slice)
					{
					lxs_span_plan *span = &plan->levels[level].spans[span_index++];
					if (!lxs_init_span_from_slice(
						span,
						type,
						src_a,
						src_b,
						dst,
						slice_starts[slice],
						slice_counts[slice]))
						{
						lxs_free_aligned(src_a);
						lxs_free_aligned(src_b);
						lxs_free_aligned(dst);
						lxs_free_aligned(slice_starts);
						lxs_free_aligned(slice_counts);
						lxs_free_aligned(span_counts);
						lxs_free_plan(plan);
						lxs_free_aligned(comb_driver);
						lxs_free_aligned(level_cache);
						lxs_free_aligned(visiting);
						return NULL;
						}
					}

				lxs_free_aligned(src_a);
				lxs_free_aligned(src_b);
				lxs_free_aligned(dst);
				lxs_free_aligned(slice_starts);
				lxs_free_aligned(slice_counts);
				}
			}

		lxs_free_aligned(span_counts);
		}

	{
	uint32_t dff_index = 0;
	for (uint32_t i = 0; i < nl->gate_count; ++i)
		{
		if (nl->gates[i].type == LXS_GATE_DFF)
			{
			plan->state.d_inputs[dff_index] = nl->gates[i].input_count > 0 ? nl->gates[i].inputs[0] : 0U;
			plan->state.q_outputs[dff_index] = nl->gates[i].output;
			dff_index++;
			}
		}
	}

	lxs_free_aligned(comb_driver);
	lxs_free_aligned(level_cache);
	lxs_free_aligned(visiting);
	return plan;
	}

void lxs_free_plan(lxs_plan *plan)
	{
	if (!plan)
		{
		return;
		}

	for (uint32_t level = 0; level < plan->level_count; ++level)
		{
		for (uint32_t span = 0; span < plan->levels[level].span_count; ++span)
			{
			lxs_free_span(&plan->levels[level].spans[span]);
			}
		lxs_free_aligned(plan->levels[level].spans);
		}

	lxs_free_aligned(plan->levels);
	lxs_free_aligned(plan->inputs.net_ids);
	lxs_free_aligned(plan->outputs.net_ids);
	lxs_free_aligned(plan->state.d_inputs);
	lxs_free_aligned(plan->state.q_outputs);
	lxs_free_aligned(plan);
	}
