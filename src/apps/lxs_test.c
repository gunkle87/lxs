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

static int lxs_set_env_var(const char *name, const char *value)
	{
#ifdef _WIN32
	return _putenv_s(name, value ? value : "") == 0;
#else
	if (!value)
		{
		return unsetenv(name) == 0;
		}
	return setenv(name, value, 1) == 0;
#endif
	}

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

static void lxs_set_scalar_bits(
	uint64_t *values,
	uint64_t *masks,
	uint32_t width,
	uint64_t scalar)
	{
	for (uint32_t bit = 0; bit < width; ++bit)
		{
		values[bit] = ((scalar >> bit) & 1ULL) ? ~0ULL : 0ULL;
		if (masks)
			{
			masks[bit] = 0ULL;
			}
		}
	}

static uint64_t lxs_collect_scalar_bits(
	const uint64_t *values,
	const uint64_t *masks,
	uint32_t width)
	{
	uint64_t scalar = 0ULL;

	for (uint32_t bit = 0; bit < width; ++bit)
		{
		if (masks && masks[bit] != 0ULL)
			{
			continue;
			}
		if (values[bit] != 0ULL)
			{
			scalar |= (1ULL << bit);
			}
		}

	return scalar;
	}

static uint64_t lxs_next_rand(uint64_t *state)
	{
	*state = (*state * 6364136223846793005ULL) + 1ULL;
	return *state;
	}

static int lxs_test_standard_mux_equivalence(
	const char *primitive_path,
	const char *explicit_path,
	const char *label_prefix,
	uint32_t input_count,
	uint32_t output_count,
	uint32_t expected_kind,
	uint8_t use_random_masks)
	{
	lxs_loaded_case lhs;
	lxs_loaded_case rhs;
	uint64_t values[40];
	uint64_t masks[40];
	uint64_t lhs_out_values[8];
	uint64_t lhs_out_masks[8];
	uint64_t rhs_out_values[8];
	uint64_t rhs_out_masks[8];
	uint64_t rng = 0x4D55585FULL;
	int ok = 1;

	if (!lxs_load_case(primitive_path, &lhs))
		{
		fprintf(stderr, "FAIL %s: unable to load primitive circuit\n", label_prefix);
		return 0;
		}

	if (!lxs_load_case(explicit_path, &rhs))
		{
		fprintf(stderr, "FAIL %s: unable to load explicit circuit\n", label_prefix);
		lxs_unload_case(&lhs);
		return 0;
		}

	{
	char label[96];
	snprintf(label, sizeof(label), "%s.count", label_prefix);
	ok &= lxs_expect_u64(label, rhs.plan->standard_mux_count, 1ULL);
	snprintf(label, sizeof(label), "%s.comb_gate_count", label_prefix);
	ok &= lxs_expect_u64(label, rhs.plan->comb_gate_count, 0ULL);
	snprintf(label, sizeof(label), "%s.width_bits", label_prefix);
	ok &= lxs_expect_u64(label, rhs.plan->standard_muxes[0].width_bits, output_count);
	snprintf(label, sizeof(label), "%s.kind", label_prefix);
	ok &= lxs_expect_u64(label, rhs.plan->standard_muxes[0].kind, expected_kind);
	}

	for (uint32_t iter = 0; iter < 256U; ++iter)
		{
		char label[128];

		for (uint32_t i = 0; i < input_count; ++i)
			{
			values[i] = lxs_next_rand(&rng);
			masks[i] = use_random_masks ? lxs_next_rand(&rng) : 0ULL;
			values[i] &= ~masks[i];
			}

		lxs_apply_inputs(&lhs.ctx, lhs.plan, values, masks);
		lxs_execute_plan(&lhs.ctx, lhs.plan);
		lxs_read_outputs(&lhs.ctx, lhs.plan, lhs_out_values, lhs_out_masks);

		lxs_apply_inputs(&rhs.ctx, rhs.plan, values, masks);
		lxs_execute_plan(&rhs.ctx, rhs.plan);
		lxs_read_outputs(&rhs.ctx, rhs.plan, rhs_out_values, rhs_out_masks);

		for (uint32_t out = 0; out < output_count; ++out)
			{
			snprintf(label, sizeof(label), "%s.value.%u.%u", label_prefix, iter, out);
			ok &= lxs_expect_u64(label, rhs_out_values[out], lhs_out_values[out]);
			snprintf(label, sizeof(label), "%s.mask.%u.%u", label_prefix, iter, out);
			ok &= lxs_expect_u64(label, rhs_out_masks[out], lhs_out_masks[out]);
			}
		}

	lxs_unload_case(&lhs);
	lxs_unload_case(&rhs);
	return ok;
	}

static int lxs_test_standard_register_equivalence(
	const char *reference_path,
	const char *explicit_path,
	const char *label_prefix,
	uint32_t width_bits)
	{
	lxs_loaded_case lhs;
	lxs_loaded_case rhs;
	uint64_t values[64];
	uint64_t masks[64];
	uint64_t lhs_out_values[64];
	uint64_t lhs_out_masks[64];
	uint64_t rhs_out_values[64];
	uint64_t rhs_out_masks[64];
	uint64_t rng = 0x5245475F535444ULL;
	int ok = 1;

	if (!lxs_load_case(reference_path, &lhs))
		{
		fprintf(stderr, "FAIL %s: unable to load reference circuit\n", label_prefix);
		return 0;
		}

	if (!lxs_load_case(explicit_path, &rhs))
		{
		fprintf(stderr, "FAIL %s: unable to load explicit circuit\n", label_prefix);
		lxs_unload_case(&lhs);
		return 0;
		}

	ok &= lxs_expect_u64(label_prefix, rhs.plan->register_count, 1ULL);
	ok &= lxs_expect_u64("register.width_bits", rhs.plan->registers[0].width_bits, width_bits);
	ok &= lxs_expect_u64("register.output_count", rhs.plan->outputs.count, width_bits);

	memset(values, 0, sizeof(values));
	memset(masks, 0, sizeof(masks));

	for (uint32_t cycle = 0; cycle < 64U; ++cycle)
		{
		char label[128];

		lxs_set_scalar_bits(values, masks, width_bits, lxs_next_rand(&rng));

		lxs_apply_inputs(&lhs.ctx, lhs.plan, values, masks);
		lxs_execute_plan(&lhs.ctx, lhs.plan);
		lxs_read_outputs(&lhs.ctx, lhs.plan, lhs_out_values, lhs_out_masks);

		lxs_apply_inputs(&rhs.ctx, rhs.plan, values, masks);
		lxs_execute_plan(&rhs.ctx, rhs.plan);
		lxs_read_outputs(&rhs.ctx, rhs.plan, rhs_out_values, rhs_out_masks);

		for (uint32_t bit = 0; bit < width_bits; ++bit)
			{
			snprintf(label, sizeof(label), "%s.value.%u.%u", label_prefix, cycle, bit);
			ok &= lxs_expect_u64(label, rhs_out_values[bit], lhs_out_values[bit]);
			snprintf(label, sizeof(label), "%s.mask.%u.%u", label_prefix, cycle, bit);
			ok &= lxs_expect_u64(label, rhs_out_masks[bit], lhs_out_masks[bit]);
			}
		}

	lxs_unload_case(&lhs);
	lxs_unload_case(&rhs);
	return ok;
	}

static int lxs_test_standard_register_en_equivalence(
	const char *reference_path,
	const char *explicit_path,
	const char *label_prefix,
	uint32_t width_bits)
	{
	lxs_loaded_case lhs;
	lxs_loaded_case rhs;
	uint64_t values[65];
	uint64_t masks[65];
	uint64_t lhs_out_values[64];
	uint64_t lhs_out_masks[64];
	uint64_t rhs_out_values[64];
	uint64_t rhs_out_masks[64];
	uint64_t rng = 0x5245475F454EULL;
	int ok = 1;

	if (!lxs_load_case(reference_path, &lhs))
		{
		fprintf(stderr, "FAIL %s: unable to load reference circuit\n", label_prefix);
		return 0;
		}

	if (!lxs_load_case(explicit_path, &rhs))
		{
		fprintf(stderr, "FAIL %s: unable to load explicit circuit\n", label_prefix);
		lxs_unload_case(&lhs);
		return 0;
		}

	ok &= lxs_expect_u64(label_prefix, rhs.plan->register_count, 1ULL);
	ok &= lxs_expect_u64("register_en.width_bits", rhs.plan->registers[0].width_bits, width_bits);

	memset(values, 0, sizeof(values));
	memset(masks, 0, sizeof(masks));

	for (uint32_t cycle = 0; cycle < 64U; ++cycle)
		{
		char label[128];
		uint64_t enable = (lxs_next_rand(&rng) >> 63U) ? ~0ULL : 0ULL;

		lxs_set_scalar_bits(values, masks, width_bits, lxs_next_rand(&rng));
		values[width_bits] = enable;
		masks[width_bits] = 0ULL;

		lxs_apply_inputs(&lhs.ctx, lhs.plan, values, masks);
		lxs_execute_plan(&lhs.ctx, lhs.plan);
		lxs_read_outputs(&lhs.ctx, lhs.plan, lhs_out_values, lhs_out_masks);

		lxs_apply_inputs(&rhs.ctx, rhs.plan, values, masks);
		lxs_execute_plan(&rhs.ctx, rhs.plan);
		lxs_read_outputs(&rhs.ctx, rhs.plan, rhs_out_values, rhs_out_masks);

		for (uint32_t bit = 0; bit < width_bits; ++bit)
			{
			snprintf(label, sizeof(label), "%s.value.%u.%u", label_prefix, cycle, bit);
			ok &= lxs_expect_u64(label, rhs_out_values[bit], lhs_out_values[bit]);
			snprintf(label, sizeof(label), "%s.mask.%u.%u", label_prefix, cycle, bit);
			ok &= lxs_expect_u64(label, rhs_out_masks[bit], lhs_out_masks[bit]);
			}
		}

	lxs_unload_case(&lhs);
	lxs_unload_case(&rhs);
	return ok;
	}

static int lxs_test_standard_register_en_rst_equivalence(
	const char *reference_path,
	const char *explicit_path,
	const char *label_prefix,
	uint32_t width_bits)
	{
	lxs_loaded_case lhs;
	lxs_loaded_case rhs;
	uint64_t values[66];
	uint64_t masks[66];
	uint64_t lhs_out_values[64];
	uint64_t lhs_out_masks[64];
	uint64_t rhs_out_values[64];
	uint64_t rhs_out_masks[64];
	uint64_t rng = 0x5245475F525354ULL;
	int ok = 1;

	if (!lxs_load_case(reference_path, &lhs))
		{
		fprintf(stderr, "FAIL %s: unable to load reference circuit\n", label_prefix);
		return 0;
		}

	if (!lxs_load_case(explicit_path, &rhs))
		{
		fprintf(stderr, "FAIL %s: unable to load explicit circuit\n", label_prefix);
		lxs_unload_case(&lhs);
		return 0;
		}

	ok &= lxs_expect_u64(label_prefix, rhs.plan->register_count, 1ULL);
	ok &= lxs_expect_u64("register_en_rst.width_bits", rhs.plan->registers[0].width_bits, width_bits);

	memset(values, 0, sizeof(values));
	memset(masks, 0, sizeof(masks));

	for (uint32_t cycle = 0; cycle < 64U; ++cycle)
		{
		char label[128];
		uint64_t enable = (lxs_next_rand(&rng) >> 63U) ? ~0ULL : 0ULL;
		uint64_t reset = (lxs_next_rand(&rng) >> 63U) ? ~0ULL : 0ULL;

		lxs_set_scalar_bits(values, masks, width_bits, lxs_next_rand(&rng));
		values[width_bits] = enable;
		values[width_bits + 1U] = reset;
		masks[width_bits] = 0ULL;
		masks[width_bits + 1U] = 0ULL;

		lxs_apply_inputs(&lhs.ctx, lhs.plan, values, masks);
		lxs_execute_plan(&lhs.ctx, lhs.plan);
		lxs_read_outputs(&lhs.ctx, lhs.plan, lhs_out_values, lhs_out_masks);

		lxs_apply_inputs(&rhs.ctx, rhs.plan, values, masks);
		lxs_execute_plan(&rhs.ctx, rhs.plan);
		lxs_read_outputs(&rhs.ctx, rhs.plan, rhs_out_values, rhs_out_masks);

		for (uint32_t bit = 0; bit < width_bits; ++bit)
			{
			snprintf(label, sizeof(label), "%s.value.%u.%u", label_prefix, cycle, bit);
			ok &= lxs_expect_u64(label, rhs_out_values[bit], lhs_out_values[bit]);
			snprintf(label, sizeof(label), "%s.mask.%u.%u", label_prefix, cycle, bit);
			ok &= lxs_expect_u64(label, rhs_out_masks[bit], lhs_out_masks[bit]);
			}
		}

	lxs_unload_case(&lhs);
	lxs_unload_case(&rhs);
	return ok;
	}

static int lxs_test_standard_add_equivalence(
	const char *reference_path,
	const char *explicit_path,
	const char *label_prefix,
	uint32_t width_bits)
	{
	lxs_loaded_case lhs;
	lxs_loaded_case rhs;
	uint64_t values[129];
	uint64_t masks[129];
	uint64_t lhs_out_values[65];
	uint64_t lhs_out_masks[65];
	uint64_t rhs_out_values[65];
	uint64_t rhs_out_masks[65];
	uint64_t rng = 0x4144445F535444ULL;
	int ok = 1;

	if (!lxs_load_case(reference_path, &lhs))
		{
		fprintf(stderr, "FAIL %s: unable to load reference circuit\n", label_prefix);
		return 0;
		}

	if (!lxs_load_case(explicit_path, &rhs))
		{
		fprintf(stderr, "FAIL %s: unable to load explicit circuit\n", label_prefix);
		lxs_unload_case(&lhs);
		return 0;
		}

	ok &= lxs_expect_u64(label_prefix, rhs.plan->standard_add_count, 1ULL);
	ok &= lxs_expect_u64("standard_add.width_bits", rhs.plan->standard_adders[0].width_bits, width_bits);
	ok &= lxs_expect_u64("standard_add.comb_gate_count", rhs.plan->comb_gate_count, 0ULL);

	memset(values, 0, sizeof(values));
	memset(masks, 0, sizeof(masks));

	for (uint32_t iter = 0; iter < 128U; ++iter)
		{
		char label[128];
		uint64_t carry_in = (lxs_next_rand(&rng) >> 63U) ? ~0ULL : 0ULL;

		lxs_set_scalar_bits(values, masks, width_bits, lxs_next_rand(&rng));
		lxs_set_scalar_bits(values + width_bits, masks + width_bits, width_bits, lxs_next_rand(&rng));
		values[width_bits * 2U] = carry_in;
		masks[width_bits * 2U] = 0ULL;

		lxs_apply_inputs(&lhs.ctx, lhs.plan, values, masks);
		lxs_execute_plan(&lhs.ctx, lhs.plan);
		lxs_read_outputs(&lhs.ctx, lhs.plan, lhs_out_values, lhs_out_masks);

		lxs_apply_inputs(&rhs.ctx, rhs.plan, values, masks);
		lxs_execute_plan(&rhs.ctx, rhs.plan);
		lxs_read_outputs(&rhs.ctx, rhs.plan, rhs_out_values, rhs_out_masks);

		for (uint32_t bit = 0; bit < (width_bits + 1U); ++bit)
			{
			snprintf(label, sizeof(label), "%s.value.%u.%u", label_prefix, iter, bit);
			ok &= lxs_expect_u64(label, rhs_out_values[bit], lhs_out_values[bit]);
			snprintf(label, sizeof(label), "%s.mask.%u.%u", label_prefix, iter, bit);
			ok &= lxs_expect_u64(label, rhs_out_masks[bit], lhs_out_masks[bit]);
			}
		}

	lxs_unload_case(&lhs);
	lxs_unload_case(&rhs);
	return ok;
	}

static int lxs_test_standard_cmp_equivalence(
	const char *reference_path,
	const char *explicit_path,
	const char *label_prefix,
	uint32_t width_bits)
	{
	lxs_loaded_case lhs;
	lxs_loaded_case rhs;
	uint64_t values[128];
	uint64_t masks[128];
	uint64_t lhs_out_values[3];
	uint64_t lhs_out_masks[3];
	uint64_t rhs_out_values[3];
	uint64_t rhs_out_masks[3];
	uint64_t rng = 0x434D505F535444ULL;
	int ok = 1;

	if (!lxs_load_case(reference_path, &lhs))
		{
		fprintf(stderr, "FAIL %s: unable to load reference circuit\n", label_prefix);
		return 0;
		}

	if (!lxs_load_case(explicit_path, &rhs))
		{
		fprintf(stderr, "FAIL %s: unable to load explicit circuit\n", label_prefix);
		lxs_unload_case(&lhs);
		return 0;
		}

	ok &= lxs_expect_u64(label_prefix, rhs.plan->standard_cmp_count, 1ULL);
	ok &= lxs_expect_u64("standard_cmp.width_bits", rhs.plan->standard_cmps[0].width_bits, width_bits);
	ok &= lxs_expect_u64("standard_cmp.comb_gate_count", rhs.plan->comb_gate_count, 0ULL);

	memset(values, 0, sizeof(values));
	memset(masks, 0, sizeof(masks));

	for (uint32_t iter = 0; iter < 128U; ++iter)
		{
		char label[128];

		for (uint32_t bit = 0; bit < width_bits; ++bit)
			{
			values[bit] = lxs_next_rand(&rng);
			masks[bit] = 0ULL;
			values[width_bits + bit] = lxs_next_rand(&rng);
			masks[width_bits + bit] = 0ULL;
			}

		lxs_apply_inputs(&lhs.ctx, lhs.plan, values, masks);
		lxs_execute_plan(&lhs.ctx, lhs.plan);
		lxs_read_outputs(&lhs.ctx, lhs.plan, lhs_out_values, lhs_out_masks);

		lxs_apply_inputs(&rhs.ctx, rhs.plan, values, masks);
		lxs_execute_plan(&rhs.ctx, rhs.plan);
		lxs_read_outputs(&rhs.ctx, rhs.plan, rhs_out_values, rhs_out_masks);

		for (uint32_t out = 0; out < 3U; ++out)
			{
			snprintf(label, sizeof(label), "%s.value.%u.%u", label_prefix, iter, out);
			ok &= lxs_expect_u64(label, rhs_out_values[out], lhs_out_values[out]);
			snprintf(label, sizeof(label), "%s.mask.%u.%u", label_prefix, iter, out);
			ok &= lxs_expect_u64(label, rhs_out_masks[out], lhs_out_masks[out]);
			}
		}

	lxs_unload_case(&lhs);
	lxs_unload_case(&rhs);
	return ok;
	}

static int lxs_test_standard_alu_equivalence(
	const char *reference_path,
	const char *explicit_path,
	const char *label_prefix,
	uint32_t width_bits)
	{
	lxs_loaded_case lhs;
	lxs_loaded_case rhs;
	uint64_t values[131];
	uint64_t masks[131];
	uint64_t lhs_out_values[68];
	uint64_t lhs_out_masks[68];
	uint64_t rhs_out_values[68];
	uint64_t rhs_out_masks[68];
	uint64_t rng = 0x414C555F535444ULL;
	int ok = 1;

	if (!lxs_load_case(reference_path, &lhs))
		{
		fprintf(stderr, "FAIL %s: unable to load reference circuit\n", label_prefix);
		return 0;
		}

	if (!lxs_load_case(explicit_path, &rhs))
		{
		fprintf(stderr, "FAIL %s: unable to load explicit circuit\n", label_prefix);
		lxs_unload_case(&lhs);
		return 0;
		}

	ok &= lxs_expect_u64(label_prefix, rhs.plan->standard_alu_count, 1ULL);
	ok &= lxs_expect_u64("standard_alu.width_bits", rhs.plan->standard_alus[0].width_bits, width_bits);
	ok &= lxs_expect_u64("standard_alu.comb_gate_count", rhs.plan->comb_gate_count, 0ULL);

	memset(values, 0, sizeof(values));
	memset(masks, 0, sizeof(masks));

	for (uint32_t iter = 0; iter < 128U; ++iter)
		{
		char label[128];
		uint32_t op = (uint32_t)(lxs_next_rand(&rng) & 7U);

		lxs_set_scalar_bits(values, masks, width_bits, lxs_next_rand(&rng));
		lxs_set_scalar_bits(values + width_bits, masks + width_bits, width_bits, lxs_next_rand(&rng));
		values[width_bits * 2U + 0U] = (op & 1U) ? ~0ULL : 0ULL;
		values[width_bits * 2U + 1U] = (op & 2U) ? ~0ULL : 0ULL;
		values[width_bits * 2U + 2U] = (op & 4U) ? ~0ULL : 0ULL;
		masks[width_bits * 2U + 0U] = 0ULL;
		masks[width_bits * 2U + 1U] = 0ULL;
		masks[width_bits * 2U + 2U] = 0ULL;

		lxs_apply_inputs(&lhs.ctx, lhs.plan, values, masks);
		lxs_execute_plan(&lhs.ctx, lhs.plan);
		lxs_read_outputs(&lhs.ctx, lhs.plan, lhs_out_values, lhs_out_masks);

		lxs_apply_inputs(&rhs.ctx, rhs.plan, values, masks);
		lxs_execute_plan(&rhs.ctx, rhs.plan);
		lxs_read_outputs(&rhs.ctx, rhs.plan, rhs_out_values, rhs_out_masks);

		for (uint32_t out = 0; out < (width_bits + 4U); ++out)
			{
			snprintf(label, sizeof(label), "%s.value.%u.%u", label_prefix, iter, out);
			ok &= lxs_expect_u64(label, rhs_out_values[out], lhs_out_values[out]);
			snprintf(label, sizeof(label), "%s.mask.%u.%u", label_prefix, iter, out);
			ok &= lxs_expect_u64(label, rhs_out_masks[out], lhs_out_masks[out]);
			}
		}

	lxs_unload_case(&lhs);
	lxs_unload_case(&rhs);
	return ok;
	}

