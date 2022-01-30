/*
----------------------------------------------------------
| License: GNU GPL-3.0                                   |
----------------------------------------------------------
| This source file contains all the necessary components |
| for managing the structures of the ncda prgoram.       |
----------------------------------------------------------
*/ 

#include <stdlib.h>
#include "general.h"
#include "informative.h"
#include "structs.h"


static inline void *alloc_stat()
{
        return malloc_inf(sizeof(struct stat));
}

static void *alloc_fdata(size_t name_len, size_t path_len)
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

static void free_fdata(struct fdata *ptr)
{
	free(ptr->fstatus);
	free(ptr->fpath);
	free(ptr->fname);
	free(ptr);
}

static inline void *alloc_cdata()
{
	return malloc_inf(sizeof(struct cdata));
}

static void *alloc_entry_data(size_t name_len, size_t path_len) 
{
	struct entry_data *retval;

	if ((retval = malloc_inf(sizeof(struct entry_data)))) {
		if (!(retval->file = alloc_fdata(name_len, path_len)))
			goto err_free_entry_data;
		if (!(retval->curses = alloc_cdata()))
			goto err_free_fdata;
	}
	return retval;

err_free_fdata:
	free_fdata(retval->file);
err_free_entry_data:
	free(retval);

	return NULL;
}

static void free_entry_data(struct entry_data *ptr)
{
	free(ptr->curses);
	free_fdata(ptr->file);
	free(ptr);
}

static inline void null_dtree_members(struct dtree *node)
{
	node->parent = NULL;
	node->prev = NULL;
	node->next = NULL;
	node->child = NULL;
}

void *alloc_dtree(size_t name_len, size_t path_len)
{
	struct dtree *retval;

	if ((retval = malloc_inf(sizeof(struct dtree)))) {
		if ((retval->data = alloc_entry_data(name_len, path_len)))
			null_dtree_members(retval);
		else
			free_and_null((void **) &retval);
	}
	return retval;
}

void free_dtree(struct dtree *node)
{
	struct dtree *current, *next;
	
	for (current=node; current; current=next) {
		next = current->next;

		if (current->child)
			free_dtree(current->child);
		free_entry_data(current->data);
		free(current);
	}
}
