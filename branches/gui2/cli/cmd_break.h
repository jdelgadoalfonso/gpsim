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

#ifndef __CMD_BREAK_H__
#define __CMD_BREAK_H__

#include <unistd.h>
#include <glib.h>
#include "command.h"

class cmd_break : public command
{
public:

  cmd_break(void);
  void list(void);

  void set_break(cmd_options *co, ExprList_t *el=0);
  void set_break(cmd_options *co, char *sym);
private:
  
  void set_break(int bit_flag);
  void set_break(int bit_flag,guint64 value);
  void set_break(int bit_flag,guint64 reg, guint64 value,guint64 mask);
  //void set_break(int bit_flag,char *sym);
  void set_break(int bit_flag,char *sym, int value,int mask);
   

};

extern cmd_break c_break;
#endif