static int lxs_test_standard_rom_equivalence(
	const char *reference_path,
	const char *explicit_path,
	const char *label_prefix,
	uint32_t data_width,
	uint32_t expected_depth)
	{
	lxs_loaded_case lhs;
	lxs_loaded_case rhs;
	uint64_t values[16];
	uint64_t masks[16];
	uint64_t lhs_out_values[64];
	uint64_t lhs_out_masks[64];
	uint64_t rhs_out_values[64];
	uint64_t rhs_out_masks[64];
	int ok = 1;

	if (!lxs_load_case(reference_path, &lhs))
		{
		fprintf(stderr, "FAIL %s: unable to load reference circuit\n", label_prefix);
		return 0;
		}

	if (!lxs_load_case(explicit_path, &rhs))
		{
		fprintf(stderr, "FAIL %s: unable to load explicit circuit\n", label_prefix);
		lxs_unload_case(&lhs);
		return 0;
		}

	ok &= lxs_expect_u64(label_prefix, rhs.plan->rom_count, 1ULL);
	ok &= lxs_expect_u64("standard_rom.data_width", rhs.plan->roms[0].data_width, data_width);
	ok &= lxs_expect_u64("standard_rom.depth", rhs.plan->roms[0].depth, expected_depth);
	ok &= lxs_expect_u64("standard_rom.output_count", rhs.plan->outputs.count, data_width);

	memset(values, 0, sizeof(values));
	memset(masks, 0, sizeof(masks));

	for (uint32_t addr = 0; addr < expected_depth; ++addr)
		{
		char label[128];

		lxs_set_scalar_bits(values, masks, rhs.plan->roms[0].addr_width, addr);

		lxs_apply_inputs(&lhs.ctx, lhs.plan, values, masks);
		lxs_execute_plan(&lhs.ctx, lhs.plan);
		lxs_read_outputs(&lhs.ctx, lhs.plan, lhs_out_values, lhs_out_masks);

		lxs_apply_inputs(&rhs.ctx, rhs.plan, values, masks);
		lxs_execute_plan(&rhs.ctx, rhs.plan);
		lxs_read_outputs(&rhs.ctx, rhs.plan, rhs_out_values, rhs_out_masks);

		for (uint32_t bit = 0; bit < data_width; ++bit)
			{
			snprintf(label, sizeof(label), "%s.value.%u.%u", label_prefix, addr, bit);
			ok &= lxs_expect_u64(label, rhs_out_values[bit], lhs_out_values[bit]);
			snprintf(label, sizeof(label), "%s.mask.%u.%u", label_prefix, addr, bit);
			ok &= lxs_expect_u64(label, rhs_out_masks[bit], lhs_out_masks[bit]);
			}
		}

	lxs_unload_case(&lhs);
	lxs_unload_case(&rhs);
	return ok;
	}

static int lxs_test_standard_ram_equivalence(
	const char *reference_path,
	const char *explicit_path,
	const char *label_prefix,
	uint32_t data_width,
	uint32_t expected_depth)
	{
	lxs_loaded_case lhs;
	lxs_loaded_case rhs;
	uint64_t values[128];
	uint64_t masks[128];
	uint64_t lhs_out_values[64];
	uint64_t lhs_out_masks[64];
	uint64_t rhs_out_values[64];
	uint64_t rhs_out_masks[64];
	uint64_t write_scalar = 0xA5U;
	int ok = 1;

	if (data_width >= 24U)
		{
		write_scalar = 0xA5C33CU;
		}
	else if (data_width >= 16U)
		{
		write_scalar = 0xA55AU;
		}

	if (!lxs_load_case(reference_path, &lhs))
		{
		fprintf(stderr, "FAIL %s: unable to load reference circuit\n", label_prefix);
		return 0;
		}

	if (!lxs_load_case(explicit_path, &rhs))
		{
		fprintf(stderr, "FAIL %s: unable to load explicit circuit\n", label_prefix);
		lxs_unload_case(&lhs);
		return 0;
		}

	ok &= lxs_expect_u64(label_prefix, rhs.plan->ram_count, 1ULL);
	ok &= lxs_expect_u64("standard_ram.data_width", rhs.plan->rams[0].data_width, data_width);
	ok &= lxs_expect_u64("standard_ram.depth", rhs.plan->rams[0].depth, expected_depth);
	ok &= lxs_expect_u64("standard_ram.output_count", rhs.plan->outputs.count, data_width);

	memset(values, 0, sizeof(values));
	memset(masks, 0, sizeof(masks));

	lxs_set_scalar_bits(values + 0U, masks + 0U, rhs.plan->rams[0].addr_width, 0U);
	lxs_set_scalar_bits(values + rhs.plan->rams[0].addr_width, masks + rhs.plan->rams[0].addr_width, rhs.plan->rams[0].addr_width, 1U);
	lxs_set_scalar_bits(values + (rhs.plan->rams[0].addr_width * 2U), masks + (rhs.plan->rams[0].addr_width * 2U), data_width, write_scalar);
	values[(rhs.plan->rams[0].addr_width * 2U) + data_width] = ~0ULL;
	masks[(rhs.plan->rams[0].addr_width * 2U) + data_width] = 0ULL;

	lxs_apply_inputs(&lhs.ctx, lhs.plan, values, masks);
	lxs_execute_plan(&lhs.ctx, lhs.plan);
	lxs_read_outputs(&lhs.ctx, lhs.plan, lhs_out_values, lhs_out_masks);

	lxs_apply_inputs(&rhs.ctx, rhs.plan, values, masks);
	lxs_execute_plan(&rhs.ctx, rhs.plan);
	lxs_read_outputs(&rhs.ctx, rhs.plan, rhs_out_values, rhs_out_masks);

	for (uint32_t bit = 0; bit < data_width; ++bit)
		{
		char label[128];
		snprintf(label, sizeof(label), "%s.cycle1.value.%u", label_prefix, bit);
		ok &= lxs_expect_u64(label, rhs_out_values[bit], lhs_out_values[bit]);
		snprintf(label, sizeof(label), "%s.cycle1.mask.%u", label_prefix, bit);
		ok &= lxs_expect_u64(label, rhs_out_masks[bit], lhs_out_masks[bit]);
		}

	memset(values, 0, sizeof(values));
	memset(masks, 0, sizeof(masks));
	lxs_set_scalar_bits(values + 0U, masks + 0U, rhs.plan->rams[0].addr_width, 1U);

	lxs_apply_inputs(&lhs.ctx, lhs.plan, values, masks);
	lxs_execute_plan(&lhs.ctx, lhs.plan);
	lxs_read_outputs(&lhs.ctx, lhs.plan, lhs_out_values, lhs_out_masks);

	lxs_apply_inputs(&rhs.ctx, rhs.plan, values, masks);
	lxs_execute_plan(&rhs.ctx, rhs.plan);
	lxs_read_outputs(&rhs.ctx, rhs.plan, rhs_out_values, rhs_out_masks);

	for (uint32_t bit = 0; bit < data_width; ++bit)
		{
		char label[128];
		snprintf(label, sizeof(label), "%s.cycle2.value.%u", label_prefix, bit);
		ok &= lxs_expect_u64(label, rhs_out_values[bit], lhs_out_values[bit]);
		snprintf(label, sizeof(label), "%s.cycle2.mask.%u", label_prefix, bit);
		ok &= lxs_expect_u64(label, rhs_out_masks[bit], lhs_out_masks[bit]);
		}

	memset(values, 0, sizeof(values));
	memset(masks, 0, sizeof(masks));
	for (uint32_t bit = 0; bit < rhs.plan->rams[0].addr_width; ++bit)
		{
		masks[bit] = ~0ULL;
		values[bit] = 0ULL;
		}

	lxs_apply_inputs(&lhs.ctx, lhs.plan, values, masks);
	lxs_execute_plan(&lhs.ctx, lhs.plan);
	lxs_read_outputs(&lhs.ctx, lhs.plan, lhs_out_values, lhs_out_masks);

	lxs_apply_inputs(&rhs.ctx, rhs.plan, values, masks);
	lxs_execute_plan(&rhs.ctx, rhs.plan);
	lxs_read_outputs(&rhs.ctx, rhs.plan, rhs_out_values, rhs_out_masks);

	for (uint32_t bit = 0; bit < data_width; ++bit)
		{
		char label[128];
		snprintf(label, sizeof(label), "%s.unknown.value.%u", label_prefix, bit);
		ok &= lxs_expect_u64(label, rhs_out_values[bit], lhs_out_values[bit]);
		snprintf(label, sizeof(label), "%s.unknown.mask.%u", label_prefix, bit);
		ok &= lxs_expect_u64(label, rhs_out_masks[bit], lhs_out_masks[bit]);
		}

	lxs_unload_case(&lhs);
	lxs_unload_case(&rhs);
	return ok;
	}

static int lxs_test_standard_regfile_equivalence(
	const char *reference_path,
	const char *explicit_path,
	const char *label_prefix,
	uint32_t data_width,
	uint32_t expected_depth)
	{
	lxs_loaded_case lhs;
	lxs_loaded_case rhs;
	uint64_t values[192];
	uint64_t masks[192];
	uint64_t lhs_out_values[128];
	uint64_t lhs_out_masks[128];
	uint64_t rhs_out_values[128];
	uint64_t rhs_out_masks[128];
	uint64_t rng = 0x52454746494C455FULL;
	int ok = 1;

	if (!lxs_load_case(reference_path, &lhs))
		{
		fprintf(stderr, "FAIL %s: unable to load reference circuit\n", label_prefix);
		return 0;
		}

	if (!lxs_load_case(explicit_path, &rhs))
		{
		fprintf(stderr, "FAIL %s: unable to load explicit circuit\n", label_prefix);
		lxs_unload_case(&lhs);
		return 0;
		}

	ok &= lxs_expect_u64(label_prefix, rhs.plan->regfile_count, 1ULL);
	ok &= lxs_expect_u64("standard_regfile.data_width", rhs.plan->regfiles[0].data_width, data_width);
	ok &= lxs_expect_u64("standard_regfile.depth", rhs.plan->regfiles[0].depth, expected_depth);
	ok &= lxs_expect_u64("standard_regfile.output_count", rhs.plan->outputs.count, data_width * 2U);

	memset(masks, 0, sizeof(masks));
	for (uint32_t cycle = 0; cycle < 64U; ++cycle)
		{
		char label[128];
		uint32_t addr_width = rhs.plan->regfiles[0].addr_width;
		uint32_t read_a = (uint32_t)(lxs_next_rand(&rng) & ((1U << addr_width) - 1U));
		uint32_t read_b = (uint32_t)(lxs_next_rand(&rng) & ((1U << addr_width) - 1U));
		uint32_t write_a = (uint32_t)(lxs_next_rand(&rng) & ((1U << addr_width) - 1U));
		uint64_t enable = (lxs_next_rand(&rng) >> 63U) ? ~0ULL : 0ULL;

		memset(values, 0, sizeof(values));
		lxs_set_scalar_bits(values, masks, addr_width, read_a);
		lxs_set_scalar_bits(values + addr_width, masks + addr_width, addr_width, read_b);
		lxs_set_scalar_bits(values + (addr_width * 2U), masks + (addr_width * 2U), addr_width, write_a);
		lxs_set_scalar_bits(values + (addr_width * 3U), masks + (addr_width * 3U), data_width, lxs_next_rand(&rng));
		values[(addr_width * 3U) + data_width] = enable;
		masks[(addr_width * 3U) + data_width] = 0ULL;

		lxs_apply_inputs(&lhs.ctx, lhs.plan, values, masks);
		lxs_execute_plan(&lhs.ctx, lhs.plan);
		lxs_read_outputs(&lhs.ctx, lhs.plan, lhs_out_values, lhs_out_masks);

		lxs_apply_inputs(&rhs.ctx, rhs.plan, values, masks);
		lxs_execute_plan(&rhs.ctx, rhs.plan);
		lxs_read_outputs(&rhs.ctx, rhs.plan, rhs_out_values, rhs_out_masks);

		for (uint32_t out = 0; out < (data_width * 2U); ++out)
			{
			snprintf(label, sizeof(label), "%s.value.%u.%u", label_prefix, cycle, out);
			ok &= lxs_expect_u64(label, rhs_out_values[out], lhs_out_values[out]);
			snprintf(label, sizeof(label), "%s.mask.%u.%u", label_prefix, cycle, out);
			ok &= lxs_expect_u64(label, rhs_out_masks[out], lhs_out_masks[out]);
			}
		}

	lxs_unload_case(&lhs);
	lxs_unload_case(&rhs);
	return ok;
	}

static int lxs_test_standard_counter_equivalence(
	const char *reference_path,
	const char *explicit_path,
	const char *label_prefix,
	uint32_t width_bits)
	{
	lxs_loaded_case lhs;
	lxs_loaded_case rhs;
	uint64_t lhs_values[1];
	uint64_t lhs_masks[1];
	uint64_t lhs_out_values[64];
	uint64_t lhs_out_masks[64];
	uint64_t rhs_out_values[64];
	uint64_t rhs_out_masks[64];
	int ok = 1;

	if (!lxs_load_case(reference_path, &lhs))
		{
		fprintf(stderr, "FAIL %s: unable to load reference circuit\n", label_prefix);
		return 0;
		}

	if (!lxs_load_case(explicit_path, &rhs))
		{
		fprintf(stderr, "FAIL %s: unable to load explicit circuit\n", label_prefix);
		lxs_unload_case(&lhs);
		return 0;
		}

	ok &= lxs_expect_u64(label_prefix, rhs.plan->register_count, 1ULL);
	ok &= lxs_expect_u64("standard_counter.width_bits", rhs.plan->registers[0].width_bits, width_bits);

	memset(lhs_masks, 0, sizeof(lhs_masks));
	lhs_values[0] = ~0ULL;
	for (uint32_t cycle = 0; cycle < 64U; ++cycle)
		{
		char label[128];

		lxs_apply_inputs(&lhs.ctx, lhs.plan, lhs_values, lhs_masks);
		lxs_execute_plan(&lhs.ctx, lhs.plan);
		lxs_read_outputs(&lhs.ctx, lhs.plan, lhs_out_values, lhs_out_masks);

		lxs_execute_plan(&rhs.ctx, rhs.plan);
		lxs_read_outputs(&rhs.ctx, rhs.plan, rhs_out_values, rhs_out_masks);

		for (uint32_t bit = 0; bit < width_bits; ++bit)
			{
			snprintf(label, sizeof(label), "%s.value.%u.%u", label_prefix, cycle, bit);
			ok &= lxs_expect_u64(label, rhs_out_values[bit], lhs_out_values[bit]);
			snprintf(label, sizeof(label), "%s.mask.%u.%u", label_prefix, cycle, bit);
			ok &= lxs_expect_u64(label, rhs_out_masks[bit], lhs_out_masks[bit]);
			}
		}

	lxs_unload_case(&lhs);
	lxs_unload_case(&rhs);
	return ok;
	}

static int lxs_test_standard_counter_en_equivalence(
	const char *reference_path,
	const char *explicit_path,
	const char *label_prefix,
	uint32_t width_bits)
	{
	lxs_loaded_case lhs;
	lxs_loaded_case rhs;
	uint64_t values[1];
	uint64_t masks[1];
	uint64_t lhs_out_values[64];
	uint64_t lhs_out_masks[64];
	uint64_t rhs_out_values[64];
	uint64_t rhs_out_masks[64];
	uint64_t rng = 0x434F554E54455245ULL;
	int ok = 1;

	if (!lxs_load_case(reference_path, &lhs))
		{
		fprintf(stderr, "FAIL %s: unable to load reference circuit\n", label_prefix);
		return 0;
		}

	if (!lxs_load_case(explicit_path, &rhs))
		{
		fprintf(stderr, "FAIL %s: unable to load explicit circuit\n", label_prefix);
		lxs_unload_case(&lhs);
		return 0;
		}

	ok &= lxs_expect_u64(label_prefix, rhs.plan->register_count, 1ULL);
	ok &= lxs_expect_u64("standard_counter_en.width_bits", rhs.plan->registers[0].width_bits, width_bits);

	memset(masks, 0, sizeof(masks));
	for (uint32_t cycle = 0; cycle < 64U; ++cycle)
		{
		char label[128];
		values[0] = (lxs_next_rand(&rng) >> 63U) ? ~0ULL : 0ULL;

		lxs_apply_inputs(&lhs.ctx, lhs.plan, values, masks);
		lxs_execute_plan(&lhs.ctx, lhs.plan);
		lxs_read_outputs(&lhs.ctx, lhs.plan, lhs_out_values, lhs_out_masks);

		lxs_apply_inputs(&rhs.ctx, rhs.plan, values, masks);
		lxs_execute_plan(&rhs.ctx, rhs.plan);
		lxs_read_outputs(&rhs.ctx, rhs.plan, rhs_out_values, rhs_out_masks);

		for (uint32_t bit = 0; bit < width_bits; ++bit)
			{
			snprintf(label, sizeof(label), "%s.value.%u.%u", label_prefix, cycle, bit);
			ok &= lxs_expect_u64(label, rhs_out_values[bit], lhs_out_values[bit]);
			snprintf(label, sizeof(label), "%s.mask.%u.%u", label_prefix, cycle, bit);
			ok &= lxs_expect_u64(label, rhs_out_masks[bit], lhs_out_masks[bit]);
			}
		}

	lxs_unload_case(&lhs);
	lxs_unload_case(&rhs);
	return ok;
	}

static int lxs_test_mux2_8_explicit(void)
	{
	return lxs_test_standard_mux_equivalence(
		"Tests\\Circuits\\mux2_8_primitive.bench",
		"Tests\\Circuits\\mux2_8_explicit.bench",
		"mux2_8_explicit",
		17U,
		8U,
		LXS_STANDARD_MUX_KIND_2,
		1U);
	}

static int lxs_test_mux4_8_explicit(void)
	{
	return lxs_test_standard_mux_equivalence(
		"Tests\\Circuits\\mux4_8_primitive.bench",
		"Tests\\Circuits\\mux4_8_explicit.bench",
		"mux4_8_explicit",
		34U,
		8U,
		LXS_STANDARD_MUX_KIND_4,
		0U);
	}

static int lxs_test_reg8_explicit(void)
	{
	return lxs_test_standard_register_equivalence(
		"Tests\\Circuits\\reg8_reference.bench",
		"Tests\\Circuits\\reg8_explicit.bench",
		"reg8_explicit",
		8U);
	}

static int lxs_test_reg16_explicit(void)
	{
	return lxs_test_standard_register_equivalence(
		"Tests\\Circuits\\reg16_reference.bench",
		"Tests\\Circuits\\reg16_explicit.bench",
		"reg16_explicit",
		16U);
	}

static int lxs_test_reg_en8_explicit(void)
	{
	return lxs_test_standard_register_en_equivalence(
		"Tests\\Circuits\\reg_en8_reference.bench",
		"Tests\\Circuits\\reg_en8_explicit.bench",
		"reg_en8_explicit",
		8U);
	}

static int lxs_test_reg_en16_explicit(void)
	{
	return lxs_test_standard_register_en_equivalence(
		"Tests\\Circuits\\reg_en16_reference.bench",
		"Tests\\Circuits\\reg_en16_explicit.bench",
		"reg_en16_explicit",
		16U);
	}

static int lxs_test_reg_en_rst8_explicit(void)
	{
	return lxs_test_standard_register_en_rst_equivalence(
		"Tests\\Circuits\\reg_en_rst8_reference.bench",
		"Tests\\Circuits\\reg_en_rst8_explicit.bench",
		"reg_en_rst8_explicit",
		8U);
	}

static int lxs_test_reg_en_rst16_explicit(void)
	{
	return lxs_test_standard_register_en_rst_equivalence(
		"Tests\\Circuits\\reg_en_rst16_reference.bench",
		"Tests\\Circuits\\reg_en_rst16_explicit.bench",
		"reg_en_rst16_explicit",
		16U);
	}

static int lxs_test_add8_explicit(void)
	{
	return lxs_test_standard_add_equivalence(
		"Tests\\Circuits\\add8_reference.bench",
		"Tests\\Circuits\\add8_explicit.bench",
		"add8_explicit",
		8U);
	}

static int lxs_test_add16_explicit(void)
	{
	return lxs_test_standard_add_equivalence(
		"Tests\\Circuits\\add16_reference.bench",
		"Tests\\Circuits\\add16_explicit.bench",
		"add16_explicit",
		16U);
	}

static int lxs_test_cmp8_explicit(void)
	{
	return lxs_test_standard_cmp_equivalence(
		"Tests\\Circuits\\cmp8_reference.bench",
		"Tests\\Circuits\\cmp8_explicit.bench",
		"cmp8_explicit",
		8U);
	}

static int lxs_test_cmp16_explicit(void)
	{
	return lxs_test_standard_cmp_equivalence(
		"Tests\\Circuits\\cmp16_reference.bench",
		"Tests\\Circuits\\cmp16_explicit.bench",
		"cmp16_explicit",
		16U);
	}

static int lxs_test_alu8_explicit(void)
	{
	return lxs_test_standard_alu_equivalence(
		"Tests\\Circuits\\alu8_reference.bench",
		"Tests\\Circuits\\alu8_explicit.bench",
		"alu8_explicit",
		8U);
	}

static int lxs_test_alu16_explicit(void)
	{
	return lxs_test_standard_alu_equivalence(
		"Tests\\Circuits\\alu16_reference.bench",
		"Tests\\Circuits\\alu16_explicit.bench",
		"alu16_explicit",
		16U);
	}

static int lxs_test_rom16_explicit(void)
	{
	return lxs_test_standard_rom_equivalence(
		"Tests\\Circuits\\rom16_reference.bench",
		"Tests\\Circuits\\rom16_explicit.bench",
		"rom16_explicit",
		16U,
		4U);
	}

static int lxs_test_ram8_explicit(void)
	{
	return lxs_test_standard_ram_equivalence(
		"Tests\\Circuits\\ram8_reference.bench",
		"Tests\\Circuits\\ram8_explicit.bench",
		"ram8_explicit",
		8U,
		4U);
	}

static int lxs_test_regfile8_explicit(void)
	{
	return lxs_test_standard_regfile_equivalence(
		"Tests\\Circuits\\regfile8_reference.bench",
		"Tests\\Circuits\\regfile8_explicit.bench",
		"regfile8_explicit",
		8U,
		2U);
	}

