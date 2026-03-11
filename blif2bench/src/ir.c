#include "ir.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "names.h"

#define N2B_INITIAL_CAPACITY 16

static int n2b_set_error(char *error_text, int error_size, const char *message)
{
	if (error_text != NULL && error_size > 0)
	{
		snprintf(error_text, (size_t)error_size, "%s", message);
	}

	return 0;
}

static int n2b_set_decl_conflict_error(const char *file_path, int line_number, const char *raw_name, char *error_text, int error_size)
{
	if (error_text != NULL && error_size > 0)
	{
		snprintf(
			error_text,
			(size_t)error_size,
			"error[N2B_DECL_CONFLICT]: %s:%d: conflicting redeclaration of net '%s'",
			file_path,
			line_number,
			raw_name);
	}

	return 0;
}

static int n2b_fail(char *error_text, int error_size, const char *message)
{
	n2b_set_error(error_text, error_size, message);
	return -1;
}

static int n2b_fail_decl_conflict(const char *file_path, int line_number, const char *raw_name, char *error_text, int error_size)
{
	n2b_set_decl_conflict_error(file_path, line_number, raw_name, error_text, error_size);
	return -1;
}

static int n2b_grow_bytes(void **buffer, int *capacity, size_t element_size)
{
	void *grown;
	int next_capacity;

	next_capacity = (*capacity == 0) ? N2B_INITIAL_CAPACITY : (*capacity * 2);
	grown = realloc(*buffer, (size_t)next_capacity * element_size);

	if (grown == NULL)
	{
		return 0;
	}

	*buffer = grown;
	*capacity = next_capacity;
	return 1;
}

void n2b_ir_init(N2B_IR *ir)
{
	memset(ir, 0, sizeof(*ir));
}

void n2b_ir_free(N2B_IR *ir)
{
	int index;

	for (index = 0; index < ir->net_count; ++index)
	{
		free(ir->nets[index].original_name);
		free(ir->nets[index].canonical_name);
	}

	for (index = 0; index < ir->node_count; ++index)
	{
		free(ir->nodes[index].input_nets);
	}

	free(ir->nets);
	free(ir->nodes);
	free(ir->input_nets);
	free(ir->output_nets);
	memset(ir, 0, sizeof(*ir));
}

static int n2b_find_net_by_original(const N2B_IR *ir, const char *raw_name)
{
	int index;

	for (index = 0; index < ir->net_count; ++index)
	{
		if (strcmp(ir->nets[index].original_name, raw_name) == 0)
		{
			return index;
		}
	}

	return -1;
}

static int n2b_find_net_by_canonical(const N2B_IR *ir, const char *canonical_name)
{
	int index;

	for (index = 0; index < ir->net_count; ++index)
	{
		if (strcmp(ir->nets[index].canonical_name, canonical_name) == 0)
		{
			return index;
		}
	}

	return -1;
}

int n2b_ir_intern_net(N2B_IR *ir, const char *raw_name, char *error_text, int error_size)
{
	N2B_Net net;
	char canonical_name[320];
	int changed;
	int existing;

	if (raw_name == NULL || raw_name[0] == '\0')
	{
		return n2b_fail(error_text, error_size, "encountered empty net name");
	}

	existing = n2b_find_net_by_original(ir, raw_name);
	if (existing >= 0)
	{
		return existing;
	}

	changed = 0;
	if (!n2b_sanitize_name(raw_name, ir->next_stable_index + 1, canonical_name, sizeof(canonical_name), &changed))
	{
		return n2b_fail(error_text, error_size, "failed to sanitize net name");
	}

	if (!changed && n2b_find_net_by_canonical(ir, canonical_name) >= 0)
	{
		return n2b_fail(error_text, error_size, "canonical net name collision");
	}

	if (ir->net_count == ir->net_capacity && !n2b_grow_bytes((void **)&ir->nets, &ir->net_capacity, sizeof(ir->nets[0])))
	{
		return n2b_fail(error_text, error_size, "out of memory while growing net table");
	}

	memset(&net, 0, sizeof(net));
	net.original_name = n2b_strdup(raw_name);
	net.canonical_name = n2b_strdup(canonical_name);

	if (net.original_name == NULL || net.canonical_name == NULL)
	{
		free(net.original_name);
		free(net.canonical_name);
		return n2b_fail(error_text, error_size, "out of memory while storing net name");
	}

	net.stable_index = ++ir->next_stable_index;
	net.name_changed = changed;
	net.first_seen_order = net.stable_index;

	ir->nets[ir->net_count] = net;
	++ir->net_count;
	return ir->net_count - 1;
}

static int n2b_add_unique_id(int **values, int *count, int *capacity, int value, char *error_text, int error_size)
{
	int index;

	for (index = 0; index < *count; ++index)
	{
		if ((*values)[index] == value)
		{
			return 1;
		}
	}

	if (*count == *capacity && !n2b_grow_bytes((void **)values, capacity, sizeof((*values)[0])))
	{
		return n2b_set_error(error_text, error_size, "out of memory while growing id list");
	}

	(*values)[*count] = value;
	++(*count);
	return 1;
}

