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
//#include <gtkextra/gtksheetentry.h>

#include "../xpms/pc.xpm"

/*
unsigned int gpsim_get_opcode(unsigned int processor_id, unsigned int address);
char *gpsim_get_opcode_name(unsigned int processor_id, unsigned int address);
unsigned int gpsim_get_program_memory_size(unsigned int processor_id);
*/

#include "gui.h"

#include <assert.h>

#define PROGRAM_MEMORY_WINDOW_COLUMNS 4   //yuk
#define DEFAULT_ROWS  256

#define PROFILE_COLUMN  0
#define ADDRESS_COLUMN  1
#define OPCODE_COLUMN   2
#define MNEMONIC_COLUMN 3

typedef enum {
    MENU_BREAK_CLEAR,
    MENU_BREAK_READ,
    MENU_BREAK_WRITE,
    MENU_BREAK_EXECUTE,
    MENU_ADD_WATCH,
    MENU_ASCII_1BYTE,
    MENU_ASCII_2BYTELSB,
    MENU_ASCII_2BYTEMSB,
} menu_id;




typedef struct _menu_item {
    char *name;
    menu_id id;
} menu_item;

static menu_item menu_items[] = {
    {"Clear breakpoints", MENU_BREAK_CLEAR},
    {"Set break on read", MENU_BREAK_READ},
    {"Set break on write", MENU_BREAK_WRITE},
    {"Set break on execute", MENU_BREAK_EXECUTE},
    {"Add watch", MENU_ADD_WATCH},
};

static menu_item submenu_items[] = {
    {"One byte per cell",             MENU_ASCII_1BYTE},
    {"Two bytes per cell, MSB first", MENU_ASCII_2BYTEMSB},
    {"Two bytes per cell, LSB first", MENU_ASCII_2BYTELSB},
};

// Used only in popup menus
SourceBrowserOpcode_Window *popup_sbow;

static char profile_buffer[128];
static char address_buffer[128];
static char opcode_buffer[128];
static char mnemonic_buffer[128];
static char *row_text[PROGRAM_MEMORY_WINDOW_COLUMNS]={
    profile_buffer,address_buffer,opcode_buffer,mnemonic_buffer
};

static GtkStyle *row_default_style;


// update ascii column in sheet
static void update_ascii( SourceBrowserOpcode_Window *sbow, gint row)
{
    gint i;
    gchar name[45];
    unsigned char byte;

    if(sbow == NULL || row<0 || row > GTK_SHEET(sbow->sheet)->maxrow)
    {
	printf("Warning update_ascii(%x,%x)\n",(unsigned int)sbow,row);
	return;
    }

    if(row<0 || row>GTK_SHEET(sbow->sheet)->maxrow)
	return;

    switch(sbow->ascii_mode)
    {
    case 0:
	for(i=0; i<16; i++)
	{
	    byte = sbow->memory[row*16 + i]&0xff;

	    name[i] = byte;
	    
	    if( (name[i] < ' ') || (name[i]>'z'))
		name[i] = '.';
	}
	name[i] = 0;
	break;
    case 1: // two bytes, MSB first
	for(i=0; i<32; i++)
	{
	    if(i%2)
		byte = sbow->memory[row*16 + i/2]&0xff;
	    else
		byte = (sbow->memory[row*16 + i/2]&0xff00) >>8;

	    name[i] = byte;
	    
	    if( (name[i] < ' ') || (name[i]>'z'))
		name[i] = '.';
	}
	name[i] = 0;
	break;
    case 2: // two bytes, LSB first
	for(i=0; i<32; i++)
	{

	    if(i%2)
		byte = (sbow->memory[row*16 + i/2]&0xff00) >>8;
	    else
		byte = sbow->memory[row*16 + i/2]&0xff;

	    name[i] = byte;
	    
	    if( (name[i] < ' ') || (name[i]>'z'))
		name[i] = '.';
	}
	name[i] = 0;
	break;
    }
    gtk_sheet_set_cell(GTK_SHEET(sbow->sheet), row,REGISTERS_PER_ROW, GTK_JUSTIFY_RIGHT,name);

}

