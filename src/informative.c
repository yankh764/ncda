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
        void *retval;

        if(!(retval = malloc(size)))
                error(0, errno, "could not allocate memory");
        
        return retval;
}

int stat_inf(const char *path, struct stat *statbuf)
{
        int retval;

        if ((retval = stat(path, statbuf)))
                error(0, errno, "could not get status on '%s'", path);

        return retval;
}

DIR *opendir_inf(const char *path)
{
        DIR *retval;

        if (!(retval = opendir(path)))
                error(0, errno, "could not open '%s'", path);

        return retval;
}

int closedir_inf(DIR *dp)
{
        int retval;

        if ((retval = closedir(dp)))
                error(0, errno, "could not close directory");

        return retval;
}

struct dirent *readdir_inf(DIR *dp)
{
        struct dirent *retval;

        if(!(retval = readdir(dp)) && errno)
                error(0, errno, "could not read directory's entries");

        return retval;
} 
