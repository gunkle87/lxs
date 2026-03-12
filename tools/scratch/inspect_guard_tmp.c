#include "lxs_types.h"
#include <stdio.h>
int main(void)
	{
	lxs_netlist *nl = lxs_load_iscas("Benchmarks\\Generated\\arbiter_guard4.bench");
	if (!nl)
		{
		fprintf(stderr, "load failed\n");
		return 1;
		}
	printf("loaded nets=%u gates=%u sm=%u smm=%u\n", nl->net_count, nl->gate_count, nl->source_macro_count, nl->source_multi_macro_count);
	lxs_plan *plan = lxs_compile_to_plan(nl);
	if (!plan)
		{
		fprintf(stderr, "compile failed\n");
		lxs_free_netlist(nl);
		return 2;
		}
	printf("plan levels=%u spans=%u macros=%u multi=%u gates=%u comb=%u\n", plan->level_count, plan->span_count, plan->macro_count, plan->multi_macro_count, plan->gate_count, plan->comb_gate_count);
	lxs_free_plan(plan);
	lxs_free_netlist(nl);
	return 0;
	}
