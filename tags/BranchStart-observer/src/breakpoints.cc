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


#include <iostream.h>
#include <iomanip.h>

#include "pic-processor.h"
#include "breakpoints.h"
#include "14bit-processors.h"
#include "xref.h"

extern unsigned int simulation_start_cycle;
extern pic_processor *active_cpu;

Breakpoints bp;

unsigned int Breakpoints::set_breakpoint(BREAKPOINT_TYPES break_type, pic_processor *cpu,unsigned int arg1, unsigned arg2, BreakCallBack *f1 = NULL)
{
  file_register *fr;
  int i;
  Breakpoint_Instruction *abp;
  Break_register_read *bfrr;
  Break_register_write *bfrw;
  Break_register_read_value *bfrrv;
  Break_register_write_value *bfrwv;
  bool found =0;

  // First, find a free break point
  i=0;
  //  breakpoint_number = (last_breakpoint+ 1) % MAX_BREAKPOINTS;
  breakpoint_number = 0;

  do
    {
      if(break_status[breakpoint_number].type == BREAK_CLEAR)
	found=1;
      else
	breakpoint_number = (breakpoint_number + 1) % MAX_BREAKPOINTS;

      i++;
    } while((i < MAX_BREAKPOINTS) && !found);


  if(!found)
    {
      cout << "*** out of breakpoints\n";
      return(MAX_BREAKPOINTS);
    }
  //  else
  //  {
  //      last_breakpoint = breakpoint_number;
  //    }

  break_status[breakpoint_number].type = break_type;
  break_status[breakpoint_number].cpu  = cpu;
  break_status[breakpoint_number].arg1 = arg1;
  break_status[breakpoint_number].arg2 = arg2;
  break_status[breakpoint_number].f    = f1;


  switch (break_type)
    {
    case BREAK_ON_EXECUTION:

      // Get a new breakpoint instruction and let it replace
      // the instruction at which we are setting the break
      //   (note: the constructor saves the instruction we are replacing)

      if(arg1 < cpu->program_memory_size())
	{

	  cpu->pma.put(arg1,  new Breakpoint_Instruction(cpu,arg1,BREAK_ON_EXECUTION | breakpoint_number));

	  //cpu->program_memory[arg1] =
	  //  new Breakpoint_Instruction(cpu,arg1,BREAK_ON_EXECUTION | breakpoint_number);

	  return(breakpoint_number);
	}
      else
	break_status[breakpoint_number].type = BREAK_CLEAR;

      break;

    case BREAK_ON_REG_WRITE:
      bfrw = new Break_register_write();
      bfrw->break_point = BREAK_ON_REG_WRITE | breakpoint_number;
      bfrw->replace(cpu,arg1);

      return(breakpoint_number);
      break;

    case BREAK_ON_REG_WRITE_VALUE:
      bfrwv = new Break_register_write_value();
      bfrwv->break_value = arg2;
      bfrwv->break_point = BREAK_ON_REG_WRITE_VALUE | breakpoint_number;
      bfrwv->replace(cpu,arg1);

      return(breakpoint_number);
      break;

    case BREAK_ON_REG_READ:
      bfrr = new Break_register_read();
      bfrr->break_point = BREAK_ON_REG_READ | breakpoint_number;
      bfrr->replace(cpu,arg1);

      return(breakpoint_number);
      break;

    case BREAK_ON_REG_READ_VALUE:
      bfrrv = new Break_register_read_value();
      bfrrv->break_value = arg2;
      bfrrv->break_point = BREAK_ON_REG_READ_VALUE | breakpoint_number;
      bfrrv->replace(cpu,arg1);

      return(breakpoint_number);
      break;

    case BREAK_ON_INVALID_FR:
      fr = cpu->registers[arg1];
      fr->break_point = BREAK_ON_INVALID_FR | breakpoint_number;
      return(breakpoint_number);
      break;

    case BREAK_ON_CYCLE:
      {
	guint64 cyc = arg2;
	cyc = (cyc<<32) | arg1;

	// The cycle counter does its own break points.
	if(cpu->cycles.set_break(cyc, f1, breakpoint_number))
	  return(breakpoint_number);
	else
	  break_status[breakpoint_number].type = BREAK_CLEAR;
      }
      break;

    case BREAK_ON_WDT_TIMEOUT:
      ((_14bit_processor *)cpu)->wdt.break_point = BREAK_ON_WDT_TIMEOUT | breakpoint_number;
      return(breakpoint_number);
      break;

    default:   // Not a valid type
      break_status[breakpoint_number].type = BREAK_CLEAR;
      break;
    }

  return(MAX_BREAKPOINTS);
}


