#ifndef LXS_TYPES_H
#define LXS_TYPES_H

#include <stdint.h>

#ifndef LXS_TEST_PROBES
#define LXS_TEST_PROBES 0
#endif

typedef enum lxs_gate_type
	{
	LXS_GATE_AND = 0,
	LXS_GATE_OR,
	LXS_GATE_XOR,
	LXS_GATE_TRI,
	LXS_GATE_NOT,
	LXS_GATE_NAND,
	LXS_GATE_NOR,
	LXS_GATE_XNOR,
	LXS_GATE_BUF,
	LXS_GATE_DFF,
	LXS_GATE_TYPE_COUNT
	} lxs_gate_type;

typedef enum lxs_macro_type
	{
	LXS_MACRO_MUX2 = 0,
	LXS_MACRO_XOR2,
	LXS_MACRO_XNOR2,
	LXS_MACRO_CARRY_INV2,
	LXS_MACRO_SUM_CINV2,
	LXS_MACRO_TYPE_COUNT
	} lxs_macro_type;

typedef struct lxs_gate_ir lxs_gate_ir;
struct lxs_gate_ir
	{
	uint32_t type;
	uint32_t input_count;
	uint32_t inputs[2];
	uint32_t output;
	uint32_t level;
	};

typedef struct lxs_netlist lxs_netlist;
struct lxs_netlist
	{
	char **net_names;
	uint32_t net_count;
	uint32_t net_cap;

	uint32_t *inputs;
	uint32_t input_count;
	uint32_t input_cap;

	uint32_t *outputs;
	uint32_t output_count;
	uint32_t output_cap;

	lxs_gate_ir *gates;
	uint32_t gate_count;
	uint32_t gate_cap;
	};

typedef struct lxs_chunk_plan lxs_chunk_plan;
struct lxs_chunk_plan
	{
	uint32_t level;
	uint32_t type;
	uint32_t start;
	uint32_t count;
	};

typedef struct lxs_level_plan lxs_level_plan;
struct lxs_level_plan
	{
	uint32_t chunk_start;
	uint32_t chunk_count;
	uint32_t macro_start;
	uint32_t macro_count;
	};

typedef struct lxs_macro_plan lxs_macro_plan;
struct lxs_macro_plan
	{
	uint32_t type;
	uint32_t level;
	uint32_t inputs[3];
	uint32_t output;
	uint32_t gate_equiv_count;
	};

typedef struct lxs_io_plan lxs_io_plan;
struct lxs_io_plan
	{
	uint32_t count;
	uint32_t *net_ids;
	uint32_t contiguous_base;
	uint8_t is_contiguous;
	};

typedef struct lxs_state_plan lxs_state_plan;
struct lxs_state_plan
	{
	uint32_t count;
	uint32_t *d_inputs;
	uint32_t *q_outputs;
	uint32_t d_contiguous_base;
	uint32_t q_contiguous_base;
	uint8_t d_is_contiguous;
	uint8_t q_is_contiguous;
	};

typedef struct lxs_plan lxs_plan;
struct lxs_plan
	{
	uint32_t net_count;
	uint32_t gate_count;
	uint32_t comb_gate_count;
	uint32_t level_count;
	uint32_t span_count;
	uint32_t max_span_count;
	uint32_t macro_count;

	lxs_gate_ir *comb_gates;
	lxs_chunk_plan *chunks;
	lxs_macro_plan *macros;
	lxs_level_plan *levels;

	lxs_io_plan inputs;
	lxs_io_plan outputs;
	lxs_state_plan state;
	};

typedef struct lxs_probes lxs_probes;
struct lxs_probes
	{
	uint64_t input_apply;
	uint64_t chunk_exec;
	uint64_t gate_eval;
	uint64_t dff_exec;
	uint64_t tick_count;
	uint64_t state_commit_count;

#if LXS_TEST_PROBES
	/* TEST_ONLY semantic diagnostics. Remove when no longer asserted by tests. */
	uint64_t input_toggle;
	uint64_t state_change_commit;
	uint64_t contention_count;
	uint64_t unknown_state_materialize_count;
	uint64_t highz_materialize_count;
	uint64_t multi_driver_resolve_count;
	uint64_t tri_no_drive_count;
	uint64_t pup_z_source_count;
	uint64_t pdn_z_source_count;
#endif
	};

typedef struct lxs_engine_ctx lxs_engine_ctx;
struct lxs_engine_ctx
	{
	uint64_t *net_value;
	uint64_t *net_mask;
	uint64_t *next_state_value;
	uint64_t *next_state_mask;
	uint64_t *output_value;
	uint64_t *output_mask;

#if LXS_TEST_PROBES
	uint64_t *input_shadow_value;
	uint64_t *input_shadow_mask;
#endif

	lxs_probes probes;
	};

lxs_netlist* lxs_load_iscas(const char *path);
void lxs_free_netlist(lxs_netlist *nl);

lxs_plan* lxs_compile_to_plan(lxs_netlist *nl);
void lxs_free_plan(lxs_plan *plan);

int lxs_init_engine(lxs_engine_ctx *ctx, const lxs_plan *plan);
void lxs_reset_engine(lxs_engine_ctx *ctx, const lxs_plan *plan);
void lxs_free_engine(lxs_engine_ctx *ctx);

void lxs_apply_inputs(
	lxs_engine_ctx *ctx,
	const lxs_plan *plan,
	const uint64_t *values,
	const uint64_t *masks);

void lxs_begin_tick(lxs_engine_ctx *ctx);
void lxs_execute_levels(lxs_engine_ctx *ctx, const lxs_plan *plan);
void lxs_capture_outputs(lxs_engine_ctx *ctx, const lxs_plan *plan);
void lxs_capture_next_state(lxs_engine_ctx *ctx, const lxs_plan *plan);
void lxs_commit_state(lxs_engine_ctx *ctx, const lxs_plan *plan);
void lxs_execute_plan(lxs_engine_ctx *ctx, const lxs_plan *plan);

void lxs_read_outputs(
	const lxs_engine_ctx *ctx,
	const lxs_plan *plan,
	uint64_t *values,
	uint64_t *masks);

lxs_probes lxs_get_probes(const lxs_engine_ctx *ctx);

#endif
