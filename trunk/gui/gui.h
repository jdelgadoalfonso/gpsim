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

#ifndef __GUI_H__
#define __GUI_H__

#include "../config.h"

#ifdef HAVE_GUI

#include <unistd.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <gtkextra/gtksheet.h>
#include "../src/interface.h"
#include "../src/gpsim_def.h"
#include "../src/modules.h"
#include "../src/processor.h"
#include "../src/pic-processor.h"

#define SBAW_NRFILES 20 // Max number of source files
//#define MAX_BREAKPOINTS 32

//------------------------------------------------------------
//
// Create structures to generically access the pic-processor
//

//class pic_processor;

//
// Here's a list of all the types of windows that are supported
//

enum window_types {
  WT_register_window,
  WT_status_bar,
  WT_sfr_window,
  WT_watch_window,
  WT_stack_window,
  WT_symbol_window,
  WT_asm_source_window,
  WT_opcode_source_window,
  WT_list_source_window,
  WT_breadboard_window,
  WT_trace_window,
  WT_profile_window,
  WT_stopwatch_window
};

//
// Here's a list of all the categories of windows that are supported
//
enum window_category {
  WC_misc,
  WC_source,
  WC_data
};

//
// This structure will cross reference the data in the simulator
// to its gui representation. There are cases when the same data
// appears in more than one place (e.g. the status register is in
// both the status bar and register windows). gpsim accomodates this
// with a singly-linked list. In other words, for each data element
// that is presented graphically there's a pointer within the simulator
// to reference it. The simulator keeps a linked listed of pointers
// to all instances of these graphical representations

struct cross_reference_to_gui {
  enum window_types parent_window_type;
  gpointer     parent_window;
  gpointer     data;
  void         (*update)  
       (struct cross_reference_to_gui  *_this,int new_value);
  void         (*remove)  
       (struct cross_reference_to_gui  *_this);
};


//
// Forward reference
//

class GUI_Processor;

//========================================================================
// GUI_Object 
//  All window attributes that are common are placed into the GUI_Object
// structure. This structure is then include in each of the other structures.
// It's also the very first item in these 'derived' structures. Consequently a
// pointer to one object may be type cast into another. 
//

class GUI_Object {
 public:

  GUI_Processor *gp;
  bool has_processor;

  GtkWidget *window;
  enum window_category wc;
  enum window_types wt;

  char *name;
  char *menu;

  // Window geometry. This info is saved when the window associated
  // with this gui object is hidden. Note: gtk saves the window origin
  // (x,y) but doesn't save the size (width,height).
  int x,y,width,height;
  int enabled;   // Whether or not the window is up on the screen
  int is_built;  // Whether or not the window is built

  // A pointer to a function that will allow the window associated
  // with this gui object to be viewable or hidden.

#define VIEW_HIDE 0
#define VIEW_SHOW 1
#define VIEW_TOGGLE 2

  GUI_Object(void);
  virtual void ChangeView(int view_state);

  int get_config(void);
  void check(void);
  int set_default_config(void);
  int set_config(void);

  virtual void Build(void);
  virtual int Create(GUI_Processor *_gp);
  virtual void UpdateMenuItem(void);
  virtual void NewProcessor(GUI_Processor *_gp)
    {
      has_processor = true;
    }

};


//
// A 'register' has two attributes as far as the gui is concerned:
//   1) its location and 2) value that is being displayed
//

class GUIRegister {
 public:
  int row;        // row & col in register window
  int col;
  int value;      // value displayed in register window
  gboolean update_full;
};


//
// A 'labeled entry' is an object consisting of gtk entry
// widget that is labeled (with a gtk lable widget) and 
// has information about its parent.
//

struct _labeled_entry {
  GtkWidget *label;
  GtkWidget *entry;

  union {
    gint32    i32;
    guint64   ui64;
    double    db;
  } value;           // value displayed

  gpointer parent;   // a pointer to the owner
  int handle_id;     // unique identifier
};

typedef struct _labeled_entry labeled_entry;


