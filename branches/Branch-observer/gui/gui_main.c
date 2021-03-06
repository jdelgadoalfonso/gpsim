

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../config.h"
#ifdef HAVE_GUI

#include <gtk/gtk.h>

#include <gdk/gdktypes.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>

#include "../src/gpsim_def.h"

//extern pic_processor *get_processor(unsigned int processor_id);

#include "gui.h"
#include "gui_interface.h"

#include "eXdbm/eXdbm.h"
// #include <eXdbm.h>

/*
 * --- Function prototypes
 */
void redisplay_prompt(void);

void gui_styles_init(void);
void create_dispatcher (void);

void init_link_to_gpsim(GUI_Processor *gp);
void link_src_to_gpsim(GUI_Processor *gp);

/* 
 * --- Global variables
 */

GUI_Processor *gp=NULL;
GSList *gui_processors=NULL;


DB_ID dbid=-1;

extern GtkWidget *dispatcher_window;

/*------------------------------------------------------------------
 * quit_gui
 *
 */
void quit_gui(void)
{
    int x,y,width,height;

    gdk_window_get_root_origin(dispatcher_window->window,&x,&y);
    gdk_window_get_size(dispatcher_window->window,&width,&height);

    config_set_variable("dispatcher", "x", x);
    config_set_variable("dispatcher", "y", y);
    config_set_variable("dispatcher", "width", width);
    config_set_variable("dispatcher", "height", height);

    gtk_main_quit();
}

/*------------------------------------------------------------------
 * gui_new_processor - Add a new processor to the register viewer
 *
 * This routine adds another pic processor to the list of currently
 * simulated processors (as of 0.0.14 though, you're still limited
 * to a list of one). It then notifies each child window. Finally
 * a communication link between the gui and the simulator is established.
 * (This was a corba link, but now it consists of direct calls...)
 */

void gui_new_processor (unsigned int pic_id)
{

  // printf("gui is adding a new processor\n");

  // Create an gui representation of the new processor
  /*
  pic_processor *p;

  p = get_processor(pic_id);

  gp->p = p;
  p->gp = gp;

  gp->pic_id = pic_id;
  */
  // Add it to the list 

  if(gp)
    {
      gp->pic_id = pic_id;
      gui_processors = g_slist_append(gui_processors,gp);

      RegWindow_new_processor(gp->regwin_ram, gp);
      StatusBar_new_processor(gp->status_bar, gp);
      SourceBrowserOpcode_new_processor((SourceBrowserOpcode_Window*)gp->program_memory, gp);
      SourceBrowserAsm_new_processor((SourceBrowserAsm_Window*)gp->source_browser, gp);
      BreadboardWindow_new_processor((Breadboard_Window*)gp->breadboard_window, gp);

      init_link_to_gpsim(gp);
      //  redisplay_prompt();
    }

}

/*------------------------------------------------------------------
 *
 */
void gui_new_program (unsigned int pic_id)
{

  // FIX ME - need to search for *p in the gp list...
  if(gp)
  {

      // this is here because the eeprom is not set to values in cod
      // when gui_new_processor is run. eeprom is with program memory data
      RegWindow_new_processor(gp->regwin_eeprom, gp);

      
      SourceBrowserOpcode_new_program((SourceBrowserOpcode_Window*)gp->program_memory, gp);
      link_src_to_gpsim( gp);
      //      redisplay_prompt();
    }
}

/*------------------------------------------------------------------
 *
 */
void gui_new_source (unsigned int pic_id)
{

  // FIX ME - need to search for *p in the gp list...
  if(gp)
  {
	
      SourceBrowserOpcode_new_program((SourceBrowserOpcode_Window*)gp->program_memory, gp);
      SourceBrowserAsm_new_source((SourceBrowserAsm_Window*)gp->source_browser, gp);
      SymbolWindow_new_symbols(gp->symbol_window, gp);

      link_src_to_gpsim( gp);
      //      redisplay_prompt();

    }
}

/*------------------------------------------------------------------
 * update_program_memory
 *
 */
void update_program_memory(GUI_Processor *gp, unsigned int reg_number)
{

    //  printf("program memory needs to be updated\n");

}
/*------------------------------------------------------------------
 * gui_update_object
 *
 * Each 'thing' that the gui displays about a simulated pic has an
 * associated cross reference structure. Sometimes these 'things' 
 * displayed in more than one place (like the status register).
 * Each graphical instance has its own structure. All of the structures
 * pertaining to the same pic object (again, like the status register)
 * are stored in a singly-linked list. This routine scans through
 * this list and updates each instance of the object.
 */

void gui_update_object(gpointer gui_xref,int new_value)
{

  struct cross_reference_to_gui *xref;

  xref = (struct cross_reference_to_gui *)gui_xref;
  xref->update(xref,new_value);

}

/*------------------------------------------------------------------
 * gui_remove_object
 *
 */