// called when user has selected a menu item
static void
popup_activated(GtkWidget *widget, gpointer data)
{
    GtkSheet *sheet;

    menu_item *item;
    int i,j;
    unsigned int pic_id;
    GtkSheetRange range;
    unsigned int address;
    int value;
    int pm_size;
    gint char_width;

    if(widget==NULL || data==NULL)
    {
	printf("Warning popup_activated(%x,%x)\n",(unsigned int)widget,(unsigned int)data);
	return;
    }
    
    item = (menu_item *)data;
    sheet=GTK_SHEET(popup_sbow->sheet);
    range = sheet->range;
    pic_id = ((GUI_Object*)popup_sbow)->gp->pic_id;
    
    pm_size = gpsim_get_program_memory_size(popup_sbow->sbw.gui_obj.gp->pic_id);
    char_width = gdk_string_width (normal_style->font,"9");
    
    switch(item->id)
    {
    case MENU_BREAK_READ:
	for(j=range.row0;j<=range.rowi;j++)
	    for(i=range.col0;i<=range.coli;i++)
	    {
		address=j*16+i;
		gpsim_set_read_break_at_address(popup_sbow->sbw.gui_obj.gp->pic_id, address);
	    }
	break;
    case MENU_BREAK_WRITE:
	for(j=range.row0;j<=range.rowi;j++)
	    for(i=range.col0;i<=range.coli;i++)
	    {
		address=j*16+i;
		gpsim_set_write_break_at_address(popup_sbow->sbw.gui_obj.gp->pic_id, address);
	    }
	break;
    case MENU_BREAK_EXECUTE:
	for(j=range.row0;j<=range.rowi;j++)
	    for(i=range.col0;i<=range.coli;i++)
	    {
		address=j*16+i;
		gpsim_set_execute_break_at_address(popup_sbow->sbw.gui_obj.gp->pic_id, address);
	    }
	break;
    case MENU_BREAK_CLEAR:
	for(j=range.row0;j<=range.rowi;j++)
	    for(i=range.col0;i<=range.coli;i++)
	    {
		address=j*16+i;
		gpsim_clear_breakpoints_at_address(popup_sbow->sbw.gui_obj.gp->pic_id, address);
	    }
	break;
    case MENU_ADD_WATCH:
	puts("not implemented");
	for(j=range.row0;j<=range.rowi;j++)
	    for(i=range.col0;i<=range.coli;i++)
	    {
		address=j*16+i;
//		WatchWindow_add(popup_sbow->gui_obj.gp->watch_window,pic_id, popup_sbow->type, address);
	    }
	break;
    case MENU_ASCII_1BYTE:
	popup_sbow->ascii_mode=0;
	config_set_variable(popup_sbow->sbw.gui_obj.name,"ascii_mode",popup_sbow->ascii_mode);
	gtk_sheet_set_column_width (GTK_SHEET(popup_sbow->sheet), 16, 16*char_width + 6);
	for(i=0;i<pm_size/16;i++)
	    update_ascii(popup_sbow,i);
	break;
    case MENU_ASCII_2BYTEMSB:
	popup_sbow->ascii_mode=1;
	config_set_variable(popup_sbow->sbw.gui_obj.name,"ascii_mode",popup_sbow->ascii_mode);
	gtk_sheet_set_column_width (GTK_SHEET(popup_sbow->sheet), 16, 32*char_width + 6);
	for(i=0;i<pm_size/16;i++)
	    update_ascii(popup_sbow,i);
	break;
    case MENU_ASCII_2BYTELSB:
	popup_sbow->ascii_mode=2;
	config_set_variable(popup_sbow->sbw.gui_obj.name,"ascii_mode",popup_sbow->ascii_mode);
	gtk_sheet_set_column_width (GTK_SHEET(popup_sbow->sheet), 16, 32*char_width + 6);
	for(i=0;i<pm_size/16;i++)
	    update_ascii(popup_sbow,i);
	break;
    default:
	puts("Unhandled menuitem?");
	break;
    }
}


static GtkWidget *
build_menu(SourceBrowserOpcode_Window *sbow)
{
    GtkWidget *menu;
    GtkWidget *item;

    GSList *group=NULL;
    
    GtkWidget *submenu;
    //  GtkAccelGroup *accel_group;
  int i;

  if(sbow==NULL)
  {
      printf("Warning build_menu(%x)\n",(unsigned int)sbow);
      return NULL;
  }

  popup_sbow=sbow;
  
  menu=gtk_menu_new();

/*  accel_group = gtk_accel_group_new ();
  gtk_accel_group_attach (accel_group, GTK_OBJECT (sbow->gui_obj.window));
  
  gtk_menu_set_accel_group (GTK_MENU (menu), accel_group);
  */

  item = gtk_tearoff_menu_item_new ();
  gtk_menu_append (GTK_MENU (menu), item);
  gtk_widget_show (item);
  
  
  for (i=0; i < (sizeof(menu_items)/sizeof(menu_items[0])) ; i++){
      item=gtk_menu_item_new_with_label(menu_items[i].name);

      gtk_signal_connect(GTK_OBJECT(item),"activate",
			 (GtkSignalFunc) popup_activated,
			 &menu_items[i]);
      GTK_WIDGET_SET_FLAGS (item, GTK_SENSITIVE | GTK_CAN_FOCUS);

      if(menu_items[i].id==MENU_ADD_WATCH)
      {
	  GTK_WIDGET_UNSET_FLAGS (item,
				  GTK_SENSITIVE | GTK_CAN_FOCUS);
      }
      gtk_widget_show(item);
      gtk_menu_append(GTK_MENU(menu),item);
  }
  
    submenu=gtk_menu_new();
    for (i=0; i < (sizeof(submenu_items)/sizeof(submenu_items[0])) ; i++){
	item=gtk_radio_menu_item_new_with_label(group, submenu_items[i].name);

	group=gtk_radio_menu_item_group(GTK_RADIO_MENU_ITEM(item));
	gtk_signal_connect(GTK_OBJECT(item),"activate",
			   (GtkSignalFunc) popup_activated,
			   &submenu_items[i]);
	
	GTK_WIDGET_SET_FLAGS (item, GTK_SENSITIVE | GTK_CAN_FOCUS);

/*	if(submenu_items[i].id==MENU_STOP)
	{
	    GTK_WIDGET_UNSET_FLAGS (item,
				    GTK_SENSITIVE | GTK_CAN_FOCUS);
	}*/
      
	gtk_widget_show(item);
	
	if(i==sbow->ascii_mode)
	    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item),1);
	
	gtk_menu_append(GTK_MENU(submenu),item);
    }
    item = gtk_menu_item_new_with_label ("ASCII mode");
    gtk_menu_append (GTK_MENU (menu), item);
    gtk_widget_show (item);
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), submenu);

  
  return menu;
}

