/*
----------------------------------------------------------
| License: GNU GPL-3.0                                   |
----------------------------------------------------------
| This source file contains all the necessary components |
| for managing the structures of the ncda prgoram.       |
----------------------------------------------------------
*/

#include <stdlib.h>
#include "informative.h"
#include "structs.h"


static inline void *alloc_stat()
{
        return malloc_inf(sizeof(struct stat));
}

void free_fdata(struct fdata *ptr)
{
	if (ptr->fstatus)
		free(ptr->fstatus);
	free(ptr->fpath);
	free(ptr->fname);
	free(ptr);
}

void *alloc_fdata(size_t name_len, size_t path_len)
{
	struct fdata *retval;

	if ((retval = malloc_inf(sizeof(struct fdata)))) {
		if (!(retval->fname = malloc_inf(name_len)))
			goto err_free_fdata;
		if (!(retval->fpath = malloc_inf(path_len)))
			goto err_free_fdata_fname;
		if (!(retval->fstatus = alloc_stat()))
			goto err_free_fdata_fpath;
	}
        return retval;

err_free_fdata_fpath:
	free(retval->fpath);
err_free_fdata_fname:
	free(retval->fname);
err_free_fdata:
	free(retval);

	return NULL;
}

static inline void *alloc_cdata()
{
	return malloc_inf(sizeof(struct cdata));
}

void *alloc_list()
{
	struct list *retval;

	if ((retval = malloc_inf(sizeof(struct list))))
		/* 
		 * NULL next now so I don't 
		 * forget to do that later 
		 */
		retval->next = NULL;

	return retval;
}

void free_list(struct list *node)
{
	if (node->next)
		free_list(node->next);
	free_fdata(node->data);
	free(node);
}
