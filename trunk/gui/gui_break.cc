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
#include <errno.h>

#include "../config.h"

#ifdef HAVE_GUI

#include <unistd.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <glib.h>
#include <string.h>

//#include <iostream.h>

#include <gtkextra/gtkcombobox.h>
#include <gtkextra/gtkbordercombo.h>
#include <gtkextra/gtkcolorcombo.h>
#include <gtkextra/gtksheet.h>
//#include <gtkextra/gtksheetentry.h>

#include "gui.h"

extern GUI_Processor *gp;

extern int gui_animate_delay; // in milliseconds

void gui_simulation_has_stopped(gpointer callback_data)
{
    while(gtk_events_pending())
	gtk_main_iteration();

  if(callback_data)
    {
      GUI_Processor *gp = (GUI_Processor *) callback_data;

      gp->regwin_ram->Update();
      gp->regwin_eeprom->Update();
      gp->status_bar->Update();
      gp->program_memory->Update();
      gp->source_browser->Update();
      gp->watch_window->Update();
      gp->stack_window->Update();
      gp->breadboard_window->Update();
      gp->trace_window->Update();
      gp->profile_window->Update();
      gp->stopwatch_window->Update();
    }

  if(gui_animate_delay!=0)
      usleep(1000*gui_animate_delay);

  while(gtk_events_pending())
      gtk_main_iteration();
}



/*
 * link_src_to_gpsim
 *
 * After a new program has been loaded by gpsim, this routine is called
 * so that the gui can create links to it. This consists of creating
 * a 'cross_reference' structure and attaching it to each pic instruction.
 * The information in the cross_reference structure is a pointer to the
 * gp.
 */

void link_src_to_gpsim(GUI_Processor *gp)
{
  int i,*address,pm_size;
  struct cross_reference_to_gui *cross_reference;


  if(gp) {

      // Create a cross reference between the pic's program memory and the gui.
      pm_size =  gp->cpu->program_memory_size();

      if(verbose) {
	printf("link_src_to_gpsim\n");
	printf(" processor pma = %d\n",pm_size);
      }

      for(i=0; i < pm_size; i++) {
	
	cross_reference = (struct cross_reference_to_gui *) malloc(sizeof(struct cross_reference_to_gui));
	cross_reference->parent_window_type =   WT_asm_source_window;
	cross_reference->parent_window = (gpointer) gp;
	address = (int *) malloc(sizeof(int));
	*address = i;

	cross_reference->data = (gpointer) address;
	cross_reference->update = SourceBrowser_update_line;
	cross_reference->remove = NULL;

	gp->cpu->pma.assign_xref(i,(gpointer) cross_reference);
      }
    }
}

#endif //HAVE_GUI