// button press handler
static gint
do_popup(GtkWidget *widget, GdkEventButton *event, SourceBrowserOpcode_Window *sbow)
{

    GtkWidget *popup;
//	GdkModifierType mods;
    GtkSheet *sheet;

    popup=sbow->popup_menu;
    
  if(widget==NULL || event==NULL || sbow==NULL)
  {
      printf("Warning do_popup(%x,%x,%x)\n",(unsigned int)widget,(unsigned int)event,(unsigned int)sbow);
      return 0;
  }
  
    sheet=GTK_SHEET(widget);

    if( (event->type == GDK_BUTTON_PRESS) &&  (event->button == 3) )
    {

/*	if (event->window == sheet->column_title_window )
	    //printf("popup column window\n");
	    return TRUE;
	else if (event->window == sheet->row_title_window )
	    //printf("popup  window\n");
	    return TRUE;
	else*/
	popup_sbow = sbow;
  
	gtk_menu_popup(GTK_MENU(popup), NULL, NULL, NULL, NULL,
			   3, event->time);
    }
    return FALSE;
}


static void filter(char *clean, char *dirty, int max)
{

    int i=0,j;

    do {


	if(*dirty == '\t')
	    for(j=0,dirty++; j<8 && i%8; j++,i++)
		*clean++ = ' ';
	else if (*dirty <' ')
	    dirty++;
	else
	    *clean++ = *dirty++;



  }while(*dirty && ++i < max);

  *clean = 0;

}

static gint
button_press(GtkWidget *widget,
	     GdkEvent  *event, 
	     gpointer data)
{
    SourceBrowserOpcode_Window *sbow = (SourceBrowserOpcode_Window*)data;
    int break_row;

    if ((event->type == GDK_2BUTTON_PRESS) &&
	(event->button.button == 1))

    {
	break_row =  GTK_CLIST (sbow->clist)->focus_row;

	if(!sbow->processor)
	    return TRUE;      // no code is in this window
	//sbow->sbw.gui_obj.gp->p->toggle_break_at_address(break_row);
	gpsim_toggle_break_at_address(sbow->sbw.gui_obj.gp->pic_id, break_row);
	return TRUE;

    }
    return FALSE;
}

static void update_styles(SourceBrowserOpcode_Window *sbow, int address)
{
    int pc;
    GtkSheetRange range;
    int row=address/16;
    int column=address%16;
    
    range.row0=row;
    range.rowi=row;
    range.col0=column;
    range.coli=column;
    
/*    pc=gpsim_get_pc_value(sbow->sbw.gui_obj.gp->pic_id);
    // Set styles/indicators
    if(address==pc)
    {
	SourceBrowserOpcode_set_pc(sbow, pc);
    }
    else
    {*/
    if(gpsim_address_has_breakpoint(sbow->sbw.gui_obj.gp->pic_id,  address))
    {
	gtk_clist_set_row_style (GTK_CLIST (sbow->clist), address, breakpoint_line_number_style);
    }
    else
    {
	gtk_clist_set_row_style (GTK_CLIST (sbow->clist), address, row_default_style);
    }

//    }
    
    if(gpsim_address_has_breakpoint(sbow->sbw.gui_obj.gp->pic_id, address))
	gtk_sheet_range_set_background(GTK_SHEET(sbow->sheet), &range, &breakpoint_color);
    else if(gpsim_address_has_changed(sbow->sbw.gui_obj.gp->pic_id, address))
	gtk_sheet_range_set_background(GTK_SHEET(sbow->sheet), &range, &pm_has_changed_color);
    else
        gtk_sheet_range_set_background(GTK_SHEET(sbow->sheet), &range, &normal_pm_bg_color);

}

static void update_label(SourceBrowserOpcode_Window *sbow, int address)
{
    char labeltext[128];
    char entrytext[128];
    GtkEntry *sheet_entry;
    unsigned int oc;

    if(address<0)
    {
	strcpy(labeltext,"ASCII");
    }
    else
    {
	oc=gpsim_get_opcode(sbow->sbw.gui_obj.gp->pic_id  ,address);
	
	filter(labeltext,gpsim_get_opcode_name( sbow->sbw.gui_obj.gp->pic_id, address,entrytext),sizeof(labeltext));
	sprintf(entrytext, "0x%04X", oc);
    }
    
    sheet_entry = GTK_ENTRY(gtk_sheet_get_entry(GTK_SHEET(sbow->sheet)));
    gtk_label_set(GTK_LABEL(sbow->label), labeltext);
    gtk_entry_set_max_length(GTK_ENTRY(sbow->entry),
			     GTK_ENTRY(sheet_entry)->text_max_length);
    gtk_entry_set_text(GTK_ENTRY(sbow->entry), entrytext);

}

