/*
---------------------------------------------------------
| License: GNU GPL-3.0                                  |
---------------------------------------------------------
| This source file contains all the necessary functions |
| for the disk analysis.                                |
---------------------------------------------------------
*/

#include <stdlib.h>
#include "informative.h" 
#include "disk_analysis.h"

static void free_and_null(void **ptr)
{
        free(*ptr);
        *ptr = NULL;
}

/*
static inline void *alloc_stat()
{
        return malloc_inf(sizeof(struct stat));
}

static struct stat *get_stat(const int fd)
{
        struct stat *statbuf;

        if ((statbuf = alloc_stat())
                if (fstat_inf(fd, statbuf))
                        free_and_null(&statbuf);

        return statbuf;
}
*/


