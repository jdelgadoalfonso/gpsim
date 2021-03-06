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
#include "unistd.h"
#else
#include <unistd.h>
#endif
#ifndef _MSC_VER
#include <sys/time.h>
#endif
#include <time.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <map>

#include "exports.h"
#include "gpsim_def.h"
#include "pic-processor.h"
#include "pic-registers.h"
#include "picdis.h"
#include "symbol.h"
#include "stimuli.h"
#include "p16x5x.h"
#include "p16f62x.h"
#include "p16f8x.h"
#include "p16x8x.h"
#include "p16f87x.h"
#include "p16x6x.h"
#include "p16x7x.h"
#include "p12x.h"
#ifdef P17C7XX	// code no longer works
#include "p17c75x.h"
#endif
#include "p18x.h"
#include "icd.h"

#include "fopen-path.h"

guint64 simulation_start_cycle;

#include "cod.h"


//================================================================================
// Global Declarations
//  FIXME -  move these global references somewhere else

// don't print out a bunch of garbage while initializing


//================================================================================
//
// pic_processor
//
// This file contains all (most?) of the code that simulates those features 
// common to all pic microcontrollers.
//
//

ProcessorConstructor pP10F200(P10F200::construct ,
			      "__10F200", "pic10f200",  "p10f200", "10f200");
ProcessorConstructor pP10F202(P10F202::construct ,
			      "__10F202", "pic10f202",  "p10f202", "10f202");
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
ProcessorConstructor pP16C56(P16C56::construct ,
			     "__16C56",   "pic16c56",   "p16c56", "16c56");
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
ProcessorConstructor pP16F73(P16F73::construct ,
			     "__16F73",   "pic16f73",   "p16f73", "16f73");
ProcessorConstructor pP16F74(P16F74::construct ,
			     "__16F74",   "pic16f74",   "p16f74", "16f74");
ProcessorConstructor pP16F627(P16F627::construct ,
			      "__16F627", "pic16f627",  "p16f627", "16f627");
ProcessorConstructor pP16F627A(P16F627::construct ,
			      "__16F627A", "pic16f627a",  "p16f627a", "16f627a");
ProcessorConstructor pP16F628(P16F628::construct ,
			      "__16F628", "pic16f628",  "p16f628", "16f628");
ProcessorConstructor pP16F628A(P16F628::construct ,
			      "__16F628A", "pic16f628a",  "p16f628a", "16f628a");
ProcessorConstructor pP16F648(P16F648::construct ,
			      "__16F648", "pic16f648",  "p16f648", "16f648");
ProcessorConstructor pP16F648A(P16F648::construct ,
			      "__16F648A", "pic16f648a",  "p16f648a", "16f648a");
ProcessorConstructor pP16F87(P16F87::construct ,
			      "__16F87", "pic16f87",  "p16f87", "16f87");
ProcessorConstructor pP16F88(P16F88::construct ,
			      "__16F88", "pic16f88",  "p16f88", "16f88");
ProcessorConstructor pP16F871(P16F871::construct ,
			      "__16F871", "pic16f871",  "p16f871", "16f871");
ProcessorConstructor pP16F873(P16F873::construct ,
			      "__16F873", "pic16f873",  "p16f873", "16f873");
ProcessorConstructor pP16F874(P16F874::construct ,
			      "__16F874", "pic16f874",  "p16f874", "16f874");
ProcessorConstructor pP16F876(P16F876::construct ,
			      "__16F876", "pic16f876",  "p16f876", "16f876");
ProcessorConstructor pP16F877(P16F877::construct ,
			      "__16F877", "pic16f877",  "p16f877", "16f877");
ProcessorConstructor pP16F873A(P16F873A::construct ,
			      "__16F873a", "pic16f873a", "p16f873a", "16f873a");
ProcessorConstructor pP16F874A(P16F874A::construct ,
			      "__16F874a", "pic16f874a", "p16f874a", "16f874a");
ProcessorConstructor pP16F876A(P16F876A::construct ,
			      "__16F876a", "pic16f876a", "p16f876a", "16f876a");
ProcessorConstructor pP16F877A(P16F877A::construct ,
			      "__16F877a", "pic16f877a", "p16f877a", "16f877a");
#ifdef P17C7XX	// code no longer works
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
#endif // P17C7XX
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
ProcessorConstructor pP18F448(P18F448::construct ,
			      "__18F448", "pic18f448",  "p18f448", "18f448");