static void update_values(SourceBrowserOpcode_Window *sbow, int address)
{
    int row=address/16;
    int column=address%16;
    char buf[128];
    unsigned int oc;

    oc=gpsim_get_opcode(sbow->sbw.gui_obj.gp->pic_id  ,address);

    if(oc != sbow->memory[address])
    {
	sbow->memory[address]=oc;
	// Put new values, in case they changed
	sprintf (row_text[ADDRESS_COLUMN], "0x%04X", address);
	sprintf(row_text[OPCODE_COLUMN], "0x%04X", oc);
	filter(row_text[MNEMONIC_COLUMN], gpsim_get_opcode_name( sbow->sbw.gui_obj.gp->pic_id, address,buf), 128);
	gtk_clist_set_text (GTK_CLIST (sbow->clist), address, OPCODE_COLUMN, row_text[OPCODE_COLUMN]);
	gtk_clist_set_text (GTK_CLIST (sbow->clist), address, MNEMONIC_COLUMN, row_text[MNEMONIC_COLUMN]);

	gtk_sheet_set_cell(GTK_SHEET(sbow->sheet),
			   row,column,
			   GTK_JUSTIFY_RIGHT,row_text[OPCODE_COLUMN]+2);
    }
}

static void update(SourceBrowserOpcode_Window *sbow, int address)
{

    if(sbow->sbw.gui_obj.gp->pic_id==0)
	return;

    update_values(sbow,address);
    update_styles(sbow,address);
//    update_label(sbow,address);
}


static unsigned long get_number_in_string(char *number_string)
{
  unsigned long retval = 0;
  char *bad_position;
  int current_base = 16;
  
  if(number_string==NULL)
  {
      printf("Warning get_number_in_string(%x)\n",(unsigned int)number_string);
      errno = EINVAL;
      return -1;
  }


  errno = 0;

  retval = strtoul(number_string, &bad_position, current_base);

  if( strlen(bad_position) ) 
    errno = EINVAL;  /* string contains an invalid number */

  /*
  if(retval > 255)
    errno = ERANGE;
  */

  return(retval);
}


// when a new cell is selected, we write changes in
// previously selected cell to gpsim
static void
parse_numbers(GtkWidget *widget, int row, int col, SourceBrowserOpcode_Window *sbow)
{
  GtkSheet *sheet;
  gchar *text;
  int justification,n=0;

  GUI_Processor *gp;
  
  sheet=GTK_SHEET(widget);
  
  if(widget==NULL ||
     row>sheet->maxrow || row<0 ||
     col>sheet->maxcol || col<0 || sbow==NULL)
  {
      printf("Warning parse_numbers(%x,%x,%x,%x)\n",(unsigned int)widget,row,col,(unsigned int)sbow);
      return;
  }

  gp = ((GUI_Object*)sbow)->gp;
  
  justification=GTK_JUSTIFY_RIGHT;

  if(col < REGISTERS_PER_ROW)
    {

      int reg = row*16+col;

      text = gtk_entry_get_text(GTK_ENTRY(sheet->sheet_entry));

      errno = 0;
      if(strlen(text)>0)
	n = get_number_in_string(text);
      else
	errno = ERANGE;

      if(errno != 0)
      {
	  n = gpsim_get_opcode(sbow->sbw.gui_obj.gp->pic_id,reg);
	  sbow->memory[reg] = -1;
      }

      if(n != sbow->memory[reg])
      {
	  printf("Writing new value, new %d, last %d\n",n,sbow->memory[reg]);
	  sbow->memory[reg]=n;
	  gpsim_put_opcode(gp->pic_id, reg, n);
	  update_ascii(sbow,row);
      }
    }
  else
      ; // ignore user changes in ascii column for right now
}

/* when the entry above the sheet is changed (typed a digit), we
   copy it to the cell entry */
static void
show_sheet_entry(GtkWidget *widget, SourceBrowserOpcode_Window *sbow)
{
 char *text;
 GtkSheet *sheet;
 GtkEntry *sheet_entry;

 int row,col;
 
 if(widget==NULL|| sbow==NULL)
  {
      printf("Warning show_sheet_entry(%x,%x)\n",(unsigned int)widget,(unsigned int)sbow);
      return;
  }

 if(!GTK_WIDGET_HAS_FOCUS(widget)) return;
 
 sheet=GTK_SHEET(sbow->sheet);
 sheet_entry = GTK_ENTRY(gtk_sheet_get_entry(sheet));

 row=sheet->active_cell.row; col=sheet->active_cell.col;

 if((text=gtk_entry_get_text (GTK_ENTRY(sbow->entry))))
     gtk_entry_set_text(sheet_entry, text);

}

/* when we have new data in the entry above the sheet, we
 copy the data to the cells/registers

 this don't get called when it is clicked
 in, only when we hit return
 */
static void
activate_sheet_entry(GtkWidget *widget, SourceBrowserOpcode_Window *sbow)
{
  GtkSheet *sheet;
  gint row, col;

  if(widget==NULL|| sbow==NULL)
  {
      printf("Warning activate_sheet_entry(%x,%x)\n",(unsigned int)widget,(unsigned int)sbow);
      return;
  }

  sheet=GTK_SHEET(sbow->sheet);
  row=sheet->active_cell.row; col=sheet->active_cell.col;

  parse_numbers(GTK_WIDGET(sheet),sheet->active_cell.row,sheet->active_cell.col,sbow);
  
  update_label(sbow,row*16+col);
}

/*
 we get here when the entry in a cell is changed (typed a digit), we
 copy it to the entry above the sheet.
 */
