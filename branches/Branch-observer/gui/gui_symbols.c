#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>

#include "../config.h"
#ifdef HAVE_GUI

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <glib.h>
#include <string.h>

#include <assert.h>

#include "../src/interface.h"

#include "gui.h"


/*
unsigned int gpsim_reg_has_breakpoint(unsigned int processor_id, unsigned int register_number);
void  gpsim_assign_register_xref(unsigned int processor_id, unsigned int reg_number, gpointer xref);
*/

static void update_symbols(Symbol_Window *sw, GUI_Processor *gp)
{

    char **entry; // 'name', 'type', 'typedata'
    sym *s;
    GList *iter;

    sw->load_symbols=1;
    
    if(!sw->gui_obj.enabled)
	return;
    

    gtk_clist_freeze(GTK_CLIST(sw->symbol_clist));
    
    gtk_clist_clear(GTK_CLIST(sw->symbol_clist));

    // free all old allocations
    for(iter=sw->symbols;iter!=NULL;)
    {
	GList *next=iter->next;
	free((sym*)iter->data);
	g_list_remove(iter,iter->data); // FIXME. I really need a tutorial
	iter=next;
	//	g_list_free_1(sa_xlate_list[id]);  // FIXME, g_list_free() difference?
    }
    sw->symbols=NULL;

    gpsim_symbol_rewind((unsigned int)gp->pic_id);


    // FIXME memory leaks
    while(NULL != (s = gpsim_symbol_iter(gp->pic_id)))
    {
	int row;
	sym *e;
	
	if( (sw->filter_addresses && s->type == SYMBOL_ADDRESS) ||
	    (sw->filter_constants && s->type == SYMBOL_CONSTANT) ||
	    (sw->filter_registers && s->type == SYMBOL_REGISTER) )
            continue;

#define SYMBOL_NR_COLUMNS 3
	entry=(char**)malloc(sizeof(char*)*SYMBOL_NR_COLUMNS);
	
	entry[0]=(char*)malloc(strlen(s->name)+1);
	strcpy(entry[0],s->name);
	entry[1]=(char*)malloc(64);
	switch(s->type)
	{
	case SYMBOL_ADDRESS:
	    strcpy(entry[1],"address");
	    break;
	case SYMBOL_CONSTANT:
	    strcpy(entry[1],"constant");
	    break;
	case SYMBOL_REGISTER:
	    strcpy(entry[1],"register");
	    break;
	case SYMBOL_IOPORT:
	    strcpy(entry[1],"ioport");
	    break;
	default:
	    strcpy(entry[1],"unknown symbol type, add it to regwin.c");
            break;
	}
	entry[2]=(char*)malloc(32);
	if(s->type!=SYMBOL_IOPORT)
	    sprintf(entry[2],"0x%X",s->value);
	else
            strcpy(entry[2],"");
	
	e=(sym*)malloc(sizeof(sym));
	memcpy(e,s,sizeof(sym));
	sw->symbols=g_list_append(sw->symbols,e);

	row=gtk_clist_append(GTK_CLIST(sw->symbol_clist),entry);
	gtk_clist_set_row_data(GTK_CLIST(sw->symbol_clist),row,e);
    }
    gtk_clist_thaw(GTK_CLIST(sw->symbol_clist));
}

static void do_symbol_select(Symbol_Window *sw, sym *e)
{
    // Do what is to be done when a symbol is selected.
    // Except for selecting the symbol row in the symbol_clist
    switch(e->type)
    {
    case SYMBOL_REGISTER:
	RegWindow_select_register(((GUI_Object*)sw)->gp->regwin_ram,e->value);
	break;
    case SYMBOL_ADDRESS:
	SourceBrowser_select_address(((GUI_Object*)sw)->gp->source_browser,e->value);
	SourceBrowser_select_address(((GUI_Object*)sw)->gp->program_memory,e->value);
	break;
    default:
	// symbols that can't be 'selected' (e.g constants)
	break;
    }
}

static gint symbol_list_row_selected(GtkCList *symlist,gint row, gint column,GdkEvent *event, Symbol_Window *sw)
{
    sym *e=(sym*)gtk_clist_get_row_data(symlist,row);
    do_symbol_select(sw,e);
    return 0;
}

/*
 pop up symbol window and select row with regnumber if it exists
 */
void SymbolWindow_select_symbol_regnumber(Symbol_Window *sw, int regnumber)
{
    GList *p;
    
    if(!sw->gui_obj.enabled)
	return;
    
    p=sw->symbols;
    while(p)
    {
	sym *e;
	e=(sym*)p->data;
	if(e->type==SYMBOL_REGISTER && e->value==regnumber)
	{
	    int row;
	    row=gtk_clist_find_row_from_data(GTK_CLIST(sw->symbol_clist),e);
	    if(row!=-1)
	    {
		gtk_clist_select_row(GTK_CLIST(sw->symbol_clist),row,0);
		gtk_clist_moveto(GTK_CLIST(sw->symbol_clist),row,0,0.5,0.5);

		do_symbol_select(sw,e);
	    }
	    break;
	}
	p=p->next;
    }
}

