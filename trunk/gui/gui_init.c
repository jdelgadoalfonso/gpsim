/*
   Copyright (C) 1998,1999,2000,2001
   T. Scott Dattalo and Ralf Forsberg

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



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../config.h"
#ifdef HAVE_GUI

#include <unistd.h>
#include <gtk/gtk.h>

#include <gdk/gdktypes.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>

//#include <iostream.h>

#include <gtkextra/gtkcombobox.h>
#include <gtkextra/gtkbordercombo.h>
#include <gtkextra/gtkcolorcombo.h>
#include <gtkextra/gtksheet.h>
//#include <gtkextra/gtksheetentry.h>


#include "gui.h"

GdkColor black_color;
GdkColor high_output_color;
GdkColor low_output_color;

void gui_styles_init(void)
{
    GdkColormap *colormap = gdk_colormap_get_system();

    gdk_color_parse("red",&high_output_color);
    gdk_color_parse("green",&low_output_color);

    g_assert(gdk_color_parse("black",&black_color)!=FALSE);


    gdk_colormap_alloc_color(colormap, &high_output_color,FALSE,TRUE);
    gdk_colormap_alloc_color(colormap, &low_output_color,FALSE,TRUE);
    gdk_colormap_alloc_color(colormap, &black_color,FALSE,TRUE);

}
#endif // HAVE_GUI
