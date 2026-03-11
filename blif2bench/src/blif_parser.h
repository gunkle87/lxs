#ifndef N2B_BLIF_PARSER_H
#define N2B_BLIF_PARSER_H

#include "ir.h"

int n2b_parse_blif(const char *path, N2B_IR *ir, char *error_text, int error_size);

#endif
