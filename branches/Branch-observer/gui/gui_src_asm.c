#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>

#include "../config.h"
#ifdef HAVE_GUI

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>
#include <glib.h>
#include <string.h>
#include <ctype.h>

#include <gtkextra/gtkcombobox.h>
#include <gtkextra/gtkbordercombo.h>
#include <gtkextra/gtkcolorcombo.h>
#include <gtkextra/gtksheet.h>
//#include <gtkextra/gtksheetentry.h>

#include "gui.h"

#include <assert.h>

#include "../xpms/pc.xpm"
#include "../xpms/break.xpm"
#define PIXMAP_SIZE 14
#define PIXMAP_POS(sbaw,e) ((e)->pixel+(sbaw)->layout_offset+-PIXMAP_SIZE/2-(e)->font_center)

static GList *sa_xlate_list[SBAW_NRFILES]={NULL};  // lists containing sa_entry pointers

static struct sa_entry *gui_line_to_entry(int id,int line);
static struct sa_entry *gui_index_to_entry(int id, int index);

typedef enum {
    MENU_FIND_TEXT,
    MENU_FIND_PC,
    MENU_MOVE_PC,
    MENU_RUN_HERE,
    MENU_BP_HERE,
    MENU_SELECT_SYMBOL,
    MENU_STEP,
    MENU_STEP_OVER,
    MENU_RUN,
    MENU_STOP,
    MENU_RETURN,
    MENU_RESET
} menu_id;


typedef struct _menu_item {
    char *name;
    menu_id id;
    GtkWidget *item;
} menu_item;

static menu_item menu_items[] = {
    {"Find PC",         MENU_FIND_PC,NULL},
    {"Run here",        MENU_RUN_HERE,NULL},
    {"Move PC here",    MENU_MOVE_PC,NULL},
    {"Breakpoint here", MENU_BP_HERE,NULL},
    {"Select symbol",   MENU_SELECT_SYMBOL,NULL},
    {"Find text...",    MENU_FIND_TEXT,NULL},
};

static menu_item submenu_items[] = {
    {"Step",            MENU_STEP,NULL},
    {"Step Over",       MENU_STEP_OVER,NULL},
    {"Run",             MENU_RUN,NULL},
    {"Stop",            MENU_STOP,NULL},
    {"Reset",           MENU_RESET,NULL},
    {"Return",          MENU_RETURN,NULL},
};

// this should be in SourceBrowserAsm struct FIXME
static struct {
    int found;                   //
    int looped;                  // if search hit start or end of text
    int start;                   //
    int lastfound;               // index into text for start of last found string
    int i;                       //
    int lastid;                  //
    GtkWidget *window;           // the window for find dialog
    GtkWidget *entry;            // string GtkCombo
    GtkWidget *backwards_button; // togglebutton for direction
    GtkWidget *case_button;      // togglebutton for case sensitivity
    GList *combo_strings;        // list of strings for combo
    char *string;                // current string, extracted from entry
} search = {0,0,-1,0,0,0,0,0,0,0};


static int dlg_x=200, dlg_y=200;

// all of these gui_xxxx_to_entry() do linear searching.
// Binary search is possible, the list is sorted. But pic
// sources don't become large (not mine anyways).

//pixel is 0 -> maxfont-1 for line zero.
//         maxfont -> maxfont*2-1 for line one
//         ...
static struct sa_entry *gui_pixel_to_entry(int id, int pixel)
{
    struct sa_entry *e;      // to simplify expressions
    GList *p;                // iterator

    assert(sa_xlate_list[id]!=NULL);

    if(pixel<0)
	return (struct sa_entry*)sa_xlate_list[id]->data;
    
    p=sa_xlate_list[id];

    // find listentry with address larger than argument
    while(p->next!=NULL)
    {
        e = (struct sa_entry*)p->data;
	if(e->pixel > pixel)
	    break;
	p=p->next;
    }
    
    e=(struct sa_entry*)p->data;

    return e;
}

static struct sa_entry *gui_line_to_entry(int id, int line)
{
    struct sa_entry *e;
    GList *p;

    assert(sa_xlate_list[id]!=NULL);

    assert(line>=0);

    p=sa_xlate_list[id];

    /*
     locate listentry with index larger than argument
     */
    while(p->next!=NULL)
    {
        e = (struct sa_entry*)p->data;
	      
	if(e->line > line)
	    break;
	p=p->next;
    }

    assert(p->prev); // FIXME, happens if only one line of source
    p=p->prev;
    
    e=(struct sa_entry*)p->data;
    return e;
}

static struct sa_entry *gui_index_to_entry(int id, int index)
{
    struct sa_entry *e;
    GList *p;
    
    assert(sa_xlate_list[id]!=NULL);

    assert(index>=0);

    p=sa_xlate_list[id];
    
    /*
     locate listentry with index larger than argument
     */
    while(p->next!=NULL)
    {
        e = (struct sa_entry*)p->data;
	      
	if(e->index > index)
	    break;
	p=p->next;
    }
    
    assert(p->prev); // FIXME, could happen
    
    p=p->prev;
    
    e=(struct sa_entry*)p->data;
    return e;
}

void SourceBrowserAsm_set_pc(SourceBrowserAsm_Window *sbaw, int address)
{
    struct sa_entry *e;
    int row;
    unsigned int pixel;
    float inc;
    int i;

//    static GtkWidget *old_pcw=NULL;

    GtkWidget *new_pcw;
    int id=-1;
    
    assert(sbaw);

    if(!sbaw->source_loaded) return;


    // find notebook page containing address 'address'
    for(i=0;i<SBAW_NRFILES;i++)
    {
	if(sbaw->pageindex_to_fileid[i]==gpsim_get_file_id( ((GUI_Object*)sbaw)->gp->pic_id, address))
	{
	    id=i;
	}
	else
	{
	    if( sbaw->source_pcwidget[i]!=NULL &&
		GTK_WIDGET_VISIBLE(sbaw->source_pcwidget[i]) )
		gtk_widget_hide(sbaw->source_pcwidget[i]);
	}
    }

    assert(id>-1);
    
    new_pcw = sbaw->source_pcwidget[id];

    row = gpsim_get_src_line(((GUI_Object*)sbaw)->gp->pic_id, address);
    if(row==INVALID_VALUE)
	return;
    row--;
    
    gtk_notebook_set_page(GTK_NOTEBOOK(sbaw->notebook),id);

    if(sbaw->layout_offset<0)
    {   // can it normally be less than zero?
	// FIXME, this should be done whenever windows are reconfigured.
	int xtext,ytext;
	int xfixed, yfixed;

	if(GTK_TEXT(sbaw->source_text[id])->text_area!=NULL &&
	   sbaw->source_layout[id]->window!=NULL)
	{
	    gdk_window_get_origin(GTK_TEXT(sbaw->source_text[id])->text_area,&xtext,&ytext);
	    gdk_window_get_origin(sbaw->source_layout[id]->window,&xfixed,&yfixed);

	    sbaw->layout_offset = ytext-yfixed;
	}
    }
    e = gui_line_to_entry(id, row);

    pixel = PIXMAP_POS(sbaw,e);
    inc = GTK_ADJUSTMENT(GTK_TEXT(sbaw->source_text[id])->vadj)->page_increment;

    if( pixel<= GTK_TEXT(sbaw->source_text[id])->first_onscreen_ver_pixel ||
	pixel>= GTK_TEXT(sbaw->source_text[id])->first_onscreen_ver_pixel+inc )
	gtk_adjustment_set_value(GTK_ADJUSTMENT( GTK_TEXT( sbaw->source_text[id])->vadj),
				 pixel-inc/2);

//    if(GTK_IS_WIDGET(old_pcw) && old_pcw!=new_pcw)
//	gtk_widget_hide(old_pcw);
//    old_pcw=new_pcw;
    
    gtk_layout_move(GTK_LAYOUT(sbaw->source_layout[id]),
		    new_pcw,
		    PIXMAP_SIZE,
		    PIXMAP_POS(sbaw,e)
		   );

    if(!GTK_WIDGET_VISIBLE(new_pcw))
	gtk_widget_show(new_pcw);
}

static void pc_changed(struct cross_reference_to_gui *xref, int new_address)
{
    SourceBrowserAsm_Window *sbaw;

    sbaw = (SourceBrowserAsm_Window*)(xref->parent_window);

    if(sbaw->source_loaded)
    {
        SourceBrowserAsm_set_pc(sbaw, new_address);
    }
}

