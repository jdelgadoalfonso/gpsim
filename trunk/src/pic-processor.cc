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
along with gpasm; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#include <stdio.h>
#ifdef _WIN32
#include "uxtime.h"
#endif
#ifndef _MSC_VER
#include <sys/time.h>
#endif
#include <time.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <list>

#include "../config.h"
#include "gpsim_def.h"
#include "pic-processor.h"
#include "pic-registers.h"
#include "picdis.h"
#include "symbol.h"
#include "stimuli.h"
#include "p16x5x.h"
#include "p16f62x.h"
#include "p16x8x.h"
#include "p16f87x.h"
#include "p16x6x.h"
#include "p16x7x.h"
#include "p12x.h"
#include "p17c75x.h"
#include "p18x.h"
#include "icd.h"

#include "fopen-path.h"

guint64 simulation_start_cycle;


//================================================================================
// Global Declarations
//  FIXME -  move these global references somewhere else

// don't print out a bunch of garbage while initializing
int     verbose=0;


//================================================================================
//
// pic_processor
//
// This file contains all (most?) of the code that simulates those features 
// common to all pic microcontrollers.
//
//

//-------------------------------------------------------------------
//
// Define a list for keeping track of the processors being simulated.
// (Recall, gpsim can simultaneously simulate more than one processor.)

list <Processor *> processor_list;
list <Processor *> :: iterator processor_iterator;

// active_cpu_id is the id of the currently active cpu. In other words:
//  active_cpu_id == active_cpu->processor_id
// It's redundant to define this id in addition to the *active_cpu pointer.
// However, if there ever comes a day when the cli is truely separate from
// the simulator, then it would be more convenient to deal with ints than
// pointers.

static  int  active_cpu_id = 0;

// cpu_ids is a counter that increments everytime a processor is added by the
// user. If the user deletes a processor, this counter will not be affected.
// The value of this counter will be assigned to the processor's id when a
// new processor is added. It's purpose is to uniquely identifier user 
// processors.

static  int  cpu_ids = 0;

ProcessorConstructor Generic(pic_processor::construct,
			     "generic_pic", "generic_pic");

ProcessorConstructor pP12C508(P12C508::construct ,
			      "__12C508", "pic12c508",  "p12c508", "12c508");
ProcessorConstructor pP12C509(P12C509::construct ,
			      "__12C509", "pic12c509",  "p12c509", "12c509");
ProcessorConstructor pP12CE518(P12CE518::construct ,
			      "__12ce518", "pic12ce518",  "p12ce518", "12ce518");
ProcessorConstructor pP12CE519(P12CE519::construct ,
			      "__12ce519", "pic12ce519",  "p12ce519", "12ce519");
ProcessorConstructor pP16C84(P16C84::construct ,
			     "__16C84",  "pic16c84",   "p16c84", "16c84");
ProcessorConstructor pP16CR83(P16CR83::construct ,
			      "__16CR83", "pic16cr83",  "p16cr83", "16cr83");
ProcessorConstructor pP16CR84(P16CR84::construct ,
			      "__16CR84", "pic16cr84",  "p16cr84", "16cr84");
ProcessorConstructor pP16F83(P16F83::construct ,
			     "__16F83",   "pic16f83",   "p16f83", "16f83");
ProcessorConstructor pP16F84(P16F84::construct ,
			     "__16F84",   "pic16f84",   "p16f84", "16f84");
ProcessorConstructor pP16C54(P16C54::construct ,
			     "__16C54",   "pic16c54",   "p16c54", "16c54");
ProcessorConstructor pP16C55(P16C55::construct ,
			     "__16C55",   "pic16c55",   "p16c55", "16c55");
ProcessorConstructor pP16C61(P16C61::construct ,
			     "__16C61",   "pic16c61",   "p16c61", "16c61");
ProcessorConstructor pP16C71(P16C71::construct ,
			     "__16C71",   "pic16c71",   "p16c71", "16c71");
ProcessorConstructor pP16C712(P16C712::construct ,
			      "__16C712",  "pic16c712",  "p16c712", "16c712");
ProcessorConstructor pP16C716(P16C716::construct ,
			      "__16C716",  "pic16c716",  "p16c716", "16c716");