static int lxs_test_regfile16_explicit(void)
	{
	return lxs_test_standard_regfile_equivalence(
		"Tests\\Circuits\\regfile16_reference.bench",
		"Tests\\Circuits\\regfile16_explicit.bench",
		"regfile16_explicit",
		16U,
		2U);
	}

static int lxs_test_regfile32_explicit(void)
	{
	return lxs_test_standard_regfile_equivalence(
		"Tests\\Circuits\\regfile32_reference.bench",
		"Tests\\Circuits\\regfile32_explicit.bench",
		"regfile32_explicit",
		32U,
		2U);
	}

static int lxs_test_counter8_explicit(void)
	{
	return lxs_test_standard_counter_equivalence(
		"Tests\\Circuits\\counter8_reference.bench",
		"Tests\\Circuits\\counter8_explicit.bench",
		"counter8_explicit",
		8U);
	}

static int lxs_test_counter16_explicit(void)
	{
	return lxs_test_standard_counter_equivalence(
		"Tests\\Circuits\\counter16_reference.bench",
		"Tests\\Circuits\\counter16_explicit.bench",
		"counter16_explicit",
		16U);
	}

static int lxs_test_counter_en8_explicit(void)
	{
	return lxs_test_standard_counter_en_equivalence(
		"Tests\\Circuits\\counter_en8_reference.bench",
		"Tests\\Circuits\\counter_en8_explicit.bench",
		"counter_en8_explicit",
		8U);
	}

static int lxs_test_counter_en16_explicit(void)
	{
	return lxs_test_standard_counter_en_equivalence(
		"Tests\\Circuits\\counter_en16_reference.bench",
		"Tests\\Circuits\\counter_en16_explicit.bench",
		"counter_en16_explicit",
		16U);
	}

static int lxs_test_ram16_explicit(void)
	{
	return lxs_test_standard_ram_equivalence(
		"Tests\\Circuits\\ram16_reference.bench",
		"Tests\\Circuits\\ram16_explicit.bench",
		"ram16_explicit",
		16U,
		4U);
	}

static int lxs_test_ram24_explicit(void)
	{
	return lxs_test_standard_ram_equivalence(
		"Tests\\Circuits\\ram24_reference.bench",
		"Tests\\Circuits\\ram24_explicit.bench",
		"ram24_explicit",
		24U,
		4U);
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

static int lxs_test_canonical_basic(void)
	{
	lxs_loaded_case loaded;
	uint64_t values[2];
	uint64_t masks[2];
	uint64_t out_values[3];
	uint64_t out_masks[3];
	int ok = 1;

	if (!lxs_load_case("Tests\\Circuits\\canonical_basic.bench", &loaded))
		{
		fprintf(stderr, "FAIL canonical_basic: unable to load test circuit\n");
		return 0;
		}

	values[0] = ~0ULL;
	values[1] = 0ULL;
	masks[0] = 0ULL;
	masks[1] = 0ULL;
	lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
	ok &= lxs_expect_u64("canonical_basic.y_or.high", out_values[0], ~0ULL);
	ok &= lxs_expect_u64("canonical_basic.y_or.mask.high", out_masks[0], 0ULL);
	ok &= lxs_expect_u64("canonical_basic.y_sel.high", out_values[1], ~0ULL);
	ok &= lxs_expect_u64("canonical_basic.y_sel.mask.high", out_masks[1], 0ULL);
	ok &= lxs_expect_u64("canonical_basic.y_const1.high", out_values[2], ~0ULL);
	ok &= lxs_expect_u64("canonical_basic.y_const1.mask.high", out_masks[2], 0ULL);

	values[0] = 0ULL;
	values[1] = ~0ULL;
	lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
	ok &= lxs_expect_u64("canonical_basic.y_or.low", out_values[0], ~0ULL);
	ok &= lxs_expect_u64("canonical_basic.y_sel.low", out_values[1], 0ULL);
	ok &= lxs_expect_u64("canonical_basic.y_const1.low", out_values[2], ~0ULL);

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

static int lxs_test_parity4_explicit(void)
	{
	lxs_loaded_case loaded;
	uint64_t values[4];
	uint64_t masks[4];
	uint64_t out_values[1];
	uint64_t out_masks[1];
	int ok = 1;

	if (!lxs_load_case("Tests\\Circuits\\parity4_explicit.bench", &loaded))
		{
		fprintf(stderr, "FAIL parity4_explicit: unable to load test circuit\n");
		return 0;
		}

	ok &= lxs_expect_u64("parity4_explicit.macro_count", loaded.plan->macro_count, 1ULL);
	ok &= lxs_expect_u64("parity4_explicit.comb_gate_count", loaded.plan->comb_gate_count, 0ULL);

	for (uint32_t combo = 0; combo < 16U; ++combo)
		{
		uint32_t a = (combo >> 3U) & 1U;
		uint32_t b = (combo >> 2U) & 1U;
		uint32_t c = (combo >> 1U) & 1U;
		uint32_t d = combo & 1U;
		uint32_t y = a ^ b ^ c ^ d;
		char label[80];

		values[0] = a ? ~0ULL : 0ULL;
		values[1] = b ? ~0ULL : 0ULL;
		values[2] = c ? ~0ULL : 0ULL;
		values[3] = d ? ~0ULL : 0ULL;
		memset(masks, 0, sizeof(masks));

		lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
		lxs_execute_plan(&loaded.ctx, loaded.plan);
		lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);

		snprintf(label, sizeof(label), "parity4_explicit.y.%u", combo);
		ok &= lxs_expect_u64(label, out_values[0], y ? ~0ULL : 0ULL);
		snprintf(label, sizeof(label), "parity4_explicit.y.mask.%u", combo);
		ok &= lxs_expect_u64(label, out_masks[0], 0ULL);
		}

	lxs_unload_case(&loaded);
	return ok;
	}

static int lxs_test_parity8_explicit(void)
	{
	lxs_loaded_case loaded;
	uint64_t values[8];
	uint64_t masks[8];
	uint64_t out_values[1];
	uint64_t out_masks[1];
	int ok = 1;

	if (!lxs_load_case("Tests\\Circuits\\parity8_explicit.bench", &loaded))
		{
		fprintf(stderr, "FAIL parity8_explicit: unable to load test circuit\n");
		return 0;
		}

	ok &= lxs_expect_u64("parity8_explicit.macro_count", loaded.plan->macro_count, 1ULL);
	ok &= lxs_expect_u64("parity8_explicit.comb_gate_count", loaded.plan->comb_gate_count, 0ULL);

	for (uint32_t combo = 0; combo < 256U; ++combo)
		{
		uint32_t y = 0U;
		char label[80];

		memset(masks, 0, sizeof(masks));
		for (uint32_t i = 0; i < 8U; ++i)
			{
			uint32_t bit = (combo >> i) & 1U;
			values[i] = bit ? ~0ULL : 0ULL;
			y ^= bit;
			}

		lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
		lxs_execute_plan(&loaded.ctx, loaded.plan);
		lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);

		snprintf(label, sizeof(label), "parity8_explicit.y.%u", combo);
		ok &= lxs_expect_u64(label, out_values[0], y ? ~0ULL : 0ULL);
		snprintf(label, sizeof(label), "parity8_explicit.y.mask.%u", combo);
		ok &= lxs_expect_u64(label, out_masks[0], 0ULL);
		}

	lxs_unload_case(&loaded);
	return ok;
	}

static int lxs_test_parity4_recognition(void)
	{
	lxs_loaded_case loaded;
	uint64_t values[4];
	uint64_t masks[4];
	uint64_t out_values[1];
	uint64_t out_masks[1];
	int ok = 1;

	if (!lxs_set_env_var("LXS_RECOGNITION_MASK", "1"))
		{
		fprintf(stderr, "FAIL parity4_recognition: unable to set recognition mask\n");
		return 0;
		}

	if (!lxs_load_case("Tests\\Circuits\\parity4_primitive.bench", &loaded))
		{
		lxs_set_env_var("LXS_RECOGNITION_MASK", "");
		fprintf(stderr, "FAIL parity4_recognition: unable to load test circuit\n");
		return 0;
		}

	ok &= lxs_expect_u64("parity4_recognition.macro_count", loaded.plan->macro_count, 1ULL);
	ok &= lxs_expect_u64("parity4_recognition.comb_gate_count", loaded.plan->comb_gate_count, 0ULL);
	ok &= lxs_expect_u64(
		"parity4_recognition.match_count",
		loaded.plan->recognition_match_count[LXS_RECOGNITION_FAMILY_PARITY],
		1ULL);
	ok &= lxs_expect_u64(
		"parity4_recognition.node_reduction",
		loaded.plan->recognition_node_reduction[LXS_RECOGNITION_FAMILY_PARITY],
		2ULL);

	for (uint32_t combo = 0; combo < 16U; ++combo)
		{
		uint32_t a = (combo >> 3U) & 1U;
		uint32_t b = (combo >> 2U) & 1U;
		uint32_t c = (combo >> 1U) & 1U;
		uint32_t d = combo & 1U;
		uint32_t y = a ^ b ^ c ^ d;
		char label[96];

		values[0] = a ? ~0ULL : 0ULL;
		values[1] = b ? ~0ULL : 0ULL;
		values[2] = c ? ~0ULL : 0ULL;
		values[3] = d ? ~0ULL : 0ULL;
		memset(masks, 0, sizeof(masks));

		lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
		lxs_execute_plan(&loaded.ctx, loaded.plan);
		lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);

		snprintf(label, sizeof(label), "parity4_recognition.y.%u", combo);
		ok &= lxs_expect_u64(label, out_values[0], y ? ~0ULL : 0ULL);
		snprintf(label, sizeof(label), "parity4_recognition.mask.%u", combo);
		ok &= lxs_expect_u64(label, out_masks[0], 0ULL);
		}

	lxs_unload_case(&loaded);
	lxs_set_env_var("LXS_RECOGNITION_MASK", "");
	return ok;
	}

static int lxs_test_parity4_recognition_negative(void)
	{
	lxs_loaded_case loaded;
	int ok = 1;

	if (!lxs_set_env_var("LXS_RECOGNITION_MASK", "1"))
		{
		fprintf(stderr, "FAIL parity4_recognition_negative: unable to set recognition mask\n");
		return 0;
		}

	if (!lxs_load_case("Tests\\Circuits\\parity4_negative.bench", &loaded))
		{
		lxs_set_env_var("LXS_RECOGNITION_MASK", "");
		fprintf(stderr, "FAIL parity4_recognition_negative: unable to load test circuit\n");
		return 0;
		}

	ok &= lxs_expect_u64("parity4_recognition_negative.macro_count", loaded.plan->macro_count, 0ULL);
	ok &= lxs_expect_u64("parity4_recognition_negative.comb_gate_count", loaded.plan->comb_gate_count, 4ULL);
	ok &= lxs_expect_u64(
		"parity4_recognition_negative.match_count",
		loaded.plan->recognition_match_count[LXS_RECOGNITION_FAMILY_PARITY],
		0ULL);
	ok &= lxs_expect_u64(
		"parity4_recognition_negative.node_reduction",
		loaded.plan->recognition_node_reduction[LXS_RECOGNITION_FAMILY_PARITY],
		0ULL);

	lxs_unload_case(&loaded);
	lxs_set_env_var("LXS_RECOGNITION_MASK", "");
	return ok;
	}

static int lxs_test_xor_fan8_explicit(void)
	{
	lxs_loaded_case loaded;
	uint64_t values[9];
	uint64_t masks[9];
	uint64_t out_values[8];
	uint64_t out_masks[8];
	int ok = 1;

	if (!lxs_load_case("Tests\\Circuits\\xor_fan8_explicit.bench", &loaded))
		{
		fprintf(stderr, "FAIL xor_fan8_explicit: unable to load test circuit\n");
		return 0;
		}

	ok &= lxs_expect_u64("xor_fan8_explicit.macro_count", loaded.plan->macro_count, 0ULL);
	ok &= lxs_expect_u64("xor_fan8_explicit.multi_macro_count", loaded.plan->multi_macro_count, 1ULL);
	ok &= lxs_expect_u64("xor_fan8_explicit.comb_gate_count", loaded.plan->comb_gate_count, 0ULL);

	for (uint32_t combo = 0; combo < 512U; ++combo)
		{
		uint32_t s = (combo >> 8U) & 1U;
		char label[96];

		values[0] = s ? ~0ULL : 0ULL;
		masks[0] = 0ULL;
		for (uint32_t i = 0; i < 8U; ++i)
			{
			uint32_t bit = (combo >> i) & 1U;
			values[i + 1U] = bit ? ~0ULL : 0ULL;
			masks[i + 1U] = 0ULL;
			}

		lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
		lxs_execute_plan(&loaded.ctx, loaded.plan);
		lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);

		for (uint32_t i = 0; i < 8U; ++i)
			{
			uint32_t y = s ^ ((combo >> i) & 1U);
			snprintf(label, sizeof(label), "xor_fan8_explicit.y%u.%u", i, combo);
			ok &= lxs_expect_u64(label, out_values[i], y ? ~0ULL : 0ULL);
			snprintf(label, sizeof(label), "xor_fan8_explicit.y%u.mask.%u", i, combo);
			ok &= lxs_expect_u64(label, out_masks[i], 0ULL);
			}
		}

	lxs_unload_case(&loaded);
	return ok;
	}

static int lxs_test_xor_fan8_recognition(void)
	{
	lxs_loaded_case loaded;
	uint64_t values[9];
	uint64_t masks[9];
	uint64_t out_values[8];
	uint64_t out_masks[8];
	int ok = 1;

	if (!lxs_set_env_var("LXS_RECOGNITION_MASK", "2"))
		{
		fprintf(stderr, "FAIL xor_fan8_recognition: unable to set recognition mask\n");
		return 0;
		}

	if (!lxs_load_case("Tests\\Circuits\\xor_fan8_primitive.bench", &loaded))
		{
		lxs_set_env_var("LXS_RECOGNITION_MASK", "");
		fprintf(stderr, "FAIL xor_fan8_recognition: unable to load test circuit\n");
		return 0;
		}

	ok &= lxs_expect_u64("xor_fan8_recognition.macro_count", loaded.plan->macro_count, 0ULL);
	ok &= lxs_expect_u64("xor_fan8_recognition.multi_macro_count", loaded.plan->multi_macro_count, 1ULL);
	ok &= lxs_expect_u64("xor_fan8_recognition.comb_gate_count", loaded.plan->comb_gate_count, 0ULL);
	ok &= lxs_expect_u64(
		"xor_fan8_recognition.match_count",
		loaded.plan->recognition_match_count[LXS_RECOGNITION_FAMILY_SHARED_XOR],
		1ULL);
	ok &= lxs_expect_u64(
		"xor_fan8_recognition.node_reduction",
		loaded.plan->recognition_node_reduction[LXS_RECOGNITION_FAMILY_SHARED_XOR],
		7ULL);

	for (uint32_t combo = 0; combo < 512U; ++combo)
		{
		uint32_t s = (combo >> 8U) & 1U;
		char label[96];

		values[0] = s ? ~0ULL : 0ULL;
		masks[0] = 0ULL;
		for (uint32_t i = 0; i < 8U; ++i)
			{
			uint32_t bit = (combo >> i) & 1U;
			values[i + 1U] = bit ? ~0ULL : 0ULL;
			masks[i + 1U] = 0ULL;
			}

		lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
		lxs_execute_plan(&loaded.ctx, loaded.plan);
		lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);

		for (uint32_t i = 0; i < 8U; ++i)
			{
			uint32_t y = s ^ ((combo >> i) & 1U);
			snprintf(label, sizeof(label), "xor_fan8_recognition.y%u.%u", i, combo);
			ok &= lxs_expect_u64(label, out_values[i], y ? ~0ULL : 0ULL);
			snprintf(label, sizeof(label), "xor_fan8_recognition.mask%u.%u", i, combo);
			ok &= lxs_expect_u64(label, out_masks[i], 0ULL);
			}
		}

	lxs_unload_case(&loaded);
	lxs_set_env_var("LXS_RECOGNITION_MASK", "");
	return ok;
	}

static int lxs_test_xor_fan8_recognition_negative(void)
	{
	lxs_loaded_case loaded;
	int ok = 1;

	if (!lxs_set_env_var("LXS_RECOGNITION_MASK", "2"))
		{
		fprintf(stderr, "FAIL xor_fan8_recognition_negative: unable to set recognition mask\n");
		return 0;
		}

	if (!lxs_load_case("Tests\\Circuits\\xor_fan8_negative.bench", &loaded))
		{
		lxs_set_env_var("LXS_RECOGNITION_MASK", "");
		fprintf(stderr, "FAIL xor_fan8_recognition_negative: unable to load test circuit\n");
		return 0;
		}

	ok &= lxs_expect_u64("xor_fan8_recognition_negative.multi_macro_count", loaded.plan->multi_macro_count, 0ULL);
	ok &= lxs_expect_u64("xor_fan8_recognition_negative.comb_gate_count", loaded.plan->comb_gate_count, 9ULL);
	ok &= lxs_expect_u64(
		"xor_fan8_recognition_negative.match_count",
		loaded.plan->recognition_match_count[LXS_RECOGNITION_FAMILY_SHARED_XOR],
		0ULL);
	ok &= lxs_expect_u64(
		"xor_fan8_recognition_negative.node_reduction",
		loaded.plan->recognition_node_reduction[LXS_RECOGNITION_FAMILY_SHARED_XOR],
		0ULL);

	lxs_unload_case(&loaded);
	lxs_set_env_var("LXS_RECOGNITION_MASK", "");
	return ok;
	}

static int lxs_test_and_fan8_explicit(void)
	{
	lxs_loaded_case loaded;
	uint64_t values[9];
	uint64_t masks[9];
	uint64_t out_values[8];
	uint64_t out_masks[8];
	int ok = 1;

	if (!lxs_load_case("Tests\\Circuits\\and_fan8_explicit.bench", &loaded))
		{
		fprintf(stderr, "FAIL and_fan8_explicit: unable to load test circuit\n");
		return 0;
		}

	ok &= lxs_expect_u64("and_fan8_explicit.macro_count", loaded.plan->macro_count, 0ULL);
	ok &= lxs_expect_u64("and_fan8_explicit.multi_macro_count", loaded.plan->multi_macro_count, 1ULL);
	ok &= lxs_expect_u64("and_fan8_explicit.comb_gate_count", loaded.plan->comb_gate_count, 0ULL);

	for (uint32_t combo = 0; combo < 512U; ++combo)
		{
		uint32_t s = (combo >> 8U) & 1U;
		char label[96];

		values[0] = s ? ~0ULL : 0ULL;
		masks[0] = 0ULL;
		for (uint32_t i = 0; i < 8U; ++i)
			{
			uint32_t bit = (combo >> i) & 1U;
			values[i + 1U] = bit ? ~0ULL : 0ULL;
			masks[i + 1U] = 0ULL;
			}

		lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
		lxs_execute_plan(&loaded.ctx, loaded.plan);
		lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);

		for (uint32_t i = 0; i < 8U; ++i)
			{
			uint32_t y = s & ((combo >> i) & 1U);
			snprintf(label, sizeof(label), "and_fan8_explicit.y%u.%u", i, combo);
			ok &= lxs_expect_u64(label, out_values[i], y ? ~0ULL : 0ULL);
			snprintf(label, sizeof(label), "and_fan8_explicit.y%u.mask.%u", i, combo);
			ok &= lxs_expect_u64(label, out_masks[i], 0ULL);
			}
		}

	lxs_unload_case(&loaded);
	return ok;
	}

static int lxs_test_guard_chain4_explicit(void)
	{
	lxs_loaded_case loaded;
	uint64_t values[5];
	uint64_t masks[5];
	uint64_t out_values[5];
	uint64_t out_masks[5];
	int ok = 1;

	if (!lxs_load_case("Tests\\Circuits\\guard_chain4_explicit.bench", &loaded))
		{
		fprintf(stderr, "FAIL guard_chain4_explicit: unable to load test circuit\n");
		return 0;
		}

	ok &= lxs_expect_u64("guard_chain4_explicit.macro_count", loaded.plan->macro_count, 0ULL);
	ok &= lxs_expect_u64("guard_chain4_explicit.multi_macro_count", loaded.plan->multi_macro_count, 1ULL);
	ok &= lxs_expect_u64("guard_chain4_explicit.comb_gate_count", loaded.plan->comb_gate_count, 0ULL);

	for (uint32_t combo = 0; combo < 32U; ++combo)
		{
		uint32_t inv_in = (combo >> 0U) & 1U;
		uint32_t x0 = (combo >> 1U) & 1U;
		uint32_t x1 = (combo >> 2U) & 1U;
		uint32_t x2 = (combo >> 3U) & 1U;
		uint32_t x3 = (combo >> 4U) & 1U;
		uint32_t y0;
		uint32_t y1;
		uint32_t y2;
		uint32_t y3;
		uint32_t inv_out;
		char label[96];

		for (uint32_t i = 0; i < 5U; ++i)
			{
			uint32_t bit = (combo >> i) & 1U;
			values[i] = bit ? ~0ULL : 0ULL;
			masks[i] = 0ULL;
			}

		y0 = x0 & inv_in;
		y1 = x1 & (y0 ? 0U : 1U);
		y2 = x2 & (y1 ? 0U : 1U);
		y3 = x3 & (y2 ? 0U : 1U);
		inv_out = y3 ? 0U : 1U;

		lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
		lxs_execute_plan(&loaded.ctx, loaded.plan);
		lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);

		snprintf(label, sizeof(label), "guard_chain4_explicit.y0.%u", combo);
		ok &= lxs_expect_u64(label, out_values[0], y0 ? ~0ULL : 0ULL);
		snprintf(label, sizeof(label), "guard_chain4_explicit.y1.%u", combo);
		ok &= lxs_expect_u64(label, out_values[1], y1 ? ~0ULL : 0ULL);
		snprintf(label, sizeof(label), "guard_chain4_explicit.y2.%u", combo);
		ok &= lxs_expect_u64(label, out_values[2], y2 ? ~0ULL : 0ULL);
		snprintf(label, sizeof(label), "guard_chain4_explicit.y3.%u", combo);
		ok &= lxs_expect_u64(label, out_values[3], y3 ? ~0ULL : 0ULL);
		snprintf(label, sizeof(label), "guard_chain4_explicit.inv_out.%u", combo);
		ok &= lxs_expect_u64(label, out_values[4], inv_out ? ~0ULL : 0ULL);

		for (uint32_t i = 0; i < 5U; ++i)
			{
			snprintf(label, sizeof(label), "guard_chain4_explicit.mask%u.%u", i, combo);
			ok &= lxs_expect_u64(label, out_masks[i], 0ULL);
			}
		}

	lxs_unload_case(&loaded);
	return ok;
	}