void SymbolWindow_select_symbol_name(Symbol_Window *sw, char *name)
{
    GList *p;
    
    if(name==NULL)
	return;

    if(!sw->gui_obj.enabled)
	return;
    
    p=sw->symbols;
    while(p)
    {
	sym *e;
	e=(sym*)p->data;
	if(!strcmp(e->name,name))
	{
	    int row;
	    row=gtk_clist_find_row_from_data(GTK_CLIST(sw->symbol_clist),e);
	    if(row!=-1)
	    {
		gtk_clist_select_row(GTK_CLIST(sw->symbol_clist),row,0);
		gtk_clist_moveto(GTK_CLIST(sw->symbol_clist),row,0,0.5,0.5);
		
		do_symbol_select(sw,e);
		
	    }
	}
	p=p->next;
    }
}

void SymbolWindow_new_symbols(Symbol_Window *sw, GUI_Processor *gp)
{
    update_symbols(sw,gp);
}

/*
 the function comparing rows of symbol list for sorting
 FIXME this can be improved. When we have equal cells in sort_column
 of the two rows, compare another column instead of returning 'match'.
 */
static gint
symbol_compare_func(GtkCList *clist, gconstpointer ptr1,gconstpointer ptr2)
{
    char *text1, *text2;
    long val1, val2;
    GtkCListRow *row1 = (GtkCListRow *) ptr1;
    GtkCListRow *row2 = (GtkCListRow *) ptr2;
//    char *p;

    switch (row1->cell[clist->sort_column].type)
    {
    case GTK_CELL_TEXT:
	text1 = GTK_CELL_TEXT (row1->cell[clist->sort_column])->text;
	break;
    case GTK_CELL_PIXTEXT:
	text1 = GTK_CELL_PIXTEXT (row1->cell[clist->sort_column])->text;
	break;
    default:
	assert(0);
	break;
    }

    switch (row2->cell[clist->sort_column].type)
    {
    case GTK_CELL_TEXT:
	text2 = GTK_CELL_TEXT (row2->cell[clist->sort_column])->text;
	break;
    case GTK_CELL_PIXTEXT:
	text2 = GTK_CELL_PIXTEXT (row2->cell[clist->sort_column])->text;
	break;
    default:
	assert(0);
	break;
    }

    if (!text2)
	assert(0);
    //	return (text1 != NULL);

    if (!text1)
	assert(0);
    //	return -1;

    if(1==sscanf(text1,"%li",&val1))
    {
	if(1==sscanf(text2,"%li",&val2))
	{
//	    printf("Value %d %d\n",val1,val2);
	    return val1-val2;
	}
    }
    return strcmp(text1,text2);
}

static void symbol_click_column(GtkCList *clist, int column)
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
                        Symbol_Window *sw)
{
    ((GUI_Object *)sw)->change_view((GUI_Object*)sw,VIEW_HIDE);
    return TRUE;
}

static void
toggle_addresses (GtkToggleButton *button, Symbol_Window *sw)
{
    sw->filter_addresses = !sw->filter_addresses;
    config_set_variable(sw->gui_obj.name, "filter_addresses", sw->filter_addresses);
    update_symbols(sw, ((GUI_Object*)sw)->gp);
}

static void
toggle_constants (GtkToggleButton *button, Symbol_Window *sw)
{
    sw->filter_constants = !sw->filter_constants;
    config_set_variable(sw->gui_obj.name, "filter_constants", sw->filter_constants);
    update_symbols(sw, ((GUI_Object*)sw)->gp);
}

static void
toggle_registers (GtkToggleButton *button, Symbol_Window *sw)
{
    sw->filter_registers = !sw->filter_registers;
    config_set_variable(sw->gui_obj.name, "filter_registers", sw->filter_registers);
    update_symbols(sw, ((GUI_Object*)sw)->gp);
}

static char *symbol_titles[3]={"Name","Type","Value"};