ProcessorConstructor pP16C62(P16C62::construct ,
			     "__16C62",   "pic16c62",   "p16c62", "16c62");
ProcessorConstructor pP16C62A(P16C62::construct ,
			      "__16C62A", "pic16c62a",  "p16c62a", "16c62a");
ProcessorConstructor pP16CR62(P16C62::construct ,
			      "__16CR62", "pic16cr62",  "p16cr62", "16cr62");
ProcessorConstructor pP16C63(P16C63::construct ,
			     "__16C63",   "pic16c63",   "p16c63", "16c63");
ProcessorConstructor pP16C64(P16C64::construct ,
			     "__16C64",   "pic16c64",   "p16c64", "16c64");
ProcessorConstructor pP16C65A(P16C65::construct ,
			     "__16C65A", "pic16c65a",  "p16c65a", "16c65a");
ProcessorConstructor pP16C65(P16C65::construct ,
			     "__16C65",   "pic16c65",   "p16c65", "16c65");
ProcessorConstructor pP16C72(P16C72::construct ,
			     "__16C72",   "pic16c72",   "p16c72", "16c72");
ProcessorConstructor pP16C73(P16C73::construct ,
			     "__16C73",   "pic16c73",   "p16c73", "16c73");
ProcessorConstructor pP16C74(P16C74::construct ,
			     "__16C74",   "pic16c74",   "p16c74", "16c74");
ProcessorConstructor pP16F627(P16F627::construct ,
			      "__16F627", "pic16f627",  "p16f627", "16f627");
ProcessorConstructor pP16F628(P16F628::construct ,
			      "__16F628", "pic16f628",  "p16f628", "16f628");
ProcessorConstructor pP16F871(P16F871::construct ,
			      "__16F871", "pic16f871",  "p16f871", "16f871");
ProcessorConstructor pP16F873(P16F873::construct ,
			      "__16F873", "pic16f873",  "p16f873", "16f873");
ProcessorConstructor pP16F874(P16F874::construct ,
			      "__16F874", "pic16f874",  "p16f874", "16f874");
ProcessorConstructor pP16F877(P16F877::construct ,
			      "__16F877", "pic16f877",  "p16f877", "16f877");
ProcessorConstructor pP17C7xx(P17C7xx::construct ,
			      "__17C7xx", "pic17c7xx",  "p17c7xx", "17c7xx");
ProcessorConstructor pP17C75x(P17C75x::construct ,
			      "__17C75x", "pic17c75x",  "p17c75x", "17c75x");
ProcessorConstructor pP17C752(P17C752::construct ,
			      "__17C752", "pic17c752",  "p17c752", "17c752");
ProcessorConstructor pP17C756(P17C756::construct ,
			      "__17C756", "pic17c756",  "p17c756", "17c756");
ProcessorConstructor pP17C756A(P17C756A::construct ,
			       "__17C756A", "pic17c756a",  "p17c756a", "17c756a");
ProcessorConstructor pP17C762(P17C762::construct ,
			      "__17C762", "pic17c762",  "p17c762", "17c762");
ProcessorConstructor pP17C766(P17C766::construct ,
			      "__17C766", "pic17c766",  "p17c766", "17c766");
ProcessorConstructor pP18C242(P18C242::construct ,
			      "__18C242", "pic18c242",  "p18c242", "18c242");
ProcessorConstructor pP18C252(P18C252::construct ,
			      "__18C252", "pic18c252",  "p18c252", "18c252");
ProcessorConstructor pP18C442(P18C442::construct ,
			      "__18C442", "pic18c442",  "p18c442", "18c442");
ProcessorConstructor pP18C452(P18C452::construct ,
			      "__18C452", "pic18c452",  "p18c452", "18c452");
ProcessorConstructor pP18F242(P18F242::construct ,
			      "__18F242", "pic18f242",  "p18f242", "18f242");
ProcessorConstructor pP18F252(P18F252::construct ,
			      "__18F252", "pic18f252",  "p18f252", "18f252");
ProcessorConstructor pP18F442(P18F442::construct ,
			      "__18F442", "pic18f442",  "p18f442", "18f442");