static int lxs_test_xnor_bank4_explicit(void)
	{
	lxs_loaded_case loaded;
	uint64_t values[8];
	uint64_t masks[8];
	uint64_t out_values[4];
	uint64_t out_masks[4];
	int ok = 1;

	if (!lxs_load_case("Tests\\Circuits\\xnor_bank4_explicit.bench", &loaded))
		{
		fprintf(stderr, "FAIL xnor_bank4_explicit: unable to load test circuit\n");
		return 0;
		}

	ok &= lxs_expect_u64("xnor_bank4_explicit.macro_count", loaded.plan->macro_count, 0ULL);
	ok &= lxs_expect_u64("xnor_bank4_explicit.multi_macro_count", loaded.plan->multi_macro_count, 1ULL);
	ok &= lxs_expect_u64("xnor_bank4_explicit.comb_gate_count", loaded.plan->comb_gate_count, 0ULL);

	for (uint32_t combo = 0; combo < 256U; ++combo)
		{
		char label[96];

		for (uint32_t i = 0; i < 8U; ++i)
			{
			uint32_t bit = (combo >> i) & 1U;
			values[i] = bit ? ~0ULL : 0ULL;
			masks[i] = 0ULL;
			}

		lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
		lxs_execute_plan(&loaded.ctx, loaded.plan);
		lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);

		for (uint32_t i = 0; i < 4U; ++i)
			{
			uint32_t lhs = (combo >> (i * 2U)) & 1U;
			uint32_t rhs = (combo >> (i * 2U + 1U)) & 1U;
			uint32_t eq = lhs == rhs ? 1U : 0U;

			snprintf(label, sizeof(label), "xnor_bank4_explicit.eq%u.%u", i, combo);
			ok &= lxs_expect_u64(label, out_values[i], eq ? ~0ULL : 0ULL);
			snprintf(label, sizeof(label), "xnor_bank4_explicit.mask%u.%u", i, combo);
			ok &= lxs_expect_u64(label, out_masks[i], 0ULL);
			}
		}

	lxs_unload_case(&loaded);
	return ok;
	}

static int lxs_test_xnor_bank4_recognition(void)
	{
	lxs_loaded_case loaded;
	uint64_t values[8];
	uint64_t masks[8];
	uint64_t out_values[4];
	uint64_t out_masks[4];
	int ok = 1;

	if (!lxs_set_env_var("LXS_RECOGNITION_MASK", "8"))
		{
		fprintf(stderr, "FAIL xnor_bank4_recognition: unable to set recognition mask\n");
		return 0;
		}

	if (!lxs_load_case("Tests\\Circuits\\xnor_bank4_primitive.bench", &loaded))
		{
		fprintf(stderr, "FAIL xnor_bank4_recognition: unable to load test circuit\n");
		lxs_set_env_var("LXS_RECOGNITION_MASK", NULL);
		return 0;
		}

	ok &= lxs_expect_u64("xnor_bank4_recognition.macro_count", loaded.plan->macro_count, 0ULL);
	ok &= lxs_expect_u64("xnor_bank4_recognition.multi_macro_count", loaded.plan->multi_macro_count, 1ULL);
	ok &= lxs_expect_u64("xnor_bank4_recognition.comb_gate_count", loaded.plan->comb_gate_count, 0ULL);
	ok &= lxs_expect_u64(
		"xnor_bank4_recognition.match_count",
		loaded.plan->recognition_match_count[LXS_RECOGNITION_FAMILY_COMPARE],
		1ULL);
	ok &= lxs_expect_u64(
		"xnor_bank4_recognition.node_reduction",
		loaded.plan->recognition_node_reduction[LXS_RECOGNITION_FAMILY_COMPARE],
		7ULL);

	for (uint32_t combo = 0; combo < 256U; ++combo)
		{
		char label[96];

		for (uint32_t i = 0; i < 8U; ++i)
			{
			uint32_t bit = (combo >> i) & 1U;
			values[i] = bit ? ~0ULL : 0ULL;
			masks[i] = 0ULL;
			}

		lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
		lxs_execute_plan(&loaded.ctx, loaded.plan);
		lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);

		for (uint32_t i = 0; i < 4U; ++i)
			{
			uint32_t lhs = (combo >> (i * 2U)) & 1U;
			uint32_t rhs = (combo >> (i * 2U + 1U)) & 1U;
			uint32_t eq = lhs == rhs ? 1U : 0U;

			snprintf(label, sizeof(label), "xnor_bank4_recognition.eq%u.%u", i, combo);
			ok &= lxs_expect_u64(label, out_values[i], eq ? ~0ULL : 0ULL);
			snprintf(label, sizeof(label), "xnor_bank4_recognition.mask%u.%u", i, combo);
			ok &= lxs_expect_u64(label, out_masks[i], 0ULL);
			}
		}

	lxs_unload_case(&loaded);
	lxs_set_env_var("LXS_RECOGNITION_MASK", NULL);
	return ok;
	}

static int lxs_test_xnor_bank4_recognition_negative(void)
	{
	lxs_loaded_case loaded;
	int ok = 1;

	if (!lxs_set_env_var("LXS_RECOGNITION_MASK", "8"))
		{
		fprintf(stderr, "FAIL xnor_bank4_recognition_negative: unable to set recognition mask\n");
		return 0;
		}

	if (!lxs_load_case("Tests\\Circuits\\xnor_bank4_negative.bench", &loaded))
		{
		fprintf(stderr, "FAIL xnor_bank4_recognition_negative: unable to load test circuit\n");
		lxs_set_env_var("LXS_RECOGNITION_MASK", NULL);
		return 0;
		}

	ok &= lxs_expect_u64("xnor_bank4_recognition_negative.multi_macro_count", loaded.plan->multi_macro_count, 0ULL);
	ok &= lxs_expect_u64("xnor_bank4_recognition_negative.comb_gate_count", loaded.plan->comb_gate_count, 9ULL);
	ok &= lxs_expect_u64(
		"xnor_bank4_recognition_negative.match_count",
		loaded.plan->recognition_match_count[LXS_RECOGNITION_FAMILY_COMPARE],
		0ULL);
	ok &= lxs_expect_u64(
		"xnor_bank4_recognition_negative.node_reduction",
		loaded.plan->recognition_node_reduction[LXS_RECOGNITION_FAMILY_COMPARE],
		0ULL);

	lxs_unload_case(&loaded);
	lxs_set_env_var("LXS_RECOGNITION_MASK", NULL);
	return ok;
	}

static int lxs_test_compare_and4_recognition(void)
	{
	lxs_loaded_case loaded;
	uint64_t values[8];
	uint64_t masks[8];
	uint64_t out_values[1];
	uint64_t out_masks[1];
	int ok = 1;

	if (!lxs_set_env_var("LXS_RECOGNITION_MASK", "8"))
		{
		fprintf(stderr, "FAIL compare_and4_recognition: unable to set recognition mask\n");
		return 0;
		}

	if (!lxs_load_case("Tests\\Circuits\\compare_and4_primitive.bench", &loaded))
		{
		fprintf(stderr, "FAIL compare_and4_recognition: unable to load test circuit\n");
		lxs_set_env_var("LXS_RECOGNITION_MASK", NULL);
		return 0;
		}

	ok &= lxs_expect_u64("compare_and4_recognition.multi_macro_count", loaded.plan->multi_macro_count, 1ULL);
	ok &= lxs_expect_u64("compare_and4_recognition.comb_gate_count", loaded.plan->comb_gate_count, 4ULL);
	ok &= lxs_expect_u64(
		"compare_and4_recognition.match_count",
		loaded.plan->recognition_match_count[LXS_RECOGNITION_FAMILY_COMPARE],
		1ULL);
	ok &= lxs_expect_u64(
		"compare_and4_recognition.node_reduction",
		loaded.plan->recognition_node_reduction[LXS_RECOGNITION_FAMILY_COMPARE],
		2ULL);

	for (uint32_t combo = 0; combo < 256U; ++combo)
		{
		char label[96];
		uint32_t x0;
		uint32_t x1;
		uint32_t x2;
		uint32_t x3;
		uint32_t expected;

		for (uint32_t i = 0; i < 8U; ++i)
			{
			uint32_t bit = (combo >> i) & 1U;
			values[i] = bit ? ~0ULL : 0ULL;
			masks[i] = 0ULL;
			}

		x0 = ((combo >> 0U) & 1U) ^ ((combo >> 1U) & 1U);
		x1 = ((combo >> 2U) & 1U) ^ ((combo >> 3U) & 1U);
		x2 = ((combo >> 4U) & 1U) ^ ((combo >> 5U) & 1U);
		x3 = ((combo >> 6U) & 1U) ^ ((combo >> 7U) & 1U);
		expected = ((x0 ^ 1U) & x1 & (x2 ^ 1U) & x3);

		lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
		lxs_execute_plan(&loaded.ctx, loaded.plan);
		lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);

		snprintf(label, sizeof(label), "compare_and4_recognition.out.%u", combo);
		ok &= lxs_expect_u64(label, out_values[0], expected ? ~0ULL : 0ULL);
		snprintf(label, sizeof(label), "compare_and4_recognition.mask.%u", combo);
		ok &= lxs_expect_u64(label, out_masks[0], 0ULL);
		}

	lxs_unload_case(&loaded);
	lxs_set_env_var("LXS_RECOGNITION_MASK", NULL);
	return ok;
	}

static int lxs_test_compare_and4_recognition_negative(void)
	{
	lxs_loaded_case loaded;
	int ok = 1;

	if (!lxs_set_env_var("LXS_RECOGNITION_MASK", "8"))
		{
		fprintf(stderr, "FAIL compare_and4_recognition_negative: unable to set recognition mask\n");
		return 0;
		}

	if (!lxs_load_case("Tests\\Circuits\\compare_and4_negative.bench", &loaded))
		{
		fprintf(stderr, "FAIL compare_and4_recognition_negative: unable to load test circuit\n");
		lxs_set_env_var("LXS_RECOGNITION_MASK", NULL);
		return 0;
		}

	ok &= lxs_expect_u64("compare_and4_recognition_negative.multi_macro_count", loaded.plan->multi_macro_count, 0ULL);
	ok &= lxs_expect_u64("compare_and4_recognition_negative.comb_gate_count", loaded.plan->comb_gate_count, 8ULL);
	ok &= lxs_expect_u64(
		"compare_and4_recognition_negative.match_count",
		loaded.plan->recognition_match_count[LXS_RECOGNITION_FAMILY_COMPARE],
		0ULL);
	ok &= lxs_expect_u64(
		"compare_and4_recognition_negative.node_reduction",
		loaded.plan->recognition_node_reduction[LXS_RECOGNITION_FAMILY_COMPARE],
		0ULL);

	lxs_unload_case(&loaded);
	lxs_set_env_var("LXS_RECOGNITION_MASK", NULL);
	return ok;
	}

static int lxs_test_compare_or4_recognition(void)
	{
	lxs_loaded_case loaded;
	uint64_t values[8];
	uint64_t masks[8];
	uint64_t out_values[1];
	uint64_t out_masks[1];
	int ok = 1;

	if (!lxs_set_env_var("LXS_RECOGNITION_MASK", "8"))
		{
		fprintf(stderr, "FAIL compare_or4_recognition: unable to set recognition mask\n");
		return 0;
		}

	if (!lxs_load_case("Tests\\Circuits\\compare_or4_primitive.bench", &loaded))
		{
		fprintf(stderr, "FAIL compare_or4_recognition: unable to load test circuit\n");
		lxs_set_env_var("LXS_RECOGNITION_MASK", NULL);
		return 0;
		}

	ok &= lxs_expect_u64("compare_or4_recognition.multi_macro_count", loaded.plan->multi_macro_count, 1ULL);
	ok &= lxs_expect_u64("compare_or4_recognition.comb_gate_count", loaded.plan->comb_gate_count, 4ULL);
	ok &= lxs_expect_u64(
		"compare_or4_recognition.match_count",
		loaded.plan->recognition_match_count[LXS_RECOGNITION_FAMILY_COMPARE],
		1ULL);
	ok &= lxs_expect_u64(
		"compare_or4_recognition.node_reduction",
		loaded.plan->recognition_node_reduction[LXS_RECOGNITION_FAMILY_COMPARE],
		8ULL);

	for (uint32_t combo = 0; combo < 256U; ++combo)
		{
		char label[96];
		uint32_t x0;
		uint32_t x1;
		uint32_t x2;
		uint32_t x3;
		uint32_t expected;

		for (uint32_t i = 0; i < 8U; ++i)
			{
			uint32_t bit = (combo >> i) & 1U;
			values[i] = bit ? ~0ULL : 0ULL;
			masks[i] = 0ULL;
			}

		x0 = ((combo >> 0U) & 1U) ^ ((combo >> 1U) & 1U);
		x1 = ((combo >> 2U) & 1U) ^ ((combo >> 3U) & 1U);
		x2 = ((combo >> 4U) & 1U) ^ ((combo >> 5U) & 1U);
		x3 = ((combo >> 6U) & 1U) ^ ((combo >> 7U) & 1U);
		expected =
			(((x0 ^ 1U) & (x1 ^ 1U) & (x2 ^ 1U) & x3) |
			 ((x0 ^ 1U) & (x1 ^ 1U) & x2 & (x3 ^ 1U)) |
			 ((x0 ^ 1U) & x1 & (x2 ^ 1U) & (x3 ^ 1U)) |
			 (x0 & (x1 ^ 1U) & (x2 ^ 1U) & (x3 ^ 1U)));

		lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
		lxs_execute_plan(&loaded.ctx, loaded.plan);
		lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);

		snprintf(label, sizeof(label), "compare_or4_recognition.out.%u", combo);
		ok &= lxs_expect_u64(label, out_values[0], expected ? ~0ULL : 0ULL);
		snprintf(label, sizeof(label), "compare_or4_recognition.mask.%u", combo);
		ok &= lxs_expect_u64(label, out_masks[0], 0ULL);
		}

	lxs_unload_case(&loaded);
	lxs_set_env_var("LXS_RECOGNITION_MASK", NULL);
	return ok;
	}

static int lxs_test_ripple_slice2_cinv_recognition(void)
	{
	lxs_loaded_case loaded;
	uint64_t values[5];
	uint64_t masks[5];
	uint64_t out_values[3];
	uint64_t out_masks[3];
	int ok = 1;

	if (!lxs_set_env_var("LXS_RECOGNITION_MASK", "32"))
		{
		fprintf(stderr, "FAIL ripple_slice2_cinv_recognition: unable to set recognition mask\n");
		return 0;
		}

	if (!lxs_load_case("Tests\\Circuits\\ripple_slice2_cinv_primitive.bench", &loaded))
		{
		fprintf(stderr, "FAIL ripple_slice2_cinv_recognition: unable to load test circuit\n");
		lxs_set_env_var("LXS_RECOGNITION_MASK", NULL);
		return 0;
		}

	ok &= lxs_expect_u64("ripple_slice2_cinv_recognition.macro_count", loaded.plan->macro_count, 0ULL);
	ok &= lxs_expect_u64("ripple_slice2_cinv_recognition.multi_macro_count", loaded.plan->multi_macro_count, 1ULL);
	ok &= lxs_expect_u64("ripple_slice2_cinv_recognition.comb_gate_count", loaded.plan->comb_gate_count, 0ULL);
	ok &= lxs_expect_u64(
		"ripple_slice2_cinv_recognition.match_count",
		loaded.plan->recognition_match_count[LXS_RECOGNITION_FAMILY_ARITHMETIC],
		1ULL);
	ok &= lxs_expect_u64(
		"ripple_slice2_cinv_recognition.node_reduction",
		loaded.plan->recognition_node_reduction[LXS_RECOGNITION_FAMILY_ARITHMETIC],
		11ULL);

	for (uint32_t combo = 0; combo < 32U; ++combo)
		{
		uint32_t a0 = (combo >> 4U) & 1U;
		uint32_t b0 = (combo >> 3U) & 1U;
		uint32_t a1 = (combo >> 2U) & 1U;
		uint32_t b1 = (combo >> 1U) & 1U;
		uint32_t cin_n = combo & 1U;
		uint32_t cin = cin_n ? 0U : 1U;
		uint32_t value = (a0 + b0 + cin) + ((a1 + b1) << 1U);
		uint32_t low_sum = (a0 ^ b0 ^ cin) & 1U;
		uint32_t carry0 = ((a0 & b0) | (a0 & cin) | (b0 & cin)) & 1U;
		uint32_t high_sum = (a1 ^ b1 ^ carry0) & 1U;
		uint32_t carry1 = ((a1 & b1) | (a1 & carry0) | (b1 & carry0)) & 1U;
		char label[112];

		(void)value;
		values[0] = a0 ? ~0ULL : 0ULL;
		values[1] = b0 ? ~0ULL : 0ULL;
		values[2] = a1 ? ~0ULL : 0ULL;
		values[3] = b1 ? ~0ULL : 0ULL;
		values[4] = cin_n ? ~0ULL : 0ULL;
		memset(masks, 0, sizeof(masks));

		lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
		lxs_execute_plan(&loaded.ctx, loaded.plan);
		lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);

		snprintf(label, sizeof(label), "ripple_slice2_cinv_recognition.sum0.%u", combo);
		ok &= lxs_expect_u64(label, out_values[0], low_sum ? ~0ULL : 0ULL);
		snprintf(label, sizeof(label), "ripple_slice2_cinv_recognition.sum1.%u", combo);
		ok &= lxs_expect_u64(label, out_values[1], high_sum ? ~0ULL : 0ULL);
		snprintf(label, sizeof(label), "ripple_slice2_cinv_recognition.cout_n.%u", combo);
		ok &= lxs_expect_u64(label, out_values[2], carry1 ? 0ULL : ~0ULL);
		for (uint32_t i = 0; i < 3U; ++i)
			{
			snprintf(label, sizeof(label), "ripple_slice2_cinv_recognition.mask%u.%u", i, combo);
			ok &= lxs_expect_u64(label, out_masks[i], 0ULL);
			}
		}

	lxs_unload_case(&loaded);
	lxs_set_env_var("LXS_RECOGNITION_MASK", NULL);
	return ok;
	}

static int lxs_test_ripple_slice2_cinv_recognition_negative(void)
	{
	lxs_loaded_case loaded;
	int ok = 1;

	if (!lxs_set_env_var("LXS_RECOGNITION_MASK", "32"))
		{
		fprintf(stderr, "FAIL ripple_slice2_cinv_recognition_negative: unable to set recognition mask\n");
		return 0;
		}

	if (!lxs_load_case("Tests\\Circuits\\ripple_slice2_cinv_negative.bench", &loaded))
		{
		fprintf(stderr, "FAIL ripple_slice2_cinv_recognition_negative: unable to load test circuit\n");
		lxs_set_env_var("LXS_RECOGNITION_MASK", NULL);
		return 0;
		}

	ok &= lxs_expect_u64("ripple_slice2_cinv_recognition_negative.multi_macro_count", loaded.plan->multi_macro_count, 0ULL);
	ok &= lxs_expect_u64(
		"ripple_slice2_cinv_recognition_negative.match_count",
		loaded.plan->recognition_match_count[LXS_RECOGNITION_FAMILY_ARITHMETIC],
		0ULL);
	ok &= lxs_expect_u64(
		"ripple_slice2_cinv_recognition_negative.node_reduction",
		loaded.plan->recognition_node_reduction[LXS_RECOGNITION_FAMILY_ARITHMETIC],
		0ULL);

	lxs_unload_case(&loaded);
	lxs_set_env_var("LXS_RECOGNITION_MASK", NULL);
	return ok;
	}

static int lxs_test_wide_gates_explicit(void)
	{
	lxs_loaded_case loaded;
	uint64_t values[4];
	uint64_t masks[4];
	uint64_t out_values[6];
	uint64_t out_masks[6];
	int ok = 1;

	if (!lxs_load_case("Tests\\Circuits\\wide_gates_explicit.bench", &loaded))
		{
		fprintf(stderr, "FAIL wide_gates_explicit: unable to load test circuit\n");
		return 0;
		}

	ok &= lxs_expect_u64("wide_gates_explicit.macro_count", loaded.plan->macro_count, 0ULL);
	ok &= lxs_expect_u64("wide_gates_explicit.multi_macro_count", loaded.plan->multi_macro_count, 0ULL);
	ok &= lxs_expect_u64("wide_gates_explicit.comb_gate_count", loaded.plan->comb_gate_count, 6ULL);

	for (uint32_t combo = 0; combo < 16U; ++combo)
		{
		uint32_t a = (combo >> 3U) & 1U;
		uint32_t b = (combo >> 2U) & 1U;
		uint32_t c = (combo >> 1U) & 1U;
		uint32_t d = combo & 1U;
		uint32_t and_y = a & b & c & d;
		uint32_t or_y = a | b | c | d;
		uint32_t xor_y = a ^ b ^ c ^ d;
		char label[96];

		values[0] = a ? ~0ULL : 0ULL;
		values[1] = b ? ~0ULL : 0ULL;
		values[2] = c ? ~0ULL : 0ULL;
		values[3] = d ? ~0ULL : 0ULL;
		memset(masks, 0, sizeof(masks));

		lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
		lxs_execute_plan(&loaded.ctx, loaded.plan);
		lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);

		snprintf(label, sizeof(label), "wide_gates_explicit.and4.%u", combo);
		ok &= lxs_expect_u64(label, out_values[0], and_y ? ~0ULL : 0ULL);
		snprintf(label, sizeof(label), "wide_gates_explicit.or4.%u", combo);
		ok &= lxs_expect_u64(label, out_values[1], or_y ? ~0ULL : 0ULL);
		snprintf(label, sizeof(label), "wide_gates_explicit.xor4.%u", combo);
		ok &= lxs_expect_u64(label, out_values[2], xor_y ? ~0ULL : 0ULL);
		snprintf(label, sizeof(label), "wide_gates_explicit.nand4.%u", combo);
		ok &= lxs_expect_u64(label, out_values[3], and_y ? 0ULL : ~0ULL);
		snprintf(label, sizeof(label), "wide_gates_explicit.nor4.%u", combo);
		ok &= lxs_expect_u64(label, out_values[4], or_y ? 0ULL : ~0ULL);
		snprintf(label, sizeof(label), "wide_gates_explicit.xnor4.%u", combo);
		ok &= lxs_expect_u64(label, out_values[5], xor_y ? 0ULL : ~0ULL);

		for (uint32_t i = 0; i < 6U; ++i)
			{
			snprintf(label, sizeof(label), "wide_gates_explicit.mask%u.%u", i, combo);
			ok &= lxs_expect_u64(label, out_masks[i], 0ULL);
			}
		}

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

	if (!lxs_set_env_var("LXS_RECOGNITION_MASK", "32"))
		{
		fprintf(stderr, "FAIL ripple_slice2_macro: unable to set env\n");
		return 0;
		}

	if (!lxs_load_case("Tests\\Circuits\\ripple_slice2_macro.bench", &loaded))
		{
		fprintf(stderr, "FAIL ripple_slice2_macro: unable to load test circuit\n");
		lxs_set_env_var("LXS_RECOGNITION_MASK", NULL);
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
	lxs_set_env_var("LXS_RECOGNITION_MASK", NULL);
	return ok;
	}