int n2b_ir_add_input(N2B_IR *ir, const char *raw_name, char *error_text, int error_size)
{
	return n2b_ir_declare_input(ir, raw_name, "<internal>", 0, error_text, error_size);
}

int n2b_ir_add_output(N2B_IR *ir, const char *raw_name, char *error_text, int error_size)
{
	return n2b_ir_declare_output(ir, raw_name, "<internal>", 0, error_text, error_size);
}

int n2b_ir_add_wire(N2B_IR *ir, const char *raw_name, char *error_text, int error_size)
{
	return n2b_ir_declare_wire(ir, raw_name, "<internal>", 0, error_text, error_size);
}

int n2b_ir_declare_input(N2B_IR *ir, const char *raw_name, const char *file_path, int line_number, char *error_text, int error_size)
{
	int net_id;

	net_id = n2b_ir_intern_net(ir, raw_name, error_text, error_size);
	if (net_id < 0)
	{
		return net_id;
	}

	if (ir->nets[net_id].declared_wire || ir->nets[net_id].declared_output)
	{
		return n2b_fail_decl_conflict(file_path, line_number, raw_name, error_text, error_size);
	}

	ir->nets[net_id].declared_input = 1;

	if (!n2b_add_unique_id(&ir->input_nets, &ir->input_count, &ir->input_capacity, net_id, error_text, error_size))
	{
		return -1;
	}

	return net_id;
}

int n2b_ir_declare_output(N2B_IR *ir, const char *raw_name, const char *file_path, int line_number, char *error_text, int error_size)
{
	int net_id;

	net_id = n2b_ir_intern_net(ir, raw_name, error_text, error_size);
	if (net_id < 0)
	{
		return net_id;
	}

	if (ir->nets[net_id].declared_wire || ir->nets[net_id].declared_input)
	{
		return n2b_fail_decl_conflict(file_path, line_number, raw_name, error_text, error_size);
	}

	ir->nets[net_id].declared_output = 1;

	if (!n2b_add_unique_id(&ir->output_nets, &ir->output_count, &ir->output_capacity, net_id, error_text, error_size))
	{
		return -1;
	}

	return net_id;
}

int n2b_ir_declare_wire(N2B_IR *ir, const char *raw_name, const char *file_path, int line_number, char *error_text, int error_size)
{
	int net_id;

	net_id = n2b_ir_intern_net(ir, raw_name, error_text, error_size);
	if (net_id < 0)
	{
		return net_id;
	}

	if (ir->nets[net_id].declared_input || ir->nets[net_id].declared_output)
	{
		return n2b_fail_decl_conflict(file_path, line_number, raw_name, error_text, error_size);
	}

	ir->nets[net_id].declared_wire = 1;
	return net_id;
}

int n2b_ir_make_temp(N2B_IR *ir, char *error_text, int error_size)
{
	char temp_name[64];

	++ir->temp_counter;
	snprintf(temp_name, sizeof(temp_name), "__t_%06d", ir->temp_counter);
	return n2b_ir_intern_net(ir, temp_name, error_text, error_size);
}

int n2b_ir_add_node(N2B_IR *ir, N2B_Op op, int output_net, const int *input_nets, int input_count, int synthetic, char *error_text, int error_size)
{
	N2B_Node node;

	if (input_count <= 0)
	{
		return n2b_fail(error_text, error_size, "node must have at least one input");
	}

	if (ir->node_count == ir->node_capacity && !n2b_grow_bytes((void **)&ir->nodes, &ir->node_capacity, sizeof(ir->nodes[0])))
	{
		return n2b_fail(error_text, error_size, "out of memory while growing node table");
	}

	memset(&node, 0, sizeof(node));
	node.op = op;
	node.output_net = output_net;
	node.input_count = input_count;
	node.source_order = ++ir->next_source_order;
	node.synthetic = synthetic;
	node.input_nets = (int *)malloc((size_t)input_count * sizeof(input_nets[0]));

	if (node.input_nets == NULL)
	{
		return n2b_fail(error_text, error_size, "out of memory while storing node inputs");
	}

	memcpy(node.input_nets, input_nets, (size_t)input_count * sizeof(input_nets[0]));
	ir->nodes[ir->node_count] = node;
	++ir->node_count;
	return ir->node_count - 1;
}

const char *n2b_ir_op_name(N2B_Op op)
{
	switch (op)
	{
		case N2B_OP_BUF:
			return "BUF";
		case N2B_OP_NOT:
			return "NOT";
		case N2B_OP_AND:
			return "AND";
		case N2B_OP_NAND:
			return "NAND";
		case N2B_OP_OR:
			return "OR";
		case N2B_OP_NOR:
			return "NOR";
		case N2B_OP_XOR:
			return "XOR";
		case N2B_OP_XNOR:
			return "XNOR";
	}

	return "UNKNOWN";
}