//======================================================================
// The register window 
//
#define MAX_REGISTERS      4096
#define REGISTERS_PER_ROW    16
#define MAX_ROWS ((MAX_REGISTERS)/(REGISTERS_PER_ROW))

class Register_Window : public GUI_Object
{
 public:

  // This array is indexed with row, and gives the address of the
  // first cell in the given row.
  int row_to_address[MAX_ROWS];

  char normalfont_string[256];
#if GTK_MAJOR_VERSION >= 2
  PangoFontDescription *normalfont;
#else
  GdkFont *normalfont;
#endif
  GtkStyle *current_line_number_style;
  GtkStyle *breakpoint_line_number_style;
  GdkColor breakpoint_color;
  GdkColor item_has_changed_color;
  GdkColor normal_fg_color;
  GdkColor normal_bg_color;
  GdkColor sfr_bg_color;
  GdkColor alias_color;
  GdkColor invalid_color;

  REGISTER_TYPE type;
  GUIRegister **registers;
  GtkSheet *register_sheet;
    
  GtkWidget *entry;
  GtkWidget *location;
  GtkWidget *popup_menu;

  int registers_loaded; // non zero when registers array is loaded


  virtual void Build(void);
  virtual void Update(void);
  virtual void UpdateLabel(void);
  virtual void UpdateEntry(void);
  virtual void UpdateLabelEntry(void);
  virtual void SelectRegister(int reg_number);
  virtual void NewProcessor(GUI_Processor *gp);
  virtual gboolean UpdateRegisterCell(unsigned int reg_number);

  Register_Window(GUI_Processor *gp);
  Register_Window(void);
};



class RAM_RegisterWindow : public Register_Window
{
 public:
  RAM_RegisterWindow(GUI_Processor *gp);

};

class EEPROM_RegisterWindow : public Register_Window
{
 public:
  EEPROM_RegisterWindow(GUI_Processor *gp);

};

//
// The watch window
//
class Watch_Window : public  GUI_Object
{
 public:

  GList *watches;

  int current_row;
  int current_column;
    
  GtkWidget *watch_clist;
  GtkWidget *popup_menu;

  Watch_Window(GUI_Processor *gp);
  virtual void Build(void);
  virtual void ClearWatches(void);
  virtual void Add(unsigned int pic_id, REGISTER_TYPE type, int address);
  virtual void Update(void);
  
};

//
// The stack window
//
class Stack_Window : public GUI_Object
{
 public:

  int last_stacklen;
    
  int current_row;
  int current_column;

  GtkWidget *stack_clist;


  Stack_Window(GUI_Processor *gp);
  virtual void Build(void);
  virtual void Update(void);

};



//
// The stopwatch window
//
class StopWatch_Window : public GUI_Object
{
 public:
  int count_dir;

  long long rollover;
  long long cyclecounter;
  long long offset;

  GtkWidget *cycleentry;
  GtkWidget *timeentry;
  GtkWidget *frequencyentry;
  GtkWidget *offsetentry;
  GtkWidget *rolloverentry;

  StopWatch_Window(GUI_Processor *gp);
  virtual void Build(void);
  virtual void Update(void);

};


//
// The symbol window
//
class Symbol_Window : public GUI_Object
{
 public:

  GtkWidget *symbol_clist;
  GList *symbols;
    
  GtkWidget *popup_menu;
  
  int current_row;

  int filter_addresses;
  int filter_constants;
  int filter_registers;

  GtkWidget *addressesbutton;
  GtkWidget *constantsbutton;
  GtkWidget *registersbutton;

  int load_symbols;

  Symbol_Window(GUI_Processor *gp);
  virtual void Build(void);
  virtual void Update(void);
  void NewSymbols(void);
  void SelectSymbolName(char *name);

};


//
// The Status Bar window 
//

class StatusBar_Window {
 public:
  GUI_Processor *gp;

  GtkWidget *popup_menu;
  
  labeled_entry *status;
  labeled_entry *W;
  labeled_entry *pc;
  labeled_entry *cpu_cycles;
  labeled_entry *time;
  
  int created;

  StatusBar_Window(void);
  void NewProcessor(GUI_Processor *_gp);
  void Create(GtkWidget *vbox_main);
  void Update(void);

};


