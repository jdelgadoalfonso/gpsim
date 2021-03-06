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

#include <assert.h>

#include "../src/interface.h"
#include "../src/cmd_gpsim.h"

#include "gui.h"
#include "gui_register.h"
#include "gui_regwin.h"
#include "gui_watch.h"

#define BPCOL 0
#define NAMECOL 2
#define MASKCOL 4
#define DECIMALCOL 5
#define HEXCOL 6
#define ASCIICOL 7
#define MSBCOL 8
#define LSBCOL 23
#define LASTCOL LSBCOL

static char *watch_titles[]={"bp?", "type", "name","address","mask","dec","hex","ascii","b15","b14","b13","b12","b11","b10","b9","b8","b7","b6","b5","b4","b3","b2","b1","b0"};

#define COLUMNS sizeof(watch_titles)/sizeof(char*)

struct _coldata{
    GtkWidget *clist;
    int column;
    int visible; // Loaded on startup
    Watch_Window *ww;
} coldata[COLUMNS];
static void select_columns(Watch_Window *ww, GtkWidget *clist);


typedef enum {
    MENU_REMOVE,
    MENU_SET_VALUE,
    MENU_BREAK_CLEAR,
    MENU_BREAK_READ,
    MENU_BREAK_WRITE,
    MENU_BREAK_READ_VALUE,
    MENU_BREAK_WRITE_VALUE,
    MENU_COLUMNS,
} menu_id;


typedef struct _menu_item {
    char *name;
    menu_id id;
    GtkWidget *item;
} menu_item;

static menu_item menu_items[] = {
    {"Remove watch", MENU_REMOVE},
    {"Set value...", MENU_SET_VALUE},
    {"Clear breakpoints", MENU_BREAK_CLEAR},
    {"Set break on read", MENU_BREAK_READ},
    {"Set break on write", MENU_BREAK_WRITE},
    {"Set break on read value...", MENU_BREAK_READ_VALUE},
    {"Set break on write value...", MENU_BREAK_WRITE_VALUE},
    {"Columns...", MENU_COLUMNS},
};

// Used only in popup menus
Watch_Window *popup_ww;

//========================================================================

class WatchWindowXREF : public CrossReferenceToGUI
{
public:

  void Update(int new_value)
  {

    Watch_Window *ww  = (Watch_Window *) (parent_window);

    ww->Update();

  }
};

//========================================================================

void Watch_Window::ClearWatch(WatchEntry *entry)
{
  gtk_clist_remove(GTK_CLIST(watch_clist),current_row);
  watches=g_list_remove(watches,entry);
  entry->Clear_xref();
  free(entry);
}

void Watch_Window::UpdateMenus(void)
{
  GtkWidget *item;
  WatchEntry *entry;

  unsigned int i;

  for (i=0; i < (sizeof(menu_items)/sizeof(menu_items[0])) ; i++) {
    item=menu_items[i].item;
    if(menu_items[i].id!=MENU_COLUMNS) {

      entry = (WatchEntry*) gtk_clist_get_row_data(GTK_CLIST(watch_clist),current_row);
      if(menu_items[i].id!=MENU_COLUMNS && 
          (entry==0 ||
          (entry->type==REGISTER_EEPROM && menu_items[i].id==MENU_BREAK_CLEAR)||
          (entry->type==REGISTER_EEPROM && menu_items[i].id==MENU_BREAK_READ)||
          (entry->type==REGISTER_EEPROM && menu_items[i].id==MENU_BREAK_WRITE)||
          (entry->type==REGISTER_EEPROM && menu_items[i].id==MENU_BREAK_READ_VALUE)||
          (entry->type==REGISTER_EEPROM && menu_items[i].id==MENU_BREAK_WRITE_VALUE)
          ))
        gtk_widget_set_sensitive (item, FALSE);
      else
        gtk_widget_set_sensitive (item, TRUE);
    }
  }
}