ProcessorConstructor pP18F248(P18F248::construct ,
			      "__18F248", "pic18f248",  "p18f248", "18f248");
ProcessorConstructor pP18F452(P18F452::construct,
			      "__18F452", "pic18f452",  "p18f452", "18f452");
ProcessorConstructor pP18F1220(P18F1220::construct,
			      "__18F1220", "pic18f1220",  "p18f1220", "18f1220");
ProcessorConstructor pP18F1320(P18F1320::construct,
			      "__18F1320", "pic18f1320",  "p18f1320", "18f1320");


//-------------------------------------------------------------------
//
// display_available_processors - list all of the processors gpsim supports.

void display_available_processors(void)
{
  Generic.dump();  // any ProcessorConstructor object could've been called.
}

//-------------------------------------------------------------------
Processor * add_processor(char * processor_type, char * processor_new_name)
{
  if(verbose)
    cout << "Trying to add new processor '" << processor_type << "' named '" 
	 << processor_new_name << "'\n";

  ProcessorConstructor *pc = Generic.find(processor_type);
  if(pc)
    cout << "Found " << pc->names[1] << endl;

  if(pc) {

    Processor *p = pc->cpu_constructor();

    if(p) {

      processor_list.push_back(p);
      active_cpu = p;
      //p->processor_id = 
      active_cpu_id = ++cpu_ids;
      if(verbose) {
	cout << processor_type << '\n';
	cout << "Program Memory size " <<  p->program_memory_size() << '\n';
	cout << "Register Memory size " <<  p->register_memory_size() << '\n';
      }

      // Tell the gui or any modules that are interfaced to gpsim
      // that a new processor has been declared.
      gi.new_processor(p);

      return p;
    }
    else
      cout << " unable to add a processor (BUG?)\n";

  } else
    cout << processor_type << " is not a valid processor.\n(try 'processor list' to see a list of valid processors.\n";

  return(0);
}


//------------------------------------------------------------------------
// dump_processor_list - print out all of the processors a user is 
//                       simulating.

void dump_processor_list(void)
{

  cout << "Processor List\n";

  bool have_processors = 0;

  for (processor_iterator = processor_list.begin();
       processor_iterator != processor_list.end(); 
       processor_iterator++) {

      Processor *p = *processor_iterator;
      cout << p->name() << '\n';
      have_processors = 1;
    }

  if(!have_processors)
    cout << "(empty)\n";

}

//-------------------------------------------------------------------
//
Processor * pic_processor::construct(void)
{

  cout << " Can't create a generic pic processor\n";

  return 0;

}

void pic_processor::set_eeprom(EEPROM *e)
{ 
  eeprom = e; 

  ema.set_cpu(this);
  ema.set_Registers(e->rom, e->rom_size);

}
//-------------------------------------------------------------------
//
// sleep - Begin sleeping and stay asleep until something causes a wake
//

void pic_processor::sleep (void)
{
  simulation_mode = SLEEPING;

  if(!bp.have_sleep())
    return;

  do
    {
      cycles.increment();   // burn cycles until something wakes us
    } while(bp.have_sleep() && !bp.have_halt());

  if(!bp.have_sleep())
    pc->increment();

  simulation_mode = RUNNING;

}

//-------------------------------------------------------------------
//
// sleep - Begin sleeping and stay asleep until something causes a wake
//

void pic_processor::pm_write (void)
{
  //  simulation_mode = PM_WRITE;

  do
    {
      cycles.increment();     // burn cycles until we're through writing
    } while(bp.have_pm_write());

  simulation_mode = RUNNING;

}

int realtime_mode=0;
int realtime_mode_with_gui=0;
extern void update_gui(void);

class RealTimeBreakPoint : public TriggerObject
{
public:
  Processor *cpu;
  struct timeval tv_start;
  guint64 cycle_start;
  guint64 future_cycle;
  int warntimer;
  guint64 period;

  RealTimeBreakPoint(void)
  {
    cpu = 0;
    warntimer = 1;
    period = 1;
    future_cycle = 0;
  }