static int lxs_test_half_adder_explicit(void)
	{
	lxs_loaded_case loaded;
	uint64_t values[2];
	uint64_t masks[2];
	uint64_t out_values[2];
	uint64_t out_masks[2];
	int ok = 1;

	if (!lxs_load_case("Tests\\Circuits\\half_adder_explicit.bench", &loaded))
		{
		fprintf(stderr, "FAIL half_adder_explicit: unable to load test circuit\n");
		return 0;
		}

	ok &= lxs_expect_u64("half_adder_explicit.macro_count", loaded.plan->macro_count, 0ULL);
	ok &= lxs_expect_u64("half_adder_explicit.multi_macro_count", loaded.plan->multi_macro_count, 1ULL);
	ok &= lxs_expect_u64("half_adder_explicit.comb_gate_count", loaded.plan->comb_gate_count, 0ULL);

	for (uint32_t combo = 0; combo < 4U; ++combo)
		{
		uint32_t a = (combo >> 1U) & 1U;
		uint32_t b = combo & 1U;
		char label[80];

		values[0] = a ? ~0ULL : 0ULL;
		values[1] = b ? ~0ULL : 0ULL;
		masks[0] = 0ULL;
		masks[1] = 0ULL;

		lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
		lxs_execute_plan(&loaded.ctx, loaded.plan);
		lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);

		snprintf(label, sizeof(label), "half_adder_explicit.sum.%u", combo);
		ok &= lxs_expect_u64(label, out_values[0], (a ^ b) ? ~0ULL : 0ULL);
		snprintf(label, sizeof(label), "half_adder_explicit.carry.%u", combo);
		ok &= lxs_expect_u64(label, out_values[1], (a & b) ? ~0ULL : 0ULL);
		snprintf(label, sizeof(label), "half_adder_explicit.sum.mask.%u", combo);
		ok &= lxs_expect_u64(label, out_masks[0], 0ULL);
		snprintf(label, sizeof(label), "half_adder_explicit.carry.mask.%u", combo);
		ok &= lxs_expect_u64(label, out_masks[1], 0ULL);
		}

	lxs_unload_case(&loaded);
	return ok;
	}

static int lxs_test_full_adder_explicit(void)
	{
	lxs_loaded_case loaded;
	uint64_t values[3];
	uint64_t masks[3];
	uint64_t out_values[2];
	uint64_t out_masks[2];
	int ok = 1;

	if (!lxs_load_case("Tests\\Circuits\\full_adder_explicit.bench", &loaded))
		{
		fprintf(stderr, "FAIL full_adder_explicit: unable to load test circuit\n");
		return 0;
		}

	ok &= lxs_expect_u64("full_adder_explicit.macro_count", loaded.plan->macro_count, 0ULL);
	ok &= lxs_expect_u64("full_adder_explicit.multi_macro_count", loaded.plan->multi_macro_count, 1ULL);
	ok &= lxs_expect_u64("full_adder_explicit.comb_gate_count", loaded.plan->comb_gate_count, 0ULL);

	for (uint32_t combo = 0; combo < 8U; ++combo)
		{
		uint32_t a = (combo >> 2U) & 1U;
		uint32_t b = (combo >> 1U) & 1U;
		uint32_t cin = combo & 1U;
		uint32_t sum = a ^ b ^ cin;
		uint32_t carry = (a & b) | (a & cin) | (b & cin);
		char label[80];

		values[0] = a ? ~0ULL : 0ULL;
		values[1] = b ? ~0ULL : 0ULL;
		values[2] = cin ? ~0ULL : 0ULL;
		memset(masks, 0, sizeof(masks));

		lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
		lxs_execute_plan(&loaded.ctx, loaded.plan);
		lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);

		snprintf(label, sizeof(label), "full_adder_explicit.sum.%u", combo);
		ok &= lxs_expect_u64(label, out_values[0], sum ? ~0ULL : 0ULL);
		snprintf(label, sizeof(label), "full_adder_explicit.carry.%u", combo);
		ok &= lxs_expect_u64(label, out_values[1], carry ? ~0ULL : 0ULL);
		snprintf(label, sizeof(label), "full_adder_explicit.sum.mask.%u", combo);
		ok &= lxs_expect_u64(label, out_masks[0], 0ULL);
		snprintf(label, sizeof(label), "full_adder_explicit.carry.mask.%u", combo);
		ok &= lxs_expect_u64(label, out_masks[1], 0ULL);
		}

	lxs_unload_case(&loaded);
	return ok;
	}

static int lxs_test_ripple_slice2_explicit(void)
	{
	lxs_loaded_case loaded;
	uint64_t values[5];
	uint64_t masks[5];
	uint64_t out_values[3];
	uint64_t out_masks[3];
	int ok = 1;

	if (!lxs_load_case("Tests\\Circuits\\ripple_slice2_explicit.bench", &loaded))
		{
		fprintf(stderr, "FAIL ripple_slice2_explicit: unable to load test circuit\n");
		return 0;
		}

	ok &= lxs_expect_u64("ripple_slice2_explicit.macro_count", loaded.plan->macro_count, 0ULL);
	ok &= lxs_expect_u64("ripple_slice2_explicit.multi_macro_count", loaded.plan->multi_macro_count, 1ULL);
	ok &= lxs_expect_u64("ripple_slice2_explicit.comb_gate_count", loaded.plan->comb_gate_count, 0ULL);

	for (uint32_t combo = 0; combo < 32U; ++combo)
		{
		uint32_t a0 = (combo >> 4U) & 1U;
		uint32_t b0 = (combo >> 3U) & 1U;
		uint32_t a1 = (combo >> 2U) & 1U;
		uint32_t b1 = (combo >> 1U) & 1U;
		uint32_t cin = combo & 1U;
		uint32_t total = (a0 + (a1 << 1U)) + (b0 + (b1 << 1U)) + cin;
		char label[96];

		values[0] = a0 ? ~0ULL : 0ULL;
		values[1] = b0 ? ~0ULL : 0ULL;
		values[2] = a1 ? ~0ULL : 0ULL;
		values[3] = b1 ? ~0ULL : 0ULL;
		values[4] = cin ? ~0ULL : 0ULL;
		memset(masks, 0, sizeof(masks));

		lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
		lxs_execute_plan(&loaded.ctx, loaded.plan);
		lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);

		snprintf(label, sizeof(label), "ripple_slice2_explicit.sum0.%u", combo);
		ok &= lxs_expect_u64(label, out_values[0], (total & 1U) ? ~0ULL : 0ULL);
		snprintf(label, sizeof(label), "ripple_slice2_explicit.sum1.%u", combo);
		ok &= lxs_expect_u64(label, out_values[1], (total & 2U) ? ~0ULL : 0ULL);
		snprintf(label, sizeof(label), "ripple_slice2_explicit.carry.%u", combo);
		ok &= lxs_expect_u64(label, out_values[2], (total & 4U) ? ~0ULL : 0ULL);
		snprintf(label, sizeof(label), "ripple_slice2_explicit.sum0.mask.%u", combo);
		ok &= lxs_expect_u64(label, out_masks[0], 0ULL);
		snprintf(label, sizeof(label), "ripple_slice2_explicit.sum1.mask.%u", combo);
		ok &= lxs_expect_u64(label, out_masks[1], 0ULL);
		snprintf(label, sizeof(label), "ripple_slice2_explicit.carry.mask.%u", combo);
		ok &= lxs_expect_u64(label, out_masks[2], 0ULL);
		}

	lxs_unload_case(&loaded);
	return ok;
	}

static int lxs_test_ripple_add4_explicit(void)
	{
	lxs_loaded_case loaded;
	uint64_t values[9];
	uint64_t masks[9];
	uint64_t out_values[5];
	uint64_t out_masks[5];
	int ok = 1;

	if (!lxs_load_case("Tests\\Circuits\\ripple_add4_explicit.bench", &loaded))
		{
		fprintf(stderr, "FAIL ripple_add4_explicit: unable to load test circuit\n");
		return 0;
		}

	ok &= lxs_expect_u64("ripple_add4_explicit.multi_macro_count", loaded.plan->multi_macro_count, 1ULL);
	ok &= lxs_expect_u64("ripple_add4_explicit.comb_gate_count", loaded.plan->comb_gate_count, 0ULL);

	for (uint32_t combo = 0; combo < 512U; ++combo)
		{
		uint32_t a;
		uint32_t b;
		uint32_t cin;
		uint32_t total;
		char label[96];

		for (uint32_t i = 0; i < 9U; ++i)
			{
			uint32_t bit = (combo >> i) & 1U;
			values[i] = bit ? ~0ULL : 0ULL;
			masks[i] = 0ULL;
			}

		a = ((combo >> 0U) & 1U) |
			(((combo >> 2U) & 1U) << 1U) |
			(((combo >> 4U) & 1U) << 2U) |
			(((combo >> 6U) & 1U) << 3U);
		b = ((combo >> 1U) & 1U) |
			(((combo >> 3U) & 1U) << 1U) |
			(((combo >> 5U) & 1U) << 2U) |
			(((combo >> 7U) & 1U) << 3U);
		cin = (combo >> 8U) & 1U;
		total = a + b + cin;

		lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
		lxs_execute_plan(&loaded.ctx, loaded.plan);
		lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);

		for (uint32_t i = 0; i < 4U; ++i)
			{
			uint32_t bit = (total >> i) & 1U;
			snprintf(label, sizeof(label), "ripple_add4_explicit.sum%u.%u", i, combo);
			ok &= lxs_expect_u64(label, out_values[i], bit ? ~0ULL : 0ULL);
			snprintf(label, sizeof(label), "ripple_add4_explicit.mask%u.%u", i, combo);
			ok &= lxs_expect_u64(label, out_masks[i], 0ULL);
			}

		snprintf(label, sizeof(label), "ripple_add4_explicit.cout.%u", combo);
		ok &= lxs_expect_u64(label, out_values[4], ((total >> 4U) & 1U) ? ~0ULL : 0ULL);
		snprintf(label, sizeof(label), "ripple_add4_explicit.mask4.%u", combo);
		ok &= lxs_expect_u64(label, out_masks[4], 0ULL);
		}

	lxs_unload_case(&loaded);
	return ok;
	}

static int lxs_test_carry_save_row4_explicit(void)
	{
	lxs_loaded_case loaded;
	uint64_t values[12];
	uint64_t masks[12];
	uint64_t out_values[8];
	uint64_t out_masks[8];
	int ok = 1;

	if (!lxs_load_case("Tests\\Circuits\\carry_save_row4_explicit.bench", &loaded))
		{
		fprintf(stderr, "FAIL carry_save_row4_explicit: unable to load test circuit\n");
		return 0;
		}

	ok &= lxs_expect_u64("carry_save_row4_explicit.multi_macro_count", loaded.plan->multi_macro_count, 1ULL);
	ok &= lxs_expect_u64("carry_save_row4_explicit.comb_gate_count", loaded.plan->comb_gate_count, 0ULL);

	for (uint32_t combo = 0; combo < 4096U; ++combo)
		{
		char label[128];

		for (uint32_t i = 0; i < 12U; ++i)
			{
			uint32_t bit = (combo >> i) & 1U;
			values[i] = bit ? ~0ULL : 0ULL;
			masks[i] = 0ULL;
			}

		lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
		lxs_execute_plan(&loaded.ctx, loaded.plan);
		lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);

		for (uint32_t column = 0; column < 4U; ++column)
			{
			uint32_t x = (combo >> (column * 3U + 0U)) & 1U;
			uint32_t y = (combo >> (column * 3U + 1U)) & 1U;
			uint32_t z = (combo >> (column * 3U + 2U)) & 1U;
			uint32_t sum = (x ^ y ^ z) & 1U;
			uint32_t carry = ((x & y) | (x & z) | (y & z)) & 1U;

			snprintf(label, sizeof(label), "carry_save_row4_explicit.sum%u.%u", column, combo);
			ok &= lxs_expect_u64(label, out_values[column], sum ? ~0ULL : 0ULL);
			snprintf(label, sizeof(label), "carry_save_row4_explicit.carry%u.%u", column, combo);
			ok &= lxs_expect_u64(label, out_values[4U + column], carry ? ~0ULL : 0ULL);
			snprintf(label, sizeof(label), "carry_save_row4_explicit.sum_mask%u.%u", column, combo);
			ok &= lxs_expect_u64(label, out_masks[column], 0ULL);
			snprintf(label, sizeof(label), "carry_save_row4_explicit.carry_mask%u.%u", column, combo);
			ok &= lxs_expect_u64(label, out_masks[4U + column], 0ULL);
			}
		}

	lxs_unload_case(&loaded);
	return ok;
	}

static int lxs_test_reduce_propagate4_explicit(void)
	{
	lxs_loaded_case loaded;
	uint64_t values[12];
	uint64_t masks[12];
	uint64_t out_values[6];
	uint64_t out_masks[6];
	int ok = 1;

	if (!lxs_load_case("Tests\\Circuits\\reduce_propagate4_explicit.bench", &loaded))
		{
		fprintf(stderr, "FAIL reduce_propagate4_explicit: unable to load test circuit\n");
		return 0;
		}

	ok &= lxs_expect_u64("reduce_propagate4_explicit.multi_macro_count", loaded.plan->multi_macro_count, 1ULL);
	ok &= lxs_expect_u64("reduce_propagate4_explicit.comb_gate_count", loaded.plan->comb_gate_count, 0ULL);

	for (uint32_t combo = 0; combo < 4096U; ++combo)
		{
		uint32_t total = 0U;
		char label[128];

		for (uint32_t i = 0; i < 12U; ++i)
			{
			uint32_t bit = (combo >> i) & 1U;
			values[i] = bit ? ~0ULL : 0ULL;
			masks[i] = 0ULL;
			}

		for (uint32_t column = 0; column < 4U; ++column)
			{
			uint32_t x = (combo >> (column * 3U + 0U)) & 1U;
			uint32_t y = (combo >> (column * 3U + 1U)) & 1U;
			uint32_t z = (combo >> (column * 3U + 2U)) & 1U;
			total += (x + y + z) << column;
			}

		lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
		lxs_execute_plan(&loaded.ctx, loaded.plan);
		lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);

		for (uint32_t i = 0; i < 6U; ++i)
			{
			uint32_t bit = (total >> i) & 1U;
			snprintf(label, sizeof(label), "reduce_propagate4_explicit.out%u.%u", i, combo);
			ok &= lxs_expect_u64(label, out_values[i], bit ? ~0ULL : 0ULL);
			snprintf(label, sizeof(label), "reduce_propagate4_explicit.mask%u.%u", i, combo);
			ok &= lxs_expect_u64(label, out_masks[i], 0ULL);
			}
		}

	lxs_unload_case(&loaded);
	return ok;
	}

