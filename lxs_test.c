#include "lxs_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct lxs_loaded_case
	{
	lxs_netlist *nl;
	lxs_plan *plan;
	lxs_engine_ctx ctx;
	} lxs_loaded_case;

static int lxs_load_case(const char *path, lxs_loaded_case *loaded)
	{
	memset(loaded, 0, sizeof(*loaded));
	loaded->nl = lxs_load_iscas(path);
	if (!loaded->nl)
		{
		return 0;
		}

	loaded->plan = lxs_compile_to_plan(loaded->nl);
	if (!loaded->plan)
		{
		lxs_free_netlist(loaded->nl);
		memset(loaded, 0, sizeof(*loaded));
		return 0;
		}

	if (!lxs_init_engine(&loaded->ctx, loaded->plan))
		{
		lxs_free_plan(loaded->plan);
		lxs_free_netlist(loaded->nl);
		memset(loaded, 0, sizeof(*loaded));
		return 0;
		}

	return 1;
	}

static void lxs_unload_case(lxs_loaded_case *loaded)
	{
	lxs_free_engine(&loaded->ctx);
	lxs_free_plan(loaded->plan);
	lxs_free_netlist(loaded->nl);
	memset(loaded, 0, sizeof(*loaded));
	}

static int lxs_expect_u64(const char *label, uint64_t actual, uint64_t expected)
	{
	if (actual != expected)
		{
		fprintf(stderr, "FAIL %s: expected %llu, got %llu\n",
			label,
			(unsigned long long)expected,
			(unsigned long long)actual);
		return 0;
		}
	return 1;
	}

static int lxs_test_multi_macro_full_adder_cinv(void)
	{
	lxs_plan plan;
	lxs_level_plan level;
	lxs_multi_macro_plan macro;
	lxs_engine_ctx ctx;
	uint64_t values[3];
	uint64_t masks[3];
	uint64_t out_values[2];
	uint64_t out_masks[2];
	int ok = 1;

	memset(&plan, 0, sizeof(plan));
	memset(&level, 0, sizeof(level));
	memset(&macro, 0, sizeof(macro));

	plan.net_count = 5U;
	plan.level_count = 1U;
	plan.multi_macro_count = 1U;
	plan.inputs.count = 3U;
	plan.outputs.count = 2U;
	plan.levels = &level;
	plan.multi_macros = &macro;
	plan.inputs.net_ids = (uint32_t*)calloc(3U, sizeof(uint32_t));
	plan.outputs.net_ids = (uint32_t*)calloc(2U, sizeof(uint32_t));
	if (!plan.inputs.net_ids || !plan.outputs.net_ids)
		{
		free(plan.inputs.net_ids);
		free(plan.outputs.net_ids);
		fprintf(stderr, "FAIL multi_macro_full_adder_cinv: allocation failure\n");
		return 0;
		}

	plan.inputs.net_ids[0] = 0U;
	plan.inputs.net_ids[1] = 1U;
	plan.inputs.net_ids[2] = 2U;
	plan.outputs.net_ids[0] = 3U;
	plan.outputs.net_ids[1] = 4U;
	level.multi_macro_start = 0U;
	level.multi_macro_count = 1U;

	macro.type = LXS_MULTI_MACRO_FULL_ADDER_CINV;
	macro.level = 0U;
	macro.inputs[0] = 0U;
	macro.inputs[1] = 1U;
	macro.inputs[2] = 2U;
	macro.outputs[0] = 3U;
	macro.outputs[1] = 4U;
	macro.input_count = 3U;
	macro.output_count = 2U;
	macro.gate_equiv_count = 6U;

	memset(&ctx, 0, sizeof(ctx));
	if (!lxs_init_engine(&ctx, &plan))
		{
		free(plan.inputs.net_ids);
		free(plan.outputs.net_ids);
		fprintf(stderr, "FAIL multi_macro_full_adder_cinv: engine init failure\n");
		return 0;
		}

	for (uint32_t combo = 0; combo < 8U; ++combo)
		{
		uint32_t a = (combo >> 2U) & 1U;
		uint32_t b = (combo >> 1U) & 1U;
		uint32_t cin_n = combo & 1U;
		uint32_t cin = cin_n ? 0U : 1U;
		uint32_t sum = a ^ b ^ cin;
		uint32_t cout = (a & b) | (a & cin) | (b & cin);
		char label[64];

		values[0] = a ? ~0ULL : 0ULL;
		values[1] = b ? ~0ULL : 0ULL;
		values[2] = cin_n ? ~0ULL : 0ULL;
		masks[0] = 0ULL;
		masks[1] = 0ULL;
		masks[2] = 0ULL;

		lxs_apply_inputs(&ctx, &plan, values, masks);
		lxs_execute_plan(&ctx, &plan);
		lxs_read_outputs(&ctx, &plan, out_values, out_masks);

		snprintf(label, sizeof(label), "multi_macro_full_adder_cinv.sum.%u", combo);
		ok &= lxs_expect_u64(label, out_values[0], sum ? ~0ULL : 0ULL);
		snprintf(label, sizeof(label), "multi_macro_full_adder_cinv.cout_n.%u", combo);
		ok &= lxs_expect_u64(label, out_values[1], cout ? 0ULL : ~0ULL);
		}

	ok &= lxs_expect_u64("multi_macro_full_adder_cinv.chunk_exec", ctx.probes.chunk_exec, 8ULL);
	ok &= lxs_expect_u64("multi_macro_full_adder_cinv.gate_eval", ctx.probes.gate_eval, 48ULL);

	lxs_free_engine(&ctx);
	free(plan.inputs.net_ids);
	free(plan.outputs.net_ids);
	return ok;
	}

