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


#include <gtkextra/gtkcombobox.h>
#include <gtkextra/gtkbordercombo.h>
#include <gtkextra/gtkcolorcombo.h>
#include <gtkextra/gtksheet.h>


#include "gui.h"
#include "gui_statusbar.h"
#include "gui_regwin.h"

typedef enum {
    MENU_TIME_USECONDS,
    MENU_TIME_MSECONDS,
    MENU_TIME_SECONDS,
    MENU_TIME_HHMMSS
} menu_id;

typedef struct _menu_item {
    char *name;
    menu_id id;
} menu_item;



//========================================================================
//
// A LabeledEntry is an object consisting of gtk entry
// widget that is labeled (with a gtk lable widget)
//

class LabeledEntry {
public:
  GtkWidget *label;
  GtkWidget *entry;
  StatusBar_Window *sbw;

  LabeledEntry(void);
  void Create(GtkWidget *box,char *clabel, int string_width,bool isEditable);
  void NewLabel(char *clabel);
  virtual void Update(void);
  void AssignParent(StatusBar_Window *);
  virtual void put_value(unsigned int);

};

class RegisterLabeledEntry : public LabeledEntry {
public:

  Register *reg;
  char *pCellFormat;

  RegisterLabeledEntry(Register *);

  virtual void put_value(unsigned int);
  void AssignRegister(Register *new_reg);
  virtual void Update(void);

};

class CyclesLabeledEntry : public LabeledEntry {
public:

  CyclesLabeledEntry();
  virtual void Update(void);
};

class PCLabeledEntry : public LabeledEntry {
public:

  PCLabeledEntry();
  virtual void Update(void);
  virtual void put_value(unsigned int);
  void AssignPma(ProgramMemoryAccess *new_pma) { pma = new_pma;}

  ProgramMemoryAccess *pma;
};

class TimeLabeledEntry : public LabeledEntry {
public:
  TimeLabeledEntry();
  virtual void Update(void);
  GtkWidget *build_menu();

  void set_time_format(menu_id id)
  {
    time_format = id;
  }

  GtkWidget *menu;
  menu_id time_format;

};
struct popup_data {
  TimeLabeledEntry *tle;
  menu_id id;
};


//========================================================================

class StatusBarXREF : public CrossReferenceToGUI
{
public:

  void Update(int new_value)
  {

    StatusBar_Window *sbw;

    sbw  = (StatusBar_Window *) (parent_window);
    sbw->Update();

  }
};

//========================================================================


LabeledEntry::LabeledEntry(void)
{
  label = 0;
  entry = 0;
  sbw = 0;
}