void SourceBrowserAsm_select_address( SourceBrowserAsm_Window *sbaw, int address)
{
    struct sa_entry *e;
    int id=-1, i;
    int pixel;
    float inc;
    int line;
    
    for(i=0;i<SBAW_NRFILES;i++)
    {
	if(sbaw->pageindex_to_fileid[i]==gpsim_get_file_id( ((GUI_Object*)sbaw)->gp->pic_id, address))
	{
	    id=i;
	}
    }
    assert(id>-1);
    gtk_notebook_set_page(GTK_NOTEBOOK(sbaw->notebook),id);
    
    line = gpsim_get_src_line(((GUI_Object*)sbaw)->gp->pic_id, address);
    if(line==INVALID_VALUE)
	return;
    
    //line = ((GUI_Object*)sbaw)->gp->p->program_memory[address]->get_src_line();
    e = gui_line_to_entry(id, line);

    pixel = PIXMAP_POS(sbaw,e);
    inc = GTK_ADJUSTMENT(GTK_TEXT(sbaw->source_text[id])->vadj)->page_increment;

    if( pixel<= GTK_TEXT(sbaw->source_text[id])->first_onscreen_ver_pixel ||
	pixel>= GTK_TEXT(sbaw->source_text[id])->first_onscreen_ver_pixel+inc )
	gtk_adjustment_set_value(GTK_ADJUSTMENT( GTK_TEXT( sbaw->source_text[id])->vadj),
				 pixel-inc/2);
}

/*
 this happens when breakpoint is set or unset
 ( Can it happen for another reason? )
 */
void SourceBrowserAsm_update_line( SourceBrowserAsm_Window *sbaw, int address)
{
  int row;

  int i,id=-1;
  struct sa_entry *e;
  
  assert(sbaw);

  
  assert(sbaw->sbw.gui_obj.wt == WT_asm_source_window);
  assert(address>=0);

  if(!sbaw->source_loaded) return;
  
  for(i=0;i<SBAW_NRFILES;i++)
  {
      if(sbaw->pageindex_to_fileid[i]==gpsim_get_file_id( ((GUI_Object*)sbaw)->gp->pic_id, address))
      {
	  id=i;
      }
  }

  if(id==-1)
  {
      puts("SourceBrowserAsm_update_line(): could not find notebook page");
      return;
  }
  
  row = gpsim_get_src_line(((GUI_Object*)sbaw)->gp->pic_id, address);
  //row = ((GUI_Object*)sbaw)->gp->p->program_memory[address]->get_src_line();

  if(row==INVALID_VALUE)
      return;
  row--;

  e = gui_line_to_entry(id,row);

  if(e==NULL)
  {
      puts("This is odd!?");
      return;
  }
  
  for(i=0;i<MAX_BREAKPOINTS;i++)
  {  // FIXME, reuse pixmaps or not?
      if(sbaw->breakpoint[i].widget==NULL)
	  break;
      if(sbaw->breakpoint[i].address==address)
	  break;
  }
  if(i>=MAX_BREAKPOINTS)
      return;
  
  if( gpsim_address_has_breakpoint( ((GUI_Object*)sbaw)->gp->pic_id, address))
  {
      if(sbaw->breakpoint[i].widget==NULL)
      {
	  sbaw->breakpoint[i].widget = gtk_pixmap_new(sbaw->pixmap_break,sbaw->mask);
	  gtk_layout_put(GTK_LAYOUT(sbaw->source_layout[id]),
			 sbaw->breakpoint[i].widget,
			 0,
			 PIXMAP_POS(sbaw,e)
			);
      }
      else
      {
	  gtk_layout_move(GTK_LAYOUT(sbaw->source_layout[id]),
			  sbaw->breakpoint[i].widget,
			  0,
			  PIXMAP_POS(sbaw,e)
			 );
      }
      sbaw->breakpoint[i].address=address;
      gtk_widget_show(sbaw->breakpoint[i].widget);
  }
  else if(sbaw->breakpoint[i].widget!=NULL)
  {
      gtk_widget_hide(sbaw->breakpoint[i].widget);
  }
}

SourceBrowserAsm_Window *popup_sbaw;

static void
popup_activated(GtkWidget *widget, gpointer data)
{
    menu_item *item;
    int id, address, line;
    char text[256];
    int i,start,end, temp;
    int pic_id;

    item = (menu_item *)data;
    id = gtk_notebook_get_current_page(GTK_NOTEBOOK(popup_sbaw->notebook));

    pic_id=((GUI_Object*)popup_sbaw)->gp->pic_id;
    
    switch(item->id)
    {
    case MENU_FIND_TEXT:
	gtk_widget_set_uposition(GTK_WIDGET(search.window),dlg_x,dlg_y);
	gtk_widget_show(search.window);
	break;
    case MENU_FIND_PC:
	pic_id = popup_sbaw->sbw.gui_obj.gp->pic_id;
	address=gpsim_get_pc_value(pic_id);
	SourceBrowserAsm_set_pc(popup_sbaw, address);
//	gui_simulation_has_stopped(); // FIXME
	break;
    case MENU_MOVE_PC:
	line = popup_sbaw->menu_data->line;

	address = gpsim_find_closest_address_to_line(pic_id,popup_sbaw->pageindex_to_fileid[id],line+1);
	if(address!=INVALID_VALUE)
	   gpsim_put_pc_value(pic_id,address);
	break;
    case MENU_RUN_HERE:
	line = popup_sbaw->menu_data->line;

	address = gpsim_find_closest_address_to_line(pic_id,popup_sbaw->pageindex_to_fileid[id],line+1);
        if(address!=INVALID_VALUE)
	    gpsim_run_to_address(pic_id, address);
	break;
    case MENU_BP_HERE:
	line = popup_sbaw->menu_data->line;
	gpsim_toggle_break_at_line(pic_id,popup_sbaw->pageindex_to_fileid[id] ,line+1);
	break;
    case MENU_SELECT_SYMBOL:
        start=GTK_EDITABLE(popup_sbaw->source_text[id])->selection_start_pos;
	end=GTK_EDITABLE(popup_sbaw->source_text[id])->selection_end_pos;
	if(start>end)
	{
		temp=start;
		start=end;
		end=temp;
	}
	if((end-start+2)>256) // FIXME bounds?
	    end=start+256-2;
	for(i=start;i<end;i++)
	{
	    text[i-start]=GTK_TEXT_INDEX(GTK_TEXT(popup_sbaw->source_text[id]),i);
	}
	text[i-start]=0;
	
	if(!popup_sbaw->sbw.gui_obj.gp->symbol_window->gui_obj.enabled)
	{
	    popup_sbaw->sbw.gui_obj.gp->symbol_window->gui_obj.
		change_view(&popup_sbaw->sbw.gui_obj.gp->symbol_window->gui_obj,
			    VIEW_SHOW);
	}
	SymbolWindow_select_symbol_name(((GUI_Object*)popup_sbaw)->gp->symbol_window,text);
	break;
    case MENU_STEP:
	gpsim_step(popup_sbaw->sbw.gui_obj.gp->pic_id, 1);
	break;
    case MENU_STEP_OVER:
	gpsim_step_over(popup_sbaw->sbw.gui_obj.gp->pic_id);
	break;
    case MENU_RUN:
	gpsim_run(popup_sbaw->sbw.gui_obj.gp->pic_id);
	break;
    case MENU_STOP:
    case MENU_RESET:
    case MENU_RETURN:
	puts("Not implemented");
	break;
    default:
	puts("Unhandled menuitem?");
	break;
    }
}

