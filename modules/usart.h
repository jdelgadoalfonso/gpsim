/*
   Copyright (C) 1998,1999,2000,2001 T. Scott Dattalo

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


#ifndef __USART_MODULE_H__
#define __USART_MODULE_H__

#include "../config.h"

#ifdef HAVE_GUI
#include <gtk/gtk.h>
#endif


#include <glib.h>

#include "../src/modules.h"
#include "../src/packages.h"
#include "../src/stimuli.h"
#include "../src/symbol.h"
#include "../src/trace.h"
#include "../src/uart.h"

class USART_IOPORT;

class USARTModule : public ExternalModule
{
 public:

  USART_IOPORT *port;
  USART_MODULE14 *usart;    // Use the PIC usart core 

  // Inheritances from the Package class
  virtual void create_iopin_map(void);

  // Inheritances from the Module class
  virtual void set_attribute(char *attr, char *val);

  USARTModule(char *new_name = NULL);

  static ExternalModule *USART_construct(char *new_name=NULL);

};
#endif //  __USART_MODULE_H__