void gui_remove_object(gpointer gui_xref)
{

  struct cross_reference_to_gui *xref;

  xref = (struct cross_reference_to_gui *)gui_xref;
  if(xref->remove)
    xref->remove(xref);

}

int config_set_variable(char *module, char *entry, int value)
{
    int ret;
    DB_LIST list;

    list = eXdbmGetList(dbid, NULL, module);
    if(list==NULL)
    {
	ret = eXdbmCreateList(dbid, NULL, module, NULL);
	if(ret==-1)
	{
	    puts(eXdbmGetErrorString(eXdbmGetLastError()));
	    return 0;
	}
	
	list = eXdbmGetList(dbid, NULL, module);
	if(list==NULL)
	{
	    puts(eXdbmGetErrorString(eXdbmGetLastError()));
	    return 0;
	}
    }

    // We have the list
    
    ret = eXdbmChangeVarInt(dbid, list, entry, value);
    if(ret == -1)
    {
	ret = eXdbmCreateVarInt(dbid, list, entry, NULL, value);
	if(ret==-1)
	{
	    puts("\n\n\n\ndidn't work");
	    puts(eXdbmGetErrorString(eXdbmGetLastError()));
	    puts("\n\n\n\n");
	    return 0;
	}
    }
    ret=eXdbmUpdateDatabase(dbid);
    if(ret==-1)
    {
	puts(eXdbmGetErrorString(eXdbmGetLastError()));
	return 0;
    }
    return 1;
}

int gui_object_set_config(GUI_Object *obj)
{
  if(!obj)
    return 0;

    config_set_variable(obj->name, "enabled", obj->enabled);
    config_set_variable(obj->name, "x", obj->x);
    config_set_variable(obj->name, "y", obj->y);
    config_set_variable(obj->name, "width", obj->width);
    config_set_variable(obj->name, "height", obj->height);
    return 1;
}

int config_get_variable(char *module, char *entry, int *value)
{
    int ret;
    DB_LIST list;

    list = eXdbmGetList(dbid, NULL, module);
    if(list==NULL)
	return 0;

    // We have the list
    
    ret = eXdbmGetVarInt(dbid, list, entry, value);
    if(ret == -1)
	return 0;
    
    return 1;
}


int gui_object_get_config(GUI_Object *obj)
{
  if(!obj)
    return 0;

    if(!config_get_variable(obj->name, "enabled", &obj->enabled))
	obj->enabled=0;
    if(!config_get_variable(obj->name, "x", &obj->x))
	obj->x=10;
    if(!config_get_variable(obj->name, "y", &obj->y))
	obj->y=10;
    if(!config_get_variable(obj->name, "width", &obj->width))
	obj->width=300;
    if(!config_get_variable(obj->name, "height", &obj->height))
	obj->height=100;
    return 1;
}


/*------------------------------------------------------------------
 * gui_init
 *
 */

int gui_init (int argc, char **argv)
{
    int ret;

    char path[256], *homedir;
    
  ret = eXdbmInit();
  if(ret==-1)
  {
      puts(eXdbmGetErrorString(eXdbmGetLastError()));
  }

  homedir=getenv("HOME");
  if(homedir==NULL)
      homedir=".";

  sprintf(path,"%s/.gpsim",homedir);
  
  ret = eXdbmOpenDatabase(path,&dbid);
  if(ret==-1)
  {
      int error=eXdbmGetLastError();
      if(error==DBM_OPEN_FILE)
      {
	  ret=eXdbmNewDatabase(path,&dbid);
	  if(ret==-1)
	      puts(eXdbmGetErrorString(eXdbmGetLastError()));
	  else
	  {
	      ret=eXdbmUpdateDatabase(dbid);
	      if(ret==-1)
		  puts(eXdbmGetErrorString(eXdbmGetLastError()));
	  }
      }
      else
	  puts(eXdbmGetErrorString(eXdbmGetLastError()));
  }


  gtk_init (&argc, &argv);

    

  //  gp = (GUI_Processor *)malloc(sizeof(GUI_Processor));
  //  gp->windows = g_list_alloc();

  gp = new_GUI_Processor();

  gui_styles_init();
  
  create_dispatcher();

  CreateRegisterWindow(gp, REGISTER_RAM);
  CreateRegisterWindow(gp, REGISTER_EEPROM);
  CreateSourceBrowserOpcodeWindow(gp);
  CreateSourceBrowserAsmWindow(gp);
  CreateSymbolWindow(gp);
  CreateWatchWindow(gp);
  CreateBreadboardWindow(gp);

  update_object = gui_update_object;
  remove_object = gui_remove_object;
  new_processor = gui_new_processor;
  simulation_has_stopped = gui_simulation_has_stopped;
  new_program = gui_new_program;

  return(0);
}

void gui_main(void)
{
  redisplay_prompt();
  gtk_main();

}

#endif //HAVE_GUI
