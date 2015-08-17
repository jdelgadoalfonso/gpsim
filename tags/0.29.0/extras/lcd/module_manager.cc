/*
   Copyright (C) 1998,1999,2000 T. Scott Dattalo

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

/*
  gpsim_modules.cc

  gpsim supports modules, or thingies, that are not part of gpsim proper.
This is to say, that the modules are not compiled and linked with the
core gpsim software. Instead, they are compiled and linked separately and
then dynamically loaded by gpsim. This approach provides a flexibility
to the user to create customized objects for simulation purposes. The
big benefit of course, is that the user doesn't have to get bogged down
in to the nitty-gritty details of the way gpsim is designed. The templates
provided here can serve as a relatively simple example of how one may
go about creating customized modules.

Please see the README.MODULES for more details on how modules are intended
to be used.

Here are a list of functions that a gpsim compliant module library should
support:

  void mod_list(void) - Prints a list of the modules in a library
  Module_Types * get_mod_list(void)  - Obtain pointer to the list of modules
*/


/* IN_MODULE should be defined for modules */
#define IN_MODULE

#include <cstdio>
#include <glib.h>
#include <src/modules.h>
#include "config.h"
#ifdef HAVE_GUI
#include "lcd.h"
#endif


Module_Types available_modules[] =
{
#ifdef HAVE_GUI
  { {"lcd_display", "lcd_2X20"}, LcdDisplay::construct},
  { {"lcd_20x4", "lcd_20x4"}, LcdDisplay20x4::construct},
  { {"lcd_dt161A",  "lcd_2X8"},  LcdDisplayDisplaytech161A::construct},
#endif

  // No more modules
  { {NULL, NULL}, NULL}
};

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/********************************************************************************
 * get_mod_list - Display all of the modules in this library.
 *
 * This is a required function for gpsim compliant libraries.
 */

  Module_Types * get_mod_list(void)
  {

    return available_modules;

  }

#ifdef __cplusplus
}
#endif /* __cplusplus */