  void start(Processor *active_cpu) 
  {
    if(!active_cpu)
      return;

    // Grab the system time and record the simulated pic's time.
    // We'll then set a break point a short time in the future
    // and compare how the two track.

    cpu = active_cpu;

    gettimeofday(&tv_start,0);

    cycle_start=cycles.value;

    guint64 fc = cycle_start+100;

    cout << "real time start : " << future_cycle << '\n';

    if(future_cycle)
      cycles.reassign_break(future_cycle, fc, this);
    else
      cycles.set_break(fc, this);

    future_cycle = fc;

  }

  void stop(void)
  {

    // Clear any pending break point.
    cout << "real time stop : " << future_cycle << '\n';

    if(future_cycle) {
      cout << " real time clearing\n";
      cycles.clear_break(this);
      future_cycle = 0;
    }
      
  }

  void callback(void)
  {
    gint64 system_time;
    double diff;
    struct timeval tv;

    // We just hit the break point. A few moments ago we 
    // grabbed a snap shot of the system time and the simulated
    // pic's time. Now we're going to compare the two deltas and
    // see how well they've tracked. If the host is running
    // way faster than the PIC, we'll put the host to sleep 
    // briefly.


    gettimeofday(&tv,0);

    system_time = (tv.tv_sec-tv_start.tv_sec)*1000000+(tv.tv_usec-tv_start.tv_usec); // in micro-seconds

    diff = system_time - ((cycles.value-cycle_start)*4.0e6*cpu->period);

    guint64  idiff;
    if( diff < 0 )
    {
	// we are simulating too fast

        idiff = (guint64)(-diff/4);

	if(idiff>1000)
	    period -= idiff/500;
	if(period<1)
            period=1;

	// Then sleep for a while
	if(idiff)
	  usleep((unsigned int)idiff);
    }
    else
    {
      idiff = (guint64)(diff/4);

	if(idiff>1000)
	    period+=idiff/500;
	if(period>10000)
            period=10000;

	if(idiff>1000000)
	{
	    // we are simulating too slow
	    if(warntimer<10)
		warntimer++;
	    else
	    {
		warntimer=0;
		puts("Processor is too slow for realtime mode!");
	    }
	}
	else
            warntimer=0;
    }

    guint64 delta_cycles= (guint64)(100*period*cpu->frequency/4000000);
    if(delta_cycles<1)
      delta_cycles=1;

    // Look at realtime_mode_with_gui and update the gui if true
    if(realtime_mode_with_gui)
    {
	update_gui();
    }


    guint64 fc = cycles.value + delta_cycles;

    if(future_cycle)
      cycles.reassign_break(future_cycle, fc, this);
    else
      cycles.set_break(fc, this);

    future_cycle = fc;

  }

};

RealTimeBreakPoint realtime_cbp;

//-------------------------------------------------------------------
void pic_processor::save_state()
{
  Processor::save_state();

  if(W)
    W->put_trace_state(W->value);

  if(eeprom)
    eeprom->save_state();

  option_reg.put_trace_state(option_reg.value);
}

//-------------------------------------------------------------------
//
// run  -- Begin simulating and don't stop until there is a break.
//
void pic_processor::run (void)
{
  if(use_icd)
  {
    cout  << "WARNING: gui_refresh is not being called " 
	  <<  __FILE__<<':'<<__LINE__<<endl;

      simulation_mode=RUNNING;
      icd_run();
      while(!icd_stopped())
      {
	//#ifdef HAVE_GUI
	//	  if(use_gui)
	//	      gui_refresh();
	//#endif
      }
      simulation_mode=STOPPED;
      disassemble((signed int)pc->get_value(), (signed int)pc->get_value());
      gi.simulation_has_stopped();
      return;
  }


  if(simulation_mode != STOPPED) {
    if(verbose)
      cout << "Ignoring run request because simulation is not stopped\n";
    return;
  }

  simulation_mode = RUNNING;

  if(realtime_mode)
    realtime_cbp.start(active_cpu);

  // If the first instruction we're simulating is a break point, then ignore it.

  //if(pma->find_instruction(pc->value,instruction::BREAKPOINT_INSTRUCTION)!=0)
  //  if((*pma)[pc->value]->isa() == instruction::BREAKPOINT_INSTRUCTION)
    simulation_start_cycle = cycles.value;

  do {

    step(1,false);

    do {

      program_memory[pc->value]->execute();

    } while(!bp.global_break);

    if(bp.have_interrupt())
      interrupt();

    if(bp.have_sleep())
      sleep();

    if(bp.have_pm_write())
      pm_write();

  } while(!bp.global_break);

  if(realtime_mode)
    realtime_cbp.stop();

  bp.clear_global();
  trace.cycle_counter(cycles.value);
  trace.dump_last_instruction();

  simulation_mode = STOPPED;

  gi.simulation_has_stopped();

}