static GtkWidget *
build_menu(GtkWidget *sheet, SourceBrowserAsm_Window *sbaw)
{
    GtkWidget *menu;
    GtkWidget *submenu;
    GtkWidget *item;
    int i;
    int id;

	popup_sbaw=sbaw;

    id = gtk_notebook_get_current_page(GTK_NOTEBOOK(popup_sbaw->notebook));
    menu=gtk_menu_new();
    for (i=0; i < (sizeof(menu_items)/sizeof(menu_items[0])) ; i++){
	item=gtk_menu_item_new_with_label(menu_items[i].name);
	menu_items[i].item=item;
	gtk_signal_connect(GTK_OBJECT(item),"activate",
			   (GtkSignalFunc) popup_activated,
			   &menu_items[i]);

	gtk_widget_show(item);
	gtk_menu_append(GTK_MENU(menu),item);
    }

    submenu=gtk_menu_new();
    item = gtk_tearoff_menu_item_new ();
    gtk_menu_append (GTK_MENU (submenu), item);
    gtk_widget_show (item);
    for (i=0; i < (sizeof(submenu_items)/sizeof(submenu_items[0])) ; i++){
	item=gtk_menu_item_new_with_label(submenu_items[i].name);
	submenu_items[i].item=item;
	gtk_signal_connect(GTK_OBJECT(item),"activate",
			   (GtkSignalFunc) popup_activated,
			   &submenu_items[i]);

	GTK_WIDGET_SET_FLAGS (item, GTK_SENSITIVE | GTK_CAN_FOCUS);

	if(submenu_items[i].id==MENU_STOP)
	{
	    GTK_WIDGET_UNSET_FLAGS (item,
				    GTK_SENSITIVE | GTK_CAN_FOCUS);
	}
      
	gtk_widget_show(item);
	gtk_menu_append(GTK_MENU(submenu),item);
    }
    item = gtk_menu_item_new_with_label ("Controls");
    gtk_menu_append (GTK_MENU (menu), item);
    gtk_widget_show (item);
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), submenu);

    return menu;
}

/*
 button event handler for sbaw->source_text[id].
 If we get button1 doubleclick event then we toggle breakpoint
 If we get button3 buttonpress then we popup menu.
 */
static gint sigh_button_event(GtkWidget *widget,
		       GdkEventButton *event,
		       SourceBrowserAsm_Window *sbaw)
{
    int id;
    int i;
    GtkWidget *item;

    assert(event&&sbaw);

    id = gtk_notebook_get_current_page(GTK_NOTEBOOK(sbaw->notebook));

    
    if(event->type==GDK_BUTTON_PRESS &&
       event->button==3)
    {
	GtkWidget *popup=NULL;

	popup_sbaw=sbaw;

	sbaw->menu_data = gui_pixel_to_entry(id, (int) (event->y + GTK_TEXT(sbaw->source_text[id])->vadj->value));

	
	for (i=0; i < (sizeof(menu_items)/sizeof(menu_items[0])) ; i++){
	    item=menu_items[i].item;
	    
	    switch(menu_items[i].id){
	    case MENU_SELECT_SYMBOL:
		// Why does "if(editable->has_selection)" not work? FIXME
		if(GTK_EDITABLE(popup_sbaw->source_text[id])->selection_start_pos
		   ==GTK_EDITABLE(popup_sbaw->source_text[id])->selection_end_pos)
		{
		    gtk_widget_set_sensitive (item, FALSE);
		}
		else
		{
		    gtk_widget_set_sensitive (item, TRUE);
		}
		break;
	    default:
		break;
	    }
	}

	gtk_menu_popup(GTK_MENU(sbaw->popup_menu), NULL, NULL, NULL, NULL,
		       3, event->time);

	// override source_text[id]'s handler
	// is there a better way? FIXME
	gtk_signal_emit_stop_by_name(GTK_OBJECT(sbaw->source_text[id]),"button_press_event");
	return TRUE;
    }

    if(event->type==GDK_BUTTON_PRESS && event->button==4)
    { // wheel scroll up
	GTK_TEXT(sbaw->source_text[id])->vadj->value-=GTK_TEXT(sbaw->source_text[id])->vadj->page_increment/4.0;
	if(GTK_TEXT(sbaw->source_text[id])->vadj->value < GTK_TEXT(sbaw->source_text[id])->vadj->lower)
	    GTK_TEXT(sbaw->source_text[id])->vadj->value = GTK_TEXT(sbaw->source_text[id])->vadj->lower;
	gtk_adjustment_value_changed(GTK_TEXT(sbaw->source_text[id])->vadj);
	return TRUE;
    }
    if(event->type==GDK_BUTTON_PRESS && event->button==5)
    { // wheel scroll down
	GTK_TEXT(sbaw->source_text[id])->vadj->value+=GTK_TEXT(sbaw->source_text[id])->vadj->page_increment/4.0;
	if(GTK_TEXT(sbaw->source_text[id])->vadj->value > GTK_TEXT(sbaw->source_text[id])->vadj->upper-GTK_TEXT(sbaw->source_text[id])->vadj->page_increment)
	    GTK_TEXT(sbaw->source_text[id])->vadj->value = GTK_TEXT(sbaw->source_text[id])->vadj->upper-GTK_TEXT(sbaw->source_text[id])->vadj->page_increment;
	gtk_adjustment_value_changed(GTK_TEXT(sbaw->source_text[id])->vadj);
	return TRUE;
    }
    return FALSE;
}

static void text_adj_cb(GtkAdjustment *adj, GtkAdjustment *adj_to_update)
{
    // when sbaw->source_text[id] adjustment changes, we update the layout adj.
    
    // I assume that both adjustments count pixels
    
    assert(adj_to_update&&adj);

    if(adj_to_update && adj )
    {
	if (adj_to_update->upper >= adj->value )
	{
	    gtk_adjustment_set_value(adj_to_update, adj->value);
	}
    }
}

static float drag_scroll_speed;
static gint drag_scroll_cb(gpointer data)
{
    SourceBrowserAsm_Window *sbaw = (SourceBrowserAsm_Window*)data;
	
    int id = gtk_notebook_get_current_page(GTK_NOTEBOOK(sbaw->notebook));
    
    GTK_TEXT(sbaw->source_text[id])->vadj->value+=
	GTK_TEXT(sbaw->source_text[id])->vadj->step_increment*drag_scroll_speed;
    
    if(GTK_TEXT(sbaw->source_text[id])->vadj->value < GTK_TEXT(sbaw->source_text[id])->vadj->lower ||
       GTK_TEXT(sbaw->source_text[id])->vadj->value > GTK_TEXT(sbaw->source_text[id])->vadj->upper-GTK_TEXT(sbaw->source_text[id])->vadj->page_increment)
    {
	if(drag_scroll_speed > 0)
	    GTK_TEXT(sbaw->source_text[id])->vadj->value = GTK_TEXT(sbaw->source_text[id])->vadj->upper-GTK_TEXT(sbaw->source_text[id])->vadj->page_increment;
	else
	    GTK_TEXT(sbaw->source_text[id])->vadj->value = GTK_TEXT(sbaw->source_text[id])->vadj->lower;
    }
    
    gtk_adjustment_value_changed(GTK_TEXT(sbaw->source_text[id])->vadj);
    
    return 1;
}

/*
 This is handler for motion, button press and release for source_layout.
 The GdkEventMotion and GdkEventButton are very similar so I use
 the same for both!
 This function is too complex, FIXME.
 */


