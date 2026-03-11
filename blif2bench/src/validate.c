#include "validate.h"

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

static int n2b_find_producer(const N2B_Normalized *normalized, int net_id)
{
	int index;

	for (index = 0; index < normalized->node_count; ++index)
	{
		if (normalized->nodes[index].output_net == net_id)
		{
			return index;
		}
	}

	return -1;
}

static int n2b_is_primary_input(const N2B_Normalized *normalized, int net_id)
{
	int index;

	for (index = 0; index < normalized->input_count; ++index)
	{
		if (normalized->input_nets[index] == net_id)
		{
			return 1;
		}
	}

	return 0;
}

static int n2b_compare_ready_nodes(const N2B_IR *ir, const N2B_Normalized *normalized, int left_index, int right_index)
{
	const char *left_name;
	const char *right_name;
	int cmp;

	left_name = ir->nets[normalized->nodes[left_index].output_net].canonical_name;
	right_name = ir->nets[normalized->nodes[right_index].output_net].canonical_name;
	cmp = strcmp(left_name, right_name);

	if (cmp != 0)
	{
		return cmp;
	}

	return normalized->nodes[left_index].source_order - normalized->nodes[right_index].source_order;
}

int n2b_validate_normalized(const N2B_IR *ir, N2B_Normalized *normalized, char *error_text, int error_size)
{
	int *indegree;
	int *emitted;
	int emitted_count;
	int node_index;

	normalized->topo_order = (int *)malloc((size_t)normalized->node_count * sizeof(int));
	indegree = (int *)calloc((size_t)normalized->node_count, sizeof(int));
	emitted = (int *)calloc((size_t)normalized->node_count, sizeof(int));

	if ((normalized->node_count > 0 && normalized->topo_order == NULL) ||
		(normalized->node_count > 0 && (indegree == NULL || emitted == NULL)))
	{
		free(indegree);
		free(emitted);
		return n2b_set_error(error_text, error_size, "out of memory while validating normalized graph");
	}

	for (node_index = 0; node_index < normalized->node_count; ++node_index)
	{
		int input_index;

		for (input_index = 0; input_index < normalized->nodes[node_index].input_count; ++input_index)
		{
			int producer;
			int input_net;

			input_net = normalized->nodes[node_index].input_nets[input_index];
			producer = n2b_find_producer(normalized, input_net);

			if (producer >= 0)
			{
				++indegree[node_index];
				continue;
			}

			if (!n2b_is_primary_input(normalized, input_net))
			{
				free(indegree);
				free(emitted);
				return n2b_set_error(error_text, error_size, "node references unresolved input net");
			}
		}
	}

	emitted_count = 0;
	while (emitted_count < normalized->node_count)
	{
		int best_index;

		best_index = -1;

		for (node_index = 0; node_index < normalized->node_count; ++node_index)
		{
			if (emitted[node_index] || indegree[node_index] != 0)
			{
				continue;
			}

			if (best_index < 0 || n2b_compare_ready_nodes(ir, normalized, node_index, best_index) < 0)
			{
				best_index = node_index;
			}
		}

		if (best_index < 0)
		{
			free(indegree);
			free(emitted);
			return n2b_set_error(error_text, error_size, "cycle or unresolved dependency detected");
		}

		emitted[node_index = best_index] = 1;
		normalized->topo_order[emitted_count++] = node_index;

		for (node_index = 0; node_index < normalized->node_count; ++node_index)
		{
			int input_index;

			if (emitted[node_index])
			{
				continue;
			}

			for (input_index = 0; input_index < normalized->nodes[node_index].input_count; ++input_index)
			{
				if (normalized->nodes[node_index].input_nets[input_index] == normalized->nodes[best_index].output_net)
				{
					--indegree[node_index];
				}
			}
		}
	}

	for (node_index = 0; node_index < normalized->output_count; ++node_index)
	{
		int output_net;

		output_net = normalized->output_nets[node_index];
		if (!n2b_is_primary_input(normalized, output_net) && n2b_find_producer(normalized, output_net) < 0)
		{
			free(indegree);
			free(emitted);
			return n2b_set_error(error_text, error_size, "primary output is not driven");
		}
	}

	normalized->topo_count = emitted_count;
	free(indegree);
	free(emitted);
	return 1;
}
