/*
   Copyright (C) 2004
   T. Scott Dattalo

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

#if !defined (__REGWIN_H__)
#define __REGWIN_H__

#include "gui_object.h"

//========================================================================
//========================================================================
// experimental register window replacement

//========================================================================
class RegCell
{
public:
  RegCell(int _address, const char *initialText,int _cell_width=2);
  GtkWidget *getWidget() { return entry; }
  gint ButtonPressEvent(GtkWidget *widget, GdkEventButton *event);
private:
  int address;
  int cell_width;
  GtkWidget *entry;

};

//========================================================================
//
class RegWindow : public GUI_Object
{
public:
  RegWindow();

  GtkWidget *Build();
  gint ButtonPressEvent(GtkWidget *widget, GdkEventButton *event);

private:

  int nRows;
  int nCols;

};


#endif //  !defined (__REGWIN_H__)