static int lxs_test_comb_chain(void)
	{
	lxs_loaded_case loaded;
	uint64_t values[3];
	uint64_t masks[3];
	uint64_t out_values[1];
	uint64_t out_masks[1];
	int ok = 1;

	if (!lxs_load_case("Tests\\Circuits\\comb_chain.bench", &loaded))
		{
		fprintf(stderr, "FAIL comb_chain: unable to load test circuit\n");
		return 0;
		}

	values[0] = ~0ULL;
	values[1] = ~0ULL;
	values[2] = 0ULL;
	masks[0] = 0ULL;
	masks[1] = 0ULL;
	masks[2] = 0ULL;
	lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
	ok &= lxs_expect_u64("comb_chain.y.high", out_values[0], ~0ULL);
	ok &= lxs_expect_u64("comb_chain.y.mask.high", out_masks[0], 0ULL);

	values[0] = 0ULL;
	values[1] = ~0ULL;
	values[2] = 0ULL;
	lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
	ok &= lxs_expect_u64("comb_chain.y.low", out_values[0], 0ULL);
	ok &= lxs_expect_u64("comb_chain.y.mask.low", out_masks[0], 0ULL);

	lxs_unload_case(&loaded);
	return ok;
	}

static int lxs_test_mask_and(void)
	{
	lxs_loaded_case loaded;
	uint64_t values[2];
	uint64_t masks[2];
	uint64_t out_values[1];
	uint64_t out_masks[1];
	int ok = 1;

	if (!lxs_load_case("Tests\\Circuits\\mask_and.bench", &loaded))
		{
		fprintf(stderr, "FAIL mask_and: unable to load test circuit\n");
		return 0;
		}

	values[0] = 0ULL;
	values[1] = ~0ULL;
	masks[0] = ~0ULL;
	masks[1] = 0ULL;
	lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
	ok &= lxs_expect_u64("mask_and.y.value", out_values[0], 0ULL);
	ok &= lxs_expect_u64("mask_and.y.mask", out_masks[0], ~0ULL);

	lxs_unload_case(&loaded);
	return ok;
	}