//-------------------------------------------------------------------
//
// step - Simulate one (or more) instructions. If a breakpoint is set
// at the current PC-> 'step' will go right through it. (That's supposed
// to be a feature.)
//

void pic_processor::step (unsigned int steps, bool refresh)
{

  if(use_icd)
  {
      if(steps!=1)
      {
	  cout << "Can only step one step in ICD mode"<<endl;
      }
      icd_step();
      pc->get_value();
      disassemble((signed int)pc->value, (signed int)pc->value); // FIXME, don't want this in HLL ICD mode.
      gi.simulation_has_stopped();
      return;
  }


  Processor::step(steps,refresh);
}

//-------------------------------------------------------------------
//
// step_over - In most cases, step_over will simulate just one instruction.
// However, if the next instruction is a branching one (e.g. goto, call, 
// return, etc.) then a break point will be set after it and gpsim will
// begin 'running'. This is useful for stepping over time-consuming calls.
//

void pic_processor::step_over (bool refresh)
{

  unsigned int saved_pc = pc->value;

  if(simulation_mode != STOPPED) {
    if(verbose)
      cout << "Ignoring step-over request because simulation is not stopped\n";
    return;
  }

  step(1,refresh); // Try one step

  if( ! ( (pc->value >= saved_pc) && (pc->value <= saved_pc+2) ) )
    {
      if(pma->find_instruction(pc->value,instruction::BREAKPOINT_INSTRUCTION)!=0)
	  return;
	else
	{
	  // Set a break point at the instruction just past the one over which we are stepping
	  unsigned int bp_num = bp.set_execution_break(this, saved_pc + 1);
	  run();
	  bp.clear(bp_num);
	}
    }

  // note that we don't need to tell the gui to update its windows since
  // that is already done by step() or run().

}

//-------------------------------------------------------------------
//
// finish
//
// this method really only applies to processors with stacks.

void pic_processor::finish(void)
{
  if(!stack)
    return;

  run_to_address( stack->contents[stack->pointer-1 & stack->stack_mask]);

}


//-------------------------------------------------------------------
//
// reset - reset the pic based on the desired reset type.
//

void pic_processor::reset (RESET_TYPE r)
{

  if(use_icd)
  {
      puts("RESET");
      icd_reset();
      disassemble((signed int)pc->get_value(), (signed int)pc->get_value());
      gi.simulation_has_stopped();
      return;
  }

  if(r == SOFT_RESET)
    {
      trace.reset(r);
      pc->reset();
      gi.simulation_has_stopped();
      cout << " --- Soft Reset (not fully implemented)\n";
      return;
    }

  cout << " --- Reset\n";

  for(unsigned int i=0; i<register_memory_size(); i++)
    registers[i]->reset(r);


  trace.reset(r);
  pc->reset();
  stack->reset();
  bp.clear_global();

  if(r == POR_RESET)
    {
      status->put_TO(1);
      status->put_PD(1);

      if(verbose) {
	cout << "POR\n";
	if(config_modes) config_modes->print();
      }
      if(config_modes)
	wdt.initialize( config_modes->get_wdt() , nominal_wdt_timeout);
    }
  else if (r==WDT_RESET)
    status->put_TO(0);

  gi.simulation_has_stopped();
}

//-------------------------------------------------------------------
//
// por - power on reset %%% FIX ME %%% needs lots of work...
//

void pic_processor::por(void)
{
  if(config_modes)
    wdt.initialize( config_modes->get_wdt(), nominal_wdt_timeout);

}

//-------------------------------------------------------------------
//
// pic_processor -- constructor
//