unsigned int  Breakpoints::set_execution_break(pic_processor *cpu, unsigned int address)
{
  return(set_breakpoint (Breakpoints::BREAK_ON_EXECUTION, cpu, address, 0));
}

unsigned int  Breakpoints::set_read_break(pic_processor *cpu, unsigned int register_number)
{
  return(set_breakpoint (Breakpoints::BREAK_ON_REG_READ, cpu, register_number, 0));
}

unsigned int  Breakpoints::set_write_break(pic_processor *cpu, unsigned int register_number)
{
  return(set_breakpoint (Breakpoints::BREAK_ON_REG_WRITE, cpu, register_number, 0));
}

unsigned int  Breakpoints::set_read_value_break(pic_processor *cpu, unsigned int register_number,unsigned int value)
{
  return(set_breakpoint (Breakpoints::BREAK_ON_REG_READ_VALUE, cpu, register_number, value));
}

unsigned int  Breakpoints::set_write_value_break(pic_processor *cpu, unsigned int register_number,unsigned int value)
{
  return(set_breakpoint (Breakpoints::BREAK_ON_REG_WRITE_VALUE, cpu, register_number, value));
}

unsigned int  Breakpoints::set_cycle_break(pic_processor *cpu, guint64 future_cycle, BreakCallBack *f1 = NULL)
{

  return(set_breakpoint (Breakpoints::BREAK_ON_CYCLE, cpu, future_cycle & 0xffffffff, future_cycle>>32,f1));    
}

unsigned int  Breakpoints::set_wdt_break(pic_processor *cpu)
{
  // Set a wdt break only if one is not already set.

  if(cpu14->wdt.break_point == 0)
    return(set_breakpoint (Breakpoints::BREAK_ON_WDT_TIMEOUT, cpu, 0, 0));
  else
    return MAX_BREAKPOINTS;
}

//---------------------------------------------------------------------------------------
unsigned int Breakpoints::check_write_break(file_register *fr)
{
  //  cout << "debug   checking for write break point " << fr->name() << " @ " << hex << fr->address << "  cpu addr = " << active_cpu->pc.value <<'\n';

  if( (fr->break_point & BREAK_MASK) ==  BREAK_ON_REG_WRITE)
  {
    if(simulation_mode == RUNNING)
      {
	cout << "Hit a breakpoint!\n";
	trace.breakpoint( (Breakpoints::BREAK_ON_REG_WRITE>>8) );
	halt();
      }
    else
      {
	cout << "Stepped over a write breakpoint\n";
      }

    return(1);
  }

  return(0);
}

//---------------------------------------------------------------------------------------
unsigned int Breakpoints::check_read_break(file_register *fr)
{

  cout << "debug   checking for read break point " << fr->address << '\n';

  if( (fr->break_point & BREAK_MASK) ==  BREAK_ON_REG_READ)
  {
    if(simulation_mode == RUNNING)
      {
	cout << "Hit a breakpoint!\n";
	trace.breakpoint(fr->break_point & ~BREAK_MASK);
	halt();
      }
    else
      {
	cout << "Stepped over a read breakpoint\n";
      }

    return(1);
  }

  return(0);
}

//---------------------------------------------------------------------------------------
unsigned int Breakpoints::check_invalid_fr_break(invalid_file_register *fr)
{

  cout << "debug   checking for invalid file register access  break point " << fr->name() << '\n';

  if( (fr->break_point & BREAK_MASK) ==  BREAK_ON_INVALID_FR)
  {
    //global_break |= STOP_RUNNING;
    halt();
    trace.breakpoint(fr->break_point & ~BREAK_MASK);
    return(1);
  }

  return(0);
}

unsigned int Breakpoints::check_cycle_break(unsigned int abp)
{
  
  halt();
  if( abp < MAX_BREAKPOINTS)
    {
      if (NULL != break_status[abp].f)
	  break_status[abp].f->callback();

      trace.breakpoint( (Breakpoints::BREAK_ON_CYCLE>>8) );

      clear(abp);
    }

  return(1);

}
//---------------------------------------------------------------------------------------
unsigned int Breakpoints::check_break(file_register *fr)
{
  cout << "checking for a bp doesn't do anything...\n";

  return(0);
}