ProcessorConstructor pP18F452(P18F452::construct,
			      "__18F452", "pic18f452",  "p18f452", "18f452");
ProcessorConstructor pP18F1220(P18F1220::construct,
			      "__18F1220", "pic18f1220",  "p18f1220", "18f1220");
ProcessorConstructor pP18F1320(P18F1320::construct,
			      "__18F1320", "pic18f1320",  "p18f1320", "18f1320");


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

void pic_processor::sleep ()
{
  simulation_mode = eSM_SLEEPING;

  if(!bp.have_sleep())
    return;

  do
    {
      get_cycles().increment();   // burn cycles until something wakes us
    } while(bp.have_sleep() && !bp.have_halt());

  if(!bp.have_sleep())
    pc->increment();

  simulation_mode = eSM_RUNNING;

}
//-------------------------------------------------------------------
//
// enter_sleep - The processor is about to go to sleep, so update 
//  the status register.

void pic_processor::enter_sleep()
{
  status->put_TO(1);
  status->put_PD(0);
}

//-------------------------------------------------------------------
//
// pm_write - program memory write
//

void pic_processor::pm_write ()
{
  //  simulation_mode = PM_WRITE;

  do
    {
      get_cycles().increment();     // burn cycles until we're through writing
    } while(bp.have_pm_write());

  simulation_mode = eSM_RUNNING;

}

static bool realtime_mode = false;
static bool realtime_mode_with_gui = false;

void EnableRealTimeMode(bool bEnable) {
  realtime_mode = bEnable;
}

void EnableRealTimeModeWithGui(bool bEnable) {
  realtime_mode_with_gui = bEnable;
}

extern void update_gui();

class RealTimeBreakPoint : public TriggerObject
{
public:
  Processor *cpu;
  struct timeval tv_start;
  guint64 cycle_start;
  guint64 future_cycle;
  int warntimer;
  guint64 period;

  RealTimeBreakPoint()
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

    cycle_start=get_cycles().value;

    guint64 fc = cycle_start+100;

    cout << "real time start : " << future_cycle << '\n';

    if(future_cycle)
      get_cycles().reassign_break(future_cycle, fc, this);
    else
      get_cycles().set_break(fc, this);

    future_cycle = fc;

  }

  void stop()
  {

    // Clear any pending break point.
    cout << "real time stop : " << future_cycle << '\n';

    if(future_cycle) {
      cout << " real time clearing\n";
      get_cycles().clear_break(this);
      future_cycle = 0;
    }
      
  }

  void callback()
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

    diff = system_time - ((get_cycles().value-cycle_start)*4.0e6*cpu->get_OSCperiod());

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

    guint64 delta_cycles= (guint64)(100*period*cpu->get_frequency()/4000000);
    if(delta_cycles<1)
      delta_cycles=1;

    // Look at realtime_mode_with_gui and update the gui if true
    if(realtime_mode_with_gui)
    {
	update_gui();
    }


    guint64 fc = get_cycles().value + delta_cycles;

    if(future_cycle)
      get_cycles().reassign_break(future_cycle, fc, this);
    else
      get_cycles().set_break(fc, this);

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
void pic_processor::run (bool refresh)
{
  if(get_use_icd())
  {
    cout  << "WARNING: gui_refresh is not being called " 
	  <<  __FILE__<<':'<<__LINE__<<endl;

      simulation_mode = eSM_RUNNING;
      icd_run();
      while(!icd_stopped())
      {
	//#ifdef HAVE_GUI
	//	  if(use_gui)
	//	      gui_refresh();
	//#endif
      }
      simulation_mode=eSM_STOPPED;
      disassemble((signed int)pc->get_value(), (signed int)pc->get_value());
      gi.simulation_has_stopped();
      return;
  }


  if(simulation_mode != eSM_STOPPED) {
    if(verbose)
      cout << "Ignoring run request because simulation is not stopped\n";
    return;
  }

  simulation_mode = eSM_RUNNING;

  if(realtime_mode)
    realtime_cbp.start(active_cpu);

  // If the first instruction we're simulating is a break point, then ignore it.

  simulation_start_cycle = get_cycles().value;

  do {

    // Take one step to get past any break point.
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

    if(bp.have_socket_break()) {
      cout << " socket break point \n";
      Interface *i = gi.get_socket_interface();
      if (i)
	i->Update(0);
      bp.clear_socket_break();
    }

  } while(!bp.global_break);

  if(realtime_mode)
    realtime_cbp.stop();

  bp.clear_global();
  trace.cycle_counter(get_cycles().value);

  simulation_mode = eSM_STOPPED;

  if(refresh) {
    trace.dump_last_instruction(); 
    gi.simulation_has_stopped();
  }


}


