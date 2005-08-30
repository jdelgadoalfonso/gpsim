/*
   Copyright (C) 1998-2000 T. Scott Dattalo

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
#include <iostream>
#include <iomanip>

#include "14bit-processors.h"
#include "interface.h"

#include <string>
#include "stimuli.h"
#include "errors.h"
#include "../config.h"
#include "xref.h"

#include "protocol.h"

//#define __DEBUG_CYCLE_COUNTER__

//--------------------------------------------------
// Global instantiation of the cycle counter 
// and the stop watch;
//--------------------------------------------------

Cycle_Counter cycles;

// create an instance of inline get_cycles() method by taking its address
Cycle_Counter &(*dummy_cycles)(void) = get_cycles;

StopWatch stop_watch;


//--------------------------------------------------
// member functions for the Cycle_Counter class
//--------------------------------------------------

/*

  Overview of Cycle Counter break points.

  The Cycle Counter break points coordinate simulation time. At the
  moment, the cycle counter advances one count for every instruction
  cycle. Thus "real time" is defined in terms of "instruction cycles".
  (NOTE - This will change!)

  Now, the way gpsim uses these break points is by allowing peripherals
  to grab control of the simulation at a particular instance of time.
  For example, if the UART peripheral needs to send the next data
  bit in 100 microseconds, then it needs to have control of the simulator
  in exactly 100 microseconds from right now. The way this is handled,
  is that the cycle counter is advanced at every instruction cycle
  (This will change...) and when the cycle counter matches the cycle
  that corresponds to that 100 microsecond gap, gpsim will divert
  control to the UART peripheral.

  There are 3 components to a cycle counter break point. First there's
  the obvious thing: the number that corresponds to the cycle at which
  we wish to break. This is a 64-bit integer and thus should cover a
  fairly significant simulation interval! A 32-bit integer only covers
  about 7 minutes of simulation time for a pic running at 20MHz. 64-bits
  provides over 100,000 years of simulation time!

  The next component is the call back function. This is a pointer to a
  function, or actually a class that contains a function, that is 
  called when the current value of the cycle counter matches the
  break point value. In the UART example, this function will be something
  that the UART peripheral passed when it set the break point. This
  is how the UART peripheral gets control of the simulator.

  Finally, the third component is the linked list mechanism. Each time
  a break point is set, it gets inserted into a linked list that is
  sorted by the cycle break point value. Thus the first element in
  the list (if there are any elements at all) is always the next 
  cycle counter break point.


*/


Cycle_Counter_breakpoint_list *Cycle_Counter_breakpoint_list::getNext()
{
  return next;
}
Cycle_Counter_breakpoint_list *Cycle_Counter_breakpoint_list::getPrev()
{
  return prev;
}

void Cycle_Counter_breakpoint_list::clear()
{
  bActive = false;
  if(f)
    f->clear_break();
}

void Cycle_Counter_breakpoint_list::invoke()
{
  if(bActive) {
    clear();
    if(f)
      f->callback();
  }
}

//--------------------------------------------------

void Cycle_Counter::preset(guint64 new_value)
{
  value = new_value;
  get_trace().cycle_counter(value);
}

void Cycle_Counter::set_cycles_per_second(guint64 cps)
{
  if(cps)
  {
    cycles_per_second = (double)cps;
    seconds_per_cycle = 1.0/cycles_per_second;
  }

}

//--------------------------------------------------
// get(double seconds_from_now)
//
// Return the cycle number that corresponds to a time
// biased from the current time. 
//
// INPUT: time in seconds (note that it's a double)
//
// OUTPUT: cycle count 

guint64 Cycle_Counter::get(double future_time_from_now)
{
  return value + (guint64)(cycles_per_second * future_time_from_now);

}

//--------------------------------------------------
// set_break
// set a cycle counter break point. Return 1 if successful.
//
//  The break points are stored in a singly-linked-list sorted by the
// order in which they will occur. When this routine is called, the
// value of 'future_cycle' is compared against the values in the
// 'active' list.