class SourceBrowser_Window : public GUI_Object {
 public:

  GtkWidget *vbox;          // for children to put widgets in

  void Create(void);
  virtual void NewProcessor(GUI_Processor *gp);
  virtual void SelectAddress(int address);
  virtual void Update(void);
  virtual void UpdateLine(int address);
  virtual void SetPC(int address);
  virtual void CloseSource(void){};
  virtual void NewSource(GUI_Processor *gp){};

};

//typedef struct _SourceBrowser_Window SourceBrowser_Window;


struct breakpoint_info {
    int address;
    GtkWidget *widget;
};

// the prefix 'sa' doesn't make sense anymore, FIXME.
struct sa_entry{         // entry in the sa_xlate_list
    int index;           // gtktext index to start of line
    int line;            // line number, first line eq. 0
    int pixel;           // pixels from top of text
    int font_center;     // from base line
};
//
// The Source Assembler Browser Data
//
class SourceBrowserAsm_Window :public  SourceBrowser_Window
{
 public:

  GList *breakpoints;       // List of breakpoint info structs
  GList *notify_start_list; // List of breakpoint info structs
  GList *notify_stop_list;  // List of breakpoint info structs
  int layout_offset;

  // We need one of these for each source file
  GtkAdjustment *source_layout_adj[SBAW_NRFILES];
  GtkWidget *source_layout[SBAW_NRFILES];
  GtkWidget *source_text[SBAW_NRFILES];
  int pageindex_to_fileid[SBAW_NRFILES];
  GtkWidget *source_pcwidget[SBAW_NRFILES];
  GtkWidget *notebook_child[SBAW_NRFILES];

  // Font strings
  char commentfont_string[256];
  char sourcefont_string[256];

  GtkWidget *popup_menu;

  struct sa_entry *menu_data;  // used by menu callbacks
    
  GdkBitmap *pc_mask;
  GdkBitmap *bp_mask;
  GdkBitmap *startp_mask;
  GdkBitmap *stopp_mask;
  GtkWidget *notebook;

  GtkStyle *symbol_text_style;       // for symbols in .asm display
  GtkStyle *label_text_style;        // for label in .asm display
  GtkStyle *instruction_text_style;  // for instruction in .asm display
  GtkStyle *number_text_style;       // for numbers in .asm display
  GtkStyle *comment_text_style;      // for comments in .asm display
  GtkStyle *default_text_style;      // the rest
    
  GdkPixmap *pixmap_pc;
  GdkPixmap *pixmap_break;
  GdkPixmap *pixmap_profile_start;
  GdkPixmap *pixmap_profile_stop;
  int source_loaded;

  int load_source;

  SourceBrowserAsm_Window(GUI_Processor *gp);
  virtual void Build(void);
  virtual void SelectAddress(int address);
  virtual void SetPC(int address);
  virtual void CloseSource(void);
  virtual void NewSource(GUI_Processor *gp);
  virtual void UpdateLine(int address);


};

//
// The Source Opcode Browser Data
//
class SourceBrowserOpcode_Window : public SourceBrowser_Window
{
 public:

  GtkWidget *clist;

  int current_address;   // current PC

  // Font strings
  char normalfont_string[256];
  char breakpointfont_string[256];
  char pcfont_string[256];
  GtkStyle *normal_style;
  GtkStyle *current_line_number_style;
  GtkStyle *breakpoint_line_number_style;
  GdkColor pm_has_changed_color;
  GdkColor normal_pm_bg_color;
  GdkColor breakpoint_color;

  char **column_titles; //
  int  columns;         //

  int program;
    
  GtkWidget *notebook;
  GtkWidget *sheet;
  GtkWidget *entry;
  GtkWidget *label;
  //    GtkWidget *pcwidget;
  GtkWidget *sheet_popup_menu;
  GtkWidget *clist_popup_menu;

  int ascii_mode; // 0, 1 or 2 equals
  // one byte/cell,
  // two bytes/cell MSB first
  // two bytes/cell LSB first
    
