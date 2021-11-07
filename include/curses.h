#ifndef _CURSES_H
#define _CURSES_H

#include <ncurses.h>
#include "structs.h"

int nc_display_fname(struct list *const);
int nc_display_fpath(struct list *const);
int nc_init_setup();

#endif