bool Cycle_Counter::set_break(guint64 future_cycle, TriggerObject *f, unsigned int bpn)
{

  Cycle_Counter_breakpoint_list  *l1 = &active, *l2;
  static unsigned int CallBackID_Sequence=1;

#ifdef __DEBUG_CYCLE_COUNTER__
  cout << "Cycle_Counter::set_break  cycle = 0x" << hex<<future_cycle;

    if(f)
      cout << " has callback\n";
    else
      cout << " does not have callback\n";
#endif


    if(inactive.next == 0) {

      cout << " too many breaks are set on the cycle counter \n";
      return 0;

    } else if(future_cycle <= value) {

      cout << "Cycle break point was ignored because cycle " << future_cycle << " has already gone by\n";
      cout << "current cycle is " << value << '\n';
      return 0;

    } else {

      // place the break point into the sorted break list

      bool break_set = 0;

      while( (l1->next) && !break_set) {

	// If the next break point is at a cycle greater than the
	// one we wish to set, then we found the insertion point.
	// Otherwise 
	if(l1->next->break_value >= future_cycle)
	  break_set = 1;
	else
	  l1 = l1->next;

      }

      // At this point, we have the position where we need to insert the 
      // break point: it's at l1->next.

      l2 = l1->next;
      l1->next = inactive.next;

      // remove the break point from the 'inactive' list
      inactive.next = inactive.next->next;

      l1->next->next = l2;
      l1->next->prev = l1;
      if(l2)
	l2->prev = l1->next;

      l1->next->break_value = future_cycle;
      l1->next->f = f;
      l1->next->breakpoint_number = bpn;
      l1->next->bActive = true;


      if(f)
	f->CallBackID = ++CallBackID_Sequence;

#ifdef __DEBUG_CYCLE_COUNTER__
      cout << "cycle break " << future_cycle << " bpn " << bpn << '\n';
      if(f)
	cout << "call back sequence number = "<< f->CallBackID <<'\n';
#endif

    }

  break_on_this = active.next->break_value;

  return 1;
}

//--------------------------------------------------
// remove_break
// remove break for TriggerObject
void Cycle_Counter::clear_break(TriggerObject *f)
{

  Cycle_Counter_breakpoint_list  *l1 = &active, *l2 = 0;

  if(!f)
    return;

  while( (l1->next) && !l2) {
      
    if(l1->next->f ==  f)
      l2 = l1;

    l1=l1->next;
  }

  if(!l2) {
    //#ifdef __DEBUG_CYCLE_COUNTER__
    cout << "WARNING Cycle_Counter::clear_break could not find break point\n  Culprit:\t";
    f->callback_print();
    //#endif
    return;
  }
  // at this point l2->next points to our break point
  // It needs to be removed from the 'active' list and put onto the 'inactive' list.

  l1 = l2;
  l2 = l1->next;              // save a copy for a moment
  l1->next = l1->next->next;  // remove the break
  if(l1->next)
    l1->next->prev = l1;        // fix the backwards link.

  l2->clear();

#ifdef __DEBUG_CYCLE_COUNTER__
  if (f) 
    cout << "Clearing break call back sequence number = "<< f->CallBackID <<'\n';
#endif

  // Now move the break to the inactive list.

  l1 = inactive.next;
  if(!l1) 
    return;

  l2->next = l1;
  inactive.next = l2;

  break_on_this =  active.next ? active.next->break_value : 0;

}

//--------------------------------------------------
// set_break_delta
// set a cycle counter break point relative to the current cpu cycle value. Return 1 if successful.
//

bool Cycle_Counter::set_break_delta(guint64 delta, TriggerObject *f, unsigned int bpn)
{

#ifdef __DEBUG_CYCLE_COUNTER__
  cout << "Cycle_Counter::set_break_delta  delta = 0x" << hex<<delta;

  if(f)
    cout << " has callback\n";
  else
    cout << " does not have callback\n";
#endif

  return set_break(value+delta,f,bpn);

  

}


//--------------------------------------------------
// clear_break
// remove the break at this cycle

void Cycle_Counter::clear_break(guint64 at_cycle)
{

  Cycle_Counter_breakpoint_list  *l1 = &active, *l2;

  bool found = 0;

  while( l1->next && !found) {
      
    // If the next break point is at the same cycle as the
    // one we wish to clear, then we found the deletion point.
    // Otherwise keep searching.

    if(l1->next->break_value ==  at_cycle)
      found = 1;
    else
      l1=l1->next;
  }

  if(!found) {
    cout << "Cycle_Counter::clear_break could not find break at cycle 0x"
      << hex << setw(16) << setfill('0') << at_cycle << endl;
    return;
  }

  l2 = l1->next;  // save a copy for a moment
  l1->next = l1->next->next;  // remove the break
  if(l1->next)
    l1->next->prev = l2;

  l2->clear();

  // Now move the break to the inactive list.

  l1 = inactive.next;
  if(!l1) 
    return;

  l2->next = l1;
  inactive.next = l2;

  break_on_this =  active.next ? active.next->break_value : 0;

}