static void marker_cb(GtkWidget *w1,
		      GdkEventButton *event,
		     SourceBrowserAsm_Window *sbaw)
{
    static int dragbreak=0;
    static int dragstartline;
    static int dragindex;
    static int button_pressed;
    static int button_pressed_y;
    static int button_pressed_x;
    static int dragwidget_oldy;
    int pixel;
    int line;
    unsigned int address;
    
    static GtkWidget *dragwidget;
    static int dragwidget_x;
    

    int mindiff;
    int i;
    int diff;

    static int timeout_tag=-1;

    int id = gtk_notebook_get_current_page(GTK_NOTEBOOK(sbaw->notebook));

    int pic_id = ((GUI_Object*)sbaw)->gp->pic_id;
    
    switch(event->type)
    {
    case GDK_MOTION_NOTIFY:
	if(button_pressed == 1 && dragbreak == 0)
	{
	    button_pressed=0;
	    // actually button is pressed, but setting
	    // this to zero makes this block of code
	    // execute exactly once for each drag motion
	    
	    if(button_pressed_x<PIXMAP_SIZE)
	    {
		// find out if we want to start drag of a breakpoint
		i=0;
		mindiff=1000000; // large distance
		dragindex=-1;   // start with invalid index
		while(sbaw->breakpoint[i].widget != NULL)
		{
		    diff = button_pressed_y - (sbaw->breakpoint[i].widget->allocation.y+PIXMAP_SIZE/2);
		    if(abs(diff) < abs(mindiff))
		    {
			mindiff=diff;
			dragindex=i;
		    }
		    i++;
		}
		if(mindiff<PIXMAP_SIZE/2)
		{  // mouse hit breakpoint pixmap with index dragindex

		    // pixel = (position of pixmap in window)
		    //         - (constant) + (constant)
		    //         + (top of window, counting from top of text)
		    pixel = sbaw->breakpoint[dragindex].widget->allocation.y-
			sbaw->layout_offset+PIXMAP_SIZE/2+
			(int)GTK_TEXT(sbaw->source_text[id])->vadj->value;

		    // we want to remember which line drag started on
		    // to be able to disable this breakpoint later
		    // FIXME: perhaps we should simply disable bp now?
		    dragstartline = gui_pixel_to_entry(id,pixel)->line;

		    dragbreak=1;  // start drag
		    dragwidget = sbaw->breakpoint[dragindex].widget;
		    dragwidget_x = 0;
		    dragwidget_oldy=dragwidget->allocation.y+
			    (int)GTK_TEXT(sbaw->source_text[id])->vadj->value;
                    gtk_grab_add(sbaw->source_layout[id]);
		}
	    }
	    else
	    { // we see if we hit the pixmap widget
		if( abs(button_pressed_y-
			(sbaw->source_pcwidget[id]->allocation.y+PIXMAP_SIZE/2)) <PIXMAP_SIZE/2)
		{ // hit
		    dragbreak=1; // start drag
		    dragwidget = sbaw->source_pcwidget[id];
		    dragwidget_x = PIXMAP_SIZE;
		    dragwidget_oldy=dragwidget->allocation.y+
			    (int)GTK_TEXT(sbaw->source_text[id])->vadj->value;
                    gtk_grab_add(sbaw->source_layout[id]);
		}
	    }
	}
	else if(dragbreak==1)
	{  // drag is in progress
	    if((event->y/GTK_TEXT(sbaw->source_text[id])->vadj->page_size) >0.9
	       ||(event->y/GTK_TEXT(sbaw->source_text[id])->vadj->page_size) <0.1)
	    {
		if(timeout_tag==-1)
		{
		    timeout_tag = gtk_timeout_add(100,drag_scroll_cb,sbaw);
		}
		if((event->y/GTK_TEXT(sbaw->source_text[id])->vadj->page_size)>0.5)
		    drag_scroll_speed = ((event->y/GTK_TEXT(sbaw->source_text[id])->vadj->page_size)-0.9)*100;
		else
		    drag_scroll_speed = -(0.1-(event->y/GTK_TEXT(sbaw->source_text[id])->vadj->page_size))*100;
	    }
	    else if(timeout_tag!=-1)
	    {
		gtk_timeout_remove(timeout_tag);
		timeout_tag=-1;
	    }
	    
	    // update position of dragged pixmap
	    gtk_layout_move(GTK_LAYOUT(sbaw->source_layout[id]),
			    dragwidget,dragwidget_x,(int)event->y-PIXMAP_SIZE/2+
			    (int)GTK_TEXT(sbaw->source_text[id])->vadj->value);
	}
	break;
    case GDK_BUTTON_PRESS:
	if(button_pressed==1)
	    break;  // click number two(/three?) of a double click?
	button_pressed = 1;
	button_pressed_x = (int)event->x;  // and initial position of
	button_pressed_y = (int)event->y;  // possible drag action
	break;
    case GDK_2BUTTON_PRESS:
	if(event->button == 1)
	{
	    line = gui_pixel_to_entry(id, (int)event->y -
				      sbaw->layout_offset +
				      (int)GTK_TEXT(sbaw->source_text[id])->vadj->value)->line;
	    gpsim_toggle_break_at_line(pic_id,sbaw->pageindex_to_fileid[id] ,line+1);
	}
	break;
    case GDK_BUTTON_RELEASE:
	button_pressed=0;
	if(timeout_tag!=-1)
	{
	    gtk_timeout_remove(timeout_tag);
	    timeout_tag=-1;
	}
	if(dragbreak==0)
	    break;  // we weren't dragging, so we don't move anything
	dragbreak=0;

	gtk_grab_remove(sbaw->source_layout[id]);

	
	// pixel = (position of pixmap in window)
	//         + (constant) - (constant)
	//         + (top of window, counting from top of text)
	pixel = dragwidget->allocation.y+PIXMAP_SIZE/2-
	    sbaw->layout_offset+
	    (int)GTK_TEXT(sbaw->source_text[id])->vadj->value;
	line = gui_pixel_to_entry(id,pixel)->line;
	
	if(dragwidget == sbaw->source_pcwidget[id])
	{
	    address = gpsim_find_closest_address_to_line(pic_id,sbaw->pageindex_to_fileid[id],line+1);
	    if(address!=INVALID_VALUE)
		gpsim_put_pc_value(pic_id,address);
	    
	}
	else
	{
	    gpsim_toggle_break_at_line(pic_id,sbaw->pageindex_to_fileid[id] ,dragstartline+1);
	    gpsim_toggle_break_at_line(pic_id,sbaw->pageindex_to_fileid[id] ,line+1);
	}
	break;
    default:
	printf("Whoops? event type %d\n",event->type);
	break;
    }
}

/*
 Adds a page to the notebook, and returns notebook-id for that page.
 */
static int add_page(SourceBrowserAsm_Window *sbaw, int file_id)
{

    GtkWidget *hbox, *label, *vscrollbar;
    GtkStyle *style=NULL;

    int id;

    int pic_id = ((GUI_Object*)sbaw)->gp->pic_id;

    printf("%d\n",file_id);
    
    hbox = gtk_hbox_new(0,0);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 3);

    //    label=gtk_label_new(((GUI_Object*)sbaw)->gp->p->files[file_id].name);
    
    label=gtk_label_new(gpsim_get_file_context(pic_id, file_id)->name);

    gtk_notebook_append_page(GTK_NOTEBOOK(sbaw->notebook),hbox,label);

    id=gtk_notebook_page_num(GTK_NOTEBOOK(sbaw->notebook),hbox);
    
    assert(id<SBAW_NRFILES && id >=0);
    sbaw->pageindex_to_fileid[id] = file_id;
    sbaw->notebook_child[id]=hbox;
    
    gtk_widget_show(hbox);
    sbaw->source_layout_adj[id] = (GtkAdjustment*)gtk_adjustment_new(0.0,0.0,0.0,0.0,0.0,0.0);
    sbaw->source_layout[id] = gtk_layout_new(NULL,sbaw->source_layout_adj[id]);
    
    gtk_widget_set_events(sbaw->source_layout[id],
			  gtk_widget_get_events(sbaw->source_layout[id])|
			  GDK_BUTTON_PRESS_MASK |
			  GDK_BUTTON_MOTION_MASK |
			  GDK_BUTTON_RELEASE_MASK);
    gtk_widget_show(sbaw->source_layout[id]);

    gtk_widget_set_usize(GTK_WIDGET(sbaw->source_layout[id]),PIXMAP_SIZE*2,0);
    gtk_box_pack_start(GTK_BOX(hbox), sbaw->source_layout[id],
		       FALSE,FALSE, 0);
    
    vscrollbar = gtk_vscrollbar_new(NULL);
    
    gtk_widget_show(vscrollbar);

    sbaw->source_text[id] = gtk_text_new(NULL,GTK_RANGE(vscrollbar)->adjustment);

    gtk_text_set_word_wrap(GTK_TEXT(sbaw->source_text[id]),0);
    gtk_text_set_line_wrap(GTK_TEXT(sbaw->source_text[id]),0);
    gtk_widget_show(sbaw->source_text[id]);

    
	style=gtk_style_new();
	style->base[GTK_STATE_NORMAL].red=65535;
	style->base[GTK_STATE_NORMAL].green=65535;
	style->base[GTK_STATE_NORMAL].blue=65535;
    
    gtk_widget_set_style(GTK_WIDGET(sbaw->source_text[id]),style);

    gtk_signal_connect(GTK_OBJECT(sbaw->source_text[id]), "button_press_event",
		       GTK_SIGNAL_FUNC(sigh_button_event), sbaw);
    gtk_box_pack_start_defaults(GTK_BOX(hbox), sbaw->source_text[id]);
    
    gtk_box_pack_start(GTK_BOX(hbox), vscrollbar,
		       FALSE,FALSE, 0);
    gtk_signal_connect(GTK_OBJECT(GTK_TEXT(sbaw->source_text[id])->vadj),
		       "value_changed",GTK_SIGNAL_FUNC(text_adj_cb),sbaw->source_layout_adj[id]);

    gtk_signal_connect(GTK_OBJECT(sbaw->source_layout[id]),"motion-notify-event",
		       GTK_SIGNAL_FUNC(marker_cb),sbaw);
    gtk_signal_connect(GTK_OBJECT(sbaw->source_layout[id]),"button_press_event",
		       GTK_SIGNAL_FUNC(marker_cb),sbaw);
    gtk_signal_connect(GTK_OBJECT(sbaw->source_layout[id]),"button_release_event",
		       GTK_SIGNAL_FUNC(marker_cb),sbaw);
    
