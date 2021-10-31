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

/*
 * I decided to pass a function pointer as an argument for freeing
 * bin_tree node's data instead of hard coding it with just one function. 
 */
static void free_bin_tree(struct bin_tree *root, void (*free_data) (void *))
{
        if (root->left)
                free_bin_tree(root->left, free_data);
        if (root->right)
                free_bin_tree(root->right, free_data);
	free_data(root->data);
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

static inline void free_fdata_fields(struct fdata *ptr)
{
	if (ptr->fstatus)
		free(ptr->fstatus);
	free(ptr->fpath);
	free(ptr->fname);
}

/*
 * Using void pointer to the ptr argument here, to pass 
 * this function with no compiler warnings to free_bin_tree()
 */
static void free_fdata(void *ptr)
{
	free_fdata_fields(ptr);
	free(ptr);
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

static struct bin_tree *get_current_node(struct bin_tree *root, 
					 struct bin_tree *new_node)
{

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

        while ((entry = readdir_inf(dp))) {
		if (!(new_node = alloc_bin_tree()))
			goto err_out;
		if (!root)
			current = root = new_node;
		else
			current = get_current_node(current, new_node);

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
                        free_bin_tree(root, free_fdata);
        }
        return root;
}
