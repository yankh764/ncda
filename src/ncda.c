/*
---------------------------------------------------------
| License: GNU GPL-3.0                                  |
---------------------------------------------------------
| This source file contains all the necessary functions |
| for all the general fucntions in the ncda program     |
---------------------------------------------------------
*/

#include <stdlib.h>
#include <string.h>
#include "ncda.h"


void free_and_null(void **ptr)
{
	free(*ptr);
	*ptr = NULL;
}

bool is_dot_entry(const char *entry_name)
{
	return (strcmp(entry_name, ".") == 0 ||
		strcmp(entry_name, "..") == 0);
}

static inline struct size_format proper_size_format(float size, char *unit)
{
	struct size_format retval;

	retval.size = size;
	retval.unit = unit;

	return retval;
}

static inline float bytes_to_tb(off_t bytes)
{
	return bytes / 1000000000000.0;
}

static inline float bytes_to_gb(off_t bytes)
{
	return bytes / 1000000000.0;
}

static inline float bytes_to_mb(off_t bytes)
{
	return bytes / 1000000.0;
}

static inline float bytes_to_kb(off_t bytes)
{
	return bytes / 1000.0;
}

struct size_format get_proper_size_format(off_t bytes)
{
	const off_t tb = 1000000000000;
	const off_t gb = 1000000000;
	const off_t mb = 1000000;
	const off_t kb = 1000;

	if (bytes >= tb)
		return proper_size_format(bytes_to_tb(bytes), "TB");
	else if (bytes >= gb)
		return proper_size_format(bytes_to_gb(bytes), "GB");
	else if (bytes >= mb)
		return proper_size_format(bytes_to_mb(bytes), "MB");
	else if (bytes >= kb)
		return proper_size_format(bytes_to_kb(bytes), "KB");
	else
		return proper_size_format(bytes, "B");
}