//    while(gtk_events_pending()) // display everything, so that
//	gtk_main_iteration();  // gtk_notebook_get_current_page() works


  // We create pixmaps here, where the gtk_widget_get_style() call will
  // succeed. I tried putting this code in CreateSourceBrowserAsmWindow()
  // but then the window was not realized. And if I manually realized
  // it, then the call to gtk_window_set_default_size() was ignored.
  // Was that a bug in gtk? (gtk version 1.2.3)
  if(sbaw->pixmap_pc==NULL)
  {
      style = gtk_widget_get_style(sbaw->sbw.gui_obj.window);
      sbaw->mask = NULL;
      sbaw->pixmap_pc = gdk_pixmap_create_from_xpm_d(sbaw->sbw.gui_obj.window->window, &sbaw->mask,
						     &style->bg[GTK_STATE_NORMAL],
						     (gchar**)pc_xpm);
      sbaw->pixmap_break = gdk_pixmap_create_from_xpm_d(sbaw->sbw.gui_obj.window->window, &sbaw->mask,
							&style->bg[GTK_STATE_NORMAL],
							(gchar**)break_xpm);
  }
  sbaw->source_pcwidget[id] = gtk_pixmap_new(sbaw->pixmap_pc,sbaw->mask);
  gtk_layout_put(GTK_LAYOUT(sbaw->source_layout[id]),
		 sbaw->source_pcwidget[id],0,0);

    return id;
    
}

/*
 Fills sbaw->source_text[id] with text from
 file pointer sbaw->sbw.gui_obj.gp->p->files[file_id].file_ptr
 */
static void set_text(SourceBrowserAsm_Window *sbaw, int id, int file_id)
{
    int totallinesheight;
    int instruction_done;
    int lineascent, linedescent;
    char *p;
    char text_buffer[256];
    int cblock=0;
    int i;

    int index;

    int pic_id;
    
    int line=0;
    struct sa_entry *entry;
    GList *iter;
    
    // get a manageable pointer to the processor
    pic_id = ((GUI_Object*)sbaw)->gp->pic_id;

    
    
    gtk_text_freeze(GTK_TEXT(sbaw->source_text[id]));

    fseek(gpsim_get_file_context(pic_id,file_id)->file_ptr,
	  0,
	  SEEK_SET);


    gtk_editable_delete_text(GTK_EDITABLE(sbaw->source_text[id]),0,-1);
    for(iter=sa_xlate_list[id];iter!=NULL;)
    {
	GList *next=iter->next;
	free( (struct sa_entry*)iter->data );
	g_list_remove(iter,iter->data); // FIXME. I really need a tutorial
	iter=next;
	//	g_list_free_1(sa_xlate_list[id]);  // FIXME, g_list_free() difference?
    }
    sa_xlate_list[id]=NULL;
    

    for(i=0;i<MAX_BREAKPOINTS;i++)
	if(sbaw->breakpoint[i].widget!=NULL)
	{
	    sbaw->breakpoint[i].address=-1;
	    gtk_widget_hide(sbaw->breakpoint[i].widget);
	    sbaw->breakpoint[i].widget=NULL;
	}


    totallinesheight=0;

    while(fgets(text_buffer, 256, gpsim_get_file_context(pic_id,file_id)->file_ptr)!=NULL)
    {
	char *end, *q;

	lineascent=-1;
	linedescent=-1;
	instruction_done=0; // to seperate instruction from other text (symbols)

	index=gtk_text_get_length(GTK_TEXT(sbaw->source_text[id]));

	p=text_buffer;

	/* all of this pattern matching is a hack, the code is
	 extended when i find misbehaviour */

	/* would it be better to use some regex stuff? */

	/* gtksctext could be a option, but then i guess gtksctext would
	 have to be extended with some gtk_sctext_get_pixel_of_line(int line)
	 or similar funtionality
         There is also a gtkextext widget...
	 */

	if(*p=='#' || !strncmp(p,"include",7))
	{ // not a label
	    q=p;
	    q++;
	    while(isalnum(*q) || *q=='_')
		q++;
	    gtk_text_insert(GTK_TEXT(sbaw->source_text[id]),
			    sbaw->default_text_style.font,
			    &sbaw->default_text_style.fg[GTK_STATE_NORMAL],
			    &sbaw->default_text_style.base[GTK_STATE_NORMAL],
			    p,
			    q-p);

	    p=q;
	    instruction_done=1; // well, varable misnamed
	}
	else if( (isalnum(*p) || *p=='_'))
	{ // a label
	    // locate end of label
	    q=p;
	    while(isalnum(*q) || *q=='_')
		q++;

	    if(lineascent<sbaw->label_text_style.font->ascent)
		lineascent=sbaw->label_text_style.font->ascent;
	    if(linedescent<sbaw->label_text_style.font->descent)
		linedescent=sbaw->label_text_style.font->descent;
	    gtk_text_insert(GTK_TEXT(sbaw->source_text[id]),
			    sbaw->label_text_style.font,
			    &sbaw->label_text_style.fg[GTK_STATE_NORMAL],
			    &sbaw->label_text_style.base[GTK_STATE_NORMAL],
			    text_buffer,
			    q-p);

	    // advance the pointer p
	    p=q;
	}

	// 'end' is end of line
	end = text_buffer + strlen(text_buffer);

	// loop through the rest of the line
	while( p < end )
	{
	    if( *p == ';')
	    { // comment
		if(lineascent<sbaw->comment_text_style.font->ascent)
		    lineascent=sbaw->comment_text_style.font->ascent;
		if(linedescent<sbaw->comment_text_style.font->descent)
		    linedescent=sbaw->comment_text_style.font->descent;
		gtk_text_insert(GTK_TEXT(sbaw->source_text[id]),
				sbaw->comment_text_style.font,
				&sbaw->comment_text_style.fg[GTK_STATE_NORMAL],
				&sbaw->comment_text_style.base[GTK_STATE_NORMAL],
				p,
				-1);
		break;
	    }
	    else if(isalpha(*p) || *p=='_')
	    { // instruction, symbol or cblock
		q=p;
		while(isalnum(*q) || *q=='_')
		    q++;
		if( ( instruction_done==0 && cblock==0) || !strncasecmp(p,"endc",4) )
		{  // instruction or cblock
		    instruction_done=1;
		    cblock=0;
		    if(lineascent<sbaw->instruction_text_style.font->ascent)
			lineascent=sbaw->instruction_text_style.font->ascent;
		    if(linedescent<sbaw->instruction_text_style.font->descent)
			linedescent=sbaw->instruction_text_style.font->descent;
		if(!strncasecmp(p,"cblock",6))
		{
		    cblock=1;
		}
		    gtk_text_insert(GTK_TEXT(sbaw->source_text[id]),
				    sbaw->instruction_text_style.font,
				    &sbaw->instruction_text_style.fg[GTK_STATE_NORMAL],
				    &sbaw->instruction_text_style.base[GTK_STATE_NORMAL],
				    p,
				    q-p);
		}
		else
		{ // symbol
		    if(lineascent<sbaw->symbol_text_style.font->ascent)
			lineascent=sbaw->symbol_text_style.font->ascent;
		    if(linedescent<sbaw->symbol_text_style.font->descent)
			linedescent=sbaw->symbol_text_style.font->descent;
		    gtk_text_insert(GTK_TEXT(sbaw->source_text[id]),
				    sbaw->symbol_text_style.font,
				    &sbaw->symbol_text_style.fg[GTK_STATE_NORMAL],
				    &sbaw->symbol_text_style.base[GTK_STATE_NORMAL],
				    p,
				    q-p);
		}
		p=q;
	    }
	    else if( isxdigit(*p))
	    { // number
		q=p;
		if(*p=='0' && toupper(*(p+1))=='X')
		    q+=2;
		while(isxdigit(*q))
		    q++;
		if(lineascent<sbaw->number_text_style.font->ascent)
		    lineascent=sbaw->number_text_style.font->ascent;
		if(linedescent<sbaw->number_text_style.font->descent)
		    linedescent=sbaw->number_text_style.font->descent;
		gtk_text_insert(GTK_TEXT(sbaw->source_text[id]),
				sbaw->number_text_style.font,
				&sbaw->number_text_style.fg[GTK_STATE_NORMAL],
				&sbaw->number_text_style.base[GTK_STATE_NORMAL],
				p,
				q-p);
		p=q;
	    }
	    else
	    { // default
		// FIXME, add a 'whitespace_text_style'
		// There is a small annoyance here. If the source
		// initially on a line have whitespace, followed by
		// a comment. Now if the comment have a smaller font
		// than the default font then the line will have line
		// spacing larger than nessesary.
		if(lineascent<sbaw->default_text_style.font->ascent)
		    lineascent=sbaw->default_text_style.font->ascent;
		if(linedescent<sbaw->default_text_style.font->descent)
		    linedescent=sbaw->default_text_style.font->descent;
		gtk_text_insert(GTK_TEXT(sbaw->source_text[id]),
				sbaw->default_text_style.font,
				&sbaw->default_text_style.fg[GTK_STATE_NORMAL],
				&sbaw->default_text_style.base[GTK_STATE_NORMAL],
				p,
				1);
		p++;
	    }
	} //end of while( p < end )

	if(lineascent==-1)
	    lineascent=sbaw->default_text_style.font->ascent;
	if(linedescent==-1)
	    linedescent=sbaw->default_text_style.font->descent;

	totallinesheight+=linedescent+lineascent;

	// crate an entry in sa_xlate_list for this source line.
	// 'this source line' is the one in 'buf' with line number
	// 'line' and index 'index' into text
	entry=(struct sa_entry*) malloc(sizeof(struct sa_entry));
	entry->index=index;
	entry->line=line;
	entry->pixel=totallinesheight;
	entry->font_center=lineascent-linedescent;

	sa_xlate_list[id]=g_list_append(sa_xlate_list[id],entry);
	line++;
    } // end while(fgets(...)...)

    // this made the end case of the search dialog simpler once
    gtk_text_insert(GTK_TEXT(sbaw->source_text[id]),
		    sbaw->default_text_style.font,
		    &sbaw->default_text_style.fg[GTK_STATE_NORMAL],
		    &sbaw->default_text_style.base[GTK_STATE_NORMAL],
		    " ",
		    1);


  
    gtk_layout_set_size(GTK_LAYOUT(sbaw->source_layout[id]),2*PIXMAP_SIZE,totallinesheight);
    gtk_text_thaw(GTK_TEXT(sbaw->source_text[id]));

}