static int lxs_test_functional_region_rowpair_reduce_propagate4_explicit(void)
	{
	lxs_loaded_case lhs;
	lxs_loaded_case micro_case;
	uint64_t values[12];
	uint64_t masks[12];
	uint64_t lhs_out_values[6];
	uint64_t lhs_out_masks[6];
	uint64_t micro_out_values[6];
	uint64_t micro_out_masks[6];
	int ok = 1;

	if (!lxs_load_case("Tests\\Circuits\\reduce_propagate4_primitive.bench", &lhs))
		{
		fprintf(stderr, "FAIL functional_region_rowpair_reduce_propagate4_explicit: unable to load primitive circuit\n");
		return 0;
		}

	if (!lxs_load_case("Tests\\Circuits\\functional_region_rowpair_reduce_propagate4.bench", &micro_case))
		{
		fprintf(stderr, "FAIL functional_region_rowpair_reduce_propagate4_explicit: unable to load functional circuit\n");
		lxs_unload_case(&lhs);
		return 0;
		}

	ok &= lxs_expect_u64("functional_region_rowpair_reduce_propagate4_explicit.count", micro_case.plan->functional_region_count, 1ULL);
	ok &= lxs_expect_u64("functional_region_rowpair_reduce_propagate4_explicit.comb_gate_count", micro_case.plan->comb_gate_count, 0ULL);
	ok &= lxs_expect_u64("functional_region_rowpair_reduce_propagate4_explicit.output_count",
		micro_case.plan->functional_regions[0].output_count,
		6ULL);

	for (uint32_t combo = 0; combo < 4096U; ++combo)
		{
		char label[144];

		for (uint32_t i = 0; i < 12U; ++i)
			{
			uint32_t bit = (combo >> i) & 1U;
			values[i] = bit ? ~0ULL : 0ULL;
			masks[i] = 0ULL;
			}

		lxs_apply_inputs(&lhs.ctx, lhs.plan, values, masks);
		lxs_execute_plan(&lhs.ctx, lhs.plan);
		lxs_read_outputs(&lhs.ctx, lhs.plan, lhs_out_values, lhs_out_masks);

		lxs_apply_inputs(&micro_case.ctx, micro_case.plan, values, masks);
		lxs_execute_plan(&micro_case.ctx, micro_case.plan);
		lxs_read_outputs(&micro_case.ctx, micro_case.plan, micro_out_values, micro_out_masks);

		for (uint32_t i = 0; i < 6U; ++i)
			{
			snprintf(label, sizeof(label), "functional_region_rowpair_reduce_propagate4_explicit.out%u.%u", i, combo);
			ok &= lxs_expect_u64(label, micro_out_values[i], lhs_out_values[i]);
			snprintf(label, sizeof(label), "functional_region_rowpair_reduce_propagate4_explicit.mask%u.%u", i, combo);
			ok &= lxs_expect_u64(label, micro_out_masks[i], lhs_out_masks[i]);
			}
		}

	lxs_unload_case(&lhs);
	lxs_unload_case(&micro_case);
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

static int lxs_test_register_descriptor(void)
	{
	lxs_plan plan;
	lxs_register_plan reg;
	lxs_engine_ctx ctx;
	uint64_t values[8];
	uint64_t masks[8];
	uint64_t out_values[8];
	uint64_t out_masks[8];
	int ok = 1;

	memset(&plan, 0, sizeof(plan));
	memset(&reg, 0, sizeof(reg));
	memset(&ctx, 0, sizeof(ctx));

	plan.net_count = 16U;
	plan.inputs.count = 8U;
	plan.outputs.count = 8U;
	plan.register_count = 1U;
	plan.register_bit_count = 8U;
	plan.inputs.net_ids = (uint32_t*)calloc(8U, sizeof(uint32_t));
	plan.outputs.net_ids = (uint32_t*)calloc(8U, sizeof(uint32_t));
	plan.registers = (lxs_register_plan*)calloc(1U, sizeof(lxs_register_plan));
	plan.register_input_net_ids = (uint32_t*)calloc(8U, sizeof(uint32_t));
	plan.register_output_net_ids = (uint32_t*)calloc(8U, sizeof(uint32_t));
	plan.register_init_value = (uint64_t*)calloc(8U, sizeof(uint64_t));
	plan.register_init_mask = (uint64_t*)calloc(8U, sizeof(uint64_t));
	if (!plan.inputs.net_ids || !plan.outputs.net_ids || !plan.registers ||
		!plan.register_input_net_ids || !plan.register_output_net_ids ||
		!plan.register_init_value || !plan.register_init_mask)
		{
		fprintf(stderr, "FAIL register_descriptor: allocation failure\n");
		ok = 0;
		goto cleanup;
		}

	for (uint32_t bit = 0; bit < 8U; ++bit)
		{
		plan.inputs.net_ids[bit] = bit;
		plan.outputs.net_ids[bit] = 8U + bit;
		plan.register_input_net_ids[bit] = bit;
		plan.register_output_net_ids[bit] = 8U + bit;
		plan.register_init_value[bit] = ((0xA5U >> bit) & 1U) ? ~0ULL : 0ULL;
		plan.register_init_mask[bit] = 0ULL;
		}

	reg.width_bits = 8U;
	reg.input_start = 0U;
	reg.output_start = 0U;
	reg.storage_offset = 0U;
	reg.control_net = UINT32_MAX;
	plan.registers[0] = reg;

	if (!lxs_init_engine(&ctx, &plan))
		{
		fprintf(stderr, "FAIL register_descriptor: engine init failure\n");
		ok = 0;
		goto cleanup;
		}

	lxs_capture_outputs(&ctx, &plan);
	lxs_read_outputs(&ctx, &plan, out_values, out_masks);
	ok &= lxs_expect_u64(
		"register_descriptor.initial",
		lxs_collect_scalar_bits(out_values, out_masks, 8U),
		0xA5U);

	lxs_set_scalar_bits(values, masks, 8U, 0x3CU);
	lxs_apply_inputs(&ctx, &plan, values, masks);
	lxs_execute_plan(&ctx, &plan);
	lxs_read_outputs(&ctx, &plan, out_values, out_masks);
	ok &= lxs_expect_u64(
		"register_descriptor.cycle1.output",
		lxs_collect_scalar_bits(out_values, out_masks, 8U),
		0xA5U);
	ok &= lxs_expect_u64(
		"register_descriptor.cycle1.committed",
		lxs_collect_scalar_bits(ctx.net_value + 8U, ctx.net_mask + 8U, 8U),
		0x3CU);

	lxs_execute_plan(&ctx, &plan);
	lxs_read_outputs(&ctx, &plan, out_values, out_masks);
	ok &= lxs_expect_u64(
		"register_descriptor.cycle2.output",
		lxs_collect_scalar_bits(out_values, out_masks, 8U),
		0x3CU);
	ok &= lxs_expect_u64("register_descriptor.tick_count", ctx.probes.tick_count, 2ULL);
	ok &= lxs_expect_u64("register_descriptor.state_commit_count", ctx.probes.state_commit_count, 16ULL);

cleanup:
	lxs_free_engine(&ctx);
	free(plan.inputs.net_ids);
	free(plan.outputs.net_ids);
	free(plan.registers);
	free(plan.register_input_net_ids);
	free(plan.register_output_net_ids);
	free(plan.register_init_value);
	free(plan.register_init_mask);
	return ok;
	}

static int lxs_test_rom_descriptor(void)
	{
	lxs_plan plan;
	lxs_rom_plan rom;
	lxs_engine_ctx ctx;
	uint64_t values[2];
	uint64_t masks[2];
	uint64_t out_values[8];
	uint64_t out_masks[8];
	const uint8_t contents[4] = { 0x12U, 0x34U, 0x56U, 0x78U };
	int ok = 1;

	memset(&plan, 0, sizeof(plan));
	memset(&rom, 0, sizeof(rom));
	memset(&ctx, 0, sizeof(ctx));

	plan.net_count = 10U;
	plan.inputs.count = 2U;
	plan.outputs.count = 8U;
	plan.rom_count = 1U;
	plan.rom_addr_net_count = 2U;
	plan.rom_output_net_count = 8U;
	plan.rom_bit_count = 32U;
	plan.inputs.net_ids = (uint32_t*)calloc(2U, sizeof(uint32_t));
	plan.outputs.net_ids = (uint32_t*)calloc(8U, sizeof(uint32_t));
	plan.roms = (lxs_rom_plan*)calloc(1U, sizeof(lxs_rom_plan));
	plan.rom_addr_net_ids = (uint32_t*)calloc(2U, sizeof(uint32_t));
	plan.rom_output_net_ids = (uint32_t*)calloc(8U, sizeof(uint32_t));
	plan.rom_init_value = (uint64_t*)calloc(32U, sizeof(uint64_t));
	plan.rom_init_mask = (uint64_t*)calloc(32U, sizeof(uint64_t));
	if (!plan.inputs.net_ids || !plan.outputs.net_ids || !plan.roms ||
		!plan.rom_addr_net_ids || !plan.rom_output_net_ids ||
		!plan.rom_init_value || !plan.rom_init_mask)
		{
		fprintf(stderr, "FAIL rom_descriptor: allocation failure\n");
		ok = 0;
		goto cleanup;
		}

	for (uint32_t bit = 0; bit < 2U; ++bit)
		{
		plan.inputs.net_ids[bit] = bit;
		plan.rom_addr_net_ids[bit] = bit;
		}
	for (uint32_t bit = 0; bit < 8U; ++bit)
		{
		plan.outputs.net_ids[bit] = 2U + bit;
		plan.rom_output_net_ids[bit] = 2U + bit;
		}
	for (uint32_t addr = 0; addr < 4U; ++addr)
		{
		for (uint32_t bit = 0; bit < 8U; ++bit)
			{
			uint32_t index = (addr * 8U) + bit;
			plan.rom_init_value[index] = ((contents[addr] >> bit) & 1U) ? ~0ULL : 0ULL;
			plan.rom_init_mask[index] = 0ULL;
			}
		}

	rom.addr_width = 2U;
	rom.data_width = 8U;
	rom.depth = 4U;
	rom.addr_input_start = 0U;
	rom.output_start = 0U;
	rom.data_offset = 0U;
	plan.roms[0] = rom;

	if (!lxs_init_engine(&ctx, &plan))
		{
		fprintf(stderr, "FAIL rom_descriptor: engine init failure\n");
		ok = 0;
		goto cleanup;
		}

	for (uint32_t addr = 0; addr < 4U; ++addr)
		{
		char label[64];

		lxs_set_scalar_bits(values, masks, 2U, addr);
		lxs_apply_inputs(&ctx, &plan, values, masks);
		lxs_execute_plan(&ctx, &plan);
		lxs_read_outputs(&ctx, &plan, out_values, out_masks);
		snprintf(label, sizeof(label), "rom_descriptor.addr_%u", addr);
		ok &= lxs_expect_u64(
			label,
			lxs_collect_scalar_bits(out_values, out_masks, 8U),
			contents[addr]);
		}

	values[0] = 0ULL;
	values[1] = ~0ULL;
	masks[0] = ~0ULL;
	masks[1] = 0ULL;
	lxs_apply_inputs(&ctx, &plan, values, masks);
	lxs_execute_plan(&ctx, &plan);
	lxs_read_outputs(&ctx, &plan, out_values, out_masks);
	ok &= lxs_expect_u64("rom_descriptor.unknown.mask", out_masks[0], ~0ULL);
	ok &= lxs_expect_u64("rom_descriptor.state_commit_count", ctx.probes.state_commit_count, 0ULL);

cleanup:
	lxs_free_engine(&ctx);
	free(plan.inputs.net_ids);
	free(plan.outputs.net_ids);
	free(plan.roms);
	free(plan.rom_addr_net_ids);
	free(plan.rom_output_net_ids);
	free(plan.rom_init_value);
	free(plan.rom_init_mask);
	return ok;
	}

static int lxs_test_ram_descriptor(void)
	{
	lxs_plan plan;
	lxs_ram_plan ram;
	lxs_engine_ctx ctx;
	uint64_t values[13];
	uint64_t masks[13];
	uint64_t out_values[8];
	uint64_t out_masks[8];
	int ok = 1;

	memset(&plan, 0, sizeof(plan));
	memset(&ram, 0, sizeof(ram));
	memset(&ctx, 0, sizeof(ctx));

	plan.net_count = 21U;
	plan.inputs.count = 13U;
	plan.outputs.count = 8U;
	plan.ram_count = 1U;
	plan.ram_storage_bit_count = 32U;
	plan.ram_stage_bit_count = 8U;
	plan.ram_read_addr_net_count = 2U;
	plan.ram_write_addr_net_count = 2U;
	plan.ram_data_input_net_count = 8U;
	plan.ram_output_net_count = 8U;
	plan.inputs.net_ids = (uint32_t*)calloc(13U, sizeof(uint32_t));
	plan.outputs.net_ids = (uint32_t*)calloc(8U, sizeof(uint32_t));
	plan.rams = (lxs_ram_plan*)calloc(1U, sizeof(lxs_ram_plan));
	plan.ram_read_addr_net_ids = (uint32_t*)calloc(2U, sizeof(uint32_t));
	plan.ram_write_addr_net_ids = (uint32_t*)calloc(2U, sizeof(uint32_t));
	plan.ram_data_input_net_ids = (uint32_t*)calloc(8U, sizeof(uint32_t));
	plan.ram_output_net_ids = (uint32_t*)calloc(8U, sizeof(uint32_t));
	plan.ram_init_value = (uint64_t*)calloc(32U, sizeof(uint64_t));
	plan.ram_init_mask = (uint64_t*)calloc(32U, sizeof(uint64_t));
	if (!plan.inputs.net_ids || !plan.outputs.net_ids || !plan.rams ||
		!plan.ram_read_addr_net_ids || !plan.ram_write_addr_net_ids ||
		!plan.ram_data_input_net_ids || !plan.ram_output_net_ids ||
		!plan.ram_init_value || !plan.ram_init_mask)
		{
		fprintf(stderr, "FAIL ram_descriptor: allocation failure\n");
		ok = 0;
		goto cleanup;
		}

	for (uint32_t net = 0; net < 13U; ++net)
		{
		plan.inputs.net_ids[net] = net;
		}
	for (uint32_t bit = 0; bit < 2U; ++bit)
		{
		plan.ram_read_addr_net_ids[bit] = bit;
		plan.ram_write_addr_net_ids[bit] = 2U + bit;
		}
	for (uint32_t bit = 0; bit < 8U; ++bit)
		{
		plan.ram_data_input_net_ids[bit] = 4U + bit;
		plan.outputs.net_ids[bit] = 13U + bit;
		plan.ram_output_net_ids[bit] = 13U + bit;
		plan.ram_init_value[bit] = ((0x12U >> bit) & 1U) ? ~0ULL : 0ULL;
		plan.ram_init_mask[bit] = 0ULL;
		plan.ram_init_value[8U + bit] = ((0x34U >> bit) & 1U) ? ~0ULL : 0ULL;
		plan.ram_init_mask[8U + bit] = 0ULL;
		}

	ram.addr_width = 2U;
	ram.data_width = 8U;
	ram.depth = 4U;
	ram.read_addr_start = 0U;
	ram.write_addr_start = 0U;
	ram.data_input_start = 0U;
	ram.output_start = 0U;
	ram.storage_offset = 0U;
	ram.stage_offset = 0U;
	ram.write_enable_net = 12U;
	plan.rams[0] = ram;

	if (!lxs_init_engine(&ctx, &plan))
		{
		fprintf(stderr, "FAIL ram_descriptor: engine init failure\n");
		ok = 0;
		goto cleanup;
		}

	memset(values, 0, sizeof(values));
	memset(masks, 0, sizeof(masks));
	lxs_set_scalar_bits(values + 0U, masks + 0U, 2U, 0U);
	lxs_set_scalar_bits(values + 2U, masks + 2U, 2U, 1U);
	lxs_set_scalar_bits(values + 4U, masks + 4U, 8U, 0xA5U);
	values[12] = ~0ULL;
	masks[12] = 0ULL;
	lxs_apply_inputs(&ctx, &plan, values, masks);
	lxs_execute_plan(&ctx, &plan);
	lxs_read_outputs(&ctx, &plan, out_values, out_masks);
	ok &= lxs_expect_u64(
		"ram_descriptor.cycle1.read_old",
		lxs_collect_scalar_bits(out_values, out_masks, 8U),
		0x12U);
	ok &= lxs_expect_u64(
		"ram_descriptor.cycle1.committed_loc1",
		lxs_collect_scalar_bits(ctx.ram_value + 8U, ctx.ram_mask + 8U, 8U),
		0xA5U);

	memset(values, 0, sizeof(values));
	memset(masks, 0, sizeof(masks));
	lxs_set_scalar_bits(values + 0U, masks + 0U, 2U, 1U);
	values[12] = 0ULL;
	masks[12] = 0ULL;
	lxs_apply_inputs(&ctx, &plan, values, masks);
	lxs_execute_plan(&ctx, &plan);
	lxs_read_outputs(&ctx, &plan, out_values, out_masks);
	ok &= lxs_expect_u64(
		"ram_descriptor.cycle2.read_new",
		lxs_collect_scalar_bits(out_values, out_masks, 8U),
		0xA5U);

	memset(values, 0, sizeof(values));
	memset(masks, 0, sizeof(masks));
	values[0] = 0ULL;
	values[1] = ~0ULL;
	masks[0] = ~0ULL;
	masks[1] = 0ULL;
	values[12] = 0ULL;
	masks[12] = 0ULL;
	lxs_apply_inputs(&ctx, &plan, values, masks);
	lxs_execute_plan(&ctx, &plan);
	lxs_read_outputs(&ctx, &plan, out_values, out_masks);
	ok &= lxs_expect_u64("ram_descriptor.unknown.mask", out_masks[0], ~0ULL);
	ok &= lxs_expect_u64("ram_descriptor.tick_count", ctx.probes.tick_count, 3ULL);
	ok &= lxs_expect_u64("ram_descriptor.state_commit_count", ctx.probes.state_commit_count, 8ULL);

cleanup:
	lxs_free_engine(&ctx);
	free(plan.inputs.net_ids);
	free(plan.outputs.net_ids);
	free(plan.rams);
	free(plan.ram_read_addr_net_ids);
	free(plan.ram_write_addr_net_ids);
	free(plan.ram_data_input_net_ids);
	free(plan.ram_output_net_ids);
	free(plan.ram_init_value);
	free(plan.ram_init_mask);
	return ok;
	}

static int lxs_test_register_explicit(void)
	{
	lxs_loaded_case loaded;
	uint64_t values[8];
	uint64_t masks[8];
	uint64_t out_values[8];
	uint64_t out_masks[8];
	int ok = 1;

	if (!lxs_load_case("Tests\\Circuits\\register_explicit.bench", &loaded))
		{
		fprintf(stderr, "FAIL register_explicit: unable to load test circuit\n");
		return 0;
		}

	lxs_set_scalar_bits(values, masks, 8U, 0x5AU);
	lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
	ok &= lxs_expect_u64("register_explicit.cycle1", lxs_collect_scalar_bits(out_values, out_masks, 8U), 0x00U);

	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
	ok &= lxs_expect_u64("register_explicit.cycle2", lxs_collect_scalar_bits(out_values, out_masks, 8U), 0x5AU);

	lxs_unload_case(&loaded);
	return ok;
	}

static int lxs_test_rom_explicit(void)
	{
	lxs_loaded_case loaded;
	uint64_t values[2];
	uint64_t masks[2];
	uint64_t out_values[8];
	uint64_t out_masks[8];
	const uint8_t expected[4] = { 0x12U, 0x34U, 0x56U, 0x78U };
	int ok = 1;

	if (!lxs_load_case("Tests\\Circuits\\rom_explicit.bench", &loaded))
		{
		fprintf(stderr, "FAIL rom_explicit: unable to load test circuit\n");
		return 0;
		}

	for (uint32_t addr = 0; addr < 4U; ++addr)
		{
		char label[64];

		lxs_set_scalar_bits(values, masks, 2U, addr);
		lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
		lxs_execute_plan(&loaded.ctx, loaded.plan);
		lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
		snprintf(label, sizeof(label), "rom_explicit.addr_%u", addr);
		ok &= lxs_expect_u64(label, lxs_collect_scalar_bits(out_values, out_masks, 8U), expected[addr]);
		}

	lxs_unload_case(&loaded);
	return ok;
	}

static int lxs_test_ram_explicit(void)
	{
	lxs_loaded_case loaded;
	uint64_t values[13];
	uint64_t masks[13];
	uint64_t out_values[8];
	uint64_t out_masks[8];
	int ok = 1;

	if (!lxs_load_case("Tests\\Circuits\\ram_explicit.bench", &loaded))
		{
		fprintf(stderr, "FAIL ram_explicit: unable to load test circuit\n");
		return 0;
		}

	memset(values, 0, sizeof(values));
	memset(masks, 0, sizeof(masks));
	lxs_set_scalar_bits(values + 0U, masks + 0U, 2U, 0U);
	lxs_set_scalar_bits(values + 2U, masks + 2U, 2U, 1U);
	lxs_set_scalar_bits(values + 4U, masks + 4U, 8U, 0xA5U);
	values[12] = ~0ULL;
	lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
	ok &= lxs_expect_u64("ram_explicit.cycle1", lxs_collect_scalar_bits(out_values, out_masks, 8U), 0x12U);

	memset(values, 0, sizeof(values));
	memset(masks, 0, sizeof(masks));
	lxs_set_scalar_bits(values + 0U, masks + 0U, 2U, 1U);
	lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
	ok &= lxs_expect_u64("ram_explicit.cycle2", lxs_collect_scalar_bits(out_values, out_masks, 8U), 0xA5U);

	lxs_unload_case(&loaded);
	return ok;
	}

static int lxs_test_register_mixed(void)
	{
	lxs_loaded_case loaded;
	uint64_t values[4];
	uint64_t masks[4];
	uint64_t out_values[6];
	uint64_t out_masks[6];
	int ok = 1;

	if (!lxs_load_case("Tests\\Circuits\\register_mixed.bench", &loaded))
		{
		fprintf(stderr, "FAIL register_mixed: unable to load test circuit\n");
		return 0;
		}

	lxs_set_scalar_bits(values, masks, 4U, 0xAU);
	lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
	ok &= lxs_expect_u64("register_mixed.cycle1.q", lxs_collect_scalar_bits(out_values, out_masks, 4U), 0x0U);
	ok &= lxs_expect_u64("register_mixed.cycle1.parity", out_values[4], 0ULL);
	ok &= lxs_expect_u64("register_mixed.cycle1.nor", out_values[5], ~0ULL);

	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
	ok &= lxs_expect_u64("register_mixed.cycle2.q", lxs_collect_scalar_bits(out_values, out_masks, 4U), 0xAU);
	ok &= lxs_expect_u64("register_mixed.cycle2.parity", out_values[4], 0ULL);
	ok &= lxs_expect_u64("register_mixed.cycle2.nor", out_values[5], 0ULL);

	lxs_unload_case(&loaded);
	return ok;
	}

static int lxs_test_register_en_explicit(void)
	{
	lxs_loaded_case loaded;
	uint64_t values[5];
	uint64_t masks[5];
	uint64_t out_values[4];
	uint64_t out_masks[4];
	int ok = 1;

	if (!lxs_load_case("Tests\\Circuits\\register_en_explicit.bench", &loaded))
		{
		fprintf(stderr, "FAIL register_en_explicit: unable to load test circuit\n");
		return 0;
		}

	memset(values, 0, sizeof(values));
	memset(masks, 0, sizeof(masks));
	lxs_set_scalar_bits(values, masks, 4U, 0x6U);
	values[4] = ~0ULL;
	lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
	ok &= lxs_expect_u64("register_en_explicit.cycle1", lxs_collect_scalar_bits(out_values, out_masks, 4U), 0x0U);

	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
	ok &= lxs_expect_u64("register_en_explicit.cycle2", lxs_collect_scalar_bits(out_values, out_masks, 4U), 0x6U);

	memset(values, 0, sizeof(values));
	memset(masks, 0, sizeof(masks));
	lxs_set_scalar_bits(values, masks, 4U, 0x9U);
	values[4] = 0ULL;
	lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
	ok &= lxs_expect_u64("register_en_explicit.hold", lxs_collect_scalar_bits(out_values, out_masks, 4U), 0x6U);

	lxs_unload_case(&loaded);
	return ok;
	}

static int lxs_test_register_hold_explicit(void)
	{
	lxs_loaded_case loaded;
	uint64_t values[5];
	uint64_t masks[5];
	uint64_t out_values[4];
	uint64_t out_masks[4];
	int ok = 1;

	if (!lxs_load_case("Tests\\Circuits\\register_hold_explicit.bench", &loaded))
		{
		fprintf(stderr, "FAIL register_hold_explicit: unable to load test circuit\n");
		return 0;
		}

	memset(values, 0, sizeof(values));
	memset(masks, 0, sizeof(masks));
	lxs_set_scalar_bits(values, masks, 4U, 0x5U);
	values[4] = 0ULL;
	lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
	ok &= lxs_expect_u64("register_hold_explicit.loaded", lxs_collect_scalar_bits(out_values, out_masks, 4U), 0x5U);

	memset(values, 0, sizeof(values));
	memset(masks, 0, sizeof(masks));
	lxs_set_scalar_bits(values, masks, 4U, 0xCU);
	values[4] = ~0ULL;
	lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
	ok &= lxs_expect_u64("register_hold_explicit.held", lxs_collect_scalar_bits(out_values, out_masks, 4U), 0x5U);

	lxs_unload_case(&loaded);
	return ok;
	}

static int lxs_test_counter_en_explicit(void)
	{
	lxs_loaded_case loaded;
	uint64_t values[1];
	uint64_t masks[1];
	uint64_t out_values[6];
	uint64_t out_masks[6];
	int ok = 1;

	if (!lxs_load_case("Tests\\Circuits\\counter_en_explicit.bench", &loaded))
		{
		fprintf(stderr, "FAIL counter_en_explicit: unable to load test circuit\n");
		return 0;
		}

	values[0] = 0ULL;
	masks[0] = 0ULL;
	lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
	ok &= lxs_expect_u64("counter_en_explicit.cycle1", lxs_collect_scalar_bits(out_values, out_masks, 4U), 0x0U);

	values[0] = ~0ULL;
	lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
	ok &= lxs_expect_u64("counter_en_explicit.step_hidden", lxs_collect_scalar_bits(out_values, out_masks, 4U), 0x0U);

	values[0] = 0ULL;
	lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
	ok &= lxs_expect_u64("counter_en_explicit.cycle2", lxs_collect_scalar_bits(out_values, out_masks, 4U), 0x1U);
	ok &= lxs_expect_u64("counter_en_explicit.parity", out_values[4], ~0ULL);
	ok &= lxs_expect_u64("counter_en_explicit.nor", out_values[5], 0ULL);

	lxs_unload_case(&loaded);
	return ok;
	}

static int lxs_test_counter_updown_explicit(void)
	{
	lxs_loaded_case loaded;
	uint64_t values[2];
	uint64_t masks[2];
	uint64_t out_values[6];
	uint64_t out_masks[6];
	int ok = 1;

	if (!lxs_load_case("Tests\\Circuits\\counter_updown_explicit.bench", &loaded))
		{
		fprintf(stderr, "FAIL counter_updown_explicit: unable to load test circuit\n");
		return 0;
		}

	memset(masks, 0, sizeof(masks));
	values[0] = ~0ULL;
	values[1] = 0ULL;
	lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	values[0] = 0ULL;
	values[1] = 0ULL;
	lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
	ok &= lxs_expect_u64("counter_updown_explicit.inc1", lxs_collect_scalar_bits(out_values, out_masks, 4U), 0x1U);

	values[0] = ~0ULL;
	values[1] = 0ULL;
	lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	values[0] = 0ULL;
	values[1] = 0ULL;
	lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
	ok &= lxs_expect_u64("counter_updown_explicit.inc2", lxs_collect_scalar_bits(out_values, out_masks, 4U), 0x2U);

	values[0] = ~0ULL;
	values[1] = ~0ULL;
	lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	values[0] = 0ULL;
	values[1] = 0ULL;
	lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
	ok &= lxs_expect_u64("counter_updown_explicit.dec1", lxs_collect_scalar_bits(out_values, out_masks, 4U), 0x1U);

	lxs_unload_case(&loaded);
	return ok;
	}

static int lxs_test_rom_mixed(void)
	{
	lxs_loaded_case loaded;
	uint64_t values[2];
	uint64_t masks[2];
	uint64_t out_values[6];
	uint64_t out_masks[6];
	const uint8_t expected_words[4] = { 0x0U, 0x1U, 0x3U, 0x7U };
	const uint8_t expected_parity[4] = { 0U, 1U, 0U, 1U };
	const uint8_t expected_nor[4] = { 1U, 0U, 0U, 0U };
	int ok = 1;

	if (!lxs_load_case("Tests\\Circuits\\rom_mixed.bench", &loaded))
		{
		fprintf(stderr, "FAIL rom_mixed: unable to load test circuit\n");
		return 0;
		}

	for (uint32_t addr = 0; addr < 4U; ++addr)
		{
		char label[64];

		lxs_set_scalar_bits(values, masks, 2U, addr);
		lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
		lxs_execute_plan(&loaded.ctx, loaded.plan);
		lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);

		snprintf(label, sizeof(label), "rom_mixed.word_%u", addr);
		ok &= lxs_expect_u64(label, lxs_collect_scalar_bits(out_values, out_masks, 4U), expected_words[addr]);
		snprintf(label, sizeof(label), "rom_mixed.parity_%u", addr);
		ok &= lxs_expect_u64(label, out_values[4], expected_parity[addr] ? ~0ULL : 0ULL);
		snprintf(label, sizeof(label), "rom_mixed.nor_%u", addr);
		ok &= lxs_expect_u64(label, out_values[5], expected_nor[addr] ? ~0ULL : 0ULL);
		}

	lxs_unload_case(&loaded);
	return ok;
	}

