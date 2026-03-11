#ifndef N2B_NAMES_H
#define N2B_NAMES_H

#include <stddef.h>

char *n2b_strdup(const char *text);
int n2b_sanitize_name(const char *raw_name, int stable_index, char *out_name, size_t out_size, int *changed);

#endif