pic_processor::pic_processor(void)
{

  if(verbose)
    cout << "pic_processor constructor\n";

  pc = 0;

  files = 0;

  eeprom = 0;
  config_modes = create_ConfigMode();

  set_frequency(DEFAULT_PIC_CLOCK);
  set_ClockCycles_per_Instruction(4);
  pll_factor = 0;

  // Test code for logging to disk:
  trace_log.switch_cpus(this);
}

//-------------------------------------------------------------------
//
// 
//    create
//
//  The purpose of this member function is to 'create' a pic processor.
// Since this is a base class member function, only those things that
// are common to all pics are created.

void pic_processor::create (void)
{

  init_program_memory (program_memory_size());

  init_register_memory (register_memory_size());

  //  create_invalid_registers ();
  create_symbols();
  create_stack();

  // Now, initialize the core stuff:
  pc->set_cpu(this);
  //  cycles.cpu = this;
  wdt.cpu = this;

  W = new WREG(this);
  //  W->set_cpu(this);

  pcl = new PCL;
  pclath = new PCLATH;
  status = new Status_register;
  //FIXME more
  
  W->new_name("W");
  
  indf = new INDF;

  register_bank = &registers[0];  // Define the active register bank 
  W->value.put(0);

  nominal_wdt_timeout = 18e-3;    // 18ms according to the data sheet (no prescale)

  Vdd = 5.0;                      // Assume 5.0 volt power supply

  if(pma) {
    
    rma.SpecialRegisters.push_back(new PCHelper(pma));
    rma.SpecialRegisters.push_back(status);
    rma.SpecialRegisters.push_back(W);

    pma->SpecialRegisters.push_back(new PCHelper(pma));
    pma->SpecialRegisters.push_back(status);
    pma->SpecialRegisters.push_back(W);

  }

}

//-------------------------------------------------------------------
//
// add_sfr_register
//
// The purpose of this routine is to add one special function register
// to the file registers. If the sfr has a physical address (like the 
// status or tmr0 registers) then a pointer to that register will be
// placed in the file register map. 

void pic_processor::add_sfr_register(sfr_register *reg, unsigned int addr,
				      RegisterValue por_value, char *new_name)
{

  reg->set_cpu(this);
  if(addr < register_memory_size())
    {
      registers[addr] = reg;
      registers[addr]->address = addr;
      registers[addr]->alias_mask = 0;
      if(new_name)
        registers[addr]->new_name(new_name);

      registers[addr]->set_write_trace(getWriteTT(addr));
      registers[addr]->set_read_trace(getReadTT(addr));
    }

  reg->value       = por_value;
  reg->por_value   = por_value;
  reg->wdtr_value  = por_value;
  reg->initialize();
}

//-------------------------------------------------------------------
//
// init_program_memory
//
// The purpose of this member function is to allocate memory for the
// pic's code space. The 'memory_size' parameter tells how much memory
// is to be allocated AND it should be an integer of the form of 2^n. 
// If the memory size is not of the form of 2^n, then this routine will
// round up to the next integer that is of the form 2^n.
//   Once the memory has been allocated, this routine will initialize
// it with the 'bad_instruction'. The bad_instruction is an instantiation
// of the instruction class that chokes gpsim if it is executed. Note that
// there is only one instance of 'bad_instruction'.

void pic_processor::init_program_memory (unsigned int memory_size)
{

  if(verbose)
    cout << "Initializing program memory: 0x"<<memory_size<<" words\n";

  // The memory_size_mask is used by the branching instructions 

  pc->memory_size_mask = memory_size - 1;

  Processor::init_program_memory(memory_size);
}

//-------------------------------------------------------------------
//
// Add a symbol table entry for each one of the sfr's
//

void pic_processor::create_symbols (void)
{

  if(verbose)
    cout << __FUNCTION__ << " register memory size = " << register_memory_size() << '\n';

  for(unsigned int i = 0; i<register_memory_size(); i++)
    {
      switch (registers[i]->isa()) {
      case Register::SFR_REGISTER:
	if(!symbol_table.find((char *)registers[i]->name().c_str()))
	  symbol_table.add_register(this, registers[i]);
	break;
      default:
	break;
      }
    }

  // now add a special symbol for W
  symbol_table.add_w(this, W);

  symbol_table.add(new val_symbol(pc));

}