  int *memory;

  SourceBrowserOpcode_Window(GUI_Processor *gp);
  virtual void Build(void);
  virtual void NewProcessor(GUI_Processor *gp);
  virtual void SelectAddress(int address);
  virtual void SetPC(int address);
  virtual void NewSource(GUI_Processor *gp);
  virtual void UpdateLine(int address);

};



//
// The Breadboard window data
//

class Breadboard_Window;

enum orientation {LEFT, RIGHT, UP, DOWN};
enum direction {PIN_INPUT, PIN_OUTPUT};
typedef enum {PIN_DIGITAL, PIN_ANALOG, PIN_OTHER} pintype;

// Routing types
typedef enum {R_NONE,R_LEFT, R_RIGHT, R_UP, R_DOWN} route_direction;
typedef struct
{
    int x;
    int y;
} point;
typedef struct _path
{
    point p;
    route_direction dir;
    struct _path *next;
} path;
// End routing types

struct gui_pin
{
    Breadboard_Window *bbw;

    class IOPIN *iopin;

    GtkWidget *widget;
    GdkPixmap *pixmap;
    GdkGC *gc;

    int x;
    int y;
    int width;
    int height;

    int layout_xpos, layout_ypos;

    int value;
    enum direction direction;
    enum orientation orientation;
    pintype type;
};


enum module_type {PIC_MODULE, EXTERNAL_MODULE};

struct gui_module
{

    GtkWidget *fixed; // Main layout that contains everything about the module

    enum module_type type;
    Breadboard_Window *bbw;
    Module *module;
    GtkWidget *module_widget;  // As returned from module. If NULL, it becomes a static GtkPixmap.
    GtkWidget *name_widget;    // Name of widget, positioned above module_widget.
    int x;    // Position in layout widget
    int y;    // Position in layout widget
    int width;  // Width of module_widget
    int height; // Height of module_widget

    int pinnamewidth;

    GdkPixmap *module_pixmap;
    GdkPixmap *name_pixmap;

    GtkWidget *tree_item;

    GList *pins;
};



struct gui_node
{
    Breadboard_Window *bbw;
    Stimulus_Node *node;
    GtkWidget *tree_item;
    int selected_row;

    GList *pins;
};



class Breadboard_Window : public GUI_Object {
 public:

    GdkFont *pinstatefont;
    GdkFont *pinnamefont;
    int pinnameheight;

    GtkWidget *layout;

    GdkGC *pinname_gc;
    GdkGC *pinline_gc;
    GdkGC *case_gc;

    GList *modules;

    GtkWidget *tree;

    GtkWidget *pic_frame;
    GtkWidget *node_frame;
    GtkWidget *module_frame;
    GtkWidget *stimulus_frame;

    GtkWidget *node_tree;

    GtkWidget *node_clist;

    GtkWidget *stimulus_settings_label;

    GtkWidget *stimulus_add_node_button;

    GdkPixmap *layout_pixmap;

    GtkAdjustment *hadj, *vadj;

    struct gui_pin *selected_pin;
    struct gui_node *selected_node;
    struct gui_module *selected_module;


  Breadboard_Window(GUI_Processor *gp);
  virtual void Build(void);
  virtual void NewProcessor(GUI_Processor *gp);
  virtual void Update(void);
  virtual void NewModule(Module *module);
  virtual void NodeConfigurationChanged(Stimulus_Node *node);
};


//
// The trace window 
//

struct TraceMapping {

  guint64 cycle;
  int simulation_trace_index;
};

class Trace_Window : public GUI_Object
{
 public:

  GtkCList *trace_clist;
  guint64   last_cycle;   // The cycle of the last trace in the window.

  GtkWidget *location;
  GtkWidget *popup_menu;

  /* trace_flags bit definitions
     bit0 - enable xref updates to refresh the display
            0 disables, 1 enables
     bit1-bit31 are unused.
   */
  int trace_flags;

  /* trace_map is a pointer to an array of cross references
   * between the trace window and gpsim trace buffer */
  struct TraceMapping *trace_map;
  int trace_map_index;


