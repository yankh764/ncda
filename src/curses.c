/*
---------------------------------------------------------
| License: GNU GPL-3.0                                  |
---------------------------------------------------------
| This source file contains all the necessary functions |
| for managing the ncurses windows                      |
---------------------------------------------------------
*/

#include "curses.h"


enum COLOR_PAIRS_NUM {
	BLUE_PAIR =  1,
	GREEN_PAIR = 2, 
	YELLOW_PAIR = 3,
	RED_PAIR = 4,
	CYAN_PAIR = 5,
	DEFAULT_PAIR = 6
};


static int display_fname_no_color(WINDOW *wp, struct list *const head)
{
	struct list *current;
	int retval;

	for (current=head; current; current=current->next)
		if ((retval = wprintw(wp, "%s\n", current->data->fname)))
			break;
	return retval;
}

static short get_reg_file_color_pair(struct stat *const statbuf)
{
	/* USE lstat() INSTEAD OF stat()*/
}

static short get_color_pair_num(struct stat *const statbuf)
{
	if (S_ISREG(statbuf->st_mode))
		return get_reg_file_color_pair(statbuf);
	else if (S_ISLNK(statbuf->st_mode))
		return CYAN_PAIR;
	else if (S_ISDIR(statbuf->st_mode))
		return BLUE_PAIR;
	else if (S_ISCHR(statbuf->st_mode) || S_ISBLK(statbuf->st_mode))
		return YELLOW_PAIR;
	else
		return DEFAULT_PAIR;
}

static int attron_proper_color(WINDOW *wp, struct stat *const statbuf)
{
	short pair_num;

	if (statbuf)
		pair_num = get_color_pair_num(statbuf);
	else
		pair_num = DEFAULT_PAIR;
	
	return wattron(wp, COLOR_PAIR(pair_num));
}

static int display_fname_color(WINDOW *wp, struct list *const head)
{
	struct list *current;

	for (current=head; current; current=current->next) {
		if (attron_proper_color(wp, current->data->fstatus))
			return -1;
		if (wprintw(wp, "%s\n", current->data->fname))
			return -1;
	}
	/* Reset the colors */
	return wattron(wp, COLOR_PAIR(DEFAULT_PAIR));
}

int nc_display_fname(WINDOW *wp, struct list *const head) 
{
	if (__COLORED_OUTPUT)
		return display_fname_color(wp, head);
	else
		return display_fname_no_color(wp, head);

}

static int display_fpath_no_color(WINDOW *wp, struct list *const head)
{
	struct list *current;
	int retval;

	for (current=head; current; current=current->next)
		if ((retval = wprintw(wp, "%s\n", current->data->fpath)))
			break;
	return retval;
}

static int display_fpath_color(WINDOW *wp, struct list *const head)
{

}

int nc_display_fpath(WINDOW *wp, struct list *const head)
{

}

static inline int init_color_pairs()
{
	return (init_pair(BLUE_PAIR,   COLOR_BLUE,   -1) ||
		init_pair(GREEN_PAIR,  COLOR_GREEN,  -1) || 
		init_pair(YELLOW_PAIR, COLOR_YELLOW, -1) ||
		init_pair(RED_PAIR,    COLOR_RED,    -1) || 
		init_pair(CYAN_PAIR,   COLOR_CYAN,   -1) ||
		init_pair(DEFAULT_PAIR,        -1,   -1));
}

static inline int start_color_if_supported()
{
	if (has_colors())
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

int nc_init_setup()
{
	return (start_color_if_supported() || cbreak() ||
		noecho() || init_local_setup(stdscr));
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