static int lxs_test_blif_basic(void)
	{
	lxs_loaded_case loaded;
	uint64_t values[2];
	uint64_t masks[2];
	uint64_t out_values[3];
	uint64_t out_masks[3];
	int ok = 1;

	if (!lxs_load_case("Tests\\Circuits\\blif_basic.bench", &loaded))
		{
		fprintf(stderr, "FAIL blif_basic: unable to load test circuit\n");
		return 0;
		}

	values[0] = ~0ULL;
	values[1] = 0ULL;
	masks[0] = 0ULL;
	masks[1] = 0ULL;
	lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
	ok &= lxs_expect_u64("blif_basic.y_or.high", out_values[0], ~0ULL);
	ok &= lxs_expect_u64("blif_basic.y_or.mask.high", out_masks[0], 0ULL);
	ok &= lxs_expect_u64("blif_basic.y_sel.high", out_values[1], ~0ULL);
	ok &= lxs_expect_u64("blif_basic.y_sel.mask.high", out_masks[1], 0ULL);
	ok &= lxs_expect_u64("blif_basic.y_const1.high", out_values[2], ~0ULL);
	ok &= lxs_expect_u64("blif_basic.y_const1.mask.high", out_masks[2], 0ULL);

	values[0] = 0ULL;
	values[1] = ~0ULL;
	lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
	ok &= lxs_expect_u64("blif_basic.y_or.low", out_values[0], ~0ULL);
	ok &= lxs_expect_u64("blif_basic.y_sel.low", out_values[1], 0ULL);
	ok &= lxs_expect_u64("blif_basic.y_const1.low", out_values[2], ~0ULL);

	lxs_unload_case(&loaded);
	return ok;
	}

static int lxs_test_mux2_macro(void)
	{
	lxs_loaded_case loaded;
	uint64_t values[3];
	uint64_t masks[3];
	uint64_t out_values[1];
	uint64_t out_masks[1];
	int ok = 1;

	if (!lxs_load_case("Tests\\Circuits\\mux2_macro.bench", &loaded))
		{
		fprintf(stderr, "FAIL mux2_macro: unable to load test circuit\n");
		return 0;
		}

	ok &= lxs_expect_u64("mux2_macro.macro_count", loaded.plan->macro_count, 1ULL);
	ok &= lxs_expect_u64("mux2_macro.comb_gate_count", loaded.plan->comb_gate_count, 0ULL);

	values[0] = ~0ULL;
	values[1] = 0ULL;
	values[2] = 0ULL;
	masks[0] = 0ULL;
	masks[1] = 0ULL;
	masks[2] = 0ULL;
	lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
	ok &= lxs_expect_u64("mux2_macro.select0", out_values[0], ~0ULL);
	ok &= lxs_expect_u64("mux2_macro.select0.mask", out_masks[0], 0ULL);

	values[2] = ~0ULL;
	lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
	ok &= lxs_expect_u64("mux2_macro.select1", out_values[0], 0ULL);
	ok &= lxs_expect_u64("mux2_macro.select1.mask", out_masks[0], 0ULL);

	lxs_unload_case(&loaded);
	return ok;
	}

static int lxs_test_xor2_macro(void)
	{
	lxs_loaded_case loaded;
	uint64_t values[2];
	uint64_t masks[2];
	uint64_t out_values[1];
	uint64_t out_masks[1];
	int ok = 1;

	if (!lxs_load_case("Tests\\Circuits\\xor2_macro.bench", &loaded))
		{
		fprintf(stderr, "FAIL xor2_macro: unable to load test circuit\n");
		return 0;
		}

	ok &= lxs_expect_u64("xor2_macro.macro_count", loaded.plan->macro_count, 1ULL);
	ok &= lxs_expect_u64("xor2_macro.comb_gate_count", loaded.plan->comb_gate_count, 0ULL);

	values[0] = 0ULL;
	values[1] = 0ULL;
	masks[0] = 0ULL;
	masks[1] = 0ULL;
	lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
	ok &= lxs_expect_u64("xor2_macro.00", out_values[0], 0ULL);
	ok &= lxs_expect_u64("xor2_macro.00.mask", out_masks[0], 0ULL);

	values[0] = ~0ULL;
	values[1] = 0ULL;
	lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
	ok &= lxs_expect_u64("xor2_macro.10", out_values[0], ~0ULL);

	values[0] = ~0ULL;
	values[1] = ~0ULL;
	lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
	ok &= lxs_expect_u64("xor2_macro.11", out_values[0], 0ULL);

	lxs_unload_case(&loaded);
	return ok;
	}