//------------------------------------------------------------------------
// breakpoint
//
// When the cycle counter has encountered a cycle that has a breakpoint,
// this routine is called.
//

void Cycle_Counter::breakpoint()
{
  // There's a break point set on this cycle. If there's a callback function, then call
  // it other wise halt execution by setting the global break flag.

  // Loop in case there are multiple breaks
  //while(value == break_on_this && active.next) {
  while(active.next  && value == active.next->break_value) {
	

    if(active.next->f) {
      // This flag will get set true if the call back
      // function moves the break point to another cycle.
      Cycle_Counter_breakpoint_list  *l1 = active.next;
      //reassigned = false;
      l1->bActive = false;
      l1->f->callback();
      if(!l1->bActive)
	clear_current_break();
    } else {
      get_bp().check_cycle_break(active.next->breakpoint_number);
      clear_current_break();
    }
  }

  if(active.next)
    break_on_this = active.next->break_value;

}
//------------------------------------------------------------------------
// reassign_break
//   change the cycle of an existing break point.
//
//  This is only called by the internal peripherals and not (directly) by 
// the user. It's purpose is to accommodate the dynamic and unpredictable
// needs of the internal cpu timing. For example, if tmr0 is set to roll 
// over on a certain cycle and the program changes the pre-scale value, 
// then the break point has to be moved to the new cycle.