//-------------------------------------------------------------------
void pic_processor::init_program_memory(unsigned int address, unsigned int value)
{


  if(address == config_word_address())
    {
      cout << "** SETTING CONFIG address = 0x"<<hex<< address << "  value = 0x"<<value<<'\n';
      set_config_word(address, value);
    }
  else
    Processor::init_program_memory(address,value);

}

//-------------------------------------------------------------------

void pic_processor::set_config_word(unsigned int address,unsigned int cfg_word)
{
  config_word = cfg_word;

  // Clear all of the configuration bits in config_modes and then
  // reset each of them based on the config bits in cfg_word:
  //config_modes &= ~(CM_WDTE);
  //config_modes |= ( (cfg_word & WDTE) ? CM_WDTE : 0);
  //cout << " setting cfg_word and cfg_modes " << hex << config_word << "  " << config_modes << '\n';

  if((address == config_word_address()) && config_modes)
    config_modes->config_mode = (config_modes->config_mode & ~7) | (cfg_word & 7);

  if(verbose && config_modes)
    config_modes->print();

}




//-------------------------------------------------------------------
//
// load_hex
//
extern int readihex16 (pic_processor *cpu, FILE * file);

bool pic_processor::load_hex (const char *hex_file)
{

  FILE *inputfile = fopen_path (hex_file, "r");

  if(verbose)
    cout << "load hex\n";

  if (inputfile == 0)
    {
      cout << "Error: Couldn't open " << hex_file << '\n';
      return false;
    }

  // assume no configuration word is in the hex file.
  set_config_word(config_word_address(),0xffff);

  if (!readihex16 (this, inputfile)) {
    
    // No errors were found in the hex file.

    fclose (inputfile);

    // Tell the gui that we've got some code.
    gi.new_program(this);

    if(verbose)
      cout << "Configuration word = 0x"  
	   << setw(4) << setfill('0') << get_config_word() << '\n';

    set_frequency(10e6);

    reset(POR_RESET);

    simulation_mode = STOPPED;

    if(verbose)
      cycles.dump_breakpoints();

    return true;
  }

  return false;
}


//-------------------------------------------------------------------
//-------------------------------------------------------------------
//  ConfigMode
//
void ConfigMode::print(void)
{


  if(config_mode & CM_FOSC1x) {
    // Internal Oscillator type processor 

    switch(config_mode& (CM_FOSC0 | CM_FOSC1)) {  // Lower two bits are the clock type
    case 0: cout << "LP"; break;
    case CM_FOSC0: cout << "XT"; break;
    case CM_FOSC1: cout << "Internal RC"; break;
    case (CM_FOSC0|CM_FOSC1): cout << "External RC"; break;

    } 
  }else {
    switch(config_mode& (CM_FOSC0 | CM_FOSC1)) {  // Lower two bits are the clock type
    case 0: cout << "LP"; break;
    case CM_FOSC0: cout << "XT"; break;
    case CM_FOSC1: cout << "HS"; break;
    case (CM_FOSC0|CM_FOSC1): cout << "RC"; break;
    }
  }

  cout << " oscillator\n";

  if(valid_bits & CM_WDTE) 
    cout << " WDT is " << (get_wdt() ? "enabled\n" : "disabled\n");

  if(valid_bits & CM_MCLRE) 
    cout << "MCLR is " << (get_mclre() ? "enabled\n" : "disabled\n");

  if(valid_bits & CM_CP0) {

    if(valid_bits & CM_CP1) {
      cout << "CP0 is " << (get_cp0() ? "high\n" : "low\n");
      cout << "CP1 is " << (get_cp1() ? "high\n" : "low\n");
    } else {

      cout << "code protection is " << (get_cp0() ? "enabled\n" : "disabled\n");

    }
  }


}

//-------------------------------------------------------------------
//-------------------------------------------------------------------
void ProgramMemoryAccess::callback(void)
{

  if(_state)
    {
      _state = 0;
      //cout << __FUNCTION__ << " address= " << address << ", opcode= " << opcode << '\n';
      //cpu->program_memory[address]->opcode = opcode;
      put_opcode(_address,_opcode);
      trace.opcode_write(_address,_opcode);
      bp.clear_pm_write();
    }

}