static int lxs_test_xor2_nor_macro(void)
	{
	lxs_loaded_case loaded;
	uint64_t values[2];
	uint64_t masks[2];
	uint64_t out_values[1];
	uint64_t out_masks[1];
	int ok = 1;

	if (!lxs_load_case("Tests\\Circuits\\xor2_nor_macro.bench", &loaded))
		{
		fprintf(stderr, "FAIL xor2_nor_macro: unable to load test circuit\n");
		return 0;
		}

	ok &= lxs_expect_u64("xor2_nor_macro.macro_count", loaded.plan->macro_count, 1ULL);
	ok &= lxs_expect_u64("xor2_nor_macro.comb_gate_count", loaded.plan->comb_gate_count, 0ULL);

	values[0] = 0ULL;
	values[1] = 0ULL;
	masks[0] = 0ULL;
	masks[1] = 0ULL;
	lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
	ok &= lxs_expect_u64("xor2_nor_macro.00", out_values[0], 0ULL);

	values[0] = ~0ULL;
	values[1] = 0ULL;
	lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
	ok &= lxs_expect_u64("xor2_nor_macro.10", out_values[0], ~0ULL);

	values[0] = ~0ULL;
	values[1] = ~0ULL;
	lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
	ok &= lxs_expect_u64("xor2_nor_macro.11", out_values[0], 0ULL);

	lxs_unload_case(&loaded);
	return ok;
	}

static int lxs_test_xnor2_macro(void)
	{
	lxs_loaded_case loaded;
	uint64_t values[2];
	uint64_t masks[2];
	uint64_t out_values[1];
	uint64_t out_masks[1];
	int ok = 1;

	if (!lxs_load_case("Tests\\Circuits\\xnor2_macro.bench", &loaded))
		{
		fprintf(stderr, "FAIL xnor2_macro: unable to load test circuit\n");
		return 0;
		}

	ok &= lxs_expect_u64("xnor2_macro.macro_count", loaded.plan->macro_count, 1ULL);
	ok &= lxs_expect_u64("xnor2_macro.comb_gate_count", loaded.plan->comb_gate_count, 0ULL);

	values[0] = 0ULL;
	values[1] = 0ULL;
	masks[0] = 0ULL;
	masks[1] = 0ULL;
	lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
	ok &= lxs_expect_u64("xnor2_macro.00", out_values[0], ~0ULL);
	ok &= lxs_expect_u64("xnor2_macro.00.mask", out_masks[0], 0ULL);

	values[0] = ~0ULL;
	values[1] = 0ULL;
	lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
	ok &= lxs_expect_u64("xnor2_macro.10", out_values[0], 0ULL);

	values[0] = ~0ULL;
	values[1] = ~0ULL;
	lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
	ok &= lxs_expect_u64("xnor2_macro.11", out_values[0], ~0ULL);

	lxs_unload_case(&loaded);
	return ok;
	}

static int lxs_test_carry_inv2_macro(void)
	{
	lxs_loaded_case loaded;
	uint64_t values[3];
	uint64_t masks[3];
	uint64_t out_values[1];
	uint64_t out_masks[1];
	int ok = 1;

	if (!lxs_load_case("Tests\\Circuits\\carry_inv2_macro.bench", &loaded))
		{
		fprintf(stderr, "FAIL carry_inv2_macro: unable to load test circuit\n");
		return 0;
		}

	ok &= lxs_expect_u64("carry_inv2_macro.macro_count", loaded.plan->macro_count, 1ULL);
	ok &= lxs_expect_u64("carry_inv2_macro.comb_gate_count", loaded.plan->comb_gate_count, 2ULL);

	values[0] = 0ULL;
	values[1] = 0ULL;
	values[2] = ~0ULL;
	masks[0] = 0ULL;
	masks[1] = 0ULL;
	masks[2] = 0ULL;
	lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
	ok &= lxs_expect_u64("carry_inv2_macro.001", out_values[0], ~0ULL);
	ok &= lxs_expect_u64("carry_inv2_macro.001.mask", out_masks[0], 0ULL);

	values[0] = ~0ULL;
	values[1] = 0ULL;
	values[2] = ~0ULL;
	lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
	ok &= lxs_expect_u64("carry_inv2_macro.101", out_values[0], ~0ULL);

	values[0] = ~0ULL;
	values[1] = 0ULL;
	values[2] = 0ULL;
	lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
	ok &= lxs_expect_u64("carry_inv2_macro.100", out_values[0], 0ULL);

	values[0] = ~0ULL;
	values[1] = ~0ULL;
	values[2] = ~0ULL;
	lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
	ok &= lxs_expect_u64("carry_inv2_macro.111", out_values[0], 0ULL);

	lxs_unload_case(&loaded);
	return ok;
	}