//-------------------------------------------------------------------
//
// step - Simulate one (or more) instructions. If a breakpoint is set
// at the current PC-> 'step' will go right through it. (That's supposed
// to be a feature.)
//

void pic_processor::step (unsigned int steps, bool refresh)
{

  if(get_use_icd())
  {
      if(steps!=1)
      {
	  cout << "Can only step one step in ICD mode"<<endl;
      }
      icd_step();
      pc->get_value();
      disassemble((signed int)pc->value, (signed int)pc->value); // FIXME, don't want this in HLL ICD mode.
      if(refresh)
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

  if(simulation_mode != eSM_STOPPED) {
    if(verbose)
      cout << "Ignoring step-over request because simulation is not stopped\n";
    return;
  }

  unsigned int saved_pc = pma->get_PC();
  instruction *nextInstruction = pma->getFromAddress(saved_pc);
  if (!nextInstruction) {
    // this is really fatal...
    return;
  }
  unsigned int nextExpected_pc = 
    saved_pc + map_pm_index2address(nextInstruction->instruction_size());

  step(1,refresh); // Try one step

  // if the pc did not advance just one instruction, then some kind of branch occurred.

  unsigned int current_pc = pma->get_PC();
  if( ! (current_pc >= saved_pc && current_pc <= nextExpected_pc)) {

    // If the branch is not a skip instruction then we'll set a break point and run.
    // (note, the test that's performed will treat a goto $+2 as a skip.
    
    instruction *nextNextInstruction = pma->getFromAddress(nextExpected_pc);
    unsigned int nextNextExpected_pc = nextExpected_pc + 
      (nextNextInstruction ? map_pm_index2address(nextNextInstruction->instruction_size()) : 0);

    if (! (current_pc >= saved_pc && current_pc <= nextNextExpected_pc)) {
    
      unsigned int bp_num = pma->set_break_at_address(nextExpected_pc);
      if (bp_num != INVALID_VALUE) {
	run();
	bp.clear(bp_num);
      }
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

void pic_processor::finish()
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
  bool bHaltSimulation = true;

  if(get_use_icd())
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

  for(unsigned int i=0; i<register_memory_size(); i++)
    if (registers[i])
      registers[i]->reset(r);


  trace.reset(r);
  pc->reset();
  stack->reset();
  bp.clear_global();

  switch (r) {
  case POR_RESET:
    status->put_TO(1);
    status->put_PD(1);

    if(verbose) {
      cout << "POR\n";
      if(config_modes) config_modes->print();
    }
    wdt.reset();

    bHaltSimulation = false;
    break;
  case WDT_RESET:
    status->put_TO(0);
    break;
  default:
    break;
  }

  if(bHaltSimulation || getBreakOnReset())
    bp.halt();

  gi.simulation_has_stopped();
}

//-------------------------------------------------------------------
//
// por - power on reset %%% FIX ME %%% needs lots of work...
//

void pic_processor::por()
{
  wdt.reset();

}

//-------------------------------------------------------------------
//
// pic_processor -- constructor
//

pic_processor::pic_processor(const char *_name, const char *_desc)
  : Processor(_name,_desc),
    wdt(this, 18.0e-3),indf(0),fsr(0), stack(0), status(0), W(0), pcl(0), pclath(0), m_configMemory(0)
{
  m_Capabilities = eSTACK | eWATCHDOGTIMER;

  if(verbose)
    cout << "pic_processor constructor\n";

  eeprom = 0;
  config_modes = create_ConfigMode();

  pll_factor = 0;

  Integer::setDefaultBitmask(0xff);

  // Test code for logging to disk:
  GetTraceLog().switch_cpus(this);
}
//-------------------------------------------------------------------
pic_processor::~pic_processor()
{

}
//-------------------------------------------------------------------
//
// 
//    create
//
//  The purpose of this member function is to 'create' a pic processor.
// Since this is a base class member function, only those things that
// are common to all pics are created.

void pic_processor::create ()
{

  init_program_memory (program_memory_size());

  init_register_memory (register_memory_size());

  create_stack();

  // Now, initialize the core stuff:
  pc->set_cpu(this);

  W = new WREG(this);

  pcl = new PCL;
  pclath = new PCLATH;
  status = new Status_register;
  //FIXME more
  
  W->new_name("W");
  
  indf = new INDF;

  register_bank = &registers[0];  // Define the active register bank 
  W->value.put(0);

  Vdd = 5.0;                      // Assume 5.0 volt power supply

  if(pma) {
    
    rma.SpecialRegisters.push_back(new PCHelper(pma));
    rma.SpecialRegisters.push_back(status);
    rma.SpecialRegisters.push_back(W);

    pma->SpecialRegisters.push_back(new PCHelper(pma));
    pma->SpecialRegisters.push_back(status);
    pma->SpecialRegisters.push_back(W);

  }

  create_config_memory();

}

//-------------------------------------------------------------------
//
// add_sfr_register
//
// The purpose of this routine is to add one special function register
// to the file registers. If the sfr has a physical address (like the 
// status or tmr0 registers) then a pointer to that register will be
// placed in the file register map. 

// FIXME It doesn't make any sense to initialize the por_value here!
// FIXME The preferred way is to initialize all member data in their
// FIXME parent's constructor.

void pic_processor::add_sfr_register(Register *reg, unsigned int addr,
				     RegisterValue por_value, const char *new_name)
{

  reg->set_cpu(this);
  if(addr < register_memory_size())
    {
      registers[addr] = reg;
      registers[addr]->address = addr;
      registers[addr]->alias_mask = 0;
      if(new_name)
        registers[addr]->new_name(new_name);

      RegisterValue rv = getWriteTT(addr);
      registers[addr]->set_write_trace(rv);
      rv = getReadTT(addr);
      registers[addr]->set_read_trace(rv);
    }

  reg->value       = por_value;
  reg->por_value   = por_value;  /// FIXME why are we doing this?
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

void pic_processor::create_symbols ()
{

  if(verbose)
    cout << __FUNCTION__ << " register memory size = " << register_memory_size() << '\n';

  for(unsigned int i = 0; i<register_memory_size(); i++)
    {
      switch (registers[i]->isa()) {
      case Register::SFR_REGISTER:
	if(!symbol_table.find((char *)registers[i]->name().c_str()))
	  symbol_table.add_register(registers[i]);
	break;
      default:
	break;
      }
    }

  val_symbol *vpc = new val_symbol(pc);
  vpc->set_description("Program Counter");
  symbol_table.add(vpc);

}


//-------------------------------------------------------------------

bool pic_processor::set_config_word(unsigned int address,unsigned int cfg_word)
{

  // Clear all of the configuration bits in config_modes and then
  // reset each of them based on the config bits in cfg_word:
  //config_modes &= ~(CM_WDTE);
  //config_modes |= ( (cfg_word & WDTE) ? CM_WDTE : 0);

  if((address == config_word_address()) && config_modes) {

    config_word = cfg_word;

    config_modes->config_mode = (config_modes->config_mode & ~7) | (cfg_word & 7);

    if((bool)verbose && config_modes)
      config_modes->print();

    return true;
  }

  return false;

}


unsigned int pic_processor::get_config_word(unsigned int address)
{
  return address == config_word_address() ? config_word : 0xffffffff;
}



//-------------------------------------------------------------------
//
// load_hex
//

bool pic_processor::LoadProgramFile(const char *pFilename, FILE *pFile,
				    const char *pProcessorName)
{
  Processor * pProcessor = this;
  // Tries the file type based on the file extension first.
  // If it fails tries the other type. This code will need
  // to change if pic_processor is moved to its own module
  // because then we cannot garrentee that these file types
  // will be the first two in the list.
  ProgramFileType * aFileTypes[] = {
    ProgramFileTypeList::GetList()[0],  // IntelHexProgramFileType
    ProgramFileTypeList::GetList()[1]   // PicCodProgramFileType
  };
  if(IsFileExtension(pFilename,"cod")) {
    // If 'cod' file extension, try PicCodProgramFileType first
    swap(aFileTypes[0], aFileTypes[1]);
  }
  int iReturn  = aFileTypes[0]->LoadProgramFile(&pProcessor, pFilename, pFile, pProcessorName);
  if (iReturn != ProgramFileType::SUCCESS) {
    fseek(pFile, 0, SEEK_SET);
    iReturn = aFileTypes[1]->LoadProgramFile(&pProcessor, pFilename, pFile, pProcessorName);
  }
  return iReturn == ProgramFileType::SUCCESS;
}

//-------------------------------------------------------------------
//-------------------------------------------------------------------
//  ConfigMode
//
void ConfigMode::print()
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
void ProgramMemoryAccess::callback()
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

//--------------------------------------------------
WDT::WDT(pic_processor *p_cpu, double _timeout)
  : cpu(p_cpu), breakpoint(0), future_cycle(0), timeout(_timeout), wdte(false)
{
}

//--------------------------------------------------
void WDT::update()
{
  if(wdte){

    value = (unsigned int )(cpu->get_frequency()*timeout);
    prescale = cpu->option_reg.get_psa() ? (cpu->option_reg.get_prescale()) : 0;

    if(future_cycle) {

      guint64 fc = get_cycles().value + value * (1<<prescale);

      //cout << "WDT::update:  moving break from " << future_cycle << " to " << fc << '\n';

      get_cycles().reassign_break(future_cycle, fc, this);
      future_cycle = fc;

    } else {
    
      future_cycle = get_cycles().value + value * (1<<prescale);

      get_cycles().set_break(future_cycle, this);
    }
  }

}

//--------------------------------------------------
// WDT::put - shouldn't be called?
//

void WDT::put(unsigned int new_value)
{
  value = new_value;

  update();

}
void WDT::set_timeout( double _timeout)
{
  timeout = _timeout;
  update();
}
void WDT::initialize(bool enable)
{
  wdte = enable;
  warned = 0;

  if(verbose)
    cout << " WDT init called "<< ( (enable) ? "enabling\n" :", but disabling WDT\n");

  if(wdte) {
    cout << "Enabling WDT " << " timeout = " << timeout << " seconds\n";
    value = (unsigned int) (cpu->get_frequency()*timeout);
    prescale = cpu->option_reg.get_psa() ? (cpu->option_reg.get_prescale()) : 0;

    future_cycle = get_cycles().value + value * (1<<prescale);

    get_cycles().set_break(future_cycle, this);

  } else {

    if (future_cycle) {
      cout << "Disabling WDT\n";
      get_cycles().clear_break(this);
      future_cycle = 0;
    }
  }

}

void WDT::reset()
{
  update();
}
void WDT::set_breakpoint(unsigned int bpn)
{
  breakpoint = bpn;
}

void WDT::callback()
{


  if(wdte) {
    cout<<"WDT timeout: " << hex << get_cycles().value << '\n';

    //future_cycle = 0;
    update();

    // The TO bit gets cleared when the WDT times out.
    cpu->status->put_TO(0);

    if(breakpoint)
      bp.halt();
    else 
    {
      bp.clear_sleep();
      cpu->reset(WDT_RESET);
    }
    /*    else if(bp.have_sleep()) {
      bp.clear_sleep();
    }else
      cpu->reset(WDT_RESET);
    */
  }

}

void WDT::clear()
{
  if(wdte)
    update();
  else
    {
      if(!warned)
	{
	  warned = 1;
	  cout << "The WDT is not enabled - clrwdt has no effect!\n";
	}
    }

}

void WDT::start_sleep()
{

  if(wdte) {
    prescale = 0;

    guint64 fc = get_cycles().value + value * (1<<prescale);

    //cout << "WDT::start_sleep:  moving break from " << future_cycle << " to " << fc << '\n';

    get_cycles().reassign_break(future_cycle, fc, this);

    future_cycle = fc;
  }
}

void WDT::new_prescale()
{

  update();

}

void WDT::callback_print()
{

  cout << "WDT\n";
}


//------------------------------------------------------------------------
// ConfigMemory - Base class
ConfigMemory::ConfigMemory(const char *_name, unsigned int default_val, const char *desc,
			   pic_processor *pCpu, unsigned int addr)
  : Integer(_name, default_val, desc), m_pCpu(pCpu), m_addr(addr)
{
  if (m_pCpu)
    m_pCpu->add_attribute(this);
}

