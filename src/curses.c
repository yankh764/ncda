/*
---------------------------------------------------------
| License: GNU GPL-3.0                                  |
---------------------------------------------------------
| This source file contains all the necessary functions |
| for managing the ncurses windows                      |
---------------------------------------------------------
*/

/*
 * Defining _GNU_SOURCE macro since it achives all the desired
 * feature test macro requirements, which are:
 *     1) _DEFAULT_SOURCE || _BSD_SOURCE for file type and mode macros
 */
#define _GNU_SOURCE
#include <stdbool.h>
#include "curses.h"

/*
 * I defined these macros to get the appropriate entry color in an effiecient,
 * fast and clear way without making an external function call that will 
 * increase the overall execution time with the function-call overhead.
 */
#define IS_EXEC(file_mode)	      ((file_mode & S_IXUSR) || \
				       (file_mode & S_IXGRP) || \
				       (file_mode & S_IXOTH))

#define SHLD_BE_BLUE(file_mode)	       S_ISDIR(file_mode)
#define SHLD_BE_CYAN(file_mode)	       S_ISLNK(file_mode)
#define SHLD_BE_MAGENTA(file_mode)     S_ISSOCK(file_mode)
#define SHLD_BE_GREEN(file_mode)       S_ISREG(file_mode) && IS_EXEC(file_mode)
#define SHLD_BE_YELLOW(file_mode)     (S_ISCHR(file_mode) || \
				       S_ISBLK(file_mode) || \
				       S_ISFIFO(file_mode))

enum COLOR_PAIRS_NUM {
	BLUE_PAIR =  1,
	GREEN_PAIR = 2, 
	YELLOW_PAIR = 3,
	CYAN_PAIR = 4,
	MAGENTA_PAIR = 5,
	DEFAULT_PAIR = 6
};
struct list *highligted_node; 


static short get_color_pair_num(mode_t st_mode)
{
	if (SHLD_BE_BLUE(st_mode))
		return BLUE_PAIR;
	else if (SHLD_BE_GREEN(st_mode))
		return GREEN_PAIR;
	else if (SHLD_BE_CYAN(st_mode))
		return CYAN_PAIR;
	else if (SHLD_BE_YELLOW(st_mode)) 
		return YELLOW_PAIR;
	else if (SHLD_BE_MAGENTA(st_mode))
		return MAGENTA_PAIR;
	else
		return DEFAULT_PAIR;
}

static inline short proper_color_pair(struct stat *statbuf)
{
	return (COLORED_OUTPUT && statbuf) ? 
		get_color_pair_num(statbuf->st_mode) : 0;
}

/*
 * Get the proper end of a string. If the file is a directory the end of
 * the string will be slash ('/'), else the end of a string will be blank
 * character (' ').
 */
static inline char proper_eos(struct stat *const statbuf)
{
	return (statbuf && S_ISDIR(statbuf->st_mode)) ? '/' : ' ';
}

static inline void insert_cdata_fields(struct doubly_list *ptr, int i)
{
	struct stat *statbuf = ptr->data->file_data->fstatus;

	ptr->data->curses_data->color_pair = proper_color_pair(statbuf);
	ptr->data->curses_data->y_cord = i + 1;
	ptr->data->curses_data->eos = proper_eos(statbuf);
}

void nc_get_cdata_fields(struct doubly_list *head)
{
	struct doubly_list *current;
	int i;
	
	for (current=head, i=0; current; current=current->next, i++)
		insert_cdata_fields(current, i);
}

static int display_fname_color(WINDOW *wp, struct doubly_list *const head)
{
	struct doubly_list *current;

	for (current=head; current; current=current->next) {
		if (wattron(wp, COLOR_PAIR(current->data->curses_data->color_pair)))
			return -1;
		if (wprintw(wp, "%s%c\n", current->data->file_data->fname, 
					  current->data->curses_data->eos))
			return -1;
	}
	/* Reset the colors */
	return wattron(wp, COLOR_PAIR(DEFAULT_PAIR));
}

static int display_fname_no_color(WINDOW *wp, struct doubly_list *const head)
{
	struct doubly_list *current;
	int retval;

	for (current=head; current; current=current->next)
		if ((retval = wprintw(wp, "%s%c\n", 
				      current->data->file_data->fname, 
				      current->data->curses_data->eos)))
			break;
	return retval;
}