void SourceBrowserAsm_close_source(SourceBrowserAsm_Window *sbaw, GUI_Processor *gp)
{
    int i;

    struct breakpoint_info breakpoint_false={-1,NULL};
    
    
    sbaw->load_source=0;
    sbaw->source_loaded = 0;
    if(! ((GUI_Object*)sbaw)->enabled)
	return;
    
    for(i=0;i<SBAW_NRFILES;i++)
    {
	if(sbaw->notebook_child[i]!=NULL)
	{
	    int num=gtk_notebook_page_num(GTK_NOTEBOOK(sbaw->notebook),sbaw->notebook_child[i]);
	    gtk_notebook_remove_page(GTK_NOTEBOOK(sbaw->notebook),num);
	    sbaw->notebook_child[i]=NULL;
	}
//	    sbaw->notebook_child[i]=
//		sbaw->notebook_child[i]=
//		sbaw->source_layout[i]=
//		sbaw->source_text[i]=
		sbaw->source_pcwidget[i]=NULL;
//	    sbaw->source_layout_adj[i]=
	    sbaw->pageindex_to_fileid[i]=-1;
    }
    sbaw->pixmap_pc=NULL;
//    gtk_widget_remove(sbaw->notebook);
//    sbaw->notebook

  for(i=0;i<MAX_BREAKPOINTS;i++)
      sbaw->breakpoint[i] = breakpoint_false;
  sbaw->layout_offset=-1;
}

void SourceBrowserAsm_new_source(SourceBrowserAsm_Window *sbaw, GUI_Processor *gp)
{

  int i;
  int id;
  
  int pic_id;

  char *file_name;
  struct file_context *gpsim_file;
  int file_id;

  struct cross_reference_to_gui *cross_reference;
  int address;

  assert(sbaw);
  
  if(sbaw == NULL || gp == NULL)
    return;

  if(! ((GUI_Object*)sbaw)->enabled)
  {
      sbaw->load_source=1;
      return;
  }
  
  assert(sbaw->sbw.gui_obj.wt==WT_asm_source_window);
  
  pic_id = ((GUI_Object*)sbaw)->gp->pic_id;

  SourceBrowserAsm_close_source(sbaw,gp);

  sbaw->load_source=1;
  
  /* Now create a cross-reference link that the
   * simulator can use to send information back to the gui
   */
  cross_reference = (struct cross_reference_to_gui *) malloc(sizeof(struct cross_reference_to_gui));
  cross_reference->parent_window_type =   WT_asm_source_window;
  cross_reference->parent_window = (gpointer) sbaw;
  cross_reference->data = (gpointer) NULL;
  cross_reference->update = pc_changed;
  cross_reference->remove = NULL;
  gpsim_assign_pc_xref(pic_id, cross_reference);

  for(i=0;i<gpsim_get_number_of_source_files(pic_id);i++)
  {
      gpsim_file = gpsim_get_file_context(pic_id, i);
      file_name = gpsim_file->name;
      if(!strcmp(file_name+strlen(file_name)-3,"asm")
	 ||!strcmp(file_name+strlen(file_name)-3,"inc")
	)
      {
	  // FIXME, gpsim may change sometime making this fail
	  file_id = i;

	  // Make sure that the file is open
	  if(gpsim_file->file_ptr == NULL)
	  {
	      if(file_name != NULL)
		  gpsim_file->file_ptr = fopen(file_name,"r");

	      if(gpsim_file->file_ptr == NULL)
	      {
		  printf("file \"%s\" not found\n",file_name);
		  continue;
	      }
	  }

	  id = add_page(sbaw,file_id);

	  set_text(sbaw,id,file_id);
	  
      }
  }

  sbaw->source_loaded = 1;

  // Why is this needed? set_page() in SourceBrowserAsm_set_pc()
  // fails with widget_map() -> not visible
  while(gtk_events_pending())
      gtk_main_iteration();
  
  address = gpsim_get_pc_value(pic_id);
  if(address==INVALID_VALUE)
      puts("Warning, PC is invalid?");
  else
      SourceBrowserAsm_set_pc(sbaw, address);
}


void SourceBrowserAsm_new_processor(SourceBrowserAsm_Window *sbaw, GUI_Processor *gp)
{
    assert(sbaw&&gp);
    
  if(sbaw == NULL || gp == NULL)
      return;

  SourceBrowserAsm_close_source(sbaw, gp);
}


static gint configure_event(GtkWidget *widget, GdkEventConfigure *e, gpointer data)
{
    if(widget->window==NULL)
	return 0;
    
    gdk_window_get_root_origin(widget->window,&dlg_x,&dlg_y);
    return 0; // what should be returned?, FIXME
}
    
static gboolean
message_close_cb(GtkWidget *widget, gpointer d)
{
    gtk_widget_hide(GTK_WIDGET(d));
    
    return FALSE;
}

static int gui_message(char *message)
{
    static GtkWidget *dialog=NULL;
    static GtkWidget *label;
    GtkWidget *button;
    GtkWidget *hbox;

    assert(message);
    
    if(dialog==NULL)
    {
	dialog = gtk_dialog_new();
	
	gtk_signal_connect(GTK_OBJECT(dialog),
			   "configure_event",GTK_SIGNAL_FUNC(configure_event),0);
	gtk_signal_connect_object(GTK_OBJECT(dialog),
				  "delete_event",GTK_SIGNAL_FUNC(gtk_widget_hide),(gpointer)dialog);

	hbox = gtk_hbox_new(0,0);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), hbox,FALSE,FALSE,20);

	button = gtk_button_new_with_label("OK");
	gtk_widget_show(button);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area), button,
			   FALSE,FALSE,10);
	gtk_signal_connect(GTK_OBJECT(button),"clicked",
			   GTK_SIGNAL_FUNC(message_close_cb),(gpointer)dialog);
	GTK_WIDGET_SET_FLAGS(button,GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);

	label=gtk_label_new(message);
	gtk_box_pack_start(GTK_BOX(hbox), label,
			   FALSE,FALSE, 20);

	gtk_widget_show(hbox);
	gtk_widget_show(label);
    }
    else
    {
	gtk_label_set_text(GTK_LABEL(label),message);
    }
    
    gtk_widget_set_uposition(GTK_WIDGET(dialog),dlg_x,dlg_y);
    gtk_widget_show_now(dialog);

    return 0;
}