static void
show_entry(GtkWidget *widget, SourceBrowserOpcode_Window *sbow)
{
 char *text; 
 GtkSheet *sheet;
 GtkWidget * sheet_entry;
  gint row, col;

  if(widget==NULL|| sbow==NULL)
  {
      printf("Warning show_entry(%x,%x)\n",(unsigned int)widget,(unsigned int)sbow);
      return;
  }
  
 if(!GTK_WIDGET_HAS_FOCUS(widget)) return;

 sheet=GTK_SHEET(sbow->sheet);
 sheet_entry = gtk_sheet_get_entry(sheet);

 row=sheet->active_cell.row; col=sheet->active_cell.col;
 
 if((text=gtk_entry_get_text (GTK_ENTRY(sheet_entry))))
     gtk_entry_set_text(GTK_ENTRY(sbow->entry), text);
}

/* when a cell is activated, we set the label and entry above the sheet
 */
static gint
activate_sheet_cell(GtkWidget *widget, gint row, gint column, SourceBrowserOpcode_Window *sbow)
{
    GtkSheet *sheet;
    GtkSheetCellAttr attributes;
  
    sheet=GTK_SHEET(sbow->sheet);

    if(widget==NULL || row>sheet->maxrow || row<0||
       column>sheet->maxcol || column<0 || sbow==NULL)
    {
	printf("Warning activate_sheet_cell(%x,%x,%x,%x)\n",(unsigned int)widget,row,column,(unsigned int)sbow);
	return 0;
    }

    if(column<16)
	update_label(sbow,row*16+column);
    else
	update_label(sbow,-1);
    
    gtk_sheet_get_attributes(sheet,sheet->active_cell.row,
			     sheet->active_cell.col, &attributes);
    gtk_entry_set_editable(GTK_ENTRY(sbow->entry), attributes.is_editable);
    gtk_sheet_range_set_justification(sheet, &sheet->range, GTK_JUSTIFY_RIGHT);


    return TRUE;
}


void SourceBrowserOpcode_select_address(SourceBrowserOpcode_Window *sbow, int address)
{
  if(! ((GUI_Object*)sbow)->enabled)
      return;
  
    gtk_clist_unselect_all(GTK_CLIST(sbow->clist));
    gtk_clist_select_row(GTK_CLIST(sbow->clist),address,0);
    if(GTK_VISIBILITY_FULL != gtk_clist_row_is_visible (GTK_CLIST (sbow->clist),
							address))
    {
	gtk_clist_moveto (GTK_CLIST (sbow->clist), address, 0, .5, 0.0);
    }
}

void SourceBrowserOpcode_update_line( SourceBrowserOpcode_Window *sbow, int address, int row)
{

    
    if(!sbow) return;

    if(! ((GUI_Object*)sbow)->enabled)
	return;

    assert(sbow->sbw.gui_obj.wt == WT_opcode_source_window);

    if(address >= 0)
    {
	update(sbow,address);

    }
}

void SourceBrowserOpcode_set_pc(SourceBrowserOpcode_Window *sbow, int address)
{
    gint last_address;
    int row=address/16;
    int col=address%16;
    GdkRectangle rect;

    if(! ((GUI_Object*)sbow)->enabled)
	return;

    last_address = sbow->current_address;
    sbow->current_address = address;

    if(address != last_address)
    {
	SourceBrowserOpcode_update_line( sbow, last_address, address);
//	gtk_clist_set_row_style (GTK_CLIST (sbow->clist), last_address, row_default_style);
	gtk_clist_set_row_style (GTK_CLIST (sbow->clist), address, current_line_number_style);

//	gtk_sheet_get_cell_area(GTK_SHEET(sbow->sheet),row,col,&rect);
//	gtk_sheet_move_child(GTK_SHEET(sbow->sheet),sbow->pcwidget,rect.x,rect.y);
    }
    
    if(GTK_VISIBILITY_FULL != gtk_clist_row_is_visible (GTK_CLIST (sbow->clist),
							sbow->current_address))
    {
	gtk_clist_moveto (GTK_CLIST (sbow->clist), sbow->current_address, 0, .5, 0.0);
    }
}

static void pc_changed(struct cross_reference_to_gui *xref, int new_address)
{
    SourceBrowserOpcode_Window *sbow;

    sbow = (SourceBrowserOpcode_Window*)(xref->parent_window);
    
    SourceBrowserOpcode_set_pc(sbow, new_address);

}

