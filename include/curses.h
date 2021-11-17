#ifndef _CURSES_H
#define _CURSES_H

#include <ncurses.h>
#include "structs.h"

extern bool  COLORED_OUTPUT;

int nc_display_fname(WINDOW *, struct list *const);
int nc_display_fpath(WINDOW *, struct list *const);
int nc_init_setup();
WINDOW *nc_newwin(int, int, int, int);

#endif
