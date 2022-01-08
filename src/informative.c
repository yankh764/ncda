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

int ERROR = 0;


void *malloc_inf(size_t size)
{
        void *retval;

        if(!(retval = malloc(size))) {
                error(0, errno, "could not allocate memory");
		ERROR = errno;
	}
	return retval;
}

int lstat_inf(const char *path, struct stat *statbuf)
{
        int retval;

        if ((retval = lstat(path, statbuf))) {
                error(0, errno, "could not get status on '%s'", path);
		ERROR = errno;
	}
        return retval;
}

DIR *opendir_inf(const char *path)
{
        DIR *retval;

        if (!(retval = opendir(path))) {
                error(0, errno, "could not open '%s'", path);
		ERROR = errno;
	}
        return retval;
}

int closedir_inf(DIR *dp)
{
        int retval;

        if ((retval = closedir(dp))) {
                error(0, errno, "could not close directory");
		ERROR = errno;
	}
        return retval;
}

struct dirent *readdir_inf(DIR *dp)
{
        struct dirent *retval;

        
	/* 
	 * To distinguish between 
	 * end of a dir and error 
	 */
	errno = 0;

	if(!(retval = readdir(dp)) && errno) {
		error(0, errno, "could not read directory's entries");
		ERROR = errno;
	}
        return retval;
}

int unlink_inf(const char *pathname)
{
	int retval;

	if ((retval = unlink(pathname))) {
		error(0, errno, "could not remove '%s'", pathname);
		ERROR = errno;
	}
	return retval;
}

int rmdir_inf(const char *pathname)
{
	int retval;

	if ((retval = rmdir(pathname))) {
		error(0, errno, "could not remove '%s'", pathname);
		ERROR = errno;
	}
	return retval;
}

FILE *fopen_inf(const char *path, const char *mode)
{
	FILE *fp;

	if (!(fp = fopen(path, mode))) {
		error(0, errno, "could not open '%s'", path);
		ERROR = errno;
	}
	return fp;
}

int fclose_inf(FILE *fp)
{
	int retval;

	if ((retval = fclose(fp))) {
                error(0, errno, "could not close file");
		ERROR = errno;
	}
	return retval;
}

time_t time_inf(time_t *tloc)
{
	time_t retval;

	if ((retval = time(tloc)) == -1) {
		error(0, errno, "could not get time in seconds");
		ERROR = errno;
	}
	return retval;
}
