/*
----------------------------------------------------------
| License: GNU GPL-3.0                                   |
----------------------------------------------------------
| This source file contains all the necessary components |
| for managing the structures of the ncda prgoram.       |
----------------------------------------------------------
*/

#include <stdlib.h>
#include "ncda.h"
#include "informative.h"
#include "structs.h"


static inline void *alloc_stat()
{
        return malloc_inf(sizeof(struct stat));
}

static inline void *alloc_fdata(size_t name_len, size_t path_len)
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

static inline void *alloc_entry_data(size_t name_len, size_t path_len) 
{
	struct entry_data *retval;

	if ((retval = malloc_inf(sizeof(struct entry_data)))) {
		if (!(retval->file_data = alloc_fdata(name_len, path_len)))
			goto err_free_entry_data;
		if (!(retval->curses_data = alloc_cdata()))
			goto err_free_fdata;
	}
	return retval;

err_free_fdata:
	free_fdata(retval->file_data);
err_free_entry_data:
	free(retval);

	return NULL;
}

static void free_entry_data(struct entry_data *ptr)
{
	free(ptr->curses_data);
	free_fdata(ptr->file_data);
	free(ptr);
}

static inline void null_prev_and_next(struct entries_dlist *node)
{
	node->prev = NULL;
	node->next = NULL;
}

void *alloc_entries_dlist(size_t name_len, size_t path_len)
{
	struct entries_dlist *retval;

	if ((retval = malloc_inf(sizeof(struct entries_dlist)))) {
		if ((retval->data = alloc_entry_data(name_len, path_len)))
			null_prev_and_next(retval);
		else
			free_and_null((void **) &retval);
	}
	return retval;
}

void free_entries_dlist(struct entries_dlist *head)
{
	if (head->next)
		free_entries_dlist(head->next);
	free_entry_data(head->data);
	free(head);
}