static int lxs_test_ram_mixed(void)
	{
	lxs_loaded_case loaded;
	uint64_t values[9];
	uint64_t masks[9];
	uint64_t out_values[6];
	uint64_t out_masks[6];
	int ok = 1;

	if (!lxs_load_case("Tests\\Circuits\\ram_mixed.bench", &loaded))
		{
		fprintf(stderr, "FAIL ram_mixed: unable to load test circuit\n");
		return 0;
		}

	memset(values, 0, sizeof(values));
	memset(masks, 0, sizeof(masks));
	lxs_set_scalar_bits(values + 0U, masks + 0U, 2U, 0U);
	lxs_set_scalar_bits(values + 2U, masks + 2U, 2U, 1U);
	lxs_set_scalar_bits(values + 4U, masks + 4U, 4U, 0xAU);
	values[8] = ~0ULL;
	lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
	ok &= lxs_expect_u64("ram_mixed.cycle1.word", lxs_collect_scalar_bits(out_values, out_masks, 4U), 0x1U);
	ok &= lxs_expect_u64("ram_mixed.cycle1.parity", out_values[4], ~0ULL);
	ok &= lxs_expect_u64("ram_mixed.cycle1.nor", out_values[5], 0ULL);

	memset(values, 0, sizeof(values));
	memset(masks, 0, sizeof(masks));
	lxs_set_scalar_bits(values + 0U, masks + 0U, 2U, 1U);
	values[8] = 0ULL;
	lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
	ok &= lxs_expect_u64("ram_mixed.cycle2.word", lxs_collect_scalar_bits(out_values, out_masks, 4U), 0xAU);
	ok &= lxs_expect_u64("ram_mixed.cycle2.parity", out_values[4], 0ULL);
	ok &= lxs_expect_u64("ram_mixed.cycle2.nor", out_values[5], 0ULL);

	lxs_unload_case(&loaded);
	return ok;
	}

static int lxs_test_regfile2_explicit(void)
	{
	lxs_loaded_case loaded;
	uint64_t values[7];
	uint64_t masks[7];
	uint64_t out_values[6];
	uint64_t out_masks[6];
	int ok = 1;

	if (!lxs_load_case("Tests\\Circuits\\regfile2_explicit.bench", &loaded))
		{
		fprintf(stderr, "FAIL regfile2_explicit: unable to load test circuit\n");
		return 0;
		}

	memset(values, 0, sizeof(values));
	memset(masks, 0, sizeof(masks));
	values[0] = 0ULL;
	values[1] = ~0ULL;
	lxs_set_scalar_bits(values + 2U, masks + 2U, 4U, 0xAU);
	values[6] = ~0ULL;
	lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
	ok &= lxs_expect_u64("regfile2_explicit.cycle1.word", lxs_collect_scalar_bits(out_values, out_masks, 4U), 0x0U);

	memset(values, 0, sizeof(values));
	memset(masks, 0, sizeof(masks));
	values[0] = ~0ULL;
	values[6] = 0ULL;
	lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
	ok &= lxs_expect_u64("regfile2_explicit.cycle2.word", lxs_collect_scalar_bits(out_values, out_masks, 4U), 0xAU);
	ok &= lxs_expect_u64("regfile2_explicit.parity", out_values[4], 0ULL);
	ok &= lxs_expect_u64("regfile2_explicit.nor", out_values[5], 0ULL);

	lxs_unload_case(&loaded);
	return ok;
	}

static int lxs_test_ram_rmw_xor_explicit(void)
	{
	lxs_loaded_case loaded;
	uint64_t values[9];
	uint64_t masks[9];
	uint64_t out_values[10];
	uint64_t out_masks[10];
	int ok = 1;

	if (!lxs_load_case("Tests\\Circuits\\ram_rmw_xor_explicit.bench", &loaded))
		{
		fprintf(stderr, "FAIL ram_rmw_xor_explicit: unable to load test circuit\n");
		return 0;
		}

	memset(values, 0, sizeof(values));
	memset(masks, 0, sizeof(masks));
	lxs_set_scalar_bits(values + 0U, masks + 0U, 2U, 0U);
	lxs_set_scalar_bits(values + 2U, masks + 2U, 2U, 2U);
	lxs_set_scalar_bits(values + 4U, masks + 4U, 4U, 0x5U);
	values[8] = ~0ULL;
	lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
	ok &= lxs_expect_u64("ram_rmw_xor_explicit.cycle1.mem", lxs_collect_scalar_bits(out_values, out_masks, 4U), 0x1U);
	ok &= lxs_expect_u64("ram_rmw_xor_explicit.cycle1.write_data", lxs_collect_scalar_bits(out_values + 4U, out_masks + 4U, 4U), 0x4U);
	ok &= lxs_expect_u64("ram_rmw_xor_explicit.cycle1.parity", out_values[8], ~0ULL);
	ok &= lxs_expect_u64("ram_rmw_xor_explicit.cycle1.nor", out_values[9], 0ULL);

	memset(values, 0, sizeof(values));
	memset(masks, 0, sizeof(masks));
	lxs_set_scalar_bits(values + 0U, masks + 0U, 2U, 2U);
	values[8] = 0ULL;
	lxs_apply_inputs(&loaded.ctx, loaded.plan, values, masks);
	lxs_execute_plan(&loaded.ctx, loaded.plan);
	lxs_read_outputs(&loaded.ctx, loaded.plan, out_values, out_masks);
	ok &= lxs_expect_u64("ram_rmw_xor_explicit.cycle2.mem", lxs_collect_scalar_bits(out_values, out_masks, 4U), 0x4U);
	ok &= lxs_expect_u64("ram_rmw_xor_explicit.cycle2.write_data", lxs_collect_scalar_bits(out_values + 4U, out_masks + 4U, 4U), 0x4U);
	ok &= lxs_expect_u64("ram_rmw_xor_explicit.cycle2.parity", out_values[8], ~0ULL);
	ok &= lxs_expect_u64("ram_rmw_xor_explicit.cycle2.nor", out_values[9], 0ULL);

	lxs_unload_case(&loaded);
	return ok;
	}

static int lxs_test_rom_lookup_rewrite(void)
	{
	lxs_loaded_case lhs;
	lxs_loaded_case rhs;
	uint64_t values[2];
	uint64_t masks[2];
	uint64_t lhs_out_values[6];
	uint64_t lhs_out_masks[6];
	uint64_t rhs_out_values[6];
	uint64_t rhs_out_masks[6];
	int ok = 1;

	if (!lxs_load_case("Tests\\Circuits\\rom_lookup_primitive.bench", &lhs))
		{
		fprintf(stderr, "FAIL rom_lookup_rewrite: unable to load primitive circuit\n");
		return 0;
		}

	if (!lxs_load_case("Tests\\Circuits\\rom_lookup_rewritten.bench", &rhs))
		{
		fprintf(stderr, "FAIL rom_lookup_rewrite: unable to load rewritten circuit\n");
		lxs_unload_case(&lhs);
		return 0;
		}

	for (uint32_t addr = 0; addr < 4U; ++addr)
		{
		char label[64];

		lxs_set_scalar_bits(values, masks, 2U, addr);
		lxs_apply_inputs(&lhs.ctx, lhs.plan, values, masks);
		lxs_execute_plan(&lhs.ctx, lhs.plan);
		lxs_read_outputs(&lhs.ctx, lhs.plan, lhs_out_values, lhs_out_masks);

		lxs_apply_inputs(&rhs.ctx, rhs.plan, values, masks);
		lxs_execute_plan(&rhs.ctx, rhs.plan);
		lxs_read_outputs(&rhs.ctx, rhs.plan, rhs_out_values, rhs_out_masks);

		for (uint32_t out = 0; out < 6U; ++out)
			{
			snprintf(label, sizeof(label), "rom_lookup_rewrite.addr_%u.out_%u.value", addr, out);
			ok &= lxs_expect_u64(label, rhs_out_values[out], lhs_out_values[out]);
			snprintf(label, sizeof(label), "rom_lookup_rewrite.addr_%u.out_%u.mask", addr, out);
			ok &= lxs_expect_u64(label, rhs_out_masks[out], lhs_out_masks[out]);
			}
		}

	lxs_unload_case(&lhs);
	lxs_unload_case(&rhs);
	return ok;
	}

static int lxs_test_counter_en_rewrite(void)
	{
	lxs_loaded_case lhs;
	lxs_loaded_case rhs;
	uint64_t values[1];
	uint64_t masks[1];
	uint64_t lhs_out_values[6];
	uint64_t lhs_out_masks[6];
	uint64_t rhs_out_values[6];
	uint64_t rhs_out_masks[6];
	uint64_t rng = 0x3141592653589793ULL;
	int ok = 1;

	if (!lxs_load_case("Tests\\Circuits\\counter_en_primitive.bench", &lhs))
		{
		fprintf(stderr, "FAIL counter_en_rewrite: unable to load primitive circuit\n");
		return 0;
		}

	if (!lxs_load_case("Benchmarks\\Generated\\counter_en_rewritten.bench", &rhs))
		{
		fprintf(stderr, "FAIL counter_en_rewrite: unable to load rewritten circuit\n");
		lxs_unload_case(&lhs);
		return 0;
		}

	memset(masks, 0, sizeof(masks));
	for (uint32_t t = 0; t < 512U; ++t)
		{
		char label[80];

		values[0] = (lxs_next_rand(&rng) >> 63U) ? ~0ULL : 0ULL;
		lxs_apply_inputs(&lhs.ctx, lhs.plan, values, masks);
		lxs_execute_plan(&lhs.ctx, lhs.plan);
		lxs_read_outputs(&lhs.ctx, lhs.plan, lhs_out_values, lhs_out_masks);

		lxs_apply_inputs(&rhs.ctx, rhs.plan, values, masks);
		lxs_execute_plan(&rhs.ctx, rhs.plan);
		lxs_read_outputs(&rhs.ctx, rhs.plan, rhs_out_values, rhs_out_masks);

		for (uint32_t out = 0; out < 6U; ++out)
			{
			snprintf(label, sizeof(label), "counter_en_rewrite.tick_%u.out_%u.value", t, out);
			ok &= lxs_expect_u64(label, rhs_out_values[out], lhs_out_values[out]);
			snprintf(label, sizeof(label), "counter_en_rewrite.tick_%u.out_%u.mask", t, out);
			ok &= lxs_expect_u64(label, rhs_out_masks[out], lhs_out_masks[out]);
			}
		}

	lxs_unload_case(&lhs);
	lxs_unload_case(&rhs);
	return ok;
	}

static int lxs_test_regfile2_rewrite(void)
	{
	lxs_loaded_case lhs;
	lxs_loaded_case rhs;
	uint64_t values[7];
	uint64_t masks[7];
	uint64_t lhs_out_values[6];
	uint64_t lhs_out_masks[6];
	uint64_t rhs_out_values[6];
	uint64_t rhs_out_masks[6];
	uint64_t rng = 0x2718281828459045ULL;
	int ok = 1;

	if (!lxs_load_case("Tests\\Circuits\\regfile2_primitive.bench", &lhs))
		{
		fprintf(stderr, "FAIL regfile2_rewrite: unable to load primitive circuit\n");
		return 0;
		}

	if (!lxs_load_case("Benchmarks\\Generated\\regfile2_rewritten.bench", &rhs))
		{
		fprintf(stderr, "FAIL regfile2_rewrite: unable to load rewritten circuit\n");
		lxs_unload_case(&lhs);
		return 0;
		}

	memset(masks, 0, sizeof(masks));
	for (uint32_t t = 0; t < 512U; ++t)
		{
		char label[80];

		for (uint32_t i = 0; i < 7U; ++i)
			{
			values[i] = (lxs_next_rand(&rng) >> 63U) ? ~0ULL : 0ULL;
			}

		lxs_apply_inputs(&lhs.ctx, lhs.plan, values, masks);
		lxs_execute_plan(&lhs.ctx, lhs.plan);
		lxs_read_outputs(&lhs.ctx, lhs.plan, lhs_out_values, lhs_out_masks);

		lxs_apply_inputs(&rhs.ctx, rhs.plan, values, masks);
		lxs_execute_plan(&rhs.ctx, rhs.plan);
		lxs_read_outputs(&rhs.ctx, rhs.plan, rhs_out_values, rhs_out_masks);

		for (uint32_t out = 0; out < 6U; ++out)
			{
			snprintf(label, sizeof(label), "regfile2_rewrite.tick_%u.out_%u.value", t, out);
			ok &= lxs_expect_u64(label, rhs_out_values[out], lhs_out_values[out]);
			snprintf(label, sizeof(label), "regfile2_rewrite.tick_%u.out_%u.mask", t, out);
			ok &= lxs_expect_u64(label, rhs_out_masks[out], lhs_out_masks[out]);
			}
		}

	lxs_unload_case(&lhs);
	lxs_unload_case(&rhs);
	return ok;
	}

static int lxs_test_functional_region_explicit(void)
	{
	lxs_loaded_case lhs;
	lxs_loaded_case expr_case;
	lxs_loaded_case micro_case;
	uint64_t values[5];
	uint64_t masks[5];
	uint64_t lhs_out_values[1];
	uint64_t lhs_out_masks[1];
	uint64_t expr_out_values[1];
	uint64_t expr_out_masks[1];
	uint64_t micro_out_values[1];
	uint64_t micro_out_masks[1];
	int ok = 1;

	if (!lxs_load_case("Tests\\Circuits\\functional_region_primitive.bench", &lhs))
		{
		fprintf(stderr, "FAIL functional_region_explicit: unable to load primitive circuit\n");
		return 0;
		}

	if (!lxs_load_case("Tests\\Circuits\\functional_region_explicit.bench", &expr_case))
		{
		fprintf(stderr, "FAIL functional_region_explicit: unable to load expression circuit\n");
		lxs_unload_case(&lhs);
		return 0;
		}

	if (!lxs_load_case("Tests\\Circuits\\functional_region_micro.bench", &micro_case))
		{
		fprintf(stderr, "FAIL functional_region_explicit: unable to load micro circuit\n");
		lxs_unload_case(&lhs);
		lxs_unload_case(&expr_case);
		return 0;
		}

	ok &= lxs_expect_u64("functional_region_explicit.expr_count", expr_case.plan->functional_region_count, 1ULL);
	ok &= lxs_expect_u64("functional_region_explicit.expr_comb_gate_count", expr_case.plan->comb_gate_count, 0ULL);
	ok &= lxs_expect_u64("functional_region_explicit.expr_exec_kind",
		expr_case.plan->functional_regions[0].exec_kind,
		LXS_FUNCTIONAL_REGION_EXEC_EXPR);
	ok &= lxs_expect_u64("functional_region_explicit.micro_count", micro_case.plan->functional_region_count, 1ULL);
	ok &= lxs_expect_u64("functional_region_explicit.micro_comb_gate_count", micro_case.plan->comb_gate_count, 0ULL);
	ok &= lxs_expect_u64("functional_region_explicit.micro_exec_kind",
		micro_case.plan->functional_regions[0].exec_kind,
		LXS_FUNCTIONAL_REGION_EXEC_MICROPROGRAM);

	memset(masks, 0, sizeof(masks));
	for (uint32_t combo = 0; combo < 32U; ++combo)
		{
		char label[96];

		for (uint32_t bit = 0; bit < 5U; ++bit)
			{
			values[bit] = ((combo >> bit) & 1U) ? ~0ULL : 0ULL;
			}

		lxs_apply_inputs(&lhs.ctx, lhs.plan, values, masks);
		lxs_execute_plan(&lhs.ctx, lhs.plan);
		lxs_read_outputs(&lhs.ctx, lhs.plan, lhs_out_values, lhs_out_masks);

		lxs_apply_inputs(&expr_case.ctx, expr_case.plan, values, masks);
		lxs_execute_plan(&expr_case.ctx, expr_case.plan);
		lxs_read_outputs(&expr_case.ctx, expr_case.plan, expr_out_values, expr_out_masks);

		lxs_apply_inputs(&micro_case.ctx, micro_case.plan, values, masks);
		lxs_execute_plan(&micro_case.ctx, micro_case.plan);
		lxs_read_outputs(&micro_case.ctx, micro_case.plan, micro_out_values, micro_out_masks);

		snprintf(label, sizeof(label), "functional_region_explicit.expr_combo_%u.value", combo);
		ok &= lxs_expect_u64(label, expr_out_values[0], lhs_out_values[0]);
		snprintf(label, sizeof(label), "functional_region_explicit.expr_combo_%u.mask", combo);
		ok &= lxs_expect_u64(label, expr_out_masks[0], lhs_out_masks[0]);
		snprintf(label, sizeof(label), "functional_region_explicit.micro_combo_%u.value", combo);
		ok &= lxs_expect_u64(label, micro_out_values[0], lhs_out_values[0]);
		snprintf(label, sizeof(label), "functional_region_explicit.micro_combo_%u.mask", combo);
		ok &= lxs_expect_u64(label, micro_out_masks[0], lhs_out_masks[0]);
		}

	lxs_unload_case(&lhs);
	lxs_unload_case(&expr_case);
	lxs_unload_case(&micro_case);
	return ok;
	}

static int lxs_test_functional_region_recognition(void)
	{
	lxs_loaded_case lhs;
	lxs_loaded_case rhs;
	uint64_t values[6];
	uint64_t masks[6];
	uint64_t lhs_out_values[1];
	uint64_t lhs_out_masks[1];
	uint64_t rhs_out_values[1];
	uint64_t rhs_out_masks[1];
	int ok = 1;

	if (!lxs_set_env_var("LXS_RECOGNITION_MASK", "128") ||
		!lxs_set_env_var("LXS_RECOGNITION_MODE", "report_only"))
		{
		fprintf(stderr, "FAIL functional_region_recognition: unable to set env\n");
		return 0;
		}

	if (!lxs_load_case("Tests\\Circuits\\functional_region_merge_primitive.bench", &lhs))
		{
		fprintf(stderr, "FAIL functional_region_recognition: unable to load primitive circuit\n");
		lxs_set_env_var("LXS_RECOGNITION_MODE", NULL);
		lxs_set_env_var("LXS_RECOGNITION_MASK", NULL);
		return 0;
		}

	if (!lxs_set_env_var("LXS_RECOGNITION_MASK", "0") ||
		!lxs_set_env_var("LXS_RECOGNITION_MODE", "replace") ||
		!lxs_load_case("Tests\\Circuits\\functional_region_merge_primitive.bench", &rhs))
		{
		fprintf(stderr, "FAIL functional_region_recognition: unable to load reference circuit\n");
		lxs_unload_case(&lhs);
		lxs_set_env_var("LXS_RECOGNITION_MODE", NULL);
		lxs_set_env_var("LXS_RECOGNITION_MASK", NULL);
		return 0;
		}

	ok &= lxs_expect_u64("functional_region_recognition.count", lhs.plan->functional_region_count, 0ULL);
	ok &= lxs_expect_u64("functional_region_recognition.match_count",
		lhs.plan->recognition_match_count[LXS_RECOGNITION_FAMILY_FUNCTIONAL], 2ULL);
	ok &= lxs_expect_u64("functional_region_recognition.comb_gate_count", lhs.plan->comb_gate_count, 5ULL);

	memset(masks, 0, sizeof(masks));
	for (uint32_t combo = 0; combo < 64U; ++combo)
		{
		char label[96];

		for (uint32_t bit = 0; bit < 6U; ++bit)
			{
			values[bit] = ((combo >> bit) & 1U) ? ~0ULL : 0ULL;
			}

		lxs_apply_inputs(&lhs.ctx, lhs.plan, values, masks);
		lxs_execute_plan(&lhs.ctx, lhs.plan);
		lxs_read_outputs(&lhs.ctx, lhs.plan, lhs_out_values, lhs_out_masks);

		lxs_apply_inputs(&rhs.ctx, rhs.plan, values, masks);
		lxs_execute_plan(&rhs.ctx, rhs.plan);
		lxs_read_outputs(&rhs.ctx, rhs.plan, rhs_out_values, rhs_out_masks);

		snprintf(label, sizeof(label), "functional_region_recognition.combo_%u.value", combo);
		ok &= lxs_expect_u64(label, lhs_out_values[0], rhs_out_values[0]);
		snprintf(label, sizeof(label), "functional_region_recognition.combo_%u.mask", combo);
		ok &= lxs_expect_u64(label, lhs_out_masks[0], rhs_out_masks[0]);
		}

	lxs_unload_case(&lhs);
	lxs_unload_case(&rhs);
	lxs_set_env_var("LXS_RECOGNITION_MODE", NULL);
	lxs_set_env_var("LXS_RECOGNITION_MASK", NULL);
	return ok;
	}

