#ifndef N2B_BENCH_EMIT_H
#define N2B_BENCH_EMIT_H

#include <stdio.h>

#include "cli.h"
#include "normalize.h"

int n2b_emit_bench(FILE *stream, const N2B_IR *ir, const N2B_Normalized *normalized, N2B_WideGateMode wide_gate_mode, char *error_text, int error_size);
int n2b_write_name_map(FILE *stream, const N2B_IR *ir, char *error_text, int error_size);

#endif