  Trace_Window(GUI_Processor *gp);
  virtual void Build(void);
  virtual void Update(void);
  virtual void NewProcessor(GUI_Processor *gp);

};




//
// The profile window
//

struct cycle_histogram_counter{
    // Three variables that determine which cycle_histogram_counter we add
    // the differences in cycle counter to:
    unsigned int start_address; // Start profile address
    unsigned int stop_address; // Stop profile address
    guint64 histo_cycles; // The number of cycles that this counter counts.

    int count; // The number of time 'cycles' cycles are used.
};

class Profile_Window : public GUI_Object
{
 public:

  int program;    // if non-zero window has program

  GtkCList *profile_clist;
  GtkCList *profile_range_clist;
  GtkCList *profile_register_clist;
  GtkCList *profile_exestats_clist;
  GList *profile_list;
  GList *profile_range_list;
  GList *profile_register_list;
  GList *profile_exestats_list;
  GtkWidget *notebook;
  gint range_current_row;
  GtkWidget *exestats_popup_menu;
  GtkWidget *range_popup_menu;
  GtkWidget *plot_popup_menu;
  GtkWidget *plot_canvas;

  // List of cycle_count structs
  GList *histogram_profile_list;

  Profile_Window(GUI_Processor *gp);
  virtual void Build(void);
  virtual void Update(void);
  virtual void NewProcessor(GUI_Processor *gp);
  virtual void NewProgram(GUI_Processor *gp);

};



//
//  The gui_processor structure ties the gui window(s)
// to a pic that is being simulated.
//
/*
class WindowList{
 public:
  GUI_Object *gui_object;
  WindowList *next;

  Add
};
*/
class GUI_Processor {
 public:

  GUI_Processor(void);

  RAM_RegisterWindow *regwin_ram;
  EEPROM_RegisterWindow *regwin_eeprom;
  StatusBar_Window *status_bar;
  SourceBrowser_Window *program_memory;
  SourceBrowser_Window *source_browser;
  Symbol_Window *symbol_window;
  Watch_Window *watch_window;
  Stack_Window *stack_window;
  Breadboard_Window *breadboard_window;
  Trace_Window *trace_window;
  Profile_Window *profile_window;
  StopWatch_Window *stopwatch_window;


  // The pic that's associated with the gui
  unsigned int pic_id;
  Processor *cpu;
};


//
// External references and function prototypes
//

extern GtkItemFactory *item_factory;
// gui_processor.c
extern GUI_Processor *gp;

void exit_gpsim(void);

void update_menu_item(GUI_Object *_this);


void SourceBrowser_update_line(struct cross_reference_to_gui *xref, int new_value);


// Configuration -- records window states.

int config_get_variable(char *module, char *entry, int *value);
int config_set_variable(char *module, char *entry, int value);
int config_get_string(char *module, char *entry, char **string);
int config_set_string(char *module, char *entry, const char *string);

gint gui_object_configure_event(GtkWidget *widget, GdkEventConfigure *e, GUI_Object *go);


void ProfileWindow_notify_start_callback(Profile_Window *pw);
void ProfileWindow_notify_stop_callback(Profile_Window *pw);
int gui_get_value(char *prompt);


#if 0
// gui_symbols.c
void SymbolWindow_select_symbol_regnumber(Symbol_Window *sw, int regnumber);
void SymbolWindow_select_symbol_name(Symbol_Window *sw, char *name);
void SymbolWindow_new_symbols(Symbol_Window *sw, GUI_Processor *gp);
int CreateSymbolWindow(GUI_Processor *gp);
int BuildSymbolWindow(Symbol_Window *sw);

// gui_statusbar.c
void StatusBar_create(GtkWidget *vbox_main, StatusBar_Window *sbw);
void StatusBar_update(StatusBar_Window *sbw);
void StatusBar_new_processor(StatusBar_Window *sbw, GUI_Processor *gp);