bool Cycle_Counter::reassign_break(guint64 old_cycle, guint64 new_cycle, TriggerObject *f)
{

  Cycle_Counter_breakpoint_list  *l1 = &active, *l2;

  bool found_old = false;
  bool break_set = false;

  reassigned = true;   // assume that the break point does actually get reassigned.

#ifdef __DEBUG_CYCLE_COUNTER__
  cout << "Cycle_Counter::reassign_break, old " << old_cycle << " new " << new_cycle;
  if(f) 
    cout << " Call back ID = " << f->CallBackID;

  cout << '\n';

  dump_breakpoints();
#endif

  //
  // First, we search for the break point by comparing the 'old_cycle'
  // with the break point cycle of all active break points. Two criteria
  // must be satisfied for a match:
  //
  //     1)  The 'old_cycle' must exactly match the cycle of an active
  //         break point.
  //     2)  The Call back function pointer must exactly match the call back
  //         function point of the same active break point.
  //
  // The reason for both of these, is so that we can differentiate multiple
  // break points set at the same cycle.
  //
  // NOTE to consider:
  //
  // It would be far more efficient to have the "set_break" function return
  // a handle or a pointer that we could use here to immediately identify
  // the break we wish to reassign. This would also disambiguate multiple
  // breaks set at the same cycle. We'd still have to perform a search through
  // the linked list to find the new point, but that search would be limited
  // (i.e. the reassignment is either before *or* after the current). A bi-
  // directional search can be optimized with a doubly-linked list...

  while( (l1->next) && !found_old) {
      
    // If the next break point is at a cycle greater than the
    // one we wish to set, then we found the insertion point.

    if(l1->next->break_value == old_cycle) {
#ifdef __DEBUG_CYCLE_COUNTER__
      cout << " cycle match ";
      if(f && l1->next->f && (f->CallBackID == l1->next->f->CallBackID))
	cout << " Call Back IDs match = " << f->CallBackID << ' ';
#endif
	
      if(l1->next->f == f)
	found_old = true;
      else
	l1 = l1->next;
    }
    else
      l1 = l1->next;

  }

  if(found_old) {

    // Now move the break point
#ifdef __DEBUG_CYCLE_COUNTER__
    cout << " found old ";
#endif
    if(l1->next->bActive == false) {
      cout << "CycleCounter - reassigning in active break ";
      if(l1->next->f)
	l1->next->f->callback_print();
      cout << endl;
    }
      

    if(new_cycle > old_cycle) {

      // First check to see if we can stay in the same relative position within the list
      // Is this the last one in the list? (or equivalently, is the one after this one 
      // a NULL?)

      if(l1->next->next == 0) {

	l1->next->break_value = new_cycle;
	break_on_this = active.next->break_value;
#ifdef __DEBUG_CYCLE_COUNTER__
	cout << " replaced at current position (next is NULL)\n";
	dump_breakpoints();   // debug
#endif
	return 1;
      }

      // Is the next one in the list still beyond this one?
      if(l1->next->next->break_value >= new_cycle) {
	l1->next->break_value = new_cycle;
	break_on_this = active.next->break_value;
#ifdef __DEBUG_CYCLE_COUNTER__
	cout << " replaced at current position (next is greater)\n";
	dump_breakpoints();   // debug
#endif
	return 1;
      }

      // Darn. Looks like we have to move it.

#ifdef __DEBUG_CYCLE_COUNTER__
      cout << " moving \n";
#endif
      l2 = l1->next;                        // l2 now points to this break point
      l1->next = l1->next->next;            // Unlink this break point
      l1->next->prev = l1;

      while( l1->next && !break_set) {

	// If the next break point is at a cycle greater than the
	// one we wish to set, then we found the insertion point.
	// Otherwise, continue searching.

	if(l1->next->break_value > new_cycle)
	  break_set = 1;
	else
	  l1 = l1->next;

      }

      // At this point, we know that our breakpoint needs to be
      // moved to the position just after l1
      l2->next = l1->next;
      l1->next = l2;
      l2->prev = l1;
      if(l2->next)
	l2->next->prev = l2;

      break_on_this = active.next->break_value;

      l2->break_value = new_cycle;

#ifdef __DEBUG_CYCLE_COUNTER__
      dump_breakpoints();   // debug
#endif
    } else {      // old_cycle < new_cycle

      // First check to see if we can stay in the same relative position within the list

#ifdef __DEBUG_CYCLE_COUNTER__
      cout << " old cycle is less than new one\n";
#endif
      // Is this the first one in the list?
      if(l1 == &active)
	{
	  l1->next->break_value = new_cycle;
	  break_on_this = new_cycle;
#ifdef __DEBUG_CYCLE_COUNTER__
	  cout << " replaced at current position\n";
	  dump_breakpoints();   // debug
#endif
	  return 1;
	}

      // Is the previous one in the list still before this one?
      if(l1->break_value < new_cycle)
	{
	  l1->next->break_value = new_cycle;

	  break_on_this = active.next->break_value;

#ifdef __DEBUG_CYCLE_COUNTER__
	  cout << " replaced at current position\n";
	  dump_breakpoints();   // debug
#endif
	  return 1;
	}

      // Darn. Looks like we have to move it.
      l2 = l1->next;                        // l2 now points to this break point

      l1->next = l1->next->next;            // Unlink this break point
      if(l1->next)
        l1->next->prev = l1;

      l1 = &active;                         // Start searching from the beginning of the list

      while( (l1->next) && !break_set)
	{

	  // If the next break point is at a cycle greater than the
	  // one we wish to set, then we found the insertion point.
	  // Otherwise 
	  if(l1->next->break_value > new_cycle)
	    break_set = 1;
	  else
	    l1 = l1->next;

	}

      l2->next = l1->next;
      l2->next->prev = l2;
      l1->next = l2;
      l2->prev = l1;

      l2->break_value = new_cycle;

      break_on_this = active.next->break_value;

#ifdef __DEBUG_CYCLE_COUNTER__
      dump_breakpoints();   // debug
#endif
    }
  }
  else {

    // oops our assumption was wrong, we were unable to reassign the break point 
    // to another cycle because we couldn't find the old one!

    reassigned = false;

    // If the break point was not found, it can't be moved. So let's just create
    // a new break point.
    cout << "WARNING Cycle_Counter::reassign_break could not find old break point\n";
    cout << "      a new break will created at cycle: 0x"<<hex<<new_cycle<<endl;
    if(f) {
      cout << " Culprit:\t";
      f->callback_print();
    }

    set_break(new_cycle, f);
  }
  return 1;
}

