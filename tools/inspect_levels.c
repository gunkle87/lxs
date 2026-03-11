#include "lxs_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *lxs_find_name(const lxs_netlist *nl, uint32_t net_id)
	{
	for (uint32_t i = 0; i < nl->net_count; ++i)
		{
		if (i == net_id)
			{
			return nl->net_names[i];
			}
		}

	return NULL;
	}

static int lxs_find_net_id(const lxs_netlist *nl, const char *name)
	{
	for (uint32_t i = 0; i < nl->net_count; ++i)
		{
		if (strcmp(nl->net_names[i], name) == 0)
			{
			return (int)i;
			}
		}

	return -1;
	}

int main(int argc, char **argv)
	{
	static const char *watch_names[] =
		{
		"add_1_sum_2",
		"add_1_sum_3",
		"add_1_carry_3",
		"add_2_sum_2",
		"add_2_carry_2",
		"1901"
		};
	lxs_netlist *nl;
	lxs_plan *plan;

	if (argc != 2)
		{
		fprintf(stderr, "usage: inspect_levels <file.bench>\n");
		return 1;
		}

	nl = lxs_load_iscas(argv[1]);
	if (!nl)
		{
		fprintf(stderr, "failed to load %s\n", argv[1]);
		return 1;
		}

	plan = lxs_compile_to_plan(nl);
	if (!plan)
		{
		fprintf(stderr, "failed to compile %s\n", argv[1]);
		lxs_free_netlist(nl);
		return 1;
		}

	for (uint32_t w = 0; w < (uint32_t)(sizeof(watch_names) / sizeof(watch_names[0])); ++w)
		{
		int net_id = lxs_find_net_id(nl, watch_names[w]);
		int gate_driver = -1;
		int macro_driver = -1;
		int multi_driver = -1;

		if (net_id < 0)
			{
			printf("%s: missing\n", watch_names[w]);
			continue;
			}

		for (uint32_t i = 0; i < nl->gate_count; ++i)
			{
			if (nl->gates[i].output == (uint32_t)net_id)
				{
				gate_driver = (int)i;
				break;
				}
			}

		for (uint32_t i = 0; i < plan->macro_count; ++i)
			{
			if (plan->macros[i].output == (uint32_t)net_id)
				{
				macro_driver = (int)i;
				break;
				}
			}

		for (uint32_t i = 0; i < plan->multi_macro_count; ++i)
			{
			for (uint32_t j = 0; j < plan->multi_macros[i].output_count; ++j)
				{
				if (plan->multi_macros[i].outputs[j] == (uint32_t)net_id)
					{
					multi_driver = (int)i;
					break;
					}
				}
			if (multi_driver >= 0)
				{
				break;
				}
			}

		printf("%s net=%d", watch_names[w], net_id);
		if (gate_driver >= 0)
			{
			printf(" gate=%d gate_level=%u", gate_driver, nl->gates[gate_driver].level);
			for (uint32_t j = 0; j < nl->gates[gate_driver].input_count; ++j)
				{
				uint32_t in = nl->gates[gate_driver].inputs[j];
				int in_driver = -1;
				for (uint32_t k = 0; k < nl->gate_count; ++k)
					{
					if (nl->gates[k].output == in)
						{
						in_driver = (int)k;
						break;
						}
					}
				printf(
					" in%u=%u(%s)",
					j,
					in,
					lxs_find_name(nl, in) ? lxs_find_name(nl, in) : "?");
				if (in_driver >= 0)
					{
					printf("[g%d:l%u]", in_driver, nl->gates[in_driver].level);
					}
				}
			}
		if (macro_driver >= 0)
			{
			printf(" macro=%d macro_level=%u", macro_driver, plan->macros[macro_driver].level);
			}
		if (multi_driver >= 0)
			{
			printf(" multi=%d multi_level=%u", multi_driver, plan->multi_macros[multi_driver].level);
			}
		printf("\n");
		}

	lxs_free_plan(plan);
	lxs_free_netlist(nl);
	return 0;
	}