static int lxs_test_sum_cinv2_macro(void)
	{
	lxs_loaded_case loaded;
	uint64_t values[3];
	uint64_t masks[3];
	uint64_t out_values[1];
	uint64_t out_masks[1];
	int ok = 1;

	if (!lxs_load_case("Tests\\Circuits\\sum_cinv2_macro.bench", &loaded))
		{
		fprintf(stderr, "FAIL sum_cinv2_macro: unable to load test circuit\n");
		return 0;
		}

	ok &= lxs_expect_u64("sum_cinv2_macro.macro_count", loaded.plan->macro_count, 1ULL);
	ok &= lxs_expect_u64("sum_cinv2_macro.comb_gate_count", loaded.plan->comb_gate_count, 2ULL);

	for (uint32_t combo = 0; combo < 8U; ++combo)
		{
		uint32_t a = (combo >> 2U) & 1U;
		uint32_t b = (combo >> 1U) & 1U;
		uint32_t cin_n = combo & 1U;
		uint32_t cin = cin_n ? 0U : 1U;
		uint32_t sum = a ^ b ^ cin;
		char label[64];

		values[0] = a ? ~0ULL : 0ULL;
		values[1] = b ? ~0ULL : 0ULL;
		values[2] = cin_n ? ~0ULL : 0ULL;
		masks[0] = 0ULL;
		masks[1] = 0ULL;
		masks[2] = 0ULL;

		lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
		lxs_execute_plan(&loaded.ctx, loaded.plan);
		lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);

		snprintf(label, sizeof(label), "sum_cinv2_macro.sum.%u", combo);
		ok &= lxs_expect_u64(label, out_values[0], sum ? ~0ULL : 0ULL);
		snprintf(label, sizeof(label), "sum_cinv2_macro.sum.mask.%u", combo);
		ok &= lxs_expect_u64(label, out_masks[0], 0ULL);
		}

	lxs_unload_case(&loaded);
	return ok;
	}

static int lxs_test_ripple_slice2_macro(void)
	{
	lxs_loaded_case loaded;
	uint64_t values[5];
	uint64_t masks[5];
	uint64_t out_values[3];
	uint64_t out_masks[3];
	int ok = 1;

	if (!lxs_load_case("Tests\\Circuits\\ripple_slice2_macro.bench", &loaded))
		{
		fprintf(stderr, "FAIL ripple_slice2_macro: unable to load test circuit\n");
		return 0;
		}

	ok &= lxs_expect_u64("ripple_slice2_macro.macro_count", loaded.plan->macro_count, 0ULL);
	ok &= lxs_expect_u64("ripple_slice2_macro.multi_macro_count", loaded.plan->multi_macro_count, 1ULL);
	ok &= lxs_expect_u64("ripple_slice2_macro.comb_gate_count", loaded.plan->comb_gate_count, 0ULL);

	for (uint32_t combo = 0; combo < 32U; ++combo)
		{
		uint32_t a0 = (combo >> 4U) & 1U;
		uint32_t b0 = (combo >> 3U) & 1U;
		uint32_t a1 = (combo >> 2U) & 1U;
		uint32_t b1 = (combo >> 1U) & 1U;
		uint32_t cin_n = combo & 1U;
		uint32_t cin = cin_n ? 0U : 1U;
		uint32_t total = (a0 + (a1 << 1U)) + (b0 + (b1 << 1U)) + cin;
		char label[80];

		values[0] = a0 ? ~0ULL : 0ULL;
		values[1] = b0 ? ~0ULL : 0ULL;
		values[2] = a1 ? ~0ULL : 0ULL;
		values[3] = b1 ? ~0ULL : 0ULL;
		values[4] = cin_n ? ~0ULL : 0ULL;
		memset(masks, 0, sizeof(masks));

		lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
		lxs_execute_plan(&loaded.ctx, loaded.plan);
		lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);

		snprintf(label, sizeof(label), "ripple_slice2_macro.sum0.%u", combo);
		ok &= lxs_expect_u64(label, out_values[0], (total & 1U) ? ~0ULL : 0ULL);
		snprintf(label, sizeof(label), "ripple_slice2_macro.sum1.%u", combo);
		ok &= lxs_expect_u64(label, out_values[1], (total & 2U) ? ~0ULL : 0ULL);
		snprintf(label, sizeof(label), "ripple_slice2_macro.cout_n.%u", combo);
		ok &= lxs_expect_u64(label, out_values[2], (total & 4U) ? 0ULL : ~0ULL);
		snprintf(label, sizeof(label), "ripple_slice2_macro.sum0.mask.%u", combo);
		ok &= lxs_expect_u64(label, out_masks[0], 0ULL);
		snprintf(label, sizeof(label), "ripple_slice2_macro.sum1.mask.%u", combo);
		ok &= lxs_expect_u64(label, out_masks[1], 0ULL);
		snprintf(label, sizeof(label), "ripple_slice2_macro.cout_n.mask.%u", combo);
		ok &= lxs_expect_u64(label, out_masks[2], 0ULL);
		}

	lxs_unload_case(&loaded);
	return ok;
	}