int BuildSymbolWindow(Symbol_Window *sw)
{
    GtkWidget *window;
//    GtkWidget *register_sheet;
    GtkWidget *vbox;
  GtkWidget *scrolled_window;
//  GtkWidget *separator;
  GtkWidget *hbox;
  GtkWidget *button;

  int x,y,width,height;
  
  window=sw->gui_obj.window=gtk_window_new(GTK_WINDOW_TOPLEVEL);

  sw->gui_obj.window = window;

  gtk_window_set_title(GTK_WINDOW(sw->gui_obj.window), "Symbol Viewer");
  
  width=((GUI_Object*)sw)->width;
  height=((GUI_Object*)sw)->height;
  x=((GUI_Object*)sw)->x;
  y=((GUI_Object*)sw)->y;
  gtk_window_set_default_size(GTK_WINDOW(sw->gui_obj.window), width,height);
  gtk_widget_set_uposition(GTK_WIDGET(sw->gui_obj.window),x,y);
  
  gtk_signal_connect (GTK_OBJECT (window), "destroy",
		      GTK_SIGNAL_FUNC (gtk_widget_destroyed), &window);
  gtk_signal_connect (GTK_OBJECT (sw->gui_obj.window), "delete_event",
			    GTK_SIGNAL_FUNC(delete_event), (gpointer)sw);

  sw->symbol_clist=gtk_clist_new_with_titles(3,symbol_titles);
  gtk_widget_show(sw->symbol_clist);
  gtk_clist_set_column_auto_resize(GTK_CLIST(sw->symbol_clist),0,TRUE);
  gtk_clist_set_column_auto_resize(GTK_CLIST(sw->symbol_clist),1,TRUE);
  gtk_clist_set_column_auto_resize(GTK_CLIST(sw->symbol_clist),2,TRUE);
  gtk_clist_set_auto_sort(GTK_CLIST(sw->symbol_clist),TRUE);
  gtk_clist_set_compare_func(GTK_CLIST(sw->symbol_clist),
			     (GtkCListCompareFunc)symbol_compare_func);

  gtk_signal_connect(GTK_OBJECT(sw->symbol_clist),"click_column",
		     (GtkSignalFunc)symbol_click_column,NULL);
  gtk_signal_connect(GTK_OBJECT(sw->symbol_clist),"select_row",
		     (GtkSignalFunc)symbol_list_row_selected,sw);

  scrolled_window=gtk_scrolled_window_new(NULL, NULL);
  gtk_widget_show(scrolled_window);

  vbox = gtk_vbox_new(FALSE,1);
  
  gtk_container_add(GTK_CONTAINER(scrolled_window), sw->symbol_clist);
  
  gtk_container_add(GTK_CONTAINER(sw->gui_obj.window),vbox);



  hbox = gtk_hbox_new(FALSE,1);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE,FALSE,0);
  gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);

  button = gtk_check_button_new_with_label ("addresses");
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 5);
  if(sw->filter_addresses)
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), FALSE);
  else
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
  gtk_signal_connect (GTK_OBJECT (button), "toggled",
		      GTK_SIGNAL_FUNC (toggle_addresses), (gpointer)sw);

  
  button = gtk_check_button_new_with_label ("constants");
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 5);
  if(sw->filter_constants)
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), FALSE);
  else
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
  gtk_signal_connect (GTK_OBJECT (button), "toggled",
		      GTK_SIGNAL_FUNC (toggle_constants), (gpointer)sw);

  
  button = gtk_check_button_new_with_label ("registers");
  gtk_box_pack_start (GTK_BOX (hbox), button, TRUE, TRUE, 5);
  if(sw->filter_registers)
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), FALSE);
  else
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
  gtk_signal_connect (GTK_OBJECT (button), "toggled",
		      GTK_SIGNAL_FUNC (toggle_registers), (gpointer)sw);

  gtk_signal_connect_after(GTK_OBJECT(sw->gui_obj.window), "configure_event",
			   GTK_SIGNAL_FUNC(gui_object_configure_event),sw);

  
  gtk_widget_show_all (window);
  
  sw->gui_obj.enabled=1;

  if(sw->load_symbols)
      SymbolWindow_new_symbols(sw, sw->gui_obj.gp);

  return 0;
}

int CreateSymbolWindow(GUI_Processor *gp)
{
//    int i;

  Symbol_Window *symbol_window;

#define MAXROWS  (MAX_REGISTERS/REGISTERS_PER_ROW)
#define MAXCOLS  (REGISTERS_PER_ROW+1)


  symbol_window = malloc(sizeof(Symbol_Window));
  
  symbol_window->gui_obj.gp = gp;
  symbol_window->gui_obj.name = "symbol_viewer";
  symbol_window->gui_obj.wc = WC_misc;
  symbol_window->gui_obj.wt = WT_symbol_window;
  symbol_window->gui_obj.change_view = SourceBrowser_change_view;
  symbol_window->gui_obj.window = NULL;
  gp->symbol_window = symbol_window;

  symbol_window->symbols=NULL;
  symbol_window->filter_addresses=0;
  symbol_window->filter_constants=1;
  symbol_window->filter_registers=0;

  symbol_window->load_symbols=0;
  
  gp_add_window_to_list(gp, (GUI_Object *)symbol_window);

  gui_object_get_config((GUI_Object*)symbol_window);
  config_get_variable(symbol_window->gui_obj.name,"filter_addresses",&symbol_window->filter_addresses);
  config_get_variable(symbol_window->gui_obj.name,"filter_constants",&symbol_window->filter_constants);
  config_get_variable(symbol_window->gui_obj.name,"filter_registers",&symbol_window->filter_registers);

  if(symbol_window->gui_obj.enabled)
      BuildSymbolWindow(symbol_window);
  
  return 1;
}

#endif // HAVE_GUI