void Cycle_Counter::clear_current_break(void)
{

  if(active.next == 0)
    return;

  if(value == break_on_this)
    {
#ifdef __DEBUG_CYCLE_COUNTER__
      cout << "clearing current cycle break " << hex << setw(16) << setfill('0') << break_on_this;
      if(active.next->f)
	cout << " Call Back ID  = " << active.next->f->CallBackID;
      cout <<'\n';

      if(active.next->next) {
	cout << "  but there's one pending at the same cycle";
	if(active.next->next->f)
	  cout << " With ID = " << active.next->next->f->CallBackID;
	cout << '\n';
      }

#endif
      Cycle_Counter_breakpoint_list  *l1;

      active.next->bActive = false;
      l1 = inactive.next;                  // ptr to 1st inactive bp
      inactive.next = active.next;         // let the 1st active bp become the 1st inactive one
      active.next = active.next->next;     // The 2nd active bp is now the 1st
      inactive.next->next = l1;            // The 2nd inactive bp used to be the 1st

      if(active.next) {
	break_on_this = active.next->break_value;
	active.next->prev = &active;
      } else
	break_on_this = END_OF_TIME;
    }
  else
    {
      // If 'value' doesn't equal 'break_on_this' then what's most probably
      // happened is that the breakpoint associated with 'break_on_this'
      // has invoked a callback function that then did a ::reassign_break().
      // There's a slight chance that we have a bug - but very slight...
      if(verbose & 4) {
	cout << "debug::Didn't clear the current cycle break because != break_on_this\n";
	cout << "value = " << value << "\nbreak_on_this = " << break_on_this <<'\n';
      }
    }
}

void Cycle_Counter::dump_breakpoints(void)
{
  Cycle_Counter_breakpoint_list  *l1 = &active;

  cout << "Next scheduled cycle break " << hex << setw(16) << setfill('0') << break_on_this << '\n';

  while(l1->next)
    {
      //cout << cpu->name_str << "  " << "internal cycle break  " <<
      cout << "internal cycle break  " <<
	hex << setw(16) << setfill('0') <<  l1->next->break_value << ' ';

      if(l1->next->f)
	l1->next->f->callback_print();
      else
	cout << "does not have callback\n";

      l1 = l1->next;
    }

}


Cycle_Counter::Cycle_Counter(void)
{
  value         = 0;
  break_on_this = END_OF_TIME;

  cycles_per_second = 5.0e6;
  seconds_per_cycle = 1 / cycles_per_second;

  active.next   = 0;
  active.prev   = 0;
  inactive.next = 0;
  inactive.prev = 0;

  Cycle_Counter_breakpoint_list  *l1 = &inactive;

  for(int i=0; i<BREAK_ARRAY_SIZE; i++)
    {
      l1->next = new Cycle_Counter_breakpoint_list;
      l1->next->prev = l1;

      l1 = l1->next;
    }
  l1->next = 0;

}


//------------------------------------------------------------------------
// StopWatch 
//

//========================================================================
// Stop Watch Attributes
//========================================================================
class StopWatchValue : public Integer
{
private:
  StopWatch *sw;
public:
  StopWatchValue(StopWatch *_sw) : Integer(0), sw(_sw) 
  {
    m_bClearableSymbol = false;
    new_name("stopwatch");
    set_description(" A timer for monitoring and controlling the simulation.\n"
		    " The units are in simulation cycles.\n"
		    "  stopwatch.rollover - specifies rollover value.\n"
		    "  stopwatch.direction - specifies count direction.\n"
		    "  stopwatch.enable - enables counting if true.\n"
		    );
  }
  virtual void set(Value *v)
  {
    Integer::set(v);
    if(sw) sw->update();
  }
  virtual void get(gint64 &i)
  {
    i = (sw) ? sw->get() : 0;
    Integer::set(i);
  }
  virtual void set_break()
  {
    if(sw) sw->set_break(true);
  }
  virtual void clear_break()
  {
    if(sw) sw->set_break(false);
  }

};
class StopWatchRollover : public Integer
{
private:
  StopWatch *sw;
public:
  StopWatchRollover(StopWatch *_sw) : Integer(1000000), sw(_sw) 
  {
    m_bClearableSymbol = false;
    new_name("stopwatch.rollover");
    set_description(" specifies the stop watch roll over time.");
  }
  virtual void set(Value *v)
  {
    Integer::set(v);
    if(sw) sw->update();
  }
};

