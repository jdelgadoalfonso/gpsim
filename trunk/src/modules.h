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
  modules.h

  The base class for modules is defined here.

  Include this file into yours for creating custom modules.
 */


#ifndef __MODULES_H__
#define __MODULES_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/*

port
 pin
  type
  number
  position in port

struct Module_Pin {
 char *port_name,
 char *pin_name,
 PIN_TYPE pin_type,
 int  pin_number
 int  pin_position
} 
*/


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __MODULES_H__
