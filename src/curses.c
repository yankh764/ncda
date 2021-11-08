/*
---------------------------------------------------------
| License: GNU GPL-3.0                                  |
---------------------------------------------------------
| This source file contains all the necessary functions |
| for managing the ncurses windows                      |
---------------------------------------------------------
*/

#include "curses.h"


int nc_display_fname(struct list *const head)
{
	struct list *current;
	int retval;

	for (current=head; current; current=current->next)
		if ((retval = printw("%s\n", current->data->fname)))
			break;
	return retval;
}

int nc_display_fpath(struct list *const head)
{
	struct list *current;
	int retval;

	for (current=head; current; current=current->next)
		if ((retval = printw("%s\n", current->data->fpath)))
			break;
	return retval;
}

int nc_init_setup()
{
	return (start_color() || cbreak() || keypad(stdscr, TRUE) || 
		noecho() || attrset(A_BOLD));
}