static void a_cb(GtkWidget *w, gpointer user_data)
{
    *(int*)user_data=TRUE;
    gtk_main_quit();
}

static void b_cb(GtkWidget *w, gpointer user_data)
{
    *(int*)user_data=FALSE;
    gtk_main_quit();
}

// modal dialog, asking a yes/no question
static int gui_question(char *question, char *a, char *b)
{
    static GtkWidget *dialog=NULL;
    static GtkWidget *label;
    GtkWidget *button;
    GtkWidget *hbox;
    static int retval;
    
    if(dialog==NULL)
    {
	dialog = gtk_dialog_new();
	gtk_signal_connect(GTK_OBJECT(dialog),
			   "configure_event",GTK_SIGNAL_FUNC(configure_event),0);
	gtk_signal_connect_object(GTK_OBJECT(dialog),
			      "delete_event",GTK_SIGNAL_FUNC(gtk_widget_hide),(gpointer)dialog);
	
	hbox = gtk_hbox_new(0,0);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), hbox,FALSE,FALSE,20);

	button = gtk_button_new_with_label(a);
	gtk_widget_show(button);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area), button,
			   FALSE,FALSE,10);
	gtk_signal_connect(GTK_OBJECT(button),"clicked",
			   GTK_SIGNAL_FUNC(a_cb),(gpointer)&retval);
	GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
	
	button = gtk_button_new_with_label(b);
	gtk_widget_show(button);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area), button,
			   FALSE,FALSE,10);
	gtk_signal_connect(GTK_OBJECT(button),"clicked",
			   GTK_SIGNAL_FUNC(b_cb),(gpointer)&retval);

	label=gtk_label_new(question);
	gtk_box_pack_start(GTK_BOX(hbox), label,
			   FALSE,FALSE, 20);

	gtk_widget_show(hbox);
	gtk_widget_show(label);
    }
    else
    {
	gtk_label_set_text(GTK_LABEL(label),question);
    }
    
    gtk_widget_set_uposition(GTK_WIDGET(dialog),dlg_x,dlg_y);
    gtk_widget_show_now(dialog);

    gtk_grab_add(dialog);
    gtk_main();
    gtk_grab_remove(dialog);
    
    gtk_widget_hide(dialog);

//    puts(retval==1?"Yes":"No");
    
    return retval;
}


/*
 A rather long function, simplify main loop. FIXME.
 */
static void find_cb(GtkWidget *w, SourceBrowserAsm_Window *sbaw)
{

    
    char *p;
    GList *l;
    
    int casesensitive;
    int direction;
    
    int last_matched=0;
    int k=0;
    
    int char1, char2;
    int j;  // index into search string
    int tlen;
    int id;

  if(!sbaw->source_loaded) return;
    
  id = gtk_notebook_get_current_page(GTK_NOTEBOOK(sbaw->notebook));

    if(id != search.lastid)
    { //  Changed notebook tab since last search. reset search.
        search.lastid=id;
	search.found=0;
	search.looped=0;
	search.start=0;
	search.lastfound=0;
	search.i=0;
    }
    
    if(GTK_TOGGLE_BUTTON(search.case_button)->active)
	casesensitive=TRUE;
    else
	casesensitive=FALSE;

    if(GTK_TOGGLE_BUTTON(search.backwards_button)->active)
	direction=-1;
    else
	direction=1;

    p=gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(search.entry)->entry));

    if(*p=='\0')
	return;

    if(search.string==NULL || strcmp(search.string,p))
    {  // not same string as last time
	// search list to prevent duplicates
	l=search.combo_strings;
	while(l)
	{
	    if(!strcmp((char*)l->data,p))
	    {
		// the string p already is in list
		// move it first?, FIXME

		search.string = (char*)l->data;
		break;
	    }

	    l=l->next;
	}
	if(l == NULL)
	{ // we didn't find string in history, create a new one
	    search.string=(char*)malloc(strlen(p)+1);
	    strcpy(search.string,p);
	    search.combo_strings = g_list_prepend(search.combo_strings,search.string);
	    gtk_combo_set_popdown_strings(GTK_COMBO(search.entry),search.combo_strings);
	}

	// initialize variables for a new search
	search.found=0;
	search.looped=0;
	search.i = gui_pixel_to_entry(id,(int)GTK_TEXT(sbaw->source_text[id])->vadj->value)->index;
	search.start = search.i; // remember where we started searching
    }

    tlen =gtk_text_get_length(GTK_TEXT(sbaw->source_text[id]));
    j=0;
    for(;search.i>=0 && search.i<tlen;search.i+=direction)
    {
	if(search.string[j]=='\0')
	{  // match! We found the string in text.
	    int start_i, end_i;
	    
	    search.found++;

	    start_i = k+ (direction==-1);      // comparing backwards means
	    end_i = search.i+ (direction==-1); // we have to add 1

	    if(start_i>end_i)
	    {
		int temp=end_i;  // swap, so that k is the smaller
		end_i=start_i;
		start_i=temp;
	    }
	    assert(start_i<end_i);
	    if(start_i==search.lastfound)
	    {  // we found the same position as last time
                // happens when searching backwards
		j=0;
		if(direction==1)
		    search.i++; // skip this match
		else
		    search.i--; // skip this match
		last_matched=0;
	    }
	    else
	    {
		int pixel;
		float inc;

		search.lastfound=start_i;

		pixel = gui_index_to_entry(id,start_i)->pixel;
		inc = GTK_ADJUSTMENT(GTK_TEXT(sbaw->source_text[id])->vadj)->page_increment;
		gtk_adjustment_set_value(GTK_ADJUSTMENT( GTK_TEXT( sbaw->source_text[id])->vadj),
					 pixel-inc/2);
		gtk_editable_select_region(GTK_EDITABLE(sbaw->source_text[id]),start_i,end_i);
		return;
	    }
	}
	if(search.looped && (search.start == search.i))
	{
	    if(search.found==0)
	    {
		gui_message("Not found");
		return;
	    }
	    else if(search.found==1)
	    {
		gui_message("Just a single occurance in text");

		// so that the next next call marks text too, we do:
		search.found=0;
		search.looped=0;
		search.lastfound=-1;
		return;
	    }
	}

	// get another character
	char1=GTK_TEXT_INDEX(GTK_TEXT(sbaw->source_text[id]),(unsigned)search.i);
	if(direction==1)
	    char2=search.string[j];
	else
	    char2=search.string[strlen(search.string)-1-j];
	//FIXME, many calls to strlen

	if(casesensitive==FALSE)
	{
	    char1=toupper(char1); // FIXME, what about native letters?
	    char2=toupper(char2);
	}

	if(char1!=char2)
	{                   // if these characters don't match
	    j=0;            // set search index for string back to zero
	    last_matched=0; // char in this loop didn't match
	}
	else
	{
	    if(!last_matched)
	    {
		k=search.i;     // remember first matching index for later
		last_matched=1; // char in this loop matched
	    }
	    j++;  // forward string index to compare next char
	}
	
    }
    // the string was not found in text between index 'search_start' and
    // one end of text (index '0' or index 'tlen')

    // We ask user it he want to search from other end of file
    if(direction==1)
    {
	if(gui_question("I reached end of file\ncontinue from start?","Yes","No")==TRUE)
	{
	    search.i=0;
	    search.looped=1;
	    find_cb(w,sbaw);  // tail recursive, FIXME
	    return;
	}
	else
	    search.i=tlen-1;
    }
    else
    {
	if(gui_question("I reached top of file\ncontinue from end?","Yes", "No")==TRUE)
	{
	    search.i=tlen-1;
	    search.looped=1;
	    find_cb(w,sbaw);  // tail recursive, FIXME
	    return;
	}
	else
	    search.i=0;
    }
}

static void find_clear_cb(GtkWidget *w, SourceBrowserAsm_Window *sbaw)
{
    gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(search.entry)->entry),"");
}

