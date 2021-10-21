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

/*
static void *alloc_stat()
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

static void free_bin_tree(struct bin_tree *root)
{
        if (root->left)
                free_bin_tree(root->left);
        if (root->right)
                free_bin_tree(root->right);
        
        free_bin_tree(root);
}

/*
 * An helper function for get_dirs_content()
 */
static struct bin_tree *_get_dirs_content(DIR *dp, const char *dir_path)
{
        struct bin_tree *currnet;
        struct bin_tree *root;
        struct dirent *entry;
        
        if ((root = alloc_bin_tree())) {
                /* 
                 * Reset errno to 0 to distnguish between 
                 * error and end of directory in readdir_inf() 
                 */
                errno = 0;

                while ((entry = readdir_inf(dp))) {
                           
                }
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
