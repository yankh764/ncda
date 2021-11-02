/*
---------------------------------------------------------
| License: GNU GPL-3.0                                  |
---------------------------------------------------------
| This source file contains all the necessary functions |
| for the disk analysis.                                |
---------------------------------------------------------
*/


/*
 * Defining _GNU_SOURCE macro since it achives all the desired
 * feature test macro requirements, which are:
 *     1) _XOPEN_SOURCE >= 500 || _ISOC99_SOURCE for snprintf()
 *     2) _DEFAULT_SOURCE || _BSD_SOURCE for d_type field in struct dirent
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "informative.h" 
#include "disk_analysis.h"


static inline void *alloc_stat()
{
        return malloc_inf(sizeof(struct stat));
}

static inline void free_and_null(void **ptr)
{
        free(*ptr);
        *ptr = NULL;
}

static void *alloc_bin_tree()
{
        struct bin_tree *node;

        if ((node = malloc_inf(sizeof(struct bin_tree)))) {
                /* 
                 * I decided to NULL the left and right child here
                 * so I don't mistakely forget to do that in further 
                 * functions
                 */
                node->left = NULL;
                node->right = NULL;
        }
        return node;
}

static inline void free_fdata_fields(struct fdata *ptr)
{
	if (ptr->fstatus)
		free(ptr->fstatus);

	free(ptr->fpath);
	free(ptr->fname);
}

static void free_fdata(struct fdata *ptr)
{
	free_fdata_fields(ptr);
	free(ptr);
}

static void free_bin_tree(struct bin_tree *root)
{
        if (root->left)
                free_bin_tree(root->left);
        if (root->right)
                free_bin_tree(root->right);
	
	free_fdata(root->data);
	free(root);
}

static char *get_entry_path(const char *dir_path, const char *entry_name)
{
        char *entry_path;
        size_t len;

	/* The 2 stands for a slash and a null byte */
	len = strlen(dir_path) + strlen(entry_name) + 2;
        
	if ((entry_path = malloc_inf(len)))
                snprintf(entry_path, len, "%s/%s", dir_path, entry_name);
        
	return entry_path;
}

static inline void *alloc_fdata()
{
        return malloc_inf(sizeof(struct fdata));
}

/*
 * The fucntion uses stat() to get files status but it 
 * will not consider permission denied (EACCES) error as a failure.
 */
static int stat_custom_fail(const char *path, struct stat *statbuf)
{
        int retval;

        if ((retval = stat_inf(path, statbuf)))
                if (errno == EACCES)
                        retval = EACCES;
        return retval;
}

static inline void insert_rem_fdata_fields(struct fdata *data, unsigned int i,
					   const char *name, size_t name_len,
				           const char *path, size_t path_len)
{
	data->node_num = i;
	memcpy(data->fname, name, name_len);
	memcpy(data->fpath, path, path_len);
}

static int alloc_fdata_fields(struct fdata *ptr, size_t name_len, size_t path_len)
{	
	if (!(ptr->fname = malloc_inf(name_len)))
		goto err_out;
	if (!(ptr->fpath = malloc_inf(path_len)))
		goto err_free_fname;
	if (!(ptr->fstatus = alloc_stat())) 
		goto err_free_fpath;

	return 0;

err_free_fpath:
	free(ptr->fpath);
err_free_fname:
	free(ptr->fname);
err_out:
	return -1;

}

static struct fdata *get_fdata(const char *path, 
			       const char *entry_name, 
			       unsigned int i)
{
        const size_t name_len = strlen(entry_name) + 1;
        const size_t path_len = strlen(path) + 1;
        struct fdata *data;
	int ret;

        if ((data = alloc_fdata())) {
		if (alloc_fdata_fields(data, name_len, path_len))
			goto err_free_fdata;
		if ((ret = stat_custom_fail(path, data->fstatus))) {
			if (ret == -1)
				goto err_free_fdata_fields;
			
			free_and_null((void **) &data->fstatus);
		}
		insert_rem_fdata_fields(data, i, entry_name, 
				        name_len, path, path_len);
	}
        return data;

err_free_fdata_fields:
	free_fdata_fields(data);
err_free_fdata:
	free(data);
	
	return NULL;
}

/*
 * Decide which node between root->left or root->right
 */
static struct bin_tree *decide_which_node(struct bin_tree *root, 
					  struct bin_tree *new_node)
{
	struct bin_tree *retval;

	if (root->left)
		retval = root->right = new_node;
	else
		retval = root->left = new_node;

	return retval;
}

static struct bin_tree *get_current_node(struct bin_tree *root, 
					 struct bin_tree *new_node, 
					 const unsigned int parent)
{
	struct bin_tree *ret;

	if (root->data->node_num == parent) {
		return decide_which_node(root, new_node);
	} else if (root->left) {
		if ((ret = get_current_node(root->left, new_node, parent)))
			return ret;
	} else if (root->right) {
		if ((ret = get_current_node(root->right, new_node, parent)))
			return ret;
	} else {
		return NULL;
	}
}

static unsigned int get_parent_i(const unsigned int i)
{
	return ((i-1) / 2);
}

/*
 * A helper function for get_dirs_content()
 */
static struct bin_tree *_get_dirs_content(DIR *dp, const char *dir_path)
{
        struct bin_tree *new_node;
        struct bin_tree *current;
        struct bin_tree *root;
        struct dirent *entry;
	char *entry_path;
	unsigned int i;
        
	root = NULL;
	/* 
         * Reset errno to 0 to distnguish between 
         * error and end of directory in readdir_inf() 
         */
        errno = 0;

	for (i=0; (entry = readdir_inf(dp)); i++) {
		if (!(new_node = alloc_bin_tree()))
			/*error*/;
		if (!root)
			current = root = new_node;
		else
			current = get_current_node(root, new_node, 
						   get_parent_i(i));

		if (!(entry_path = get_entry_path(dir_path, entry->d_name)))
			goto err_out;
		if (!(current->data = get_fdata(entry_path, entry->d_name, i)))
			goto err_out;
	}
}

static struct bin_tree *get_dirs_content(const char *path)
{
        struct bin_tree *root = NULL;
        DIR *dp;

        if ((dp = opendir_inf(path))) {
                root = _get_dirs_content(dp, path);
                
                if (closedir_inf(dp) && root)
                        free_bin_tree(root);
        }
        return root;
}
