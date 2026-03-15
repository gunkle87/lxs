#include "lxs_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <time.h>

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

static int lxs_reserve_u64(uint64_t **items, uint32_t *cap, uint32_t needed)
	{
	size_t old_size;

	if (needed <= *cap)
		{
		return 1;
		}

	old_size = (size_t)(*cap) * sizeof(uint64_t);
	while (*cap < needed)
		{
		*cap = (*cap == 0U) ? LXS_INITIAL_CAP : (*cap * 2U);
		}

	*items = lxs_realloc_aligned(*items, old_size, (size_t)(*cap) * sizeof(uint64_t));
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

static int lxs_reserve_source_multi_macros(
	lxs_source_multi_macro **items,
	uint32_t *cap,
	uint32_t needed)
	{
	size_t old_size;

	if (needed <= *cap)
		{
		return 1;
		}

	old_size = (size_t)(*cap) * sizeof(lxs_source_multi_macro);
	while (*cap < needed)
		{
		*cap = (*cap == 0U) ? LXS_INITIAL_CAP : (*cap * 2U);
		}

	*items = lxs_realloc_aligned(
		*items,
		old_size,
		(size_t)(*cap) * sizeof(lxs_source_multi_macro));
	return *items != NULL;
	}

static int lxs_reserve_source_standard_muxes(
	lxs_source_standard_mux **items,
	uint32_t *cap,
	uint32_t needed)
	{
	size_t old_size;

	if (needed <= *cap)
		{
		return 1;
		}

	old_size = (size_t)(*cap) * sizeof(lxs_source_standard_mux);
	while (*cap < needed)
		{
		*cap = (*cap == 0U) ? LXS_INITIAL_CAP : (*cap * 2U);
		}

	*items = lxs_realloc_aligned(
		*items,
		old_size,
		(size_t)(*cap) * sizeof(lxs_source_standard_mux));
	return *items != NULL;
	}

static int lxs_reserve_source_standard_adders(
	lxs_source_standard_add **items,
	uint32_t *cap,
	uint32_t needed)
	{
	size_t old_size;

	if (needed <= *cap)
		{
		return 1;
		}

	old_size = (size_t)(*cap) * sizeof(lxs_source_standard_add);
	while (*cap < needed)
		{
		*cap = (*cap == 0U) ? LXS_INITIAL_CAP : (*cap * 2U);
		}

	*items = lxs_realloc_aligned(
		*items,
		old_size,
		(size_t)(*cap) * sizeof(lxs_source_standard_add));
	return *items != NULL;
	}

static int lxs_reserve_source_standard_cmps(
	lxs_source_standard_cmp **items,
	uint32_t *cap,
	uint32_t needed)
	{
	size_t old_size;

	if (needed <= *cap)
		{
		return 1;
		}

	old_size = (size_t)(*cap) * sizeof(lxs_source_standard_cmp);
	while (*cap < needed)
		{
		*cap = (*cap == 0U) ? LXS_INITIAL_CAP : (*cap * 2U);
		}

	*items = lxs_realloc_aligned(
		*items,
		old_size,
		(size_t)(*cap) * sizeof(lxs_source_standard_cmp));
	return *items != NULL;
	}

static int lxs_reserve_source_standard_alus(
	lxs_source_standard_alu **items,
	uint32_t *cap,
	uint32_t needed)
	{
	size_t old_size;

	if (needed <= *cap)
		{
		return 1;
		}

	old_size = (size_t)(*cap) * sizeof(lxs_source_standard_alu);
	while (*cap < needed)
		{
		*cap = (*cap == 0U) ? LXS_INITIAL_CAP : (*cap * 2U);
		}

	*items = lxs_realloc_aligned(
		*items,
		old_size,
		(size_t)(*cap) * sizeof(lxs_source_standard_alu));
	return *items != NULL;
	}

static int lxs_reserve_source_functional_regions(
	lxs_source_functional_region **items,
	uint32_t *cap,
	uint32_t needed)
	{
	size_t old_size;

	if (needed <= *cap)
		{
		return 1;
		}

	old_size = (size_t)(*cap) * sizeof(lxs_source_functional_region);
	while (*cap < needed)
		{
		*cap = (*cap == 0U) ? LXS_INITIAL_CAP : (*cap * 2U);
		}

	*items = lxs_realloc_aligned(
		*items,
		old_size,
		(size_t)(*cap) * sizeof(lxs_source_functional_region));
	return *items != NULL;
	}

static int lxs_reserve_source_macros(
	lxs_source_macro **items,
	uint32_t *cap,
	uint32_t needed)
	{
	size_t old_size;

	if (needed <= *cap)
		{
		return 1;
		}

	old_size = (size_t)(*cap) * sizeof(lxs_source_macro);
	while (*cap < needed)
		{
		*cap = (*cap == 0U) ? LXS_INITIAL_CAP : (*cap * 2U);
		}

	*items = lxs_realloc_aligned(
		*items,
		old_size,
		(size_t)(*cap) * sizeof(lxs_source_macro));
	return *items != NULL;
	}

static int lxs_reserve_source_registers(
	lxs_source_register **items,
	uint32_t *cap,
	uint32_t needed)
	{
	size_t old_size;

	if (needed <= *cap)
		{
		return 1;
		}

	old_size = (size_t)(*cap) * sizeof(lxs_source_register);
	while (*cap < needed)
		{
		*cap = (*cap == 0U) ? LXS_INITIAL_CAP : (*cap * 2U);
		}

	*items = lxs_realloc_aligned(*items, old_size, (size_t)(*cap) * sizeof(lxs_source_register));
	return *items != NULL;
	}

static int lxs_reserve_source_roms(
	lxs_source_rom **items,
	uint32_t *cap,
	uint32_t needed)
	{
	size_t old_size;

	if (needed <= *cap)
		{
		return 1;
		}

	old_size = (size_t)(*cap) * sizeof(lxs_source_rom);
	while (*cap < needed)
		{
		*cap = (*cap == 0U) ? LXS_INITIAL_CAP : (*cap * 2U);
		}

	*items = lxs_realloc_aligned(*items, old_size, (size_t)(*cap) * sizeof(lxs_source_rom));
	return *items != NULL;
	}

static int lxs_reserve_source_rams(
	lxs_source_ram **items,
	uint32_t *cap,
	uint32_t needed)
	{
	size_t old_size;

	if (needed <= *cap)
		{
		return 1;
		}

	old_size = (size_t)(*cap) * sizeof(lxs_source_ram);
	while (*cap < needed)
		{
		*cap = (*cap == 0U) ? LXS_INITIAL_CAP : (*cap * 2U);
		}

	*items = lxs_realloc_aligned(*items, old_size, (size_t)(*cap) * sizeof(lxs_source_ram));
	return *items != NULL;
	}

static int lxs_reserve_source_regfiles(
	lxs_source_regfile **items,
	uint32_t *cap,
	uint32_t needed)
	{
	size_t old_size;

	if (needed <= *cap)
		{
		return 1;
		}

	old_size = (size_t)(*cap) * sizeof(lxs_source_regfile);
	while (*cap < needed)
		{
		*cap = (*cap == 0U) ? LXS_INITIAL_CAP : (*cap * 2U);
		}

	*items = lxs_realloc_aligned(*items, old_size, (size_t)(*cap) * sizeof(lxs_source_regfile));
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

static int lxs_push_source_multi_macro(lxs_netlist *nl, const lxs_source_multi_macro *macro)
	{
	if (!lxs_reserve_source_multi_macros(
		&nl->source_multi_macros,
		&nl->source_multi_macro_cap,
		nl->source_multi_macro_count + 1U))
		{
		return 0;
		}

	nl->source_multi_macros[nl->source_multi_macro_count++] = *macro;
	return 1;
	}

static int lxs_push_source_standard_mux(lxs_netlist *nl, const lxs_source_standard_mux *mux)
	{
	if (!lxs_reserve_source_standard_muxes(
		&nl->source_standard_muxes,
		&nl->source_standard_mux_cap,
		nl->source_standard_mux_count + 1U))
		{
		return 0;
		}

	nl->source_standard_muxes[nl->source_standard_mux_count++] = *mux;
	return 1;
	}

static int lxs_push_source_standard_add(lxs_netlist *nl, const lxs_source_standard_add *adder)
	{
	if (!lxs_reserve_source_standard_adders(
		&nl->source_standard_adders,
		&nl->source_standard_add_cap,
		nl->source_standard_add_count + 1U))
		{
		return 0;
		}

	nl->source_standard_adders[nl->source_standard_add_count++] = *adder;
	return 1;
	}

static int lxs_push_source_standard_cmp(lxs_netlist *nl, const lxs_source_standard_cmp *cmp)
	{
	if (!lxs_reserve_source_standard_cmps(
		&nl->source_standard_cmps,
		&nl->source_standard_cmp_cap,
		nl->source_standard_cmp_count + 1U))
		{
		return 0;
		}

	nl->source_standard_cmps[nl->source_standard_cmp_count++] = *cmp;
	return 1;
	}

static int lxs_push_source_standard_alu(lxs_netlist *nl, const lxs_source_standard_alu *alu)
	{
	if (!lxs_reserve_source_standard_alus(
		&nl->source_standard_alus,
		&nl->source_standard_alu_cap,
		nl->source_standard_alu_count + 1U))
		{
		return 0;
		}

	nl->source_standard_alus[nl->source_standard_alu_count++] = *alu;
	return 1;
	}

static int lxs_push_source_macro(lxs_netlist *nl, const lxs_source_macro *macro)
	{
	if (!lxs_reserve_source_macros(
		&nl->source_macros,
		&nl->source_macro_cap,
		nl->source_macro_count + 1U))
		{
		return 0;
		}

	nl->source_macros[nl->source_macro_count++] = *macro;
	return 1;
	}

static int lxs_push_source_functional_region(
	lxs_netlist *nl,
	const lxs_source_functional_region *region)
	{
	if (!lxs_reserve_source_functional_regions(
		&nl->source_functional_regions,
		&nl->source_functional_region_cap,
		nl->source_functional_region_count + 1U))
		{
		return 0;
		}

	nl->source_functional_regions[nl->source_functional_region_count++] = *region;
	return 1;
	}

static int lxs_push_u64(uint64_t **items, uint32_t *count, uint32_t *cap, uint64_t value)
	{
	if (!lxs_reserve_u64(items, cap, *count + 1U))
		{
		return 0;
		}

	(*items)[(*count)++] = value;
	return 1;
	}

static int lxs_push_source_register(lxs_netlist *nl, const lxs_source_register *reg)
	{
	if (!lxs_reserve_source_registers(
		&nl->source_registers,
		&nl->source_register_cap,
		nl->source_register_count + 1U))
		{
		return 0;
		}

	nl->source_registers[nl->source_register_count++] = *reg;
	return 1;
	}

static int lxs_push_source_rom(lxs_netlist *nl, const lxs_source_rom *rom)
	{
	if (!lxs_reserve_source_roms(
		&nl->source_roms,
		&nl->source_rom_cap,
		nl->source_rom_count + 1U))
		{
		return 0;
		}

	nl->source_roms[nl->source_rom_count++] = *rom;
	return 1;
	}

static int lxs_push_source_ram(lxs_netlist *nl, const lxs_source_ram *ram)
	{
	if (!lxs_reserve_source_rams(
		&nl->source_rams,
		&nl->source_ram_cap,
		nl->source_ram_count + 1U))
		{
		return 0;
		}

	nl->source_rams[nl->source_ram_count++] = *ram;
	return 1;
	}

static int lxs_push_source_regfile(lxs_netlist *nl, const lxs_source_regfile *regfile)
	{
	if (!lxs_reserve_source_regfiles(
		&nl->source_regfiles,
		&nl->source_regfile_cap,
		nl->source_regfile_count + 1U))
		{
		return 0;
		}

	nl->source_regfiles[nl->source_regfile_count++] = *regfile;
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

static int lxs_emit_gate_record(
	lxs_netlist *nl,
	uint32_t type,
	uint32_t output,
	const uint32_t *inputs,
	uint32_t input_count,
	uint32_t *gate_index_out)
	{
	lxs_gate_ir gate;

	memset(&gate, 0, sizeof(gate));
	gate.type = type;
	gate.output = output;
	gate.input_count = input_count;
	for (uint32_t i = 0; i < input_count && i < 8U; ++i)
		{
		gate.inputs[i] = inputs[i];
		}

	if (!lxs_push_gate(nl, &gate))
		{
		return 0;
		}

	if (gate_index_out)
		{
		*gate_index_out = nl->gate_count - 1U;
		}
	return 1;
	}

static uint32_t lxs_intern_temp_net(lxs_netlist *nl, const char *tag, uint32_t serial, const char *suffix)
	{
	char name[128];

	snprintf(name, sizeof(name), "__lxs_%s_%u_%s", tag, serial, suffix);
	return lxs_intern_net(nl, name);
	}

static int lxs_parse_literal_bits(const char *text, uint32_t width, uint64_t *values, uint64_t *masks)
	{
	char *end = NULL;
	unsigned long long scalar;

	scalar = strtoull(text, &end, 0);
	if (!end || *lxs_trim(end) != '\0')
		{
		return 0;
		}

	for (uint32_t bit = 0; bit < width; ++bit)
		{
		values[bit] = ((scalar >> bit) & 1ULL) ? ~0ULL : 0ULL;
		masks[bit] = 0ULL;
		}

	return 1;
	}

static int lxs_parse_standard_mux_name(
	const char *gate_name,
	uint32_t *kind_out,
	uint32_t *width_bits_out)
	{
	const char *width_text = NULL;
	uint32_t kind = 0U;
	uint32_t width_bits = 0U;

	if (strncmp(gate_name, "MUX2_", 5) == 0)
		{
		kind = LXS_STANDARD_MUX_KIND_2;
		width_text = gate_name + 5;
		}
	else if (strncmp(gate_name, "MUX4_", 5) == 0)
		{
		kind = LXS_STANDARD_MUX_KIND_4;
		width_text = gate_name + 5;
		}
	else
		{
		return 0;
		}

	if (strcmp(width_text, "8") == 0)
		{
		width_bits = 8U;
		}
	else if (strcmp(width_text, "16") == 0)
		{
		width_bits = 16U;
		}
	else if (strcmp(width_text, "32") == 0)
		{
		width_bits = 32U;
		}
	else if (strcmp(width_text, "64") == 0)
		{
		width_bits = 64U;
		}
	else
		{
		return 0;
		}

	if (kind_out)
		{
		*kind_out = kind;
		}
	if (width_bits_out)
		{
		*width_bits_out = width_bits;
		}
	return 1;
	}

static int lxs_parse_standard_register_name(
	const char *gate_name,
	uint32_t *width_bits_out)
	{
	uint32_t width_bits = 0U;
	const char *width_text = NULL;

	if (strncmp(gate_name, "REG", 3) != 0)
		{
		return 0;
		}

	width_text = gate_name + 3;
	if (strcmp(width_text, "8") == 0)
		{
		width_bits = 8U;
		}
	else if (strcmp(width_text, "16") == 0)
		{
		width_bits = 16U;
		}
	else if (strcmp(width_text, "32") == 0)
		{
		width_bits = 32U;
		}
	else if (strcmp(width_text, "64") == 0)
		{
		width_bits = 64U;
		}
	else
		{
		return 0;
		}

	if (width_bits_out)
		{
		*width_bits_out = width_bits;
		}
	return 1;
	}

static int lxs_parse_standard_register_en_name(
	const char *gate_name,
	uint32_t *width_bits_out)
	{
	uint32_t width_bits = 0U;
	const char *width_text = NULL;

	if (strncmp(gate_name, "REG_EN", 6) != 0)
		{
		return 0;
		}

	width_text = gate_name + 6;
	if (strcmp(width_text, "8") == 0)
		{
		width_bits = 8U;
		}
	else if (strcmp(width_text, "16") == 0)
		{
		width_bits = 16U;
		}
	else if (strcmp(width_text, "32") == 0)
		{
		width_bits = 32U;
		}
	else if (strcmp(width_text, "64") == 0)
		{
		width_bits = 64U;
		}
	else
		{
		return 0;
		}

	if (width_bits_out)
		{
		*width_bits_out = width_bits;
		}
	return 1;
	}

static int lxs_parse_standard_register_en_rst_name(
	const char *gate_name,
	uint32_t *width_bits_out)
	{
	uint32_t width_bits = 0U;
	const char *width_text = NULL;

	if (strncmp(gate_name, "REG_EN_RST", 10) != 0)
		{
		return 0;
		}

	width_text = gate_name + 10;
	if (strcmp(width_text, "8") == 0)
		{
		width_bits = 8U;
		}
	else if (strcmp(width_text, "16") == 0)
		{
		width_bits = 16U;
		}
	else if (strcmp(width_text, "32") == 0)
		{
		width_bits = 32U;
		}
	else if (strcmp(width_text, "64") == 0)
		{
		width_bits = 64U;
		}
	else
		{
		return 0;
		}

	if (width_bits_out)
		{
		*width_bits_out = width_bits;
		}
	return 1;
	}

static int lxs_parse_standard_add_name(
	const char *gate_name,
	uint32_t *width_bits_out)
	{
	uint32_t width_bits = 0U;
	const char *width_text = NULL;

	if (strncmp(gate_name, "ADD", 3) != 0)
		{
		return 0;
		}

	width_text = gate_name + 3;
	if (strcmp(width_text, "8") == 0)
		{
		width_bits = 8U;
		}
	else if (strcmp(width_text, "16") == 0)
		{
		width_bits = 16U;
		}
	else if (strcmp(width_text, "32") == 0)
		{
		width_bits = 32U;
		}
	else if (strcmp(width_text, "64") == 0)
		{
		width_bits = 64U;
		}
	else
		{
		return 0;
		}

	if (width_bits_out)
		{
		*width_bits_out = width_bits;
		}
	return 1;
	}

static int lxs_parse_standard_cmp_name(
	const char *gate_name,
	uint32_t *width_bits_out)
	{
	uint32_t width_bits = 0U;
	const char *width_text = NULL;

	if (strncmp(gate_name, "CMP", 3) != 0)
		{
		return 0;
		}

	width_text = gate_name + 3;
	if (strcmp(width_text, "8") == 0)
		{
		width_bits = 8U;
		}
	else if (strcmp(width_text, "16") == 0)
		{
		width_bits = 16U;
		}
	else if (strcmp(width_text, "32") == 0)
		{
		width_bits = 32U;
		}
	else if (strcmp(width_text, "64") == 0)
		{
		width_bits = 64U;
		}
	else
		{
		return 0;
		}

	if (width_bits_out)
		{
		*width_bits_out = width_bits;
		}
	return 1;
	}

static int lxs_parse_standard_alu_name(
	const char *gate_name,
	uint32_t *width_bits_out)
	{
	uint32_t width_bits = 0U;
	const char *width_text = NULL;

	if (strncmp(gate_name, "ALU", 3) != 0)
		{
		return 0;
		}

	width_text = gate_name + 3;
	if (strcmp(width_text, "8") == 0)
		{
		width_bits = 8U;
		}
	else if (strcmp(width_text, "16") == 0)
		{
		width_bits = 16U;
		}
	else if (strcmp(width_text, "32") == 0)
		{
		width_bits = 32U;
		}
	else if (strcmp(width_text, "64") == 0)
		{
		width_bits = 64U;
		}
	else
		{
		return 0;
		}

	if (width_bits_out)
		{
		*width_bits_out = width_bits;
		}
	return 1;
	}

static int lxs_parse_standard_rom_name(
	const char *gate_name,
	uint32_t *data_width_out)
	{
	if (strcmp(gate_name, "ROM16") != 0)
		{
		return 0;
		}

	if (data_width_out)
		{
		*data_width_out = 16U;
		}
	return 1;
	}

static int lxs_parse_standard_ram_name(
	const char *gate_name,
	uint32_t *data_width_out)
	{
	uint32_t width_bits = 0U;

	if (strcmp(gate_name, "RAM8") == 0)
		{
		width_bits = 8U;
		}
	else if (strcmp(gate_name, "RAM16") == 0)
		{
		width_bits = 16U;
		}
	else if (strcmp(gate_name, "RAM24") == 0)
		{
		width_bits = 24U;
		}
	else
		{
		return 0;
		}

	if (data_width_out)
		{
		*data_width_out = width_bits;
		}
	return 1;
	}

static int lxs_parse_standard_regfile_name(
	const char *gate_name,
	uint32_t *data_width_out)
	{
	uint32_t width_bits = 0U;

	if (strcmp(gate_name, "REGFILE8_2R1W") == 0)
		{
		width_bits = 8U;
		}
	else if (strcmp(gate_name, "REGFILE16_2R1W") == 0)
		{
		width_bits = 16U;
		}
	else if (strcmp(gate_name, "REGFILE32_2R1W") == 0)
		{
		width_bits = 32U;
		}
	else
		{
		return 0;
		}

	if (data_width_out)
		{
		*data_width_out = width_bits;
		}
	return 1;
	}

static int lxs_parse_standard_counter_name(
	const char *gate_name,
	uint32_t *width_bits_out)
	{
	uint32_t width_bits = 0U;

	if (strcmp(gate_name, "COUNTER8") == 0)
		{
		width_bits = 8U;
		}
	else if (strcmp(gate_name, "COUNTER16") == 0)
		{
		width_bits = 16U;
		}
	else if (strcmp(gate_name, "COUNTER32") == 0)
		{
		width_bits = 32U;
		}
	else if (strcmp(gate_name, "COUNTER64") == 0)
		{
		width_bits = 64U;
		}
	else
		{
		return 0;
		}

	if (width_bits_out)
		{
		*width_bits_out = width_bits;
		}
	return 1;
	}

static int lxs_parse_standard_counter_en_name(
	const char *gate_name,
	uint32_t *width_bits_out)
	{
	uint32_t width_bits = 0U;

	if (strcmp(gate_name, "COUNTER_EN8") == 0)
		{
		width_bits = 8U;
		}
	else if (strcmp(gate_name, "COUNTER_EN16") == 0)
		{
		width_bits = 16U;
		}
	else if (strcmp(gate_name, "COUNTER_EN32") == 0)
		{
		width_bits = 32U;
		}
	else if (strcmp(gate_name, "COUNTER_EN64") == 0)
		{
		width_bits = 64U;
		}
	else
		{
		return 0;
		}

	if (width_bits_out)
		{
		*width_bits_out = width_bits;
		}
	return 1;
	}

static int lxs_emit_tracked_gate(
	lxs_netlist *nl,
	uint32_t gate_type,
	uint32_t output,
	const uint32_t *inputs,
	uint32_t input_count,
	uint32_t **gate_indices,
	uint32_t *gate_index_count,
	uint32_t *gate_index_cap)
	{
	uint32_t gate_index;

	if (!lxs_emit_gate_record(nl, gate_type, output, inputs, input_count, &gate_index))
		{
		return 0;
		}
	return lxs_push_u32(gate_indices, gate_index_count, gate_index_cap, gate_index);
	}

static int lxs_emit_standard_mux_descriptor(
	lxs_netlist *nl,
	const uint32_t *outputs,
	const uint32_t *inputs,
	uint32_t input_count,
	uint32_t output_count,
	uint32_t kind,
	uint32_t width_bits,
	uint32_t serial)
	{
	lxs_source_standard_mux mux;
	uint32_t select_count;
	uint32_t data_bus_count;

	select_count = kind == LXS_STANDARD_MUX_KIND_2 ? 1U : 2U;
	data_bus_count = kind == LXS_STANDARD_MUX_KIND_2 ? 2U : 4U;

	if (width_bits == 0U || width_bits > LXS_STANDARD_MACRO_MAX_WIDTH)
		{
		return 0;
		}
	if (output_count != width_bits)
		{
		return 0;
		}
	if (input_count != (width_bits * data_bus_count) + select_count)
		{
		return 0;
		}

	memset(&mux, 0, sizeof(mux));
	mux.kind = kind;
	mux.width_bits = width_bits;
	mux.data_start = nl->source_standard_mux_data_count;
	mux.select_start = nl->source_standard_mux_select_count;
	mux.output_start = nl->source_standard_mux_output_count;
	mux.gate_index_start = nl->source_standard_mux_gate_index_count;

	for (uint32_t i = 0; i < width_bits * data_bus_count; ++i)
		{
		if (!lxs_push_u32(
				&nl->source_standard_mux_data_net_ids,
				&nl->source_standard_mux_data_count,
				&nl->source_standard_mux_data_cap,
				inputs[i]))
			{
			return 0;
			}
		}

	for (uint32_t i = 0; i < select_count; ++i)
		{
		if (!lxs_push_u32(
				&nl->source_standard_mux_select_net_ids,
				&nl->source_standard_mux_select_count,
				&nl->source_standard_mux_select_cap,
				inputs[width_bits * data_bus_count + i]))
			{
			return 0;
			}
		}

	for (uint32_t i = 0; i < width_bits; ++i)
		{
		if (!lxs_push_u32(
				&nl->source_standard_mux_output_net_ids,
				&nl->source_standard_mux_output_count,
				&nl->source_standard_mux_output_cap,
				outputs[i]))
			{
			return 0;
			}
		}

	if (kind == LXS_STANDARD_MUX_KIND_2)
		{
		uint32_t sel = inputs[width_bits * 2U];
		uint32_t nsel;
		uint32_t gate_inputs[3];
		uint32_t gate_index;

		nsel = lxs_intern_temp_net(nl, "smx2", serial, "nsel");
		if (nsel == UINT32_MAX)
			{
			return 0;
			}
		gate_inputs[0] = sel;
		if (!lxs_emit_gate_record(nl, LXS_GATE_NOT, nsel, gate_inputs, 1U, &gate_index) ||
			!lxs_push_u32(
				&nl->source_standard_mux_gate_indices,
				&nl->source_standard_mux_gate_index_count,
				&nl->source_standard_mux_gate_index_cap,
				gate_index))
			{
			return 0;
			}

		for (uint32_t bit = 0; bit < width_bits; ++bit)
			{
			char suffix[32];
			uint32_t lo_term;
			uint32_t hi_term;

			snprintf(suffix, sizeof(suffix), "b%u_lo", bit);
			lo_term = lxs_intern_temp_net(nl, "smx2", serial, suffix);
			snprintf(suffix, sizeof(suffix), "b%u_hi", bit);
			hi_term = lxs_intern_temp_net(nl, "smx2", serial, suffix);
			if (lo_term == UINT32_MAX || hi_term == UINT32_MAX)
				{
				return 0;
				}

			gate_inputs[0] = inputs[bit];
			gate_inputs[1] = nsel;
			if (!lxs_emit_gate_record(nl, LXS_GATE_AND, lo_term, gate_inputs, 2U, &gate_index) ||
				!lxs_push_u32(
					&nl->source_standard_mux_gate_indices,
					&nl->source_standard_mux_gate_index_count,
					&nl->source_standard_mux_gate_index_cap,
					gate_index))
				{
				return 0;
				}

			gate_inputs[0] = inputs[width_bits + bit];
			gate_inputs[1] = sel;
			if (!lxs_emit_gate_record(nl, LXS_GATE_AND, hi_term, gate_inputs, 2U, &gate_index) ||
				!lxs_push_u32(
					&nl->source_standard_mux_gate_indices,
					&nl->source_standard_mux_gate_index_count,
					&nl->source_standard_mux_gate_index_cap,
					gate_index))
				{
				return 0;
				}

			gate_inputs[0] = lo_term;
			gate_inputs[1] = hi_term;
			if (!lxs_emit_gate_record(nl, LXS_GATE_OR, outputs[bit], gate_inputs, 2U, &gate_index) ||
				!lxs_push_u32(
					&nl->source_standard_mux_gate_indices,
					&nl->source_standard_mux_gate_index_count,
					&nl->source_standard_mux_gate_index_cap,
					gate_index))
				{
				return 0;
				}
			}
		}
	else
		{
		uint32_t s0 = inputs[width_bits * 4U + 0U];
		uint32_t s1 = inputs[width_bits * 4U + 1U];
		uint32_t ns0;
		uint32_t ns1;
		uint32_t gate_inputs[3];
		uint32_t gate_index;

		ns0 = lxs_intern_temp_net(nl, "smx4", serial, "ns0");
		ns1 = lxs_intern_temp_net(nl, "smx4", serial, "ns1");
		if (ns0 == UINT32_MAX || ns1 == UINT32_MAX)
			{
			return 0;
			}

		gate_inputs[0] = s0;
		if (!lxs_emit_gate_record(nl, LXS_GATE_NOT, ns0, gate_inputs, 1U, &gate_index) ||
			!lxs_push_u32(
				&nl->source_standard_mux_gate_indices,
				&nl->source_standard_mux_gate_index_count,
				&nl->source_standard_mux_gate_index_cap,
				gate_index))
			{
			return 0;
			}
		gate_inputs[0] = s1;
		if (!lxs_emit_gate_record(nl, LXS_GATE_NOT, ns1, gate_inputs, 1U, &gate_index) ||
			!lxs_push_u32(
				&nl->source_standard_mux_gate_indices,
				&nl->source_standard_mux_gate_index_count,
				&nl->source_standard_mux_gate_index_cap,
				gate_index))
			{
			return 0;
			}

		for (uint32_t bit = 0; bit < width_bits; ++bit)
			{
			char suffix[32];
			uint32_t lo_a;
			uint32_t lo_b;
			uint32_t lo_mux;
			uint32_t hi_a;
			uint32_t hi_b;
			uint32_t hi_mux;
			uint32_t out_a;
			uint32_t out_b;

			snprintf(suffix, sizeof(suffix), "b%u_lo_a", bit);
			lo_a = lxs_intern_temp_net(nl, "smx4", serial, suffix);
			snprintf(suffix, sizeof(suffix), "b%u_lo_b", bit);
			lo_b = lxs_intern_temp_net(nl, "smx4", serial, suffix);
			snprintf(suffix, sizeof(suffix), "b%u_lo", bit);
			lo_mux = lxs_intern_temp_net(nl, "smx4", serial, suffix);
			snprintf(suffix, sizeof(suffix), "b%u_hi_a", bit);
			hi_a = lxs_intern_temp_net(nl, "smx4", serial, suffix);
			snprintf(suffix, sizeof(suffix), "b%u_hi_b", bit);
			hi_b = lxs_intern_temp_net(nl, "smx4", serial, suffix);
			snprintf(suffix, sizeof(suffix), "b%u_hi", bit);
			hi_mux = lxs_intern_temp_net(nl, "smx4", serial, suffix);
			snprintf(suffix, sizeof(suffix), "b%u_out_a", bit);
			out_a = lxs_intern_temp_net(nl, "smx4", serial, suffix);
			snprintf(suffix, sizeof(suffix), "b%u_out_b", bit);
			out_b = lxs_intern_temp_net(nl, "smx4", serial, suffix);
			if (lo_a == UINT32_MAX || lo_b == UINT32_MAX || lo_mux == UINT32_MAX ||
				hi_a == UINT32_MAX || hi_b == UINT32_MAX || hi_mux == UINT32_MAX ||
				out_a == UINT32_MAX || out_b == UINT32_MAX)
				{
				return 0;
				}

			gate_inputs[0] = inputs[bit];
			gate_inputs[1] = ns0;
			if (!lxs_emit_gate_record(nl, LXS_GATE_AND, lo_a, gate_inputs, 2U, &gate_index) ||
				!lxs_push_u32(&nl->source_standard_mux_gate_indices, &nl->source_standard_mux_gate_index_count, &nl->source_standard_mux_gate_index_cap, gate_index))
				{
				return 0;
				}
			gate_inputs[0] = inputs[width_bits + bit];
			gate_inputs[1] = s0;
			if (!lxs_emit_gate_record(nl, LXS_GATE_AND, lo_b, gate_inputs, 2U, &gate_index) ||
				!lxs_push_u32(&nl->source_standard_mux_gate_indices, &nl->source_standard_mux_gate_index_count, &nl->source_standard_mux_gate_index_cap, gate_index))
				{
				return 0;
				}
			gate_inputs[0] = lo_a;
			gate_inputs[1] = lo_b;
			if (!lxs_emit_gate_record(nl, LXS_GATE_OR, lo_mux, gate_inputs, 2U, &gate_index) ||
				!lxs_push_u32(&nl->source_standard_mux_gate_indices, &nl->source_standard_mux_gate_index_count, &nl->source_standard_mux_gate_index_cap, gate_index))
				{
				return 0;
				}

			gate_inputs[0] = inputs[(width_bits * 2U) + bit];
			gate_inputs[1] = ns0;
			if (!lxs_emit_gate_record(nl, LXS_GATE_AND, hi_a, gate_inputs, 2U, &gate_index) ||
				!lxs_push_u32(&nl->source_standard_mux_gate_indices, &nl->source_standard_mux_gate_index_count, &nl->source_standard_mux_gate_index_cap, gate_index))
				{
				return 0;
				}
			gate_inputs[0] = inputs[(width_bits * 3U) + bit];
			gate_inputs[1] = s0;
			if (!lxs_emit_gate_record(nl, LXS_GATE_AND, hi_b, gate_inputs, 2U, &gate_index) ||
				!lxs_push_u32(&nl->source_standard_mux_gate_indices, &nl->source_standard_mux_gate_index_count, &nl->source_standard_mux_gate_index_cap, gate_index))
				{
				return 0;
				}
			gate_inputs[0] = hi_a;
			gate_inputs[1] = hi_b;
			if (!lxs_emit_gate_record(nl, LXS_GATE_OR, hi_mux, gate_inputs, 2U, &gate_index) ||
				!lxs_push_u32(&nl->source_standard_mux_gate_indices, &nl->source_standard_mux_gate_index_count, &nl->source_standard_mux_gate_index_cap, gate_index))
				{
				return 0;
				}

			gate_inputs[0] = lo_mux;
			gate_inputs[1] = ns1;
			if (!lxs_emit_gate_record(nl, LXS_GATE_AND, out_a, gate_inputs, 2U, &gate_index) ||
				!lxs_push_u32(&nl->source_standard_mux_gate_indices, &nl->source_standard_mux_gate_index_count, &nl->source_standard_mux_gate_index_cap, gate_index))
				{
				return 0;
				}
			gate_inputs[0] = hi_mux;
			gate_inputs[1] = s1;
			if (!lxs_emit_gate_record(nl, LXS_GATE_AND, out_b, gate_inputs, 2U, &gate_index) ||
				!lxs_push_u32(&nl->source_standard_mux_gate_indices, &nl->source_standard_mux_gate_index_count, &nl->source_standard_mux_gate_index_cap, gate_index))
				{
				return 0;
				}
			gate_inputs[0] = out_a;
			gate_inputs[1] = out_b;
			if (!lxs_emit_gate_record(nl, LXS_GATE_OR, outputs[bit], gate_inputs, 2U, &gate_index) ||
				!lxs_push_u32(&nl->source_standard_mux_gate_indices, &nl->source_standard_mux_gate_index_count, &nl->source_standard_mux_gate_index_cap, gate_index))
				{
				return 0;
				}
			}
		}

	mux.gate_count = nl->source_standard_mux_gate_index_count - mux.gate_index_start;
	return lxs_push_source_standard_mux(nl, &mux);
	}

static int lxs_emit_standard_add_descriptor(
	lxs_netlist *nl,
	const uint32_t *outputs,
	const uint32_t *inputs,
	uint32_t input_count,
	uint32_t output_count,
	uint32_t width_bits,
	uint32_t serial)
	{
	lxs_source_standard_add adder;
	uint32_t gate_inputs[3];
	uint32_t gate_index;
	uint32_t carry_in;

	if (width_bits == 0U || width_bits > 64U)
		{
		return 0;
		}
	if (input_count != (width_bits * 2U) + 1U)
		{
		return 0;
		}
	if (output_count != width_bits + 1U)
		{
		return 0;
		}

	memset(&adder, 0, sizeof(adder));
	adder.kind = LXS_STANDARD_ADD_KIND_RIPPLE;
	adder.width_bits = width_bits;
	adder.input_start = nl->source_standard_add_input_count;
	adder.output_start = nl->source_standard_add_output_count;
	adder.gate_index_start = nl->source_standard_add_gate_index_count;

	for (uint32_t i = 0; i < input_count; ++i)
		{
		if (!lxs_push_u32(
				&nl->source_standard_add_input_net_ids,
				&nl->source_standard_add_input_count,
				&nl->source_standard_add_input_cap,
				inputs[i]))
			{
			return 0;
			}
		}

	for (uint32_t i = 0; i < output_count; ++i)
		{
		if (!lxs_push_u32(
				&nl->source_standard_add_output_net_ids,
				&nl->source_standard_add_output_count,
				&nl->source_standard_add_output_cap,
				outputs[i]))
			{
			return 0;
			}
		}

	carry_in = inputs[width_bits * 2U];
	for (uint32_t bit = 0; bit < width_bits; ++bit)
		{
		char suffix[32];
		uint32_t a_net = inputs[bit];
		uint32_t b_net = inputs[width_bits + bit];
		uint32_t xor_ab;
		uint32_t and_ab;
		uint32_t and_carry;
		uint32_t carry_out = (bit + 1U == width_bits) ? outputs[width_bits] : UINT32_MAX;

		snprintf(suffix, sizeof(suffix), "b%u_xab", bit);
		xor_ab = lxs_intern_temp_net(nl, "sadd", serial, suffix);
		snprintf(suffix, sizeof(suffix), "b%u_aab", bit);
		and_ab = lxs_intern_temp_net(nl, "sadd", serial, suffix);
		snprintf(suffix, sizeof(suffix), "b%u_ac", bit);
		and_carry = lxs_intern_temp_net(nl, "sadd", serial, suffix);
		if (bit + 1U != width_bits)
			{
			snprintf(suffix, sizeof(suffix), "b%u_cout", bit);
			carry_out = lxs_intern_temp_net(nl, "sadd", serial, suffix);
			}
		if (xor_ab == UINT32_MAX || and_ab == UINT32_MAX ||
			and_carry == UINT32_MAX || carry_out == UINT32_MAX)
			{
			return 0;
			}

		gate_inputs[0] = a_net;
		gate_inputs[1] = b_net;
		if (!lxs_emit_gate_record(nl, LXS_GATE_XOR, xor_ab, gate_inputs, 2U, &gate_index) ||
			!lxs_push_u32(&nl->source_standard_add_gate_indices, &nl->source_standard_add_gate_index_count, &nl->source_standard_add_gate_index_cap, gate_index))
			{
			return 0;
			}

		gate_inputs[0] = xor_ab;
		gate_inputs[1] = carry_in;
		if (!lxs_emit_gate_record(nl, LXS_GATE_XOR, outputs[bit], gate_inputs, 2U, &gate_index) ||
			!lxs_push_u32(&nl->source_standard_add_gate_indices, &nl->source_standard_add_gate_index_count, &nl->source_standard_add_gate_index_cap, gate_index))
			{
			return 0;
			}

		gate_inputs[0] = a_net;
		gate_inputs[1] = b_net;
		if (!lxs_emit_gate_record(nl, LXS_GATE_AND, and_ab, gate_inputs, 2U, &gate_index) ||
			!lxs_push_u32(&nl->source_standard_add_gate_indices, &nl->source_standard_add_gate_index_count, &nl->source_standard_add_gate_index_cap, gate_index))
			{
			return 0;
			}

		gate_inputs[0] = xor_ab;
		gate_inputs[1] = carry_in;
		if (!lxs_emit_gate_record(nl, LXS_GATE_AND, and_carry, gate_inputs, 2U, &gate_index) ||
			!lxs_push_u32(&nl->source_standard_add_gate_indices, &nl->source_standard_add_gate_index_count, &nl->source_standard_add_gate_index_cap, gate_index))
			{
			return 0;
			}

		gate_inputs[0] = and_ab;
		gate_inputs[1] = and_carry;
		if (!lxs_emit_gate_record(nl, LXS_GATE_OR, carry_out, gate_inputs, 2U, &gate_index) ||
			!lxs_push_u32(&nl->source_standard_add_gate_indices, &nl->source_standard_add_gate_index_count, &nl->source_standard_add_gate_index_cap, gate_index))
			{
			return 0;
			}

		carry_in = carry_out;
		}

	adder.gate_count = nl->source_standard_add_gate_index_count - adder.gate_index_start;
	return lxs_push_source_standard_add(nl, &adder);
	}

static int lxs_emit_standard_cmp_descriptor(
	lxs_netlist *nl,
	const uint32_t *outputs,
	const uint32_t *inputs,
	uint32_t input_count,
	uint32_t output_count,
	uint32_t width_bits,
	uint32_t serial)
	{
	lxs_source_standard_cmp cmp;
	uint32_t gate_inputs[2];
	uint32_t gate_index;
	uint32_t zero_net;
	uint32_t true_net;
	uint32_t eq_prev;
	uint32_t lt_prev;
	uint32_t gt_prev;

	if (width_bits == 0U || width_bits > 64U)
		{
		return 0;
		}
	if (input_count != (width_bits * 2U))
		{
		return 0;
		}
	if (output_count != 3U)
		{
		return 0;
		}

	memset(&cmp, 0, sizeof(cmp));
	cmp.kind = LXS_STANDARD_CMP_KIND_UNSIGNED;
	cmp.width_bits = width_bits;
	cmp.input_start = nl->source_standard_cmp_input_count;
	cmp.output_start = nl->source_standard_cmp_output_count;
	cmp.gate_index_start = nl->source_standard_cmp_gate_index_count;

	for (uint32_t i = 0; i < input_count; ++i)
		{
		if (!lxs_push_u32(
				&nl->source_standard_cmp_input_net_ids,
				&nl->source_standard_cmp_input_count,
				&nl->source_standard_cmp_input_cap,
				inputs[i]))
			{
			return 0;
			}
		}

	for (uint32_t i = 0; i < output_count; ++i)
		{
		if (!lxs_push_u32(
				&nl->source_standard_cmp_output_net_ids,
				&nl->source_standard_cmp_output_count,
				&nl->source_standard_cmp_output_cap,
				outputs[i]))
			{
			return 0;
			}
		}

	zero_net = lxs_intern_temp_net(nl, "scmp", serial, "zero");
	true_net = lxs_intern_temp_net(nl, "scmp", serial, "true");
	if (zero_net == UINT32_MAX || true_net == UINT32_MAX)
		{
		return 0;
		}

	gate_inputs[0] = zero_net;
	if (!lxs_emit_gate_record(nl, LXS_GATE_NOT, true_net, gate_inputs, 1U, &gate_index) ||
		!lxs_push_u32(&nl->source_standard_cmp_gate_indices, &nl->source_standard_cmp_gate_index_count, &nl->source_standard_cmp_gate_index_cap, gate_index))
		{
		return 0;
		}

	eq_prev = true_net;
	lt_prev = zero_net;
	gt_prev = zero_net;

	for (uint32_t step = 0; step < width_bits; ++step)
		{
		char suffix[32];
		uint32_t bit = width_bits - 1U - step;
		uint32_t a_net = inputs[bit];
		uint32_t b_net = inputs[width_bits + bit];
		uint32_t not_a;
		uint32_t not_b;
		uint32_t xor_ab;
		uint32_t bit_eq;
		uint32_t lt_raw;
		uint32_t gt_raw;
		uint32_t lt_candidate;
		uint32_t gt_candidate;
		uint32_t eq_next = (bit == 0U) ? outputs[0] : UINT32_MAX;
		uint32_t lt_next = (bit == 0U) ? outputs[1] : UINT32_MAX;
		uint32_t gt_next = (bit == 0U) ? outputs[2] : UINT32_MAX;

		snprintf(suffix, sizeof(suffix), "b%u_na", bit);
		not_a = lxs_intern_temp_net(nl, "scmp", serial, suffix);
		snprintf(suffix, sizeof(suffix), "b%u_nb", bit);
		not_b = lxs_intern_temp_net(nl, "scmp", serial, suffix);
		snprintf(suffix, sizeof(suffix), "b%u_xab", bit);
		xor_ab = lxs_intern_temp_net(nl, "scmp", serial, suffix);
		snprintf(suffix, sizeof(suffix), "b%u_beq", bit);
		bit_eq = lxs_intern_temp_net(nl, "scmp", serial, suffix);
		snprintf(suffix, sizeof(suffix), "b%u_lraw", bit);
		lt_raw = lxs_intern_temp_net(nl, "scmp", serial, suffix);
		snprintf(suffix, sizeof(suffix), "b%u_graw", bit);
		gt_raw = lxs_intern_temp_net(nl, "scmp", serial, suffix);
		snprintf(suffix, sizeof(suffix), "b%u_lcand", bit);
		lt_candidate = lxs_intern_temp_net(nl, "scmp", serial, suffix);
		snprintf(suffix, sizeof(suffix), "b%u_gcand", bit);
		gt_candidate = lxs_intern_temp_net(nl, "scmp", serial, suffix);
		if (bit != 0U)
			{
			snprintf(suffix, sizeof(suffix), "b%u_eqn", bit);
			eq_next = lxs_intern_temp_net(nl, "scmp", serial, suffix);
			snprintf(suffix, sizeof(suffix), "b%u_ltn", bit);
			lt_next = lxs_intern_temp_net(nl, "scmp", serial, suffix);
			snprintf(suffix, sizeof(suffix), "b%u_gtn", bit);
			gt_next = lxs_intern_temp_net(nl, "scmp", serial, suffix);
			}

		if (not_a == UINT32_MAX || not_b == UINT32_MAX || xor_ab == UINT32_MAX ||
			bit_eq == UINT32_MAX || lt_raw == UINT32_MAX || gt_raw == UINT32_MAX ||
			lt_candidate == UINT32_MAX || gt_candidate == UINT32_MAX ||
			eq_next == UINT32_MAX || lt_next == UINT32_MAX || gt_next == UINT32_MAX)
			{
			return 0;
			}

		gate_inputs[0] = a_net;
		if (!lxs_emit_gate_record(nl, LXS_GATE_NOT, not_a, gate_inputs, 1U, &gate_index) ||
			!lxs_push_u32(&nl->source_standard_cmp_gate_indices, &nl->source_standard_cmp_gate_index_count, &nl->source_standard_cmp_gate_index_cap, gate_index))
			{
			return 0;
			}
		gate_inputs[0] = b_net;
		if (!lxs_emit_gate_record(nl, LXS_GATE_NOT, not_b, gate_inputs, 1U, &gate_index) ||
			!lxs_push_u32(&nl->source_standard_cmp_gate_indices, &nl->source_standard_cmp_gate_index_count, &nl->source_standard_cmp_gate_index_cap, gate_index))
			{
			return 0;
			}
		gate_inputs[0] = a_net;
		gate_inputs[1] = b_net;
		if (!lxs_emit_gate_record(nl, LXS_GATE_XOR, xor_ab, gate_inputs, 2U, &gate_index) ||
			!lxs_push_u32(&nl->source_standard_cmp_gate_indices, &nl->source_standard_cmp_gate_index_count, &nl->source_standard_cmp_gate_index_cap, gate_index))
			{
			return 0;
			}
		gate_inputs[0] = xor_ab;
		if (!lxs_emit_gate_record(nl, LXS_GATE_NOT, bit_eq, gate_inputs, 1U, &gate_index) ||
			!lxs_push_u32(&nl->source_standard_cmp_gate_indices, &nl->source_standard_cmp_gate_index_count, &nl->source_standard_cmp_gate_index_cap, gate_index))
			{
			return 0;
			}
		gate_inputs[0] = not_a;
		gate_inputs[1] = b_net;
		if (!lxs_emit_gate_record(nl, LXS_GATE_AND, lt_raw, gate_inputs, 2U, &gate_index) ||
			!lxs_push_u32(&nl->source_standard_cmp_gate_indices, &nl->source_standard_cmp_gate_index_count, &nl->source_standard_cmp_gate_index_cap, gate_index))
			{
			return 0;
			}
		gate_inputs[0] = a_net;
		gate_inputs[1] = not_b;
		if (!lxs_emit_gate_record(nl, LXS_GATE_AND, gt_raw, gate_inputs, 2U, &gate_index) ||
			!lxs_push_u32(&nl->source_standard_cmp_gate_indices, &nl->source_standard_cmp_gate_index_count, &nl->source_standard_cmp_gate_index_cap, gate_index))
			{
			return 0;
			}
		gate_inputs[0] = eq_prev;
		gate_inputs[1] = lt_raw;
		if (!lxs_emit_gate_record(nl, LXS_GATE_AND, lt_candidate, gate_inputs, 2U, &gate_index) ||
			!lxs_push_u32(&nl->source_standard_cmp_gate_indices, &nl->source_standard_cmp_gate_index_count, &nl->source_standard_cmp_gate_index_cap, gate_index))
			{
			return 0;
			}
		gate_inputs[0] = eq_prev;
		gate_inputs[1] = gt_raw;
		if (!lxs_emit_gate_record(nl, LXS_GATE_AND, gt_candidate, gate_inputs, 2U, &gate_index) ||
			!lxs_push_u32(&nl->source_standard_cmp_gate_indices, &nl->source_standard_cmp_gate_index_count, &nl->source_standard_cmp_gate_index_cap, gate_index))
			{
			return 0;
			}
		gate_inputs[0] = eq_prev;
		gate_inputs[1] = bit_eq;
		if (!lxs_emit_gate_record(nl, LXS_GATE_AND, eq_next, gate_inputs, 2U, &gate_index) ||
			!lxs_push_u32(&nl->source_standard_cmp_gate_indices, &nl->source_standard_cmp_gate_index_count, &nl->source_standard_cmp_gate_index_cap, gate_index))
			{
			return 0;
			}
		gate_inputs[0] = lt_prev;
		gate_inputs[1] = lt_candidate;
		if (!lxs_emit_gate_record(nl, LXS_GATE_OR, lt_next, gate_inputs, 2U, &gate_index) ||
			!lxs_push_u32(&nl->source_standard_cmp_gate_indices, &nl->source_standard_cmp_gate_index_count, &nl->source_standard_cmp_gate_index_cap, gate_index))
			{
			return 0;
			}
		gate_inputs[0] = gt_prev;
		gate_inputs[1] = gt_candidate;
		if (!lxs_emit_gate_record(nl, LXS_GATE_OR, gt_next, gate_inputs, 2U, &gate_index) ||
			!lxs_push_u32(&nl->source_standard_cmp_gate_indices, &nl->source_standard_cmp_gate_index_count, &nl->source_standard_cmp_gate_index_cap, gate_index))
			{
			return 0;
			}

		eq_prev = eq_next;
		lt_prev = lt_next;
		gt_prev = gt_next;
		}

	cmp.gate_count = nl->source_standard_cmp_gate_index_count - cmp.gate_index_start;
	return lxs_push_source_standard_cmp(nl, &cmp);
	}

static int lxs_emit_standard_alu_descriptor(
	lxs_netlist *nl,
	const uint32_t *outputs,
	const uint32_t *inputs,
	uint32_t input_count,
	uint32_t output_count,
	uint32_t width_bits,
	uint32_t serial)
	{
	lxs_source_standard_alu alu;
	uint32_t gate_inputs[3];
	uint32_t add_sum[64];
	uint32_t sub_sum[64];
	uint32_t and_bits[64];
	uint32_t or_bits[64];
	uint32_t xor_bits[64];
	uint32_t add_carry;
	uint32_t sub_carry;
	uint32_t add_cout_net;
	uint32_t sub_cout_net;
	uint32_t zero_net;
	uint32_t one_net;
	uint32_t s0;
	uint32_t s1;
	uint32_t s2;
	uint32_t ns0;
	uint32_t ns1;
	uint32_t ns2;
	uint32_t eq_prev;
	uint32_t lt_prev;
	uint32_t gt_prev;

	if (width_bits == 0U || width_bits > 64U)
		{
		return 0;
		}
	if (input_count != (width_bits * 2U) + 3U)
		{
		return 0;
		}
	if (output_count != width_bits + 4U)
		{
		return 0;
		}

	memset(&alu, 0, sizeof(alu));
	memset(add_sum, 0, sizeof(add_sum));
	memset(sub_sum, 0, sizeof(sub_sum));
	memset(and_bits, 0, sizeof(and_bits));
	memset(or_bits, 0, sizeof(or_bits));
	memset(xor_bits, 0, sizeof(xor_bits));
	alu.kind = LXS_STANDARD_ALU_KIND_CORE;
	alu.width_bits = width_bits;
	alu.input_start = nl->source_standard_alu_input_count;
	alu.output_start = nl->source_standard_alu_output_count;
	alu.gate_index_start = nl->source_standard_alu_gate_index_count;

	for (uint32_t i = 0; i < input_count; ++i)
		{
		if (!lxs_push_u32(
				&nl->source_standard_alu_input_net_ids,
				&nl->source_standard_alu_input_count,
				&nl->source_standard_alu_input_cap,
				inputs[i]))
			{
			return 0;
			}
		}

	for (uint32_t i = 0; i < output_count; ++i)
		{
		if (!lxs_push_u32(
				&nl->source_standard_alu_output_net_ids,
				&nl->source_standard_alu_output_count,
				&nl->source_standard_alu_output_cap,
				outputs[i]))
			{
			return 0;
			}
		}

	s0 = inputs[width_bits * 2U + 0U];
	s1 = inputs[width_bits * 2U + 1U];
	s2 = inputs[width_bits * 2U + 2U];
	zero_net = lxs_intern_temp_net(nl, "salu", serial, "zero");
	one_net = lxs_intern_temp_net(nl, "salu", serial, "one");
	ns0 = lxs_intern_temp_net(nl, "salu", serial, "ns0");
	ns1 = lxs_intern_temp_net(nl, "salu", serial, "ns1");
	ns2 = lxs_intern_temp_net(nl, "salu", serial, "ns2");
	add_cout_net = lxs_intern_temp_net(nl, "salu", serial, "add_cout");
	sub_cout_net = lxs_intern_temp_net(nl, "salu", serial, "sub_cout");
	if (zero_net == UINT32_MAX || one_net == UINT32_MAX ||
		ns0 == UINT32_MAX || ns1 == UINT32_MAX || ns2 == UINT32_MAX ||
		add_cout_net == UINT32_MAX || sub_cout_net == UINT32_MAX)
		{
		return 0;
		}

	gate_inputs[0] = zero_net;
	if (!lxs_emit_tracked_gate(
			nl, LXS_GATE_NOT, one_net, gate_inputs, 1U,
			&nl->source_standard_alu_gate_indices,
			&nl->source_standard_alu_gate_index_count,
			&nl->source_standard_alu_gate_index_cap))
		{
		return 0;
		}
	gate_inputs[0] = s0;
	if (!lxs_emit_tracked_gate(
			nl, LXS_GATE_NOT, ns0, gate_inputs, 1U,
			&nl->source_standard_alu_gate_indices,
			&nl->source_standard_alu_gate_index_count,
			&nl->source_standard_alu_gate_index_cap))
		{
		return 0;
		}
	gate_inputs[0] = s1;
	if (!lxs_emit_tracked_gate(
			nl, LXS_GATE_NOT, ns1, gate_inputs, 1U,
			&nl->source_standard_alu_gate_indices,
			&nl->source_standard_alu_gate_index_count,
			&nl->source_standard_alu_gate_index_cap))
		{
		return 0;
		}
	gate_inputs[0] = s2;
	if (!lxs_emit_tracked_gate(
			nl, LXS_GATE_NOT, ns2, gate_inputs, 1U,
			&nl->source_standard_alu_gate_indices,
			&nl->source_standard_alu_gate_index_count,
			&nl->source_standard_alu_gate_index_cap))
		{
		return 0;
		}

	add_carry = zero_net;
	sub_carry = one_net;
	for (uint32_t bit = 0; bit < width_bits; ++bit)
		{
		char suffix[48];
		uint32_t a_net = inputs[bit];
		uint32_t b_net = inputs[width_bits + bit];
		uint32_t not_b;
		uint32_t add_xor;
		uint32_t add_and_ab;
		uint32_t add_and_carry;
		uint32_t sub_xor;
		uint32_t sub_and_ab;
		uint32_t sub_and_carry;
		uint32_t next_add_carry = UINT32_MAX;
		uint32_t next_sub_carry = UINT32_MAX;

		snprintf(suffix, sizeof(suffix), "b%u_nb", bit);
		not_b = lxs_intern_temp_net(nl, "salu", serial, suffix);
		snprintf(suffix, sizeof(suffix), "b%u_ax", bit);
		add_xor = lxs_intern_temp_net(nl, "salu", serial, suffix);
		snprintf(suffix, sizeof(suffix), "b%u_aab", bit);
		add_and_ab = lxs_intern_temp_net(nl, "salu", serial, suffix);
		snprintf(suffix, sizeof(suffix), "b%u_aac", bit);
		add_and_carry = lxs_intern_temp_net(nl, "salu", serial, suffix);
		snprintf(suffix, sizeof(suffix), "b%u_sx", bit);
		sub_xor = lxs_intern_temp_net(nl, "salu", serial, suffix);
		snprintf(suffix, sizeof(suffix), "b%u_sab", bit);
		sub_and_ab = lxs_intern_temp_net(nl, "salu", serial, suffix);
		snprintf(suffix, sizeof(suffix), "b%u_sac", bit);
		sub_and_carry = lxs_intern_temp_net(nl, "salu", serial, suffix);
		snprintf(suffix, sizeof(suffix), "b%u_add", bit);
		add_sum[bit] = lxs_intern_temp_net(nl, "salu", serial, suffix);
		snprintf(suffix, sizeof(suffix), "b%u_sub", bit);
		sub_sum[bit] = lxs_intern_temp_net(nl, "salu", serial, suffix);
		snprintf(suffix, sizeof(suffix), "b%u_and", bit);
		and_bits[bit] = lxs_intern_temp_net(nl, "salu", serial, suffix);
		snprintf(suffix, sizeof(suffix), "b%u_or", bit);
		or_bits[bit] = lxs_intern_temp_net(nl, "salu", serial, suffix);
		snprintf(suffix, sizeof(suffix), "b%u_xor", bit);
		xor_bits[bit] = lxs_intern_temp_net(nl, "salu", serial, suffix);
		if (bit + 1U != width_bits)
			{
			snprintf(suffix, sizeof(suffix), "b%u_aco", bit);
			next_add_carry = lxs_intern_temp_net(nl, "salu", serial, suffix);
			snprintf(suffix, sizeof(suffix), "b%u_sco", bit);
			next_sub_carry = lxs_intern_temp_net(nl, "salu", serial, suffix);
			}
		if (not_b == UINT32_MAX || add_xor == UINT32_MAX || add_and_ab == UINT32_MAX ||
			add_and_carry == UINT32_MAX || sub_xor == UINT32_MAX ||
			sub_and_ab == UINT32_MAX || sub_and_carry == UINT32_MAX ||
			add_sum[bit] == UINT32_MAX || sub_sum[bit] == UINT32_MAX ||
			and_bits[bit] == UINT32_MAX || or_bits[bit] == UINT32_MAX ||
			xor_bits[bit] == UINT32_MAX ||
			(bit + 1U != width_bits && (next_add_carry == UINT32_MAX || next_sub_carry == UINT32_MAX)))
			{
			return 0;
			}

		gate_inputs[0] = b_net;
		if (!lxs_emit_tracked_gate(nl, LXS_GATE_NOT, not_b, gate_inputs, 1U,
				&nl->source_standard_alu_gate_indices,
				&nl->source_standard_alu_gate_index_count,
				&nl->source_standard_alu_gate_index_cap))
			{
			return 0;
			}

		gate_inputs[0] = a_net;
		gate_inputs[1] = b_net;
		if (!lxs_emit_tracked_gate(nl, LXS_GATE_XOR, add_xor, gate_inputs, 2U,
				&nl->source_standard_alu_gate_indices,
				&nl->source_standard_alu_gate_index_count,
				&nl->source_standard_alu_gate_index_cap))
			{
			return 0;
			}
		gate_inputs[0] = add_xor;
		gate_inputs[1] = add_carry;
		if (!lxs_emit_tracked_gate(nl, LXS_GATE_XOR, add_sum[bit], gate_inputs, 2U,
				&nl->source_standard_alu_gate_indices,
				&nl->source_standard_alu_gate_index_count,
				&nl->source_standard_alu_gate_index_cap))
			{
			return 0;
			}
		gate_inputs[0] = a_net;
		gate_inputs[1] = b_net;
		if (!lxs_emit_tracked_gate(nl, LXS_GATE_AND, add_and_ab, gate_inputs, 2U,
				&nl->source_standard_alu_gate_indices,
				&nl->source_standard_alu_gate_index_count,
				&nl->source_standard_alu_gate_index_cap))
			{
			return 0;
			}
		gate_inputs[0] = add_xor;
		gate_inputs[1] = add_carry;
		if (!lxs_emit_tracked_gate(nl, LXS_GATE_AND, add_and_carry, gate_inputs, 2U,
				&nl->source_standard_alu_gate_indices,
				&nl->source_standard_alu_gate_index_count,
				&nl->source_standard_alu_gate_index_cap))
			{
			return 0;
			}
		gate_inputs[0] = add_and_ab;
		gate_inputs[1] = add_and_carry;
		if (!lxs_emit_tracked_gate(
				nl, LXS_GATE_OR,
				(bit + 1U == width_bits) ? add_cout_net : next_add_carry,
				gate_inputs, 2U,
				&nl->source_standard_alu_gate_indices,
				&nl->source_standard_alu_gate_index_count,
				&nl->source_standard_alu_gate_index_cap))
			{
			return 0;
			}

		gate_inputs[0] = a_net;
		gate_inputs[1] = not_b;
		if (!lxs_emit_tracked_gate(nl, LXS_GATE_XOR, sub_xor, gate_inputs, 2U,
				&nl->source_standard_alu_gate_indices,
				&nl->source_standard_alu_gate_index_count,
				&nl->source_standard_alu_gate_index_cap))
			{
			return 0;
			}
		gate_inputs[0] = sub_xor;
		gate_inputs[1] = sub_carry;
		if (!lxs_emit_tracked_gate(nl, LXS_GATE_XOR, sub_sum[bit], gate_inputs, 2U,
				&nl->source_standard_alu_gate_indices,
				&nl->source_standard_alu_gate_index_count,
				&nl->source_standard_alu_gate_index_cap))
			{
			return 0;
			}
		gate_inputs[0] = a_net;
		gate_inputs[1] = not_b;
		if (!lxs_emit_tracked_gate(nl, LXS_GATE_AND, sub_and_ab, gate_inputs, 2U,
				&nl->source_standard_alu_gate_indices,
				&nl->source_standard_alu_gate_index_count,
				&nl->source_standard_alu_gate_index_cap))
			{
			return 0;
			}
		gate_inputs[0] = sub_xor;
		gate_inputs[1] = sub_carry;
		if (!lxs_emit_tracked_gate(nl, LXS_GATE_AND, sub_and_carry, gate_inputs, 2U,
				&nl->source_standard_alu_gate_indices,
				&nl->source_standard_alu_gate_index_count,
				&nl->source_standard_alu_gate_index_cap))
			{
			return 0;
			}
		gate_inputs[0] = sub_and_ab;
		gate_inputs[1] = sub_and_carry;
		if (!lxs_emit_tracked_gate(
				nl, LXS_GATE_OR,
				(bit + 1U == width_bits) ? sub_cout_net : next_sub_carry,
				gate_inputs, 2U,
				&nl->source_standard_alu_gate_indices,
				&nl->source_standard_alu_gate_index_count,
				&nl->source_standard_alu_gate_index_cap))
			{
			return 0;
			}

		gate_inputs[0] = a_net;
		gate_inputs[1] = b_net;
		if (!lxs_emit_tracked_gate(nl, LXS_GATE_AND, and_bits[bit], gate_inputs, 2U,
				&nl->source_standard_alu_gate_indices,
				&nl->source_standard_alu_gate_index_count,
				&nl->source_standard_alu_gate_index_cap) ||
			!lxs_emit_tracked_gate(nl, LXS_GATE_OR, or_bits[bit], gate_inputs, 2U,
				&nl->source_standard_alu_gate_indices,
				&nl->source_standard_alu_gate_index_count,
				&nl->source_standard_alu_gate_index_cap) ||
			!lxs_emit_tracked_gate(nl, LXS_GATE_XOR, xor_bits[bit], gate_inputs, 2U,
				&nl->source_standard_alu_gate_indices,
				&nl->source_standard_alu_gate_index_count,
				&nl->source_standard_alu_gate_index_cap))
			{
			return 0;
			}

		if (bit + 1U != width_bits)
			{
			add_carry = next_add_carry;
			sub_carry = next_sub_carry;
			}
		else
			{
			add_carry = add_cout_net;
			sub_carry = sub_cout_net;
			}
		}

	eq_prev = one_net;
	lt_prev = zero_net;
	gt_prev = zero_net;
	for (uint32_t step = 0; step < width_bits; ++step)
		{
		char suffix[48];
		uint32_t bit = width_bits - 1U - step;
		uint32_t a_net = inputs[bit];
		uint32_t b_net = inputs[width_bits + bit];
		uint32_t not_a;
		uint32_t not_b;
		uint32_t xor_ab;
		uint32_t bit_eq;
		uint32_t lt_raw;
		uint32_t gt_raw;
		uint32_t lt_candidate;
		uint32_t gt_candidate;
		uint32_t eq_next = (bit == 0U) ? outputs[width_bits + 1U] : UINT32_MAX;
		uint32_t lt_next = (bit == 0U) ? outputs[width_bits + 2U] : UINT32_MAX;
		uint32_t gt_next = (bit == 0U) ? outputs[width_bits + 3U] : UINT32_MAX;

		snprintf(suffix, sizeof(suffix), "b%u_cna", bit);
		not_a = lxs_intern_temp_net(nl, "salu", serial, suffix);
		snprintf(suffix, sizeof(suffix), "b%u_cnb", bit);
		not_b = lxs_intern_temp_net(nl, "salu", serial, suffix);
		snprintf(suffix, sizeof(suffix), "b%u_cx", bit);
		xor_ab = lxs_intern_temp_net(nl, "salu", serial, suffix);
		snprintf(suffix, sizeof(suffix), "b%u_ceq", bit);
		bit_eq = lxs_intern_temp_net(nl, "salu", serial, suffix);
		snprintf(suffix, sizeof(suffix), "b%u_clr", bit);
		lt_raw = lxs_intern_temp_net(nl, "salu", serial, suffix);
		snprintf(suffix, sizeof(suffix), "b%u_cgr", bit);
		gt_raw = lxs_intern_temp_net(nl, "salu", serial, suffix);
		snprintf(suffix, sizeof(suffix), "b%u_clc", bit);
		lt_candidate = lxs_intern_temp_net(nl, "salu", serial, suffix);
		snprintf(suffix, sizeof(suffix), "b%u_cgc", bit);
		gt_candidate = lxs_intern_temp_net(nl, "salu", serial, suffix);
		if (bit != 0U)
			{
			snprintf(suffix, sizeof(suffix), "b%u_eqn", bit);
			eq_next = lxs_intern_temp_net(nl, "salu", serial, suffix);
			snprintf(suffix, sizeof(suffix), "b%u_ltn", bit);
			lt_next = lxs_intern_temp_net(nl, "salu", serial, suffix);
			snprintf(suffix, sizeof(suffix), "b%u_gtn", bit);
			gt_next = lxs_intern_temp_net(nl, "salu", serial, suffix);
			}
		if (not_a == UINT32_MAX || not_b == UINT32_MAX || xor_ab == UINT32_MAX ||
			bit_eq == UINT32_MAX || lt_raw == UINT32_MAX || gt_raw == UINT32_MAX ||
			lt_candidate == UINT32_MAX || gt_candidate == UINT32_MAX ||
			eq_next == UINT32_MAX || lt_next == UINT32_MAX || gt_next == UINT32_MAX)
			{
			return 0;
			}

		gate_inputs[0] = a_net;
		if (!lxs_emit_tracked_gate(nl, LXS_GATE_NOT, not_a, gate_inputs, 1U,
				&nl->source_standard_alu_gate_indices,
				&nl->source_standard_alu_gate_index_count,
				&nl->source_standard_alu_gate_index_cap))
			{
			return 0;
			}
		gate_inputs[0] = b_net;
		if (!lxs_emit_tracked_gate(nl, LXS_GATE_NOT, not_b, gate_inputs, 1U,
				&nl->source_standard_alu_gate_indices,
				&nl->source_standard_alu_gate_index_count,
				&nl->source_standard_alu_gate_index_cap))
			{
			return 0;
			}
		gate_inputs[0] = a_net;
		gate_inputs[1] = b_net;
		if (!lxs_emit_tracked_gate(nl, LXS_GATE_XOR, xor_ab, gate_inputs, 2U,
				&nl->source_standard_alu_gate_indices,
				&nl->source_standard_alu_gate_index_count,
				&nl->source_standard_alu_gate_index_cap))
			{
			return 0;
			}
		gate_inputs[0] = xor_ab;
		if (!lxs_emit_tracked_gate(nl, LXS_GATE_NOT, bit_eq, gate_inputs, 1U,
				&nl->source_standard_alu_gate_indices,
				&nl->source_standard_alu_gate_index_count,
				&nl->source_standard_alu_gate_index_cap))
			{
			return 0;
			}
		gate_inputs[0] = not_a;
		gate_inputs[1] = b_net;
		if (!lxs_emit_tracked_gate(nl, LXS_GATE_AND, lt_raw, gate_inputs, 2U,
				&nl->source_standard_alu_gate_indices,
				&nl->source_standard_alu_gate_index_count,
				&nl->source_standard_alu_gate_index_cap))
			{
			return 0;
			}
		gate_inputs[0] = a_net;
		gate_inputs[1] = not_b;
		if (!lxs_emit_tracked_gate(nl, LXS_GATE_AND, gt_raw, gate_inputs, 2U,
				&nl->source_standard_alu_gate_indices,
				&nl->source_standard_alu_gate_index_count,
				&nl->source_standard_alu_gate_index_cap))
			{
			return 0;
			}
		gate_inputs[0] = eq_prev;
		gate_inputs[1] = lt_raw;
		if (!lxs_emit_tracked_gate(nl, LXS_GATE_AND, lt_candidate, gate_inputs, 2U,
				&nl->source_standard_alu_gate_indices,
				&nl->source_standard_alu_gate_index_count,
				&nl->source_standard_alu_gate_index_cap))
			{
			return 0;
			}
		gate_inputs[0] = eq_prev;
		gate_inputs[1] = gt_raw;
		if (!lxs_emit_tracked_gate(nl, LXS_GATE_AND, gt_candidate, gate_inputs, 2U,
				&nl->source_standard_alu_gate_indices,
				&nl->source_standard_alu_gate_index_count,
				&nl->source_standard_alu_gate_index_cap))
			{
			return 0;
			}
		gate_inputs[0] = eq_prev;
		gate_inputs[1] = bit_eq;
		if (!lxs_emit_tracked_gate(nl, LXS_GATE_AND, eq_next, gate_inputs, 2U,
				&nl->source_standard_alu_gate_indices,
				&nl->source_standard_alu_gate_index_count,
				&nl->source_standard_alu_gate_index_cap))
			{
			return 0;
			}
		gate_inputs[0] = lt_prev;
		gate_inputs[1] = lt_candidate;
		if (!lxs_emit_tracked_gate(nl, LXS_GATE_OR, lt_next, gate_inputs, 2U,
				&nl->source_standard_alu_gate_indices,
				&nl->source_standard_alu_gate_index_count,
				&nl->source_standard_alu_gate_index_cap))
			{
			return 0;
			}
		gate_inputs[0] = gt_prev;
		gate_inputs[1] = gt_candidate;
		if (!lxs_emit_tracked_gate(nl, LXS_GATE_OR, gt_next, gate_inputs, 2U,
				&nl->source_standard_alu_gate_indices,
				&nl->source_standard_alu_gate_index_count,
				&nl->source_standard_alu_gate_index_cap))
			{
			return 0;
			}

		eq_prev = eq_next;
		lt_prev = lt_next;
		gt_prev = gt_next;
		}

	for (uint32_t bit = 0; bit < width_bits; ++bit)
		{
		char suffix[48];
		uint32_t low0_a;
		uint32_t low0_b;
		uint32_t low0;
		uint32_t low1_a;
		uint32_t low1_b;
		uint32_t low1;
		uint32_t group0_a;
		uint32_t group0_b;
		uint32_t group0;
		uint32_t high0_a;
		uint32_t high0_b;
		uint32_t high0;
		uint32_t high1_a;
		uint32_t high1_b;
		uint32_t high1;
		uint32_t group1_a;
		uint32_t group1_b;
		uint32_t group1;
		uint32_t final_a;
		uint32_t final_b;

		snprintf(suffix, sizeof(suffix), "b%u_l0a", bit);
		low0_a = lxs_intern_temp_net(nl, "salu", serial, suffix);
		snprintf(suffix, sizeof(suffix), "b%u_l0b", bit);
		low0_b = lxs_intern_temp_net(nl, "salu", serial, suffix);
		snprintf(suffix, sizeof(suffix), "b%u_l0", bit);
		low0 = lxs_intern_temp_net(nl, "salu", serial, suffix);
		snprintf(suffix, sizeof(suffix), "b%u_l1a", bit);
		low1_a = lxs_intern_temp_net(nl, "salu", serial, suffix);
		snprintf(suffix, sizeof(suffix), "b%u_l1b", bit);
		low1_b = lxs_intern_temp_net(nl, "salu", serial, suffix);
		snprintf(suffix, sizeof(suffix), "b%u_l1", bit);
		low1 = lxs_intern_temp_net(nl, "salu", serial, suffix);
		snprintf(suffix, sizeof(suffix), "b%u_g0a", bit);
		group0_a = lxs_intern_temp_net(nl, "salu", serial, suffix);
		snprintf(suffix, sizeof(suffix), "b%u_g0b", bit);
		group0_b = lxs_intern_temp_net(nl, "salu", serial, suffix);
		snprintf(suffix, sizeof(suffix), "b%u_g0", bit);
		group0 = lxs_intern_temp_net(nl, "salu", serial, suffix);
		snprintf(suffix, sizeof(suffix), "b%u_h0a", bit);
		high0_a = lxs_intern_temp_net(nl, "salu", serial, suffix);
		snprintf(suffix, sizeof(suffix), "b%u_h0b", bit);
		high0_b = lxs_intern_temp_net(nl, "salu", serial, suffix);
		snprintf(suffix, sizeof(suffix), "b%u_h0", bit);
		high0 = lxs_intern_temp_net(nl, "salu", serial, suffix);
		snprintf(suffix, sizeof(suffix), "b%u_h1a", bit);
		high1_a = lxs_intern_temp_net(nl, "salu", serial, suffix);
		snprintf(suffix, sizeof(suffix), "b%u_h1b", bit);
		high1_b = lxs_intern_temp_net(nl, "salu", serial, suffix);
		snprintf(suffix, sizeof(suffix), "b%u_h1", bit);
		high1 = lxs_intern_temp_net(nl, "salu", serial, suffix);
		snprintf(suffix, sizeof(suffix), "b%u_g1a", bit);
		group1_a = lxs_intern_temp_net(nl, "salu", serial, suffix);
		snprintf(suffix, sizeof(suffix), "b%u_g1b", bit);
		group1_b = lxs_intern_temp_net(nl, "salu", serial, suffix);
		snprintf(suffix, sizeof(suffix), "b%u_g1", bit);
		group1 = lxs_intern_temp_net(nl, "salu", serial, suffix);
		snprintf(suffix, sizeof(suffix), "b%u_fa", bit);
		final_a = lxs_intern_temp_net(nl, "salu", serial, suffix);
		snprintf(suffix, sizeof(suffix), "b%u_fb", bit);
		final_b = lxs_intern_temp_net(nl, "salu", serial, suffix);
		if (low0_a == UINT32_MAX || low0_b == UINT32_MAX || low0 == UINT32_MAX ||
			low1_a == UINT32_MAX || low1_b == UINT32_MAX || low1 == UINT32_MAX ||
			group0_a == UINT32_MAX || group0_b == UINT32_MAX || group0 == UINT32_MAX ||
			high0_a == UINT32_MAX || high0_b == UINT32_MAX || high0 == UINT32_MAX ||
			high1_a == UINT32_MAX || high1_b == UINT32_MAX || high1 == UINT32_MAX ||
			group1_a == UINT32_MAX || group1_b == UINT32_MAX || group1 == UINT32_MAX ||
			final_a == UINT32_MAX || final_b == UINT32_MAX)
			{
			return 0;
			}

		gate_inputs[0] = add_sum[bit];
		gate_inputs[1] = ns0;
		if (!lxs_emit_tracked_gate(nl, LXS_GATE_AND, low0_a, gate_inputs, 2U,
				&nl->source_standard_alu_gate_indices,
				&nl->source_standard_alu_gate_index_count,
				&nl->source_standard_alu_gate_index_cap))
			{
			return 0;
			}
		gate_inputs[0] = sub_sum[bit];
		gate_inputs[1] = s0;
		if (!lxs_emit_tracked_gate(nl, LXS_GATE_AND, low0_b, gate_inputs, 2U,
				&nl->source_standard_alu_gate_indices,
				&nl->source_standard_alu_gate_index_count,
				&nl->source_standard_alu_gate_index_cap) ||
			!lxs_emit_tracked_gate(nl, LXS_GATE_OR, low0, (uint32_t[]){ low0_a, low0_b }, 2U,
				&nl->source_standard_alu_gate_indices,
				&nl->source_standard_alu_gate_index_count,
				&nl->source_standard_alu_gate_index_cap))
			{
			return 0;
			}

		gate_inputs[0] = and_bits[bit];
		gate_inputs[1] = ns0;
		if (!lxs_emit_tracked_gate(nl, LXS_GATE_AND, low1_a, gate_inputs, 2U,
				&nl->source_standard_alu_gate_indices,
				&nl->source_standard_alu_gate_index_count,
				&nl->source_standard_alu_gate_index_cap))
			{
			return 0;
			}
		gate_inputs[0] = or_bits[bit];
		gate_inputs[1] = s0;
		if (!lxs_emit_tracked_gate(nl, LXS_GATE_AND, low1_b, gate_inputs, 2U,
				&nl->source_standard_alu_gate_indices,
				&nl->source_standard_alu_gate_index_count,
				&nl->source_standard_alu_gate_index_cap) ||
			!lxs_emit_tracked_gate(nl, LXS_GATE_OR, low1, (uint32_t[]){ low1_a, low1_b }, 2U,
				&nl->source_standard_alu_gate_indices,
				&nl->source_standard_alu_gate_index_count,
				&nl->source_standard_alu_gate_index_cap))
			{
			return 0;
			}

		gate_inputs[0] = low0;
		gate_inputs[1] = ns1;
		if (!lxs_emit_tracked_gate(nl, LXS_GATE_AND, group0_a, gate_inputs, 2U,
				&nl->source_standard_alu_gate_indices,
				&nl->source_standard_alu_gate_index_count,
				&nl->source_standard_alu_gate_index_cap))
			{
			return 0;
			}
		gate_inputs[0] = low1;
		gate_inputs[1] = s1;
		if (!lxs_emit_tracked_gate(nl, LXS_GATE_AND, group0_b, gate_inputs, 2U,
				&nl->source_standard_alu_gate_indices,
				&nl->source_standard_alu_gate_index_count,
				&nl->source_standard_alu_gate_index_cap) ||
			!lxs_emit_tracked_gate(nl, LXS_GATE_OR, group0, (uint32_t[]){ group0_a, group0_b }, 2U,
				&nl->source_standard_alu_gate_indices,
				&nl->source_standard_alu_gate_index_count,
				&nl->source_standard_alu_gate_index_cap))
			{
			return 0;
			}

		gate_inputs[0] = xor_bits[bit];
		gate_inputs[1] = ns0;
		if (!lxs_emit_tracked_gate(nl, LXS_GATE_AND, high0_a, gate_inputs, 2U,
				&nl->source_standard_alu_gate_indices,
				&nl->source_standard_alu_gate_index_count,
				&nl->source_standard_alu_gate_index_cap))
			{
			return 0;
			}
		gate_inputs[0] = inputs[bit];
		gate_inputs[1] = s0;
		if (!lxs_emit_tracked_gate(nl, LXS_GATE_AND, high0_b, gate_inputs, 2U,
				&nl->source_standard_alu_gate_indices,
				&nl->source_standard_alu_gate_index_count,
				&nl->source_standard_alu_gate_index_cap) ||
			!lxs_emit_tracked_gate(nl, LXS_GATE_OR, high0, (uint32_t[]){ high0_a, high0_b }, 2U,
				&nl->source_standard_alu_gate_indices,
				&nl->source_standard_alu_gate_index_count,
				&nl->source_standard_alu_gate_index_cap))
			{
			return 0;
			}

		gate_inputs[0] = inputs[width_bits + bit];
		gate_inputs[1] = ns0;
		if (!lxs_emit_tracked_gate(nl, LXS_GATE_AND, high1_a, gate_inputs, 2U,
				&nl->source_standard_alu_gate_indices,
				&nl->source_standard_alu_gate_index_count,
				&nl->source_standard_alu_gate_index_cap))
			{
			return 0;
			}
		gate_inputs[0] = zero_net;
		gate_inputs[1] = s0;
		if (!lxs_emit_tracked_gate(nl, LXS_GATE_AND, high1_b, gate_inputs, 2U,
				&nl->source_standard_alu_gate_indices,
				&nl->source_standard_alu_gate_index_count,
				&nl->source_standard_alu_gate_index_cap) ||
			!lxs_emit_tracked_gate(nl, LXS_GATE_OR, high1, (uint32_t[]){ high1_a, high1_b }, 2U,
				&nl->source_standard_alu_gate_indices,
				&nl->source_standard_alu_gate_index_count,
				&nl->source_standard_alu_gate_index_cap))
			{
			return 0;
			}

		gate_inputs[0] = high0;
		gate_inputs[1] = ns1;
		if (!lxs_emit_tracked_gate(nl, LXS_GATE_AND, group1_a, gate_inputs, 2U,
				&nl->source_standard_alu_gate_indices,
				&nl->source_standard_alu_gate_index_count,
				&nl->source_standard_alu_gate_index_cap))
			{
			return 0;
			}
		gate_inputs[0] = high1;
		gate_inputs[1] = s1;
		if (!lxs_emit_tracked_gate(nl, LXS_GATE_AND, group1_b, gate_inputs, 2U,
				&nl->source_standard_alu_gate_indices,
				&nl->source_standard_alu_gate_index_count,
				&nl->source_standard_alu_gate_index_cap) ||
			!lxs_emit_tracked_gate(nl, LXS_GATE_OR, group1, (uint32_t[]){ group1_a, group1_b }, 2U,
				&nl->source_standard_alu_gate_indices,
				&nl->source_standard_alu_gate_index_count,
				&nl->source_standard_alu_gate_index_cap))
			{
			return 0;
			}

		gate_inputs[0] = group0;
		gate_inputs[1] = ns2;
		if (!lxs_emit_tracked_gate(nl, LXS_GATE_AND, final_a, gate_inputs, 2U,
				&nl->source_standard_alu_gate_indices,
				&nl->source_standard_alu_gate_index_count,
				&nl->source_standard_alu_gate_index_cap))
			{
			return 0;
			}
		gate_inputs[0] = group1;
		gate_inputs[1] = s2;
		if (!lxs_emit_tracked_gate(nl, LXS_GATE_AND, final_b, gate_inputs, 2U,
				&nl->source_standard_alu_gate_indices,
				&nl->source_standard_alu_gate_index_count,
				&nl->source_standard_alu_gate_index_cap))
			{
			return 0;
			}
		if (!lxs_emit_tracked_gate(nl, LXS_GATE_OR, outputs[bit], (uint32_t[]){ final_a, final_b }, 2U,
				&nl->source_standard_alu_gate_indices,
				&nl->source_standard_alu_gate_index_count,
				&nl->source_standard_alu_gate_index_cap))
			{
			return 0;
			}
		}

	{
	uint32_t low0_a;
	uint32_t low0_b;
	uint32_t low0;
	uint32_t group0_a;
	uint32_t group0_b;
	uint32_t group0;
	uint32_t final_a;
	uint32_t final_b;
	low0_a = lxs_intern_temp_net(nl, "salu", serial, "cout_l0a");
	low0_b = lxs_intern_temp_net(nl, "salu", serial, "cout_l0b");
	low0 = lxs_intern_temp_net(nl, "salu", serial, "cout_l0");
	group0_a = lxs_intern_temp_net(nl, "salu", serial, "cout_g0a");
	group0_b = lxs_intern_temp_net(nl, "salu", serial, "cout_g0b");
	group0 = lxs_intern_temp_net(nl, "salu", serial, "cout_g0");
	final_a = lxs_intern_temp_net(nl, "salu", serial, "cout_fa");
	final_b = lxs_intern_temp_net(nl, "salu", serial, "cout_fb");
	if (low0_a == UINT32_MAX || low0_b == UINT32_MAX || low0 == UINT32_MAX ||
		group0_a == UINT32_MAX || group0_b == UINT32_MAX || group0 == UINT32_MAX ||
		final_a == UINT32_MAX || final_b == UINT32_MAX)
		{
		return 0;
		}

	gate_inputs[0] = add_cout_net;
	gate_inputs[1] = ns0;
	if (!lxs_emit_tracked_gate(nl, LXS_GATE_AND, low0_a, gate_inputs, 2U,
			&nl->source_standard_alu_gate_indices,
			&nl->source_standard_alu_gate_index_count,
			&nl->source_standard_alu_gate_index_cap))
		{
		return 0;
		}
	gate_inputs[0] = sub_cout_net;
	gate_inputs[1] = s0;
	if (!lxs_emit_tracked_gate(nl, LXS_GATE_AND, low0_b, gate_inputs, 2U,
			&nl->source_standard_alu_gate_indices,
			&nl->source_standard_alu_gate_index_count,
			&nl->source_standard_alu_gate_index_cap))
		{
		return 0;
		}
	if (!lxs_emit_tracked_gate(nl, LXS_GATE_OR, low0, (uint32_t[]){ low0_a, low0_b }, 2U,
			&nl->source_standard_alu_gate_indices,
			&nl->source_standard_alu_gate_index_count,
			&nl->source_standard_alu_gate_index_cap))
		{
		return 0;
		}
	gate_inputs[0] = low0;
	gate_inputs[1] = ns1;
	if (!lxs_emit_tracked_gate(nl, LXS_GATE_AND, group0_a, gate_inputs, 2U,
			&nl->source_standard_alu_gate_indices,
			&nl->source_standard_alu_gate_index_count,
			&nl->source_standard_alu_gate_index_cap))
		{
		return 0;
		}
	gate_inputs[0] = zero_net;
	gate_inputs[1] = s1;
	if (!lxs_emit_tracked_gate(nl, LXS_GATE_AND, group0_b, gate_inputs, 2U,
			&nl->source_standard_alu_gate_indices,
			&nl->source_standard_alu_gate_index_count,
			&nl->source_standard_alu_gate_index_cap))
		{
		return 0;
		}
	if (!lxs_emit_tracked_gate(nl, LXS_GATE_OR, group0, (uint32_t[]){ group0_a, group0_b }, 2U,
			&nl->source_standard_alu_gate_indices,
			&nl->source_standard_alu_gate_index_count,
			&nl->source_standard_alu_gate_index_cap))
		{
		return 0;
		}
	gate_inputs[0] = group0;
	gate_inputs[1] = ns2;
	if (!lxs_emit_tracked_gate(nl, LXS_GATE_AND, final_a, gate_inputs, 2U,
			&nl->source_standard_alu_gate_indices,
			&nl->source_standard_alu_gate_index_count,
			&nl->source_standard_alu_gate_index_cap))
		{
		return 0;
		}
	gate_inputs[0] = zero_net;
	gate_inputs[1] = s2;
	if (!lxs_emit_tracked_gate(nl, LXS_GATE_AND, final_b, gate_inputs, 2U,
			&nl->source_standard_alu_gate_indices,
			&nl->source_standard_alu_gate_index_count,
			&nl->source_standard_alu_gate_index_cap))
		{
		return 0;
		}
	if (!lxs_emit_tracked_gate(nl, LXS_GATE_OR, outputs[width_bits], (uint32_t[]){ final_a, final_b }, 2U,
			&nl->source_standard_alu_gate_indices,
			&nl->source_standard_alu_gate_index_count,
			&nl->source_standard_alu_gate_index_cap))
		{
		return 0;
		}
	}

	alu.gate_count = nl->source_standard_alu_gate_index_count - alu.gate_index_start;
	return lxs_push_source_standard_alu(nl, &alu);
	}

static int lxs_emit_register_descriptor(
	lxs_netlist *nl,
	const uint32_t *outputs,
	const uint32_t *inputs,
	uint32_t width_bits,
	uint32_t control_net,
	uint8_t control_invert)
	{
	lxs_source_register reg;

	memset(&reg, 0, sizeof(reg));
	reg.width_bits = width_bits;
	reg.input_start = nl->source_register_input_count;
	reg.output_start = nl->source_register_output_count;
	reg.control_net = control_net;
	reg.aux_control_net = UINT32_MAX;
	reg.mode = control_net == UINT32_MAX ? LXS_REGISTER_MODE_PLAIN :
		(control_invert ? LXS_REGISTER_MODE_HOLD : LXS_REGISTER_MODE_ENABLE);
	reg.control_invert = control_invert;

	for (uint32_t i = 0; i < width_bits; ++i)
		{
		if (!lxs_push_u32(
				&nl->source_register_input_net_ids,
				&nl->source_register_input_count,
				&nl->source_register_input_cap,
				inputs[i]) ||
			!lxs_push_u32(
				&nl->source_register_output_net_ids,
				&nl->source_register_output_count,
				&nl->source_register_output_cap,
				outputs[i]))
			{
			return 0;
			}
		}

	return lxs_push_source_register(nl, &reg);
	}

static int lxs_emit_register_en_rst_descriptor(
	lxs_netlist *nl,
	const uint32_t *outputs,
	const uint32_t *inputs,
	uint32_t width_bits,
	uint32_t enable_net,
	uint32_t reset_net)
	{
	lxs_source_register reg;

	memset(&reg, 0, sizeof(reg));
	reg.width_bits = width_bits;
	reg.input_start = nl->source_register_input_count;
	reg.output_start = nl->source_register_output_count;
	reg.control_net = enable_net;
	reg.aux_control_net = reset_net;
	reg.mode = LXS_REGISTER_MODE_ENABLE_RESET;
	reg.control_invert = 0U;

	for (uint32_t i = 0; i < width_bits; ++i)
		{
		if (!lxs_push_u32(
				&nl->source_register_input_net_ids,
				&nl->source_register_input_count,
				&nl->source_register_input_cap,
				inputs[i]) ||
			!lxs_push_u32(
				&nl->source_register_output_net_ids,
				&nl->source_register_output_count,
				&nl->source_register_output_cap,
				outputs[i]))
			{
			return 0;
			}
		}

	return lxs_push_source_register(nl, &reg);
	}

static int lxs_emit_counter_descriptor(
	lxs_netlist *nl,
	const uint32_t *outputs,
	uint32_t width_bits,
	uint32_t enable_net,
	uint32_t down_net,
	uint32_t mode)
	{
	lxs_source_register reg;

	memset(&reg, 0, sizeof(reg));
	reg.width_bits = width_bits;
	reg.input_start = nl->source_register_input_count;
	reg.output_start = nl->source_register_output_count;
	reg.control_net = enable_net;
	reg.aux_control_net = down_net;
	reg.mode = mode;
	reg.control_invert = 0U;

	for (uint32_t i = 0; i < width_bits; ++i)
		{
		if (!lxs_push_u32(
				&nl->source_register_output_net_ids,
				&nl->source_register_output_count,
				&nl->source_register_output_cap,
				outputs[i]))
			{
			return 0;
			}
		}

	return lxs_push_source_register(nl, &reg);
	}

static int lxs_emit_rom_descriptor(
	lxs_netlist *nl,
	const uint32_t *outputs,
	uint32_t output_count,
	const uint32_t *addr_inputs,
	uint32_t addr_width,
	char **data_tokens,
	uint32_t data_token_count)
	{
	lxs_source_rom rom;
	uint64_t values[64];
	uint64_t masks[64];

	if (output_count == 0U || output_count > 64U)
		{
		return 0;
		}

	if (data_token_count != (1U << addr_width))
		{
		return 0;
		}

	memset(&rom, 0, sizeof(rom));
	rom.addr_width = addr_width;
	rom.data_width = output_count;
	rom.depth = data_token_count;
	rom.addr_start = nl->source_rom_addr_count;
	rom.output_start = nl->source_rom_output_count;
	rom.data_start = nl->source_rom_init_count;

	for (uint32_t i = 0; i < addr_width; ++i)
		{
		if (!lxs_push_u32(
				&nl->source_rom_addr_net_ids,
				&nl->source_rom_addr_count,
				&nl->source_rom_addr_cap,
				addr_inputs[i]))
			{
			return 0;
			}
		}

	for (uint32_t i = 0; i < output_count; ++i)
		{
		if (!lxs_push_u32(
				&nl->source_rom_output_net_ids,
				&nl->source_rom_output_count,
				&nl->source_rom_output_cap,
				outputs[i]))
			{
			return 0;
			}
		}

	for (uint32_t word = 0; word < data_token_count; ++word)
		{
		if (!lxs_parse_literal_bits(data_tokens[word], output_count, values, masks))
			{
			return 0;
			}
		for (uint32_t bit = 0; bit < output_count; ++bit)
			{
			if (!lxs_push_u64(
					&nl->source_rom_init_value,
					&nl->source_rom_init_count,
					&nl->source_rom_init_cap,
					values[bit]) ||
				!lxs_push_u64(
					&nl->source_rom_init_mask,
					&nl->source_rom_mask_count,
					&nl->source_rom_mask_cap,
					masks[bit]))
				{
				return 0;
				}
			}
		}

	return lxs_push_source_rom(nl, &rom);
	}

static int lxs_emit_ram_descriptor(
	lxs_netlist *nl,
	const uint32_t *outputs,
	uint32_t output_count,
	const uint32_t *read_addr_inputs,
	uint32_t read_addr_count,
	const uint32_t *write_addr_inputs,
	uint32_t write_addr_count,
	const uint32_t *data_inputs,
	uint32_t data_input_count,
	uint32_t write_enable_net,
	char **data_tokens,
	uint32_t data_token_count)
	{
	lxs_source_ram ram;
	uint64_t values[64];
	uint64_t masks[64];

	if (output_count == 0U || output_count > 64U)
		{
		return 0;
		}
	if (read_addr_count != write_addr_count || output_count != data_input_count)
		{
		return 0;
		}
	if (data_token_count != (1U << read_addr_count))
		{
		return 0;
		}

	memset(&ram, 0, sizeof(ram));
	ram.addr_width = read_addr_count;
	ram.data_width = output_count;
	ram.depth = data_token_count;
	ram.read_addr_start = nl->source_ram_read_addr_count;
	ram.write_addr_start = nl->source_ram_write_addr_count;
	ram.data_input_start = nl->source_ram_data_input_count;
	ram.output_start = nl->source_ram_output_count;
	ram.data_start = nl->source_ram_init_count;
	ram.write_enable_net = write_enable_net;

	for (uint32_t i = 0; i < read_addr_count; ++i)
		{
		if (!lxs_push_u32(
				&nl->source_ram_read_addr_net_ids,
				&nl->source_ram_read_addr_count,
				&nl->source_ram_read_addr_cap,
				read_addr_inputs[i]) ||
			!lxs_push_u32(
				&nl->source_ram_write_addr_net_ids,
				&nl->source_ram_write_addr_count,
				&nl->source_ram_write_addr_cap,
				write_addr_inputs[i]))
			{
			return 0;
			}
		}

	for (uint32_t i = 0; i < data_input_count; ++i)
		{
		if (!lxs_push_u32(
				&nl->source_ram_data_input_net_ids,
				&nl->source_ram_data_input_count,
				&nl->source_ram_data_input_cap,
				data_inputs[i]) ||
			!lxs_push_u32(
				&nl->source_ram_output_net_ids,
				&nl->source_ram_output_count,
				&nl->source_ram_output_cap,
				outputs[i]))
			{
			return 0;
			}
		}

	for (uint32_t word = 0; word < data_token_count; ++word)
		{
		if (!lxs_parse_literal_bits(data_tokens[word], output_count, values, masks))
			{
			return 0;
			}
		for (uint32_t bit = 0; bit < output_count; ++bit)
			{
			if (!lxs_push_u64(
					&nl->source_ram_init_value,
					&nl->source_ram_init_count,
					&nl->source_ram_init_cap,
					values[bit]) ||
				!lxs_push_u64(
					&nl->source_ram_init_mask,
					&nl->source_ram_mask_count,
					&nl->source_ram_mask_cap,
					masks[bit]))
				{
				return 0;
				}
			}
		}

	return lxs_push_source_ram(nl, &ram);
	}

static int lxs_emit_regfile_descriptor(
	lxs_netlist *nl,
	const uint32_t *outputs,
	uint32_t output_count,
	const uint32_t *read_a_addr_inputs,
	uint32_t read_a_addr_count,
	const uint32_t *read_b_addr_inputs,
	uint32_t read_b_addr_count,
	const uint32_t *write_addr_inputs,
	uint32_t write_addr_count,
	const uint32_t *data_inputs,
	uint32_t data_input_count,
	uint32_t write_enable_net,
	char **data_tokens,
	uint32_t data_token_count)
	{
	lxs_source_regfile regfile;
	uint64_t values[64];
	uint64_t masks[64];
	uint32_t data_width;

	if (output_count == 0U || output_count > 128U || (output_count & 1U) != 0U)
		{
		return 0;
		}

	data_width = output_count / 2U;
	if (data_width == 0U || data_width > 64U)
		{
		return 0;
		}
	if (read_a_addr_count != read_b_addr_count ||
		read_a_addr_count != write_addr_count ||
		data_input_count != data_width)
		{
		return 0;
		}
	if (data_token_count != (1U << read_a_addr_count))
		{
		return 0;
		}

	memset(&regfile, 0, sizeof(regfile));
	regfile.addr_width = read_a_addr_count;
	regfile.data_width = data_width;
	regfile.depth = data_token_count;
	regfile.read_a_addr_start = nl->source_regfile_read_a_addr_count;
	regfile.read_b_addr_start = nl->source_regfile_read_b_addr_count;
	regfile.write_addr_start = nl->source_regfile_write_addr_count;
	regfile.data_input_start = nl->source_regfile_data_input_count;
	regfile.output_start = nl->source_regfile_output_count;
	regfile.data_start = nl->source_regfile_init_count;
	regfile.write_enable_net = write_enable_net;

	for (uint32_t i = 0; i < read_a_addr_count; ++i)
		{
		if (!lxs_push_u32(
				&nl->source_regfile_read_a_addr_net_ids,
				&nl->source_regfile_read_a_addr_count,
				&nl->source_regfile_read_a_addr_cap,
				read_a_addr_inputs[i]) ||
			!lxs_push_u32(
				&nl->source_regfile_read_b_addr_net_ids,
				&nl->source_regfile_read_b_addr_count,
				&nl->source_regfile_read_b_addr_cap,
				read_b_addr_inputs[i]) ||
			!lxs_push_u32(
				&nl->source_regfile_write_addr_net_ids,
				&nl->source_regfile_write_addr_count,
				&nl->source_regfile_write_addr_cap,
				write_addr_inputs[i]))
			{
			return 0;
			}
		}

	for (uint32_t i = 0; i < data_input_count; ++i)
		{
		if (!lxs_push_u32(
				&nl->source_regfile_data_input_net_ids,
				&nl->source_regfile_data_input_count,
				&nl->source_regfile_data_input_cap,
				data_inputs[i]))
			{
			return 0;
			}
		}

	for (uint32_t i = 0; i < output_count; ++i)
		{
		if (!lxs_push_u32(
				&nl->source_regfile_output_net_ids,
				&nl->source_regfile_output_count,
				&nl->source_regfile_output_cap,
				outputs[i]))
			{
			return 0;
			}
		}

	for (uint32_t word = 0; word < data_token_count; ++word)
		{
		if (!lxs_parse_literal_bits(data_tokens[word], data_width, values, masks))
			{
			return 0;
			}
		for (uint32_t bit = 0; bit < data_width; ++bit)
			{
			if (!lxs_push_u64(
					&nl->source_regfile_init_value,
					&nl->source_regfile_init_count,
					&nl->source_regfile_init_cap,
					values[bit]) ||
				!lxs_push_u64(
					&nl->source_regfile_init_mask,
					&nl->source_regfile_mask_count,
					&nl->source_regfile_mask_cap,
					masks[bit]))
				{
				return 0;
				}
			}
		}

	return lxs_push_source_regfile(nl, &regfile);
	}

static int lxs_emit_half_adder_macro(
	lxs_netlist *nl,
	const uint32_t *outputs,
	const uint32_t *inputs)
	{
	lxs_source_multi_macro macro;
	uint32_t gate_inputs[2];

	memset(&macro, 0, sizeof(macro));
	macro.type = LXS_SOURCE_MULTI_MACRO_HALF_ADDER;
	macro.input_count = 2U;
	macro.output_count = 2U;
	macro.gate_count = 2U;
	macro.inputs[0] = inputs[0];
	macro.inputs[1] = inputs[1];
	macro.outputs[0] = outputs[0];
	macro.outputs[1] = outputs[1];

	gate_inputs[0] = inputs[0];
	gate_inputs[1] = inputs[1];
	if (!lxs_emit_gate_record(nl, LXS_GATE_XOR, outputs[0], gate_inputs, 2U, &macro.gate_indices[0]) ||
		!lxs_emit_gate_record(nl, LXS_GATE_AND, outputs[1], gate_inputs, 2U, &macro.gate_indices[1]) ||
		!lxs_push_source_multi_macro(nl, &macro))
		{
		return 0;
		}

	return 1;
	}

static int lxs_emit_parity4_macro(
	lxs_netlist *nl,
	uint32_t output,
	const uint32_t *inputs,
	uint32_t serial)
	{
	lxs_source_macro macro;
	uint32_t gate_inputs[2];
	uint32_t xor_ab;
	uint32_t xor_cd;

	memset(&macro, 0, sizeof(macro));
	macro.type = LXS_SOURCE_MACRO_PARITY4;
	macro.input_count = 4U;
	macro.gate_count = 3U;
	for (uint32_t i = 0; i < 4U; ++i)
		{
		macro.inputs[i] = inputs[i];
		}
	macro.output = output;

	xor_ab = lxs_intern_temp_net(nl, "p4", serial, "xor_ab");
	xor_cd = lxs_intern_temp_net(nl, "p4", serial, "xor_cd");
	if (xor_ab == UINT32_MAX || xor_cd == UINT32_MAX)
		{
		return 0;
		}

	gate_inputs[0] = inputs[0];
	gate_inputs[1] = inputs[1];
	if (!lxs_emit_gate_record(nl, LXS_GATE_XOR, xor_ab, gate_inputs, 2U, &macro.gate_indices[0]))
		{
		return 0;
		}

	gate_inputs[0] = inputs[2];
	gate_inputs[1] = inputs[3];
	if (!lxs_emit_gate_record(nl, LXS_GATE_XOR, xor_cd, gate_inputs, 2U, &macro.gate_indices[1]))
		{
		return 0;
		}

	gate_inputs[0] = xor_ab;
	gate_inputs[1] = xor_cd;
	if (!lxs_emit_gate_record(nl, LXS_GATE_XOR, output, gate_inputs, 2U, &macro.gate_indices[2]) ||
		!lxs_push_source_macro(nl, &macro))
		{
		return 0;
		}

	return 1;
	}

static int lxs_emit_parity8_macro(
	lxs_netlist *nl,
	uint32_t output,
	const uint32_t *inputs,
	uint32_t serial)
	{
	lxs_source_macro macro;
	uint32_t gate_inputs[2];
	uint32_t xor01;
	uint32_t xor23;
	uint32_t xor45;
	uint32_t xor67;
	uint32_t xor0123;
	uint32_t xor4567;

	memset(&macro, 0, sizeof(macro));
	macro.type = LXS_SOURCE_MACRO_PARITY8;
	macro.input_count = 8U;
	macro.gate_count = 7U;
	for (uint32_t i = 0; i < 8U; ++i)
		{
		macro.inputs[i] = inputs[i];
		}
	macro.output = output;

	xor01 = lxs_intern_temp_net(nl, "p8", serial, "xor01");
	xor23 = lxs_intern_temp_net(nl, "p8", serial, "xor23");
	xor45 = lxs_intern_temp_net(nl, "p8", serial, "xor45");
	xor67 = lxs_intern_temp_net(nl, "p8", serial, "xor67");
	xor0123 = lxs_intern_temp_net(nl, "p8", serial, "xor0123");
	xor4567 = lxs_intern_temp_net(nl, "p8", serial, "xor4567");
	if (xor01 == UINT32_MAX || xor23 == UINT32_MAX || xor45 == UINT32_MAX || xor67 == UINT32_MAX ||
		xor0123 == UINT32_MAX || xor4567 == UINT32_MAX)
		{
		return 0;
		}

	gate_inputs[0] = inputs[0];
	gate_inputs[1] = inputs[1];
	if (!lxs_emit_gate_record(nl, LXS_GATE_XOR, xor01, gate_inputs, 2U, &macro.gate_indices[0]))
		{
		return 0;
		}

	gate_inputs[0] = inputs[2];
	gate_inputs[1] = inputs[3];
	if (!lxs_emit_gate_record(nl, LXS_GATE_XOR, xor23, gate_inputs, 2U, &macro.gate_indices[1]))
		{
		return 0;
		}

	gate_inputs[0] = inputs[4];
	gate_inputs[1] = inputs[5];
	if (!lxs_emit_gate_record(nl, LXS_GATE_XOR, xor45, gate_inputs, 2U, &macro.gate_indices[2]))
		{
		return 0;
		}

	gate_inputs[0] = inputs[6];
	gate_inputs[1] = inputs[7];
	if (!lxs_emit_gate_record(nl, LXS_GATE_XOR, xor67, gate_inputs, 2U, &macro.gate_indices[3]))
		{
		return 0;
		}

	gate_inputs[0] = xor01;
	gate_inputs[1] = xor23;
	if (!lxs_emit_gate_record(nl, LXS_GATE_XOR, xor0123, gate_inputs, 2U, &macro.gate_indices[4]))
		{
		return 0;
		}

	gate_inputs[0] = xor45;
	gate_inputs[1] = xor67;
	if (!lxs_emit_gate_record(nl, LXS_GATE_XOR, xor4567, gate_inputs, 2U, &macro.gate_indices[5]))
		{
		return 0;
		}

	gate_inputs[0] = xor0123;
	gate_inputs[1] = xor4567;
	if (!lxs_emit_gate_record(nl, LXS_GATE_XOR, output, gate_inputs, 2U, &macro.gate_indices[6]) ||
		!lxs_push_source_macro(nl, &macro))
		{
		return 0;
		}

	return 1;
	}

static int lxs_functional_op_gate_type(uint8_t type, uint32_t *gate_type_out)
	{
	switch (type)
		{
		case LXS_FUNCTIONAL_REGION_OP_BUF:
			*gate_type_out = LXS_GATE_BUF;
			return 1;
		case LXS_FUNCTIONAL_REGION_OP_NOT:
			*gate_type_out = LXS_GATE_NOT;
			return 1;
		case LXS_FUNCTIONAL_REGION_OP_AND:
			*gate_type_out = LXS_GATE_AND;
			return 1;
		case LXS_FUNCTIONAL_REGION_OP_OR:
			*gate_type_out = LXS_GATE_OR;
			return 1;
		case LXS_FUNCTIONAL_REGION_OP_XOR:
			*gate_type_out = LXS_GATE_XOR;
			return 1;
		case LXS_FUNCTIONAL_REGION_OP_NAND:
			*gate_type_out = LXS_GATE_NAND;
			return 1;
		case LXS_FUNCTIONAL_REGION_OP_NOR:
			*gate_type_out = LXS_GATE_NOR;
			return 1;
		case LXS_FUNCTIONAL_REGION_OP_XNOR:
			*gate_type_out = LXS_GATE_XNOR;
			return 1;
		case LXS_FUNCTIONAL_REGION_OP_MAJ3:
			return 0;
		default:
			return 0;
		}
	}

static uint32_t lxs_choose_functional_exec_kind(uint32_t op_count, uint32_t temp_count)
	{
	(void)op_count;
	(void)temp_count;
	return LXS_FUNCTIONAL_REGION_EXEC_MICROPROGRAM;
	}

static int lxs_emit_functional_region(
	lxs_netlist *nl,
	uint32_t exec_kind,
	const uint32_t *inputs,
	uint32_t input_count,
	const uint32_t *outputs,
	uint32_t output_count,
	const uint8_t *op_type,
	const uint8_t *op_dst_count,
	const uint8_t op_dst_kind[LXS_FUNCTIONAL_REGION_MAX_OPS][2],
	const uint8_t op_dst[LXS_FUNCTIONAL_REGION_MAX_OPS][2],
	const uint8_t *op_arity,
	const uint8_t op_src[LXS_FUNCTIONAL_REGION_MAX_OPS][4],
	uint32_t temp_count,
	uint32_t op_count,
	uint32_t serial)
	{
	lxs_source_functional_region region;
	uint32_t temp_nets[LXS_FUNCTIONAL_REGION_MAX_TEMPS];

	memset(&region, 0, sizeof(region));
	memset(temp_nets, 0, sizeof(temp_nets));

	if (input_count == 0U || input_count > LXS_FUNCTIONAL_REGION_MAX_INPUTS ||
		output_count == 0U || output_count > LXS_FUNCTIONAL_REGION_MAX_OUTPUTS ||
		op_count == 0U || op_count > LXS_FUNCTIONAL_REGION_MAX_OPS ||
		temp_count > LXS_FUNCTIONAL_REGION_MAX_TEMPS)
		{
		return 0;
		}

	region.exec_kind = exec_kind;
	region.input_count = input_count;
	region.output_count = output_count;
	region.temp_count = (uint8_t)temp_count;
	region.op_count = (uint8_t)op_count;
	region.gate_count = 0U;
	region.node_budget = 8U;
	region.max_depth = 3U;
	memcpy(region.inputs, inputs, (size_t)input_count * sizeof(uint32_t));
	memcpy(region.outputs, outputs, (size_t)output_count * sizeof(uint32_t));

	for (uint32_t i = 0; i < temp_count; ++i)
		{
		char suffix[16];

		snprintf(suffix, sizeof(suffix), "tmp%u", i);
		temp_nets[i] = lxs_intern_temp_net(nl, "fr", serial, suffix);
		if (temp_nets[i] == UINT32_MAX)
			{
			return 0;
			}
		}

	for (uint32_t i = 0; i < op_count; ++i)
		{
		uint32_t gate_inputs[4];
		uint32_t dst_nets[2] = { 0U, 0U };
		uint32_t arity;
		uint32_t dst_count;

		arity = (uint32_t)op_arity[i];
		if (arity == 0U || arity > 4U)
			{
			return 0;
			}
		dst_count = (uint32_t)op_dst_count[i];
		if (dst_count == 0U || dst_count > 2U)
			{
			return 0;
			}

		for (uint32_t src_index = 0; src_index < arity; ++src_index)
			{
			uint32_t src_ref = op_src[i][src_index];
			if (src_ref < input_count)
				{
				gate_inputs[src_index] = inputs[src_ref];
				}
			else
				{
				uint32_t temp_index = src_ref - input_count;
				if (temp_index >= temp_count)
					{
					return 0;
					}
				gate_inputs[src_index] = temp_nets[temp_index];
				}
			}

		for (uint32_t dst_index = 0; dst_index < dst_count; ++dst_index)
			{
			if (op_dst_kind[i][dst_index] == LXS_FUNCTIONAL_REGION_DST_TEMP)
				{
				if (op_dst[i][dst_index] >= temp_count)
					{
					return 0;
					}
				dst_nets[dst_index] = temp_nets[op_dst[i][dst_index]];
				}
			else if (op_dst_kind[i][dst_index] == LXS_FUNCTIONAL_REGION_DST_OUTPUT)
				{
				if (op_dst[i][dst_index] >= output_count)
					{
					return 0;
					}
				dst_nets[dst_index] = outputs[op_dst[i][dst_index]];
				}
			else
				{
				return 0;
				}
			}

		if (op_type[i] == LXS_FUNCTIONAL_REGION_OP_MAJ3)
			{
			uint32_t maj_ab;
			uint32_t maj_ac;
			uint32_t maj_bc;
			uint32_t or_inputs[3];
			char suffix[16];

			if (arity != 3U)
				{
				return 0;
				}
			if (dst_count != 1U)
				{
				return 0;
				}

			snprintf(suffix, sizeof(suffix), "m%uab", i);
			maj_ab = lxs_intern_temp_net(nl, "fr", serial, suffix);
			snprintf(suffix, sizeof(suffix), "m%uac", i);
			maj_ac = lxs_intern_temp_net(nl, "fr", serial, suffix);
			snprintf(suffix, sizeof(suffix), "m%ubc", i);
			maj_bc = lxs_intern_temp_net(nl, "fr", serial, suffix);
			if (maj_ab == UINT32_MAX || maj_ac == UINT32_MAX || maj_bc == UINT32_MAX)
				{
				return 0;
				}

			if (!lxs_emit_gate_record(nl, LXS_GATE_AND, maj_ab, gate_inputs, 2U, &region.gate_indices[region.gate_count++]))
				{
				return 0;
				}
			or_inputs[0] = gate_inputs[0];
			or_inputs[1] = gate_inputs[2];
			if (!lxs_emit_gate_record(nl, LXS_GATE_AND, maj_ac, or_inputs, 2U, &region.gate_indices[region.gate_count++]))
				{
				return 0;
				}
			or_inputs[0] = gate_inputs[1];
			or_inputs[1] = gate_inputs[2];
			if (!lxs_emit_gate_record(nl, LXS_GATE_AND, maj_bc, or_inputs, 2U, &region.gate_indices[region.gate_count++]))
				{
				return 0;
				}
			or_inputs[0] = maj_ab;
			or_inputs[1] = maj_ac;
			or_inputs[2] = maj_bc;
			if (!lxs_emit_gate_record(nl, LXS_GATE_OR, dst_nets[0], or_inputs, 3U, &region.gate_indices[region.gate_count++]))
				{
				return 0;
				}
			}
		else if (op_type[i] == LXS_FUNCTIONAL_REGION_OP_HA2)
			{
			if (arity != 2U || dst_count != 2U)
				{
				return 0;
				}
			if (!lxs_emit_gate_record(nl, LXS_GATE_XOR, dst_nets[0], gate_inputs, 2U, &region.gate_indices[region.gate_count++]) ||
				!lxs_emit_gate_record(nl, LXS_GATE_AND, dst_nets[1], gate_inputs, 2U, &region.gate_indices[region.gate_count++]))
				{
				return 0;
				}
			}
		else if (op_type[i] == LXS_FUNCTIONAL_REGION_OP_FA3)
			{
			uint32_t xor_inputs[3];
			uint32_t maj_ab;
			uint32_t maj_ac;
			uint32_t maj_bc;
			uint32_t or_inputs[3];
			char suffix[16];

			if (arity != 3U || dst_count != 2U)
				{
				return 0;
				}

			xor_inputs[0] = gate_inputs[0];
			xor_inputs[1] = gate_inputs[1];
			xor_inputs[2] = gate_inputs[2];
			if (!lxs_emit_gate_record(nl, LXS_GATE_XOR, dst_nets[0], xor_inputs, 3U, &region.gate_indices[region.gate_count++]))
				{
				return 0;
				}

			snprintf(suffix, sizeof(suffix), "f%uab", i);
			maj_ab = lxs_intern_temp_net(nl, "fr", serial, suffix);
			snprintf(suffix, sizeof(suffix), "f%uac", i);
			maj_ac = lxs_intern_temp_net(nl, "fr", serial, suffix);
			snprintf(suffix, sizeof(suffix), "f%ubc", i);
			maj_bc = lxs_intern_temp_net(nl, "fr", serial, suffix);
			if (maj_ab == UINT32_MAX || maj_ac == UINT32_MAX || maj_bc == UINT32_MAX)
				{
				return 0;
				}
			if (!lxs_emit_gate_record(nl, LXS_GATE_AND, maj_ab, gate_inputs, 2U, &region.gate_indices[region.gate_count++]))
				{
				return 0;
				}
			or_inputs[0] = gate_inputs[0];
			or_inputs[1] = gate_inputs[2];
			if (!lxs_emit_gate_record(nl, LXS_GATE_AND, maj_ac, or_inputs, 2U, &region.gate_indices[region.gate_count++]))
				{
				return 0;
				}
			or_inputs[0] = gate_inputs[1];
			or_inputs[1] = gate_inputs[2];
			if (!lxs_emit_gate_record(nl, LXS_GATE_AND, maj_bc, or_inputs, 2U, &region.gate_indices[region.gate_count++]))
				{
				return 0;
				}
			or_inputs[0] = maj_ab;
			or_inputs[1] = maj_ac;
			or_inputs[2] = maj_bc;
			if (!lxs_emit_gate_record(nl, LXS_GATE_OR, dst_nets[1], or_inputs, 3U, &region.gate_indices[region.gate_count++]))
				{
				return 0;
				}
			}
		else
			{
			uint32_t gate_type;
			if (dst_count != 1U)
				{
				return 0;
				}
			if (!lxs_functional_op_gate_type(op_type[i], &gate_type))
				{
				return 0;
				}
			if (!lxs_emit_gate_record(
					nl,
					gate_type,
					dst_nets[0],
					gate_inputs,
					arity,
					&region.gate_indices[region.gate_count++]))
				{
				return 0;
				}
			}

		region.op_type[i] = op_type[i];
		region.op_dst_count[i] = op_dst_count[i];
		for (uint32_t dst_index = 0; dst_index < dst_count; ++dst_index)
			{
			region.op_dst_kind[i][dst_index] = op_dst_kind[i][dst_index];
			region.op_dst[i][dst_index] = op_dst[i][dst_index];
			}
		region.op_arity[i] = op_arity[i];
		for (uint32_t src_index = 0; src_index < 4U; ++src_index)
			{
			region.op_src[i][src_index] = op_src[i][src_index];
			}
		}

	return lxs_push_source_functional_region(nl, &region);
	}

static int lxs_emit_full_adder_macro(
	lxs_netlist *nl,
	const uint32_t *outputs,
	const uint32_t *inputs,
	uint32_t serial)
	{
	lxs_source_multi_macro macro;
	uint32_t xor_ab;
	uint32_t and_ab;
	uint32_t and_cin;
	uint32_t gate_inputs[2];

	memset(&macro, 0, sizeof(macro));
	macro.type = LXS_SOURCE_MULTI_MACRO_FULL_ADDER;
	macro.input_count = 3U;
	macro.output_count = 2U;
	macro.gate_count = 5U;
	macro.inputs[0] = inputs[0];
	macro.inputs[1] = inputs[1];
	macro.inputs[2] = inputs[2];
	macro.outputs[0] = outputs[0];
	macro.outputs[1] = outputs[1];

	xor_ab = lxs_intern_temp_net(nl, "fa", serial, "xor_ab");
	and_ab = lxs_intern_temp_net(nl, "fa", serial, "and_ab");
	and_cin = lxs_intern_temp_net(nl, "fa", serial, "and_cin");
	if (xor_ab == UINT32_MAX || and_ab == UINT32_MAX || and_cin == UINT32_MAX)
		{
		return 0;
		}

	gate_inputs[0] = inputs[0];
	gate_inputs[1] = inputs[1];
	if (!lxs_emit_gate_record(nl, LXS_GATE_XOR, xor_ab, gate_inputs, 2U, &macro.gate_indices[0]) ||
		!lxs_emit_gate_record(nl, LXS_GATE_AND, and_ab, gate_inputs, 2U, &macro.gate_indices[2]))
		{
		return 0;
		}

	gate_inputs[0] = xor_ab;
	gate_inputs[1] = inputs[2];
	if (!lxs_emit_gate_record(nl, LXS_GATE_XOR, outputs[0], gate_inputs, 2U, &macro.gate_indices[1]) ||
		!lxs_emit_gate_record(nl, LXS_GATE_AND, and_cin, gate_inputs, 2U, &macro.gate_indices[3]))
		{
		return 0;
		}

	gate_inputs[0] = and_ab;
	gate_inputs[1] = and_cin;
	if (!lxs_emit_gate_record(nl, LXS_GATE_OR, outputs[1], gate_inputs, 2U, &macro.gate_indices[4]) ||
		!lxs_push_source_multi_macro(nl, &macro))
		{
		return 0;
		}

	return 1;
	}

static int lxs_emit_ripple_slice2_macro(
	lxs_netlist *nl,
	const uint32_t *outputs,
	const uint32_t *inputs,
	uint32_t serial)
	{
	lxs_source_multi_macro macro;
	uint32_t gate_inputs[2];
	uint32_t xor0;
	uint32_t and0a;
	uint32_t and0b;
	uint32_t carry0;
	uint32_t xor1;
	uint32_t and1a;
	uint32_t and1b;

	memset(&macro, 0, sizeof(macro));
	macro.type = LXS_SOURCE_MULTI_MACRO_RIPPLE_SLICE2;
	macro.input_count = 5U;
	macro.output_count = 3U;
	macro.gate_count = 10U;
	for (uint32_t i = 0; i < 5U; ++i)
		{
		macro.inputs[i] = inputs[i];
		}
	for (uint32_t i = 0; i < 3U; ++i)
		{
		macro.outputs[i] = outputs[i];
		}

	xor0 = lxs_intern_temp_net(nl, "rs2", serial, "xor0");
	and0a = lxs_intern_temp_net(nl, "rs2", serial, "and0a");
	and0b = lxs_intern_temp_net(nl, "rs2", serial, "and0b");
	carry0 = lxs_intern_temp_net(nl, "rs2", serial, "carry0");
	xor1 = lxs_intern_temp_net(nl, "rs2", serial, "xor1");
	and1a = lxs_intern_temp_net(nl, "rs2", serial, "and1a");
	and1b = lxs_intern_temp_net(nl, "rs2", serial, "and1b");
	if (xor0 == UINT32_MAX || and0a == UINT32_MAX || and0b == UINT32_MAX || carry0 == UINT32_MAX ||
		xor1 == UINT32_MAX || and1a == UINT32_MAX || and1b == UINT32_MAX)
		{
		return 0;
		}

	gate_inputs[0] = inputs[0];
	gate_inputs[1] = inputs[1];
	if (!lxs_emit_gate_record(nl, LXS_GATE_XOR, xor0, gate_inputs, 2U, &macro.gate_indices[0]) ||
		!lxs_emit_gate_record(nl, LXS_GATE_AND, and0a, gate_inputs, 2U, &macro.gate_indices[2]))
		{
		return 0;
		}

	gate_inputs[0] = xor0;
	gate_inputs[1] = inputs[4];
	if (!lxs_emit_gate_record(nl, LXS_GATE_XOR, outputs[0], gate_inputs, 2U, &macro.gate_indices[1]) ||
		!lxs_emit_gate_record(nl, LXS_GATE_AND, and0b, gate_inputs, 2U, &macro.gate_indices[3]))
		{
		return 0;
		}

	gate_inputs[0] = and0a;
	gate_inputs[1] = and0b;
	if (!lxs_emit_gate_record(nl, LXS_GATE_OR, carry0, gate_inputs, 2U, &macro.gate_indices[4]))
		{
		return 0;
		}

	gate_inputs[0] = inputs[2];
	gate_inputs[1] = inputs[3];
	if (!lxs_emit_gate_record(nl, LXS_GATE_XOR, xor1, gate_inputs, 2U, &macro.gate_indices[5]) ||
		!lxs_emit_gate_record(nl, LXS_GATE_AND, and1a, gate_inputs, 2U, &macro.gate_indices[7]))
		{
		return 0;
		}

	gate_inputs[0] = xor1;
	gate_inputs[1] = carry0;
	if (!lxs_emit_gate_record(nl, LXS_GATE_XOR, outputs[1], gate_inputs, 2U, &macro.gate_indices[6]) ||
		!lxs_emit_gate_record(nl, LXS_GATE_AND, and1b, gate_inputs, 2U, &macro.gate_indices[8]))
		{
		return 0;
		}

	gate_inputs[0] = and1a;
	gate_inputs[1] = and1b;
	if (!lxs_emit_gate_record(nl, LXS_GATE_OR, outputs[2], gate_inputs, 2U, &macro.gate_indices[9]) ||
		!lxs_push_source_multi_macro(nl, &macro))
		{
		return 0;
		}

	return 1;
	}

static int lxs_emit_ripple_add4_macro(
	lxs_netlist *nl,
	const uint32_t *outputs,
	const uint32_t *inputs,
	uint32_t serial)
	{
	lxs_source_multi_macro macro;
	uint32_t gate_inputs[2];
	uint32_t xor0;
	uint32_t and0a;
	uint32_t and0b;
	uint32_t carry0;
	uint32_t xor1;
	uint32_t and1a;
	uint32_t and1b;
	uint32_t carry1;
	uint32_t xor2;
	uint32_t and2a;
	uint32_t and2b;
	uint32_t carry2;
	uint32_t xor3;
	uint32_t and3a;
	uint32_t and3b;

	memset(&macro, 0, sizeof(macro));
	macro.type = LXS_SOURCE_MULTI_MACRO_RIPPLE_ADD4;
	macro.input_count = 9U;
	macro.output_count = 5U;
	macro.gate_count = 20U;
	for (uint32_t i = 0; i < 9U; ++i)
		{
		macro.inputs[i] = inputs[i];
		}
	for (uint32_t i = 0; i < 5U; ++i)
		{
		macro.outputs[i] = outputs[i];
		}

	xor0 = lxs_intern_temp_net(nl, "ra4", serial, "xor0");
	and0a = lxs_intern_temp_net(nl, "ra4", serial, "and0a");
	and0b = lxs_intern_temp_net(nl, "ra4", serial, "and0b");
	carry0 = lxs_intern_temp_net(nl, "ra4", serial, "carry0");
	xor1 = lxs_intern_temp_net(nl, "ra4", serial, "xor1");
	and1a = lxs_intern_temp_net(nl, "ra4", serial, "and1a");
	and1b = lxs_intern_temp_net(nl, "ra4", serial, "and1b");
	carry1 = lxs_intern_temp_net(nl, "ra4", serial, "carry1");
	xor2 = lxs_intern_temp_net(nl, "ra4", serial, "xor2");
	and2a = lxs_intern_temp_net(nl, "ra4", serial, "and2a");
	and2b = lxs_intern_temp_net(nl, "ra4", serial, "and2b");
	carry2 = lxs_intern_temp_net(nl, "ra4", serial, "carry2");
	xor3 = lxs_intern_temp_net(nl, "ra4", serial, "xor3");
	and3a = lxs_intern_temp_net(nl, "ra4", serial, "and3a");
	and3b = lxs_intern_temp_net(nl, "ra4", serial, "and3b");
	if (xor0 == UINT32_MAX || and0a == UINT32_MAX || and0b == UINT32_MAX || carry0 == UINT32_MAX ||
		xor1 == UINT32_MAX || and1a == UINT32_MAX || and1b == UINT32_MAX || carry1 == UINT32_MAX ||
		xor2 == UINT32_MAX || and2a == UINT32_MAX || and2b == UINT32_MAX || carry2 == UINT32_MAX ||
		xor3 == UINT32_MAX || and3a == UINT32_MAX || and3b == UINT32_MAX)
		{
		return 0;
		}

	gate_inputs[0] = inputs[0];
	gate_inputs[1] = inputs[1];
	if (!lxs_emit_gate_record(nl, LXS_GATE_XOR, xor0, gate_inputs, 2U, &macro.gate_indices[0]) ||
		!lxs_emit_gate_record(nl, LXS_GATE_AND, and0a, gate_inputs, 2U, &macro.gate_indices[2]))
		{
		return 0;
		}

	gate_inputs[0] = xor0;
	gate_inputs[1] = inputs[8];
	if (!lxs_emit_gate_record(nl, LXS_GATE_XOR, outputs[0], gate_inputs, 2U, &macro.gate_indices[1]) ||
		!lxs_emit_gate_record(nl, LXS_GATE_AND, and0b, gate_inputs, 2U, &macro.gate_indices[3]))
		{
		return 0;
		}

	gate_inputs[0] = and0a;
	gate_inputs[1] = and0b;
	if (!lxs_emit_gate_record(nl, LXS_GATE_OR, carry0, gate_inputs, 2U, &macro.gate_indices[4]))
		{
		return 0;
		}

	gate_inputs[0] = inputs[2];
	gate_inputs[1] = inputs[3];
	if (!lxs_emit_gate_record(nl, LXS_GATE_XOR, xor1, gate_inputs, 2U, &macro.gate_indices[5]) ||
		!lxs_emit_gate_record(nl, LXS_GATE_AND, and1a, gate_inputs, 2U, &macro.gate_indices[7]))
		{
		return 0;
		}

	gate_inputs[0] = xor1;
	gate_inputs[1] = carry0;
	if (!lxs_emit_gate_record(nl, LXS_GATE_XOR, outputs[1], gate_inputs, 2U, &macro.gate_indices[6]) ||
		!lxs_emit_gate_record(nl, LXS_GATE_AND, and1b, gate_inputs, 2U, &macro.gate_indices[8]))
		{
		return 0;
		}

	gate_inputs[0] = and1a;
	gate_inputs[1] = and1b;
	if (!lxs_emit_gate_record(nl, LXS_GATE_OR, carry1, gate_inputs, 2U, &macro.gate_indices[9]))
		{
		return 0;
		}

	gate_inputs[0] = inputs[4];
	gate_inputs[1] = inputs[5];
	if (!lxs_emit_gate_record(nl, LXS_GATE_XOR, xor2, gate_inputs, 2U, &macro.gate_indices[10]) ||
		!lxs_emit_gate_record(nl, LXS_GATE_AND, and2a, gate_inputs, 2U, &macro.gate_indices[12]))
		{
		return 0;
		}

	gate_inputs[0] = xor2;
	gate_inputs[1] = carry1;
	if (!lxs_emit_gate_record(nl, LXS_GATE_XOR, outputs[2], gate_inputs, 2U, &macro.gate_indices[11]) ||
		!lxs_emit_gate_record(nl, LXS_GATE_AND, and2b, gate_inputs, 2U, &macro.gate_indices[13]))
		{
		return 0;
		}

	gate_inputs[0] = and2a;
	gate_inputs[1] = and2b;
	if (!lxs_emit_gate_record(nl, LXS_GATE_OR, carry2, gate_inputs, 2U, &macro.gate_indices[14]))
		{
		return 0;
		}

	gate_inputs[0] = inputs[6];
	gate_inputs[1] = inputs[7];
	if (!lxs_emit_gate_record(nl, LXS_GATE_XOR, xor3, gate_inputs, 2U, &macro.gate_indices[15]) ||
		!lxs_emit_gate_record(nl, LXS_GATE_AND, and3a, gate_inputs, 2U, &macro.gate_indices[17]))
		{
		return 0;
		}

	gate_inputs[0] = xor3;
	gate_inputs[1] = carry2;
	if (!lxs_emit_gate_record(nl, LXS_GATE_XOR, outputs[3], gate_inputs, 2U, &macro.gate_indices[16]) ||
		!lxs_emit_gate_record(nl, LXS_GATE_AND, and3b, gate_inputs, 2U, &macro.gate_indices[18]))
		{
		return 0;
		}

	gate_inputs[0] = and3a;
	gate_inputs[1] = and3b;
	if (!lxs_emit_gate_record(nl, LXS_GATE_OR, outputs[4], gate_inputs, 2U, &macro.gate_indices[19]) ||
		!lxs_push_source_multi_macro(nl, &macro))
		{
		return 0;
		}

	return 1;
	}

static int lxs_emit_carry_save_row4_macro(
	lxs_netlist *nl,
	const uint32_t *outputs,
	const uint32_t *inputs,
	uint32_t serial)
	{
	lxs_source_multi_macro macro;
	uint32_t gate_inputs[2];
	uint32_t xor0;
	uint32_t xor1;
	uint32_t xor2;
	uint32_t xor3;
	uint32_t and0a;
	uint32_t and0b;
	uint32_t and1a;
	uint32_t and1b;
	uint32_t and2a;
	uint32_t and2b;
	uint32_t and3a;
	uint32_t and3b;

	memset(&macro, 0, sizeof(macro));
	macro.type = LXS_SOURCE_MULTI_MACRO_CARRY_SAVE_ROW4;
	macro.input_count = 12U;
	macro.output_count = 8U;
	macro.gate_count = 20U;
	for (uint32_t i = 0; i < macro.input_count; ++i)
		{
		macro.inputs[i] = inputs[i];
		}
	for (uint32_t i = 0; i < macro.output_count; ++i)
		{
		macro.outputs[i] = outputs[i];
		}

	xor0 = lxs_intern_temp_net(nl, "csr4", serial, "xor0");
	xor1 = lxs_intern_temp_net(nl, "csr4", serial, "xor1");
	xor2 = lxs_intern_temp_net(nl, "csr4", serial, "xor2");
	xor3 = lxs_intern_temp_net(nl, "csr4", serial, "xor3");
	and0a = lxs_intern_temp_net(nl, "csr4", serial, "and0a");
	and0b = lxs_intern_temp_net(nl, "csr4", serial, "and0b");
	and1a = lxs_intern_temp_net(nl, "csr4", serial, "and1a");
	and1b = lxs_intern_temp_net(nl, "csr4", serial, "and1b");
	and2a = lxs_intern_temp_net(nl, "csr4", serial, "and2a");
	and2b = lxs_intern_temp_net(nl, "csr4", serial, "and2b");
	and3a = lxs_intern_temp_net(nl, "csr4", serial, "and3a");
	and3b = lxs_intern_temp_net(nl, "csr4", serial, "and3b");
	if (xor0 == UINT32_MAX || xor1 == UINT32_MAX || xor2 == UINT32_MAX || xor3 == UINT32_MAX ||
		and0a == UINT32_MAX || and0b == UINT32_MAX || and1a == UINT32_MAX || and1b == UINT32_MAX ||
		and2a == UINT32_MAX || and2b == UINT32_MAX || and3a == UINT32_MAX || and3b == UINT32_MAX)
		{
		return 0;
		}

	for (uint32_t column = 0; column < 4U; ++column)
		{
		uint32_t input_base = column * 3U;
		uint32_t output_base = column;
		uint32_t gate_base = column * 5U;
		uint32_t xor_net = (column == 0U) ? xor0 : (column == 1U) ? xor1 : (column == 2U) ? xor2 : xor3;
		uint32_t and_a = (column == 0U) ? and0a : (column == 1U) ? and1a : (column == 2U) ? and2a : and3a;
		uint32_t and_b = (column == 0U) ? and0b : (column == 1U) ? and1b : (column == 2U) ? and2b : and3b;

		gate_inputs[0] = inputs[input_base + 0U];
		gate_inputs[1] = inputs[input_base + 1U];
		if (!lxs_emit_gate_record(nl, LXS_GATE_XOR, xor_net, gate_inputs, 2U, &macro.gate_indices[gate_base + 0U]) ||
			!lxs_emit_gate_record(nl, LXS_GATE_AND, and_a, gate_inputs, 2U, &macro.gate_indices[gate_base + 2U]))
			{
			return 0;
			}

		gate_inputs[0] = xor_net;
		gate_inputs[1] = inputs[input_base + 2U];
		if (!lxs_emit_gate_record(nl, LXS_GATE_XOR, outputs[output_base], gate_inputs, 2U, &macro.gate_indices[gate_base + 1U]) ||
			!lxs_emit_gate_record(nl, LXS_GATE_AND, and_b, gate_inputs, 2U, &macro.gate_indices[gate_base + 3U]))
			{
			return 0;
			}

		gate_inputs[0] = and_a;
		gate_inputs[1] = and_b;
		if (!lxs_emit_gate_record(nl, LXS_GATE_OR, outputs[4U + column], gate_inputs, 2U, &macro.gate_indices[gate_base + 4U]))
			{
			return 0;
			}
		}

	if (!lxs_push_source_multi_macro(nl, &macro))
		{
		return 0;
		}

	return 1;
	}

static int lxs_emit_reduce_propagate4_macro(
	lxs_netlist *nl,
	const uint32_t *outputs,
	const uint32_t *inputs,
	uint32_t serial)
	{
	lxs_source_multi_macro macro;
	uint32_t gate_inputs[2];
	uint32_t sum0;
	uint32_t sum1;
	uint32_t sum2;
	uint32_t sum3;
	uint32_t carry1;
	uint32_t carry2;
	uint32_t carry3;
	uint32_t carry4;
	uint32_t carry01;
	uint32_t bit2;
	uint32_t carry12a;
	uint32_t carry12b;
	uint32_t carry12;
	uint32_t bit3;
	uint32_t carry23a;
	uint32_t carry23b;
	uint32_t spill0a;
	uint32_t xor0;
	uint32_t xor1;
	uint32_t xor2;
	uint32_t xor3;
	uint32_t and0a;
	uint32_t and0b;
	uint32_t and1a;
	uint32_t and1b;
	uint32_t and2a;
	uint32_t and2b;
	uint32_t and3a;
	uint32_t and3b;

	memset(&macro, 0, sizeof(macro));
	macro.type = LXS_SOURCE_MULTI_MACRO_REDUCE_PROPAGATE4;
	macro.input_count = 12U;
	macro.output_count = 6U;
	macro.gate_count = 35U;
	for (uint32_t i = 0; i < macro.input_count; ++i)
		{
		macro.inputs[i] = inputs[i];
		}
	for (uint32_t i = 0; i < macro.output_count; ++i)
		{
		macro.outputs[i] = outputs[i];
		}

	sum0 = lxs_intern_temp_net(nl, "rp4", serial, "sum0");
	sum1 = lxs_intern_temp_net(nl, "rp4", serial, "sum1");
	sum2 = lxs_intern_temp_net(nl, "rp4", serial, "sum2");
	sum3 = lxs_intern_temp_net(nl, "rp4", serial, "sum3");
	carry1 = lxs_intern_temp_net(nl, "rp4", serial, "carry1");
	carry2 = lxs_intern_temp_net(nl, "rp4", serial, "carry2");
	carry3 = lxs_intern_temp_net(nl, "rp4", serial, "carry3");
	carry4 = lxs_intern_temp_net(nl, "rp4", serial, "carry4");
	carry01 = lxs_intern_temp_net(nl, "rp4", serial, "carry01");
	bit2 = lxs_intern_temp_net(nl, "rp4", serial, "bit2");
	carry12a = lxs_intern_temp_net(nl, "rp4", serial, "carry12a");
	carry12b = lxs_intern_temp_net(nl, "rp4", serial, "carry12b");
	carry12 = lxs_intern_temp_net(nl, "rp4", serial, "carry12");
	bit3 = lxs_intern_temp_net(nl, "rp4", serial, "bit3");
	carry23a = lxs_intern_temp_net(nl, "rp4", serial, "carry23a");
	carry23b = lxs_intern_temp_net(nl, "rp4", serial, "carry23b");
	spill0a = lxs_intern_temp_net(nl, "rp4", serial, "spill0a");
	xor0 = lxs_intern_temp_net(nl, "rp4", serial, "xor0");
	xor1 = lxs_intern_temp_net(nl, "rp4", serial, "xor1");
	xor2 = lxs_intern_temp_net(nl, "rp4", serial, "xor2");
	xor3 = lxs_intern_temp_net(nl, "rp4", serial, "xor3");
	and0a = lxs_intern_temp_net(nl, "rp4", serial, "and0a");
	and0b = lxs_intern_temp_net(nl, "rp4", serial, "and0b");
	and1a = lxs_intern_temp_net(nl, "rp4", serial, "and1a");
	and1b = lxs_intern_temp_net(nl, "rp4", serial, "and1b");
	and2a = lxs_intern_temp_net(nl, "rp4", serial, "and2a");
	and2b = lxs_intern_temp_net(nl, "rp4", serial, "and2b");
	and3a = lxs_intern_temp_net(nl, "rp4", serial, "and3a");
	and3b = lxs_intern_temp_net(nl, "rp4", serial, "and3b");
	if (sum0 == UINT32_MAX || sum1 == UINT32_MAX || sum2 == UINT32_MAX || sum3 == UINT32_MAX ||
		carry1 == UINT32_MAX || carry2 == UINT32_MAX || carry3 == UINT32_MAX || carry4 == UINT32_MAX ||
		carry01 == UINT32_MAX || bit2 == UINT32_MAX || carry12a == UINT32_MAX || carry12b == UINT32_MAX ||
		carry12 == UINT32_MAX || bit3 == UINT32_MAX || carry23a == UINT32_MAX || carry23b == UINT32_MAX ||
		spill0a == UINT32_MAX || xor0 == UINT32_MAX || xor1 == UINT32_MAX || xor2 == UINT32_MAX ||
		xor3 == UINT32_MAX || and0a == UINT32_MAX || and0b == UINT32_MAX || and1a == UINT32_MAX ||
		and1b == UINT32_MAX || and2a == UINT32_MAX || and2b == UINT32_MAX || and3a == UINT32_MAX ||
		and3b == UINT32_MAX)
		{
		return 0;
		}

	for (uint32_t column = 0; column < 4U; ++column)
		{
		uint32_t input_base = column * 3U;
		uint32_t gate_base = column * 5U;
		uint32_t xor_net = (column == 0U) ? xor0 : (column == 1U) ? xor1 : (column == 2U) ? xor2 : xor3;
		uint32_t and_a = (column == 0U) ? and0a : (column == 1U) ? and1a : (column == 2U) ? and2a : and3a;
		uint32_t and_b = (column == 0U) ? and0b : (column == 1U) ? and1b : (column == 2U) ? and2b : and3b;
		uint32_t sum_out = (column == 0U) ? sum0 : (column == 1U) ? sum1 : (column == 2U) ? sum2 : sum3;
		uint32_t carry_out = (column == 0U) ? carry1 : (column == 1U) ? carry2 : (column == 2U) ? carry3 : carry4;

		gate_inputs[0] = inputs[input_base + 0U];
		gate_inputs[1] = inputs[input_base + 1U];
		if (!lxs_emit_gate_record(nl, LXS_GATE_XOR, xor_net, gate_inputs, 2U, &macro.gate_indices[gate_base + 0U]) ||
			!lxs_emit_gate_record(nl, LXS_GATE_AND, and_a, gate_inputs, 2U, &macro.gate_indices[gate_base + 2U]))
			{
			return 0;
			}

		gate_inputs[0] = xor_net;
		gate_inputs[1] = inputs[input_base + 2U];
		if (!lxs_emit_gate_record(nl, LXS_GATE_XOR, sum_out, gate_inputs, 2U, &macro.gate_indices[gate_base + 1U]) ||
			!lxs_emit_gate_record(nl, LXS_GATE_AND, and_b, gate_inputs, 2U, &macro.gate_indices[gate_base + 3U]))
			{
			return 0;
			}

		gate_inputs[0] = and_a;
		gate_inputs[1] = and_b;
		if (!lxs_emit_gate_record(nl, LXS_GATE_OR, carry_out, gate_inputs, 2U, &macro.gate_indices[gate_base + 4U]))
			{
			return 0;
			}
		}

	gate_inputs[0] = sum0;
	if (!lxs_emit_gate_record(nl, LXS_GATE_BUF, outputs[0], gate_inputs, 1U, &macro.gate_indices[20]))
		{
		return 0;
		}

	gate_inputs[0] = sum1;
	gate_inputs[1] = carry1;
	if (!lxs_emit_gate_record(nl, LXS_GATE_XOR, outputs[1], gate_inputs, 2U, &macro.gate_indices[21]))
		{
		return 0;
		}
	if (!lxs_emit_gate_record(nl, LXS_GATE_AND, carry01, gate_inputs, 2U, &macro.gate_indices[22]))
		{
		return 0;
		}

	gate_inputs[0] = sum2;
	gate_inputs[1] = carry2;
	if (!lxs_emit_gate_record(nl, LXS_GATE_XOR, bit2, gate_inputs, 2U, &macro.gate_indices[23]) ||
		!lxs_emit_gate_record(nl, LXS_GATE_AND, carry12a, gate_inputs, 2U, &macro.gate_indices[24]))
		{
		return 0;
		}
	gate_inputs[0] = bit2;
	gate_inputs[1] = carry01;
	if (!lxs_emit_gate_record(nl, LXS_GATE_XOR, outputs[2], gate_inputs, 2U, &macro.gate_indices[25]) ||
		!lxs_emit_gate_record(nl, LXS_GATE_AND, carry12b, gate_inputs, 2U, &macro.gate_indices[26]))
		{
		return 0;
		}
	gate_inputs[0] = carry12a;
	gate_inputs[1] = carry12b;
	if (!lxs_emit_gate_record(nl, LXS_GATE_OR, carry12, gate_inputs, 2U, &macro.gate_indices[27]))
		{
		return 0;
		}

	gate_inputs[0] = sum3;
	gate_inputs[1] = carry3;
	if (!lxs_emit_gate_record(nl, LXS_GATE_XOR, bit3, gate_inputs, 2U, &macro.gate_indices[28]) ||
		!lxs_emit_gate_record(nl, LXS_GATE_AND, carry23a, gate_inputs, 2U, &macro.gate_indices[29]))
		{
		return 0;
		}
	gate_inputs[0] = bit3;
	gate_inputs[1] = carry12;
	if (!lxs_emit_gate_record(nl, LXS_GATE_XOR, outputs[3], gate_inputs, 2U, &macro.gate_indices[30]) ||
		!lxs_emit_gate_record(nl, LXS_GATE_AND, carry23b, gate_inputs, 2U, &macro.gate_indices[31]))
		{
		return 0;
		}
	gate_inputs[0] = carry23a;
	gate_inputs[1] = carry23b;
	if (!lxs_emit_gate_record(nl, LXS_GATE_OR, spill0a, gate_inputs, 2U, &macro.gate_indices[32]))
		{
		return 0;
		}

	gate_inputs[0] = carry4;
	gate_inputs[1] = spill0a;
	if (!lxs_emit_gate_record(nl, LXS_GATE_XOR, outputs[4], gate_inputs, 2U, &macro.gate_indices[33]) ||
		!lxs_emit_gate_record(nl, LXS_GATE_AND, outputs[5], gate_inputs, 2U, &macro.gate_indices[34]))
		{
		return 0;
		}

	if (!lxs_push_source_multi_macro(nl, &macro))
		{
		return 0;
		}

	return 1;
	}

static int lxs_emit_xor_fan8_macro(
	lxs_netlist *nl,
	const uint32_t *outputs,
	const uint32_t *inputs)
	{
	lxs_source_multi_macro macro;
	uint32_t gate_inputs[2];

	memset(&macro, 0, sizeof(macro));
	macro.type = LXS_SOURCE_MULTI_MACRO_XOR_FAN8;
	macro.input_count = 9U;
	macro.output_count = 8U;
	macro.gate_count = 8U;
	for (uint32_t i = 0; i < macro.input_count; ++i)
		{
		macro.inputs[i] = inputs[i];
		}
	for (uint32_t i = 0; i < macro.output_count; ++i)
		{
		macro.outputs[i] = outputs[i];
		gate_inputs[0] = inputs[0];
		gate_inputs[1] = inputs[i + 1U];
		if (!lxs_emit_gate_record(nl, LXS_GATE_XOR, outputs[i], gate_inputs, 2U, &macro.gate_indices[i]))
			{
			return 0;
			}
		}

	if (!lxs_push_source_multi_macro(nl, &macro))
		{
		return 0;
		}

	return 1;
	}

static int lxs_emit_and_fan8_macro(
	lxs_netlist *nl,
	const uint32_t *outputs,
	const uint32_t *inputs)
	{
	lxs_source_multi_macro macro;
	uint32_t gate_inputs[2];

	memset(&macro, 0, sizeof(macro));
	macro.type = LXS_SOURCE_MULTI_MACRO_AND_FAN8;
	macro.input_count = 9U;
	macro.output_count = 8U;
	macro.gate_count = 8U;
	for (uint32_t i = 0; i < macro.input_count; ++i)
		{
		macro.inputs[i] = inputs[i];
		}
	for (uint32_t i = 0; i < macro.output_count; ++i)
		{
		macro.outputs[i] = outputs[i];
		gate_inputs[0] = inputs[0];
		gate_inputs[1] = inputs[i + 1U];
		if (!lxs_emit_gate_record(nl, LXS_GATE_AND, outputs[i], gate_inputs, 2U, &macro.gate_indices[i]))
			{
			return 0;
			}
		}

	if (!lxs_push_source_multi_macro(nl, &macro))
		{
		return 0;
		}

	return 1;
	}

static int lxs_emit_xnor_bank4_macro(
	lxs_netlist *nl,
	const uint32_t *outputs,
	const uint32_t *inputs)
	{
	lxs_source_multi_macro macro;
	uint32_t gate_inputs[2];
	uint32_t xor_out;

	memset(&macro, 0, sizeof(macro));
	macro.type = LXS_SOURCE_MULTI_MACRO_XNOR_BANK4;
	macro.input_count = 8U;
	macro.output_count = 4U;
	macro.gate_count = 8U;
	for (uint32_t i = 0; i < macro.input_count; ++i)
		{
		macro.inputs[i] = inputs[i];
		}
	for (uint32_t i = 0; i < macro.output_count; ++i)
		{
		macro.outputs[i] = outputs[i];
		xor_out = lxs_intern_temp_net(nl, "xnb4", outputs[i], "xor");
		if (xor_out == UINT32_MAX)
			{
			return 0;
			}

		gate_inputs[0] = inputs[i * 2U];
		gate_inputs[1] = inputs[i * 2U + 1U];
		if (!lxs_emit_gate_record(nl, LXS_GATE_XOR, xor_out, gate_inputs, 2U, &macro.gate_indices[i * 2U]))
			{
			return 0;
			}

		gate_inputs[0] = xor_out;
		if (!lxs_emit_gate_record(nl, LXS_GATE_NOT, outputs[i], gate_inputs, 1U, &macro.gate_indices[i * 2U + 1U]))
			{
			return 0;
			}
		}

	if (!lxs_push_source_multi_macro(nl, &macro))
		{
		return 0;
		}

	return 1;
	}

static int lxs_emit_guard_chain4_macro(
	lxs_netlist *nl,
	const uint32_t *outputs,
	const uint32_t *inputs)
	{
	lxs_source_multi_macro macro;
	uint32_t gate_inputs[2];
	uint32_t inv1;
	uint32_t inv2;
	uint32_t inv3;

	memset(&macro, 0, sizeof(macro));
	macro.type = LXS_SOURCE_MULTI_MACRO_GUARD_CHAIN4;
	macro.input_count = 5U;
	macro.output_count = 5U;
	macro.gate_count = 8U;
	for (uint32_t i = 0; i < macro.input_count; ++i)
		{
		macro.inputs[i] = inputs[i];
		}
	for (uint32_t i = 0; i < macro.output_count; ++i)
		{
		macro.outputs[i] = outputs[i];
		}

	inv1 = lxs_intern_temp_net(nl, "gc4", outputs[0], "inv1");
	inv2 = lxs_intern_temp_net(nl, "gc4", outputs[1], "inv2");
	inv3 = lxs_intern_temp_net(nl, "gc4", outputs[2], "inv3");
	if (inv1 == UINT32_MAX || inv2 == UINT32_MAX || inv3 == UINT32_MAX)
		{
		return 0;
		}

	gate_inputs[0] = inputs[1];
	gate_inputs[1] = inputs[0];
	if (!lxs_emit_gate_record(nl, LXS_GATE_AND, outputs[0], gate_inputs, 2U, &macro.gate_indices[0]))
		{
		return 0;
		}

	gate_inputs[0] = outputs[0];
	if (!lxs_emit_gate_record(nl, LXS_GATE_NOT, inv1, gate_inputs, 1U, &macro.gate_indices[1]))
		{
		return 0;
		}

	gate_inputs[0] = inputs[2];
	gate_inputs[1] = inv1;
	if (!lxs_emit_gate_record(nl, LXS_GATE_AND, outputs[1], gate_inputs, 2U, &macro.gate_indices[2]))
		{
		return 0;
		}

	gate_inputs[0] = outputs[1];
	if (!lxs_emit_gate_record(nl, LXS_GATE_NOT, inv2, gate_inputs, 1U, &macro.gate_indices[3]))
		{
		return 0;
		}

	gate_inputs[0] = inputs[3];
	gate_inputs[1] = inv2;
	if (!lxs_emit_gate_record(nl, LXS_GATE_AND, outputs[2], gate_inputs, 2U, &macro.gate_indices[4]))
		{
		return 0;
		}

	gate_inputs[0] = outputs[2];
	if (!lxs_emit_gate_record(nl, LXS_GATE_NOT, inv3, gate_inputs, 1U, &macro.gate_indices[5]))
		{
		return 0;
		}

	gate_inputs[0] = inputs[4];
	gate_inputs[1] = inv3;
	if (!lxs_emit_gate_record(nl, LXS_GATE_AND, outputs[3], gate_inputs, 2U, &macro.gate_indices[6]))
		{
		return 0;
		}

	gate_inputs[0] = outputs[3];
	if (!lxs_emit_gate_record(nl, LXS_GATE_NOT, outputs[4], gate_inputs, 1U, &macro.gate_indices[7]) ||
		!lxs_push_source_multi_macro(nl, &macro))
		{
		return 0;
		}

	return 1;
	}

static int lxs_parse_functional_op_type(const char *name, uint8_t *type_out)
	{
	if (strcmp(name, "BUF") == 0 || strcmp(name, "BUFF") == 0)
		{
		*type_out = LXS_FUNCTIONAL_REGION_OP_BUF;
		return 1;
		}
	if (strcmp(name, "NOT") == 0)
		{
		*type_out = LXS_FUNCTIONAL_REGION_OP_NOT;
		return 1;
		}
	if (strcmp(name, "AND") == 0)
		{
		*type_out = LXS_FUNCTIONAL_REGION_OP_AND;
		return 1;
		}
	if (strcmp(name, "OR") == 0)
		{
		*type_out = LXS_FUNCTIONAL_REGION_OP_OR;
		return 1;
		}
	if (strcmp(name, "XOR") == 0)
		{
		*type_out = LXS_FUNCTIONAL_REGION_OP_XOR;
		return 1;
		}
	if (strcmp(name, "NAND") == 0)
		{
		*type_out = LXS_FUNCTIONAL_REGION_OP_NAND;
		return 1;
		}
	if (strcmp(name, "NOR") == 0)
		{
		*type_out = LXS_FUNCTIONAL_REGION_OP_NOR;
		return 1;
		}
	if (strcmp(name, "XNOR") == 0)
		{
		*type_out = LXS_FUNCTIONAL_REGION_OP_XNOR;
		return 1;
		}
	if (strcmp(name, "MAJ3") == 0)
		{
		*type_out = LXS_FUNCTIONAL_REGION_OP_MAJ3;
		return 1;
		}
	if (strcmp(name, "HA2") == 0)
		{
		*type_out = LXS_FUNCTIONAL_REGION_OP_HA2;
		return 1;
		}
	if (strcmp(name, "FA3") == 0)
		{
		*type_out = LXS_FUNCTIONAL_REGION_OP_FA3;
		return 1;
		}
	return 0;
	}

static int lxs_parse_functional_ref(
	const char *text,
	uint32_t input_count,
	uint64_t defined_temp_mask,
	uint8_t *ref_out)
	{
	char *end = NULL;
	unsigned long index;

	if (!text || strlen(text) < 2U)
		{
		return 0;
		}

	if (text[0] != 'i' && text[0] != 't')
		{
		return 0;
		}

	index = strtoul(text + 1, &end, 10);
	if (!end || *end != '\0')
		{
		return 0;
		}

	if (text[0] == 'i')
		{
		if (index >= input_count)
			{
			return 0;
			}
		*ref_out = (uint8_t)index;
		return 1;
		}

	if (index >= LXS_FUNCTIONAL_REGION_MAX_TEMPS || (defined_temp_mask & (1ULL << index)) == 0ULL)
		{
		return 0;
		}

	*ref_out = (uint8_t)(input_count + index);
	return 1;
	}

static int lxs_parse_functional_dst(
	const char *text,
	uint32_t output_count,
	uint8_t *dst_kind_out,
	uint8_t *dst_out)
	{
	char *end = NULL;
	unsigned long index;

	if (!text || strlen(text) < 2U)
		{
		return 0;
		}

	if (text[0] != 't' && text[0] != 'o')
		{
		return 0;
		}

	index = strtoul(text + 1, &end, 10);
	if (!end || *end != '\0')
		{
		return 0;
		}

	if (text[0] == 't')
		{
		if (index >= LXS_FUNCTIONAL_REGION_MAX_TEMPS)
			{
			return 0;
			}
		*dst_kind_out = LXS_FUNCTIONAL_REGION_DST_TEMP;
		*dst_out = (uint8_t)index;
		return 1;
		}

	if (index >= output_count)
		{
		return 0;
		}

	*dst_kind_out = LXS_FUNCTIONAL_REGION_DST_OUTPUT;
	*dst_out = (uint8_t)index;
	return 1;
	}

static uint32_t lxs_gate_equiv_count(const lxs_gate_ir *gate)
	{
	switch (gate->type)
		{
		case LXS_GATE_AND:
		case LXS_GATE_OR:
		case LXS_GATE_XOR:
			return gate->input_count > 0U ? (gate->input_count - 1U) : 0U;
		case LXS_GATE_NAND:
		case LXS_GATE_NOR:
		case LXS_GATE_XNOR:
			return gate->input_count;
		case LXS_GATE_NOT:
		case LXS_GATE_BUF:
		case LXS_GATE_TRI:
			return 1U;
		default:
			return 1U;
		}
	}

static void lxs_record_recognition_candidate(
	uint64_t *candidate_roots,
	uint32_t family);

static void lxs_record_recognition_visit(
	uint64_t *nodes_visited,
	uint32_t *max_depth_reached,
	uint32_t family,
	uint32_t depth);

static void lxs_record_recognition_abort(
	uint64_t abort_count[LXS_RECOGNITION_FAMILY_COUNT][LXS_RECOGNITION_ABORT_REASON_COUNT],
	uint32_t family,
	uint32_t reason);

static int lxs_is_functional_gate_type(uint32_t type)
	{
	switch (type)
		{
		case LXS_GATE_AND:
		case LXS_GATE_OR:
		case LXS_GATE_XOR:
		case LXS_GATE_NAND:
		case LXS_GATE_NOR:
		case LXS_GATE_XNOR:
		case LXS_GATE_NOT:
		case LXS_GATE_BUF:
			return 1;
		default:
			return 0;
		}
	}

static int lxs_add_unique_boundary_input(
	uint32_t *boundary_inputs,
	uint32_t *boundary_count,
	uint32_t max_boundary_inputs,
	uint32_t net_id)
	{
	for (uint32_t i = 0; i < *boundary_count; ++i)
		{
		if (boundary_inputs[i] == net_id)
			{
			return 1;
			}
		}

	if (*boundary_count >= max_boundary_inputs)
		{
		return 0;
		}

	boundary_inputs[*boundary_count] = net_id;
	(*boundary_count)++;
	return 1;
	}

typedef struct lxs_functional_build_op lxs_functional_build_op;
struct lxs_functional_build_op
	{
	uint8_t type;
	uint8_t dst;
	uint8_t arity;
	uint8_t src_is_temp[4];
	uint8_t src_index[4];
	};

typedef struct lxs_functional_build_state lxs_functional_build_state;
struct lxs_functional_build_state
	{
	uint32_t gate_indices[8];
	uint32_t gate_count;
	uint32_t boundary_inputs[6];
	uint32_t boundary_count;
	uint8_t temp_count;
	uint8_t op_count;
	lxs_functional_build_op ops[8];
	};

static int lxs_find_or_add_boundary_input(
	lxs_functional_build_state *state,
	uint32_t net_id,
	uint8_t *index_out)
	{
	for (uint32_t i = 0; i < state->boundary_count; ++i)
		{
		if (state->boundary_inputs[i] == net_id)
			{
			*index_out = (uint8_t)i;
			return 1;
			}
		}

	if (state->boundary_count >= 6U)
		{
		return 0;
		}

	state->boundary_inputs[state->boundary_count] = net_id;
	*index_out = (uint8_t)state->boundary_count;
	state->boundary_count++;
	return 1;
	}

static uint8_t lxs_gate_type_to_functional_op(uint32_t type)
	{
	switch (type)
		{
		case LXS_GATE_BUF:
			return LXS_FUNCTIONAL_REGION_OP_BUF;
		case LXS_GATE_NOT:
			return LXS_FUNCTIONAL_REGION_OP_NOT;
		case LXS_GATE_AND:
			return LXS_FUNCTIONAL_REGION_OP_AND;
		case LXS_GATE_OR:
			return LXS_FUNCTIONAL_REGION_OP_OR;
		case LXS_GATE_XOR:
			return LXS_FUNCTIONAL_REGION_OP_XOR;
		case LXS_GATE_NAND:
			return LXS_FUNCTIONAL_REGION_OP_NAND;
		case LXS_GATE_NOR:
			return LXS_FUNCTIONAL_REGION_OP_NOR;
		case LXS_GATE_XNOR:
		default:
			return LXS_FUNCTIONAL_REGION_OP_XNOR;
		}
	}

static int lxs_build_functional_cone_rooted_visit(
	const lxs_netlist *nl,
	const int32_t *comb_driver,
	uint32_t gate_index,
	uint32_t gate_seen[8],
	uint32_t *gate_seen_count,
	lxs_functional_build_state *state,
	uint8_t is_root,
	uint8_t *is_temp_out,
	uint8_t *ref_out)
	{
	const lxs_gate_ir *gate = &nl->gates[gate_index];
	lxs_functional_build_op *op;
	uint8_t src_is_temp[4] = { 0 };
	uint8_t src_index[4] = { 0 };

	for (uint32_t i = 0; i < *gate_seen_count; ++i)
		{
		if (gate_seen[i] == gate_index)
			{
			return 0;
			}
		}

	if (*gate_seen_count >= 8U || state->gate_count >= 8U || state->op_count >= 8U)
		{
		return 0;
			}

	gate_seen[*gate_seen_count] = gate_index;
	(*gate_seen_count)++;

	if ((!is_root && gate->input_count > 2U) || gate->input_count == 0U || gate->input_count > 4U)
		{
		return 0;
		}

	for (uint32_t i = 0; i < gate->input_count; ++i)
		{
		uint32_t net_id = gate->inputs[i];
		int32_t driver = comb_driver[net_id];

		if (driver >= 0)
			{
			uint8_t child_is_temp = 0U;
			uint8_t child_ref = 0U;

			if (!lxs_build_functional_cone_rooted_visit(
					nl,
					comb_driver,
					(uint32_t)driver,
					gate_seen,
					gate_seen_count,
					state,
					0U,
					&child_is_temp,
					&child_ref))
				{
				return 0;
				}

			src_is_temp[i] = child_is_temp;
			src_index[i] = child_ref;
			}
		else
			{
			uint8_t boundary_index = 0U;

			if (!lxs_find_or_add_boundary_input(state, net_id, &boundary_index))
				{
				return 0;
				}

			src_is_temp[i] = 0U;
			src_index[i] = boundary_index;
			}
		}

	op = &state->ops[state->op_count];
	memset(op, 0, sizeof(*op));
	op->type = lxs_gate_type_to_functional_op(gate->type);
	op->arity = (uint8_t)gate->input_count;
	for (uint32_t i = 0; i < gate->input_count; ++i)
		{
		op->src_is_temp[i] = src_is_temp[i];
		op->src_index[i] = src_index[i];
		}
	if (is_root)
		{
		op->dst = 0xFFU;
		*is_temp_out = 0U;
		*ref_out = 0U;
		}
	else
		{
		if (state->temp_count >= 8U)
			{
			return 0;
			}
		op->dst = state->temp_count;
		*is_temp_out = 1U;
		*ref_out = state->temp_count;
		state->temp_count++;
		}

	state->gate_indices[state->gate_count++] = gate_index;
	state->op_count++;
	return 1;
	}

static int lxs_try_match_functional_cone_rooted_visit(
	const lxs_netlist *nl,
	const int32_t *comb_driver,
	const uint32_t *net_use_count,
	const uint8_t *matched_gates,
	uint32_t gate_index,
	uint32_t depth,
	uint32_t gate_seen[8],
	uint32_t *gate_seen_count,
	uint32_t *matched_gate_count,
	uint32_t *gate_equiv_count,
	uint32_t *boundary_inputs,
	uint32_t *boundary_count,
	uint64_t *family_nodes_visited,
	uint32_t *family_max_depth_reached,
	uint64_t family_abort_count[LXS_RECOGNITION_FAMILY_COUNT][LXS_RECOGNITION_ABORT_REASON_COUNT])
	{
	const lxs_gate_ir *gate;

	lxs_record_recognition_visit(
		family_nodes_visited,
		family_max_depth_reached,
		LXS_RECOGNITION_FAMILY_FUNCTIONAL,
		depth);

	if (depth > 3U)
		{
		lxs_record_recognition_abort(
			family_abort_count,
			LXS_RECOGNITION_FAMILY_FUNCTIONAL,
			LXS_RECOGNITION_ABORT_DEPTH);
		return 0;
		}

	for (uint32_t i = 0; i < *gate_seen_count; ++i)
		{
		if (gate_seen[i] == gate_index)
			{
			lxs_record_recognition_abort(
				family_abort_count,
				LXS_RECOGNITION_FAMILY_FUNCTIONAL,
				LXS_RECOGNITION_ABORT_OVERLAP);
			return 0;
			}
		}

	if (*gate_seen_count >= 8U)
		{
		lxs_record_recognition_abort(
			family_abort_count,
			LXS_RECOGNITION_FAMILY_FUNCTIONAL,
			LXS_RECOGNITION_ABORT_NODE_BUDGET);
		return 0;
		}

	gate = &nl->gates[gate_index];
	if (!lxs_is_functional_gate_type(gate->type) || gate->input_count == 0U ||
		(depth > 0U && gate->input_count > 2U) || gate->input_count > 4U)
		{
		lxs_record_recognition_abort(
			family_abort_count,
			LXS_RECOGNITION_FAMILY_FUNCTIONAL,
			LXS_RECOGNITION_ABORT_SHAPE);
		return 0;
		}

	if (matched_gates[gate_index])
		{
		lxs_record_recognition_abort(
			family_abort_count,
			LXS_RECOGNITION_FAMILY_FUNCTIONAL,
			LXS_RECOGNITION_ABORT_OVERLAP);
		return 0;
		}

	gate_seen[*gate_seen_count] = gate_index;
	(*gate_seen_count)++;
	(*matched_gate_count)++;
	(*gate_equiv_count) += lxs_gate_equiv_count(gate);

	for (uint32_t i = 0; i < gate->input_count; ++i)
		{
		uint32_t net_id = gate->inputs[i];
		int32_t driver = comb_driver[net_id];

		if (driver < 0)
			{
			if (!lxs_add_unique_boundary_input(boundary_inputs, boundary_count, 6U, net_id))
				{
				lxs_record_recognition_abort(
					family_abort_count,
					LXS_RECOGNITION_FAMILY_FUNCTIONAL,
					LXS_RECOGNITION_ABORT_NODE_BUDGET);
				return 0;
				}
			continue;
			}

		if (depth >= 3U)
			{
			lxs_record_recognition_abort(
				family_abort_count,
				LXS_RECOGNITION_FAMILY_FUNCTIONAL,
				LXS_RECOGNITION_ABORT_DEPTH);
			return 0;
			}

		if (net_use_count[net_id] != 1U)
			{
			lxs_record_recognition_abort(
				family_abort_count,
				LXS_RECOGNITION_FAMILY_FUNCTIONAL,
				LXS_RECOGNITION_ABORT_FANOUT);
			return 0;
			}

		if (!lxs_try_match_functional_cone_rooted_visit(
				nl,
				comb_driver,
				net_use_count,
				matched_gates,
				(uint32_t)driver,
				depth + 1U,
				gate_seen,
				gate_seen_count,
				matched_gate_count,
				gate_equiv_count,
				boundary_inputs,
				boundary_count,
				family_nodes_visited,
				family_max_depth_reached,
				family_abort_count))
			{
			return 0;
			}
		}

	return 1;
	}

static int lxs_try_match_functional_cone_rooted(
	const lxs_netlist *nl,
	const int32_t *comb_driver,
	const uint32_t *net_use_count,
	const uint8_t *matched_gates,
	uint32_t gate_index,
	uint64_t *family_candidate_roots,
	uint64_t *family_nodes_visited,
	uint32_t *family_max_depth_reached,
	uint64_t family_abort_count[LXS_RECOGNITION_FAMILY_COUNT][LXS_RECOGNITION_ABORT_REASON_COUNT],
	uint32_t *matched_gate_count,
	uint32_t *gate_equiv_count)
	{
	uint32_t gate_seen[8];
	uint32_t gate_seen_count = 0U;
	uint32_t boundary_inputs[6];
	uint32_t boundary_count = 0U;
	const lxs_gate_ir *root = &nl->gates[gate_index];

	lxs_record_recognition_candidate(
		family_candidate_roots,
		LXS_RECOGNITION_FAMILY_FUNCTIONAL);

	if (net_use_count[root->output] != 1U)
		{
		lxs_record_recognition_abort(
			family_abort_count,
			LXS_RECOGNITION_FAMILY_FUNCTIONAL,
			LXS_RECOGNITION_ABORT_FANOUT);
		return 0;
		}

	if (root->input_count < 2U || root->input_count > 3U)
		{
		lxs_record_recognition_abort(
			family_abort_count,
			LXS_RECOGNITION_FAMILY_FUNCTIONAL,
			LXS_RECOGNITION_ABORT_SHAPE);
		return 0;
		}

	*matched_gate_count = 0U;
	*gate_equiv_count = 0U;
	if (!lxs_try_match_functional_cone_rooted_visit(
			nl,
			comb_driver,
			net_use_count,
			matched_gates,
			gate_index,
			0U,
			gate_seen,
			&gate_seen_count,
			matched_gate_count,
			gate_equiv_count,
			boundary_inputs,
			&boundary_count,
			family_nodes_visited,
			family_max_depth_reached,
			family_abort_count))
		{
		return 0;
		}

	if (*matched_gate_count < 3U || *matched_gate_count > 8U || boundary_count == 0U || boundary_count > 6U)
		{
		lxs_record_recognition_abort(
			family_abort_count,
			LXS_RECOGNITION_FAMILY_FUNCTIONAL,
			LXS_RECOGNITION_ABORT_SHAPE);
		return 0;
		}

	return 1;
	}

static int lxs_functional_region_is_cache_merge(
	const lxs_functional_build_state *state)
	{
	uint8_t temp_use_count[8] = { 0 };
	uint8_t temp_depth[8] = { 0 };
	uint8_t root_temp_depths[4] = { 0 };
	uint32_t root_index;
	uint32_t root_temp_sources = 0U;

	if (state->op_count != 5U)
		{
		return 0;
		}

	if (state->temp_count != 4U || state->boundary_count == 0U || state->boundary_count > 6U)
		{
		return 0;
		}

	root_index = (uint32_t)(state->op_count - 1U);
	if (state->ops[root_index].arity < 2U || state->ops[root_index].arity > 3U)
		{
		return 0;
		}

	for (uint32_t i = 0; i < root_index; ++i)
		{
		uint8_t depth = 1U;
		if (state->ops[i].arity == 0U || state->ops[i].arity > 2U)
			{
			return 0;
			}

		for (uint32_t src = 0; src < state->ops[i].arity; ++src)
			{
			if (state->ops[i].src_is_temp[src])
				{
				uint8_t temp_index = state->ops[i].src_index[src];
				if (temp_index >= state->temp_count)
					{
					return 0;
					}
				temp_use_count[temp_index]++;
				if ((uint8_t)(temp_depth[temp_index] + 1U) > depth)
					{
					depth = (uint8_t)(temp_depth[temp_index] + 1U);
					}
				}
			}
		temp_depth[i] = depth;
		}

	for (uint32_t src = 0; src < state->ops[root_index].arity; ++src)
		{
		if (state->ops[root_index].src_is_temp[src])
			{
			uint8_t temp_index = state->ops[root_index].src_index[src];
			if (temp_index >= state->temp_count)
				{
				return 0;
				}
			temp_use_count[temp_index]++;
			root_temp_depths[root_temp_sources++] = temp_depth[temp_index];
			}
		}

	if (root_temp_sources != 2U)
		{
		return 0;
		}

	for (uint32_t i = 0; i < state->temp_count; ++i)
		{
		if (temp_use_count[i] != 1U)
			{
			return 0;
			}
		}

	if (!((root_temp_depths[0] == 1U && root_temp_depths[1] == 3U) ||
	      (root_temp_depths[0] == 3U && root_temp_depths[1] == 1U)))
		{
		return 0;
		}

	return 1;
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

static uint32_t lxs_recompute_gate_level(
	uint32_t gate_index,
	const lxs_netlist *nl,
	const int32_t *comb_driver)
	{
	const lxs_gate_ir *gate = &nl->gates[gate_index];
	uint32_t gate_level = 0U;

	for (uint32_t i = 0; i < gate->input_count; ++i)
		{
		int32_t driver = comb_driver[gate->inputs[i]];
		if (driver >= 0)
			{
			uint32_t input_level = nl->gates[(uint32_t)driver].level + 1U;
			if (input_level > gate_level)
				{
				gate_level = input_level;
				}
			}
		}

	return gate_level;
	}

static uint8_t lxs_source_multi_macro_use_unit_level(uint32_t type)
	{
	switch (type)
		{
		case LXS_SOURCE_MULTI_MACRO_HALF_ADDER:
		case LXS_SOURCE_MULTI_MACRO_FULL_ADDER:
		case LXS_SOURCE_MULTI_MACRO_RIPPLE_SLICE2:
		case LXS_SOURCE_MULTI_MACRO_RIPPLE_ADD4:
		case LXS_SOURCE_MULTI_MACRO_CARRY_SAVE_ROW4:
		case LXS_SOURCE_MULTI_MACRO_REDUCE_PROPAGATE4:
			return 1U;
		default:
			return 0U;
		}
	}

static uint32_t lxs_compute_source_multi_macro_unit_level(
	const lxs_netlist *nl,
	const int32_t *comb_driver,
	const lxs_source_multi_macro *macro)
	{
	uint32_t macro_level = 0U;

	for (uint32_t j = 0; j < macro->input_count; ++j)
		{
		int32_t driver = comb_driver[macro->inputs[j]];
		if (driver >= 0)
			{
			uint32_t input_level = nl->gates[(uint32_t)driver].level + 1U;
			if (input_level > macro_level)
				{
				macro_level = input_level;
				}
			}
		}

	return macro_level;
	}

static uint32_t lxs_compute_source_multi_macro_level(
	const lxs_netlist *nl,
	const int32_t *comb_driver,
	const lxs_source_multi_macro *macro)
	{
	uint32_t local_levels[40];
	uint32_t macro_level = 0U;

	if (lxs_source_multi_macro_use_unit_level(macro->type))
		{
		return lxs_compute_source_multi_macro_unit_level(nl, comb_driver, macro);
		}

	for (uint32_t j = 0; j < macro->gate_count; ++j)
		{
		const lxs_gate_ir *gate = &nl->gates[macro->gate_indices[j]];
		uint32_t gate_level = 0U;

		for (uint32_t k = 0; k < gate->input_count; ++k)
			{
			int32_t driver = comb_driver[gate->inputs[k]];
			uint32_t input_level = 0U;

			if (driver >= 0)
				{
				uint8_t local_found = 0U;
				for (uint32_t m = 0; m < j; ++m)
					{
					if (macro->gate_indices[m] == (uint32_t)driver)
						{
						input_level = local_levels[m];
						local_found = 1U;
						break;
						}
					}
				if (!local_found)
					{
					input_level = nl->gates[(uint32_t)driver].level;
					}
				}

			if ((input_level + 1U) > gate_level)
				{
				gate_level = input_level + 1U;
				}
			}

		local_levels[j] = gate_level;
		if (gate_level > macro_level)
			{
			macro_level = gate_level;
			}
		}

	return macro_level;
	}

static uint8_t lxs_propagate_source_multi_output_levels(
	const lxs_netlist *nl,
	const int32_t *comb_driver)
	{
	uint32_t *net_consumer_count;
	uint32_t *net_consumer_start;
	uint32_t *net_consumer_fill;
	uint32_t *net_consumers;
	uint32_t *gate_macro_count;
	uint32_t *gate_macro_start;
	uint32_t *gate_macro_fill;
	uint32_t *gate_macros;
	uint32_t *gate_queue;
	uint32_t *macro_queue;
	uint8_t *gate_queued;
	uint8_t *macro_queued;
	uint32_t gate_head = 0U;
	uint32_t gate_tail = 0U;
	uint32_t gate_queue_count = 0U;
	uint32_t macro_head = 0U;
	uint32_t macro_tail = 0U;
	uint32_t macro_queue_count = 0U;
	uint32_t macro_queue_cap = nl->source_multi_macro_count ? nl->source_multi_macro_count : 1U;

	net_consumer_count = lxs_calloc_aligned(nl->net_count, sizeof(uint32_t));
	net_consumer_start = lxs_calloc_aligned(nl->net_count + 1U, sizeof(uint32_t));
	net_consumer_fill = lxs_calloc_aligned(nl->net_count, sizeof(uint32_t));
	gate_macro_count = lxs_calloc_aligned(nl->gate_count, sizeof(uint32_t));
	gate_macro_start = lxs_calloc_aligned(nl->gate_count + 1U, sizeof(uint32_t));
	gate_macro_fill = lxs_calloc_aligned(nl->gate_count, sizeof(uint32_t));
	gate_queue = lxs_calloc_aligned(nl->gate_count, sizeof(uint32_t));
	macro_queue = lxs_calloc_aligned(nl->source_multi_macro_count ? nl->source_multi_macro_count : 1U, sizeof(uint32_t));
	gate_queued = lxs_calloc_aligned(nl->gate_count, sizeof(uint8_t));
	macro_queued = lxs_calloc_aligned(nl->source_multi_macro_count ? nl->source_multi_macro_count : 1U, sizeof(uint8_t));
	if (!net_consumer_count || !net_consumer_start || !net_consumer_fill ||
		!gate_macro_count || !gate_macro_start || !gate_macro_fill ||
		!gate_queue || !macro_queue || !gate_queued || !macro_queued)
		{
		lxs_free_aligned(net_consumer_count);
		lxs_free_aligned(net_consumer_start);
		lxs_free_aligned(net_consumer_fill);
		lxs_free_aligned(gate_macro_count);
		lxs_free_aligned(gate_macro_start);
		lxs_free_aligned(gate_macro_fill);
		lxs_free_aligned(gate_queue);
		lxs_free_aligned(macro_queue);
		lxs_free_aligned(gate_queued);
		lxs_free_aligned(macro_queued);
		return 0U;
		}

	for (uint32_t i = 0; i < nl->gate_count; ++i)
		{
		for (uint32_t j = 0; j < nl->gates[i].input_count; ++j)
			{
			net_consumer_count[nl->gates[i].inputs[j]]++;
			}
		}

	for (uint32_t i = 0; i < nl->net_count; ++i)
		{
		net_consumer_start[i + 1U] = net_consumer_start[i] + net_consumer_count[i];
		}

	net_consumers = lxs_calloc_aligned(net_consumer_start[nl->net_count], sizeof(uint32_t));
	gate_macros = lxs_calloc_aligned(
		nl->source_multi_macro_count ?
			(size_t)nl->source_multi_macro_count *
			(sizeof(nl->source_multi_macros[0].gate_indices) / sizeof(nl->source_multi_macros[0].gate_indices[0])) :
			1U,
		sizeof(uint32_t));
	if (!net_consumers || !gate_macros)
		{
		lxs_free_aligned(net_consumer_count);
		lxs_free_aligned(net_consumer_start);
		lxs_free_aligned(net_consumer_fill);
		lxs_free_aligned(gate_macro_count);
		lxs_free_aligned(gate_macro_start);
		lxs_free_aligned(gate_macro_fill);
		lxs_free_aligned(gate_queue);
		lxs_free_aligned(macro_queue);
		lxs_free_aligned(gate_queued);
		lxs_free_aligned(macro_queued);
		lxs_free_aligned(net_consumers);
		lxs_free_aligned(gate_macros);
		return 0U;
		}

	memcpy(net_consumer_fill, net_consumer_start, (size_t)nl->net_count * sizeof(uint32_t));
	for (uint32_t i = 0; i < nl->gate_count; ++i)
		{
		for (uint32_t j = 0; j < nl->gates[i].input_count; ++j)
			{
			uint32_t net_id = nl->gates[i].inputs[j];
			net_consumers[net_consumer_fill[net_id]++] = i;
			}
		}

	for (uint32_t i = 0; i < nl->source_multi_macro_count; ++i)
		{
		const lxs_source_multi_macro *macro = &nl->source_multi_macros[i];
		for (uint32_t j = 0; j < macro->input_count; ++j)
			{
			int32_t driver = comb_driver[macro->inputs[j]];
			if (driver >= 0)
				{
				gate_macro_count[(uint32_t)driver]++;
				}
			}
		}

	for (uint32_t i = 0; i < nl->gate_count; ++i)
		{
		gate_macro_start[i + 1U] = gate_macro_start[i] + gate_macro_count[i];
		}

	memcpy(gate_macro_fill, gate_macro_start, (size_t)nl->gate_count * sizeof(uint32_t));
	for (uint32_t i = 0; i < nl->source_multi_macro_count; ++i)
		{
		const lxs_source_multi_macro *macro = &nl->source_multi_macros[i];
		for (uint32_t j = 0; j < macro->input_count; ++j)
			{
			int32_t driver = comb_driver[macro->inputs[j]];
			if (driver >= 0)
				{
				gate_macros[gate_macro_fill[(uint32_t)driver]++] = i;
				}
			}
		}

	for (uint32_t i = 0; i < nl->source_multi_macro_count; ++i)
		{
		macro_queue[macro_tail] = i;
		macro_tail = (macro_tail + 1U) % macro_queue_cap;
		macro_queue_count++;
		macro_queued[i] = 1U;
		}

	while (macro_queue_count > 0U || gate_queue_count > 0U)
		{
		while (macro_queue_count > 0U)
			{
			uint32_t macro_index = macro_queue[macro_head];
			const lxs_source_multi_macro *macro = &nl->source_multi_macros[macro_index];
			uint32_t macro_level;

			macro_head = (macro_head + 1U) % macro_queue_cap;
			macro_queue_count--;
			macro_queued[macro_index] = 0U;
			macro_level = lxs_compute_source_multi_macro_level(nl, comb_driver, macro);
			for (uint32_t j = 0; j < macro->output_count; ++j)
				{
				int32_t driver = comb_driver[macro->outputs[j]];
				if (driver >= 0 && nl->gates[(uint32_t)driver].level < macro_level)
					{
					uint32_t gate_index = (uint32_t)driver;

					nl->gates[gate_index].level = macro_level;
					if (!gate_queued[gate_index])
						{
						gate_queue[gate_tail] = gate_index;
						gate_tail = (gate_tail + 1U) % nl->gate_count;
						gate_queue_count++;
						gate_queued[gate_index] = 1U;
						}
					}
				}
			}

		while (gate_queue_count > 0U)
			{
			uint32_t gate_index = gate_queue[gate_head];
			uint32_t net_id = nl->gates[gate_index].output;

			gate_head = (gate_head + 1U) % nl->gate_count;
			gate_queue_count--;
			gate_queued[gate_index] = 0U;
			for (uint32_t i = net_consumer_start[net_id]; i < net_consumer_start[net_id + 1U]; ++i)
				{
				uint32_t consumer = net_consumers[i];
				uint32_t consumer_level = lxs_recompute_gate_level(consumer, nl, comb_driver);

					if (consumer_level > nl->gates[consumer].level)
						{
						nl->gates[consumer].level = consumer_level;
						if (!gate_queued[consumer])
							{
							gate_queue[gate_tail] = consumer;
							gate_tail = (gate_tail + 1U) % nl->gate_count;
							gate_queue_count++;
							gate_queued[consumer] = 1U;
							}
						}
				}

			for (uint32_t i = gate_macro_start[gate_index]; i < gate_macro_start[gate_index + 1U]; ++i)
				{
				uint32_t macro_index = gate_macros[i];
				if (!macro_queued[macro_index])
					{
					macro_queue[macro_tail] = macro_index;
					macro_tail = (macro_tail + 1U) % macro_queue_cap;
					macro_queue_count++;
					macro_queued[macro_index] = 1U;
					}
				}
			}
		}

	lxs_free_aligned(net_consumer_count);
	lxs_free_aligned(net_consumer_start);
	lxs_free_aligned(net_consumer_fill);
	lxs_free_aligned(net_consumers);
	lxs_free_aligned(gate_macro_count);
	lxs_free_aligned(gate_macro_start);
	lxs_free_aligned(gate_macro_fill);
	lxs_free_aligned(gate_macros);
	lxs_free_aligned(gate_queue);
	lxs_free_aligned(macro_queue);
	lxs_free_aligned(gate_queued);
	lxs_free_aligned(macro_queued);
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

static int lxs_compare_macros(const void *lhs, const void *rhs)
	{
	const lxs_macro_plan *a = (const lxs_macro_plan*)lhs;
	const lxs_macro_plan *b = (const lxs_macro_plan*)rhs;

	if (a->level != b->level)
		{
		return a->level < b->level ? -1 : 1;
		}

	if (a->output != b->output)
		{
		return a->output < b->output ? -1 : 1;
		}

	if (a->type != b->type)
		{
		return a->type < b->type ? -1 : 1;
		}

	return 0;
	}

static int lxs_compare_multi_macros(const void *lhs, const void *rhs)
	{
	const lxs_multi_macro_plan *a = (const lxs_multi_macro_plan*)lhs;
	const lxs_multi_macro_plan *b = (const lxs_multi_macro_plan*)rhs;

	if (a->level != b->level)
		{
		return a->level < b->level ? -1 : 1;
		}

	if (a->outputs[0] != b->outputs[0])
		{
		return a->outputs[0] < b->outputs[0] ? -1 : 1;
		}

	if (a->type != b->type)
		{
		return a->type < b->type ? -1 : 1;
		}

	return 0;
	}

static int lxs_compare_standard_muxes(const void *lhs, const void *rhs)
	{
	const lxs_standard_mux_plan *a = (const lxs_standard_mux_plan*)lhs;
	const lxs_standard_mux_plan *b = (const lxs_standard_mux_plan*)rhs;

	if (a->level != b->level)
		{
		return a->level < b->level ? -1 : 1;
		}

	if (a->output_start != b->output_start)
		{
		return a->output_start < b->output_start ? -1 : 1;
		}

	if (a->kind != b->kind)
		{
		return a->kind < b->kind ? -1 : 1;
		}

	return 0;
	}

static int lxs_compare_standard_adders(const void *lhs, const void *rhs)
	{
	const lxs_standard_add_plan *a = (const lxs_standard_add_plan*)lhs;
	const lxs_standard_add_plan *b = (const lxs_standard_add_plan*)rhs;

	if (a->level < b->level)
		{
		return -1;
		}
	if (a->level > b->level)
		{
		return 1;
		}
	if (a->output_start < b->output_start)
		{
		return -1;
		}
	if (a->output_start > b->output_start)
		{
		return 1;
		}
	return 0;
	}

static int lxs_compare_standard_cmps(const void *lhs, const void *rhs)
	{
	const lxs_standard_cmp_plan *a = (const lxs_standard_cmp_plan*)lhs;
	const lxs_standard_cmp_plan *b = (const lxs_standard_cmp_plan*)rhs;

	if (a->level < b->level)
		{
		return -1;
		}
	if (a->level > b->level)
		{
		return 1;
		}
	if (a->input_start < b->input_start)
		{
		return -1;
		}
	if (a->input_start > b->input_start)
		{
		return 1;
		}
	return 0;
	}

static int lxs_compare_standard_alus(const void *lhs, const void *rhs)
	{
	const lxs_standard_alu_plan *a = (const lxs_standard_alu_plan*)lhs;
	const lxs_standard_alu_plan *b = (const lxs_standard_alu_plan*)rhs;

	if (a->level < b->level)
		{
		return -1;
		}
	if (a->level > b->level)
		{
		return 1;
		}
	if (a->input_start < b->input_start)
		{
		return -1;
		}
	if (a->input_start > b->input_start)
		{
		return 1;
		}
	return 0;
	}

static int lxs_compare_functional_regions(const void *lhs, const void *rhs)
	{
	const lxs_functional_region_plan *left = (const lxs_functional_region_plan*)lhs;
	const lxs_functional_region_plan *right = (const lxs_functional_region_plan*)rhs;

	if (left->level < right->level)
		{
		return -1;
		}
	if (left->level > right->level)
		{
		return 1;
		}
	if (left->outputs[0] < right->outputs[0])
		{
		return -1;
		}
	if (left->outputs[0] > right->outputs[0])
		{
		return 1;
		}
	if (left->output_count < right->output_count)
		{
		return -1;
		}
	if (left->output_count > right->output_count)
		{
		return 1;
		}
	return 0;
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

static void lxs_apply_net_remap_to_source_multi_macros(
	lxs_source_multi_macro *macros,
	uint32_t count,
	const uint32_t *remap)
	{
	for (uint32_t i = 0; i < count; ++i)
		{
		for (uint32_t j = 0; j < macros[i].input_count; ++j)
			{
			macros[i].inputs[j] = remap[macros[i].inputs[j]];
			}
		for (uint32_t j = 0; j < macros[i].output_count; ++j)
			{
			macros[i].outputs[j] = remap[macros[i].outputs[j]];
			}
		}
	}

static void lxs_apply_net_remap_to_source_standard_muxes(
	lxs_netlist *nl,
	const uint32_t *remap)
	{
	lxs_apply_net_remap_to_array(
		nl->source_standard_mux_data_net_ids,
		nl->source_standard_mux_data_count,
		remap);
	lxs_apply_net_remap_to_array(
		nl->source_standard_mux_select_net_ids,
		nl->source_standard_mux_select_count,
		remap);
	lxs_apply_net_remap_to_array(
		nl->source_standard_mux_output_net_ids,
		nl->source_standard_mux_output_count,
		remap);
	}

static void lxs_apply_net_remap_to_source_standard_adders(
	lxs_netlist *nl,
	const uint32_t *remap)
	{
	lxs_apply_net_remap_to_array(
		nl->source_standard_add_input_net_ids,
		nl->source_standard_add_input_count,
		remap);
	lxs_apply_net_remap_to_array(
		nl->source_standard_add_output_net_ids,
		nl->source_standard_add_output_count,
		remap);
	}

static void lxs_apply_net_remap_to_source_standard_cmps(
	lxs_netlist *nl,
	const uint32_t *remap)
	{
	lxs_apply_net_remap_to_array(
		nl->source_standard_cmp_input_net_ids,
		nl->source_standard_cmp_input_count,
		remap);
	lxs_apply_net_remap_to_array(
		nl->source_standard_cmp_output_net_ids,
		nl->source_standard_cmp_output_count,
		remap);
	}

static void lxs_apply_net_remap_to_source_standard_alus(
	lxs_netlist *nl,
	const uint32_t *remap)
	{
	lxs_apply_net_remap_to_array(
		nl->source_standard_alu_input_net_ids,
		nl->source_standard_alu_input_count,
		remap);
	lxs_apply_net_remap_to_array(
		nl->source_standard_alu_output_net_ids,
		nl->source_standard_alu_output_count,
		remap);
	}

static void lxs_apply_net_remap_to_source_macros(
	lxs_source_macro *macros,
	uint32_t count,
	const uint32_t *remap)
	{
	for (uint32_t i = 0; i < count; ++i)
		{
		for (uint32_t j = 0; j < macros[i].input_count; ++j)
			{
			macros[i].inputs[j] = remap[macros[i].inputs[j]];
			}
		macros[i].output = remap[macros[i].output];
		}
	}

static void lxs_apply_net_remap_to_source_registers(
	lxs_netlist *nl,
	const uint32_t *remap)
	{
	lxs_apply_net_remap_to_array(
		nl->source_register_input_net_ids,
		nl->source_register_input_count,
		remap);
	lxs_apply_net_remap_to_array(
		nl->source_register_output_net_ids,
		nl->source_register_output_count,
		remap);
	for (uint32_t i = 0; i < nl->source_register_count; ++i)
		{
		if (nl->source_registers[i].control_net != UINT32_MAX)
			{
			nl->source_registers[i].control_net = remap[nl->source_registers[i].control_net];
			}
		if (nl->source_registers[i].aux_control_net != UINT32_MAX)
			{
			nl->source_registers[i].aux_control_net = remap[nl->source_registers[i].aux_control_net];
			}
		}
	}

static void lxs_apply_net_remap_to_source_roms(
	lxs_netlist *nl,
	const uint32_t *remap)
	{
	lxs_apply_net_remap_to_array(
		nl->source_rom_addr_net_ids,
		nl->source_rom_addr_count,
		remap);
	lxs_apply_net_remap_to_array(
		nl->source_rom_output_net_ids,
		nl->source_rom_output_count,
		remap);
	}

static void lxs_apply_net_remap_to_source_rams(
	lxs_netlist *nl,
	const uint32_t *remap)
	{
	lxs_apply_net_remap_to_array(
		nl->source_ram_read_addr_net_ids,
		nl->source_ram_read_addr_count,
		remap);
	lxs_apply_net_remap_to_array(
		nl->source_ram_write_addr_net_ids,
		nl->source_ram_write_addr_count,
		remap);
	lxs_apply_net_remap_to_array(
		nl->source_ram_data_input_net_ids,
		nl->source_ram_data_input_count,
		remap);
	lxs_apply_net_remap_to_array(
		nl->source_ram_output_net_ids,
		nl->source_ram_output_count,
		remap);
	if (nl->source_ram_count > 0U)
		{
		for (uint32_t i = 0; i < nl->source_ram_count; ++i)
			{
			nl->source_rams[i].write_enable_net = remap[nl->source_rams[i].write_enable_net];
			}
		}
	}

static void lxs_apply_net_remap_to_source_regfiles(
	lxs_netlist *nl,
	const uint32_t *remap)
	{
	lxs_apply_net_remap_to_array(
		nl->source_regfile_read_a_addr_net_ids,
		nl->source_regfile_read_a_addr_count,
		remap);
	lxs_apply_net_remap_to_array(
		nl->source_regfile_read_b_addr_net_ids,
		nl->source_regfile_read_b_addr_count,
		remap);
	lxs_apply_net_remap_to_array(
		nl->source_regfile_write_addr_net_ids,
		nl->source_regfile_write_addr_count,
		remap);
	lxs_apply_net_remap_to_array(
		nl->source_regfile_data_input_net_ids,
		nl->source_regfile_data_input_count,
		remap);
	lxs_apply_net_remap_to_array(
		nl->source_regfile_output_net_ids,
		nl->source_regfile_output_count,
		remap);
	if (nl->source_regfile_count > 0U)
		{
		for (uint32_t i = 0; i < nl->source_regfile_count; ++i)
			{
			nl->source_regfiles[i].write_enable_net = remap[nl->source_regfiles[i].write_enable_net];
			}
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

static int32_t lxs_find_unique_consumer(
	const lxs_netlist *nl,
	uint32_t net_id,
	uint32_t type,
	int32_t exclude_index)
	{
	int32_t found = -1;

	for (uint32_t i = 0; i < nl->gate_count; ++i)
		{
		if ((int32_t)i == exclude_index)
			{
			continue;
			}

		if (nl->gates[i].type != type || !lxs_gate_uses_input(&nl->gates[i], net_id))
			{
			continue;
			}

		if (found >= 0)
			{
			return -1;
			}

		found = (int32_t)i;
		}

	return found;
	}

typedef struct lxs_fa_cinv_desc lxs_fa_cinv_desc;
struct lxs_fa_cinv_desc
	{
	uint32_t a;
	uint32_t b;
	uint32_t cin_n;
	uint32_t sum_out;
	uint32_t carry_out;
	uint32_t level;
	uint32_t gate_indices[6];
	};

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

static uint32_t lxs_default_recognition_mask(void)
	{
	const char *text = getenv("LXS_RECOGNITION_MASK");
	char *end = NULL;
	unsigned long value;

	if (!text || text[0] == '\0')
		{
		return 1UL << LXS_RECOGNITION_FAMILY_PARITY;
		}

	value = strtoul(text, &end, 0);
	if (!end || *end != '\0')
		{
		return 1UL << LXS_RECOGNITION_FAMILY_PARITY;
		}

	return (uint32_t)value;
	}

static uint32_t lxs_default_recognition_mode(void)
	{
	const char *text = getenv("LXS_RECOGNITION_MODE");

	if (!text || text[0] == '\0')
		{
		return LXS_RECOGNITION_MODE_REPLACE;
		}

	if (strcmp(text, "report") == 0 || strcmp(text, "report_only") == 0)
		{
		return LXS_RECOGNITION_MODE_REPORT_ONLY;
		}

	return LXS_RECOGNITION_MODE_REPLACE;
	}

static int lxs_recognition_family_enabled(uint32_t mask, uint32_t family)
	{
	if (family >= LXS_RECOGNITION_FAMILY_COUNT)
		{
		return 0;
		}

	return ((mask >> family) & 1U) != 0U;
	}

static void lxs_record_recognition(
	uint32_t *match_count,
	uint32_t *node_reduction,
	uint32_t family,
	uint32_t matched_gate_count)
	{
	if (!match_count || !node_reduction || family >= LXS_RECOGNITION_FAMILY_COUNT)
		{
		return;
		}

	match_count[family]++;
	node_reduction[family] += matched_gate_count > 0U ? matched_gate_count - 1U : 0U;
	}

static void lxs_record_recognition_candidate(
	uint64_t *candidate_roots,
	uint32_t family)
	{
	if (!candidate_roots || family >= LXS_RECOGNITION_FAMILY_COUNT)
		{
		return;
		}

	candidate_roots[family]++;
	}

static void lxs_record_recognition_visit(
	uint64_t *nodes_visited,
	uint32_t *max_depth_reached,
	uint32_t family,
	uint32_t depth)
	{
	if (family >= LXS_RECOGNITION_FAMILY_COUNT)
		{
		return;
		}

	if (nodes_visited)
		{
		nodes_visited[family]++;
		}
	if (max_depth_reached && depth > max_depth_reached[family])
		{
		max_depth_reached[family] = depth;
		}
	}

static void lxs_record_recognition_abort(
	uint64_t abort_count[LXS_RECOGNITION_FAMILY_COUNT][LXS_RECOGNITION_ABORT_REASON_COUNT],
	uint32_t family,
	uint32_t reason)
	{
	if (!abort_count ||
		family >= LXS_RECOGNITION_FAMILY_COUNT ||
		reason >= LXS_RECOGNITION_ABORT_REASON_COUNT)
		{
		return;
		}

	abort_count[family][reason]++;
	}

static void lxs_record_recognition_time(
	uint64_t *time_us,
	uint32_t family,
	clock_t start_clock,
	clock_t end_clock)
	{
	double delta_seconds;

	if (!time_us || family >= LXS_RECOGNITION_FAMILY_COUNT)
		{
		return;
		}

	delta_seconds = (double)(end_clock - start_clock) / (double)CLOCKS_PER_SEC;
	if (delta_seconds > 0.0)
		{
		time_us[family] += (uint64_t)(delta_seconds * 1000000.0);
		}
	}

static const lxs_recognition_pattern lxs_pattern_parity4 =
	{
	LXS_RECOGNITION_FAMILY_PARITY,
	LXS_RECOGNITION_PATTERN_REDUCTION_TREE,
	LXS_GATE_XOR,
	LXS_GATE_XOR,
	4U,
	3U,
	LXS_MACRO_PARITY4,
	0U,
	0U
	};

static const lxs_recognition_pattern lxs_pattern_parity8 =
	{
	LXS_RECOGNITION_FAMILY_PARITY,
	LXS_RECOGNITION_PATTERN_REDUCTION_TREE,
	LXS_GATE_XOR,
	LXS_GATE_XOR,
	8U,
	7U,
	LXS_MACRO_PARITY8,
	0U,
	0U
	};

static const lxs_recognition_legality lxs_legality_strict_reduction =
	{
	1U,
	1U,
	1U,
	0U,
	8U,
	8U
	};

static int lxs_collect_reduction_leaves(
	const lxs_netlist *nl,
	const int32_t *comb_driver,
	const uint32_t *net_use_count,
	const uint8_t *matched_gates,
	uint32_t internal_gate_type,
	const lxs_recognition_legality *legality,
	uint32_t net_id,
	uint32_t max_leaves,
	uint32_t *leaves,
	uint32_t *leaf_count,
	uint32_t *gate_indices,
	uint32_t *gate_count)
	{
	int32_t driver = comb_driver[net_id];
	uint32_t leaf_slot;

	if (driver < 0)
		{
		if (*leaf_count >= max_leaves)
			{
			return 0;
			}
		leaf_slot = *leaf_count;
		leaves[leaf_slot] = net_id;
		*leaf_count = leaf_slot + 1U;
		return 1;
		}

	{
	const lxs_gate_ir *gate = &nl->gates[(uint32_t)driver];
	if (gate->type != internal_gate_type ||
		gate->input_count != 2U ||
		(legality && legality->forbid_matched_overlap && matched_gates[(uint32_t)driver]) ||
		(legality && legality->require_single_use_internal && net_use_count[net_id] != 1U))
		{
		if (*leaf_count >= max_leaves)
			{
			return 0;
			}
		leaf_slot = *leaf_count;
		leaves[leaf_slot] = net_id;
		*leaf_count = leaf_slot + 1U;
		return 1;
		}

	if (!lxs_collect_reduction_leaves(
		nl,
		comb_driver,
		net_use_count,
		matched_gates,
		internal_gate_type,
		legality,
		gate->inputs[0],
		max_leaves,
		leaves,
		leaf_count,
		gate_indices,
		gate_count) ||
		!lxs_collect_reduction_leaves(
			nl,
			comb_driver,
			net_use_count,
			matched_gates,
			internal_gate_type,
			legality,
			gate->inputs[1],
			max_leaves,
			leaves,
			leaf_count,
			gate_indices,
			gate_count))
		{
		return 0;
		}

	gate_indices[*gate_count] = (uint32_t)driver;
	(*gate_count)++;
	return 1;
	}
	}

static int lxs_try_match_reduction_pattern(
	const lxs_netlist *nl,
	const int32_t *comb_driver,
	const uint32_t *net_use_count,
	uint8_t *matched_gates,
	uint32_t gate_index,
	const lxs_recognition_pattern *pattern,
	const lxs_recognition_legality *legality,
	lxs_macro_plan *macro)
	{
	const lxs_gate_ir *root = &nl->gates[gate_index];
	uint32_t leaves[8];
	uint32_t gate_indices[7];
	uint32_t leaf_count = 0U;
	uint32_t gate_count = 0U;

	if (!pattern || pattern->kind != LXS_RECOGNITION_PATTERN_REDUCTION_TREE)
		{
		return 0;
		}

	if (root->type != pattern->root_gate_type ||
		root->input_count != 2U ||
		(legality && legality->forbid_matched_overlap && matched_gates[gate_index]))
		{
		return 0;
		}

	if (!lxs_collect_reduction_leaves(
		nl,
		comb_driver,
		net_use_count,
		matched_gates,
		pattern->internal_gate_type,
		legality,
		root->inputs[0],
		pattern->leaf_count,
		leaves,
		&leaf_count,
		gate_indices,
		&gate_count) ||
		!lxs_collect_reduction_leaves(
			nl,
			comb_driver,
			net_use_count,
			matched_gates,
			pattern->internal_gate_type,
			legality,
			root->inputs[1],
			pattern->leaf_count,
			leaves,
			&leaf_count,
			gate_indices,
			&gate_count))
		{
		return 0;
		}

	gate_indices[gate_count++] = gate_index;
	if (leaf_count != pattern->leaf_count ||
		gate_count != pattern->gate_count ||
		(legality && legality->max_nodes > 0U && gate_count > legality->max_nodes) ||
		(legality && legality->max_leaves > 0U && leaf_count > legality->max_leaves))
		{
		return 0;
		}

	for (uint32_t i = 0; i < gate_count; ++i)
		{
		matched_gates[gate_indices[i]] = 1U;
		}

	memset(macro, 0, sizeof(*macro));
	macro->type = pattern->macro_type;
	macro->level = root->level;
	macro->output = root->output;
	for (uint32_t i = 0; i < pattern->leaf_count; ++i)
		{
		macro->inputs[i] = leaves[i];
		}
	macro->gate_equiv_count = gate_count;
	return 1;
	}

static uint32_t lxs_collect_macros(
	const lxs_netlist *nl,
	const int32_t *comb_driver,
	uint8_t *matched_gates,
	uint32_t recognition_mask,
	uint32_t recognition_mode,
	uint32_t *family_match_count,
	uint32_t *family_node_reduction,
	uint64_t *family_candidate_roots,
	uint64_t *family_nodes_visited,
	uint32_t *family_max_depth_reached,
	uint64_t family_abort_count[LXS_RECOGNITION_FAMILY_COUNT][LXS_RECOGNITION_ABORT_REASON_COUNT],
	uint64_t *family_time_us,
	lxs_macro_plan *macros)
	{
	uint32_t *net_use_count;
	uint32_t macro_count = 0U;

	net_use_count = lxs_calloc_aligned(nl->net_count, sizeof(uint32_t));
	if (!net_use_count)
		{
		lxs_free_aligned(net_use_count);
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

	if (lxs_recognition_family_enabled(recognition_mask, LXS_RECOGNITION_FAMILY_PARITY))
		{
		clock_t family_start = clock();
		for (uint32_t i = 0; i < nl->gate_count; ++i)
			{
			lxs_macro_plan macro;

			if (lxs_try_match_reduction_pattern(
					nl,
					comb_driver,
					net_use_count,
					matched_gates,
					i,
					&lxs_pattern_parity8,
					&lxs_legality_strict_reduction,
					&macro) ||
				lxs_try_match_reduction_pattern(
					nl,
					comb_driver,
					net_use_count,
					matched_gates,
					i,
					&lxs_pattern_parity4,
					&lxs_legality_strict_reduction,
					&macro))
				{
				if (macros)
					{
					macros[macro_count] = macro;
					}
				lxs_record_recognition(
					family_match_count,
					family_node_reduction,
					LXS_RECOGNITION_FAMILY_PARITY,
					macro.gate_equiv_count);
				macro_count++;
				}
			}
		lxs_record_recognition_time(
			family_time_us,
			LXS_RECOGNITION_FAMILY_PARITY,
			family_start,
			clock());
		}

	if (recognition_mode == LXS_RECOGNITION_MODE_REPORT_ONLY &&
		lxs_recognition_family_enabled(recognition_mask, LXS_RECOGNITION_FAMILY_FUNCTIONAL))
		{
		clock_t family_start = clock();
		for (uint32_t i = 0; i < nl->gate_count; ++i)
			{
			uint32_t matched_gate_count = 0U;
			uint32_t gate_equiv_count = 0U;

			if (!lxs_try_match_functional_cone_rooted(
					nl,
					comb_driver,
					net_use_count,
					matched_gates,
					i,
					family_candidate_roots,
					family_nodes_visited,
					family_max_depth_reached,
					family_abort_count,
					&matched_gate_count,
					&gate_equiv_count))
				{
				continue;
				}

			lxs_record_recognition(
				family_match_count,
				family_node_reduction,
				LXS_RECOGNITION_FAMILY_FUNCTIONAL,
				matched_gate_count);
			if (gate_equiv_count > 0U)
				{
				macro_count++;
				}
			}
		lxs_record_recognition_time(
			family_time_us,
			LXS_RECOGNITION_FAMILY_FUNCTIONAL,
			family_start,
			clock());
		}

	for (uint32_t i = 0; i < nl->gate_count; ++i)
		{
		lxs_macro_plan macro;

		if (lxs_try_match_sum_cinv2(nl, comb_driver, net_use_count, matched_gates, i, &macro))
			{
			if (macros)
				{
				macros[macro_count] = macro;
				}
			macro_count++;
			}
		}

	for (uint32_t i = 0; i < nl->gate_count; ++i)
		{
		lxs_macro_plan macro;

		if (lxs_try_match_xnor2(nl, comb_driver, net_use_count, matched_gates, i, &macro) ||
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

static uint32_t lxs_collect_source_macros(
	const lxs_netlist *nl,
	uint8_t *matched_gates,
	lxs_macro_plan *macros)
	{
	uint32_t macro_count = 0U;

	for (uint32_t i = 0; i < nl->source_macro_count; ++i)
		{
		lxs_macro_plan macro;
		const lxs_source_macro *source = &nl->source_macros[i];

		for (uint32_t j = 0; j < source->gate_count; ++j)
			{
			matched_gates[source->gate_indices[j]] = 1U;
			}

		memset(&macro, 0, sizeof(macro));
		macro.type = (source->type == LXS_SOURCE_MACRO_PARITY8) ? LXS_MACRO_PARITY8 : LXS_MACRO_PARITY4;
		macro.level = nl->gates[source->gate_indices[source->gate_count - 1U]].level;
		macro.output = source->output;
		for (uint32_t j = 0; j < source->input_count; ++j)
			{
			macro.inputs[j] = source->inputs[j];
			}
		macro.gate_equiv_count = source->gate_count;

		if (macros)
			{
			macros[macro_count] = macro;
			}
		macro_count++;
		}

	return macro_count;
	}

static uint32_t lxs_collect_source_standard_muxes(
	const lxs_netlist *nl,
	uint8_t *matched_gates,
	lxs_standard_mux_plan *muxes)
	{
	uint32_t mux_count = 0U;

	for (uint32_t i = 0; i < nl->source_standard_mux_count; ++i)
		{
		const lxs_source_standard_mux *source = &nl->source_standard_muxes[i];
		lxs_standard_mux_plan mux;
		uint32_t level = 0U;

		memset(&mux, 0, sizeof(mux));
		mux.kind = source->kind;
		mux.width_bits = source->width_bits;
		mux.data_start = source->data_start;
		mux.select_start = source->select_start;
		mux.output_start = source->output_start;
		mux.gate_equiv_count = source->gate_count;

		for (uint32_t j = 0; j < source->gate_count; ++j)
			{
			uint32_t gate_index = nl->source_standard_mux_gate_indices[source->gate_index_start + j];
			matched_gates[gate_index] = 1U;
			if (nl->gates[gate_index].level > level)
				{
				level = nl->gates[gate_index].level;
				}
			}

		mux.level = level;
		if (muxes)
			{
			muxes[mux_count] = mux;
			}
		mux_count++;
		}

	return mux_count;
	}

static uint32_t lxs_collect_source_standard_adders(
	const lxs_netlist *nl,
	uint8_t *matched_gates,
	lxs_standard_add_plan *adders)
	{
	uint32_t adder_count = 0U;

	for (uint32_t i = 0; i < nl->source_standard_add_count; ++i)
		{
		const lxs_source_standard_add *source = &nl->source_standard_adders[i];
		lxs_standard_add_plan adder;
		uint32_t level = 0U;

		memset(&adder, 0, sizeof(adder));
		adder.kind = source->kind;
		adder.width_bits = source->width_bits;
		adder.input_start = source->input_start;
		adder.output_start = source->output_start;
		adder.gate_equiv_count = source->gate_count;

		for (uint32_t j = 0; j < source->gate_count; ++j)
			{
			uint32_t gate_index = nl->source_standard_add_gate_indices[source->gate_index_start + j];
			matched_gates[gate_index] = 1U;
			if (nl->gates[gate_index].level > level)
				{
				level = nl->gates[gate_index].level;
				}
			}

		adder.level = level;
		if (adders)
			{
			adders[adder_count] = adder;
			}
		adder_count++;
		}

	return adder_count;
	}

static uint32_t lxs_collect_source_standard_cmps(
	const lxs_netlist *nl,
	uint8_t *matched_gates,
	lxs_standard_cmp_plan *cmps)
	{
	uint32_t cmp_count = 0U;

	for (uint32_t i = 0; i < nl->source_standard_cmp_count; ++i)
		{
		const lxs_source_standard_cmp *source = &nl->source_standard_cmps[i];
		lxs_standard_cmp_plan cmp;
		uint32_t level = 0U;

		memset(&cmp, 0, sizeof(cmp));
		cmp.kind = source->kind;
		cmp.width_bits = source->width_bits;
		cmp.input_start = source->input_start;
		cmp.output_start = source->output_start;
		cmp.gate_equiv_count = source->gate_count;

		for (uint32_t j = 0; j < source->gate_count; ++j)
			{
			uint32_t gate_index = nl->source_standard_cmp_gate_indices[source->gate_index_start + j];
			matched_gates[gate_index] = 1U;
			if (nl->gates[gate_index].level > level)
				{
				level = nl->gates[gate_index].level;
				}
			}

		cmp.level = level;
		if (cmps)
			{
			cmps[cmp_count] = cmp;
			}
		cmp_count++;
		}

	return cmp_count;
	}

static uint32_t lxs_collect_source_standard_alus(
	const lxs_netlist *nl,
	uint8_t *matched_gates,
	lxs_standard_alu_plan *alus)
	{
	uint32_t alu_count = 0U;

	for (uint32_t i = 0; i < nl->source_standard_alu_count; ++i)
		{
		const lxs_source_standard_alu *source = &nl->source_standard_alus[i];
		lxs_standard_alu_plan alu;
		uint32_t level = 0U;

		memset(&alu, 0, sizeof(alu));
		alu.kind = source->kind;
		alu.width_bits = source->width_bits;
		alu.input_start = source->input_start;
		alu.output_start = source->output_start;
		alu.gate_equiv_count = source->gate_count;

		for (uint32_t j = 0; j < source->gate_count; ++j)
			{
			uint32_t gate_index = nl->source_standard_alu_gate_indices[source->gate_index_start + j];
			matched_gates[gate_index] = 1U;
			if (nl->gates[gate_index].level > level)
				{
				level = nl->gates[gate_index].level;
				}
			}

		alu.level = level;
		if (alus)
			{
			alus[alu_count] = alu;
			}
		alu_count++;
		}

	return alu_count;
	}

static uint32_t lxs_collect_source_functional_regions(
	const lxs_netlist *nl,
	uint8_t *matched_gates,
	lxs_functional_region_plan *regions)
	{
	uint32_t region_count = 0U;

	for (uint32_t i = 0; i < nl->source_functional_region_count; ++i)
		{
		const lxs_source_functional_region *source = &nl->source_functional_regions[i];
		lxs_functional_region_plan region;
		uint32_t level = 0U;

		memset(&region, 0, sizeof(region));
		region.exec_kind = source->exec_kind;
		region.input_count = source->input_count;
		region.output_count = source->output_count;
		region.gate_equiv_count = source->gate_count;
		region.temp_count = source->temp_count;
		region.op_count = source->op_count;
		region.node_budget = source->node_budget;
		region.max_depth = source->max_depth;
		memcpy(region.inputs, source->inputs, sizeof(region.inputs));
		memcpy(region.outputs, source->outputs, sizeof(region.outputs));

		for (uint32_t j = 0; j < source->gate_count; ++j)
			{
			uint32_t gate_index = source->gate_indices[j];

			matched_gates[gate_index] = 1U;
			if (nl->gates[gate_index].level > level)
				{
				level = nl->gates[gate_index].level;
				}
			}
		for (uint32_t j = 0; j < source->op_count; ++j)
			{
			region.ops[j].type = source->op_type[j];
			region.ops[j].dst_count = source->op_dst_count[j];
			for (uint32_t dst_index = 0; dst_index < source->op_dst_count[j]; ++dst_index)
				{
				region.ops[j].dst_kind[dst_index] = source->op_dst_kind[j][dst_index];
				region.ops[j].dst[dst_index] = source->op_dst[j][dst_index];
				}
			region.ops[j].arity = source->op_arity[j];
			for (uint32_t src_index = 0; src_index < 4U; ++src_index)
				{
				region.ops[j].src[src_index] = source->op_src[j][src_index];
				}
			}

		region.level = level;
		if (regions)
			{
			regions[region_count] = region;
			}
		region_count++;
		}

	return region_count;
	}

static uint32_t lxs_collect_recognized_functional_regions(
	const lxs_netlist *nl,
	const int32_t *comb_driver,
	const uint32_t *net_use_count,
	uint8_t *matched_gates,
	uint32_t recognition_mask,
	uint32_t *family_match_count,
	uint32_t *family_node_reduction,
	uint64_t *family_candidate_roots,
	uint64_t *family_nodes_visited,
	uint32_t *family_max_depth_reached,
	uint64_t family_abort_count[LXS_RECOGNITION_FAMILY_COUNT][LXS_RECOGNITION_ABORT_REASON_COUNT],
	uint64_t *family_time_us,
	lxs_functional_region_plan *regions)
	{
	uint32_t region_count = 0U;

	if (!lxs_recognition_family_enabled(recognition_mask, LXS_RECOGNITION_FAMILY_FUNCTIONAL))
		{
		return 0U;
		}

	{
	clock_t family_start = clock();
	for (uint32_t rev = nl->gate_count; rev > 0U; --rev)
		{
		uint32_t i = rev - 1U;
		lxs_functional_build_state state;
		lxs_functional_region_plan region;
		uint32_t gate_seen[8];
		uint32_t gate_seen_count = 0U;
		uint32_t matched_gate_count = 0U;
		uint32_t gate_equiv_count = 0U;
		uint8_t ignored_is_temp = 0U;
		uint8_t ignored_ref = 0U;

		if (!lxs_try_match_functional_cone_rooted(
				nl,
				comb_driver,
				net_use_count,
				matched_gates,
				i,
				family_candidate_roots,
				family_nodes_visited,
				family_max_depth_reached,
				family_abort_count,
				&matched_gate_count,
				&gate_equiv_count))
			{
			continue;
			}

		memset(&state, 0, sizeof(state));
		memset(&region, 0, sizeof(region));
		if (!lxs_build_functional_cone_rooted_visit(
				nl,
				comb_driver,
				i,
				gate_seen,
				&gate_seen_count,
				&state,
				1U,
				&ignored_is_temp,
				&ignored_ref))
			{
			lxs_record_recognition_abort(
				family_abort_count,
				LXS_RECOGNITION_FAMILY_FUNCTIONAL,
				LXS_RECOGNITION_ABORT_SHAPE);
			continue;
			}

		if (!lxs_functional_region_is_cache_merge(&state))
			{
			lxs_record_recognition_abort(
				family_abort_count,
				LXS_RECOGNITION_FAMILY_FUNCTIONAL,
				LXS_RECOGNITION_ABORT_SHAPE);
			continue;
			}

		region.exec_kind = lxs_choose_functional_exec_kind(state.op_count, state.temp_count);
		region.level = nl->gates[i].level;
		region.input_count = state.boundary_count;
		region.output_count = 1U;
		region.outputs[0] = nl->gates[i].output;
		region.temp_count = state.temp_count;
		region.op_count = state.op_count;
		region.node_budget = 8U;
		region.max_depth = 3U;
		region.gate_equiv_count = gate_equiv_count;
		for (uint32_t j = 0; j < state.boundary_count; ++j)
			{
			region.inputs[j] = state.boundary_inputs[j];
			}
		for (uint32_t j = 0; j < state.op_count; ++j)
			{
			region.ops[j].type = state.ops[j].type;
			region.ops[j].dst_count = 1U;
			region.ops[j].dst_kind[0] =
				(state.ops[j].dst == 0xFFU) ? LXS_FUNCTIONAL_REGION_DST_OUTPUT : LXS_FUNCTIONAL_REGION_DST_TEMP;
			region.ops[j].dst[0] = (state.ops[j].dst == 0xFFU) ? 0U : state.ops[j].dst;
			region.ops[j].arity = state.ops[j].arity;
			for (uint32_t src_index = 0; src_index < state.ops[j].arity; ++src_index)
				{
				region.ops[j].src[src_index] = state.ops[j].src_is_temp[src_index] ?
					(uint8_t)(state.boundary_count + state.ops[j].src_index[src_index]) :
					state.ops[j].src_index[src_index];
				}
			}

		for (uint32_t j = 0; j < state.gate_count; ++j)
			{
			matched_gates[state.gate_indices[j]] = 1U;
			}

		lxs_record_recognition(
			family_match_count,
			family_node_reduction,
			LXS_RECOGNITION_FAMILY_FUNCTIONAL,
			matched_gate_count);
		if (regions)
			{
			regions[region_count] = region;
			}
		region_count++;
		}
	lxs_record_recognition_time(
		family_time_us,
		LXS_RECOGNITION_FAMILY_FUNCTIONAL,
		family_start,
		clock());
	}

	return region_count;
	}

static int lxs_try_describe_full_adder_cinv(
	const lxs_netlist *nl,
	const int32_t *comb_driver,
	const uint32_t *net_use_count,
	const uint8_t *matched_gates,
	uint32_t gate_index,
	uint32_t xor_index,
	uint32_t cin_n,
	lxs_fa_cinv_desc *desc)
	{
	const lxs_gate_ir *sum_gate;
	const lxs_gate_ir *xor_gate;
	const lxs_gate_ir *carry_gate;
	const lxs_gate_ir *inner_nor;
	const lxs_gate_ir *and_gate;
	const lxs_gate_ir *nor_ab_gate;
	int32_t carry_index;
	int32_t inner_index;
	int32_t and_index;
	int32_t nor_ab_index;
	uint32_t a;
	uint32_t b;
	uint32_t inner_output;
	uint32_t level;

	sum_gate = &nl->gates[gate_index];
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
		and_gate->input_count != 2U || nor_ab_gate->input_count != 2U ||
		net_use_count[and_gate->output] != 2U || net_use_count[nor_ab_gate->output] != 2U)
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

	carry_index = lxs_find_unique_consumer(nl, and_gate->output, LXS_GATE_NOR, (int32_t)xor_index);
	if (carry_index < 0 || matched_gates[carry_index])
		{
		return 0;
		}

	carry_gate = &nl->gates[carry_index];
	if (carry_gate->input_count != 2U || !lxs_gate_uses_input(carry_gate, and_gate->output))
		{
		return 0;
		}

	inner_index = lxs_find_unique_consumer(nl, nor_ab_gate->output, LXS_GATE_NOR, (int32_t)xor_index);
	if (inner_index < 0 || matched_gates[inner_index])
		{
		return 0;
		}

	inner_nor = &nl->gates[inner_index];
	inner_output = inner_nor->output;
	if (inner_nor->input_count != 2U || net_use_count[inner_nor->output] != 1U ||
		!lxs_gate_uses_input(inner_nor, cin_n) ||
		!lxs_gate_uses_input(inner_nor, nor_ab_gate->output) ||
		!lxs_gate_uses_input(carry_gate, inner_output))
		{
		return 0;
		}

	level = sum_gate->level;
	if (carry_gate->level > level)
		{
		level = carry_gate->level;
		}

	memset(desc, 0, sizeof(*desc));
	desc->a = a;
	desc->b = b;
	desc->cin_n = cin_n;
	desc->sum_out = sum_gate->output;
	desc->carry_out = carry_gate->output;
	desc->level = level;
	desc->gate_indices[0] = gate_index;
	desc->gate_indices[1] = (uint32_t)xor_index;
	desc->gate_indices[2] = (uint32_t)and_index;
	desc->gate_indices[3] = (uint32_t)nor_ab_index;
	desc->gate_indices[4] = (uint32_t)inner_index;
	desc->gate_indices[5] = (uint32_t)carry_index;
	return 1;
	}

static int lxs_describe_full_adder_cinv(
	const lxs_netlist *nl,
	const int32_t *comb_driver,
	const uint32_t *net_use_count,
	const uint8_t *matched_gates,
	uint32_t gate_index,
	lxs_fa_cinv_desc *desc)
	{
	const lxs_gate_ir *sum_gate;
	int32_t xor_index;

	sum_gate = &nl->gates[gate_index];
	if (sum_gate->type != LXS_GATE_XNOR || sum_gate->input_count != 2U || matched_gates[gate_index])
		{
		return 0;
		}

	xor_index = comb_driver[sum_gate->inputs[0]];
	if (xor_index >= 0 && nl->gates[xor_index].type == LXS_GATE_NOR &&
		lxs_try_describe_full_adder_cinv(
			nl,
			comb_driver,
			net_use_count,
			matched_gates,
			gate_index,
			(uint32_t)xor_index,
			sum_gate->inputs[1],
			desc))
		{
		return 1;
		}

	xor_index = comb_driver[sum_gate->inputs[1]];
	if (xor_index >= 0 && nl->gates[xor_index].type == LXS_GATE_NOR &&
		lxs_try_describe_full_adder_cinv(
			nl,
			comb_driver,
			net_use_count,
			matched_gates,
			gate_index,
			(uint32_t)xor_index,
			sum_gate->inputs[0],
			desc))
		{
		return 1;
		}

	return 0;
	}

static uint32_t lxs_collect_source_multi_macros(
	const lxs_netlist *nl,
	const int32_t *comb_driver,
	uint8_t *matched_gates,
	lxs_multi_macro_plan *macros)
	{
	for (uint32_t i = 0; i < nl->source_multi_macro_count; ++i)
		{
		const lxs_source_multi_macro *source = &nl->source_multi_macros[i];
		lxs_multi_macro_plan macro;
		uint32_t level = 0U;

		memset(&macro, 0, sizeof(macro));
		if (source->type == LXS_SOURCE_MULTI_MACRO_HALF_ADDER)
			{
			macro.type = LXS_MULTI_MACRO_HALF_ADDER;
			macro.gate_equiv_count = 2U;
			}
		else if (source->type == LXS_SOURCE_MULTI_MACRO_FULL_ADDER)
			{
			macro.type = LXS_MULTI_MACRO_FULL_ADDER;
			macro.gate_equiv_count = 5U;
			}
		else if (source->type == LXS_SOURCE_MULTI_MACRO_XOR_FAN8)
			{
			macro.type = LXS_MULTI_MACRO_XOR_FAN8;
			macro.gate_equiv_count = 8U;
			}
		else if (source->type == LXS_SOURCE_MULTI_MACRO_RIPPLE_ADD4)
			{
			macro.type = LXS_MULTI_MACRO_RIPPLE_ADD4;
			macro.gate_equiv_count = 20U;
			}
		else if (source->type == LXS_SOURCE_MULTI_MACRO_CARRY_SAVE_ROW4)
			{
			macro.type = LXS_MULTI_MACRO_CARRY_SAVE_ROW4;
			macro.gate_equiv_count = 20U;
			}
		else if (source->type == LXS_SOURCE_MULTI_MACRO_REDUCE_PROPAGATE4)
			{
			macro.type = LXS_MULTI_MACRO_REDUCE_PROPAGATE4;
			macro.gate_equiv_count = 35U;
			}
		else if (source->type == LXS_SOURCE_MULTI_MACRO_AND_FAN8)
			{
			macro.type = LXS_MULTI_MACRO_AND_FAN8;
			macro.gate_equiv_count = 8U;
			}
		else if (source->type == LXS_SOURCE_MULTI_MACRO_XNOR_BANK4)
			{
			macro.type = LXS_MULTI_MACRO_XNOR_BANK4;
			macro.gate_equiv_count = 8U;
			}
		else if (source->type == LXS_SOURCE_MULTI_MACRO_GUARD_CHAIN4)
			{
			macro.type = LXS_MULTI_MACRO_GUARD_CHAIN4;
			macro.gate_equiv_count = 8U;
			}
		else
			{
			macro.type = LXS_MULTI_MACRO_RIPPLE_SLICE2;
			macro.gate_equiv_count = 10U;
			}

		for (uint32_t j = 0; j < source->gate_count; ++j)
			{
			uint32_t gate_index = source->gate_indices[j];
			matched_gates[gate_index] = 1U;
			}

		if (lxs_source_multi_macro_use_unit_level(source->type))
			{
			level = lxs_compute_source_multi_macro_unit_level(nl, comb_driver, source);
			}
		else
			{
			for (uint32_t j = 0; j < source->gate_count; ++j)
				{
				uint32_t gate_index = source->gate_indices[j];
				if (nl->gates[gate_index].level > level)
					{
					level = nl->gates[gate_index].level;
					}
				}
			}

		macro.level = level;
		macro.input_count = source->input_count;
		macro.output_count = source->output_count;
		for (uint32_t j = 0; j < source->input_count; ++j)
			{
			macro.inputs[j] = source->inputs[j];
			}
		for (uint32_t j = 0; j < source->output_count; ++j)
			{
			macro.outputs[j] = source->outputs[j];
			}

		if (macros)
			{
			macros[i] = macro;
			}
		}

	return nl->source_multi_macro_count;
	}

static int lxs_try_match_xnor_bank4_local(
	const lxs_netlist *nl,
	const int32_t *comb_driver,
	const uint32_t *gate_use_count,
	const uint8_t *matched_gates,
	uint32_t root_index,
	uint64_t *family_candidate_roots,
	uint64_t *family_nodes_visited,
	uint32_t *family_max_depth_reached,
	uint64_t family_abort_count[LXS_RECOGNITION_FAMILY_COUNT][LXS_RECOGNITION_ABORT_REASON_COUNT],
	uint32_t *matched_gate_indices,
	uint32_t *matched_gate_count,
	lxs_multi_macro_plan *macro)
	{
	uint32_t input_ids[8];
	uint32_t output_ids[4];
	uint32_t not_indices[4];
	uint32_t unique_xor_indices[4];
	uint8_t unique_xor_absorb[4];
	uint32_t unique_xor_count = 0U;
	uint32_t found = 0U;
	uint32_t level = 0U;

	if (root_index >= nl->gate_count)
		{
		lxs_record_recognition_abort(
			family_abort_count,
			LXS_RECOGNITION_FAMILY_COMPARE,
			LXS_RECOGNITION_ABORT_SHAPE);
		return 0;
		}

	if (matched_gates[root_index] ||
		nl->gates[root_index].type != LXS_GATE_NOT ||
		nl->gates[root_index].input_count != 1U)
		{
		return 0;
		}

	lxs_record_recognition_candidate(family_candidate_roots, LXS_RECOGNITION_FAMILY_COMPARE);

	for (uint32_t gate_index = root_index;
		gate_index < nl->gate_count && gate_index < root_index + 8U && found < 4U;
		++gate_index)
		{
		const lxs_gate_ir *not_gate = &nl->gates[gate_index];
		int32_t xor_index;
		const lxs_gate_ir *xor_gate;

		lxs_record_recognition_visit(
			family_nodes_visited,
			family_max_depth_reached,
			LXS_RECOGNITION_FAMILY_COMPARE,
			1U);

		if (matched_gates[gate_index] ||
			not_gate->type != LXS_GATE_NOT ||
			not_gate->input_count != 1U)
			{
			continue;
			}

		xor_index = comb_driver[not_gate->inputs[0]];
		if (xor_index < 0)
			{
			continue;
			}

		xor_gate = &nl->gates[(uint32_t)xor_index];
		lxs_record_recognition_visit(
			family_nodes_visited,
			family_max_depth_reached,
			LXS_RECOGNITION_FAMILY_COMPARE,
			2U);

		if (matched_gates[(uint32_t)xor_index])
			{
			continue;
			}

		if (xor_gate->type != LXS_GATE_XOR || xor_gate->input_count != 2U)
			{
			continue;
			}

		not_indices[found] = gate_index;
		input_ids[found * 2U] = xor_gate->inputs[0];
		input_ids[found * 2U + 1U] = xor_gate->inputs[1];
		output_ids[found] = not_gate->output;
		if (not_gate->level > level)
			{
			level = not_gate->level;
			}
		if (xor_gate->level > level)
			{
			level = xor_gate->level;
			}

		{
		uint32_t seen = 0U;
		for (uint32_t j = 0; j < unique_xor_count; ++j)
			{
			if (unique_xor_indices[j] == (uint32_t)xor_index)
				{
				seen = 1U;
				break;
				}
			}
		if (!seen)
			{
			unique_xor_indices[unique_xor_count++] = (uint32_t)xor_index;
			}
		}
		found++;
		}

	if (found != 4U)
		{
		lxs_record_recognition_abort(
			family_abort_count,
			LXS_RECOGNITION_FAMILY_COMPARE,
			LXS_RECOGNITION_ABORT_SHAPE);
		return 0;
		}

	for (uint32_t i = 0; i < unique_xor_count; ++i)
		{
		uint32_t total_consumers = 0U;
		uint32_t local_consumers = 0U;
		uint32_t xor_output = nl->gates[unique_xor_indices[i]].output;

		for (uint32_t j = 0; j < 4U; ++j)
			{
			if (nl->gates[not_indices[j]].inputs[0] == xor_output)
				{
				local_consumers++;
				}
			}

		for (uint32_t gate_index = 0; gate_index < nl->gate_count; ++gate_index)
			{
			const lxs_gate_ir *consumer = &nl->gates[gate_index];
			uint32_t consumes_output = 0U;

			for (uint32_t input_index = 0; input_index < consumer->input_count; ++input_index)
				{
				if (consumer->inputs[input_index] == xor_output)
					{
					consumes_output = 1U;
					break;
					}
				}

			if (!consumes_output)
				{
				continue;
				}

			total_consumers++;
			if (consumer->type != LXS_GATE_NOT)
				{
				if (!(consumer->type == LXS_GATE_AND || consumer->type == LXS_GATE_OR))
					{
					lxs_record_recognition_abort(
						family_abort_count,
						LXS_RECOGNITION_FAMILY_COMPARE,
						LXS_RECOGNITION_ABORT_FANOUT);
					return 0;
					}
				}
			}

		if (total_consumers != gate_use_count[xor_output])
			{
			lxs_record_recognition_abort(
				family_abort_count,
				LXS_RECOGNITION_FAMILY_COMPARE,
				LXS_RECOGNITION_ABORT_BRANCH);
			return 0;
			}

		unique_xor_absorb[i] = total_consumers == local_consumers ? 1U : 0U;
		}

	memset(macro, 0, sizeof(*macro));
	macro->type = LXS_MULTI_MACRO_XNOR_BANK4;
	macro->level = level;
	for (uint32_t i = 0; i < 8U; ++i)
		{
		macro->inputs[i] = input_ids[i];
		}
	for (uint32_t i = 0; i < 4U; ++i)
		{
		macro->outputs[i] = output_ids[i];
		}
	macro->input_count = 8U;
	macro->output_count = 4U;
	macro->gate_equiv_count = 4U;
	if (matched_gate_count)
		{
		*matched_gate_count = 0U;
		}
	for (uint32_t i = 0; i < 4U; ++i)
		{
		if (matched_gate_indices && matched_gate_count)
			{
			matched_gate_indices[*matched_gate_count] = not_indices[i];
			(*matched_gate_count)++;
			}
		}
	for (uint32_t i = 0; i < unique_xor_count; ++i)
		{
		if (unique_xor_absorb[i])
			{
			macro->gate_equiv_count++;
			if (matched_gate_indices && matched_gate_count)
				{
				matched_gate_indices[*matched_gate_count] = unique_xor_indices[i];
				(*matched_gate_count)++;
				}
			}
		}
	return 1;
	}

static int lxs_try_match_compare_and4_local(
	const lxs_netlist *nl,
	const int32_t *comb_driver,
	const uint32_t *gate_use_count,
	const uint8_t *matched_gates,
	uint32_t root_index,
	uint64_t *family_candidate_roots,
	uint64_t *family_nodes_visited,
	uint32_t *family_max_depth_reached,
	uint64_t family_abort_count[LXS_RECOGNITION_FAMILY_COUNT][LXS_RECOGNITION_ABORT_REASON_COUNT],
	uint32_t *matched_gate_indices,
	uint32_t *matched_gate_count,
	lxs_multi_macro_plan *macro)
	{
	const lxs_gate_ir *root_gate;
	uint32_t source_ids[4];
	uint32_t invert_mask = 0U;
	uint32_t invert_count = 0U;

	if (root_index >= nl->gate_count)
		{
		lxs_record_recognition_abort(
			family_abort_count,
			LXS_RECOGNITION_FAMILY_COMPARE,
			LXS_RECOGNITION_ABORT_SHAPE);
		return 0;
		}

	root_gate = &nl->gates[root_index];
	if (matched_gates[root_index] ||
		root_gate->type != LXS_GATE_AND ||
		root_gate->input_count != 4U)
		{
		return 0;
		}

	lxs_record_recognition_candidate(family_candidate_roots, LXS_RECOGNITION_FAMILY_COMPARE);
	lxs_record_recognition_visit(
		family_nodes_visited,
		family_max_depth_reached,
		LXS_RECOGNITION_FAMILY_COMPARE,
		1U);

	if (matched_gate_count)
		{
		*matched_gate_count = 0U;
		}

	for (uint32_t input_index = 0; input_index < 4U; ++input_index)
		{
		uint32_t source_net = root_gate->inputs[input_index];
		int32_t source_gate_index;

		{
		int32_t not_index = comb_driver[source_net];
		if (not_index >= 0)
			{
			const lxs_gate_ir *not_gate = &nl->gates[(uint32_t)not_index];

			lxs_record_recognition_visit(
				family_nodes_visited,
				family_max_depth_reached,
				LXS_RECOGNITION_FAMILY_COMPARE,
				2U);

			if (not_gate->type == LXS_GATE_NOT &&
				not_gate->input_count == 1U &&
				gate_use_count[source_net] == 1U)
				{
				source_net = not_gate->inputs[0];
				invert_mask |= (1U << input_index);
				invert_count++;
				if (matched_gate_indices && matched_gate_count)
					{
					matched_gate_indices[*matched_gate_count] = (uint32_t)not_index;
					(*matched_gate_count)++;
					}
				}
			}
		}

		source_gate_index = comb_driver[source_net];
		if (source_gate_index < 0)
			{
			lxs_record_recognition_abort(
				family_abort_count,
				LXS_RECOGNITION_FAMILY_COMPARE,
				LXS_RECOGNITION_ABORT_SHAPE);
			if (matched_gate_count)
				{
				*matched_gate_count = 0U;
				}
			return 0;
			}

		lxs_record_recognition_visit(
			family_nodes_visited,
			family_max_depth_reached,
			LXS_RECOGNITION_FAMILY_COMPARE,
			3U);

		if (nl->gates[(uint32_t)source_gate_index].type != LXS_GATE_XOR ||
			nl->gates[(uint32_t)source_gate_index].input_count != 2U)
			{
			lxs_record_recognition_abort(
				family_abort_count,
				LXS_RECOGNITION_FAMILY_COMPARE,
				LXS_RECOGNITION_ABORT_SHAPE);
			if (matched_gate_count)
				{
				*matched_gate_count = 0U;
				}
			return 0;
			}

		source_ids[input_index] = source_net;
		}

	memset(macro, 0, sizeof(*macro));
	macro->type = LXS_MULTI_MACRO_COMPARE_AND4;
	macro->level = root_gate->level;
	macro->input_count = 4U;
	macro->output_count = 1U;
	macro->gate_equiv_count = 3U + invert_count;
	macro->param0 = invert_mask;
	for (uint32_t i = 0; i < 4U; ++i)
		{
		macro->inputs[i] = source_ids[i];
		}
	macro->outputs[0] = root_gate->output;

	if (matched_gate_indices && matched_gate_count)
		{
		matched_gate_indices[*matched_gate_count] = root_index;
		(*matched_gate_count)++;
		}

	return 1;
	}

static int lxs_try_match_compare_or4_local(
	const lxs_netlist *nl,
	const int32_t *comb_driver,
	const uint32_t *gate_use_count,
	const uint8_t *matched_gates,
	uint32_t root_index,
	uint64_t *family_candidate_roots,
	uint64_t *family_nodes_visited,
	uint32_t *family_max_depth_reached,
	uint64_t family_abort_count[LXS_RECOGNITION_FAMILY_COUNT][LXS_RECOGNITION_ABORT_REASON_COUNT],
	uint32_t *matched_gate_indices,
	uint32_t *matched_gate_count,
	lxs_multi_macro_plan *macro)
	{
	const lxs_gate_ir *root_gate;
	uint32_t base_ids[4];
	uint32_t term_masks[4];
	uint32_t invert_count = 0U;
	uint32_t and_indices[4];
	uint32_t unique_not_indices[16];
	uint32_t unique_not_count = 0U;

	if (root_index >= nl->gate_count)
		{
		lxs_record_recognition_abort(
			family_abort_count,
			LXS_RECOGNITION_FAMILY_COMPARE,
			LXS_RECOGNITION_ABORT_SHAPE);
		return 0;
		}

	root_gate = &nl->gates[root_index];
	if (matched_gates[root_index] ||
		root_gate->type != LXS_GATE_OR ||
		root_gate->input_count != 4U)
		{
		return 0;
		}

	lxs_record_recognition_candidate(family_candidate_roots, LXS_RECOGNITION_FAMILY_COMPARE);
	lxs_record_recognition_visit(
		family_nodes_visited,
		family_max_depth_reached,
		LXS_RECOGNITION_FAMILY_COMPARE,
		1U);

	if (matched_gate_count)
		{
		*matched_gate_count = 0U;
		}

	for (uint32_t term_index = 0; term_index < 4U; ++term_index)
		{
		uint32_t and_net = root_gate->inputs[term_index];
		int32_t and_index = comb_driver[and_net];
		const lxs_gate_ir *and_gate;
		uint32_t local_mask = 0U;
		uint32_t seen_mask = 0U;

		if (and_index < 0 || gate_use_count[and_net] != 1U)
			{
			lxs_record_recognition_abort(
				family_abort_count,
				LXS_RECOGNITION_FAMILY_COMPARE,
				LXS_RECOGNITION_ABORT_BRANCH);
			if (matched_gate_count)
				{
				*matched_gate_count = 0U;
				}
			return 0;
			}

		and_gate = &nl->gates[(uint32_t)and_index];
		and_indices[term_index] = (uint32_t)and_index;
		lxs_record_recognition_visit(
			family_nodes_visited,
			family_max_depth_reached,
			LXS_RECOGNITION_FAMILY_COMPARE,
			2U);

		if (matched_gates[(uint32_t)and_index] ||
			and_gate->type != LXS_GATE_AND ||
			and_gate->input_count != 4U)
			{
			lxs_record_recognition_abort(
				family_abort_count,
				LXS_RECOGNITION_FAMILY_COMPARE,
				LXS_RECOGNITION_ABORT_SHAPE);
			if (matched_gate_count)
				{
				*matched_gate_count = 0U;
				}
			return 0;
			}

		if (matched_gate_indices && matched_gate_count)
			{
			matched_gate_indices[*matched_gate_count] = (uint32_t)and_index;
			(*matched_gate_count)++;
			}

		for (uint32_t input_index = 0; input_index < 4U; ++input_index)
			{
			uint32_t source_net = and_gate->inputs[input_index];
			int32_t not_index = comb_driver[source_net];
			int32_t source_gate_index;
			uint32_t base_position = UINT32_MAX;

			if (not_index >= 0)
				{
				const lxs_gate_ir *not_gate = &nl->gates[(uint32_t)not_index];

				lxs_record_recognition_visit(
					family_nodes_visited,
					family_max_depth_reached,
					LXS_RECOGNITION_FAMILY_COMPARE,
					3U);

				if (not_gate->type == LXS_GATE_NOT &&
					not_gate->input_count == 1U)
					{
					source_net = not_gate->inputs[0];
					{
					uint32_t seen_not = 0U;
					for (uint32_t k = 0; k < unique_not_count; ++k)
						{
						if (unique_not_indices[k] == (uint32_t)not_index)
							{
							seen_not = 1U;
							break;
							}
						}
					if (!seen_not)
						{
						unique_not_indices[unique_not_count++] = (uint32_t)not_index;
						invert_count++;
						}
					}
					}
				}

			source_gate_index = comb_driver[source_net];
			if (source_gate_index < 0 ||
				nl->gates[(uint32_t)source_gate_index].type != LXS_GATE_XOR ||
				nl->gates[(uint32_t)source_gate_index].input_count != 2U)
				{
				lxs_record_recognition_abort(
					family_abort_count,
					LXS_RECOGNITION_FAMILY_COMPARE,
					LXS_RECOGNITION_ABORT_SHAPE);
				if (matched_gate_count)
					{
					*matched_gate_count = 0U;
					}
				return 0;
				}

			lxs_record_recognition_visit(
				family_nodes_visited,
				family_max_depth_reached,
				LXS_RECOGNITION_FAMILY_COMPARE,
				4U);

			if (term_index == 0U)
				{
				base_ids[input_index] = source_net;
				base_position = input_index;
				}
			else
				{
				for (uint32_t base_index = 0; base_index < 4U; ++base_index)
					{
					if (base_ids[base_index] == source_net)
						{
						base_position = base_index;
						break;
						}
					}
				}

			if (base_position == UINT32_MAX || ((seen_mask >> base_position) & 1U) != 0U)
				{
				lxs_record_recognition_abort(
					family_abort_count,
					LXS_RECOGNITION_FAMILY_COMPARE,
					LXS_RECOGNITION_ABORT_OVERLAP);
				if (matched_gate_count)
					{
					*matched_gate_count = 0U;
					}
				return 0;
				}

			seen_mask |= (1U << base_position);
			if (source_net != and_gate->inputs[input_index])
				{
				local_mask |= (1U << base_position);
				}
			}

		term_masks[term_index] = local_mask;
		}

	for (uint32_t i = 0; i < unique_not_count; ++i)
		{
		uint32_t not_output = nl->gates[unique_not_indices[i]].output;

		for (uint32_t gate_index = 0; gate_index < nl->gate_count; ++gate_index)
			{
			const lxs_gate_ir *consumer = &nl->gates[gate_index];
			uint32_t consumes_output = 0U;
			uint32_t local_consumer = 0U;

			for (uint32_t input_index = 0; input_index < consumer->input_count; ++input_index)
				{
				if (consumer->inputs[input_index] == not_output)
					{
					consumes_output = 1U;
					break;
					}
				}

			if (!consumes_output)
				{
				continue;
				}

			for (uint32_t term_index = 0; term_index < 4U; ++term_index)
				{
				if (and_indices[term_index] == gate_index)
					{
					local_consumer = 1U;
					break;
					}
				}

			if (!local_consumer)
				{
				lxs_record_recognition_abort(
					family_abort_count,
					LXS_RECOGNITION_FAMILY_COMPARE,
					LXS_RECOGNITION_ABORT_BRANCH);
				if (matched_gate_count)
					{
					*matched_gate_count = 0U;
					}
				return 0;
				}
			}
		}

	memset(macro, 0, sizeof(*macro));
	macro->type = LXS_MULTI_MACRO_COMPARE_OR4;
	macro->level = root_gate->level;
	macro->input_count = 4U;
	macro->output_count = 1U;
	macro->gate_equiv_count = 15U + invert_count;
	macro->param0 =
		(term_masks[0] & 0xFU) |
		((term_masks[1] & 0xFU) << 4U) |
		((term_masks[2] & 0xFU) << 8U) |
		((term_masks[3] & 0xFU) << 12U);
	for (uint32_t i = 0; i < 4U; ++i)
		{
		macro->inputs[i] = base_ids[i];
		}
	macro->outputs[0] = root_gate->output;

	if (matched_gate_indices && matched_gate_count)
		{
		for (uint32_t i = 0; i < unique_not_count; ++i)
			{
			matched_gate_indices[*matched_gate_count] = unique_not_indices[i];
			(*matched_gate_count)++;
			}
		matched_gate_indices[*matched_gate_count] = root_index;
		(*matched_gate_count)++;
		}

	return 1;
	}

static int lxs_try_match_compare_decode4_local(
	const lxs_netlist *nl,
	const int32_t *comb_driver,
	const uint8_t *matched_gates,
	uint32_t root_index,
	uint64_t *family_candidate_roots,
	uint64_t *family_nodes_visited,
	uint32_t *family_max_depth_reached,
	uint64_t family_abort_count[LXS_RECOGNITION_FAMILY_COUNT][LXS_RECOGNITION_ABORT_REASON_COUNT],
	uint32_t *matched_gate_count,
	uint32_t *gate_equiv_count)
	{
	const lxs_gate_ir *root_gate;
	uint32_t compare_literals = 0U;

	if (root_index >= nl->gate_count)
		{
		lxs_record_recognition_abort(
			family_abort_count,
			LXS_RECOGNITION_FAMILY_COMPARE,
			LXS_RECOGNITION_ABORT_SHAPE);
		return 0;
		}

	root_gate = &nl->gates[root_index];
	if (matched_gates[root_index] ||
		(root_gate->type != LXS_GATE_AND && root_gate->type != LXS_GATE_OR) ||
		root_gate->input_count < 4U ||
		root_gate->input_count > 8U)
		{
		return 0;
		}

	lxs_record_recognition_candidate(family_candidate_roots, LXS_RECOGNITION_FAMILY_COMPARE);
	lxs_record_recognition_visit(
		family_nodes_visited,
		family_max_depth_reached,
		LXS_RECOGNITION_FAMILY_COMPARE,
		1U);

	for (uint32_t input_index = 0; input_index < root_gate->input_count; ++input_index)
		{
		int32_t driver_index = comb_driver[root_gate->inputs[input_index]];

		if (driver_index < 0)
			{
			continue;
			}

		if ((uint32_t)driver_index >= nl->gate_count)
			{
			lxs_record_recognition_abort(
				family_abort_count,
				LXS_RECOGNITION_FAMILY_COMPARE,
				LXS_RECOGNITION_ABORT_NODE_BUDGET);
			return 0;
			}

		{
		const lxs_gate_ir *driver_gate = &nl->gates[(uint32_t)driver_index];

		lxs_record_recognition_visit(
			family_nodes_visited,
			family_max_depth_reached,
			LXS_RECOGNITION_FAMILY_COMPARE,
			2U);

		if (driver_gate->type == LXS_GATE_XOR && driver_gate->input_count == 2U)
			{
			compare_literals++;
			continue;
			}

		if (driver_gate->type == LXS_GATE_NOT && driver_gate->input_count == 1U)
			{
			int32_t xor_index = comb_driver[driver_gate->inputs[0]];
			if (xor_index < 0)
				{
				continue;
				}
			if ((uint32_t)xor_index >= nl->gate_count)
				{
				lxs_record_recognition_abort(
					family_abort_count,
					LXS_RECOGNITION_FAMILY_COMPARE,
					LXS_RECOGNITION_ABORT_NODE_BUDGET);
				return 0;
				}
			lxs_record_recognition_visit(
				family_nodes_visited,
				family_max_depth_reached,
				LXS_RECOGNITION_FAMILY_COMPARE,
				3U);
			if (nl->gates[(uint32_t)xor_index].type == LXS_GATE_XOR &&
				nl->gates[(uint32_t)xor_index].input_count == 2U)
				{
				compare_literals++;
				}
			}
		}
		}

	if (compare_literals < 4U)
		{
		lxs_record_recognition_abort(
			family_abort_count,
			LXS_RECOGNITION_FAMILY_COMPARE,
			LXS_RECOGNITION_ABORT_SHAPE);
		return 0;
		}

	if (matched_gate_count)
		{
		*matched_gate_count = compare_literals;
		}
	if (gate_equiv_count)
		{
		*gate_equiv_count = compare_literals;
		}
	return 1;
	}

static uint32_t lxs_collect_multi_macros(
	const lxs_netlist *nl,
	const int32_t *comb_driver,
	uint8_t *matched_gates,
	uint32_t recognition_mask,
	uint32_t recognition_mode,
	uint32_t *family_match_count,
	uint32_t *family_node_reduction,
	uint64_t *family_candidate_roots,
	uint64_t *family_nodes_visited,
	uint32_t *family_max_depth_reached,
	uint64_t family_abort_count[LXS_RECOGNITION_FAMILY_COUNT][LXS_RECOGNITION_ABORT_REASON_COUNT],
	uint64_t *family_time_us,
	lxs_multi_macro_plan *macros)
	{
	uint32_t *net_use_count;
	uint32_t *gate_use_count;
	uint32_t macro_count = 0U;

	net_use_count = lxs_calloc_aligned(nl->net_count, sizeof(uint32_t));
	gate_use_count = lxs_calloc_aligned(nl->net_count, sizeof(uint32_t));
	if (!net_use_count || !gate_use_count)
		{
		lxs_free_aligned(net_use_count);
		lxs_free_aligned(gate_use_count);
		return UINT32_MAX;
		}

	for (uint32_t i = 0; i < nl->gate_count; ++i)
		{
		for (uint32_t j = 0; j < nl->gates[i].input_count; ++j)
			{
			net_use_count[nl->gates[i].inputs[j]]++;
			gate_use_count[nl->gates[i].inputs[j]]++;
			}
		}

	for (uint32_t i = 0; i < nl->output_count; ++i)
		{
		net_use_count[nl->outputs[i]]++;
		}

	if (lxs_recognition_family_enabled(recognition_mask, LXS_RECOGNITION_FAMILY_SHARED_XOR))
		{
		for (uint32_t i = 0; i < nl->gate_count; ++i)
			{
			const lxs_gate_ir *seed = &nl->gates[i];
			for (uint32_t shared_slot = 0; shared_slot < 2U; ++shared_slot)
				{
				uint32_t shared;
				uint32_t output_ids[8];
				uint32_t leaf_ids[8];
				uint32_t level = 0U;
				uint32_t found = 0U;
				int blocked = 0;
				lxs_multi_macro_plan macro;

				if (seed->type != LXS_GATE_XOR || seed->input_count != 2U || matched_gates[i])
					{
					continue;
					}

				shared = seed->inputs[shared_slot];
				for (uint32_t j = 0; j < i; ++j)
					{
					const lxs_gate_ir *prior = &nl->gates[j];
					if (matched_gates[j] || prior->type != LXS_GATE_XOR || prior->input_count != 2U)
						{
						continue;
						}
					if (prior->inputs[0] == shared || prior->inputs[1] == shared)
						{
						blocked = 1;
						break;
						}
					}
				if (blocked)
					{
					continue;
					}

				for (uint32_t j = i; j < nl->gate_count && found < 8U; ++j)
					{
					const lxs_gate_ir *gate = &nl->gates[j];
					uint32_t leaf;
					if (matched_gates[j] || gate->type != LXS_GATE_XOR || gate->input_count != 2U)
						{
						continue;
						}
					if (gate->inputs[0] == shared)
						{
						leaf = gate->inputs[1];
						}
					else if (gate->inputs[1] == shared)
						{
						leaf = gate->inputs[0];
						}
					else
						{
						continue;
						}
					if (gate_use_count[gate->output] > 0U)
						{
						continue;
						}

					output_ids[found] = gate->output;
					leaf_ids[found] = leaf;
					if (gate->level > level)
						{
						level = gate->level;
						}
					found++;
					}

				if (found != 8U)
					{
					continue;
					}

				for (uint32_t j = i, captured = 0U; j < nl->gate_count && captured < 8U; ++j)
					{
					const lxs_gate_ir *gate = &nl->gates[j];
					if (matched_gates[j] || gate->type != LXS_GATE_XOR || gate->input_count != 2U)
						{
						continue;
						}
					if ((gate->inputs[0] == shared && gate->inputs[1] == leaf_ids[captured] && gate->output == output_ids[captured]) ||
						(gate->inputs[1] == shared && gate->inputs[0] == leaf_ids[captured] && gate->output == output_ids[captured]))
						{
						matched_gates[j] = 1U;
						captured++;
						}
					}

				memset(&macro, 0, sizeof(macro));
				macro.type = LXS_MULTI_MACRO_XOR_FAN8;
				macro.level = level;
				macro.inputs[0] = shared;
				for (uint32_t j = 0; j < 8U; ++j)
					{
					macro.inputs[j + 1U] = leaf_ids[j];
					macro.outputs[j] = output_ids[j];
					}
				macro.input_count = 9U;
				macro.output_count = 8U;
				macro.gate_equiv_count = 8U;

				if (macros)
					{
					macros[macro_count] = macro;
					}
				lxs_record_recognition(
					family_match_count,
					family_node_reduction,
					LXS_RECOGNITION_FAMILY_SHARED_XOR,
					8U);
				macro_count++;
				break;
				}
			}
		}

	if (lxs_recognition_family_enabled(recognition_mask, LXS_RECOGNITION_FAMILY_COMPARE))
		{
		clock_t family_start = clock();
		if (recognition_mode == LXS_RECOGNITION_MODE_REPLACE)
			{
			for (uint32_t i = 0; i < nl->gate_count; ++i)
				{
				lxs_multi_macro_plan macro;
				uint32_t matched_gate_indices[32];
				uint32_t matched_gate_count = 0U;

				if (!lxs_try_match_compare_or4_local(
						nl,
						comb_driver,
						gate_use_count,
						matched_gates,
						i,
						family_candidate_roots,
						family_nodes_visited,
						family_max_depth_reached,
						family_abort_count,
						matched_gate_indices,
						&matched_gate_count,
						&macro))
					{
					continue;
					}

				for (uint32_t j = 0; j < matched_gate_count; ++j)
					{
					matched_gates[matched_gate_indices[j]] = 1U;
					}

				if (macros)
					{
					macros[macro_count] = macro;
					}
				lxs_record_recognition(
					family_match_count,
					family_node_reduction,
					LXS_RECOGNITION_FAMILY_COMPARE,
					matched_gate_count);
				macro_count++;
				}

			for (uint32_t i = 0; i < nl->gate_count; ++i)
				{
				lxs_multi_macro_plan macro;
				uint32_t matched_gate_indices[32];
				uint32_t matched_gate_count = 0U;

				if (!lxs_try_match_compare_and4_local(
						nl,
						comb_driver,
						gate_use_count,
						matched_gates,
						i,
						family_candidate_roots,
						family_nodes_visited,
						family_max_depth_reached,
						family_abort_count,
						matched_gate_indices,
						&matched_gate_count,
						&macro) &&
					!lxs_try_match_xnor_bank4_local(
						nl,
						comb_driver,
						gate_use_count,
						matched_gates,
						i,
						family_candidate_roots,
						family_nodes_visited,
						family_max_depth_reached,
						family_abort_count,
						matched_gate_indices,
						&matched_gate_count,
						&macro))
					{
					continue;
					}

				for (uint32_t j = 0; j < matched_gate_count; ++j)
					{
					matched_gates[matched_gate_indices[j]] = 1U;
					}

				if (macros)
					{
					macros[macro_count] = macro;
					}
				lxs_record_recognition(
					family_match_count,
						family_node_reduction,
						LXS_RECOGNITION_FAMILY_COMPARE,
						matched_gate_count);
				macro_count++;
				}
			}
		else
			{
			for (uint32_t i = 0; i < nl->gate_count; ++i)
				{
				uint32_t matched_gate_count = 0U;
				uint32_t gate_equiv_count = 0U;

				if (!lxs_try_match_compare_decode4_local(
						nl,
						comb_driver,
						matched_gates,
						i,
						family_candidate_roots,
						family_nodes_visited,
						family_max_depth_reached,
						family_abort_count,
						&matched_gate_count,
						&gate_equiv_count))
					{
					continue;
					}

				lxs_record_recognition(
					family_match_count,
					family_node_reduction,
					LXS_RECOGNITION_FAMILY_COMPARE,
					matched_gate_count);
				if (gate_equiv_count > 0U)
					{
					macro_count++;
					}
				}
			}
		lxs_record_recognition_time(
			family_time_us,
			LXS_RECOGNITION_FAMILY_COMPARE,
			family_start,
			clock());
		}

	if (lxs_recognition_family_enabled(recognition_mask, LXS_RECOGNITION_FAMILY_ARITHMETIC))
		{
		for (uint32_t i = 0; i < nl->gate_count; ++i)
			{
			lxs_fa_cinv_desc cell0;
			lxs_fa_cinv_desc cell1;
			int32_t next_sum_index;
			lxs_multi_macro_plan macro;
			uint32_t level;

			if (!lxs_describe_full_adder_cinv(nl, comb_driver, net_use_count, matched_gates, i, &cell0))
				{
				continue;
				}

			next_sum_index = lxs_find_unique_consumer(nl, cell0.carry_out, LXS_GATE_XNOR, (int32_t)i);
			if (next_sum_index < 0 ||
				!lxs_describe_full_adder_cinv(
					nl,
					comb_driver,
					net_use_count,
					matched_gates,
					(uint32_t)next_sum_index,
					&cell1) ||
				cell1.cin_n != cell0.carry_out)
				{
				continue;
				}

			for (uint32_t j = 0; j < 6U; ++j)
				{
				matched_gates[cell0.gate_indices[j]] = 1U;
				matched_gates[cell1.gate_indices[j]] = 1U;
				}

			level = cell0.level;
			if (cell1.level > level)
				{
				level = cell1.level;
				}

			memset(&macro, 0, sizeof(macro));
			macro.type = LXS_MULTI_MACRO_RIPPLE_SLICE2_CINV;
			macro.level = level;
			macro.inputs[0] = cell0.a;
			macro.inputs[1] = cell0.b;
			macro.inputs[2] = cell1.a;
			macro.inputs[3] = cell1.b;
			macro.inputs[4] = cell0.cin_n;
			macro.outputs[0] = cell0.sum_out;
			macro.outputs[1] = cell1.sum_out;
			macro.outputs[2] = cell1.carry_out;
			macro.input_count = 5U;
			macro.output_count = 3U;
			macro.gate_equiv_count = 12U;

			if (macros)
				{
				macros[macro_count] = macro;
				}
			lxs_record_recognition(
				family_match_count,
				family_node_reduction,
				LXS_RECOGNITION_FAMILY_ARITHMETIC,
				12U);
			macro_count++;
			}
		}

	lxs_free_aligned(net_use_count);
	lxs_free_aligned(gate_use_count);
	return macro_count;
	}


static lxs_netlist* lxs_load_bench(const char *path)
	{
	FILE *stream;
	char line[1024];
	lxs_netlist *nl;
	uint32_t macro_serial = 0U;

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
		char *output_ctx = NULL;
		char *output_token;
		char *output_names[LXS_STANDARD_MACRO_MAX_OUTPUTS];
		uint32_t output_ids[LXS_STANDARD_MACRO_MAX_OUTPUTS];
		uint32_t input_ids[LXS_STANDARD_MACRO_MAX_INPUTS];
		uint32_t output_count = 0U;
		uint32_t input_count = 0U;
		uint32_t is_special_macro = 0U;
		char *gate_name;
		lxs_gate_ir gate;

		if (!open_paren)
			{
			continue;
			}

		*open_paren = '\0';
		close_paren = strrchr(open_paren + 1, ')');
		if (!close_paren)
			{
			continue;
			}

		*close_paren = '\0';
		output_token = strtok_s(out_name, ",", &output_ctx);
		while (output_token && output_count < LXS_STANDARD_MACRO_MAX_OUTPUTS)
			{
			output_names[output_count++] = lxs_trim(output_token);
			output_token = strtok_s(NULL, ",", &output_ctx);
			}

		for (uint32_t i = 0; i < output_count; ++i)
			{
			output_ids[i] = lxs_intern_net(nl, output_names[i]);
			if (output_ids[i] == UINT32_MAX)
				{
				lxs_free_netlist(nl);
				fclose(stream);
				return NULL;
				}
			}

		gate_name = lxs_trim(gate_part);

		{
		uint32_t standard_mux_kind = 0U;
		uint32_t standard_mux_width_bits = 0U;
		uint32_t standard_add_width_bits = 0U;
		uint32_t standard_cmp_width_bits = 0U;
		uint32_t standard_alu_width_bits = 0U;
		if (lxs_parse_standard_mux_name(gate_name, &standard_mux_kind, &standard_mux_width_bits))
			{
			char *input_ctx = NULL;
			char *input_token = strtok_s(open_paren + 1, ",", &input_ctx);

			while (input_token && input_count < LXS_STANDARD_MACRO_MAX_INPUTS)
				{
				input_ids[input_count] = lxs_intern_net(nl, lxs_trim(input_token));
				if (input_ids[input_count] == UINT32_MAX)
					{
					lxs_free_netlist(nl);
					fclose(stream);
					return NULL;
					}
				input_count++;
				input_token = strtok_s(NULL, ",", &input_ctx);
				}

			if (!lxs_emit_standard_mux_descriptor(
					nl,
					output_ids,
					input_ids,
					input_count,
					output_count,
					standard_mux_kind,
					standard_mux_width_bits,
					macro_serial++))
				{
				lxs_free_netlist(nl);
				fclose(stream);
				return NULL;
				}
			is_special_macro = 1U;
			}
		else if (lxs_parse_standard_add_name(gate_name, &standard_add_width_bits))
			{
			char *input_ctx = NULL;
			char *input_token = strtok_s(open_paren + 1, ",", &input_ctx);

			while (input_token && input_count < LXS_STANDARD_MACRO_MAX_INPUTS)
				{
				input_ids[input_count] = lxs_intern_net(nl, lxs_trim(input_token));
				if (input_ids[input_count] == UINT32_MAX)
					{
					lxs_free_netlist(nl);
					fclose(stream);
					return NULL;
					}
				input_count++;
				input_token = strtok_s(NULL, ",", &input_ctx);
				}

			if (!lxs_emit_standard_add_descriptor(
					nl,
					output_ids,
					input_ids,
					input_count,
					output_count,
					standard_add_width_bits,
					macro_serial++))
				{
				lxs_free_netlist(nl);
				fclose(stream);
				return NULL;
				}
			is_special_macro = 1U;
			}
		else if (lxs_parse_standard_cmp_name(gate_name, &standard_cmp_width_bits))
			{
			char *input_ctx = NULL;
			char *input_token = strtok_s(open_paren + 1, ",", &input_ctx);

			while (input_token && input_count < LXS_STANDARD_MACRO_MAX_INPUTS)
				{
				input_ids[input_count] = lxs_intern_net(nl, lxs_trim(input_token));
				if (input_ids[input_count] == UINT32_MAX)
					{
					lxs_free_netlist(nl);
					fclose(stream);
					return NULL;
					}
				input_count++;
				input_token = strtok_s(NULL, ",", &input_ctx);
				}

			if (!lxs_emit_standard_cmp_descriptor(
					nl,
					output_ids,
					input_ids,
					input_count,
					output_count,
					standard_cmp_width_bits,
					macro_serial++))
				{
				lxs_free_netlist(nl);
				fclose(stream);
				return NULL;
				}
			is_special_macro = 1U;
			}
		else if (lxs_parse_standard_alu_name(gate_name, &standard_alu_width_bits))
			{
			char *input_ctx = NULL;
			char *input_token = strtok_s(open_paren + 1, ",", &input_ctx);

			while (input_token && input_count < LXS_STANDARD_MACRO_MAX_INPUTS)
				{
				input_ids[input_count] = lxs_intern_net(nl, lxs_trim(input_token));
				if (input_ids[input_count] == UINT32_MAX)
					{
					lxs_free_netlist(nl);
					fclose(stream);
					return NULL;
					}
				input_count++;
				input_token = strtok_s(NULL, ",", &input_ctx);
				}

			if (!lxs_emit_standard_alu_descriptor(
					nl,
					output_ids,
					input_ids,
					input_count,
					output_count,
					standard_alu_width_bits,
					macro_serial++))
				{
				lxs_free_netlist(nl);
				fclose(stream);
				return NULL;
				}
			is_special_macro = 1U;
			}
		}

		uint32_t standard_register_width_bits = 0U;
		uint32_t standard_register_en_width_bits = 0U;
		uint32_t standard_register_en_rst_width_bits = 0U;
		uint32_t standard_counter_width_bits = 0U;
		uint32_t standard_counter_en_width_bits = 0U;
		uint32_t standard_rom_data_width = 0U;
		uint32_t standard_ram_data_width = 0U;
		uint32_t standard_regfile_data_width = 0U;
		uint8_t is_standard_register_en = lxs_parse_standard_register_en_name(
			gate_name,
			&standard_register_en_width_bits);
		uint8_t is_standard_register_en_rst = lxs_parse_standard_register_en_rst_name(
			gate_name,
			&standard_register_en_rst_width_bits);
		if (strcmp(gate_name, "REGISTER") == 0 ||
			lxs_parse_standard_register_name(gate_name, &standard_register_width_bits))
			{
			char *input_ctx = NULL;
			char *input_token = strtok_s(open_paren + 1, ",", &input_ctx);

			while (input_token && input_count < 64U)
				{
				input_ids[input_count] = lxs_intern_net(nl, lxs_trim(input_token));
				if (input_ids[input_count] == UINT32_MAX)
					{
					lxs_free_netlist(nl);
					fclose(stream);
					return NULL;
					}
				input_count++;
				input_token = strtok_s(NULL, ",", &input_ctx);
				}

			if (output_count == 0U || output_count != input_count ||
				(standard_register_width_bits != 0U && output_count != standard_register_width_bits) ||
				!lxs_emit_register_descriptor(nl, output_ids, input_ids, output_count, UINT32_MAX, 0U))
				{
				lxs_free_netlist(nl);
				fclose(stream);
				return NULL;
				}
			is_special_macro = 1U;
			}
		else if (strcmp(gate_name, "REGISTER_EN") == 0 ||
			strcmp(gate_name, "REGISTER_HOLD") == 0 ||
			is_standard_register_en)
			{
			char *section_ctx = NULL;
			char *sections[2];
			uint32_t section_count = 0U;
			char *section = strtok_s(open_paren + 1, ";", &section_ctx);
			char *input_ctx = NULL;
			char *input_token;
			uint32_t control_net;
			uint8_t control_invert =
				strcmp(gate_name, "REGISTER_HOLD") == 0 ? 1U : 0U;

			while (section && section_count < 2U)
				{
				sections[section_count++] = lxs_trim(section);
				section = strtok_s(NULL, ";", &section_ctx);
				}

			if (section_count != 2U)
				{
				lxs_free_netlist(nl);
				fclose(stream);
				return NULL;
				}

			input_token = strtok_s(sections[0], ",", &input_ctx);
			while (input_token && input_count < 64U)
				{
				input_ids[input_count] = lxs_intern_net(nl, lxs_trim(input_token));
				if (input_ids[input_count] == UINT32_MAX)
					{
					lxs_free_netlist(nl);
					fclose(stream);
					return NULL;
					}
				input_count++;
				input_token = strtok_s(NULL, ",", &input_ctx);
				}

			control_net = lxs_intern_net(nl, lxs_trim(sections[1]));
			if (control_net == UINT32_MAX ||
				output_count == 0U ||
				output_count != input_count ||
				(is_standard_register_en &&
					output_count != standard_register_en_width_bits) ||
				!lxs_emit_register_descriptor(
					nl,
					output_ids,
					input_ids,
					output_count,
					control_net,
					control_invert))
				{
				lxs_free_netlist(nl);
				fclose(stream);
				return NULL;
				}
			is_special_macro = 1U;
			}
		else if (strcmp(gate_name, "REGISTER_EN_RST") == 0 ||
			is_standard_register_en_rst)
			{
			char *section_ctx = NULL;
			char *sections[3];
			uint32_t section_count = 0U;
			char *section = strtok_s(open_paren + 1, ";", &section_ctx);
			char *input_ctx = NULL;
			char *input_token;
			uint32_t enable_net;
			uint32_t reset_net;

			while (section && section_count < 3U)
				{
				sections[section_count++] = lxs_trim(section);
				section = strtok_s(NULL, ";", &section_ctx);
				}

			if (section_count != 3U)
				{
				lxs_free_netlist(nl);
				fclose(stream);
				return NULL;
				}

			input_token = strtok_s(sections[0], ",", &input_ctx);
			while (input_token && input_count < 64U)
				{
				input_ids[input_count] = lxs_intern_net(nl, lxs_trim(input_token));
				if (input_ids[input_count] == UINT32_MAX)
					{
					lxs_free_netlist(nl);
					fclose(stream);
					return NULL;
					}
				input_count++;
				input_token = strtok_s(NULL, ",", &input_ctx);
				}

			enable_net = lxs_intern_net(nl, lxs_trim(sections[1]));
			reset_net = lxs_intern_net(nl, lxs_trim(sections[2]));
			if (enable_net == UINT32_MAX ||
				reset_net == UINT32_MAX ||
				output_count == 0U ||
				output_count != input_count ||
				(is_standard_register_en_rst &&
					output_count != standard_register_en_rst_width_bits) ||
				!lxs_emit_register_en_rst_descriptor(
					nl,
					output_ids,
					input_ids,
					output_count,
					enable_net,
					reset_net))
				{
				lxs_free_netlist(nl);
				fclose(stream);
				return NULL;
				}
			is_special_macro = 1U;
			}
		else if (strcmp(gate_name, "COUNTER_EN") == 0 ||
			lxs_parse_standard_counter_en_name(gate_name, &standard_counter_en_width_bits))
			{
			uint32_t enable_net = lxs_intern_net(nl, lxs_trim(open_paren + 1));

			if (enable_net == UINT32_MAX || output_count == 0U ||
				(standard_counter_en_width_bits != 0U && output_count != standard_counter_en_width_bits) ||
				!lxs_emit_counter_descriptor(nl, output_ids, output_count, enable_net, UINT32_MAX, LXS_REGISTER_MODE_COUNTER_EN))
				{
				lxs_free_netlist(nl);
				fclose(stream);
				return NULL;
				}
			is_special_macro = 1U;
			}
		else if (lxs_parse_standard_counter_name(gate_name, &standard_counter_width_bits))
			{
			if (output_count == 0U ||
				output_count != standard_counter_width_bits ||
				!lxs_emit_counter_descriptor(nl, output_ids, output_count, UINT32_MAX, UINT32_MAX, LXS_REGISTER_MODE_COUNTER_EN))
				{
				lxs_free_netlist(nl);
				fclose(stream);
				return NULL;
				}
			is_special_macro = 1U;
			}
		else if (strcmp(gate_name, "COUNTER_UPDOWN") == 0)
			{
			char *section_ctx = NULL;
			char *sections[2];
			uint32_t section_count = 0U;
			char *section = strtok_s(open_paren + 1, ";", &section_ctx);
			uint32_t enable_net;
			uint32_t down_net;

			while (section && section_count < 2U)
				{
				sections[section_count++] = lxs_trim(section);
				section = strtok_s(NULL, ";", &section_ctx);
				}

			if (section_count != 2U)
				{
				lxs_free_netlist(nl);
				fclose(stream);
				return NULL;
				}

			enable_net = lxs_intern_net(nl, sections[0]);
			down_net = lxs_intern_net(nl, sections[1]);
			if (enable_net == UINT32_MAX || down_net == UINT32_MAX || output_count == 0U ||
				!lxs_emit_counter_descriptor(nl, output_ids, output_count, enable_net, down_net, LXS_REGISTER_MODE_COUNTER_UPDOWN))
				{
				lxs_free_netlist(nl);
				fclose(stream);
				return NULL;
				}
			is_special_macro = 1U;
			}
		else if (lxs_parse_standard_rom_name(gate_name, &standard_rom_data_width))
			{
			char *section_ctx = NULL;
			char *sections[2];
			uint32_t section_count = 0U;
			char *section = strtok_s(open_paren + 1, ";", &section_ctx);
			char *addr_ctx = NULL;
			char *data_ctx = NULL;
			char *token;
			char *data_tokens[256];
			uint32_t data_token_count = 0U;

			while (section && section_count < 2U)
				{
				sections[section_count++] = lxs_trim(section);
				section = strtok_s(NULL, ";", &section_ctx);
				}

			if (section_count != 2U || output_count != standard_rom_data_width)
				{
				lxs_free_netlist(nl);
				fclose(stream);
				return NULL;
				}

			token = strtok_s(sections[0], ",", &addr_ctx);
			while (token && input_count < 16U)
				{
				input_ids[input_count] = lxs_intern_net(nl, lxs_trim(token));
				if (input_ids[input_count] == UINT32_MAX)
					{
					lxs_free_netlist(nl);
					fclose(stream);
					return NULL;
					}
				input_count++;
				token = strtok_s(NULL, ",", &addr_ctx);
				}

			token = strtok_s(sections[1], ",", &data_ctx);
			while (token && data_token_count < 256U)
				{
				data_tokens[data_token_count++] = lxs_trim(token);
				token = strtok_s(NULL, ",", &data_ctx);
				}

			if (!lxs_emit_rom_descriptor(nl, output_ids, output_count, input_ids, input_count, data_tokens, data_token_count))
				{
				lxs_free_netlist(nl);
				fclose(stream);
				return NULL;
				}
			is_special_macro = 1U;
			}
		else if (strcmp(gate_name, "ROM") == 0)
			{
			char *section_ctx = NULL;
			char *sections[2];
			uint32_t section_count = 0U;
			char *section = strtok_s(open_paren + 1, ";", &section_ctx);
			char *addr_ctx = NULL;
			char *data_ctx = NULL;
			char *token;
			char *data_tokens[256];
			uint32_t data_token_count = 0U;

			while (section && section_count < 2U)
				{
				sections[section_count++] = lxs_trim(section);
				section = strtok_s(NULL, ";", &section_ctx);
				}

			if (section_count != 2U)
				{
				lxs_free_netlist(nl);
				fclose(stream);
				return NULL;
				}

			token = strtok_s(sections[0], ",", &addr_ctx);
			while (token && input_count < 16U)
				{
				input_ids[input_count] = lxs_intern_net(nl, lxs_trim(token));
				if (input_ids[input_count] == UINT32_MAX)
					{
					lxs_free_netlist(nl);
					fclose(stream);
					return NULL;
					}
				input_count++;
				token = strtok_s(NULL, ",", &addr_ctx);
				}

			token = strtok_s(sections[1], ",", &data_ctx);
			while (token && data_token_count < 256U)
				{
				data_tokens[data_token_count++] = lxs_trim(token);
				token = strtok_s(NULL, ",", &data_ctx);
				}

			if (!lxs_emit_rom_descriptor(nl, output_ids, output_count, input_ids, input_count, data_tokens, data_token_count))
				{
				lxs_free_netlist(nl);
				fclose(stream);
				return NULL;
				}
			is_special_macro = 1U;
			}
		else if (lxs_parse_standard_ram_name(gate_name, &standard_ram_data_width))
			{
			char *section_ctx = NULL;
			char *sections[5];
			uint32_t section_count = 0U;
			char *section = strtok_s(open_paren + 1, ";", &section_ctx);
			char *read_ctx = NULL;
			char *write_ctx = NULL;
			char *data_ctx = NULL;
			char *init_ctx = NULL;
			char *token;
			char *data_tokens[256];
			uint32_t read_ids[16];
			uint32_t write_ids[16];
			uint32_t data_ids[64];
			uint32_t read_count = 0U;
			uint32_t write_count = 0U;
			uint32_t data_count = 0U;
			uint32_t data_token_count = 0U;
			uint32_t we_net;

			while (section && section_count < 5U)
				{
				sections[section_count++] = lxs_trim(section);
				section = strtok_s(NULL, ";", &section_ctx);
				}

			if (section_count != 5U || output_count != standard_ram_data_width)
				{
				lxs_free_netlist(nl);
				fclose(stream);
				return NULL;
				}

			token = strtok_s(sections[0], ",", &read_ctx);
			while (token && read_count < 16U)
				{
				read_ids[read_count] = lxs_intern_net(nl, lxs_trim(token));
				if (read_ids[read_count] == UINT32_MAX)
					{
					lxs_free_netlist(nl);
					fclose(stream);
					return NULL;
					}
				read_count++;
				token = strtok_s(NULL, ",", &read_ctx);
				}

			token = strtok_s(sections[1], ",", &write_ctx);
			while (token && write_count < 16U)
				{
				write_ids[write_count] = lxs_intern_net(nl, lxs_trim(token));
				if (write_ids[write_count] == UINT32_MAX)
					{
					lxs_free_netlist(nl);
					fclose(stream);
					return NULL;
					}
				write_count++;
				token = strtok_s(NULL, ",", &write_ctx);
				}

			token = strtok_s(sections[2], ",", &data_ctx);
			while (token && data_count < 64U)
				{
				data_ids[data_count] = lxs_intern_net(nl, lxs_trim(token));
				if (data_ids[data_count] == UINT32_MAX)
					{
					lxs_free_netlist(nl);
					fclose(stream);
					return NULL;
					}
				data_count++;
				token = strtok_s(NULL, ",", &data_ctx);
				}

			we_net = lxs_intern_net(nl, sections[3]);
			if (we_net == UINT32_MAX)
				{
				lxs_free_netlist(nl);
				fclose(stream);
				return NULL;
				}

			token = strtok_s(sections[4], ",", &init_ctx);
			while (token && data_token_count < 256U)
				{
				data_tokens[data_token_count++] = lxs_trim(token);
				token = strtok_s(NULL, ",", &init_ctx);
				}

			if (data_count != standard_ram_data_width ||
				!lxs_emit_ram_descriptor(
					nl,
					output_ids,
					output_count,
					read_ids,
					read_count,
					write_ids,
					write_count,
					data_ids,
					data_count,
					we_net,
					data_tokens,
					data_token_count))
				{
				lxs_free_netlist(nl);
				fclose(stream);
				return NULL;
				}
			is_special_macro = 1U;
			}
		else if (strcmp(gate_name, "RAM") == 0)
			{
			char *section_ctx = NULL;
			char *sections[5];
			uint32_t section_count = 0U;
			char *section = strtok_s(open_paren + 1, ";", &section_ctx);
			char *read_ctx = NULL;
			char *write_ctx = NULL;
			char *data_ctx = NULL;
			char *init_ctx = NULL;
			char *token;
			char *data_tokens[256];
			uint32_t read_ids[16];
			uint32_t write_ids[16];
			uint32_t data_ids[64];
			uint32_t read_count = 0U;
			uint32_t write_count = 0U;
			uint32_t data_count = 0U;
			uint32_t data_token_count = 0U;
			uint32_t we_net;

			while (section && section_count < 5U)
				{
				sections[section_count++] = lxs_trim(section);
				section = strtok_s(NULL, ";", &section_ctx);
				}

			if (section_count != 5U)
				{
				lxs_free_netlist(nl);
				fclose(stream);
				return NULL;
				}

			token = strtok_s(sections[0], ",", &read_ctx);
			while (token && read_count < 16U)
				{
				read_ids[read_count] = lxs_intern_net(nl, lxs_trim(token));
				if (read_ids[read_count] == UINT32_MAX)
					{
					lxs_free_netlist(nl);
					fclose(stream);
					return NULL;
					}
				read_count++;
				token = strtok_s(NULL, ",", &read_ctx);
				}

			token = strtok_s(sections[1], ",", &write_ctx);
			while (token && write_count < 16U)
				{
				write_ids[write_count] = lxs_intern_net(nl, lxs_trim(token));
				if (write_ids[write_count] == UINT32_MAX)
					{
					lxs_free_netlist(nl);
					fclose(stream);
					return NULL;
					}
				write_count++;
				token = strtok_s(NULL, ",", &write_ctx);
				}

			token = strtok_s(sections[2], ",", &data_ctx);
			while (token && data_count < 64U)
				{
				data_ids[data_count] = lxs_intern_net(nl, lxs_trim(token));
				if (data_ids[data_count] == UINT32_MAX)
					{
					lxs_free_netlist(nl);
					fclose(stream);
					return NULL;
					}
				data_count++;
				token = strtok_s(NULL, ",", &data_ctx);
				}

			we_net = lxs_intern_net(nl, lxs_trim(sections[3]));
			if (we_net == UINT32_MAX)
				{
				lxs_free_netlist(nl);
				fclose(stream);
				return NULL;
				}

			token = strtok_s(sections[4], ",", &init_ctx);
			while (token && data_token_count < 256U)
				{
				data_tokens[data_token_count++] = lxs_trim(token);
				token = strtok_s(NULL, ",", &init_ctx);
				}

			if (!lxs_emit_ram_descriptor(
					nl,
					output_ids,
					output_count,
					read_ids,
					read_count,
					write_ids,
					write_count,
					data_ids,
					data_count,
					we_net,
					data_tokens,
					data_token_count))
				{
				lxs_free_netlist(nl);
				fclose(stream);
				return NULL;
				}
			is_special_macro = 1U;
			}
		else if (lxs_parse_standard_regfile_name(gate_name, &standard_regfile_data_width))
			{
			char *section_ctx = NULL;
			char *sections[6];
			uint32_t section_count = 0U;
			char *section = strtok_s(open_paren + 1, ";", &section_ctx);
			char *read_a_ctx = NULL;
			char *read_b_ctx = NULL;
			char *write_ctx = NULL;
			char *data_ctx = NULL;
			char *init_ctx = NULL;
			char *token;
			char *data_tokens[256];
			uint32_t read_a_ids[16];
			uint32_t read_b_ids[16];
			uint32_t write_ids[16];
			uint32_t data_ids[64];
			uint32_t read_a_count = 0U;
			uint32_t read_b_count = 0U;
			uint32_t write_count = 0U;
			uint32_t data_count = 0U;
			uint32_t data_token_count = 0U;
			uint32_t we_net;

			while (section && section_count < 6U)
				{
				sections[section_count++] = lxs_trim(section);
				section = strtok_s(NULL, ";", &section_ctx);
				}

			if (section_count != 6U || output_count != (standard_regfile_data_width * 2U))
				{
				lxs_free_netlist(nl);
				fclose(stream);
				return NULL;
				}

			token = strtok_s(sections[0], ",", &read_a_ctx);
			while (token && read_a_count < 16U)
				{
				read_a_ids[read_a_count] = lxs_intern_net(nl, lxs_trim(token));
				if (read_a_ids[read_a_count] == UINT32_MAX)
					{
					lxs_free_netlist(nl);
					fclose(stream);
					return NULL;
					}
				read_a_count++;
				token = strtok_s(NULL, ",", &read_a_ctx);
				}

			token = strtok_s(sections[1], ",", &read_b_ctx);
			while (token && read_b_count < 16U)
				{
				read_b_ids[read_b_count] = lxs_intern_net(nl, lxs_trim(token));
				if (read_b_ids[read_b_count] == UINT32_MAX)
					{
					lxs_free_netlist(nl);
					fclose(stream);
					return NULL;
					}
				read_b_count++;
				token = strtok_s(NULL, ",", &read_b_ctx);
				}

			token = strtok_s(sections[2], ",", &write_ctx);
			while (token && write_count < 16U)
				{
				write_ids[write_count] = lxs_intern_net(nl, lxs_trim(token));
				if (write_ids[write_count] == UINT32_MAX)
					{
					lxs_free_netlist(nl);
					fclose(stream);
					return NULL;
					}
				write_count++;
				token = strtok_s(NULL, ",", &write_ctx);
				}

			token = strtok_s(sections[3], ",", &data_ctx);
			while (token && data_count < 64U)
				{
				data_ids[data_count] = lxs_intern_net(nl, lxs_trim(token));
				if (data_ids[data_count] == UINT32_MAX)
					{
					lxs_free_netlist(nl);
					fclose(stream);
					return NULL;
					}
				data_count++;
				token = strtok_s(NULL, ",", &data_ctx);
				}

			we_net = lxs_intern_net(nl, lxs_trim(sections[4]));
			if (we_net == UINT32_MAX)
				{
				lxs_free_netlist(nl);
				fclose(stream);
				return NULL;
				}

			token = strtok_s(sections[5], ",", &init_ctx);
			while (token && data_token_count < 256U)
				{
				data_tokens[data_token_count++] = lxs_trim(token);
				token = strtok_s(NULL, ",", &init_ctx);
				}

			if (data_count != standard_regfile_data_width ||
				!lxs_emit_regfile_descriptor(
					nl,
					output_ids,
					output_count,
					read_a_ids,
					read_a_count,
					read_b_ids,
					read_b_count,
					write_ids,
					write_count,
					data_ids,
					data_count,
					we_net,
					data_tokens,
					data_token_count))
				{
				lxs_free_netlist(nl);
				fclose(stream);
				return NULL;
				}
			is_special_macro = 1U;
			}
		else if (strcmp(gate_name, "REGFILE2") == 0)
			{
			char *section_ctx = NULL;
			char *sections[5];
			uint32_t section_count = 0U;
			char *section = strtok_s(open_paren + 1, ";", &section_ctx);
			char *read_ctx = NULL;
			char *write_ctx = NULL;
			char *data_ctx = NULL;
			char *init_ctx = NULL;
			char *token;
			char *data_tokens[2];
			uint32_t read_ids[1];
			uint32_t write_ids[1];
			uint32_t data_ids[64];
			uint32_t read_count = 0U;
			uint32_t write_count = 0U;
			uint32_t data_count = 0U;
			uint32_t data_token_count = 0U;
			uint32_t we_net;

			while (section && section_count < 5U)
				{
				sections[section_count++] = lxs_trim(section);
				section = strtok_s(NULL, ";", &section_ctx);
				}

			if (section_count != 5U)
				{
				lxs_free_netlist(nl);
				fclose(stream);
				return NULL;
				}

			token = strtok_s(sections[0], ",", &read_ctx);
			while (token && read_count < 1U)
				{
				read_ids[read_count] = lxs_intern_net(nl, lxs_trim(token));
				if (read_ids[read_count] == UINT32_MAX)
					{
					lxs_free_netlist(nl);
					fclose(stream);
					return NULL;
					}
				read_count++;
				token = strtok_s(NULL, ",", &read_ctx);
				}

			token = strtok_s(sections[1], ",", &write_ctx);
			while (token && write_count < 1U)
				{
				write_ids[write_count] = lxs_intern_net(nl, lxs_trim(token));
				if (write_ids[write_count] == UINT32_MAX)
					{
					lxs_free_netlist(nl);
					fclose(stream);
					return NULL;
					}
				write_count++;
				token = strtok_s(NULL, ",", &write_ctx);
				}

			token = strtok_s(sections[2], ",", &data_ctx);
			while (token && data_count < 64U)
				{
				data_ids[data_count] = lxs_intern_net(nl, lxs_trim(token));
				if (data_ids[data_count] == UINT32_MAX)
					{
					lxs_free_netlist(nl);
					fclose(stream);
					return NULL;
					}
				data_count++;
				token = strtok_s(NULL, ",", &data_ctx);
				}

			we_net = lxs_intern_net(nl, lxs_trim(sections[3]));
			if (we_net == UINT32_MAX)
				{
				lxs_free_netlist(nl);
				fclose(stream);
				return NULL;
				}

			token = strtok_s(sections[4], ",", &init_ctx);
			while (token && data_token_count < 2U)
				{
				data_tokens[data_token_count++] = lxs_trim(token);
				token = strtok_s(NULL, ",", &init_ctx);
				}

			if (!lxs_emit_ram_descriptor(
					nl,
					output_ids,
					output_count,
					read_ids,
					read_count,
					write_ids,
					write_count,
					data_ids,
					data_count,
					we_net,
					data_tokens,
					data_token_count))
				{
				lxs_free_netlist(nl);
				fclose(stream);
				return NULL;
				}
			is_special_macro = 1U;
			}
		else
			{
			if (strcmp(gate_name, "FUNC_REGION") == 0 ||
				strcmp(gate_name, "FUNC_EXPR") == 0 ||
				strcmp(gate_name, "FUNC_MICRO") == 0)
				{
				char *sections[LXS_FUNCTIONAL_REGION_MAX_OPS + 1U];
				char *section_ctx = NULL;
				char *section = strtok_s(open_paren + 1, ";", &section_ctx);
				uint8_t op_types[LXS_FUNCTIONAL_REGION_MAX_OPS];
				uint8_t op_dst_count[LXS_FUNCTIONAL_REGION_MAX_OPS];
				uint8_t op_dst_kind[LXS_FUNCTIONAL_REGION_MAX_OPS][2];
				uint8_t op_dst[LXS_FUNCTIONAL_REGION_MAX_OPS][2];
				uint8_t op_arity[LXS_FUNCTIONAL_REGION_MAX_OPS];
				uint8_t op_src[LXS_FUNCTIONAL_REGION_MAX_OPS][4];
				uint32_t op_count = 0U;
				uint32_t section_count = 0U;
				uint32_t temp_count = 0U;
				uint64_t defined_temp_mask = 0ULL;
				uint8_t assigned_outputs[LXS_FUNCTIONAL_REGION_MAX_OUTPUTS] = { 0 };
				uint8_t saw_explicit_dst = 0U;
				uint32_t exec_kind;

				if (strcmp(gate_name, "FUNC_EXPR") == 0)
					{
					exec_kind = LXS_FUNCTIONAL_REGION_EXEC_EXPR;
					}
				else if (strcmp(gate_name, "FUNC_MICRO") == 0)
					{
					exec_kind = LXS_FUNCTIONAL_REGION_EXEC_MICROPROGRAM;
					}
				else
					{
					exec_kind = LXS_FUNCTIONAL_REGION_EXEC_MICROPROGRAM;
					}

				while (section && section_count < (LXS_FUNCTIONAL_REGION_MAX_OPS + 1U))
					{
					sections[section_count++] = lxs_trim(section);
					section = strtok_s(NULL, ";", &section_ctx);
					}

				if (section_count < 2U)
					{
					lxs_free_netlist(nl);
					fclose(stream);
					return NULL;
					}

				{
				char *input_ctx = NULL;
				char *input_token = strtok_s(sections[0], ",", &input_ctx);

				while (input_token && input_count < LXS_FUNCTIONAL_REGION_MAX_INPUTS)
					{
					input_ids[input_count] = lxs_intern_net(nl, lxs_trim(input_token));
					if (input_ids[input_count] == UINT32_MAX)
						{
						lxs_free_netlist(nl);
						fclose(stream);
						return NULL;
						}
					input_count++;
					input_token = strtok_s(NULL, ",", &input_ctx);
					}
				}

				if (input_count == 0U)
					{
					lxs_free_netlist(nl);
					fclose(stream);
					return NULL;
					}

				for (uint32_t section_index = 1U; section_index < section_count; ++section_index)
					{
					char *op_expr = sections[section_index];
					char *op_open;
					char *op_close;
					char *arg_ctx = NULL;
					char *arg_token;
					uint8_t op_type;
					uint8_t dst_count = 0U;
					uint8_t dst_kind[2] = { 0U, 0U };
					uint8_t dst_index[2] = { 0U, 0U };
					uint32_t arg_count = 0U;
					char *assign = strchr(op_expr, '=');

					if (op_count >= LXS_FUNCTIONAL_REGION_MAX_OPS)
						{
						lxs_free_netlist(nl);
						fclose(stream);
						return NULL;
						}

					if (assign)
						{
						char *dst_ctx = NULL;
						char *dst_token;
						*assign = '\0';
						dst_token = strtok_s(op_expr, ",", &dst_ctx);
						while (dst_token && dst_count < 2U)
							{
							if (!lxs_parse_functional_dst(
									lxs_trim(dst_token),
									output_count,
									&dst_kind[dst_count],
									&dst_index[dst_count]))
								{
								lxs_free_netlist(nl);
								fclose(stream);
								return NULL;
								}
							dst_count++;
							dst_token = strtok_s(NULL, ",", &dst_ctx);
							}
						if (dst_count == 0U || dst_token)
							{
							lxs_free_netlist(nl);
							fclose(stream);
							return NULL;
							}
						op_expr = lxs_trim(assign + 1);
						saw_explicit_dst = 1U;
						for (uint32_t parsed_dst = 0; parsed_dst < dst_count; ++parsed_dst)
							{
							if (dst_kind[parsed_dst] == LXS_FUNCTIONAL_REGION_DST_TEMP)
								{
								if ((defined_temp_mask & (1ULL << dst_index[parsed_dst])) != 0ULL)
									{
									lxs_free_netlist(nl);
									fclose(stream);
									return NULL;
									}
								if ((uint32_t)dst_index[parsed_dst] + 1U > temp_count)
									{
									temp_count = (uint32_t)dst_index[parsed_dst] + 1U;
									}
								}
							else
								{
								if (assigned_outputs[dst_index[parsed_dst]])
									{
									lxs_free_netlist(nl);
									fclose(stream);
									return NULL;
									}
								assigned_outputs[dst_index[parsed_dst]] = 1U;
								}
							}
						}
					else
						{
						if (output_count != 1U)
							{
							lxs_free_netlist(nl);
							fclose(stream);
							return NULL;
							}
						if (section_index + 1U == section_count)
							{
							dst_count = 1U;
							dst_kind[0] = LXS_FUNCTIONAL_REGION_DST_OUTPUT;
							dst_index[0] = 0U;
							assigned_outputs[0] = 1U;
							}
						else
							{
							dst_count = 1U;
							dst_kind[0] = LXS_FUNCTIONAL_REGION_DST_TEMP;
							dst_index[0] = (uint8_t)op_count;
							temp_count = op_count + 1U;
							}
						}

					op_open = strchr(op_expr, '(');
					op_close = strrchr(op_expr, ')');
					if (!op_open || !op_close || op_close < op_open)
						{
						lxs_free_netlist(nl);
						fclose(stream);
						return NULL;
						}

					*op_open = '\0';
					*op_close = '\0';
					if (!lxs_parse_functional_op_type(lxs_trim(op_expr), &op_type))
						{
						lxs_free_netlist(nl);
						fclose(stream);
						return NULL;
						}

					arg_token = strtok_s(op_open + 1, ",", &arg_ctx);
					if (!arg_token)
						{
						lxs_free_netlist(nl);
						fclose(stream);
						return NULL;
						}

					while (arg_token && arg_count < 4U)
						{
						arg_token = lxs_trim(arg_token);
						if (!lxs_parse_functional_ref(arg_token, input_count, defined_temp_mask, &op_src[op_count][arg_count]))
							{
							lxs_free_netlist(nl);
							fclose(stream);
							return NULL;
							}
						arg_count++;
						arg_token = strtok_s(NULL, ",", &arg_ctx);
						}
					if ((op_type == LXS_FUNCTIONAL_REGION_OP_BUF || op_type == LXS_FUNCTIONAL_REGION_OP_NOT))
						{
						if (arg_count != 1U)
							{
							lxs_free_netlist(nl);
							fclose(stream);
							return NULL;
							}
						}
					else if (op_type == LXS_FUNCTIONAL_REGION_OP_MAJ3)
						{
						if (arg_count != 3U)
							{
							lxs_free_netlist(nl);
							fclose(stream);
							return NULL;
							}
						}
					else if (op_type == LXS_FUNCTIONAL_REGION_OP_HA2)
						{
						if (arg_count != 2U || dst_count != 2U)
							{
							lxs_free_netlist(nl);
							fclose(stream);
							return NULL;
							}
						}
					else if (op_type == LXS_FUNCTIONAL_REGION_OP_FA3)
						{
						if (arg_count != 3U || dst_count != 2U)
							{
							lxs_free_netlist(nl);
							fclose(stream);
							return NULL;
							}
						}
					else if (arg_count < 2U || arg_count > 4U)
						{
						lxs_free_netlist(nl);
						fclose(stream);
						return NULL;
						}
					op_types[op_count] = op_type;
					op_dst_count[op_count] = dst_count;
					for (uint32_t parsed_dst = 0; parsed_dst < dst_count; ++parsed_dst)
						{
						op_dst_kind[op_count][parsed_dst] = dst_kind[parsed_dst];
						op_dst[op_count][parsed_dst] = dst_index[parsed_dst];
						}
					op_arity[op_count] = (uint8_t)arg_count;
					for (uint32_t parsed_dst = 0; parsed_dst < dst_count; ++parsed_dst)
						{
						if (dst_kind[parsed_dst] == LXS_FUNCTIONAL_REGION_DST_TEMP)
							{
							defined_temp_mask |= (1ULL << dst_index[parsed_dst]);
							}
						}
					op_count++;
					}

				for (uint32_t output_index = 0U; output_index < output_count; ++output_index)
					{
					if (!assigned_outputs[output_index])
						{
						lxs_free_netlist(nl);
						fclose(stream);
						return NULL;
						}
					}

				if (strcmp(gate_name, "FUNC_REGION") == 0)
					{
					exec_kind = (output_count == 1U && !saw_explicit_dst) ?
						lxs_choose_functional_exec_kind(op_count, op_count > 0U ? (op_count - 1U) : 0U) :
						LXS_FUNCTIONAL_REGION_EXEC_MICROPROGRAM;
					}

				if (exec_kind == LXS_FUNCTIONAL_REGION_EXEC_EXPR && output_count != 1U)
					{
					lxs_free_netlist(nl);
					fclose(stream);
					return NULL;
					}

				if (!lxs_emit_functional_region(
						nl,
						exec_kind,
						input_ids,
						input_count,
						output_ids,
						output_count,
						op_types,
						op_dst_count,
						op_dst_kind,
						op_dst,
						op_arity,
						op_src,
						temp_count,
						op_count,
						macro_serial++))
					{
					lxs_free_netlist(nl);
					fclose(stream);
					return NULL;
					}
				is_special_macro = 1U;
				}
			else
				{
			char *input_ctx = NULL;
			char *input_token = strtok_s(open_paren + 1, ",", &input_ctx);

			while (input_token && input_count < 17U)
				{
				input_ids[input_count] = lxs_intern_net(nl, lxs_trim(input_token));
				if (input_ids[input_count] == UINT32_MAX)
					{
					lxs_free_netlist(nl);
					fclose(stream);
					return NULL;
					}

				input_count++;
				input_token = strtok_s(NULL, ",", &input_ctx);
				}
				}
			}

		if (strcmp(gate_name, "HALF_ADDER") == 0)
			{
			if (output_count != 2U || input_count != 2U ||
				!lxs_emit_half_adder_macro(nl, output_ids, input_ids))
				{
				lxs_free_netlist(nl);
				fclose(stream);
				return NULL;
				}
			is_special_macro = 1U;
			}
		else if (strcmp(gate_name, "FULL_ADDER") == 0)
			{
			if (output_count != 2U || input_count != 3U ||
				!lxs_emit_full_adder_macro(nl, output_ids, input_ids, macro_serial++))
				{
				lxs_free_netlist(nl);
				fclose(stream);
				return NULL;
				}
			is_special_macro = 1U;
			}
		else if (strcmp(gate_name, "RIPPLE_SLICE2") == 0)
			{
			if (output_count != 3U || input_count != 5U ||
				!lxs_emit_ripple_slice2_macro(nl, output_ids, input_ids, macro_serial++))
				{
				lxs_free_netlist(nl);
				fclose(stream);
				return NULL;
				}
			is_special_macro = 1U;
			}
		else if (strcmp(gate_name, "RIPPLE_ADD4") == 0)
			{
			if (output_count != 5U || input_count != 9U ||
				!lxs_emit_ripple_add4_macro(nl, output_ids, input_ids, macro_serial++))
				{
				lxs_free_netlist(nl);
				fclose(stream);
				return NULL;
			}
			is_special_macro = 1U;
			}
		else if (strcmp(gate_name, "CARRY_SAVE_ROW4") == 0)
			{
			if (output_count != 8U || input_count != 12U ||
				!lxs_emit_carry_save_row4_macro(nl, output_ids, input_ids, macro_serial++))
				{
				lxs_free_netlist(nl);
				fclose(stream);
				return NULL;
			}
			is_special_macro = 1U;
			}
		else if (strcmp(gate_name, "REDUCE_PROPAGATE4") == 0)
			{
			if (output_count != 6U || input_count != 12U ||
				!lxs_emit_reduce_propagate4_macro(nl, output_ids, input_ids, macro_serial++))
				{
				lxs_free_netlist(nl);
				fclose(stream);
				return NULL;
				}
			is_special_macro = 1U;
			}
		else if (strcmp(gate_name, "XOR_FAN8") == 0)
			{
			if (output_count != 8U || input_count != 9U ||
				!lxs_emit_xor_fan8_macro(nl, output_ids, input_ids))
				{
				lxs_free_netlist(nl);
				fclose(stream);
				return NULL;
			}
			is_special_macro = 1U;
			}
		else if (strcmp(gate_name, "AND_FAN8") == 0)
			{
			if (output_count != 8U || input_count != 9U ||
				!lxs_emit_and_fan8_macro(nl, output_ids, input_ids))
				{
				lxs_free_netlist(nl);
				fclose(stream);
				return NULL;
				}
			is_special_macro = 1U;
			}
		else if (strcmp(gate_name, "XNOR_BANK4") == 0)
			{
			if (output_count != 4U || input_count != 8U ||
				!lxs_emit_xnor_bank4_macro(nl, output_ids, input_ids))
				{
				lxs_free_netlist(nl);
				fclose(stream);
				return NULL;
				}
			is_special_macro = 1U;
			}
		else if (strcmp(gate_name, "GUARD_CHAIN4") == 0)
			{
			if (output_count != 5U || input_count != 5U ||
				!lxs_emit_guard_chain4_macro(nl, output_ids, input_ids))
				{
				lxs_free_netlist(nl);
				fclose(stream);
				return NULL;
				}
			is_special_macro = 1U;
			}
		else if (strcmp(gate_name, "PARITY4") == 0)
			{
			if (output_count != 1U || input_count != 4U ||
				!lxs_emit_parity4_macro(nl, output_ids[0], input_ids, macro_serial++))
				{
				lxs_free_netlist(nl);
				fclose(stream);
				return NULL;
				}
			is_special_macro = 1U;
			}
		else if (strcmp(gate_name, "PARITY8") == 0)
			{
			if (output_count != 1U || input_count != 8U ||
				!lxs_emit_parity8_macro(nl, output_ids[0], input_ids, macro_serial++))
				{
				lxs_free_netlist(nl);
				fclose(stream);
				return NULL;
				}
			is_special_macro = 1U;
			}

		if (!is_special_macro)
			{
			memset(&gate, 0, sizeof(gate));
			gate.type = lxs_string_to_gate_type(gate_name);
			gate.output = output_ids[0];
			gate.input_count = input_count > 8U ? 8U : input_count;
			for (uint32_t i = 0; i < gate.input_count; ++i)
				{
				gate.inputs[i] = input_ids[i];
				}

			if (!lxs_push_gate(nl, &gate))
				{
				lxs_free_netlist(nl);
				fclose(stream);
				return NULL;
				}
			}
		}
		}

	fclose(stream);
	return nl;
	}

lxs_netlist* lxs_load_iscas(const char *path)
	{
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
	lxs_free_aligned(nl->source_macros);
	lxs_free_aligned(nl->source_multi_macros);
	lxs_free_aligned(nl->source_standard_muxes);
	lxs_free_aligned(nl->source_standard_mux_data_net_ids);
	lxs_free_aligned(nl->source_standard_mux_select_net_ids);
	lxs_free_aligned(nl->source_standard_mux_output_net_ids);
	lxs_free_aligned(nl->source_standard_mux_gate_indices);
	lxs_free_aligned(nl->source_standard_adders);
	lxs_free_aligned(nl->source_standard_add_input_net_ids);
	lxs_free_aligned(nl->source_standard_add_output_net_ids);
	lxs_free_aligned(nl->source_standard_add_gate_indices);
	lxs_free_aligned(nl->source_standard_cmps);
	lxs_free_aligned(nl->source_standard_cmp_input_net_ids);
	lxs_free_aligned(nl->source_standard_cmp_output_net_ids);
	lxs_free_aligned(nl->source_standard_cmp_gate_indices);
	lxs_free_aligned(nl->source_standard_alus);
	lxs_free_aligned(nl->source_standard_alu_input_net_ids);
	lxs_free_aligned(nl->source_standard_alu_output_net_ids);
	lxs_free_aligned(nl->source_standard_alu_gate_indices);
	lxs_free_aligned(nl->source_functional_regions);
	lxs_free_aligned(nl->source_registers);
	lxs_free_aligned(nl->source_register_input_net_ids);
	lxs_free_aligned(nl->source_register_output_net_ids);
	lxs_free_aligned(nl->source_roms);
	lxs_free_aligned(nl->source_rom_addr_net_ids);
	lxs_free_aligned(nl->source_rom_output_net_ids);
	lxs_free_aligned(nl->source_rom_init_value);
	lxs_free_aligned(nl->source_rom_init_mask);
	lxs_free_aligned(nl->source_rams);
	lxs_free_aligned(nl->source_ram_read_addr_net_ids);
	lxs_free_aligned(nl->source_ram_write_addr_net_ids);
	lxs_free_aligned(nl->source_ram_data_input_net_ids);
	lxs_free_aligned(nl->source_ram_output_net_ids);
	lxs_free_aligned(nl->source_ram_init_value);
	lxs_free_aligned(nl->source_ram_init_mask);
	lxs_free_aligned(nl->source_regfiles);
	lxs_free_aligned(nl->source_regfile_read_a_addr_net_ids);
	lxs_free_aligned(nl->source_regfile_read_b_addr_net_ids);
	lxs_free_aligned(nl->source_regfile_write_addr_net_ids);
	lxs_free_aligned(nl->source_regfile_data_input_net_ids);
	lxs_free_aligned(nl->source_regfile_output_net_ids);
	lxs_free_aligned(nl->source_regfile_init_value);
	lxs_free_aligned(nl->source_regfile_init_mask);
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
	uint8_t *multi_matched_gates;
	lxs_plan *plan;
	uint32_t max_level = 0U;
	uint32_t chunk_fill = 0U;
	uint32_t gate_fill = 0U;
	uint32_t dff_fill = 0U;
	uint32_t next_net_id = 0U;
	uint32_t macro_fill = 0U;
	uint32_t multi_macro_fill = 0U;
	uint32_t standard_mux_fill = 0U;
	uint32_t standard_add_fill = 0U;
	uint32_t standard_cmp_fill = 0U;
	uint32_t standard_alu_fill = 0U;
	uint32_t macro_level_fill = 0U;
	uint32_t multi_macro_level_fill = 0U;
	uint32_t standard_mux_level_fill = 0U;
	uint32_t standard_add_level_fill = 0U;
	uint32_t standard_cmp_level_fill = 0U;
	uint32_t standard_alu_level_fill = 0U;
	uint32_t functional_region_level_fill = 0U;
	uint32_t functional_region_fill = 0U;
	uint32_t recognized_functional_count = 0U;
	uint32_t gate_alloc_count;

	if (!nl)
		{
		return NULL;
		}

	gate_alloc_count = nl->gate_count ? nl->gate_count : 1U;
	comb_driver = lxs_calloc_aligned(nl->net_count ? nl->net_count : 1U, sizeof(int32_t));
	level_cache = lxs_calloc_aligned(gate_alloc_count, sizeof(uint32_t));
	visiting = lxs_calloc_aligned(gate_alloc_count, sizeof(uint8_t));
	net_remap = lxs_calloc_aligned(nl->net_count, sizeof(uint32_t));
	net_assigned = lxs_calloc_aligned(nl->net_count, sizeof(uint8_t));
	matched_gates = lxs_calloc_aligned(gate_alloc_count, sizeof(uint8_t));
	multi_matched_gates = lxs_calloc_aligned(gate_alloc_count, sizeof(uint8_t));
	if (!comb_driver || !level_cache || !visiting ||
		!net_remap || !net_assigned || !matched_gates || !multi_matched_gates)
		{
		lxs_free_aligned(comb_driver);
		lxs_free_aligned(level_cache);
		lxs_free_aligned(visiting);
		lxs_free_aligned(net_remap);
		lxs_free_aligned(net_assigned);
		lxs_free_aligned(matched_gates);
		lxs_free_aligned(multi_matched_gates);
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

	if (nl->source_multi_macro_count > 0U)
		{
		if (!lxs_propagate_source_multi_output_levels(nl, comb_driver))
			{
			lxs_free_aligned(comb_driver);
			lxs_free_aligned(level_cache);
			lxs_free_aligned(visiting);
			lxs_free_aligned(net_remap);
			lxs_free_aligned(net_assigned);
			lxs_free_aligned(matched_gates);
			lxs_free_aligned(multi_matched_gates);
			return NULL;
			}

		max_level = 0U;
		for (uint32_t i = 0; i < nl->gate_count; ++i)
			{
			if (nl->gates[i].type != LXS_GATE_DFF && nl->gates[i].level > max_level)
				{
				max_level = nl->gates[i].level;
				}
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

	if (nl->source_multi_macro_count >= 128U)
		{
		for (uint32_t i = 0; i < nl->source_multi_macro_count; ++i)
			{
			const lxs_source_multi_macro *macro = &nl->source_multi_macros[i];
			switch (macro->type)
				{
				case LXS_SOURCE_MULTI_MACRO_HALF_ADDER:
				case LXS_SOURCE_MULTI_MACRO_FULL_ADDER:
				case LXS_SOURCE_MULTI_MACRO_RIPPLE_SLICE2:
				case LXS_SOURCE_MULTI_MACRO_RIPPLE_ADD4:
				case LXS_SOURCE_MULTI_MACRO_CARRY_SAVE_ROW4:
				case LXS_SOURCE_MULTI_MACRO_REDUCE_PROPAGATE4:
					lxs_assign_net_group(net_remap, net_assigned, macro->inputs, macro->input_count, &next_net_id);
					lxs_assign_net_group(net_remap, net_assigned, macro->outputs, macro->output_count, &next_net_id);
					break;
				default:
					break;
				}
			}
		}
	lxs_assign_remaining_nets(net_remap, net_assigned, nl->net_count, &next_net_id);

	lxs_apply_net_remap_to_array(nl->inputs, nl->input_count, net_remap);
	lxs_apply_net_remap_to_array(nl->outputs, nl->output_count, net_remap);
	lxs_apply_net_remap_to_gates(nl->gates, nl->gate_count, net_remap);
	lxs_apply_net_remap_to_source_macros(
		nl->source_macros,
		nl->source_macro_count,
		net_remap);
	lxs_apply_net_remap_to_source_multi_macros(
		nl->source_multi_macros,
		nl->source_multi_macro_count,
		net_remap);
	lxs_apply_net_remap_to_source_standard_muxes(nl, net_remap);
	lxs_apply_net_remap_to_source_standard_adders(nl, net_remap);
	lxs_apply_net_remap_to_source_standard_cmps(nl, net_remap);
	lxs_apply_net_remap_to_source_standard_alus(nl, net_remap);
	lxs_apply_net_remap_to_source_registers(nl, net_remap);
	lxs_apply_net_remap_to_source_roms(nl, net_remap);
	lxs_apply_net_remap_to_source_rams(nl, net_remap);
	lxs_apply_net_remap_to_source_regfiles(nl, net_remap);
	for (uint32_t i = 0; i < nl->net_count; ++i)
		{
		comb_driver[i] = -1;
		}
	for (uint32_t i = 0; i < nl->gate_count; ++i)
		{
		if (nl->gates[i].type != LXS_GATE_DFF)
			{
			comb_driver[nl->gates[i].output] = (int32_t)i;
			}
		}

	plan = lxs_calloc_aligned(1U, sizeof(lxs_plan));
	if (!plan)
		{
		lxs_free_aligned(comb_driver);
		lxs_free_aligned(level_cache);
		lxs_free_aligned(visiting);
		lxs_free_aligned(net_remap);
		lxs_free_aligned(net_assigned);
		lxs_free_aligned(matched_gates);
		lxs_free_aligned(multi_matched_gates);
		return NULL;
		}

	plan->net_count = nl->net_count;
	plan->gate_count = nl->gate_count;
	plan->recognition_mask = lxs_default_recognition_mask();
	plan->recognition_mode = lxs_default_recognition_mode();
	plan->multi_macro_count = lxs_collect_source_multi_macros(nl, comb_driver, multi_matched_gates, NULL);
	plan->standard_mux_count = lxs_collect_source_standard_muxes(nl, matched_gates, NULL);
	plan->standard_add_count = lxs_collect_source_standard_adders(nl, matched_gates, NULL);
	plan->standard_cmp_count = lxs_collect_source_standard_cmps(nl, matched_gates, NULL);
	plan->standard_alu_count = lxs_collect_source_standard_alus(nl, matched_gates, NULL);
	{
	uint8_t *recognition_multi_marks = multi_matched_gates;
	uint32_t recognized_multi_count;
	if (plan->recognition_mode == LXS_RECOGNITION_MODE_REPORT_ONLY)
		{
		recognition_multi_marks = lxs_malloc_aligned((size_t)gate_alloc_count * sizeof(uint8_t));
		if (!recognition_multi_marks)
			{
			lxs_free_aligned(comb_driver);
			lxs_free_aligned(level_cache);
			lxs_free_aligned(visiting);
			lxs_free_aligned(net_remap);
			lxs_free_aligned(net_assigned);
			lxs_free_aligned(matched_gates);
			lxs_free_aligned(multi_matched_gates);
			lxs_free_aligned(plan);
			return NULL;
			}
		memcpy(recognition_multi_marks, multi_matched_gates, nl->gate_count * sizeof(uint8_t));
		}
	recognized_multi_count = lxs_collect_multi_macros(
		nl,
		comb_driver,
		recognition_multi_marks,
		plan->recognition_mask,
		plan->recognition_mode,
		plan->recognition_match_count,
		plan->recognition_node_reduction,
		plan->recognition_candidate_roots,
		plan->recognition_nodes_visited,
		plan->recognition_max_depth,
		plan->recognition_abort_count,
		plan->recognition_time_us,
		NULL);
	if (plan->recognition_mode == LXS_RECOGNITION_MODE_REPORT_ONLY)
		{
		lxs_free_aligned(recognition_multi_marks);
		}
	if (recognized_multi_count == UINT32_MAX)
		{
		lxs_free_aligned(comb_driver);
		lxs_free_aligned(level_cache);
		lxs_free_aligned(visiting);
		lxs_free_aligned(net_remap);
		lxs_free_aligned(net_assigned);
		lxs_free_aligned(matched_gates);
			lxs_free_aligned(multi_matched_gates);
			lxs_free_aligned(plan);
			return NULL;
		}
	if (plan->recognition_mode == LXS_RECOGNITION_MODE_REPLACE)
		{
		plan->multi_macro_count += recognized_multi_count;
		}
	}
	memcpy(matched_gates, multi_matched_gates, (size_t)nl->gate_count * sizeof(uint8_t));
	plan->macro_count = lxs_collect_source_macros(nl, matched_gates, NULL);
	plan->functional_region_count = lxs_collect_source_functional_regions(nl, matched_gates, NULL);
	if (plan->recognition_mode == LXS_RECOGNITION_MODE_REPLACE)
		{
		uint8_t *functional_marks = lxs_malloc_aligned((size_t)gate_alloc_count * sizeof(uint8_t));
		uint32_t *net_use_count = lxs_calloc_aligned(nl->net_count ? nl->net_count : 1U, sizeof(uint32_t));
		if (!functional_marks || !net_use_count)
			{
			lxs_free_aligned(functional_marks);
			lxs_free_aligned(comb_driver);
			lxs_free_aligned(level_cache);
			lxs_free_aligned(visiting);
			lxs_free_aligned(net_remap);
			lxs_free_aligned(net_assigned);
			lxs_free_aligned(matched_gates);
			lxs_free_aligned(multi_matched_gates);
			lxs_free_aligned(plan);
			return NULL;
			}
		memcpy(functional_marks, matched_gates, nl->gate_count * sizeof(uint8_t));
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
		recognized_functional_count = lxs_collect_recognized_functional_regions(
			nl,
			comb_driver,
			net_use_count,
			functional_marks,
			plan->recognition_mask,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL,
			NULL);
		lxs_free_aligned(functional_marks);
		lxs_free_aligned(net_use_count);
		if (recognized_functional_count == UINT32_MAX)
			{
			lxs_free_aligned(comb_driver);
			lxs_free_aligned(level_cache);
			lxs_free_aligned(visiting);
			lxs_free_aligned(net_remap);
			lxs_free_aligned(net_assigned);
			lxs_free_aligned(matched_gates);
			lxs_free_aligned(multi_matched_gates);
			lxs_free_aligned(plan);
			return NULL;
			}
		plan->functional_region_count += recognized_functional_count;
		}
	{
	uint8_t *recognition_marks = matched_gates;
	uint32_t recognized_macro_count;
	if (plan->recognition_mode == LXS_RECOGNITION_MODE_REPORT_ONLY)
		{
		recognition_marks = lxs_malloc_aligned((size_t)gate_alloc_count * sizeof(uint8_t));
		if (!recognition_marks)
			{
			lxs_free_aligned(comb_driver);
			lxs_free_aligned(level_cache);
			lxs_free_aligned(visiting);
			lxs_free_aligned(net_remap);
			lxs_free_aligned(net_assigned);
			lxs_free_aligned(matched_gates);
			lxs_free_aligned(multi_matched_gates);
			lxs_free_aligned(plan);
			return NULL;
			}
		memcpy(recognition_marks, matched_gates, nl->gate_count * sizeof(uint8_t));
		}
	recognized_macro_count = lxs_collect_macros(
		nl,
		comb_driver,
		recognition_marks,
		plan->recognition_mask,
		plan->recognition_mode,
		plan->recognition_match_count,
		plan->recognition_node_reduction,
		plan->recognition_candidate_roots,
		plan->recognition_nodes_visited,
		plan->recognition_max_depth,
		plan->recognition_abort_count,
		plan->recognition_time_us,
		NULL);
	if (plan->recognition_mode == LXS_RECOGNITION_MODE_REPORT_ONLY)
		{
		lxs_free_aligned(recognition_marks);
		}
	if (recognized_macro_count == UINT32_MAX)
		{
		lxs_free_aligned(comb_driver);
		lxs_free_aligned(level_cache);
		lxs_free_aligned(visiting);
		lxs_free_aligned(net_remap);
		lxs_free_aligned(net_assigned);
		lxs_free_aligned(matched_gates);
			lxs_free_aligned(multi_matched_gates);
			lxs_free_aligned(plan);
			return NULL;
		}
	if (plan->recognition_mode == LXS_RECOGNITION_MODE_REPLACE)
		{
		plan->macro_count += recognized_macro_count;
		}
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
			plan->comb_gate_count++;
			if ((nl->gates[i].level + 1U) > plan->level_count)
				{
				plan->level_count = nl->gates[i].level + 1U;
				}
			}
		}

	plan->inputs.count = nl->input_count;
	plan->outputs.count = nl->output_count;
	plan->register_count = nl->source_register_count;
	plan->register_bit_count = nl->source_register_output_count;
	plan->register_input_net_count = nl->source_register_input_count;
	plan->standard_mux_data_net_count = nl->source_standard_mux_data_count;
	plan->standard_mux_select_net_count = nl->source_standard_mux_select_count;
	plan->standard_mux_output_net_count = nl->source_standard_mux_output_count;
	plan->standard_add_count = nl->source_standard_add_count;
	plan->standard_add_input_net_count = nl->source_standard_add_input_count;
	plan->standard_add_output_net_count = nl->source_standard_add_output_count;
	plan->standard_cmp_count = nl->source_standard_cmp_count;
	plan->standard_cmp_input_net_count = nl->source_standard_cmp_input_count;
	plan->standard_cmp_output_net_count = nl->source_standard_cmp_output_count;
	plan->standard_alu_count = nl->source_standard_alu_count;
	plan->standard_alu_input_net_count = nl->source_standard_alu_input_count;
	plan->standard_alu_output_net_count = nl->source_standard_alu_output_count;
	plan->rom_count = nl->source_rom_count;
	plan->rom_addr_net_count = nl->source_rom_addr_count;
	plan->rom_output_net_count = nl->source_rom_output_count;
	plan->rom_bit_count = nl->source_rom_init_count;
	plan->ram_count = nl->source_ram_count;
	plan->ram_storage_bit_count = nl->source_ram_init_count;
	plan->ram_read_addr_net_count = nl->source_ram_read_addr_count;
	plan->ram_write_addr_net_count = nl->source_ram_write_addr_count;
	plan->ram_data_input_net_count = nl->source_ram_data_input_count;
	plan->ram_output_net_count = nl->source_ram_output_count;
	for (uint32_t i = 0; i < nl->source_ram_count; ++i)
		{
		plan->ram_stage_bit_count += nl->source_rams[i].data_width;
		}
	plan->regfile_count = nl->source_regfile_count;
	plan->regfile_storage_bit_count = nl->source_regfile_init_count;
	plan->regfile_read_a_addr_net_count = nl->source_regfile_read_a_addr_count;
	plan->regfile_read_b_addr_net_count = nl->source_regfile_read_b_addr_count;
	plan->regfile_write_addr_net_count = nl->source_regfile_write_addr_count;
	plan->regfile_data_input_net_count = nl->source_regfile_data_input_count;
	plan->regfile_output_net_count = nl->source_regfile_output_count;
	for (uint32_t i = 0; i < nl->source_regfile_count; ++i)
		{
		plan->regfile_stage_bit_count += nl->source_regfiles[i].data_width;
		}
	plan->inputs.net_ids = lxs_calloc_aligned(plan->inputs.count, sizeof(uint32_t));
	plan->outputs.net_ids = lxs_calloc_aligned(plan->outputs.count, sizeof(uint32_t));
	plan->state.d_inputs = lxs_calloc_aligned(plan->state.count, sizeof(uint32_t));
	plan->state.q_outputs = lxs_calloc_aligned(plan->state.count, sizeof(uint32_t));
	plan->levels = lxs_calloc_aligned(plan->level_count, sizeof(lxs_level_plan));
	plan->comb_gates = lxs_calloc_aligned(plan->comb_gate_count, sizeof(lxs_gate_ir));
	plan->chunks = lxs_calloc_aligned(plan->comb_gate_count, sizeof(lxs_chunk_plan));
	plan->macros = lxs_calloc_aligned(plan->macro_count, sizeof(lxs_macro_plan));
	plan->multi_macros = lxs_calloc_aligned(plan->multi_macro_count, sizeof(lxs_multi_macro_plan));
	plan->standard_muxes = lxs_calloc_aligned(plan->standard_mux_count, sizeof(lxs_standard_mux_plan));
	plan->standard_adders = lxs_calloc_aligned(plan->standard_add_count, sizeof(lxs_standard_add_plan));
	plan->standard_cmps = lxs_calloc_aligned(plan->standard_cmp_count, sizeof(lxs_standard_cmp_plan));
	plan->standard_alus = lxs_calloc_aligned(plan->standard_alu_count, sizeof(lxs_standard_alu_plan));
	plan->functional_regions = lxs_calloc_aligned(
		plan->functional_region_count,
		sizeof(lxs_functional_region_plan));
	plan->standard_mux_data_net_ids = lxs_calloc_aligned(plan->standard_mux_data_net_count, sizeof(uint32_t));
	plan->standard_mux_select_net_ids = lxs_calloc_aligned(plan->standard_mux_select_net_count, sizeof(uint32_t));
	plan->standard_mux_output_net_ids = lxs_calloc_aligned(plan->standard_mux_output_net_count, sizeof(uint32_t));
	plan->standard_add_input_net_ids = lxs_calloc_aligned(plan->standard_add_input_net_count, sizeof(uint32_t));
	plan->standard_add_output_net_ids = lxs_calloc_aligned(plan->standard_add_output_net_count, sizeof(uint32_t));
	plan->standard_cmp_input_net_ids = lxs_calloc_aligned(plan->standard_cmp_input_net_count, sizeof(uint32_t));
	plan->standard_cmp_output_net_ids = lxs_calloc_aligned(plan->standard_cmp_output_net_count, sizeof(uint32_t));
	plan->standard_alu_input_net_ids = lxs_calloc_aligned(plan->standard_alu_input_net_count, sizeof(uint32_t));
	plan->standard_alu_output_net_ids = lxs_calloc_aligned(plan->standard_alu_output_net_count, sizeof(uint32_t));
	plan->registers = lxs_calloc_aligned(plan->register_count, sizeof(lxs_register_plan));
	plan->register_input_net_ids = lxs_calloc_aligned(plan->register_input_net_count, sizeof(uint32_t));
	plan->register_output_net_ids = lxs_calloc_aligned(plan->register_bit_count, sizeof(uint32_t));
	plan->register_init_value = lxs_calloc_aligned(plan->register_bit_count, sizeof(uint64_t));
	plan->register_init_mask = lxs_calloc_aligned(plan->register_bit_count, sizeof(uint64_t));
	plan->roms = lxs_calloc_aligned(plan->rom_count, sizeof(lxs_rom_plan));
	plan->rom_addr_net_ids = lxs_calloc_aligned(plan->rom_addr_net_count, sizeof(uint32_t));
	plan->rom_output_net_ids = lxs_calloc_aligned(plan->rom_output_net_count, sizeof(uint32_t));
	plan->rom_init_value = lxs_calloc_aligned(plan->rom_bit_count, sizeof(uint64_t));
	plan->rom_init_mask = lxs_calloc_aligned(plan->rom_bit_count, sizeof(uint64_t));
	plan->rams = lxs_calloc_aligned(plan->ram_count, sizeof(lxs_ram_plan));
	plan->ram_read_addr_net_ids = lxs_calloc_aligned(plan->ram_read_addr_net_count, sizeof(uint32_t));
	plan->ram_write_addr_net_ids = lxs_calloc_aligned(plan->ram_write_addr_net_count, sizeof(uint32_t));
	plan->ram_data_input_net_ids = lxs_calloc_aligned(plan->ram_data_input_net_count, sizeof(uint32_t));
	plan->ram_output_net_ids = lxs_calloc_aligned(plan->ram_output_net_count, sizeof(uint32_t));
	plan->ram_init_value = lxs_calloc_aligned(plan->ram_storage_bit_count, sizeof(uint64_t));
	plan->ram_init_mask = lxs_calloc_aligned(plan->ram_storage_bit_count, sizeof(uint64_t));
	plan->regfiles = lxs_calloc_aligned(plan->regfile_count, sizeof(lxs_regfile_plan));
	plan->regfile_read_a_addr_net_ids = lxs_calloc_aligned(plan->regfile_read_a_addr_net_count, sizeof(uint32_t));
	plan->regfile_read_b_addr_net_ids = lxs_calloc_aligned(plan->regfile_read_b_addr_net_count, sizeof(uint32_t));
	plan->regfile_write_addr_net_ids = lxs_calloc_aligned(plan->regfile_write_addr_net_count, sizeof(uint32_t));
	plan->regfile_data_input_net_ids = lxs_calloc_aligned(plan->regfile_data_input_net_count, sizeof(uint32_t));
	plan->regfile_output_net_ids = lxs_calloc_aligned(plan->regfile_output_net_count, sizeof(uint32_t));
	plan->regfile_init_value = lxs_calloc_aligned(plan->regfile_storage_bit_count, sizeof(uint64_t));
	plan->regfile_init_mask = lxs_calloc_aligned(plan->regfile_storage_bit_count, sizeof(uint64_t));

	if ((plan->inputs.count && !plan->inputs.net_ids) ||
		(plan->outputs.count && !plan->outputs.net_ids) ||
		(plan->state.count && (!plan->state.d_inputs || !plan->state.q_outputs)) ||
		(plan->level_count && !plan->levels) ||
		(plan->comb_gate_count && (!plan->comb_gates || !plan->chunks)) ||
		(plan->macro_count && !plan->macros) ||
		(plan->multi_macro_count && !plan->multi_macros) ||
		(plan->standard_mux_count && !plan->standard_muxes) ||
		(plan->standard_add_count && !plan->standard_adders) ||
		(plan->standard_cmp_count && !plan->standard_cmps) ||
		(plan->standard_alu_count && !plan->standard_alus) ||
		(plan->functional_region_count && !plan->functional_regions) ||
		(plan->standard_mux_data_net_count && !plan->standard_mux_data_net_ids) ||
		(plan->standard_mux_select_net_count && !plan->standard_mux_select_net_ids) ||
		(plan->standard_mux_output_net_count && !plan->standard_mux_output_net_ids) ||
		(plan->standard_add_input_net_count && !plan->standard_add_input_net_ids) ||
		(plan->standard_add_output_net_count && !plan->standard_add_output_net_ids) ||
		(plan->standard_cmp_input_net_count && !plan->standard_cmp_input_net_ids) ||
		(plan->standard_cmp_output_net_count && !plan->standard_cmp_output_net_ids) ||
		(plan->standard_alu_input_net_count && !plan->standard_alu_input_net_ids) ||
		(plan->standard_alu_output_net_count && !plan->standard_alu_output_net_ids) ||
		(plan->register_count && (!plan->registers || !plan->register_output_net_ids ||
			!plan->register_init_value || !plan->register_init_mask)) ||
		(plan->register_input_net_count && !plan->register_input_net_ids) ||
		(plan->rom_count && (!plan->roms || !plan->rom_addr_net_ids || !plan->rom_output_net_ids ||
			!plan->rom_init_value || !plan->rom_init_mask)) ||
		(plan->ram_count && (!plan->rams || !plan->ram_read_addr_net_ids || !plan->ram_write_addr_net_ids ||
			!plan->ram_data_input_net_ids || !plan->ram_output_net_ids || !plan->ram_init_value || !plan->ram_init_mask)) ||
		(plan->regfile_count && (!plan->regfiles || !plan->regfile_read_a_addr_net_ids ||
			!plan->regfile_read_b_addr_net_ids || !plan->regfile_write_addr_net_ids ||
			!plan->regfile_data_input_net_ids || !plan->regfile_output_net_ids ||
			!plan->regfile_init_value || !plan->regfile_init_mask)))
		{
		lxs_free_plan(plan);
		lxs_free_aligned(comb_driver);
		lxs_free_aligned(level_cache);
		lxs_free_aligned(visiting);
		lxs_free_aligned(net_remap);
		lxs_free_aligned(net_assigned);
		lxs_free_aligned(matched_gates);
		lxs_free_aligned(multi_matched_gates);
		return NULL;
		}

	memcpy(plan->inputs.net_ids, nl->inputs, (size_t)plan->inputs.count * sizeof(uint32_t));
	memcpy(plan->outputs.net_ids, nl->outputs, (size_t)plan->outputs.count * sizeof(uint32_t));
	memcpy(plan->standard_mux_data_net_ids, nl->source_standard_mux_data_net_ids, (size_t)plan->standard_mux_data_net_count * sizeof(uint32_t));
	memcpy(plan->standard_mux_select_net_ids, nl->source_standard_mux_select_net_ids, (size_t)plan->standard_mux_select_net_count * sizeof(uint32_t));
	memcpy(plan->standard_mux_output_net_ids, nl->source_standard_mux_output_net_ids, (size_t)plan->standard_mux_output_net_count * sizeof(uint32_t));
	memcpy(plan->standard_add_input_net_ids, nl->source_standard_add_input_net_ids, (size_t)plan->standard_add_input_net_count * sizeof(uint32_t));
	memcpy(plan->standard_add_output_net_ids, nl->source_standard_add_output_net_ids, (size_t)plan->standard_add_output_net_count * sizeof(uint32_t));
	memcpy(plan->standard_cmp_input_net_ids, nl->source_standard_cmp_input_net_ids, (size_t)plan->standard_cmp_input_net_count * sizeof(uint32_t));
	memcpy(plan->standard_cmp_output_net_ids, nl->source_standard_cmp_output_net_ids, (size_t)plan->standard_cmp_output_net_count * sizeof(uint32_t));
	memcpy(plan->standard_alu_input_net_ids, nl->source_standard_alu_input_net_ids, (size_t)plan->standard_alu_input_net_count * sizeof(uint32_t));
	memcpy(plan->standard_alu_output_net_ids, nl->source_standard_alu_output_net_ids, (size_t)plan->standard_alu_output_net_count * sizeof(uint32_t));
	memcpy(plan->register_input_net_ids, nl->source_register_input_net_ids, (size_t)plan->register_input_net_count * sizeof(uint32_t));
	memcpy(plan->register_output_net_ids, nl->source_register_output_net_ids, (size_t)plan->register_bit_count * sizeof(uint32_t));
	memcpy(plan->rom_addr_net_ids, nl->source_rom_addr_net_ids, (size_t)plan->rom_addr_net_count * sizeof(uint32_t));
	memcpy(plan->rom_output_net_ids, nl->source_rom_output_net_ids, (size_t)plan->rom_output_net_count * sizeof(uint32_t));
	memcpy(plan->rom_init_value, nl->source_rom_init_value, (size_t)plan->rom_bit_count * sizeof(uint64_t));
	memcpy(plan->rom_init_mask, nl->source_rom_init_mask, (size_t)plan->rom_bit_count * sizeof(uint64_t));
	memcpy(plan->ram_read_addr_net_ids, nl->source_ram_read_addr_net_ids, (size_t)plan->ram_read_addr_net_count * sizeof(uint32_t));
	memcpy(plan->ram_write_addr_net_ids, nl->source_ram_write_addr_net_ids, (size_t)plan->ram_write_addr_net_count * sizeof(uint32_t));
	memcpy(plan->ram_data_input_net_ids, nl->source_ram_data_input_net_ids, (size_t)plan->ram_data_input_net_count * sizeof(uint32_t));
	memcpy(plan->ram_output_net_ids, nl->source_ram_output_net_ids, (size_t)plan->ram_output_net_count * sizeof(uint32_t));
	memcpy(plan->ram_init_value, nl->source_ram_init_value, (size_t)plan->ram_storage_bit_count * sizeof(uint64_t));
	memcpy(plan->ram_init_mask, nl->source_ram_init_mask, (size_t)plan->ram_storage_bit_count * sizeof(uint64_t));
	memcpy(plan->regfile_read_a_addr_net_ids, nl->source_regfile_read_a_addr_net_ids, (size_t)plan->regfile_read_a_addr_net_count * sizeof(uint32_t));
	memcpy(plan->regfile_read_b_addr_net_ids, nl->source_regfile_read_b_addr_net_ids, (size_t)plan->regfile_read_b_addr_net_count * sizeof(uint32_t));
	memcpy(plan->regfile_write_addr_net_ids, nl->source_regfile_write_addr_net_ids, (size_t)plan->regfile_write_addr_net_count * sizeof(uint32_t));
	memcpy(plan->regfile_data_input_net_ids, nl->source_regfile_data_input_net_ids, (size_t)plan->regfile_data_input_net_count * sizeof(uint32_t));
	memcpy(plan->regfile_output_net_ids, nl->source_regfile_output_net_ids, (size_t)plan->regfile_output_net_count * sizeof(uint32_t));
	memcpy(plan->regfile_init_value, nl->source_regfile_init_value, (size_t)plan->regfile_storage_bit_count * sizeof(uint64_t));
	memcpy(plan->regfile_init_mask, nl->source_regfile_init_mask, (size_t)plan->regfile_storage_bit_count * sizeof(uint64_t));
	plan->inputs.is_contiguous =
		lxs_find_contiguous_range(plan->inputs.net_ids, plan->inputs.count, &plan->inputs.contiguous_base);
	plan->outputs.is_contiguous =
		lxs_find_contiguous_range(plan->outputs.net_ids, plan->outputs.count, &plan->outputs.contiguous_base);

	for (uint32_t i = 0; i < plan->register_count; ++i)
		{
		plan->registers[i].width_bits = nl->source_registers[i].width_bits;
		plan->registers[i].input_start = nl->source_registers[i].input_start;
		plan->registers[i].output_start = nl->source_registers[i].output_start;
		plan->registers[i].storage_offset = nl->source_registers[i].output_start;
		plan->registers[i].control_net = nl->source_registers[i].control_net;
		plan->registers[i].aux_control_net = nl->source_registers[i].aux_control_net;
		plan->registers[i].mode = nl->source_registers[i].mode;
		plan->registers[i].control_invert = nl->source_registers[i].control_invert;
		}

	for (uint32_t i = 0; i < plan->rom_count; ++i)
		{
		plan->roms[i].addr_width = nl->source_roms[i].addr_width;
		plan->roms[i].data_width = nl->source_roms[i].data_width;
		plan->roms[i].depth = nl->source_roms[i].depth;
		plan->roms[i].addr_input_start = nl->source_roms[i].addr_start;
		plan->roms[i].output_start = nl->source_roms[i].output_start;
		plan->roms[i].data_offset = nl->source_roms[i].data_start;
		}

	{
	uint32_t ram_stage_offset = 0U;
	for (uint32_t i = 0; i < plan->ram_count; ++i)
		{
		plan->rams[i].addr_width = nl->source_rams[i].addr_width;
		plan->rams[i].data_width = nl->source_rams[i].data_width;
		plan->rams[i].depth = nl->source_rams[i].depth;
		plan->rams[i].read_addr_start = nl->source_rams[i].read_addr_start;
		plan->rams[i].write_addr_start = nl->source_rams[i].write_addr_start;
		plan->rams[i].data_input_start = nl->source_rams[i].data_input_start;
		plan->rams[i].output_start = nl->source_rams[i].output_start;
		plan->rams[i].storage_offset = nl->source_rams[i].data_start;
		plan->rams[i].stage_offset = ram_stage_offset;
		plan->rams[i].write_enable_net = nl->source_rams[i].write_enable_net;
		ram_stage_offset += nl->source_rams[i].data_width;
		}
	}

	{
	uint32_t regfile_stage_offset = 0U;
	for (uint32_t i = 0; i < plan->regfile_count; ++i)
		{
		plan->regfiles[i].addr_width = nl->source_regfiles[i].addr_width;
		plan->regfiles[i].data_width = nl->source_regfiles[i].data_width;
		plan->regfiles[i].depth = nl->source_regfiles[i].depth;
		plan->regfiles[i].read_a_addr_start = nl->source_regfiles[i].read_a_addr_start;
		plan->regfiles[i].read_b_addr_start = nl->source_regfiles[i].read_b_addr_start;
		plan->regfiles[i].write_addr_start = nl->source_regfiles[i].write_addr_start;
		plan->regfiles[i].data_input_start = nl->source_regfiles[i].data_input_start;
		plan->regfiles[i].output_start = nl->source_regfiles[i].output_start;
		plan->regfiles[i].storage_offset = nl->source_regfiles[i].data_start;
		plan->regfiles[i].stage_offset = regfile_stage_offset;
		plan->regfiles[i].write_enable_net = nl->source_regfiles[i].write_enable_net;
		regfile_stage_offset += nl->source_regfiles[i].data_width;
		}
	}

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

	if (plan->multi_macro_count > 0U)
		{
		memset(multi_matched_gates, 0, (size_t)nl->gate_count * sizeof(uint8_t));
		multi_macro_fill = lxs_collect_source_multi_macros(nl, comb_driver, multi_matched_gates, plan->multi_macros);
		if (plan->recognition_mode == LXS_RECOGNITION_MODE_REPLACE)
			{
			uint32_t recognized_multi_fill = lxs_collect_multi_macros(
				nl,
				comb_driver,
				multi_matched_gates,
				plan->recognition_mask,
				plan->recognition_mode,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				plan->multi_macros + multi_macro_fill);
			if (recognized_multi_fill == UINT32_MAX)
				{
				lxs_free_plan(plan);
				lxs_free_aligned(comb_driver);
				lxs_free_aligned(level_cache);
				lxs_free_aligned(visiting);
				lxs_free_aligned(net_remap);
				lxs_free_aligned(net_assigned);
				lxs_free_aligned(matched_gates);
				lxs_free_aligned(multi_matched_gates);
				return NULL;
				}
			multi_macro_fill += recognized_multi_fill;
			}
		plan->multi_macro_count = multi_macro_fill;
		}

	memcpy(matched_gates, multi_matched_gates, (size_t)nl->gate_count * sizeof(uint8_t));
	if (plan->standard_mux_count > 0U)
		{
		standard_mux_fill = lxs_collect_source_standard_muxes(
			nl,
			matched_gates,
			plan->standard_muxes);
		plan->standard_mux_count = standard_mux_fill;
		}
	if (plan->standard_add_count > 0U)
		{
		standard_add_fill = lxs_collect_source_standard_adders(
			nl,
			matched_gates,
			plan->standard_adders);
		plan->standard_add_count = standard_add_fill;
		}
	if (plan->standard_cmp_count > 0U)
		{
		standard_cmp_fill = lxs_collect_source_standard_cmps(
			nl,
			matched_gates,
			plan->standard_cmps);
		plan->standard_cmp_count = standard_cmp_fill;
		}
	if (plan->standard_alu_count > 0U)
		{
		standard_alu_fill = lxs_collect_source_standard_alus(
			nl,
			matched_gates,
			plan->standard_alus);
		plan->standard_alu_count = standard_alu_fill;
		}
	if (plan->functional_region_count > 0U)
		{
		functional_region_fill = lxs_collect_source_functional_regions(
			nl,
			matched_gates,
			plan->functional_regions);
		if (plan->recognition_mode == LXS_RECOGNITION_MODE_REPLACE && recognized_functional_count > 0U)
			{
			uint32_t *net_use_count = lxs_calloc_aligned(nl->net_count ? nl->net_count : 1U, sizeof(uint32_t));
			if (!net_use_count)
				{
				lxs_free_plan(plan);
				lxs_free_aligned(comb_driver);
				lxs_free_aligned(level_cache);
				lxs_free_aligned(visiting);
				lxs_free_aligned(net_remap);
				lxs_free_aligned(net_assigned);
				lxs_free_aligned(matched_gates);
				lxs_free_aligned(multi_matched_gates);
				return NULL;
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
			functional_region_fill += lxs_collect_recognized_functional_regions(
				nl,
				comb_driver,
				net_use_count,
				matched_gates,
				plan->recognition_mask,
				plan->recognition_match_count,
				plan->recognition_node_reduction,
				plan->recognition_candidate_roots,
				plan->recognition_nodes_visited,
				plan->recognition_max_depth,
				plan->recognition_abort_count,
				plan->recognition_time_us,
				plan->functional_regions + functional_region_fill);
			lxs_free_aligned(net_use_count);
			}
		plan->functional_region_count = functional_region_fill;
		}
	plan->comb_gate_count = 0U;
	if (plan->macro_count > 0U)
		{
		macro_fill = lxs_collect_source_macros(nl, matched_gates, plan->macros);
		if (plan->recognition_mode == LXS_RECOGNITION_MODE_REPLACE)
			{
			uint32_t recognized_macro_fill = lxs_collect_macros(
				nl,
				comb_driver,
				matched_gates,
				plan->recognition_mask,
				plan->recognition_mode,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				NULL,
				plan->macros + macro_fill);
			if (recognized_macro_fill == UINT32_MAX)
				{
				lxs_free_plan(plan);
				lxs_free_aligned(comb_driver);
				lxs_free_aligned(level_cache);
				lxs_free_aligned(visiting);
				lxs_free_aligned(net_remap);
				lxs_free_aligned(net_assigned);
				lxs_free_aligned(matched_gates);
				lxs_free_aligned(multi_matched_gates);
				return NULL;
				}
			macro_fill += recognized_macro_fill;
			}
		plan->macro_count = macro_fill;
		qsort(plan->macros, plan->macro_count, sizeof(lxs_macro_plan), lxs_compare_macros);
		}
	else
		{
		plan->macro_count = 0U;
		}

	for (uint32_t i = 0; i < nl->gate_count; ++i)
		{
		if (nl->gates[i].type != LXS_GATE_DFF && !matched_gates[i])
			{
			plan->comb_gate_count++;
			}
		}

	if (plan->multi_macro_count > 1U)
		{
		qsort(
			plan->multi_macros,
			plan->multi_macro_count,
			sizeof(lxs_multi_macro_plan),
			lxs_compare_multi_macros);
		}

	if (plan->standard_mux_count > 1U)
		{
		qsort(
			plan->standard_muxes,
			plan->standard_mux_count,
			sizeof(lxs_standard_mux_plan),
			lxs_compare_standard_muxes);
		}

	if (plan->standard_add_count > 1U)
		{
		qsort(
			plan->standard_adders,
			plan->standard_add_count,
			sizeof(lxs_standard_add_plan),
			lxs_compare_standard_adders);
		}

	if (plan->standard_cmp_count > 1U)
		{
		qsort(
			plan->standard_cmps,
			plan->standard_cmp_count,
			sizeof(lxs_standard_cmp_plan),
			lxs_compare_standard_cmps);
		}

	if (plan->standard_alu_count > 1U)
		{
		qsort(
			plan->standard_alus,
			plan->standard_alu_count,
			sizeof(lxs_standard_alu_plan),
			lxs_compare_standard_alus);
		}

	if (plan->functional_region_count > 1U)
		{
		qsort(
			plan->functional_regions,
			plan->functional_region_count,
			sizeof(lxs_functional_region_plan),
			lxs_compare_functional_regions);
		}

	for (uint32_t level = 0; level < plan->level_count; ++level)
		{
		uint32_t level_chunk_start = chunk_fill;
		uint32_t level_macro_start = macro_level_fill;
		uint32_t level_standard_mux_start = standard_mux_level_fill;
		uint32_t level_standard_add_start = standard_add_level_fill;
		uint32_t level_standard_cmp_start = standard_cmp_level_fill;
		uint32_t level_standard_alu_start = standard_alu_level_fill;
		uint32_t level_functional_region_start = functional_region_level_fill;

		for (uint32_t type = 0; type < (uint32_t)LXS_GATE_DFF; ++type)
			{
			uint32_t count = lxs_count_bucket_gates(nl, level, type);
			uint32_t primitive_count = 0U;
			uint32_t primitive_gate_equiv_count = 0U;
			if (count == 0U)
				{
				continue;
				}

			for (uint32_t i = 0; i < nl->gate_count; ++i)
				{
				if (nl->gates[i].type == type && nl->gates[i].level == level && !matched_gates[i])
					{
					plan->comb_gates[gate_fill + primitive_count] = nl->gates[i];
					primitive_gate_equiv_count += lxs_gate_equiv_count(&nl->gates[i]);
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
			plan->chunks[chunk_fill].gate_equiv_count = primitive_gate_equiv_count;
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

		plan->levels[level].multi_macro_start = multi_macro_level_fill;
		while (multi_macro_level_fill < plan->multi_macro_count &&
			plan->multi_macros[multi_macro_level_fill].level == level)
			{
			multi_macro_level_fill++;
			}
		plan->levels[level].multi_macro_count = multi_macro_level_fill - plan->levels[level].multi_macro_start;

		plan->levels[level].standard_mux_start = level_standard_mux_start;
		while (standard_mux_level_fill < plan->standard_mux_count &&
			plan->standard_muxes[standard_mux_level_fill].level == level)
			{
			standard_mux_level_fill++;
			}
		plan->levels[level].standard_mux_count =
			standard_mux_level_fill - level_standard_mux_start;

		plan->levels[level].standard_add_start = level_standard_add_start;
		while (standard_add_level_fill < plan->standard_add_count &&
			plan->standard_adders[standard_add_level_fill].level == level)
			{
			standard_add_level_fill++;
			}
		plan->levels[level].standard_add_count =
			standard_add_level_fill - level_standard_add_start;

		plan->levels[level].standard_cmp_start = level_standard_cmp_start;
		while (standard_cmp_level_fill < plan->standard_cmp_count &&
			plan->standard_cmps[standard_cmp_level_fill].level == level)
			{
			standard_cmp_level_fill++;
			}
		plan->levels[level].standard_cmp_count =
			standard_cmp_level_fill - level_standard_cmp_start;

		plan->levels[level].standard_alu_start = level_standard_alu_start;
		while (standard_alu_level_fill < plan->standard_alu_count &&
			plan->standard_alus[standard_alu_level_fill].level == level)
			{
			standard_alu_level_fill++;
			}
		plan->levels[level].standard_alu_count =
			standard_alu_level_fill - level_standard_alu_start;

		plan->levels[level].functional_region_start = level_functional_region_start;
		while (functional_region_level_fill < plan->functional_region_count &&
			plan->functional_regions[functional_region_level_fill].level == level)
			{
			functional_region_level_fill++;
			}
		plan->levels[level].functional_region_count =
			functional_region_level_fill - level_functional_region_start;
		}

	plan->span_count = chunk_fill;
	lxs_free_aligned(comb_driver);
	lxs_free_aligned(level_cache);
	lxs_free_aligned(visiting);
	lxs_free_aligned(net_remap);
	lxs_free_aligned(net_assigned);
	lxs_free_aligned(matched_gates);
	lxs_free_aligned(multi_matched_gates);
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
	lxs_free_aligned(plan->multi_macros);
	lxs_free_aligned(plan->standard_muxes);
	lxs_free_aligned(plan->standard_adders);
	lxs_free_aligned(plan->standard_cmps);
	lxs_free_aligned(plan->standard_alus);
	lxs_free_aligned(plan->functional_regions);
	lxs_free_aligned(plan->levels);
	lxs_free_aligned(plan->inputs.net_ids);
	lxs_free_aligned(plan->outputs.net_ids);
	lxs_free_aligned(plan->standard_mux_data_net_ids);
	lxs_free_aligned(plan->standard_mux_select_net_ids);
	lxs_free_aligned(plan->standard_mux_output_net_ids);
	lxs_free_aligned(plan->standard_add_input_net_ids);
	lxs_free_aligned(plan->standard_add_output_net_ids);
	lxs_free_aligned(plan->standard_cmp_input_net_ids);
	lxs_free_aligned(plan->standard_cmp_output_net_ids);
	lxs_free_aligned(plan->standard_alu_input_net_ids);
	lxs_free_aligned(plan->standard_alu_output_net_ids);
	lxs_free_aligned(plan->state.d_inputs);
	lxs_free_aligned(plan->state.q_outputs);
	lxs_free_aligned(plan->registers);
	lxs_free_aligned(plan->register_input_net_ids);
	lxs_free_aligned(plan->register_output_net_ids);
	lxs_free_aligned(plan->register_init_value);
	lxs_free_aligned(plan->register_init_mask);
	lxs_free_aligned(plan->roms);
	lxs_free_aligned(plan->rom_addr_net_ids);
	lxs_free_aligned(plan->rom_output_net_ids);
	lxs_free_aligned(plan->rom_init_value);
	lxs_free_aligned(plan->rom_init_mask);
	lxs_free_aligned(plan->rams);
	lxs_free_aligned(plan->ram_read_addr_net_ids);
	lxs_free_aligned(plan->ram_write_addr_net_ids);
	lxs_free_aligned(plan->ram_data_input_net_ids);
	lxs_free_aligned(plan->ram_output_net_ids);
	lxs_free_aligned(plan->ram_init_value);
	lxs_free_aligned(plan->ram_init_mask);
	lxs_free_aligned(plan->regfiles);
	lxs_free_aligned(plan->regfile_read_a_addr_net_ids);
	lxs_free_aligned(plan->regfile_read_b_addr_net_ids);
	lxs_free_aligned(plan->regfile_write_addr_net_ids);
	lxs_free_aligned(plan->regfile_data_input_net_ids);
	lxs_free_aligned(plan->regfile_output_net_ids);
	lxs_free_aligned(plan->regfile_init_value);
	lxs_free_aligned(plan->regfile_init_mask);
	lxs_free_aligned(plan);
	}
