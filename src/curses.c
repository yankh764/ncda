/*
---------------------------------------------------------
| License: GNU GPL-3.0                                  |
---------------------------------------------------------
| This source file contains all the necessary functions |
| for managing the ncurses windows                      |
---------------------------------------------------------
*/

#include "curses.h"


enum COLORS_NUM {
	WHITE = 1, 
	BLUE = 2,
	GREEN = 3, 
	YELLOW = 4, 
	CYAN = 5
};


static inline int display_fname_no_color(WINDOW *wp, struct list *const head)
{
	struct list *current;
	int retval;

	for (current=head; current; current=current->next)
		if ((retval = wprintw(wp, "%s\n", current->data->fname)))
			break;
	return retval;
}

static int attron_proper_colors(WINDOW *wp, struct list *const node)
{
	if (S_ISDIR(node->fstatus->st_mode))
		return wattron(wp, COLOR_PAIR(BLUE));
}

static inline int display_fname_color(WINDOW *wp, struct list *const head)
{
	struct list *current;
	int retval;

	for (current=head; current; current=current->next)
		if ((retval = wprintw(wp, "%s\n", current->data->fname)))
			break;
	return retval;
}

int nc_display_fname(WINDOW *wp, struct list *const head) 
{
	if (__COLORED_OUTPUT)
		return display_fname_color(wp, head);
	else
		return display_fname_no_color(wp, head);

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

static inline int init_color_pairs()
{
	return (init_pair(WHITE,  COLOR_WHITE,  -1) || 
		init_pair(BLUE,   COLOR_BLUE,   -1) ||
		init_pair(GREEN,  COLOR_GREEN,  -1) || 
		init_pair(YELLOW, COLOR_YELLOW, -1) ||
		init_pair(CYAN,   COLOR_CYAN,   -1));
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
