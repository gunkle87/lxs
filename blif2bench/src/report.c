#include "report.h"

#include <stdio.h>

void n2b_print_summary(const N2B_IR *ir, const N2B_Normalized *normalized)
{
	printf("nets=%d nodes=%d inputs=%d outputs=%d\n",
		ir->net_count,
		normalized->node_count,
		normalized->input_count,
		normalized->output_count);
}
