#ifndef N2B_IR_H
#define N2B_IR_H

#include <stddef.h>

typedef enum N2B_Op
{
	N2B_OP_BUF = 0,
	N2B_OP_NOT,
	N2B_OP_AND,
	N2B_OP_NAND,
	N2B_OP_OR,
	N2B_OP_NOR,
	N2B_OP_XOR,
	N2B_OP_XNOR
} N2B_Op;

typedef struct N2B_Net
{
	char *original_name;
	char *canonical_name;
	int stable_index;
	int name_changed;
	int declared_input;
	int declared_output;
	int declared_wire;
	int first_seen_order;
} N2B_Net;

typedef struct N2B_Node
{
	N2B_Op op;
	int output_net;
	int *input_nets;
	int input_count;
	int source_order;
	int synthetic;
} N2B_Node;

typedef struct N2B_IR
{
	N2B_Net *nets;
	int net_count;
	int net_capacity;
	N2B_Node *nodes;
	int node_count;
	int node_capacity;
	int *input_nets;
	int input_count;
	int input_capacity;
	int *output_nets;
	int output_count;
	int output_capacity;
	int next_stable_index;
	int next_source_order;
	int temp_counter;
} N2B_IR;

void n2b_ir_init(N2B_IR *ir);
void n2b_ir_free(N2B_IR *ir);
int n2b_ir_intern_net(N2B_IR *ir, const char *raw_name, char *error_text, int error_size);
int n2b_ir_add_input(N2B_IR *ir, const char *raw_name, char *error_text, int error_size);
int n2b_ir_add_output(N2B_IR *ir, const char *raw_name, char *error_text, int error_size);
int n2b_ir_add_wire(N2B_IR *ir, const char *raw_name, char *error_text, int error_size);
int n2b_ir_declare_input(N2B_IR *ir, const char *raw_name, const char *file_path, int line_number, char *error_text, int error_size);
int n2b_ir_declare_output(N2B_IR *ir, const char *raw_name, const char *file_path, int line_number, char *error_text, int error_size);
int n2b_ir_declare_wire(N2B_IR *ir, const char *raw_name, const char *file_path, int line_number, char *error_text, int error_size);
int n2b_ir_make_temp(N2B_IR *ir, char *error_text, int error_size);
int n2b_ir_add_node(N2B_IR *ir, N2B_Op op, int output_net, const int *input_nets, int input_count, int synthetic, char *error_text, int error_size);
const char *n2b_ir_op_name(N2B_Op op);

#endif