static void unselect_row(GtkCList *clist,
			 gint row,
			 gint column,
			 GdkEvent *event,
			 Watch_Window *ww)
{
  ww->UpdateMenus();
}

// called when user has selected a menu item
static void
popup_activated(GtkWidget *widget, gpointer data)
{
  menu_item *item;

  WatchEntry *entry;

  int value;

  if(widget==0 || data==0)
    {
      printf("Warning popup_activated(%p,%p)\n",widget,data);
      return;
    }
    
  item = (menu_item *)data;

  entry = (WatchEntry*) gtk_clist_get_row_data(GTK_CLIST(popup_ww->watch_clist),popup_ww->current_row);

  if(entry==0 && item->id!=MENU_COLUMNS)
    return;

  if(!entry || !entry->cpu)
    return;

  switch(item->id)
    {
    case MENU_REMOVE:
      popup_ww->ClearWatch(entry);
      //remove_entry(popup_ww,entry);
      break;
    case MENU_SET_VALUE:
      value = gui_get_value("value:");
      if(value<0)
	break; // Cancel
      entry->put_value(value);
      break;
    case MENU_BREAK_READ:
      bp.set_read_break(entry->cpu,entry->address);
      break;
    case MENU_BREAK_WRITE:
      bp.set_write_break(entry->cpu,entry->address);
      break;
    case MENU_BREAK_READ_VALUE:
      value = gui_get_value("value to read for breakpoint:");
      if(value<0)
	break; // Cancel
      bp.set_read_value_break(entry->cpu,entry->address,value);
      break;
    case MENU_BREAK_WRITE_VALUE:
      value = gui_get_value("value to write for breakpoint:");
      if(value<0)
	break; // Cancel
      bp.set_write_value_break(entry->cpu,entry->address,value);
      break;
    case MENU_BREAK_CLEAR:
      bp.clear_all_register(entry->cpu,entry->address);
      break;
    case MENU_COLUMNS:
      select_columns(popup_ww, popup_ww->watch_clist);
      break;
    default:
      puts("Unhandled menuitem?");
      break;
    }
}

static void set_column(GtkCheckButton *button, struct _coldata *coldata)
{
    char str[256];
    if(button->toggle_button.active)
      gtk_clist_set_column_visibility(GTK_CLIST(coldata->clist),coldata->column,1);
    else
      gtk_clist_set_column_visibility(GTK_CLIST(coldata->clist),coldata->column,0);
    sprintf(str,"show_column%d",coldata->column);
    config_set_variable(coldata->ww->name(),str,button->toggle_button.active);
}

static void select_columns(Watch_Window *ww, GtkWidget *clist)
{
    GtkWidget *dialog=0;
    GtkWidget *button;
    int i;

    dialog = gtk_dialog_new();

    gtk_container_set_border_width(GTK_CONTAINER(dialog),30);

    gtk_signal_connect_object(GTK_OBJECT(dialog),
			      "delete_event",GTK_SIGNAL_FUNC(gtk_widget_destroy),GTK_OBJECT(dialog));

    for(i=0;i<COLUMNS;i++)
    {
      button=gtk_check_button_new_with_label(watch_titles[i]);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button),coldata[i].visible);
      gtk_widget_show(button);
      gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),button,FALSE,FALSE,0);
      coldata[i].clist=clist;
      coldata[i].column=i;
      coldata[i].ww=ww;
      gtk_signal_connect(GTK_OBJECT(button),"clicked",
			   GTK_SIGNAL_FUNC(set_column),(gpointer)&coldata[i]);
    }

    button = gtk_button_new_with_label("OK");
    gtk_widget_show(button);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area), button,
		       FALSE,FALSE,10);
    gtk_signal_connect_object(GTK_OBJECT(button),"clicked",
			      GTK_SIGNAL_FUNC(gtk_widget_destroy),GTK_OBJECT(dialog));
    GTK_WIDGET_SET_FLAGS(button,GTK_CAN_DEFAULT);
    gtk_widget_grab_default(button);

    gtk_widget_show(dialog);
    
    return;
}

