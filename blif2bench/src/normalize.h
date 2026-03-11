#ifndef N2B_NORMALIZE_H
#define N2B_NORMALIZE_H

#include "ir.h"

typedef struct N2B_Normalized
{
	N2B_Node *nodes;
	int node_count;
	int node_capacity;
	int *input_nets;
	int input_count;
	int *output_nets;
	int output_count;
	int *alias_target;
	int *topo_order;
	int topo_count;
} N2B_Normalized;

void n2b_normalized_init(N2B_Normalized *normalized);
void n2b_normalized_free(N2B_Normalized *normalized);
int n2b_normalize_ir(const N2B_IR *ir, N2B_Normalized *normalized, char *error_text, int error_size);

#endif