static int lxs_test_functional_region_full_adder_explicit(void)
	{
	lxs_loaded_case lhs;
	lxs_loaded_case micro_case;
	uint64_t values[3];
	uint64_t masks[3];
	uint64_t lhs_out_values[2];
	uint64_t lhs_out_masks[2];
	uint64_t micro_out_values[2];
	uint64_t micro_out_masks[2];
	int ok = 1;

	if (!lxs_load_case("Tests\\Circuits\\functional_region_full_adder_primitive.bench", &lhs))
		{
		fprintf(stderr, "FAIL functional_region_full_adder_explicit: unable to load primitive circuit\n");
		return 0;
		}

	if (!lxs_load_case("Tests\\Circuits\\functional_region_full_adder_micro.bench", &micro_case))
		{
		fprintf(stderr, "FAIL functional_region_full_adder_explicit: unable to load micro circuit\n");
		lxs_unload_case(&lhs);
		return 0;
		}

	ok &= lxs_expect_u64("functional_region_full_adder_explicit.count", micro_case.plan->functional_region_count, 1ULL);
	ok &= lxs_expect_u64("functional_region_full_adder_explicit.comb_gate_count", micro_case.plan->comb_gate_count, 0ULL);
	ok &= lxs_expect_u64("functional_region_full_adder_explicit.output_count",
		micro_case.plan->functional_regions[0].output_count,
		2ULL);
	ok &= lxs_expect_u64("functional_region_full_adder_explicit.exec_kind",
		micro_case.plan->functional_regions[0].exec_kind,
		LXS_FUNCTIONAL_REGION_EXEC_MICROPROGRAM);

	memset(masks, 0, sizeof(masks));
	for (uint32_t combo = 0; combo < 8U; ++combo)
		{
		char label[112];

		for (uint32_t bit = 0; bit < 3U; ++bit)
			{
			values[bit] = ((combo >> bit) & 1U) ? ~0ULL : 0ULL;
			}

		lxs_apply_inputs(&lhs.ctx, lhs.plan, values, masks);
		lxs_execute_plan(&lhs.ctx, lhs.plan);
		lxs_read_outputs(&lhs.ctx, lhs.plan, lhs_out_values, lhs_out_masks);

		lxs_apply_inputs(&micro_case.ctx, micro_case.plan, values, masks);
		lxs_execute_plan(&micro_case.ctx, micro_case.plan);
		lxs_read_outputs(&micro_case.ctx, micro_case.plan, micro_out_values, micro_out_masks);

		snprintf(label, sizeof(label), "functional_region_full_adder_explicit.combo_%u.sum.value", combo);
		ok &= lxs_expect_u64(label, micro_out_values[0], lhs_out_values[0]);
		snprintf(label, sizeof(label), "functional_region_full_adder_explicit.combo_%u.sum.mask", combo);
		ok &= lxs_expect_u64(label, micro_out_masks[0], lhs_out_masks[0]);
		snprintf(label, sizeof(label), "functional_region_full_adder_explicit.combo_%u.cout.value", combo);
		ok &= lxs_expect_u64(label, micro_out_values[1], lhs_out_values[1]);
		snprintf(label, sizeof(label), "functional_region_full_adder_explicit.combo_%u.cout.mask", combo);
		ok &= lxs_expect_u64(label, micro_out_masks[1], lhs_out_masks[1]);
		}

	lxs_unload_case(&lhs);
	lxs_unload_case(&micro_case);
	return ok;
	}

static int lxs_test_functional_region_merge_explicit(void)
	{
	lxs_loaded_case lhs;
	lxs_loaded_case micro_case;
	uint64_t values[6];
	uint64_t masks[6];
	uint64_t lhs_out_values[1];
	uint64_t lhs_out_masks[1];
	uint64_t micro_out_values[1];
	uint64_t micro_out_masks[1];
	int ok = 1;

	if (!lxs_load_case("Tests\\Circuits\\functional_region_merge_primitive.bench", &lhs))
		{
		fprintf(stderr, "FAIL functional_region_merge_explicit: unable to load primitive circuit\n");
		return 0;
		}

	if (!lxs_load_case("Tests\\Circuits\\functional_region_merge_micro.bench", &micro_case))
		{
		fprintf(stderr, "FAIL functional_region_merge_explicit: unable to load micro circuit\n");
		lxs_unload_case(&lhs);
		return 0;
		}

	ok &= lxs_expect_u64("functional_region_merge_explicit.count", micro_case.plan->functional_region_count, 1ULL);
	ok &= lxs_expect_u64("functional_region_merge_explicit.comb_gate_count", micro_case.plan->comb_gate_count, 0ULL);
	ok &= lxs_expect_u64("functional_region_merge_explicit.exec_kind",
		micro_case.plan->functional_regions[0].exec_kind,
		LXS_FUNCTIONAL_REGION_EXEC_MICROPROGRAM);

	memset(masks, 0, sizeof(masks));
	for (uint32_t combo = 0; combo < 64U; ++combo)
		{
		char label[104];

		for (uint32_t bit = 0; bit < 6U; ++bit)
			{
			values[bit] = ((combo >> bit) & 1U) ? ~0ULL : 0ULL;
			}

		lxs_apply_inputs(&lhs.ctx, lhs.plan, values, masks);
		lxs_execute_plan(&lhs.ctx, lhs.plan);
		lxs_read_outputs(&lhs.ctx, lhs.plan, lhs_out_values, lhs_out_masks);

		lxs_apply_inputs(&micro_case.ctx, micro_case.plan, values, masks);
		lxs_execute_plan(&micro_case.ctx, micro_case.plan);
		lxs_read_outputs(&micro_case.ctx, micro_case.plan, micro_out_values, micro_out_masks);

		snprintf(label, sizeof(label), "functional_region_merge_explicit.combo_%u.value", combo);
		ok &= lxs_expect_u64(label, micro_out_values[0], lhs_out_values[0]);
		snprintf(label, sizeof(label), "functional_region_merge_explicit.combo_%u.mask", combo);
		ok &= lxs_expect_u64(label, micro_out_masks[0], lhs_out_masks[0]);
		}

	lxs_unload_case(&lhs);
	lxs_unload_case(&micro_case);
	return ok;
	}

static int lxs_test_functional_region_chain_explicit(void)
	{
	lxs_loaded_case lhs;
	lxs_loaded_case micro_case;
	uint64_t values[6];
	uint64_t masks[6];
	uint64_t lhs_out_values[1];
	uint64_t lhs_out_masks[1];
	uint64_t micro_out_values[1];
	uint64_t micro_out_masks[1];
	int ok = 1;

	if (!lxs_load_case("Tests\\Circuits\\functional_region_chain_primitive.bench", &lhs))
		{
		fprintf(stderr, "FAIL functional_region_chain_explicit: unable to load primitive circuit\n");
		return 0;
		}

	if (!lxs_load_case("Tests\\Circuits\\functional_region_chain_micro.bench", &micro_case))
		{
		fprintf(stderr, "FAIL functional_region_chain_explicit: unable to load micro circuit\n");
		lxs_unload_case(&lhs);
		return 0;
		}

	ok &= lxs_expect_u64("functional_region_chain_explicit.count", micro_case.plan->functional_region_count, 1ULL);
	ok &= lxs_expect_u64("functional_region_chain_explicit.comb_gate_count", micro_case.plan->comb_gate_count, 0ULL);
	ok &= lxs_expect_u64("functional_region_chain_explicit.exec_kind",
		micro_case.plan->functional_regions[0].exec_kind,
		LXS_FUNCTIONAL_REGION_EXEC_MICROPROGRAM);

	memset(masks, 0, sizeof(masks));
	for (uint32_t combo = 0; combo < 64U; ++combo)
		{
		char label[104];

		for (uint32_t bit = 0; bit < 6U; ++bit)
			{
			values[bit] = ((combo >> bit) & 1U) ? ~0ULL : 0ULL;
			}

		lxs_apply_inputs(&lhs.ctx, lhs.plan, values, masks);
		lxs_execute_plan(&lhs.ctx, lhs.plan);
		lxs_read_outputs(&lhs.ctx, lhs.plan, lhs_out_values, lhs_out_masks);

		lxs_apply_inputs(&micro_case.ctx, micro_case.plan, values, masks);
		lxs_execute_plan(&micro_case.ctx, micro_case.plan);
		lxs_read_outputs(&micro_case.ctx, micro_case.plan, micro_out_values, micro_out_masks);

		snprintf(label, sizeof(label), "functional_region_chain_explicit.combo_%u.value", combo);
		ok &= lxs_expect_u64(label, micro_out_values[0], lhs_out_values[0]);
		snprintf(label, sizeof(label), "functional_region_chain_explicit.combo_%u.mask", combo);
		ok &= lxs_expect_u64(label, micro_out_masks[0], lhs_out_masks[0]);
		}

	lxs_unload_case(&lhs);
	lxs_unload_case(&micro_case);
	return ok;
	}

static int lxs_test_functional_region_wide_explicit(void)
	{
	lxs_loaded_case lhs;
	lxs_loaded_case expr_case;
	lxs_loaded_case micro_case;
	uint64_t values[6];
	uint64_t masks[6];
	uint64_t lhs_out_values[1];
	uint64_t lhs_out_masks[1];
	uint64_t expr_out_values[1];
	uint64_t expr_out_masks[1];
	uint64_t micro_out_values[1];
	uint64_t micro_out_masks[1];
	int ok = 1;

	if (!lxs_load_case("Tests\\Circuits\\functional_region_wide_primitive.bench", &lhs))
		{
		fprintf(stderr, "FAIL functional_region_wide_explicit: unable to load primitive circuit\n");
		return 0;
		}

	if (!lxs_load_case("Tests\\Circuits\\functional_region_wide_explicit.bench", &expr_case))
		{
		fprintf(stderr, "FAIL functional_region_wide_explicit: unable to load expression circuit\n");
		lxs_unload_case(&lhs);
		return 0;
		}

	if (!lxs_load_case("Tests\\Circuits\\functional_region_wide_micro.bench", &micro_case))
		{
		fprintf(stderr, "FAIL functional_region_wide_explicit: unable to load micro circuit\n");
		lxs_unload_case(&lhs);
		lxs_unload_case(&expr_case);
		return 0;
		}

	ok &= lxs_expect_u64("functional_region_wide_explicit.expr_count", expr_case.plan->functional_region_count, 1ULL);
	ok &= lxs_expect_u64("functional_region_wide_explicit.expr_comb_gate_count", expr_case.plan->comb_gate_count, 0ULL);
	ok &= lxs_expect_u64("functional_region_wide_explicit.expr_exec_kind",
		expr_case.plan->functional_regions[0].exec_kind,
		LXS_FUNCTIONAL_REGION_EXEC_EXPR);
	ok &= lxs_expect_u64("functional_region_wide_explicit.micro_count", micro_case.plan->functional_region_count, 1ULL);
	ok &= lxs_expect_u64("functional_region_wide_explicit.micro_comb_gate_count", micro_case.plan->comb_gate_count, 0ULL);
	ok &= lxs_expect_u64("functional_region_wide_explicit.micro_exec_kind",
		micro_case.plan->functional_regions[0].exec_kind,
		LXS_FUNCTIONAL_REGION_EXEC_MICROPROGRAM);

	memset(masks, 0, sizeof(masks));
	for (uint32_t combo = 0; combo < 64U; ++combo)
		{
		char label[104];

		for (uint32_t bit = 0; bit < 6U; ++bit)
			{
			values[bit] = ((combo >> bit) & 1U) ? ~0ULL : 0ULL;
			}

		lxs_apply_inputs(&lhs.ctx, lhs.plan, values, masks);
		lxs_execute_plan(&lhs.ctx, lhs.plan);
		lxs_read_outputs(&lhs.ctx, lhs.plan, lhs_out_values, lhs_out_masks);

		lxs_apply_inputs(&expr_case.ctx, expr_case.plan, values, masks);
		lxs_execute_plan(&expr_case.ctx, expr_case.plan);
		lxs_read_outputs(&expr_case.ctx, expr_case.plan, expr_out_values, expr_out_masks);

		lxs_apply_inputs(&micro_case.ctx, micro_case.plan, values, masks);
		lxs_execute_plan(&micro_case.ctx, micro_case.plan);
		lxs_read_outputs(&micro_case.ctx, micro_case.plan, micro_out_values, micro_out_masks);

		snprintf(label, sizeof(label), "functional_region_wide_explicit.expr_combo_%u.value", combo);
		ok &= lxs_expect_u64(label, expr_out_values[0], lhs_out_values[0]);
		snprintf(label, sizeof(label), "functional_region_wide_explicit.expr_combo_%u.mask", combo);
		ok &= lxs_expect_u64(label, expr_out_masks[0], lhs_out_masks[0]);
		snprintf(label, sizeof(label), "functional_region_wide_explicit.micro_combo_%u.value", combo);
		ok &= lxs_expect_u64(label, micro_out_values[0], lhs_out_values[0]);
		snprintf(label, sizeof(label), "functional_region_wide_explicit.micro_combo_%u.mask", combo);
		ok &= lxs_expect_u64(label, micro_out_masks[0], lhs_out_masks[0]);
		}

	lxs_unload_case(&lhs);
	lxs_unload_case(&expr_case);
	lxs_unload_case(&micro_case);
	return ok;
	}

static int lxs_test_functional_region_wide_recognition(void)
	{
	lxs_loaded_case loaded;
	int ok = 1;

	if (!lxs_set_env_var("LXS_RECOGNITION_MASK", "128") ||
		!lxs_set_env_var("LXS_RECOGNITION_MODE", "report_only"))
		{
		fprintf(stderr, "FAIL functional_region_wide_recognition: unable to set env\n");
		return 0;
		}

	if (!lxs_load_case("Tests\\Circuits\\functional_region_wide_primitive.bench", &loaded))
		{
		fprintf(stderr, "FAIL functional_region_wide_recognition: unable to load primitive circuit\n");
		lxs_set_env_var("LXS_RECOGNITION_MODE", NULL);
		lxs_set_env_var("LXS_RECOGNITION_MASK", NULL);
		return 0;
		}

	ok &= lxs_expect_u64("functional_region_wide_recognition.match_count",
		loaded.plan->recognition_match_count[LXS_RECOGNITION_FAMILY_FUNCTIONAL], 0ULL);
	ok &= lxs_expect_u64("functional_region_wide_recognition.comb_gate_count", loaded.plan->comb_gate_count, 4ULL);

	lxs_unload_case(&loaded);
	lxs_set_env_var("LXS_RECOGNITION_MODE", NULL);
	lxs_set_env_var("LXS_RECOGNITION_MASK", NULL);
	return ok;
	}

static int lxs_test_functional_region_cache_explicit(void)
	{
	lxs_loaded_case lhs;
	lxs_loaded_case micro_case;
	uint64_t values[5];
	uint64_t masks[5];
	uint64_t lhs_out_values[1];
	uint64_t lhs_out_masks[1];
	uint64_t micro_out_values[1];
	uint64_t micro_out_masks[1];
	int ok = 1;

	if (!lxs_load_case("Tests\\Circuits\\functional_region_cache_primitive.bench", &lhs))
		{
		fprintf(stderr, "FAIL functional_region_cache_explicit: unable to load primitive circuit\n");
		return 0;
		}

	if (!lxs_load_case("Tests\\Circuits\\functional_region_cache_micro.bench", &micro_case))
		{
		fprintf(stderr, "FAIL functional_region_cache_explicit: unable to load micro circuit\n");
		lxs_unload_case(&lhs);
		return 0;
		}

	ok &= lxs_expect_u64("functional_region_cache_explicit.count", micro_case.plan->functional_region_count, 1ULL);
	ok &= lxs_expect_u64("functional_region_cache_explicit.comb_gate_count", micro_case.plan->comb_gate_count, 0ULL);
	ok &= lxs_expect_u64("functional_region_cache_explicit.exec_kind",
		micro_case.plan->functional_regions[0].exec_kind,
		LXS_FUNCTIONAL_REGION_EXEC_MICROPROGRAM);

	memset(masks, 0, sizeof(masks));
	for (uint32_t combo = 0; combo < 32U; ++combo)
		{
		char label[104];

		for (uint32_t bit = 0; bit < 5U; ++bit)
			{
			values[bit] = ((combo >> bit) & 1U) ? ~0ULL : 0ULL;
			}

		lxs_apply_inputs(&lhs.ctx, lhs.plan, values, masks);
		lxs_execute_plan(&lhs.ctx, lhs.plan);
		lxs_read_outputs(&lhs.ctx, lhs.plan, lhs_out_values, lhs_out_masks);

		lxs_apply_inputs(&micro_case.ctx, micro_case.plan, values, masks);
		lxs_execute_plan(&micro_case.ctx, micro_case.plan);
		lxs_read_outputs(&micro_case.ctx, micro_case.plan, micro_out_values, micro_out_masks);

		snprintf(label, sizeof(label), "functional_region_cache_explicit.combo_%u.value", combo);
		ok &= lxs_expect_u64(label, micro_out_values[0], lhs_out_values[0]);
		snprintf(label, sizeof(label), "functional_region_cache_explicit.combo_%u.mask", combo);
		ok &= lxs_expect_u64(label, micro_out_masks[0], lhs_out_masks[0]);
		}

	lxs_unload_case(&lhs);
	lxs_unload_case(&micro_case);
	return ok;
	}

int main(void)
	{
	int ok = 1;

	ok &= lxs_test_comb_chain();
	ok &= lxs_test_mask_and();
	ok &= lxs_test_canonical_basic();
	ok &= lxs_test_multi_macro_full_adder_cinv();
	ok &= lxs_test_mux2_macro();
	ok &= lxs_test_mux2_8_explicit();
	ok &= lxs_test_mux4_8_explicit();
	ok &= lxs_test_reg8_explicit();
	ok &= lxs_test_reg16_explicit();
	ok &= lxs_test_reg_en8_explicit();
	ok &= lxs_test_reg_en16_explicit();
	ok &= lxs_test_reg_en_rst8_explicit();
	ok &= lxs_test_reg_en_rst16_explicit();
	ok &= lxs_test_add8_explicit();
	ok &= lxs_test_add16_explicit();
	ok &= lxs_test_cmp8_explicit();
	ok &= lxs_test_cmp16_explicit();
	ok &= lxs_test_alu8_explicit();
	ok &= lxs_test_alu16_explicit();
	ok &= lxs_test_rom16_explicit();
	ok &= lxs_test_ram8_explicit();
	ok &= lxs_test_ram16_explicit();
	ok &= lxs_test_ram24_explicit();
	ok &= lxs_test_regfile8_explicit();
	ok &= lxs_test_regfile16_explicit();
	ok &= lxs_test_regfile32_explicit();
	ok &= lxs_test_counter8_explicit();
	ok &= lxs_test_counter16_explicit();
	ok &= lxs_test_counter_en8_explicit();
	ok &= lxs_test_counter_en16_explicit();
	ok &= lxs_test_xor2_macro();
	ok &= lxs_test_xor2_nor_macro();
	ok &= lxs_test_xnor2_macro();
	ok &= lxs_test_parity4_explicit();
	ok &= lxs_test_parity8_explicit();
	ok &= lxs_test_parity4_recognition();
	ok &= lxs_test_parity4_recognition_negative();
	ok &= lxs_test_xor_fan8_explicit();
	ok &= lxs_test_xor_fan8_recognition();
	ok &= lxs_test_xor_fan8_recognition_negative();
	ok &= lxs_test_and_fan8_explicit();
	ok &= lxs_test_guard_chain4_explicit();
	ok &= lxs_test_xnor_bank4_explicit();
	ok &= lxs_test_xnor_bank4_recognition();
	ok &= lxs_test_xnor_bank4_recognition_negative();
	ok &= lxs_test_compare_and4_recognition();
	ok &= lxs_test_compare_and4_recognition_negative();
	ok &= lxs_test_compare_or4_recognition();
	ok &= lxs_test_ripple_slice2_cinv_recognition();
	ok &= lxs_test_ripple_slice2_cinv_recognition_negative();
	ok &= lxs_test_wide_gates_explicit();
	ok &= lxs_test_carry_inv2_macro();
	ok &= lxs_test_sum_cinv2_macro();
	ok &= lxs_test_ripple_slice2_macro();
	ok &= lxs_test_half_adder_explicit();
	ok &= lxs_test_full_adder_explicit();
	ok &= lxs_test_ripple_slice2_explicit();
	ok &= lxs_test_ripple_add4_explicit();
	ok &= lxs_test_carry_save_row4_explicit();
	ok &= lxs_test_reduce_propagate4_explicit();
	ok &= lxs_test_functional_region_rowpair_reduce_propagate4_explicit();
	ok &= lxs_test_dff_not();
	ok &= lxs_test_register_descriptor();
	ok &= lxs_test_rom_descriptor();
	ok &= lxs_test_ram_descriptor();
	ok &= lxs_test_register_explicit();
	ok &= lxs_test_rom_explicit();
	ok &= lxs_test_ram_explicit();
	ok &= lxs_test_register_mixed();
	ok &= lxs_test_register_en_explicit();
	ok &= lxs_test_register_hold_explicit();
	ok &= lxs_test_counter_en_explicit();
	ok &= lxs_test_counter_updown_explicit();
	ok &= lxs_test_rom_mixed();
	ok &= lxs_test_rom_lookup_rewrite();
	ok &= lxs_test_ram_mixed();
	ok &= lxs_test_ram_rmw_xor_explicit();
	ok &= lxs_test_regfile2_explicit();
	ok &= lxs_test_counter_en_rewrite();
	ok &= lxs_test_regfile2_rewrite();
	ok &= lxs_test_functional_region_explicit();
	ok &= lxs_test_functional_region_recognition();
	ok &= lxs_test_functional_region_full_adder_explicit();
	ok &= lxs_test_functional_region_cache_explicit();
	ok &= lxs_test_functional_region_chain_explicit();
	ok &= lxs_test_functional_region_merge_explicit();
	ok &= lxs_test_functional_region_wide_explicit();
	ok &= lxs_test_functional_region_wide_recognition();

	if (!ok)
		{
		fprintf(stderr, "LXS tests failed. Check combinational binding, 4-state propagation, or DFF commit semantics.\n");
		return 1;
		}

	printf("LXS tests passed.\n");
	return 0;
	}
