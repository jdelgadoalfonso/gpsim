/*
   Copyright (C) 1999-2000 T. Scott Dattalo

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

#ifndef __CMD_SET_H__
#define __CMD_SET_H__

class cmd_set : public command
{
public:
  cmd_set(void);
  void set(void);

  //void set(int bit_flag);
  void set(int bit_flag,int number);
  void set(cmd_options_num *con);
  //void set(cmd_options_str *cos);

};

extern cmd_set c_set;
#endif