// helper function, called from do_popup
static GtkWidget *
build_menu(GtkWidget *sheet, Watch_Window *ww)
{
  GtkWidget *menu;
  GtkWidget *item;
  unsigned int i;


  if(sheet==0 || ww==0)
  {
      printf("Warning build_menu(%p,%p)\n",sheet,ww);
      return 0;
  }
    
  popup_ww = ww;
  
  menu=gtk_menu_new();

  item = gtk_tearoff_menu_item_new ();
  gtk_menu_append (GTK_MENU (menu), item);
  gtk_widget_show (item);
  
  for (i=0; i < (sizeof(menu_items)/sizeof(menu_items[0])) ; i++){
      menu_items[i].item=item=gtk_menu_item_new_with_label(menu_items[i].name);

      gtk_signal_connect(GTK_OBJECT(item),"activate",
			 (GtkSignalFunc) popup_activated,
			 &menu_items[i]);
      gtk_widget_show(item);
      gtk_menu_append(GTK_MENU(menu),item);
  }

  ww->UpdateMenus();
  
  return menu;
}

// button press handler
static gint
do_popup(GtkWidget *widget, GdkEventButton *event, Watch_Window *ww)
{

  GtkWidget *popup;

  if(widget==0 || event==0 || ww==0)
    {
      printf("Warning do_popup(%p,%p,%p)\n",widget,event,ww);
      return 0;
    }

  popup=ww->popup_menu;

  if( (event->type == GDK_BUTTON_PRESS) &&  (event->button == 3) )
    gtk_menu_popup(GTK_MENU(popup), 0, 0, 0, 0,
		   3, event->time);


  WatchEntry *entry;

  if(event->type==GDK_2BUTTON_PRESS &&
     event->button==1)
    {
      int column=ww->current_column;
      int row=ww->current_row;
	
      entry = (WatchEntry*) gtk_clist_get_row_data(GTK_CLIST(ww->watch_clist), row);
    
      if(column>=MSBCOL && column<=LSBCOL) {
	
        int value;  // , bit;
        	  
        // Toggle the bit.
        value = entry->get_value();

        value ^= (1<< (7-(column-MSBCOL)));
        entry->put_value(value);
      }
    }

  return 0;
}

static gint
key_press(GtkWidget *widget,
	  GdkEventKey *key, 
	  gpointer data)
{

  WatchEntry *entry;
  Watch_Window *ww = (Watch_Window *) data;

  if(!ww) return(FALSE);
  if(!ww->gp) return(FALSE);
  if(!ww->gp->cpu) return(FALSE);

  switch(key->keyval) {

  case GDK_Delete:
      entry = (WatchEntry*) gtk_clist_get_row_data(GTK_CLIST(ww->watch_clist),ww->current_row);
      if(entry!=0)
	  ww->ClearWatch(entry);
      break;
  }
  return TRUE;
}

static gint watch_list_row_selected(GtkCList *watchlist,gint row, gint column,GdkEvent *event, Watch_Window *ww)
{
  WatchEntry *entry;
  GUI_Processor *gp;
    
  ww->current_row=row;
  ww->current_column=column;

  gp=ww->gp;
    
  entry = (WatchEntry*) gtk_clist_get_row_data(GTK_CLIST(ww->watch_clist), row);

  if(!entry)
    return TRUE;
    
  if(entry->type==REGISTER_RAM)
    gp->regwin_ram->SelectRegister(entry->address);
  else if(entry->type==REGISTER_EEPROM)
    gp->regwin_eeprom->SelectRegister(entry->address);


  ww->UpdateMenus();
    
  return 0;
}

