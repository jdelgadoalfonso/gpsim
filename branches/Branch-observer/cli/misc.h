/*
   Copyright (C) 1999 T. Scott Dattalo

This file is part of gpsim.

gpsim is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

gpsim is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with gpsim; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#ifndef __MISC_H__
#define __MISC_H__

extern int verbose; 

// miscellaneous definitions that are used

struct cmd_options {
  char *name;
  int  value;
  int  token_type;
};

   /* Command option with a numeric parameter */

struct cmd_options_num {
  cmd_options *co;
  int n;
};

   /* Command option with a float numeric parameter */

struct cmd_options_float {
  cmd_options *co;
  float f;
};

   /* Command option with a string parameter */

struct cmd_options_str {
  cmd_options *co;
  char *str;
};

// Char list.
// Here's a singly linked-list of char *'s.

struct char_list {
  char *name;
  char_list *next;
};

#endif
