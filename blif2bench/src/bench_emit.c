#include "bench_emit.h"

#include <stdio.h>
#include <string.h>

static int n2b_name_in_use(const N2B_IR *ir, const char *name)
{
	int index;

	for (index = 0; index < ir->net_count; ++index)
	{
		if (strcmp(ir->nets[index].canonical_name, name) == 0)
		{
			return 1;
		}
	}

	return 0;
}

static int n2b_emit_binary_lowered_gate(FILE *stream, const N2B_IR *ir, const N2B_Node *node, int *helper_counter, char *error_text, int error_size)
{
	char helper_name[64];
	const char *chain_op;
	int input_index;

	if (node->input_count < 3)
	{
		return 1;
	}

	chain_op = (node->op == N2B_OP_AND || node->op == N2B_OP_NAND) ? "AND" : "OR";
	snprintf(helper_name, sizeof(helper_name), "%s", ir->nets[node->input_nets[0]].canonical_name);

	for (input_index = 1; input_index < node->input_count - 1; ++input_index)
	{
		char next_helper[64];

		do
		{
			++(*helper_counter);
			snprintf(next_helper, sizeof(next_helper), "__t_%06d", *helper_counter);
		}
		while (n2b_name_in_use(ir, next_helper));

		fprintf(stream, "%s = %s(%s,%s)\n", next_helper, chain_op, helper_name, ir->nets[node->input_nets[input_index]].canonical_name);
		snprintf(helper_name, sizeof(helper_name), "%s", next_helper);
	}

	fprintf(
		stream,
		"%s = %s(%s,%s)\n",
		ir->nets[node->output_net].canonical_name,
		n2b_ir_op_name(node->op),
		helper_name,
		ir->nets[node->input_nets[node->input_count - 1]].canonical_name);

	(void)error_text;
	(void)error_size;
	return 1;
}

int n2b_emit_bench(FILE *stream, const N2B_IR *ir, const N2B_Normalized *normalized, N2B_WideGateMode wide_gate_mode, char *error_text, int error_size)
{
	int index;
	int helper_counter;

	(void)error_text;
	(void)error_size;
	helper_counter = ir->temp_counter;

	for (index = 0; index < normalized->input_count; ++index)
	{
		fprintf(stream, "INPUT(%s)\n", ir->nets[normalized->input_nets[index]].canonical_name);
	}

	fprintf(stream, "\n");

	for (index = 0; index < normalized->topo_count; ++index)
	{
		const N2B_Node *node;
		int input_index;

		node = &normalized->nodes[normalized->topo_order[index]];
		if (wide_gate_mode == N2B_WIDE_LOWER_BINARY &&
			node->input_count > 2 &&
			(node->op == N2B_OP_AND || node->op == N2B_OP_NAND || node->op == N2B_OP_OR || node->op == N2B_OP_NOR))
		{
			if (!n2b_emit_binary_lowered_gate(stream, ir, node, &helper_counter, error_text, error_size))
			{
				return 0;
			}

			continue;
		}

		fprintf(stream, "%s = %s(", ir->nets[node->output_net].canonical_name, n2b_ir_op_name(node->op));

		for (input_index = 0; input_index < node->input_count; ++input_index)
		{
			fprintf(stream, "%s", ir->nets[node->input_nets[input_index]].canonical_name);

			if (input_index + 1 < node->input_count)
			{
				fprintf(stream, ",");
			}
		}

		fprintf(stream, ")\n");
	}

	for (index = 0; index < normalized->output_count; ++index)
	{
		fprintf(stream, "OUTPUT(%s)\n", ir->nets[normalized->output_nets[index]].canonical_name);
	}

	return 1;
}

int n2b_write_name_map(FILE *stream, const N2B_IR *ir, char *error_text, int error_size)
{
	int index;

	(void)error_text;
	(void)error_size;

	fprintf(stream, "original,canonical\n");
	for (index = 0; index < ir->net_count; ++index)
	{
		if (ir->nets[index].name_changed)
		{
			fprintf(stream, "%s,%s\n", ir->nets[index].original_name, ir->nets[index].canonical_name);
		}
	}

	return 1;
}