static void watch_click_column(GtkCList *clist, int column)
{
    static int last_col=-1;
    static GtkSortType last_sort_type=GTK_SORT_DESCENDING;
    
    if(last_col==-1)
	last_col=column;

    if(last_col == column)
    {
	if(last_sort_type==GTK_SORT_DESCENDING)
	{
	    gtk_clist_set_sort_type(clist,GTK_SORT_ASCENDING);
	    last_sort_type=GTK_SORT_ASCENDING;
	}
	else
	{
	    gtk_clist_set_sort_type(clist,GTK_SORT_DESCENDING);
	    last_sort_type=GTK_SORT_DESCENDING;
	}
    }

    gtk_clist_set_sort_column(clist,column);
    gtk_clist_sort(clist);
    last_col=column;
}

static int delete_event(GtkWidget *widget,
			GdkEvent  *event,
                        Watch_Window *ww)
{
  ww->ChangeView(VIEW_HIDE);
  return TRUE;
}

//static void update(Watch_Window *ww, struct watch_entry *entry, int new_value)
void Watch_Window::UpdateWatch(WatchEntry *entry)
{
  char str[80];
  int i;

  int row;

  row=gtk_clist_find_row_from_data(GTK_CLIST(watch_clist),entry);
  if(row==-1)
    return;

  RegisterValue rvNewValue;
  int new_value;
  RegisterValue rvMaskedNewValue;
  unsigned int uBitmask;
  unsigned int uBitmaskForMaskedValue = entry->cpu->register_mask();
  rvNewValue = entry->getRV();
  new_value = rvNewValue;
  if(entry->pRegSymbol) {
    rvMaskedNewValue = *(entry->pRegSymbol);
    uBitmask = entry->pRegSymbol->getBitmask();
  }
  else {
    rvMaskedNewValue = entry->getRV();
    uBitmask = entry->cpu->register_mask();
  }
  if(rvNewValue.init & uBitmask) {
    strcpy(str, "?");
  }
  else {
    sprintf(str,"%d", rvNewValue);
  }
  gtk_clist_set_text(GTK_CLIST(watch_clist), row, DECIMALCOL, str);

  rvMaskedNewValue.toString(str, 80);
  gtk_clist_set_text(GTK_CLIST(watch_clist), row, HEXCOL, str);

  strcpy(str, GetUserInterface().FormatValue(
    uBitmask, entry->cpu->register_mask(), IUserInterface::eHex));
  gtk_clist_set_text(GTK_CLIST(watch_clist), row, MASKCOL, str);

  if(new_value>=32 && new_value<127)
    sprintf(str,"%c",new_value);
  else
    str[0]=0;
  gtk_clist_set_text(GTK_CLIST(watch_clist), row, ASCIICOL, str);
  int iCol;
  char sBit[2];
  char sBits[25];
  sBit[1] = 0;
  rvNewValue.toBitStr(sBits, 25, entry->cpu->register_mask(), 
			       NULL);
  for(i=15, iCol = LSBCOL;
      iCol >= MSBCOL;
      i--, iCol--) {
    sBit[0] = sBits[i];
    gtk_clist_set_text(GTK_CLIST(watch_clist), row, iCol, sBit);
  }

  if(entry->hasBreak())
    gtk_clist_set_text(GTK_CLIST(watch_clist), row, BPCOL, "yes");
  else
    gtk_clist_set_text(GTK_CLIST(watch_clist), row, BPCOL, "no");
}

//------------------------------------------------------------------------
// Update
//
//

void Watch_Window::Update(void)
{
  GList *iter;
  WatchEntry *entry;
  int clist_frozen=0;

  iter=watches;

  while(iter) {
   
    entry=(WatchEntry*)iter->data;

    RegisterValue value = entry->getRV();
	
    if(entry->get_shadow().data != value.data) {
      // The register has changed since the last update.

      if(clist_frozen==0) {
        gtk_clist_freeze(GTK_CLIST(watch_clist));
        clist_frozen=1;
      }

      // Update value in clist
      entry->put_shadow(value);
      UpdateWatch(entry);
    }
    iter=iter->next;
  }
  if(clist_frozen)
    gtk_clist_thaw(GTK_CLIST(watch_clist));
}
//------------------------------------------------------------------------
void Watch_Window::Add( REGISTER_TYPE type, GUIRegister *reg)
{
  if(!gp || !gp->cpu || !reg || !reg->bIsValid())
    return;
  Register *cpu_reg = reg->get_register();
  register_symbol * pRegSym = get_symbol_table().findRegisterSymbol(
    cpu_reg->address);
  Add(type, reg, pRegSym);
}

