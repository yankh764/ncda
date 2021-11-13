/*
---------------------------------------------------------
| License: GNU GPL-3.0                                  |
---------------------------------------------------------
| This source file contains all the necessary functions |
| for managing the ncurses windows                      |
---------------------------------------------------------
*/

#include "curses.h"


int nc_display_fname(WINDOW *wp, struct list *const head)
{
	struct list *current;
	int retval;

	for (current=head; current; current=current->next)
		if ((retval = wprintw(wp, "%s\n", current->data->fname)))
			break;
	return retval;
}

int nc_display_fpath(WINDOW *wp, struct list *const head)
{
	struct list *current;
	int retval;

	for (current=head; current; current=current->next)
		if ((retval = wprintw(wp, "%s\n", current->data->fpath)))
			break;
	return retval;
}

static inline int start_color_if_supported()
{
	if (has_colors())
		return start_color();
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
