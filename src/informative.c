/*
-----------------------------------------------------------
| License: GNU GPL-3.0                                    |
-----------------------------------------------------------
| This source file contains wrapper functions for some of |
| the GNU C library functions with informative error mes- |
| sages added to them in case of failures.                |
-----------------------------------------------------------
*/

#include <error.h>
#include <errno.h>
#include <stdlib.h>
#include "informative.h"


void *malloc_inf(size_t size)
{
        void *ptr;

        if(!(ptr = malloc(size)))
                error(0, errno, "could not allocate memory");
        
        return ptr;
}