void SourceBrowserOpcode_new_program(SourceBrowserOpcode_Window *sbow, GUI_Processor *gp)
{
    char buf[128];
    int opcode;
    gint i;
    int pic_id;
    int pm_size;
    int pc;

    struct cross_reference_to_gui *cross_reference;

    if(sbow == NULL || gp == NULL)
	return;

    sbow->current_address=0;
    sbow->program=1;

    if(! ((GUI_Object*)sbow)->enabled)
	return;

    assert(sbow->sbw.gui_obj.wt==WT_opcode_source_window);

    pic_id = ((GUI_Object*)sbow)->gp->pic_id;
    sbow->sbw.gui_obj.gp = gp;
    gp->program_memory = (SourceBrowser_Window*)sbow;

    /* Now create a cross-reference link that the
     * simulator can use to send information back to the gui
     */
    cross_reference = (struct cross_reference_to_gui *) malloc(sizeof(struct cross_reference_to_gui));
    cross_reference->parent_window_type =   WT_opcode_source_window;
    cross_reference->parent_window = (gpointer) sbow;
    cross_reference->data = (gpointer) NULL;
    cross_reference->update = pc_changed;
    cross_reference->remove = NULL;
    gpsim_assign_pc_xref(pic_id, cross_reference);

    gtk_clist_freeze (GTK_CLIST (sbow->clist));

    // Clearing and appending is faster than changing
    gtk_clist_clear(GTK_CLIST(sbow->clist));

    pm_size = gpsim_get_program_memory_size(sbow->sbw.gui_obj.gp->pic_id);

    gtk_sheet_freeze(GTK_SHEET(sbow->sheet));


    for(i=0; i < pm_size; i++)
    {
	opcode = gpsim_get_opcode(sbow->sbw.gui_obj.gp->pic_id  ,i);
	sbow->memory[i]=opcode;
	sprintf (row_text[ADDRESS_COLUMN], "0x%04X", i);
	sprintf(row_text[OPCODE_COLUMN], "0x%04X", opcode);
	filter(row_text[MNEMONIC_COLUMN], gpsim_get_opcode_name( sbow->sbw.gui_obj.gp->pic_id, i,buf), 128);

	gtk_sheet_set_cell(GTK_SHEET(sbow->sheet),
			   i/16,
			   i%16,
			   GTK_JUSTIFY_RIGHT,row_text[OPCODE_COLUMN]+2);

	gtk_clist_append (GTK_CLIST (sbow->clist), row_text);
    }

    for(i=0;i<pm_size/16;i++)
	update_ascii(sbow,i);
    
    gtk_clist_set_row_style (GTK_CLIST (sbow->clist), 0, current_line_number_style);

    gtk_clist_thaw (GTK_CLIST (sbow->clist));

    gtk_sheet_thaw(GTK_SHEET(sbow->sheet));

    pc=gpsim_get_pc_value(sbow->sbw.gui_obj.gp->pic_id);
    SourceBrowserOpcode_set_pc(sbow, pc);
    update_label(sbow,pc);
}

void SourceBrowserOpcode_new_processor(SourceBrowserOpcode_Window *sbow, GUI_Processor *gp)
{

    gint i, pm_size,opcode;
    char buf[128];
    GtkSheetRange range;

    if(sbow == NULL || gp == NULL)
	return;

    sbow->processor=1;

    sbow->current_address=0;
    
    if(! ((GUI_Object*)sbow)->enabled)
	return;

    assert(sbow->sbw.gui_obj.wt==WT_opcode_source_window);

    pm_size = gpsim_get_program_memory_size(sbow->sbw.gui_obj.gp->pic_id);

    if(sbow->memory!=NULL)
	free(sbow->memory);
    sbow->memory=malloc(pm_size*sizeof(*sbow->memory));

    gtk_clist_freeze (GTK_CLIST (sbow->clist));
    gtk_sheet_freeze(GTK_SHEET(sbow->sheet));
    for(i=0;i<pm_size;i+=16)
    {
	char row_label[100];
	if(GTK_SHEET(sbow->sheet)->maxrow<i/16)
	{
	    gtk_sheet_add_row(GTK_SHEET(sbow->sheet),1);
	}

	sprintf(row_label,"%x0",i/16);
	gtk_sheet_row_button_add_label(GTK_SHEET(sbow->sheet), i/16, row_label);
	gtk_sheet_set_row_title(GTK_SHEET(sbow->sheet), i/16, row_label);
    }
    if(i/16 < GTK_SHEET(sbow->sheet)->maxrow)
    {
	//      printf(">>>>>>>%d %d %d\n",j,sheet->maxrow,sheet->maxrow+1-j);
	gtk_sheet_delete_rows(GTK_SHEET(sbow->sheet),i/16,GTK_SHEET(sbow->sheet)->maxrow-i/16);
    }

    range.row0=0;range.col0=0;
    range.rowi=GTK_SHEET(sbow->sheet)->maxrow;
    range.coli=GTK_SHEET(sbow->sheet)->maxcol;
    gtk_sheet_range_set_background(GTK_SHEET(sbow->sheet), &range, &normal_pm_bg_color);
    gtk_sheet_range_set_font(GTK_SHEET(sbow->sheet), &range, normal_style->font);


    // Clearing and appending is faster than changing
    gtk_clist_clear(GTK_CLIST(sbow->clist));
    
    for(i=0; i < pm_size; i++)
    {
	sbow->memory[i]=opcode=gpsim_get_opcode(sbow->sbw.gui_obj.gp->pic_id  ,i);
	sprintf (row_text[ADDRESS_COLUMN], "0x%04X", i);
	sprintf(row_text[OPCODE_COLUMN], "0x%04X", opcode);
	filter(row_text[MNEMONIC_COLUMN], gpsim_get_opcode_name( sbow->sbw.gui_obj.gp->pic_id, i,buf), 128);
	gtk_clist_append (GTK_CLIST (sbow->clist), row_text);
	gtk_sheet_set_cell(GTK_SHEET(sbow->sheet),
			   i/16,i%16,
			   GTK_JUSTIFY_RIGHT,row_text[OPCODE_COLUMN]+2);
    }

    gtk_clist_thaw (GTK_CLIST (sbow->clist));


    gtk_sheet_thaw(GTK_SHEET(sbow->sheet));

    range.row0=range.rowi=0;
    range.col0=range.coli=0;
    gtk_sheet_select_range(GTK_SHEET(sbow->sheet),&range);
    
    update_label(sbow,0);
    
//    SourceBrowserOpcode_new_program(sbow, gp);
}