bool Breakpoints::dump1(unsigned int bp_num)
{

  bool set_by_user = 0;

  BREAKPOINT_TYPES break_type = break_status[bp_num].type;

  switch (break_type)
    {
    case BREAK_ON_EXECUTION:
      cout << hex << setw(0) << bp_num << ": " << break_status[bp_num].cpu->name_str << "  ";
      cout << "exec at " << hex << setw(4) << setfill('0') <<  break_status[bp_num].arg1 << '\n';
      set_by_user = 1;
      break;

    case BREAK_ON_REG_WRITE:
      cout << hex << setw(0) << bp_num << ": " << break_status[bp_num].cpu->name_str << "  ";
      cout << "reg write: " << hex << setw(2) << setfill('0') <<  break_status[bp_num].arg1 << '\n';
      set_by_user = 1;
      break;

    case BREAK_ON_REG_WRITE_VALUE:
      cout << hex << setw(0) << bp_num << ": " << break_status[bp_num].cpu->name_str << "  ";
      cout << "reg write. Break when " << hex << setw(2) << setfill('0') <<  break_status[bp_num].arg2 << 
	" is written to register " << break_status[bp_num].arg1 << '\n';
      set_by_user = 1;
      break;

    case BREAK_ON_REG_READ:
      cout << hex << setw(0) << bp_num << ": " << break_status[bp_num].cpu->name_str << "  ";
      cout << "reg read: " << hex << setw(2) << setfill('0') <<  break_status[bp_num].arg1 << '\n';
      set_by_user = 1;
      break;

    case BREAK_ON_REG_READ_VALUE:
      cout << hex << setw(0) << bp_num << ": " << break_status[bp_num].cpu->name_str << "  ";
      cout << "reg read. Break when " << hex << setw(2) << setfill('0') <<  break_status[bp_num].arg2 << 
	" is read from register " << break_status[bp_num].arg1 << '\n';
      set_by_user = 1;
      break;


    case BREAK_ON_CYCLE:
      {
	guint64 cyc =  break_status[bp_num].arg2;
	cyc = (cyc <<32)  | break_status[bp_num].arg1;
	cout << hex << setw(0) << bp_num << ": " << break_status[bp_num].cpu->name_str << "  ";
	cout << "cycle " << hex << setw(16) << setfill('0') <<  cyc << '\n';
	set_by_user = 1;
      }
      break;

    case BREAK_ON_WDT_TIMEOUT:
      cout << hex << setw(0) << bp_num << ": " << break_status[bp_num].cpu->name_str << "  ";
      cout << "wdt time out\n";
      set_by_user = 1;
      break;

    }

  return(set_by_user);

}


void Breakpoints::dump(void)
{
  bool have_breakpoints = 0;


  for(int i = 0; i<MAX_BREAKPOINTS; i++)
    {

      if(dump1(i))
	have_breakpoints = 1;
    }


  if(!have_breakpoints)
    cout << "No user breakpoints are set... I think\n";

}


void Breakpoints::clear(unsigned int b)
{
  Breakpoint_Instruction *abp;
  instruction *inst;
  file_register *fr;

  if(b<MAX_BREAKPOINTS)
    {
      BreakStatus bs = break_status[b];   // 

      switch (bs.type)
	{
	case BREAK_ON_EXECUTION:

	  inst = bs.cpu->program_memory[bs.arg1];              // Get a pointer to the break point instruction
	  abp= (Breakpoint_Instruction *) inst;
	  //bs.cpu->program_memory[bs.arg1] = abp->replaced;     // Restore the instruction
	  bs.cpu->pma.put(bs.arg1, abp->replaced);             // Restore the instruction

	  delete abp;
	  break_status[b].type = BREAK_CLEAR;
//	  bs.cpu->program_memory[bs.arg1]->xref->update();
	  cout << "Cleared execution breakpoint number " << b << '\n';

	  break;

	case BREAK_ON_CYCLE:
	  break_status[b].type = BREAK_CLEAR;
	  cout << "Cleared cycle breakpoint number " << b << '\n';

	  break;

	case BREAK_ON_REG_READ:
	case BREAK_ON_REG_READ_VALUE:
	case BREAK_ON_REG_WRITE:
	case BREAK_ON_REG_WRITE_VALUE:
	  fr = bs.cpu->registers[bs.arg1];
	  if (fr->isa() == file_register::BP_REGISTER )
	    {
	      Breakpoint_Register *br = (Breakpoint_Register *)fr;
	      int cleared = 0;
	      // There may be multiple break points set on this register.
	      // So let's loop through them all to find the one that has
	      // breakpoint 'b' set on it.

	      while( br )
		{
		  if(br->clear(b))
		    {
		      cleared = 1;
		      delete br;
		      br = NULL;
		    }
		  else
		    {
		      if(br->replaced->isa() == file_register::BP_REGISTER)
			br = (Breakpoint_Register *)br->replaced;
		      else
			br = NULL; // Break out of loop and display error
		    }
		}

	      // If we looped through all of the breakpoints that are set
	      // on this register, but we didn't find the break point number
	      // we were seeking, then there's an error in the break point
	      // logic. Specifically, when we set the break and assigned a
	      // a break point number and associated it with a file register,
	      // that information was stored in the break_status structure.
	      // If that information got corrupted somehow, then we'll get
	      // the internal error.

	      if(!cleared)
		cout << "Breakpoints::cleared: internal error\n";

	      if(bs.cpu->registers[bs.arg1]->xref)
		  bs.cpu->registers[bs.arg1]->xref->update();

	      cout << "Cleared breakpoint number " << b << " on register " << bs.arg1 << '\n';

	    }
	  else
	    {
	      cout << "??? break is not set\n";
	    }

	  break;

	case BREAK_ON_WDT_TIMEOUT:
	  bs.type = BREAK_CLEAR;
	  cout << "Cleared wdt timeout breakpoint number " << b << '\n';
	  ((_14bit_processor *)bs.cpu)->wdt.break_point = 0;

	  break;

	}
    }
}