void Watch_Window::Add( REGISTER_TYPE type, GUIRegister *reg, register_symbol * pRegSym)
{
  char name[50], addressstring[50], typestring[30];
  char *entry[COLUMNS]={"",typestring,name, addressstring, "", "","","","","","","","","",""};
  int row;
  WatchWindowXREF *cross_reference;

  WatchEntry *watch_entry;
    
  if(!gp || !gp->cpu || !reg || !reg->bIsValid())
    return;

  if(!enabled)
    Build();


  Register *cpu_reg;

  if(pRegSym == 0) {
    cpu_reg = reg->get_register();
    strncpy(name,cpu_reg->name().c_str(),sizeof(name));
  }
  else {
    cpu_reg = pRegSym->getReg();
    strncpy(name,pRegSym->name().c_str(),sizeof(name));
  }
  unsigned int uAddrMask = 0;
  unsigned int uLastAddr = gp->cpu->register_memory_size() - 1;
  while(uLastAddr) {
    uLastAddr>>=4;
    uAddrMask<<=4;
    uAddrMask |= 0xf;
  }
  strcpy(addressstring, GetUserInterface().FormatProgramAddress(
    cpu_reg->address, uAddrMask, IUserInterface::eHex));
  strncpy(typestring,type==REGISTER_RAM?"RAM":"EEPROM",30);

  gtk_clist_freeze(GTK_CLIST(watch_clist));
  row=gtk_clist_append(GTK_CLIST(watch_clist), entry);

  watch_entry = new WatchEntry();
  watch_entry->address=reg->address;
  watch_entry->pRegSymbol = pRegSym;
  watch_entry->cpu = gp->cpu;

  watch_entry->type=type;

  watch_entry->rma = reg->rma;

  gtk_clist_set_row_data(GTK_CLIST(watch_clist), row, (gpointer)watch_entry);
    
  watches = g_list_append(watches, (gpointer)watch_entry);

  UpdateWatch(watch_entry);

  cross_reference = new WatchWindowXREF();
  cross_reference->parent_window_type = WT_watch_window;
  cross_reference->parent_window = (gpointer) this;
  cross_reference->data = (gpointer) watch_entry;

  watch_entry->Assign_xref(cross_reference);
  gtk_clist_thaw(GTK_CLIST(watch_clist));

  UpdateMenus();

}

//---
// Add - given a symbol, verify that it is a register symbol. If it
// is then extract the register and use it's address to get the 
// GUI representation of the register.

void Watch_Window::Add( Value *regSym)
{
  if(regSym && gp) {

    register_symbol *rs = dynamic_cast<register_symbol *>(regSym);

    if(rs != 0) {
      Register *reg = rs->getReg();
      
      if(reg) {
        GUIRegister *greg = gp->m_pGUIRegisters->Get(reg->address);
        Add(REGISTER_RAM, greg, rs);
      }
    }
  }

}
//------------------------------------------------------------------------
// ClearWatches
//
//

void Watch_Window::ClearWatches(void)
{
  GList *iter;
  WatchEntry *entry;
  int row;

  iter=watches;

  while(iter) {

    entry=(WatchEntry*)iter->data;
    row=gtk_clist_find_row_from_data(GTK_CLIST(watch_clist),entry);
    gtk_clist_remove(GTK_CLIST(watch_clist),row);
    entry->Clear_xref();
    free(entry);
    iter=iter->next;
  }

  while( (watches=g_list_remove_link(watches,watches))!=0)
    ;
}

