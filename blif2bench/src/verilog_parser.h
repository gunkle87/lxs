#ifndef N2B_VERILOG_PARSER_H
#define N2B_VERILOG_PARSER_H

#include "ir.h"

int n2b_parse_verilog(const char *path, N2B_IR *ir, char *error_text, int error_size);

#endif
