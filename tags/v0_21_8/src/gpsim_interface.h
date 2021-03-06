/*
   Copyright (C) 1998 T. Scott Dattalo

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

#ifndef __GPSIM_INTERFACE_H__
#define __GPSIM_INTERFACE_H__

#include "interface.h"
#include "trigger.h"

class Stimulus;
class Stimulus_Node;
class ISimConsole;

//---------------------------------------------------------------------------
//
//  struct Interface
//
//  Here's a structure containing all of the information for gpsim
// and an external interface like the GUI to communicate. There are
// two major functions. First, a GUI can provide pointers to callback
// functions that the simulator can invoke upon certain conditions.
// For example, if the contents of a register change, the simulator
// can notify the GUI and thus save the gui having to continuously
// poll. (This may occur when the command line interface changes
// something; such changes need to be propogated up to the gui.) 


class Interface {
 public:

  unsigned int interface_id;
  gpointer objectPTR;

  /*
   * UpdateObject - Invoked when an object changes
   *
   * If an object, like the contents of a register, changes then this function
   * will be called. There are two parameters:
   *  xref - this is a pointer to some structure in the client's data space.
   *  new_value - this is the new value to which the object has changed.
   *
   */

  virtual void UpdateObject (gpointer xref,int new_value){};

  /*
   * remove_object - Invoked when gpsim has removed something.
   *
   * If an object, like a register, is deleted then this function 
   * will be called. There is one parameter:
   *  xref - this is a pointer to some structure in the client's data space.
   *
   */

  virtual void RemoveObject (gpointer xref) {};


  /*
   * simulation_has_stopped - invoked when gpsim has stopped simulating (e.g.
   *                          when a breakpoint is encountered).
   *
   * Some interfaces have more than one instance, so the 'object' parameter
   * allows the interface to uniquely identify the particular one.
   */

  virtual void SimulationHasStopped (gpointer object) {};


  /*
   * new_processor - Invoked when a new processor is added to gpsim
   */

  virtual void NewProcessor (Processor *new_cpu) {};


  /*
   * new_module - Invoked when a new module is instantiated.
   *
   */

  virtual void NewModule (Module *module) {};

  /*
   * node_configuration_changed - invoked when stimulus configuration has changed
   */

  virtual void NodeConfigurationChanged (Stimulus_Node *node) {};


  /*
   * new_program - Invoked when a new program is loaded into gpsim
   *
   */

  virtual void NewProgram  (Processor *new_cpu) { };

  /*
   * Update - Invoked when the interface should update itself
   */

  virtual void Update  (gpointer object) {};

  /*
   * destructor - called when the interface is destroyed - this gives
   *  the interface object a chance to save state information.
   */
  virtual ~Interface() 
  {
  }

  unsigned int get_id(void) { return interface_id;};
  void set_id(unsigned int new_id) { interface_id = new_id;};
  Interface(gpointer new_object=NULL);
};


class gpsimInterface : public TriggerObject {
 public:

  gpsimInterface(void);

  void start_simulation (void);
  void reset (void);
  void simulation_has_stopped (void);
  bool bSimulating();
  bool bUsingGUI();
  void setGUImode(bool);
  // gpsim will call these functions to notify gui and/or modules
  // that something has changed. 

  void update_object (gpointer xref,int new_value);
  void remove_object (gpointer xref);
  void update (void);
  void new_processor (Processor *);
  void new_module  (Module *module);
  void node_configuration_changed  (Stimulus_Node *node);
  void new_program  (Processor *);
  void set_update_rate(guint64 rate);
  guint64 get_update_rate();

  unsigned int add_interface(Interface *new_interface);
  unsigned int add_socket_interface(Interface *new_interface);
  Interface *get_socket_interface() { return socket_interface;}
  void remove_interface(unsigned int interface_id);


  virtual bool set_break(void) {return false;}
  virtual void callback(void);
  virtual void callback_print(void);
  virtual void print(void);
  virtual void clear(void);
  virtual char const * bpName() { return "gpsim interface"; }
  virtual ISimConsole &GetConsole();

private:
  GSList *interfaces;
  Interface *socket_interface;

  unsigned int interface_seq_number;

  guint64 update_rate;
  guint64 future_cycle;

  bool mbSimulating;   // Set true if the simulation is running.
  bool mbUseGUI;       // Set true if gui is being used.
};

#if defined(IN_MODULE) && defined(_WIN32)
// we are in a module: don't access gi object directly!
gpsimInterface &get_interface(void);
#else
// we are in gpsim: use of get_interface() is recommended,
// even if gi object can be accessed directly.
extern gpsimInterface gi;

inline gpsimInterface &get_interface(void)
{
  return gi;
}
#endif

//------------------------------------------------------------------------

class Module;

class ModuleInterface
{

 public:
  Module *module;  // The module we're interfacing with.


  ModuleInterface(Module *new_module);


};

//------------------------------------------------------------------------

class Processor;

class ProcessorInterface : public ModuleInterface
{
 public:

  ProcessorInterface(Processor *cpu);

};

class ISimConsole;

#define CMD_ERR_OK                    0
#define CMD_ERR_ABORTED               1
#define CMD_ERR_ERROR                 2
#define CMD_ERR_PROCESSORDEFINED      3
#define CMD_ERR_PROCESSORNOTDEFINED   4
#define CMD_ERR_COMMANDNOTDEFINED     5

#define GPSIM_GETCOMMANDHANDLER "GetCommandHandler"
typedef ICommandHandler * (*PFNGETCOMMANDHANDLER)(void);

class ICommandHandler {
public:
  virtual char *GetName(void) = 0;
  virtual int Execute(const char * commandline, ISimConsole *out) = 0;
};

#endif // __GPSIM_INTERFACE_H__
