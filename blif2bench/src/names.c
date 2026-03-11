#include "names.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *n2b_strdup(const char *text)
{
	size_t length;
	char *copy;

	length = strlen(text);
	copy = (char *)malloc(length + 1U);

	if (copy == NULL)
	{
		return NULL;
	}

	memcpy(copy, text, length + 1U);
	return copy;
}

static int n2b_is_safe_name_char(int c)
{
	return isalnum(c) || c == '_' || c == '[' || c == ']';
}

int n2b_sanitize_name(const char *raw_name, int stable_index, char *out_name, size_t out_size, int *changed)
{
	char sanitized[256];
	size_t in_index;
	size_t out_index;
	int needs_prefix;

	if (raw_name == NULL || out_name == NULL || out_size == 0U)
	{
		return 0;
	}

	memset(sanitized, 0, sizeof(sanitized));
	out_index = 0U;
	needs_prefix = 0;

	for (in_index = 0U; raw_name[in_index] != '\0'; ++in_index)
	{
		if (out_index + 1U >= sizeof(sanitized))
		{
			return 0;
		}

		if (n2b_is_safe_name_char((unsigned char)raw_name[in_index]))
		{
			sanitized[out_index++] = raw_name[in_index];
		}
		else
		{
			sanitized[out_index++] = '_';
			needs_prefix = 1;
		}
	}

	if (out_index == 0U)
	{
		strcpy(sanitized, "empty");
		needs_prefix = 1;
	}

	if (isdigit((unsigned char)sanitized[0]))
	{
		needs_prefix = 1;
	}

	if (needs_prefix)
	{
		snprintf(out_name, out_size, "hz_%06d_%s", stable_index, sanitized);
		*changed = 1;
		return 1;
	}

	snprintf(out_name, out_size, "%s", sanitized);
	*changed = 0;
	return 1;
}
