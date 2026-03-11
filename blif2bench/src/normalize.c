#include "normalize.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int n2b_set_error(char *error_text, int error_size, const char *message)
{
	if (error_text != NULL && error_size > 0)
	{
		snprintf(error_text, (size_t)error_size, "%s", message);
	}

	return 0;
}

static int n2b_grow_nodes(N2B_Normalized *normalized)
{
	N2B_Node *grown;
	int next_capacity;

	next_capacity = (normalized->node_capacity == 0) ? 16 : (normalized->node_capacity * 2);
	grown = (N2B_Node *)realloc(normalized->nodes, (size_t)next_capacity * sizeof(normalized->nodes[0]));

	if (grown == NULL)
	{
		return 0;
	}

	normalized->nodes = grown;
	normalized->node_capacity = next_capacity;
	return 1;
}

void n2b_normalized_init(N2B_Normalized *normalized)
{
	memset(normalized, 0, sizeof(*normalized));
}

void n2b_normalized_free(N2B_Normalized *normalized)
{
	int index;

	for (index = 0; index < normalized->node_count; ++index)
	{
		free(normalized->nodes[index].input_nets);
	}

	free(normalized->nodes);
	free(normalized->input_nets);
	free(normalized->output_nets);
	free(normalized->alias_target);
	free(normalized->topo_order);
	memset(normalized, 0, sizeof(*normalized));
}

static int n2b_resolve_alias(const N2B_Normalized *normalized, int net_id)
{
	int current;
	int guard;

	current = net_id;

	for (guard = 0; guard < 100000; ++guard)
	{
		if (normalized->alias_target[current] < 0)
		{
			return current;
		}

		current = normalized->alias_target[current];
	}

	return -1;
}

static int n2b_append_normalized_node(N2B_Normalized *normalized, const N2B_Node *source, int output_net, const int *input_nets, int input_count, char *error_text, int error_size)
{
	N2B_Node node;

	if (normalized->node_count == normalized->node_capacity && !n2b_grow_nodes(normalized))
	{
		return n2b_set_error(error_text, error_size, "out of memory while growing normalized node list");
	}

	memset(&node, 0, sizeof(node));
	node.op = source->op;
	node.output_net = output_net;
	node.input_count = input_count;
	node.source_order = source->source_order;
	node.synthetic = source->synthetic;
	node.input_nets = (int *)malloc((size_t)input_count * sizeof(input_nets[0]));

	if (node.input_nets == NULL)
	{
		return n2b_set_error(error_text, error_size, "out of memory while copying normalized node inputs");
	}

	memcpy(node.input_nets, input_nets, (size_t)input_count * sizeof(input_nets[0]));
	normalized->nodes[normalized->node_count] = node;
	++normalized->node_count;
	return 1;
}