//
//  dump_traced
//  Called by the trace class to display a breakpoint that is in the
// trace buffer.

void Breakpoints::dump_traced(unsigned int b)
{

  BREAKPOINT_TYPES break_type = (BREAKPOINT_TYPES) ((b & 0xff0000) << 8);

  switch (break_type)
    {
    case BREAK_ON_EXECUTION:
      cout << "execution at "<< hex << setw(4) << setfill('0') <<  (b & 0xffff) << '\n';
      break;

    case BREAK_ON_REG_WRITE:
      cout << "reg write: " << hex << setw(2) << setfill('0') <<  (b & 0xff) << '\n';
      break;

    case BREAK_ON_REG_WRITE_VALUE:
      cout << "wrote " << hex << setw(2) << setfill('0') <<  ((b & 0xff00)>>8) << 
	" to register " << hex << setw(2) << setfill('0') <<  (b & 0xff) << '\n';
      break;

    case BREAK_ON_REG_READ:
      cout << "reg write: " << hex << setw(2) << setfill('0') <<  (b & 0xff) << '\n';
      break;

    case BREAK_ON_REG_READ_VALUE:
      cout << "read " << hex << setw(2) << setfill('0') <<  ((b & 0xff00)>>8) << 
	" from register " << hex << setw(2) << setfill('0') <<  (b & 0xff) << '\n';
      break;

    case BREAK_ON_CYCLE:
      cout << "cycle " << '\n';
      break;

    case BREAK_ON_WDT_TIMEOUT:
      cout << "wdt time out\n";
      break;

    default:
      cout << "unknown\n";
    }



}


// Clear all break points that are set for a specific processor
// This only be called when a processor is being removed and not when a user 
// wants to clear the break points. Otherwise, internal break points like
// invalid register accesses will get cleared.

void Breakpoints::clear_all(pic_processor *c)
{

  for(int i=0; i<MAX_BREAKPOINTS; i++)
    {
      if(c == break_status[i].cpu)
	clear(i);
    }

}

void Breakpoints::clear_all_set_by_user(pic_processor *c)
{

  for(int i=0; i<MAX_BREAKPOINTS; i++)
    {
      if((c == break_status[i].cpu) && (break_status[i].type != BREAK_ON_INVALID_FR))
	clear(i);
    }

}


Breakpoints::Breakpoints(void)
{
  
  //  execution_breakpoint_ptr = new Breakpoint_Instruction(memory_size);
  breakpoint_number = 0;
  //last_breakpoint =0;

  for(int i=0; i<MAX_BREAKPOINTS; i++)
    break_status[i].type = BREAK_CLEAR;

}

//---------------------------------------------------------------------------------------
void Breakpoint_Instruction::execute(void)
{

  if( (simulation_mode == RUNNING) && (simulation_start_cycle != cpu->cycles.value))
    {
      cout << "Hit a breakpoint!\n";
      trace.breakpoint( (Breakpoints::BREAK_ON_EXECUTION>>8) | address );
      bp.halt();
    }
  else
    {
      cout << "Ignoring a breakpoint\n";
      replaced->execute();
    }

}