// gui_src_opcode.c
void SourceBrowserOpcode_select_address(SourceBrowserOpcode_Window *sbow,int address);
void SourceBrowserOpcode_set_pc(SourceBrowserOpcode_Window *sbow, int address);
void SourceBrowserOpcode_new_program(SourceBrowserOpcode_Window *sbow, GUI_Processor *gp);
void SourceBrowserOpcode_new_processor(SourceBrowserOpcode_Window *sbow, GUI_Processor *gp);
int CreateSourceBrowserOpcodeWindow(GUI_Processor *gp);
void BuildSourceBrowserOpcodeWindow(SourceBrowserOpcode_Window *sbow);

// gui_src_asm.c
int CreateSourceBrowserAsmWindow(GUI_Processor *gp);
void SourceBrowserAsm_new_source(SourceBrowserAsm_Window *sbaw, GUI_Processor *gp);
void SourceBrowserAsm_close_source(SourceBrowserAsm_Window *sbaw, GUI_Processor *gp);
void SourceBrowserAsm_set_pc(SourceBrowserAsm_Window *sbaw, int address);
void SourceBrowserAsm_select_address( SourceBrowserAsm_Window *sbaw, int address);
void BuildSourceBrowserAsmWindow(SourceBrowserAsm_Window *sbaw);

// gui_src.c
void SourceBrowser_select_address(SourceBrowser_Window *sbw,int address);
void SourceBrowser_update(SourceBrowser_Window *sbw);
void CreateSBW(SourceBrowser_Window *sbw);
void SourceBrowser_change_view (GUI_Object *_this, int view_state);

// gui_regwin.c
void RegWindow_update(Register_Window *rw);
void RegWindow_select_symbol_name(Register_Window *rw, char *name);
void RegWindow_select_symbol_regnumber(Register_Window *rw, int n);
void RegWindow_select_register(Register_Window *rw, int regnumber);
void RegWindow_new_processor(Register_Window *rw, GUI_Processor *gp);



// gui_watch.c
void WatchWindow_add(Watch_Window *ww, unsigned int pic_id, REGISTER_TYPE type, int address);
int CreateWatchWindow(GUI_Processor *gp);
int BuildWatchWindow(Watch_Window *ww);
void WatchWindow_update(Watch_Window *ww);
void WatchWindow_clear_watches(Watch_Window *ww, GUI_Processor *gp);

// gui_stack.c
int CreateStackWindow(GUI_Processor *gp);
int BuildStackWindow(Stack_Window *sw);
void StackWindow_update(Stack_Window *sw);
void StackWindow_new_processor(Stack_Window *sw, GUI_Processor *gp);

// gui_breadboard.c
void BreadboardWindow_new_processor(Breadboard_Window *bbw, GUI_Processor *gp);
void BreadboardWindow_new_module(Breadboard_Window *bbw, Module *module);
void BreadboardWindow_node_configuration_changed(Breadboard_Window *bbw,Stimulus_Node *node);

int BuildBreadboardWindow(Breadboard_Window *bbw);
int CreateBreadboardWindow(GUI_Processor *gp);
void BreadboardWindow_update(Breadboard_Window *bbw);

// gui_trace.c
void TraceWindow_new_processor(Trace_Window *bbw, GUI_Processor *gp);
int BuildTraceWindow(Trace_Window *bbw);
int CreateTraceWindow(GUI_Processor *gp);
void TraceWindow_update(Trace_Window *bbw);

// gui_profile.c
void ProfileWindow_new_processor(Profile_Window *pw, GUI_Processor *gp);
void ProfileWindow_new_program(Profile_Window *pw, GUI_Processor *gp);
int BuildProfileWindow(Profile_Window *pw);
int CreateProfileWindow(GUI_Processor *gp);
void ProfileWindow_update(Profile_Window *pw);

// gui_stopwatch.c
void StopWatchWindow_new_processor(StopWatch_Window *sww, GUI_Processor *gp);
void StopWatchWindow_new_program(StopWatch_Window *sww, GUI_Processor *gp);
int BuildStopWatchWindow(StopWatch_Window *sww);
int CreateStopWatchWindow(GUI_Processor *gp);
void StopWatchWindow_update(StopWatch_Window *sww);
#endif

#endif // __GUI_H__

#endif // HAVE_GUI
