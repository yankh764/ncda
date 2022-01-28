#ifndef _CURSES_MAN_H
#define _CURSES_MAN_H

#include <ncurses.h>
#include "structs.h"

extern bool COLORED_OUTPUT;

int nc_init_setup();
int nc_initial_display(WINDOW *, const struct dtree *, const char *);
int nc_man_input(WINDOW *);

#endif