Breakpoint_Instruction::Breakpoint_Instruction(pic_processor *new_cpu, unsigned int new_address,unsigned int bp)
{
  cpu = new_cpu;
  address = new_address;
  opcode = 0xffffffff;
  bpn = bp;
  replaced = cpu->program_memory[address];

  // use the replaced instructions xref object
  xref=cpu->program_memory[address]->xref;
}

unsigned int Breakpoint_Instruction::get_opcode(void)
{ 
  return(replaced->get_opcode());
}
int Breakpoint_Instruction::get_src_line(void)
{
  return(replaced->get_src_line());
}
int Breakpoint_Instruction::get_lst_line(void)
{
  return(replaced->get_lst_line());
}

int Breakpoint_Instruction::get_file_id(void)
{
  return(replaced->get_file_id());
}

char * Breakpoint_Instruction::name(char *return_str)
{

  return(replaced->name(return_str));
}

//---------------------------------------------------------------------------------------
void Breakpoint_Register::replace(pic_processor *_cpu, unsigned int reg)
{
  file_register *fr = _cpu->registers[reg];

  if(fr->isa() == file_register::BP_REGISTER)
    {
      // We're setting a break on top of another break
      ((Breakpoint_Register *)fr)->next = this;
    }

  cpu = _cpu;
  cpu->registers[reg] = this;
  replaced = fr;
  next = NULL;

  address=fr->address;
  
  // use the replaced registers xref object
  xref=fr->xref;

  if(fr->xref)
    fr->xref->update();

}
  
unsigned int Breakpoint_Register::clear(unsigned int bp_num)
{

  if(bp.break_status[bp_num].type == Breakpoints::BREAK_CLEAR)   // This is a redundant check.
    return 0;

  if(bp_num != (break_point & ~Breakpoints::BREAK_MASK))
    return 0;

  if(replaced->isa() == file_register::BP_REGISTER)
    {
      // We're clearing a break that is set on top of another break
      ((Breakpoint_Register *)replaced)->next = next;
    }

  if(next)
    {
      // There's a break that is set on top of this one
      if(next->replaced != this)
	cout << "Breakpoint_Register::clear is confused\n";

      next->replaced = replaced;

    }
  else
    {
      // This break is the last (perhaps only) break point that is
      // set on this cpu's file register.

      cpu->registers[bp.break_status[bp_num].arg1] = replaced;

    }

  bp.break_status[bp_num].type = Breakpoints::BREAK_CLEAR;
  return 1;
}


//====================================================================================
//
// catch_control_c
//
//  
void catch_control_c(int sig)
{

  if(simulation_mode != STOPPED)
    {
      cout << "<CTRL C> break\n";
      bp.halt();
    }
  else
    cout << "caught control c, but it doesn't seem gpsim was simulating\n";


}

#if HAVE_GUI

//-------------------------------------------------------------------
//
GuiCallBack gcb;

GuiCallBack::GuiCallBack(void)
{
  gui_callback_data = NULL;

  gui_callback = NULL;
}

//
//  GuiCallBack::callback 
// This routine provides an interface back to the gui. A 'cycle' break point
// will interrupt a running simulation and invoke this routine.
//
void GuiCallBack::callback(void)
{

  if(gui_callback)
    {
      gui_callback(gui_callback_data);

      //unsigned int lo = active_cpu->cycles.value.lo + 0x100000;
      //unsigned int hi = active_cpu->cycles.value.hi;
      //if(lo < active_cpu->cycles.value.lo) // rollover
      //hi++;
      //active_cpu->cycles.set_break(lo, hi, this);

      active_cpu->cycles.set_break(active_cpu->cycles.value + gui_update_rate, this);

    }


}

void GuiCallBack::set_break(int cycle, 
			    void (*new_gui_callback)(gpointer), 
			    gpointer new_gui_callback_data)
{
  active_cpu->cycles.set_break(active_cpu->cycles.value + 0x100000, this);


  gui_callback_data = new_gui_callback_data;

  gui_callback = new_gui_callback;

  new_gui_callback(new_gui_callback_data);

}

//-------------------------------------------------------------------
//
// utility routines to interface with the gui...
//

// gui_set_cycle_break_point - a C-wrapper for a C++ function...

//void gui_set_cycle_break_point(guint64 cycle, void (*gui_callback)(gpointer), gpointer gui_callback_data)
//{
//
//  gcb.set_break(cycle, gui_callback,gui_callback_data);
//
//}

#endif