void BuildSourceBrowserOpcodeWindow(SourceBrowserOpcode_Window *sbow)
{
    static GtkWidget *clist;
    GtkWidget *label;
    GtkWidget *entry;
    GtkWidget *vbox;
    GtkWidget *hbox;
  GtkWidget *scrolled_win;
  GtkRequisition request;
  
	gchar name[10];
	gint column_width,char_width;
  gint i;
  char address[16];

  int x,y,width,height;
  
static GdkPixmap *pixmap_pc;
static GtkStyle *style=NULL;
static GdkBitmap *mask;
  
  
  CreateSBW((SourceBrowser_Window*)sbow);


    gtk_window_set_title (GTK_WINDOW (sbow->sbw.gui_obj.window), "Program memory");




    sbow->notebook = gtk_notebook_new();
    gtk_widget_show(sbow->notebook);

    gtk_box_pack_start (GTK_BOX (sbow->sbw.vbox), sbow->notebook, TRUE, TRUE, 0);


  
  width=((GUI_Object*)sbow)->width;
  height=((GUI_Object*)sbow)->height;
  x=((GUI_Object*)sbow)->x;
  y=((GUI_Object*)sbow)->y;
  gtk_window_set_default_size(GTK_WINDOW(sbow->sbw.gui_obj.window), width,height);
  gtk_widget_set_uposition(GTK_WIDGET(sbow->sbw.gui_obj.window),x,y);


  
  /////////////////////////////////////////////////////////////////
  // create clist
  /////////////////////////////////////////////////////////////////
  scrolled_win = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show(scrolled_win);
  gtk_container_set_border_width (GTK_CONTAINER (scrolled_win), 5);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_win),
				  GTK_POLICY_AUTOMATIC, 
				  GTK_POLICY_AUTOMATIC);

  /* create GtkCList here so we have a pointer to throw at the 
   * button callbacks -- more is done with it later */
  clist = gtk_clist_new_with_titles (sbow->columns, sbow->column_titles);
  gtk_widget_show(clist);
  sbow->clist = clist;
  gtk_container_add (GTK_CONTAINER (scrolled_win), clist);

  /* Add a signal handler for button press events. This will capture
   * commands for setting and/or clearing break points
   */
  gtk_signal_connect(GTK_OBJECT(clist),"button_press_event",
		     (GtkSignalFunc) button_press,
		     (gpointer) sbow);


  label=gtk_label_new("Assembly");
  gtk_notebook_append_page(GTK_NOTEBOOK(sbow->notebook),scrolled_win,label);

  gtk_clist_set_row_height (GTK_CLIST (clist), 18);
  gtk_widget_set_usize (clist, 300, 100);
  gtk_clist_set_selection_mode (GTK_CLIST (clist), GTK_SELECTION_EXTENDED);

  for (i = 0; i < sbow->columns; i++)
    {
      gtk_clist_set_column_width (GTK_CLIST (clist), i, 80);

      gtk_clist_set_column_auto_resize (GTK_CLIST (clist), i, FALSE);
      gtk_clist_set_column_justification (GTK_CLIST (clist), i,
					  GTK_JUSTIFY_LEFT);
      // %%% FIX ME
      //   Hide the profile column for now...
      if(i == 0)
	gtk_clist_set_column_visibility (GTK_CLIST (clist), i, FALSE);
    }


  for (i = 0; i < DEFAULT_ROWS; i++)
    {
      sprintf (row_text[ADDRESS_COLUMN], "0x%04X", i);
      gtk_clist_append (GTK_CLIST (sbow->clist), row_text);
      gtk_clist_set_row_style (GTK_CLIST (sbow->clist), i, row_default_style);

    }

  row_default_style=gtk_clist_get_row_style(GTK_CLIST (clist), 0);




  /////////////////////////////////////////////////////////////////
  // create sheet
  /////////////////////////////////////////////////////////////////
  vbox=gtk_vbox_new(FALSE,1);
  gtk_widget_show(vbox);

  // Create entry bar
  hbox=gtk_hbox_new(FALSE,1);
  gtk_widget_show(hbox);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);
  
  sbow->label=gtk_label_new("");
  style=gtk_widget_get_style(sbow->label);
  style->font=normal_style->font;
  gtk_widget_set_style(sbow->label,style);
  gtk_widget_size_request(sbow->label, &request);
  gtk_widget_set_usize(sbow->label, 160, request.height);
  gtk_widget_show(sbow->label);
  gtk_box_pack_start(GTK_BOX(hbox), sbow->label, FALSE, TRUE, 0);

  sbow->entry=gtk_entry_new();
  style=gtk_widget_get_style(sbow->entry);
  style->font=normal_style->font;
  gtk_widget_set_style(sbow->entry,style);
  gtk_widget_show(sbow->entry);
  gtk_box_pack_start(GTK_BOX(hbox), sbow->entry, TRUE, TRUE, 0);
  
  // Create sheet
  scrolled_win=gtk_scrolled_window_new(NULL, NULL);
  gtk_widget_show(scrolled_win);
  gtk_box_pack_start(GTK_BOX(vbox), scrolled_win, TRUE, TRUE, 0);
  
  sbow->sheet=gtk_sheet_new(1,17,"where do this string go?");
  gtk_widget_show(sbow->sheet);
  gtk_container_add(GTK_CONTAINER(scrolled_win), sbow->sheet);
  
  label=gtk_label_new("Opcodes");
  gtk_notebook_append_page(GTK_NOTEBOOK(sbow->notebook),vbox,label);

  char_width = gdk_string_width (normal_style->font,"9");
  column_width = 5 * char_width + 6;
  for(i=0; i<GTK_SHEET(sbow->sheet)->maxcol; i++){
      //sprintf(name,"0x%02x",i);
      sprintf(name,"%02x",i);
      gtk_sheet_column_button_add_label(GTK_SHEET(sbow->sheet), i, name);
      gtk_sheet_set_column_title(GTK_SHEET(sbow->sheet), i, name);
      gtk_sheet_set_column_width (GTK_SHEET(sbow->sheet), i, column_width);
  }
  sprintf(name,"ASCII");
  gtk_sheet_column_button_add_label(GTK_SHEET(sbow->sheet), i, name);
  gtk_sheet_set_column_title(GTK_SHEET(sbow->sheet), i, name);
  gtk_sheet_set_row_titles_width(GTK_SHEET(sbow->sheet), column_width);

  
  gtk_signal_connect(GTK_OBJECT(sbow->sheet),
		     "button_press_event",
		     (GtkSignalFunc) do_popup,
		     sbow);

  gtk_signal_connect(GTK_OBJECT(gtk_sheet_get_entry(GTK_SHEET(sbow->sheet))),
		     "changed", (GtkSignalFunc)show_entry, sbow);

  gtk_signal_connect(GTK_OBJECT(sbow->sheet),
		     "activate", (GtkSignalFunc)activate_sheet_cell,
		     (gpointer) sbow);

  gtk_signal_connect(GTK_OBJECT(sbow->entry),
		     "changed", (GtkSignalFunc)show_sheet_entry, sbow);

  gtk_signal_connect(GTK_OBJECT(sbow->entry),
		     "activate", (GtkSignalFunc)activate_sheet_entry,
		     sbow);
  gtk_signal_connect(GTK_OBJECT(sbow->sheet),
		     "set_cell",
		     (GtkSignalFunc) parse_numbers,
		     sbow);
  /////////////////////////////////////////////////////////////////



  
  gtk_widget_show(scrolled_win);
  gtk_widget_show(sbow->sheet);



  
  gtk_signal_connect_after(GTK_OBJECT(sbow->sbw.gui_obj.window), "configure_event",
			   GTK_SIGNAL_FUNC(gui_object_configure_event),sbow);
  
  gtk_widget_show(sbow->sbw.gui_obj.window);

  sbow->sbw.gui_obj.enabled=1;

  sbow->sbw.gui_obj.is_built=1;


  
