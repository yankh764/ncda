#ifndef _NCDA_H 
#define _NCDA_H

#include <stdbool.h>
#include "structs.h"

void free_and_null(void **);
bool is_dot_entry(const char *);
struct size_format get_proper_size_format(off_t);

#endif