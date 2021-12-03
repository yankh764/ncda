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
#include <unistd.h>
#include "informative.h"


void *malloc_inf(size_t size)
{
        void *retval;

        if(!(retval = malloc(size)))
                error(0, errno, "could not allocate memory");
        
        return retval;
}

int lstat_inf(const char *path, struct stat *statbuf)
{
        int retval;

        if ((retval = lstat(path, statbuf)))
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

int unlink_inf(const char *pathname)
{
	int retval;

	if ((retval = unlink(pathname)))
		error(0, errno, "could not remove '%s'", pathname);
	
	return retval;
}

int rmdir_inf(const char *pathname)
{
	int retval;

	if ((retval = rmdir(pathname)))
		error(0, errno, "could not remove '%s'", pathname);

	return retval;
}

FILE *fopen_inf(const char *path, const char *mode)
{
	FILE *fp;

	if (!(fp = fopen(path, mode)))
		error(0, errno, "could not open '%s'", path);

	return fp;
}

int fclose_inf(FILE *fp)
{
	int retval;

	if ((retval = fclose(fp)))
                error(0, errno, "could not close file");

	return retval;
}

int fseek_inf(FILE *fp, long offset, int whence)
{
	int retval;

	if ((retval = fseek(fp, offset, whence)))
		error(0, errno, "could not set the file position indicator");

	return retval;
}

long ftell_inf(FILE *fp)
{
	long retval;

	if ((retval = ftell(fp)) == -1)
		error(0, errno, "could not get the value of the file position");

	return retval;
}
