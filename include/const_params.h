/*
---------------------------------------------------------
| License: GNU GPL-3.0                                  |
---------------------------------------------------------
| This header file contains all the necessary constant  |
| parameters and settings for the ncurses session       |
---------------------------------------------------------
*/
#ifndef _CONST_PARAMS_H
#define _CONST_PARAMS_H

#include <ncurses.h>

const int _def_attrs = A_BOLD;
const char _separator = '|';
const int _blank = 1;
const int _sep_blank = 2 * _blank;
const int _fsize_x = 0;
const int _mtime_x = 12;
const int _fname_x = 27;
const int _max_print_fsize = 8;
const int _max_print_mtime = 11;

#endif
