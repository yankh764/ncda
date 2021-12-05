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