//------------------------------------------------------------------------
// Build
//
//

void Watch_Window::Build(void)
{
  if(bIsBuilt)
    return;

  GtkWidget *vbox;
  GtkWidget *scrolled_window;

  int i;

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  gtk_window_set_title(GTK_WINDOW(window), "Watch Viewer");

  gtk_window_set_default_size(GTK_WINDOW(window), width,height);
  gtk_widget_set_uposition(GTK_WIDGET(window),x,y);
  gtk_window_set_wmclass(GTK_WINDOW(window),name(),"Gpsim");
  
  gtk_signal_connect (GTK_OBJECT (window), "delete_event",
		      GTK_SIGNAL_FUNC(delete_event), (gpointer)this);
  gtk_signal_connect_after(GTK_OBJECT(window), "configure_event",
			   GTK_SIGNAL_FUNC(gui_object_configure_event),this);
  
  watch_clist = gtk_clist_new_with_titles(COLUMNS,watch_titles);
  gtk_widget_show(watch_clist);

  for(i=0;i<LASTCOL;i++) {
    gtk_clist_set_column_auto_resize(GTK_CLIST(watch_clist),i,TRUE);
    gtk_clist_set_column_visibility(GTK_CLIST(watch_clist),i,coldata[i].visible);
  }
  
  gtk_clist_set_selection_mode (GTK_CLIST(watch_clist), GTK_SELECTION_BROWSE);

  gtk_signal_connect(GTK_OBJECT(watch_clist),"click_column",
		     (GtkSignalFunc)watch_click_column,0);
  gtk_signal_connect(GTK_OBJECT(watch_clist),"select_row",
		     (GtkSignalFunc)watch_list_row_selected,this);
  gtk_signal_connect(GTK_OBJECT(watch_clist),"unselect_row",
		     (GtkSignalFunc)unselect_row,this);
  
  gtk_signal_connect(GTK_OBJECT(watch_clist),
		     "button_press_event",
		     (GtkSignalFunc) do_popup,
		     this);
  gtk_signal_connect(GTK_OBJECT(window),"key_press_event",
		     (GtkSignalFunc) key_press,
		     (gpointer) this);

  scrolled_window=gtk_scrolled_window_new(0, 0);
  gtk_widget_show(scrolled_window);

  vbox = gtk_vbox_new(FALSE,1);
  gtk_widget_show(vbox);
  
  gtk_container_add(GTK_CONTAINER(scrolled_window), watch_clist);
  
  gtk_container_add(GTK_CONTAINER(window),vbox);

  gtk_box_pack_start_defaults(GTK_BOX(vbox),scrolled_window);
  
  popup_menu=build_menu(window,this);
  
  gtk_widget_show (window);
  
  
  enabled=1;

  bIsBuilt = true;

  UpdateMenuItem();

}


Watch_Window::Watch_Window(GUI_Processor *_gp)
{
  int i;
    
#define MAXROWS  (MAX_REGISTERS/REGISTERS_PER_ROW)
#define MAXCOLS  (REGISTERS_PER_ROW+1)

  menu = "<main>/Windows/Watch";

  set_name("watch_viewer");
  wc = WC_data;
  wt = WT_watch_window;
  window = 0;

  watches=0;
  current_row=0;

  gp = _gp;

  get_config();
  int iRegisterSize = _gp->cpu == NULL ? 1 :_gp->cpu->register_size();
  int iLSCol = iRegisterSize == 1 ? (MSBCOL + 8) : MSBCOL;
  for(i=0;i<COLUMNS;i++) {
    int bVisible;
    char str[128];
    sprintf(str,"show_column%d",i);
    if(i < MSBCOL) {
      bVisible = 1;
    }
    else {
      bVisible = i >= iLSCol;
    }
    coldata[i].visible=bVisible; // default
    config_get_variable(name(),str,&coldata[i].visible);
  }

  if(enabled)
    Build();

}

#endif // HAVE_GUI
