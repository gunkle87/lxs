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

static uint64_t lxs_next_rand(uint64_t *state)
	{
	*state = (*state * 6364136223846793005ULL) + 1ULL;
	return *state;
	}

int main(int argc, char **argv)
	{
	lxs_loaded_case lhs;
	lxs_loaded_case rhs;
	uint64_t *values = NULL;
	uint64_t *masks = NULL;
	uint64_t *lhs_out_values = NULL;
	uint64_t *lhs_out_masks = NULL;
	uint64_t *rhs_out_values = NULL;
	uint64_t *rhs_out_masks = NULL;
	uint64_t rng = 0x123456789abcdef0ULL;
	uint32_t trials = 1024U;
	int ok = 1;

	if (argc < 3)
		{
		fprintf(stderr, "usage: lxs_compare <lhs.bench> <rhs.bench> [trials]\n");
		return 1;
		}

	if (argc >= 4)
		{
		trials = (uint32_t)strtoul(argv[3], NULL, 10);
		}

	if (!lxs_load_case(argv[1], &lhs) || !lxs_load_case(argv[2], &rhs))
		{
		fprintf(stderr, "failed to load comparison cases\n");
		lxs_unload_case(&lhs);
		lxs_unload_case(&rhs);
		return 1;
		}

	if (lhs.plan->inputs.count != rhs.plan->inputs.count ||
		lhs.plan->outputs.count != rhs.plan->outputs.count)
		{
		fprintf(stderr, "mismatched input/output counts\n");
		lxs_unload_case(&lhs);
		lxs_unload_case(&rhs);
		return 1;
		}

	values = calloc(lhs.plan->inputs.count ? lhs.plan->inputs.count : 1U, sizeof(uint64_t));
	masks = calloc(lhs.plan->inputs.count ? lhs.plan->inputs.count : 1U, sizeof(uint64_t));
	lhs_out_values = calloc(lhs.plan->outputs.count ? lhs.plan->outputs.count : 1U, sizeof(uint64_t));
	lhs_out_masks = calloc(lhs.plan->outputs.count ? lhs.plan->outputs.count : 1U, sizeof(uint64_t));
	rhs_out_values = calloc(lhs.plan->outputs.count ? lhs.plan->outputs.count : 1U, sizeof(uint64_t));
	rhs_out_masks = calloc(lhs.plan->outputs.count ? lhs.plan->outputs.count : 1U, sizeof(uint64_t));
	if (!values || !masks || !lhs_out_values || !lhs_out_masks || !rhs_out_values || !rhs_out_masks)
		{
		fprintf(stderr, "failed to allocate comparison buffers\n");
		lxs_unload_case(&lhs);
		lxs_unload_case(&rhs);
		free(values);
		free(masks);
		free(lhs_out_values);
		free(lhs_out_masks);
		free(rhs_out_values);
		free(rhs_out_masks);
		return 1;
		}

	memset(masks, 0, (size_t)(lhs.plan->inputs.count ? lhs.plan->inputs.count : 1U) * sizeof(uint64_t));
	for (uint32_t t = 0; t < trials; ++t)
		{
		for (uint32_t i = 0; i < lhs.plan->inputs.count; ++i)
			{
			values[i] = (lxs_next_rand(&rng) >> 63U) ? ~0ULL : 0ULL;
			}

		lxs_apply_inputs(&lhs.ctx, lhs.plan, values, masks);
		lxs_execute_plan(&lhs.ctx, lhs.plan);
		lxs_read_outputs(&lhs.ctx, lhs.plan, lhs_out_values, lhs_out_masks);

		lxs_apply_inputs(&rhs.ctx, rhs.plan, values, masks);
		lxs_execute_plan(&rhs.ctx, rhs.plan);
		lxs_read_outputs(&rhs.ctx, rhs.plan, rhs_out_values, rhs_out_masks);

		for (uint32_t i = 0; i < lhs.plan->outputs.count; ++i)
			{
			if (lhs_out_values[i] != rhs_out_values[i] || lhs_out_masks[i] != rhs_out_masks[i])
				{
				fprintf(stderr, "mismatch at trial %u output %u\n", t, i);
				ok = 0;
				break;
				}
			}

		if (!ok)
			{
			break;
			}
		}

	lxs_unload_case(&lhs);
	lxs_unload_case(&rhs);
	free(values);
	free(masks);
	free(lhs_out_values);
	free(lhs_out_masks);
	free(rhs_out_values);
	free(rhs_out_masks);

	if (!ok)
		{
		return 1;
		}

	printf("Equivalent across %u trials.\n", trials);
	return 0;
	}