void LabeledEntry::Create(GtkWidget *box,
			  char *clabel, 
			  int string_width,
			  bool isEditable=true)
{

  label = (GtkWidget *)gtk_label_new (clabel);
    
  gtk_misc_set_alignment (GTK_MISC (label), 1.0, 0.5);
  gtk_widget_set_usize (label, 0, 15);
  gtk_box_pack_start (GTK_BOX (box), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  entry = gtk_entry_new ();

  gtk_entry_set_text (GTK_ENTRY (entry), "----");

#if GTK_MAJOR_VERSION >= 2
  gtk_widget_set_usize (entry,
			string_width * gdk_string_width (gtk_style_get_font(entry->style), "9") + 6,
			-1);
#else
  gtk_widget_set_usize (entry,
			string_width * gdk_string_width (entry->style->font, "9") + 6,
			-1);
#endif

  gtk_box_pack_start (GTK_BOX (box), entry, FALSE, FALSE, 0);

  gtk_widget_show (entry);

  if(!isEditable)
    gtk_entry_set_editable(GTK_ENTRY(entry),0);

}

void LabeledEntry::AssignParent(StatusBar_Window *new_sbw)
{
  sbw = new_sbw;
}

void LabeledEntry::Update(void)
{
  //  if(sbw)
  //    sbw->Update();

}

void LabeledEntry::put_value(unsigned int new_value)
{

}

void LabeledEntry::NewLabel(char *clabel)
{
  if(label)
    gtk_label_set_text(GTK_LABEL(label),clabel);

}
//------------------------------------------------------------------------
RegisterLabeledEntry::RegisterLabeledEntry(Register *new_reg) 
 : LabeledEntry()
{
  reg = new_reg;
  pCellFormat=0;
}

void RegisterLabeledEntry::put_value(unsigned int new_value)
{
  if(reg)
    reg->put_value(new_value);
}

void RegisterLabeledEntry::Update(void)
{
  char buffer[32];

  if(reg) {

    unsigned int value = reg->get_value();

    const char *format = "0x%02x";

    sprintf(buffer,format,value);

    gtk_entry_set_text (GTK_ENTRY (entry), buffer);

  }
}
void RegisterLabeledEntry::AssignRegister(Register *new_reg)
{
  reg = new_reg;

  if(pCellFormat)
    delete pCellFormat;

  if(reg) {
    pCellFormat = new char[10];
    sprintf(pCellFormat,"%%0%dx",reg->get_cpu()->register_size()*2);

  }
}

//------------------------------------------------------------------------
CyclesLabeledEntry::CyclesLabeledEntry()
{
}

void CyclesLabeledEntry::Update(void)
{
  char buffer[32];
  sprintf(buffer,"0x%016Lx",cycles.value);
  gtk_entry_set_text (GTK_ENTRY (entry), buffer);
}

TimeLabeledEntry::TimeLabeledEntry()
{
  time_format = MENU_TIME_USECONDS;
  menu = 0;
}

void TimeLabeledEntry::Update()
{
  char buffer[32];
  double time_db = 4.0 * cycles.value / (double)gp->cpu->time_to_cycles(1.0);

  if(time_format==MENU_TIME_USECONDS) {
    time_db *= 1e6;
    sprintf(buffer,"%19.2f us",time_db);
  }
  else if(time_format==MENU_TIME_MSECONDS) {
    time_db *= 1e3;
    sprintf(buffer,"%19.3f ms",time_db);
  }
  else if(time_format==MENU_TIME_HHMMSS) {
    double v=time_db;
    int hh=(int)(v/3600),mm,ss,cc;
    v-=hh*3600.0;
    mm=(int)(v/60);
    v-=mm*60.0;
    ss=(int)v;
    cc=(int)(v*100.0+0.5);
    sprintf(buffer,"    %02d:%02d:%02d.%02d",hh,mm,ss,cc);
  }
  else {
    sprintf(buffer,"%19.3f s",time_db);
  }
  gtk_entry_set_text (GTK_ENTRY (entry), buffer);

}

//----------------------------------------
// called when user has selected a menu item
static void
popup_activated(GtkWidget *widget, gpointer data)
{
  if(!widget || !data)
  {
    printf("Warning popup_activated(%x,%x)\n",(unsigned int)widget,(unsigned int)data);
    return;
  }
    
  popup_data *pd = (popup_data *)data;
  if(pd->tle) {
    pd->tle->set_time_format(pd->id);
    pd->tle->Update();
  }
}

GtkWidget * TimeLabeledEntry::build_menu(void)
{
  static menu_item menu_items[] = {
    {"Micro seconds", MENU_TIME_USECONDS},
    {"Mili seconds", MENU_TIME_MSECONDS},
    {"Seconds", MENU_TIME_SECONDS},
    {"HH:MM:SS.CC", MENU_TIME_HHMMSS}
  };


  GtkWidget *item;
  unsigned int i;

  menu=gtk_menu_new();

  item = gtk_tearoff_menu_item_new ();
  gtk_menu_append (GTK_MENU (menu), item);
  gtk_widget_show (item);
  
  
  for (i=0; i < (sizeof(menu_items)/sizeof(menu_items[0])) ; i++){
    item=gtk_menu_item_new_with_label(menu_items[i].name);

    popup_data *pd = new popup_data;
    pd->tle = this;
    pd->id = menu_items[i].id;

    gtk_signal_connect(GTK_OBJECT(item),"activate",
		       (GtkSignalFunc) popup_activated,
		       pd);

    gtk_widget_show(item);
    gtk_menu_append(GTK_MENU(menu),item);
  }
  
  return menu;
}

//------------------------------------------------------------------------
PCLabeledEntry::PCLabeledEntry()
{
  pma = 0;
}

void PCLabeledEntry::Update()
{
  char buffer[32];

  if(pma) {
    sprintf(buffer,"0x%04x",pma->get_PC());
    gtk_entry_set_text (GTK_ENTRY (entry), buffer);
  }

}

void PCLabeledEntry::put_value(unsigned int value)
{
  if(pma)
    pma->set_PC(value);
}

void StatusBar_Window::Update(void)
{

  if( !created)
      return;
  
  //update the displayed values

  if(!gp || !gp->cpu)
    return;

  status->Update();
  W->Update();
  pc->Update();
  cpu_cycles->Update();

  time->Update();
}

//------------------------------------------------------------------------
static void LabeledEntry_callback(GtkWidget *entry, LabeledEntry *le)
{
  const char *text;
  unsigned int value;
  char *bad_position;

  if(!gp || !gp->cpu || !le || !le->entry)
    return;

  text=gtk_entry_get_text (GTK_ENTRY (le->entry));
    
  value = strtoul(text, &bad_position, 16);
  if( strlen(bad_position) )
    return;  /* string contains an invalid number */

  le->put_value(value);

  if(le->sbw)
    le->sbw->Update();

  return;
}
/*
static void pc_callback(GtkWidget *entry, PCLabeledEntry *pcle)//StatusBar_Window *sbw)
{
  const char *text;
  unsigned int value;
  char *bad_position;

  if(!pcle || !pcle->pma)
    return;

  text=gtk_entry_get_text (GTK_ENTRY (entry));
    
  value = strtoul(text, &bad_position, 16);
  if( strlen(bad_position) )
    return;

  pcle->pma->set_PC(value);

  if(pcle->
  sbw->Update();
}
*/


// button press handler
static gint
do_popup(GtkWidget *widget, GdkEventButton *event, TimeLabeledEntry *tle)
{
    if(!widget || !event || !tle)
      return 0;
  
    if( (event->type == GDK_BUTTON_PRESS) &&  (event->button == 3) )
    {
	gtk_menu_popup(GTK_MENU(tle->menu), 0, 0, 0, 0,
			   3, event->time);
	// It looks like we need it to avoid a selection in the entry.
	// For this we tell the entry to stop reporting this event.
	gtk_signal_emit_stop_by_name(GTK_OBJECT(tle->entry),"button_press_event");
    }
    return FALSE;
}

/*
 * CreateStatusBar
 *
 * Create the status bar at the bottom of the window
 * 
 *
 * vbox_main - The box to which we will append.
 *
 */ 

void StatusBar_Window::Create(GtkWidget *vbox_main)
{
  GtkWidget *hbox;

  /* --- Create h-box for holding the status line --- */
  hbox = gtk_hbox_new (FALSE, 0);

  /* --- Put up h-box --- */
  gtk_box_pack_end (GTK_BOX (vbox_main), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  status = new RegisterLabeledEntry(0);
  status->Create(hbox,"Status:", 4);

  gtk_signal_connect(GTK_OBJECT(status->entry), "activate",
		     GTK_SIGNAL_FUNC(LabeledEntry_callback),
		     status);

  W = new RegisterLabeledEntry(0);
  W->Create(hbox,"W:", 4);
  gtk_signal_connect(GTK_OBJECT(W->entry), "activate",
		     GTK_SIGNAL_FUNC(LabeledEntry_callback),
		     W);

  pc = new PCLabeledEntry();
  pc->Create(hbox,"PC:", 6);
  pc->AssignParent(this);

  gtk_signal_connect(GTK_OBJECT(pc->entry), "activate",
		     GTK_SIGNAL_FUNC(LabeledEntry_callback),
		     pc);

  cpu_cycles = new CyclesLabeledEntry();
  cpu_cycles->Create(hbox,"Cycles:", 18,false);

  TimeLabeledEntry *tle = new TimeLabeledEntry();
  time = tle;
  time->Create(hbox,"Time:", 22,false);

  /* create popupmenu */
  popup_menu = tle->build_menu();
  gtk_signal_connect(GTK_OBJECT(time->entry),
		     "button_press_event",
		     (GtkSignalFunc) do_popup,
		     tle);

  created=1;
  
}

/*  NewProcessor
 *
 */

void StatusBar_Window::NewProcessor(GUI_Processor *_gp)
{


  if(_gp == 0)
    return;

  gp = _gp;

  gp->status_bar = this;

  if(gp->cpu) {

    pic_processor *pic = dynamic_cast<pic_processor *>(gp->cpu);

    RegisterLabeledEntry *rle = dynamic_cast<RegisterLabeledEntry *>(status);
    if(pic && rle) {
      rle->AssignRegister(pic->status);
      status->NewLabel((char *) rle->reg->name().c_str());
    }

    rle = dynamic_cast<RegisterLabeledEntry *>(W);
    if(pic && rle) {
      rle->AssignRegister(pic->W);
      W->NewLabel((char *) rle->reg->name().c_str());
    }

    PCLabeledEntry *pcle = dynamic_cast<PCLabeledEntry *>(pc);
    if(pcle)
      pcle->AssignPma(gp->cpu->pma);
  }

  /* Now create a cross-reference link that the simulator can use to
   * send information back to the gui
   */

  if(gp->cpu && gp->cpu->pc) {
    StatusBarXREF *cross_reference;

    cross_reference = new StatusBarXREF();
    cross_reference->parent_window_type =   WT_status_bar;
    cross_reference->parent_window = (gpointer) this;
    cross_reference->data = (gpointer) this;
  
    gp->cpu->pc->add_xref((gpointer) cross_reference);

  }

  Update();

}

StatusBar_Window::StatusBar_Window(void)
{
  gp = 0;

  popup_menu = 0;
  
  status = 0;
  W = 0;
  pc = 0;
  cpu_cycles = 0;
  time = 0;
  
}

#endif // HAVE_GUI
