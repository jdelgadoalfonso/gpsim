/*
   Copyright (C) 1998-2003 T. Scott Dattalo

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
along with gpasm; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#ifndef __PROCESSOR_H__
#define __PROCESSOR_H__
#include <unistd.h>
#include <glib.h>

#include <vector>
#include <list>

#include "gpsim_classes.h"
#include "modules.h"
#include "trace.h"
#include "registers.h"
#include "gpsim_time.h"
#include "gpsim_interface.h"


class Processor;

//---------------------------------------------------------
// The ProgramMemoryAccess class is the interface used
// by objects other than the simulator to manipulate the 
// pic's program memory. For example, the breakpoint class
// modifies program memory when break points are set or
// cleared. The modification goes through here.
//

class ProgramMemoryAccess :  public BreakpointObject
{
 public:
  // Symbolic debugging
  enum HLL_MODES {
    ASM_MODE,      // Source came from plain old .asm files
    HLL_MODE       // Source came from a high level language like C or JAL.
  };


  ProgramMemoryAccess(Processor *new_cpu=0);

  instruction &operator [] (int address);

  void put(int addr, instruction *new_instruction);
  instruction *get(int addr);
  instruction *get_base_instruction(int addr);
  unsigned int get_opcode(int addr);
  char *get_opcode_name(int addrr, char *buffer, int size);
  virtual unsigned int get_PC(void);

  void put_opcode(int addr, unsigned int new_opcode);
  // When a pic is replacing one of it's own instructions, this routine
  // is called.
  void put_opcode_start(int addr, unsigned int new_opcode);

  // Assign a cross reference object to an instruction 
  void assign_xref(unsigned int address, gpointer cross_reference);

  virtual void callback(void);
  void init(Processor *);
  // Helper functions for querying the program memory

  // hasValid_opcode -- returns true if the opcode at the address is valid
  bool hasValid_opcode(unsigned int address);

  // step - step one of more instructions
  virtual void step(unsigned int steps);
  virtual void step_over(void);

  virtual void run(void);
  virtual void stop(void);
  virtual void finish(void);

  // isModified -- returns true if the program at the address has been modified 
  // (this is only valid for those processor capable of writing to their own
  // program memory)
  bool isModified(unsigned int address);


  // Given a file and a line in that file, find the instrucion in the
  // processor's memory that's closest to it.
  virtual int  find_closest_address_to_line(int file_id, int src_line);
  //virtual int  find_closest_address_to_hll_line(int file_id, int src_line);

  // Given an address to an instruction, find the source line that 
  // created it:

  unsigned int get_src_line(unsigned int address);

  // Return the file ID of the source program responsible for the opcode at address.
  unsigned int get_file_id(unsigned int address);

  // A couple of functions for manipulating  breakpoints
  virtual void set_break_at_address(int address);
  virtual void set_notify_at_address(int address, BreakpointObject *cb);
  virtual void set_profile_start_at_address(int address, BreakpointObject *cb);
  virtual void set_profile_stop_at_address(int address, BreakpointObject *cb);
  virtual int clear_break_at_address(int address,enum instruction::INSTRUCTION_TYPES type);
  virtual int clear_notify_at_address(int address);
  virtual int clear_profile_start_at_address(int address);
  virtual int clear_profile_stop_at_address(int address);
  virtual int address_has_break(int address,enum instruction::INSTRUCTION_TYPES type=instruction::BREAKPOINT_INSTRUCTION);
  virtual int address_has_notify(int address);
  virtual int address_has_profile_start(int address);
  virtual int address_has_profile_stop(int address);
  virtual instruction *find_instruction(int address, enum instruction::INSTRUCTION_TYPES type);
  virtual void toggle_break_at_address(int address);
  virtual void set_break_at_line(int file_id, int src_line);
  virtual void clear_break_at_line(int file_id, int src_line);
  virtual void toggle_break_at_line(int file_id, int src_line);

  void set_hll_mode(int);
  enum HLL_MODES get_hll_mode(void) { return hll_mode;}
  bool isHLLmode(void) {return get_hll_mode() == HLL_MODE;}

  string &name(void)
    {
      return name_str;
    }
  void name(string &new_name);
  
  Processor *get_cpu(void);

 private:
  string name_str;            // Optional name if this is one of several pma's

  Processor *cpu;             // The processor to which this pma belongs.

  unsigned int
    _address,
    _opcode, 
    _state;


  enum HLL_MODES hll_mode;


  // breakpoint instruction pointer. This is used by get_base_instruction().
  // If an instruction has a breakpoint set on it, then get_base_instruction
  // will return a pointer to the instruction and will initialize bpi to
  // the breakpoint instruction that has replaced the one in the processor's
  // program memory.
  Breakpoint_Instruction *bpi;


};


//---------------------------------------------------------
// The RegisterMemoryAccess class is the interface used
// by objects other than the simulator to manipulate the 
// cpu's register memory.

class RegisterMemoryAccess
{
 public:
  
  RegisterMemoryAccess(void);
  virtual Register *get_register(unsigned int address);
  int get_size(void) { return nRegisters; }
  void set_cpu(Processor *p);
  void set_Registers(Register **_registers, int _nRegisters);

  bool hasBreak(int address);

  Register &operator [] (int address);

 private:
  int nRegisters;
  bool initialized;
  Register **registers;       // Pointer to the array of registers.
                              // 
  Processor *cpu;             // Pointer to the processor whose registers
                              // are being managed.
};

//------------------------------------------------------------------------
//
/// FileContext - Maintain state information about files.
/// The state of each source file for a processor is recorded in the 
/// FileContext class. Clients can query information like the name
/// of the source file or the line number responsible for generating
/// a specific instruction.

class FileContext
{
 private:
  string name_str;    
  FILE   *fptr;
  vector<int> *line_seek;
  vector<int> *pm_address;
  int    _max_line;

 public:

  FileContext(string &new_name, FILE *_fptr);
  FileContext(char *new_name, FILE *_fptr);
  ~FileContext(void);

  void ReadSource(void);
  char *ReadLine(int line_number, char *buf, int nBytes);
  char *gets(char *buf, int nBytes);
  void rewind(void);
  void open(const char *mode);

  int get_address(int line);
  void put_address(int line, int address);

  string &name(void)
    {
      return name_str;
    }

  void max_line(int new_max_line)
    {
      _max_line = new_max_line;
    }

  int max_line(void)
    {
      return _max_line;
    }

};

class Files
{
 public:

  Files(int nFiles);
  ~Files(void);

  int Add(string& new_name, FILE *fptr);
  int Add(char *new_name, FILE *fptr);

  int Find(string &fname);

  FileContext *operator [] (int file_number);

  void list_id(int new_list_id) 
    {
      list_file_id = new_list_id;
    }

  int list_id(void)
    {
      return list_file_id;
    }

  void nsrc_files(int _num_src_files) 
    {
      num_src_files = _num_src_files;
    }

  int nsrc_files(void) 
    {
      return num_src_files;
    }

  char *ReadLine(int file_id, int line_number, char *buf, int nBytes);
  char *gets(int file_id, char *buf, int nBytes);
  void rewind(int file_id);

 private:
  vector<FileContext *> *vpfile;
  int lastFile;
  int num_src_files;
  int list_file_id;
};

//------------------------------------------------------------------------
//
// Processor - a generic base class for processors supported by gpsim
//

class Processor : public Module
{
public:

  Files *files;               // The source files for this processor.

  double frequency,period;    // Oscillator frequency and period.

  Register **registers;       // 
  Register **register_bank;   //

  instruction   **program_memory;  // THE program memory

  ProgramMemoryAccess  *pma;  // Program memory interface
  RegisterMemoryAccess rma;   // register memory interface
  RegisterMemoryAccess ema;   // eeprom memory interface (if present).

  Program_Counter *pc;

  // Context debugging is a way of debugging the processor while it is
  // in different states. For example, when the interrupt flag is set
  // (for those processors that support interrupts), the processor is
  // in a different 'state' then when the interrupt flag is cleared.

  list<ProgramMemoryAccess *> pma_context;

  //
  // Creation and manipulation of registers
  //

  void create_invalid_registers (void);
  void add_file_registers(unsigned int start_address, 
			  unsigned int end_address, 
			  unsigned int alias_offset);
  void delete_file_registers(unsigned int start_address, 
			     unsigned int end_address);
  void alias_file_registers(unsigned int start_address, 
			    unsigned int end_address, 
			    unsigned int alias_offset);
  virtual void init_register_memory(unsigned int memory_size);
  virtual unsigned int register_memory_size () const { return 0;};
  virtual unsigned int register_size () const { return 1;};

  //
  // Creation and manipulation of Program Memory
  //

  virtual void init_program_memory(unsigned int memory_size);
  virtual void init_program_memory(int address, int value);
  virtual unsigned int program_memory_size(void) const {return 0;};
  void build_program_memory(int *memory,int minaddr, int maxaddr);

  virtual int  map_pm_address2index(int address) {return address;};
  virtual int  map_pm_index2address(int index) {return index;};
  virtual void set_out_of_range_pm(int address, int value);
  guint64 cycles_used(unsigned int address);

  //
  // Symbolic debugging
  //
  // First the source files:

  void attach_src_line(int address,int file_id,int sline,int lst_line);
  void read_src_files(void);


  virtual void dump_registers(void);
  virtual instruction * disasm ( unsigned int address,unsigned int inst)=0;

  virtual bool load_hex(const char *hex_file)=0;

  //
  // Execution control
  //

  virtual void run(void);
  virtual void run_to_address(unsigned int destination);
  virtual void finish(void);

  virtual void sleep(void) {};
  virtual void step(unsigned int steps);
  virtual void step_over(void);
  virtual void step_one(void) = 0;
  virtual void interrupt(void) = 0 ;

  //
  // Processor Clock control
  //

  void set_frequency(double f) { frequency = f; if(f>0) period = 1/f; };
  unsigned int time_to_cycles( double t) 
    {if(period>0) return((int) (frequency * t)); else return 0;};

  virtual void disassemble (int start_address, int end_address);
  virtual void list(int file_id, int pcval, int start_line, int end_line);

  // Configuration control

  virtual void set_config_word(unsigned int address, unsigned int cfg_word) = 0;
  virtual unsigned int config_word_address(void) {return 0;}

  //
  // Processor reset
  // 
  
  virtual void reset(RESET_TYPE r) {};
  virtual void por(void) = 0;            // por = Power On Reset

  //
  // FIXME -- create -- a way of constructing a processor (why not use constructors?)
  //

  virtual void create(void);
  static Processor *construct(void);

  Processor(void);
};


//-------------------------------------------------------------------
//
// ProcessorConstructor -- a class to handle all of gpsim's supported
// processors
//
// gpsim supports dozens of processors. All of these processors are
// grouped together in the ProcessConstructor class. Within the class
// is a static STL list<> object that holds an instance of a
// ProcessorConstructor for each gpsim supported processor. Whenever
// the user selects a processor to simulate, the find() member 
// function will search through the list and find the one that matches
// the user supplied ASCII string.
//
// Why have this class?
// The idea behind this class is that a ProcessorConstructor object 
// can be instantiated for each processor and that instantiation will
// place the object into list of processors. Prior to gpsim-0.21, a
// giant array held the list of all available processors. However,
// there were two problems with this: it was painful to look at and
// it precluded processors that were defined outside of the gpsim
// core library.



class ProcessorConstructor
{
public:
  // THE list of all of gpsim's processors:

  static list <ProcessorConstructor *> *processor_list;

  // A pointer to a function that when called will construct a processor
  Processor * (*cpu_constructor) (void);

  // The processor name (plus upto three aliases).
  #define nProcessorNames 4
  char *names[nProcessorNames];


  //------------------------------------------------------------
  // contructor -- 
  //
  ProcessorConstructor(  Processor * (*_cpu_constructor) (void),
			 char *name1, 
			 char *name2, 
			 char *name3=0,
			 char *name4=0);


  ProcessorConstructor * find(char *name);
  void dump(void);


};

//----------------------------------------------------------
// Global definitions:

#ifdef IN_MODULE
// we are in a module: don't access active_cpu object directly!
Processor *get_active_cpu(void);
#else
// we are in gpsim: use of get_active_cpu() and set_active_cpu() is recommended,
// even if active_cpu object can be accessed directly.
extern Processor *active_cpu;

inline Processor *get_active_cpu(void)
{
  return active_cpu;
}

inline void set_active_cpu(Processor *act_cpu)
{
  active_cpu = act_cpu;
}
#endif


#endif