void BuildSourceBrowserAsmWindow(SourceBrowserAsm_Window *sbaw)
{
  GtkWidget *hbox;
  GtkWidget *button;

  GtkWidget *label;
    GdkColor text_fg;
    GdkColor text_bg;

  int x,y,width,height;
  

  CreateSBW((SourceBrowser_Window*)sbaw);

  
  gtk_window_set_title (GTK_WINDOW (sbaw->sbw.gui_obj.window), "Source Browser");

  width=((GUI_Object*)sbaw)->width;
  height=((GUI_Object*)sbaw)->height;
  x=((GUI_Object*)sbaw)->x;
  y=((GUI_Object*)sbaw)->y;
  gtk_window_set_default_size(GTK_WINDOW(sbaw->sbw.gui_obj.window), width,height);
  gtk_widget_set_uposition(GTK_WIDGET(sbaw->sbw.gui_obj.window),x,y);

  
    sbaw->notebook = gtk_notebook_new();
    gtk_widget_show(sbaw->notebook);

    sbaw->popup_menu=build_menu(sbaw->notebook,sbaw);

    gtk_box_pack_start (GTK_BOX (sbaw->sbw.vbox), sbaw->notebook, TRUE, TRUE, 0);

    gdk_color_parse("black", &text_fg);
    gdk_color_parse("white", &text_bg);
    sbaw->default_text_style.base[GTK_STATE_NORMAL] = text_bg;
    sbaw->default_text_style.fg[GTK_STATE_NORMAL] = text_fg;

    gdk_color_parse("dark green", &text_fg);
    gdk_color_parse("white", &text_bg);
    sbaw->symbol_text_style.base[GTK_STATE_NORMAL] = text_bg;
    sbaw->symbol_text_style.fg[GTK_STATE_NORMAL] = text_fg;

    gdk_color_parse("orange", &text_fg);
    gdk_color_parse("white", &text_bg);
    sbaw->label_text_style.base[GTK_STATE_NORMAL] = text_bg;
    sbaw->label_text_style.fg[GTK_STATE_NORMAL] = text_fg;

    gdk_color_parse("red", &text_fg);
    gdk_color_parse("white", &text_bg);
    sbaw->instruction_text_style.base[GTK_STATE_NORMAL] = text_bg;
    sbaw->instruction_text_style.fg[GTK_STATE_NORMAL] = text_fg;

    gdk_color_parse("blue", &text_fg);
    gdk_color_parse("white", &text_bg);
    sbaw->number_text_style.base[GTK_STATE_NORMAL] = text_bg;
    sbaw->number_text_style.fg[GTK_STATE_NORMAL] = text_fg;

    gdk_color_parse("black", &text_fg);
    gdk_color_parse("gray", &text_bg);
    sbaw->comment_text_style.base[GTK_STATE_NORMAL] = text_bg;
    sbaw->comment_text_style.fg[GTK_STATE_NORMAL] = text_fg;


    sbaw->comment_text_style.font=
	gdk_font_load("-adobe-courier-bold-o-*-*-*-120-*-*-*-*-*-*");
    
    sbaw->default_text_style.font=
	sbaw->label_text_style.font=
	sbaw->symbol_text_style.font=
	sbaw->instruction_text_style.font=
	sbaw->number_text_style.font=
	gdk_font_load("-adobe-courier-bold-r-*-*-*-120-*-*-*-*-*-*");

    
    search.lastid=-1;  // will reset search
    
    search.window = gtk_dialog_new();

    gtk_signal_connect(GTK_OBJECT(search.window),
		       "configure_event",GTK_SIGNAL_FUNC(configure_event),0);
    gtk_signal_connect_object(GTK_OBJECT(search.window),
		       "delete_event",GTK_SIGNAL_FUNC(gtk_widget_hide),(gpointer)search.window);
    
    gtk_window_set_title(GTK_WINDOW(search.window),"Find");
    
    hbox = gtk_hbox_new(FALSE,15);
    gtk_widget_show(hbox);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(search.window)->vbox),hbox,
		       FALSE,TRUE,5);
    label = gtk_label_new("Find:");
    gtk_widget_show(label);
    gtk_box_pack_start(GTK_BOX(hbox),label,
		       FALSE,FALSE,5);
    search.entry = gtk_combo_new();
    gtk_widget_show(search.entry);
    gtk_box_pack_start(GTK_BOX(hbox),search.entry,
		       TRUE,TRUE,5);
    gtk_combo_disable_activate(GTK_COMBO(search.entry));
    gtk_signal_connect(GTK_OBJECT(GTK_COMBO(search.entry)->entry),"activate",
		       GTK_SIGNAL_FUNC(find_cb),sbaw);
    
    hbox = gtk_hbox_new(FALSE,15);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(search.window)->vbox),hbox,
		      FALSE,TRUE,5);
    gtk_widget_show(hbox);
    search.case_button = gtk_check_button_new_with_label("Case Sensitive");
    gtk_widget_show(search.case_button);
    gtk_box_pack_start(GTK_BOX(hbox),search.case_button,
		      FALSE,FALSE,5);
    search.backwards_button = gtk_check_button_new_with_label("Find Backwards");
    gtk_widget_show(search.backwards_button);
    gtk_box_pack_start(GTK_BOX(hbox),search.backwards_button,
		      FALSE,FALSE,5);
    
    button = gtk_button_new_with_label("Find");
    gtk_widget_show(button);
    gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(search.window)->action_area),button);
    gtk_signal_connect(GTK_OBJECT(button),"clicked",
		       GTK_SIGNAL_FUNC(find_cb),sbaw);
    GTK_WIDGET_SET_FLAGS(button,GTK_CAN_DEFAULT);
    gtk_widget_grab_default(button);
    
    button = gtk_button_new_with_label("Clear");
    gtk_widget_show(button);
    gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(search.window)->action_area),button);
    gtk_signal_connect(GTK_OBJECT(button),"clicked",
		       GTK_SIGNAL_FUNC(find_clear_cb),NULL);
    
    button = gtk_button_new_with_label("Close");
    gtk_widget_show(button);
    gtk_box_pack_start_defaults(GTK_BOX(GTK_DIALOG(search.window)->action_area),button);
    gtk_signal_connect_object(GTK_OBJECT(button),"clicked",
			      GTK_SIGNAL_FUNC(gtk_widget_hide),(GtkObject*)search.window);

  gtk_signal_connect_after(GTK_OBJECT(sbaw->sbw.gui_obj.window), "configure_event",
			   GTK_SIGNAL_FUNC(gui_object_configure_event),sbaw);

    gtk_widget_show(sbaw->sbw.gui_obj.window);

    sbaw->sbw.gui_obj.enabled=1;

    if(sbaw->load_source)
	SourceBrowserAsm_new_source(sbaw,sbaw->sbw.gui_obj.gp);
}


int CreateSourceBrowserAsmWindow(GUI_Processor *gp)
{
  gint i;

  SourceBrowserAsm_Window *sbaw;

  struct breakpoint_info breakpoint_false={-1,NULL};
  
  
  sbaw = (SourceBrowserAsm_Window *) malloc(sizeof(SourceBrowserAsm_Window));

  
  sbaw->sbw.gui_obj.window = NULL;
  sbaw->sbw.gui_obj.gp = gp;
  sbaw->sbw.gui_obj.name = "source_browser";
  sbaw->sbw.gui_obj.wc = WC_source;
  sbaw->sbw.gui_obj.wt = WT_asm_source_window;

  gp->source_browser = (SourceBrowser_Window*)sbaw;
  sbaw->sbw.gui_obj.change_view = SourceBrowser_change_view;


  for(i=0;i<SBAW_NRFILES;i++)
      sbaw->notebook_child[i]=NULL;
  
  for(i=0;i<MAX_BREAKPOINTS;i++)
      sbaw->breakpoint[i] = breakpoint_false;
  sbaw->layout_offset=-1;

    
    
    sbaw->pixmap_pc = NULL; // these are created somewhere else
    

    sbaw->source_loaded = 0;
    
    sbaw->load_source=0;

    gp_add_window_to_list(gp, (GUI_Object *)sbaw);

    gui_object_get_config((GUI_Object*)sbaw);

    if(sbaw->sbw.gui_obj.enabled)
	BuildSourceBrowserAsmWindow(sbaw);

    return 1;
}
#endif // HAVE_GUI