class StopWatchEnable : public Boolean
{
private:
  StopWatch *sw;
public:
  StopWatchEnable(StopWatch *_sw) : Boolean(true) , sw(_sw)
  {
    m_bClearableSymbol = false;
    new_name("stopwatch.enable");
    set_description(" If true, the stop watch is enabled.");
  }
  virtual void set(Value *v)
  {
    if(sw) sw->update();
    Boolean::set(v);

  }
  
};
class StopWatchDirection : public Boolean
{
private:
  StopWatch *sw;
public:
  StopWatchDirection(StopWatch *_sw) : Boolean(true) , sw(_sw)
  {
    m_bClearableSymbol = false;
    new_name("stopwatch.direction");
    set_description(" If true, the stop watch counts up otherwise down.");
  }
  virtual void set(Value *v)
  {
    if(!v)
      return;

    bool bOldVal = getVal();
    bool bNewVal;
    
    v->get(bNewVal);

    if(sw && bOldVal != bNewVal)
      sw->set_direction(bNewVal);

  }
};
//------------------------------------------------------------

StopWatch::StopWatch()
{

  offset      = 0;
  initialized = false;
  value       = new StopWatchValue(this);
  rollover    = new StopWatchRollover(this);
  enable      = new StopWatchEnable(this);
  direction   = new StopWatchDirection(this);

  if(!value || !rollover || !enable || !direction)
    throw Error("StopWatch");

}

void StopWatch::init()
{
  if(!initialized) {
    get_symbol_table().add(value);
    get_symbol_table().add(rollover);
    get_symbol_table().add(enable);
    get_symbol_table().add(direction);

    update();
    initialized = true;
  }
}

//----------------------------------------
// get()
// If the stopwatch is running, then compute
// the current value based on the cycle_counter.

guint64 StopWatch::get(void)
{

  if(enable->getVal()) {
    gint64 v = (cycles.value - offset) % rollover->getVal();
    if(!direction->getVal())
      v = rollover->getVal() - v;
    return v;
  }

  return value->getVal();

}

//----------------------------------------
// get()
// If the stopwatch is running, then compute
// the current value based on the cycle_counter.

double StopWatch::get_time(void)
{

  guint64 current_value =get();

  if(current_value)
    return current_value /4000000.0;

  return 1.0;

}

void StopWatch::set_enable(bool b)
{
  if(enable->getVal() != b)
    enable->set(b);
  update();

}
void StopWatch::set_direction(bool b)
{
  if(direction->getVal() == b)
    return;

  direction->set(b);

  offset = 
    cycles.value - 
    ((rollover->getVal() - value->getVal()) % rollover->getVal());


  if(break_cycle)
    set_break(true);

}

void StopWatch::set_rollover(guint64 new_rollover)
{
  // setting the rollover attribute will update the stopwatch too.
  if(rollover)
    rollover->set((gint64)new_rollover);
}
void StopWatch::set_value(guint64 new_value)
{
  if(value)
    value->set((gint64)new_value);
}

// update() compute a new offset such that the current
// value of the stopwatch is correlated with the cycle counter.
void StopWatch::update()
{
  if(enable->getVal()) {
    if(direction->getVal())
      offset = cycles.value - value->getVal();
    else
      offset = cycles.value - (rollover->getVal() - value->getVal());

    if(break_cycle)
      set_break(true);
  }
}
void StopWatch::set_break(bool b)
{
  if(!b) {
    cycles.clear_break(this);
    break_cycle = 0;
    return;
  }
  if(!enable->getVal())
    return;

  guint64 old_break_cycle = break_cycle;

  if(direction->getVal())
    break_cycle = cycles.value + rollover->getVal()  - get();
  else
    break_cycle = cycles.value + get();

  if(old_break_cycle == break_cycle)
    return;

  if(old_break_cycle)
    cycles.reassign_break(old_break_cycle,break_cycle ,this);
  else
    cycles.set_break(break_cycle ,this);
    
}

void StopWatch::callback()
{
  break_cycle = cycles.value + rollover->getVal();
  cycles.set_break(break_cycle,this);
  cout << " stopwatch break\n";
  get_bp().halt();
}

void StopWatch::callback_print()
{
  cout << "stopwatch\n";
}