static int lxs_test_dff_not(void)
	{
	lxs_loaded_case loaded;
	uint64_t values[1];
	uint64_t masks[1];
	uint64_t out_values[2];
	uint64_t out_masks[2];
	lxs_probes probes;
	int ok = 1;

	if (!lxs_load_case("Tests\\Circuits\\dff_not.bench", &loaded))
		{
		fprintf(stderr, "FAIL dff_not: unable to load test circuit\n");
		return 0;
		}

	values[0] = ~0ULL;
	masks[0] = 0ULL;
	lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
	ok &= lxs_expect_u64("dff_not.q.cycle1", out_values[0], 0ULL);
	ok &= lxs_expect_u64("dff_not.nq.cycle1", out_values[1], ~0ULL);
	ok &= lxs_expect_u64("dff_not.q.mask.cycle1", out_masks[0], 0ULL);
	ok &= lxs_expect_u64("dff_not.nq.mask.cycle1", out_masks[1], 0ULL);

	values[0] = 0ULL;
	lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
	ok &= lxs_expect_u64("dff_not.q.cycle2", out_values[0], ~0ULL);
	ok &= lxs_expect_u64("dff_not.nq.cycle2", out_values[1], 0ULL);

	values[0] = 0ULL;
	lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
	ok &= lxs_expect_u64("dff_not.q.cycle3", out_values[0], 0ULL);
	ok &= lxs_expect_u64("dff_not.nq.cycle3", out_values[1], ~0ULL);

	probes = lxs_get_probes(&loaded.ctx);
	ok &= lxs_expect_u64("dff_not.tick_count", probes.tick_count, 3ULL);
	ok &= lxs_expect_u64("dff_not.dff_exec", probes.dff_exec, 3ULL);
	ok &= lxs_expect_u64("dff_not.state_commit_count", probes.state_commit_count, 3ULL);
	ok &= lxs_expect_u64("dff_not.input_apply", probes.input_apply, 3ULL);

#if LXS_TEST_PROBES
	ok &= lxs_expect_u64("dff_not.input_toggle", probes.input_toggle, 2ULL);
	ok &= lxs_expect_u64("dff_not.state_change_commit", probes.state_change_commit, 2ULL);
#endif

	lxs_unload_case(&loaded);
	return ok;
	}

int main(void)
	{
	int ok = 1;

	ok &= lxs_test_comb_chain();
	ok &= lxs_test_mask_and();
	ok &= lxs_test_blif_basic();
	ok &= lxs_test_multi_macro_full_adder_cinv();
	ok &= lxs_test_mux2_macro();
	ok &= lxs_test_xor2_macro();
	ok &= lxs_test_xor2_nor_macro();
	ok &= lxs_test_xnor2_macro();
	ok &= lxs_test_carry_inv2_macro();
	ok &= lxs_test_sum_cinv2_macro();
	ok &= lxs_test_ripple_slice2_macro();
	ok &= lxs_test_dff_not();

	if (!ok)
		{
		fprintf(stderr, "LXS tests failed. Check combinational binding, 4-state propagation, or DFF commit semantics.\n");
		return 1;
		}

	printf("LXS tests passed.\n");
	return 0;
	}