int nc_display_fname(WINDOW *wp, struct doubly_list *const head) 
{
	if (COLORED_OUTPUT)
		return display_fname_color(wp, head);
	else
		return display_fname_no_color(wp, head);
}

static int display_fpath_color(WINDOW *wp, struct doubly_list *const head)
{
	struct doubly_list *current;

	for (current=head; current; current=current->next) {
		if (wattron(wp, COLOR_PAIR(current->data->curses_data->color_pair)))
			return -1;
		if (wprintw(wp, "%s%c\n", current->data->file_data->fpath, 
			                  current->data->curses_data->eos))
			return -1;
	}
	/* Reset the colors */
	return wattron(wp, COLOR_PAIR(DEFAULT_PAIR));
}

static int display_fpath_no_color(WINDOW *wp, struct doubly_list *const head)
{
	struct doubly_list *current;
	int retval;

	for (current=head; current; current=current->next)
		if ((retval = wprintw(wp, "%s%c\n", 
				      current->data->file_data->fpath, 
				      current->data->curses_data->eos)))
			break;
	return retval;
}

int nc_display_fpath(WINDOW *wp, struct doubly_list *const head)
{
	if (COLORED_OUTPUT)
		return display_fpath_color(wp, head);
	else
		return display_fpath_no_color(wp, head);
}

static inline int init_color_pairs()
{
	return (init_pair(BLUE_PAIR,    COLOR_BLUE,    -1) ||
		init_pair(GREEN_PAIR,   COLOR_GREEN,   -1) || 
		init_pair(YELLOW_PAIR,  COLOR_YELLOW,  -1) ||
		init_pair(CYAN_PAIR,    COLOR_CYAN,    -1) ||
		init_pair(MAGENTA_PAIR, COLOR_MAGENTA, -1) ||
		init_pair(DEFAULT_PAIR, -1,            -1));
}

static int start_color_if_supported()
{
	if ((COLORED_OUTPUT = has_colors()))
		return (use_default_colors() || 
			start_color()        || 
			init_color_pairs());
	else
		return 0;
}

static inline int init_local_setup(WINDOW *wp)
{
	return (keypad(wp, TRUE) || wattrset(wp, A_BOLD));
}

/*
 * Make the cursor invisible 
 */
static inline int invisibilize_cursor()
{
	return curs_set(0) != ERR ? 0 : ERR;
}

int nc_init_setup()
{
	return (start_color_if_supported() || cbreak() ||
		noecho() || invisibilize_cursor() || 
		init_local_setup(stdscr));
}

static inline int del_and_null_win(WINDOW **wp)
{
	int retval;

	retval = delwin(*wp);
	*wp = NULL;

	return retval;
}

/*
 * Return a new window with some initial setup
 */
WINDOW *nc_newwin(int lines_num, int cols_num, int begin_y, int begin_x)
{
	WINDOW *wp;

	if ((wp = newwin(lines_num, cols_num, begin_y, begin_x)))
		if (init_local_setup(wp))
			del_and_null_win(&wp);
	return wp;
}

/*
 * Check if c is in the navigation keys, and return the value of the 
 * operation that this key does
 *
static int in_navigation_keys(int c)
{
	if (c == KEY_UP || c == 'k')
		return KEY_UP;
	else if (c == KEY_DOWN || c == 'j')
		return KEY_DOWN;
	else 
		return 0;
}*/
/*
static int get_updated_y(int key, int max_y, int y)
{
	if (y && key == KEY_UP)
		y--;
	else if (y < max_y && key == KEY_DOWN)
		y++;

	return y;
}

static int update_highlight_loc(WINDOW *wp, int key, int y)
{
	return mvwchgat(wp, y, 0, -1, A_STANDOUT, DEFAULT_PAIR, NULL);
}

* 
 * An helper function for nc_man_input()
 *
static int _nc_man_input(WINDOW *wp, int c)
{
	int key;

	if ((key = in_navigation_keys(c))) {
		if (update_highlight_loc(wp, key))
			return -1;
	}
}

int nc_man_input(WINDOW *wp, struct list *const )
{
	int c;

	while ((c = wgetch(wp)) != ERR)
		if (_nc_man_input(wp, c))
			return -1;
	return c;
}*/