/*  style = gtk_widget_get_style(sbow->sbw.gui_obj.window);
  pixmap_pc = gdk_pixmap_create_from_xpm_d(sbow->sbw.gui_obj.window->window,
					   &mask,
					   &style->bg[GTK_STATE_NORMAL],
					   (gchar**)pc_xpm);
*/
//  sbow->pcwidget = gtk_pixmap_new(pixmap_pc,mask);
//  gtk_widget_show(sbow->pcwidget);
//  gtk_sheet_put(GTK_SHEET(sbow->sheet),sbow->pcwidget,0,0);

  
  
  if(sbow->processor)
      SourceBrowserOpcode_new_processor(sbow, sbow->sbw.gui_obj.gp);
  if(sbow->program)
      SourceBrowserOpcode_new_program(sbow, sbow->sbw.gui_obj.gp);
  
  /* create popupmenu */
  sbow->popup_menu=build_menu(sbow);

  update_menu_item((GUI_Object*)sbow);
}

int CreateSourceBrowserOpcodeWindow(GUI_Processor *gp)
{
    static char *titles[] =
    {
	"profile", "address", "opcode", "instruction"
    };

    static SourceBrowserOpcode_Window *sbow;

    sbow = (SourceBrowserOpcode_Window *) malloc(sizeof(SourceBrowserOpcode_Window));
    sbow->sbw.gui_obj.gp = NULL;
    sbow->sbw.gui_obj.window = NULL;

    sbow->column_titles = titles;
    sbow->columns = 4;


    sbow->sbw.gui_obj.gp = gp;
    gp->program_memory = (SourceBrowser_Window*)sbow;
    sbow->sbw.gui_obj.name = "program_memory";
    sbow->sbw.gui_obj.wc = WC_source;
    sbow->sbw.gui_obj.wt = WT_opcode_source_window;

    sbow->sbw.gui_obj.change_view = SourceBrowser_change_view;
    sbow->sbw.gui_obj.is_built = 0;

    sbow->memory=NULL;
    sbow->current_address=0;

    sbow->processor=0;
    sbow->program=0;

    gp_add_window_to_list(gp, (GUI_Object *)sbow);

    sbow->ascii_mode=1; /// default, two bytes/cell, MSB first
    config_get_variable(sbow->sbw.gui_obj.name,"ascii_mode",&sbow->ascii_mode);

    gui_object_get_config((GUI_Object*)sbow);

    if(sbow->sbw.gui_obj.enabled)
	BuildSourceBrowserOpcodeWindow(sbow);

    return 1;
}
#endif // HAVE_GUI