int n2b_normalize_ir(const N2B_IR *ir, N2B_Normalized *normalized, char *error_text, int error_size)
{
	int index;
	int *direct_driver_count;

	normalized->input_count = ir->input_count;
	normalized->output_count = ir->output_count;
	normalized->input_nets = (int *)malloc((size_t)ir->input_count * sizeof(ir->input_nets[0]));
	normalized->output_nets = (int *)malloc((size_t)ir->output_count * sizeof(ir->output_nets[0]));
	normalized->alias_target = (int *)malloc((size_t)ir->net_count * sizeof(int));
	direct_driver_count = (int *)calloc((size_t)ir->net_count, sizeof(int));

	if ((ir->input_count > 0 && normalized->input_nets == NULL) ||
		(ir->output_count > 0 && normalized->output_nets == NULL) ||
		(ir->net_count > 0 && (normalized->alias_target == NULL || direct_driver_count == NULL)))
	{
		free(direct_driver_count);
		return n2b_set_error(error_text, error_size, "out of memory while preparing normalization");
	}

	memcpy(normalized->input_nets, ir->input_nets, (size_t)ir->input_count * sizeof(ir->input_nets[0]));
	memcpy(normalized->output_nets, ir->output_nets, (size_t)ir->output_count * sizeof(ir->output_nets[0]));

	for (index = 0; index < ir->net_count; ++index)
	{
		normalized->alias_target[index] = -1;
	}

	for (index = 0; index < ir->node_count; ++index)
	{
		const N2B_Node *node;
		int resolved_input;

		node = &ir->nodes[index];

		if (node->op != N2B_OP_BUF)
		{
			continue;
		}

		resolved_input = n2b_resolve_alias(normalized, node->input_nets[0]);
		if (resolved_input < 0)
		{
			free(direct_driver_count);
			return n2b_set_error(error_text, error_size, "alias cycle detected");
		}

		if (node->output_net == resolved_input)
		{
			continue;
		}

		if (normalized->alias_target[node->output_net] >= 0 &&
			normalized->alias_target[node->output_net] != resolved_input)
		{
			free(direct_driver_count);
			return n2b_set_error(error_text, error_size, "net has multiple alias drivers");
		}

		if (ir->nets[node->output_net].declared_input)
		{
			free(direct_driver_count);
			return n2b_set_error(error_text, error_size, "primary input cannot be driven by alias");
		}

		normalized->alias_target[node->output_net] = resolved_input;
	}

	for (index = 0; index < ir->node_count; ++index)
	{
		const N2B_Node *node;
		int *resolved_inputs;
		int input_index;

		node = &ir->nodes[index];

		if (node->op == N2B_OP_BUF)
		{
			continue;
		}

		if (normalized->alias_target[node->output_net] >= 0)
		{
			free(direct_driver_count);
			return n2b_set_error(error_text, error_size, "net cannot have both alias and real driver");
		}

		if (ir->nets[node->output_net].declared_input)
		{
			free(direct_driver_count);
			return n2b_set_error(error_text, error_size, "primary input cannot be driven");
		}

		++direct_driver_count[node->output_net];

		if (direct_driver_count[node->output_net] > 1)
		{
			free(direct_driver_count);
			return n2b_set_error(error_text, error_size, "multi-driver conflict detected");
		}

		resolved_inputs = (int *)malloc((size_t)node->input_count * sizeof(node->input_nets[0]));
		if (resolved_inputs == NULL)
		{
			free(direct_driver_count);
			return n2b_set_error(error_text, error_size, "out of memory while normalizing inputs");
		}

		for (input_index = 0; input_index < node->input_count; ++input_index)
		{
			resolved_inputs[input_index] = n2b_resolve_alias(normalized, node->input_nets[input_index]);
			if (resolved_inputs[input_index] < 0)
			{
				free(resolved_inputs);
				free(direct_driver_count);
				return n2b_set_error(error_text, error_size, "alias cycle detected");
			}
		}

		if (!n2b_append_normalized_node(normalized, node, node->output_net, resolved_inputs, node->input_count, error_text, error_size))
		{
			free(resolved_inputs);
			free(direct_driver_count);
			return 0;
		}

		free(resolved_inputs);
	}

	for (index = 0; index < ir->output_count; ++index)
	{
		int output_net;
		int resolved_net;
		int input_nets[1];
		N2B_Node synthetic_node;

		output_net = ir->output_nets[index];
		resolved_net = n2b_resolve_alias(normalized, output_net);

		if (resolved_net < 0)
		{
			free(direct_driver_count);
			return n2b_set_error(error_text, error_size, "alias cycle detected");
		}

		if (resolved_net == output_net)
		{
			continue;
		}

		memset(&synthetic_node, 0, sizeof(synthetic_node));
		synthetic_node.op = N2B_OP_BUF;
		synthetic_node.source_order = ir->next_source_order + index + 1;
		synthetic_node.synthetic = 1;
		input_nets[0] = resolved_net;

		if (direct_driver_count[output_net] > 0)
		{
			free(direct_driver_count);
			return n2b_set_error(error_text, error_size, "primary output alias conflicts with real driver");
		}

		++direct_driver_count[output_net];

		if (!n2b_append_normalized_node(normalized, &synthetic_node, output_net, input_nets, 1, error_text, error_size))
		{
			free(direct_driver_count);
			return 0;
		}
	}

	free(direct_driver_count);
	return 1;
}
