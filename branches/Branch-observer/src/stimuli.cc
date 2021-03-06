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
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string>
#include <list>
#include <vector>

#include "pic-processor.h"
#include "stimuli.h"
#include "stimulus_orb.h"
#include "symbol.h"
#include "xref.h"

list <Stimulus_Node *> node_list;
list <Stimulus_Node *> :: iterator node_iterator;

list <stimulus *> stimulus_list;
list <stimulus *> :: iterator stimulus_iterator;
 
static char num_nodes = 'a';
static char num_stimuli = 'a';


/*
 * stimulus.cc
 *
 * This file contains some rudimentary infrastructure to support simulating
 * the environment outside of the pic. Simple net lists interconnecting pic
 * I/O pins and various signal generators may be created. 
 *
 * Details:
 * There are two basic concepts behind the stimulus code: nodes and stimuli.
 * The nodes are like wires and the stimuli are like sources and loads. The
 * nodes define the interconnectivity between the stimuli. In most cases there
 * will be only two stimuli connected by one node. For example, you may wish
 * to simulate the effects of a clock input connected to porta.0 . In this case,
 * the stimuli would be the external clock and the pic I/O pin.
 */

//------------------------------------------------------------------
Stimulus_Node * find_node (string name)  // %%% FIX ME %%% * name ???
{
  register int i;

  //cout << "searching for " << name << '\n';

  for (node_iterator = node_list.begin();  node_iterator != node_list.end(); node_iterator++)
    {
      Stimulus_Node *t = *node_iterator;
      string s(t->name_str);
      if ( s == name)
	{
	  //cout << "found it\n";
	  return (t);
	}
    }
  return ((Stimulus_Node *)NULL);
}

void add_node(char *node_name)
{
  
  Stimulus_Node *sn = find_node(string(node_name));

  if(sn)
    cout << "Warning node `" << node_name << "' is already in the node list.\n(You can't have duplicate nodes in the node list.)\n";
  else
    sn = new Stimulus_Node(node_name);

}
void add_node(Stimulus_Node * new_node)
{

  //  if(!node_list.find(new_node))
    node_list.push_back(new_node);
}

void dump_node_list(void)
{
  cout << "Node List\n";

  for (node_iterator = node_list.begin();  node_iterator != node_list.end(); node_iterator++)
    {
      Stimulus_Node *t = *node_iterator;
      cout << t->name() << '\n';
      if(t->stimuli)
	{
	  stimulus *s = t->stimuli;
	  while(s)
	    {
	      cout << '\t' << s->name() << '\n';
	      s = s->next;
	    }
	}
    }
}



stimulus * find_stimulus (string name)  // %%% FIX ME %%% * name ???
{
  register int i;

  //cout << "searching for " << name << '\n';

  for (stimulus_iterator = stimulus_list.begin();  stimulus_iterator != stimulus_list.end(); stimulus_iterator++)
    {
      stimulus *t = *stimulus_iterator;
      string s(t->name_str);
      if ( s == name)
	{
	  //cout << "found it\n";
	  return (t);
	}
    }
  return ((stimulus *)NULL);
}

void add_stimulus(stimulus * new_stimulus)
{
  stimulus_list.push_back(new_stimulus);
}

void dump_stimulus_list(void)
{
	cout << "Stimulus List\n";

	for (stimulus_iterator = stimulus_list.begin();  
	     stimulus_iterator != stimulus_list.end(); 
	     stimulus_iterator++)
	{

		stimulus *t = *stimulus_iterator;

		if(t)
		{
			cout << "stimulus ";
			if(t->name())
				cout << t->name();

			if(t->snode)
				cout << " attached to " << t->snode->name();

			cout << '\n';
		}
	}
	cout << "returning from dump\n";
}


//========================================================================

Stimulus_Node::Stimulus_Node(char *n=NULL)
{

  stimuli = NULL;
  warned  = 0;

  if(n)
    {
      strcpy(name_str,n);
    }
  else
    {
      strcpy(name_str,"node   ");

      // give the node a unique name
      name_str[5] = num_nodes++;    // %%% FIX ME %%%
    }

  add_node(this);

}

//
// Add the stimulus 's' to the stimulus list for this node
//

void Stimulus_Node::attach_stimulus(stimulus *s)
{
  stimulus *sptr;

  warned = 0;

  if(stimuli)
    {
      sptr = stimuli;
      bool searching=1;
      while(searching)
	{
	  if(s == sptr)
	    return;      // The stimulus is already attached to this node.

	  if(sptr->next == NULL)
	    {
	      sptr->next = s;
	      // s->next = NULL;  This is done below
	      searching=0;
	    }
	  sptr = sptr->next;
	} 
    }
  else
    stimuli = s;     // This is the first stimulus attached to this node.

  // If we reach this point, then it means that the stimulus that we're
  // trying to attach has just been placed at the end of the the stimulus
  // list for this node. So we need to NULL terminate the singly-linked list.

  s->next = NULL;

  // Now tell the stimulus to attach itself to the node too
  // (If it hasn't already.)

  s->attach(this);
  

}

//
// Search for the stimulus 's' in the stimulus list for this node.
// If it is found, then remove it from the list.
//

void Stimulus_Node::detach_stimulus(stimulus *s)
{
  stimulus *sptr;


  if(!s)          // You can't remove a non-existant stimulus
    return;

  if(stimuli)
    {
      if(s == stimuli)
	{
	  // This was the first stimulus in the list.

	  stimuli = s->next;
	  //  s->detach(this);
	}
      else
	{

	  sptr = stimuli;

	  do
	    {
	      if(s == sptr->next)
		{
		  sptr->next = s->next;
		  //  s->detach(this);
		  return;
		}

	      sptr = sptr->next;
	    } while(sptr);

	} 
    }

  

}


int Stimulus_Node::update(unsigned int current_time)
{

  int node_voltage = 0;

  //cout << "getting state of " << name() << '\n';

  if(stimuli != NULL)
    {
      stimulus *sptr = stimuli;

      // gpsim assumes that there may be multiple sources attached to
      // the node. Usually  one will be dominant. This works well for
      // pullup resistors, open collector drivers, and bi-directional
      // I/O lines. However, if there is more than one source, then
      // the contention is resolved through ohm's law. This works well
      // for analog stuff too.

      while(sptr)
	{
	  node_voltage += sptr->get_voltage(current_time);
	  //cout << sptr->name() << '\n';
	  sptr = sptr->next;
	}
      //cout << "node voltage " << node_voltage << '\n';

      // 'node_voltage' now represents the most up-to-date value of this node.
      // Now, tell all of the stimuli that are interested:
      sptr = stimuli;
      while(sptr)
	{
	  sptr->put_node_state(node_voltage);
	  sptr = sptr->next;
	}

      state = node_voltage;
      return(node_voltage);

    }
  else
    if(!warned)
      {
	cout << "Warning: No stimulus is attached to node: \"" << name_str << "\"\n";
	warned = 1;
      }

  return(0);

}

stimulus::stimulus(char *n = NULL)
{
  strcpy(name_str,"stimulus");
  //cout << "stimulus\n";
  xref = new XrefObject((unsigned int *)&state);
}

void stimulus::put_state_value(int new_state)
{
  put_state(new_state);
  if(xref) xref->update();
}

//========================================================================


square_wave::square_wave(unsigned int p, unsigned int dc, unsigned int ph, char *n=NULL)
{
      
  //cout << "creating sqw stimulus\n";

  if(n)
    strcpy(name_str,n);
  else
    {
      strcpy(name_str,"s1_square_wave");
      name_str[1] = num_stimuli++;
    }


  period = p;   // cycles
  duty   = dc;  // # of cycles over the period for which the sq wave is high
  phase  = ph;  // phase of the sq wave wrt the cycle counter
  state  = 0;   // output 
  time   = 0;   // simulation time
  snode = NULL;
  next = NULL;

  drive  = MAX_DRIVE / 2;

  add_stimulus(this);
}

int square_wave::get_voltage(guint64 current_time)
{
  //  cout << "Getting new state of the square wave.\n";
  if( ((current_time+phase) % period) <= duty)
    return drive;
  else
    return -drive;
}

//========================================================================
//
// triangle_wave

triangle_wave::triangle_wave(unsigned int p, unsigned int dc, unsigned int ph, char *n=NULL)
{
      
  //cout << "creating sqw stimulus\n";

  if(n)
    strcpy(name_str,n);
  else
    {
      strcpy(name_str,"s1_triangle_wave");
      name_str[1] = num_stimuli++;
    }

  if(p==0)  //error
    p = 1;

  // copy the square wave stuff
  period = p;   // cycles
  duty   = dc;  // # of cycles over the period for which the sq wave is high
  phase  = ph;  // phase of the sq wave wrt the cycle counter
  state  = 0;   // output 
  time   = 0;   // simulation time
  drive  = 255*5; // Hard coded for now
  snode = NULL;
  next = NULL;

  //cout << "duty cycle " << dc << " period " << p << " drive " << drive << '\n';
  // calculate the slope and the intercept for the two lines comprising
  // the triangle wave:

  if(duty)
    m1 = float(drive)/duty;
  else
    m1 = float(drive)/period;   // m1 will not be used if the duty cycle is zero

  b1 = 0;

  if(period != duty)
    m2 = float(drive)/(float(duty) - float(period));
  else
    m2 = float(drive);

  b2 = -float(period) * m2;

  //cout << "m1 = " << m1 << " b1 = " << b1 << '\n';
  //cout << "m2 = " << m2 << " b2 = " << b2 << '\n';

  add_stimulus(this);
}

int triangle_wave::get_voltage(guint64 current_time)
{
  //cout << "Getting new state of the triangle wave.\n";

  int t = (current_time+phase) % period;

  /*
  if( t <= duty)
    return int(b1 + m1 * t);
  else
    return int(b2 + m2 * t);
  */

  // debug stuff:
  int ret_val;

  if( t <= duty)
    ret_val = MAX_ANALOG_DRIVE*int(b1 + m1 * t);
  else
    ret_val = MAX_ANALOG_DRIVE*int(b2 + m2 * t);
  
  //  cout << "Triangle wave: t = " << t << " value = " << ret_val << '\n';
  return ret_val;

}

//========================================================================
//
// asynchronus_stimulus
//
// an asynchronous stimulus is a stream of data that can change values at
// arbitrary times. An array called 'transition_cycles' stores the times
// and an array 'values' stores the values.
//   When first initialized, the stimulus is driven to its initial state.
// A break point is set on the cycle counter for the next cpu cycle that
// the stimulus is expected to change values. When the break occurs,
// the current state is updated to the next value  and then a break is set
// for the next expect change. This cycle occurs until all of the values
// have been generated. When the end is reached, the asynchronous stimulus
// will restart from the beginning. The member variable 'period' describes
// the magnitude of the rollover (if it's zero then there is no rollover).
//   
//

void asynchronous_stimulus::callback(void)
{

  current_state = next_state;
  guint64 current_cycle = future_cycle;

  //cout << "asynchro cycle " << current_cycle << "  state " << current_state << '\n';

  // If we've passed through all of the states
  // then start over from the beginning.

  if( (++current_index) >= max_states)
    {
      if(period == 0)     // If the period is zero then we don't want to 
	return;           // regenerate the pulse stream.


      // we've rolled over, so let's start from the beginning.
      // The start cycle for this next set of pulses is shifted over 
      // by 'period' cycles.
      current_index = 0;
      start_cycle += period;
      future_cycle = *transition_cycles + start_cycle;

      //cout << "  stimulus rolled over\n";
      //cout << "   next start_cycle " << start_cycle << "  period " << period << '\n';
    }
  else
    {
      // toggle the next state and get the cycle at which it will change

      future_cycle = *(transition_cycles + current_index) + start_cycle;

    }

  if(future_cycle <= current_cycle)
    {
      // There's an error in the data. Set a break on the next simulation cycle
      // and see if it can be resolved.
      future_cycle = current_cycle+1;
    }
  else
      next_state = *(values + current_index);

  cpu->cycles.set_break(future_cycle, this);

  //cout <<"  next transition = " << future_cycle << '\n';
  //cout <<"  next value = " << next_state << '\n';

  snode->update(current_cycle);
}

int asynchronous_stimulus::get_voltage(guint64 current_time) 
{
  //cout << "asy getting state "  << current_state << '\n';
  
  return current_state;
}

// start the asynchronous stimulus. This should be called after the user
// has completely initialized the stimulus.

// KNOWN BUG %%%FIX ME%%% - the asynchronous stimulus assumes that the
// data is sorted correctly (that is chronologically). If it isn't, weird
// things will happen.

void asynchronous_stimulus::start(void)
{

  if(cpu  && transition_cycles)
    {

      if(max_states == 0)
	return;

      current_index = 0;

      if(digital)
	initial_state = initial_state ? (MAX_DRIVE / 2) : -(MAX_DRIVE / 2);

      current_state = initial_state;

      start_cycle = cpu->cycles.value + phase;
      future_cycle = *(transition_cycles + current_index) + start_cycle;

      if( (period!=0) && (period<transition_cycles[max_states-1]))
	cout << "Warning: Asynchronous Stimulus has a period shorter than its last event.\n";
      // This means that the stimulus will not rollover.\n";

      cpu->cycles.set_break(future_cycle, this);

      if(verbose) {
	cout << "Asynchronous stimulus\n";
	cout << "  states = " << max_states << '\n';
      }
      for(int i=0; i<max_states; i++)
	{
	  if(digital)
	    {
	      if(values[i])
		values[i] = MAX_DRIVE/2;
	      else
		values[i] = -MAX_DRIVE/2;
	    }
	  if(verbose&2)
	    cout << "    " << transition_cycles[i] <<  '\t' << values[i] << '\n';
	}

      cout << "period = " << period << '\n'
	   << "phase = " << phase << '\n'
	   << "start_cycle = " << start_cycle << '\n'
	   << "Next break cycle = " << future_cycle << '\n';

      next_state = *values;
    }

}

// Create an asynchronous stimulus. If invoked with a non-null name, then
// give the stimulus that name, other wise create one.
// Note that most of the stimulus' initialization must be performed outside
// of the constructor.

asynchronous_stimulus::asynchronous_stimulus(char *n=NULL)
{
  cpu = NULL;
  transition_cycles = NULL;
  values = NULL;
  snode = NULL;
  next = NULL;

  if(n)
    strcpy(name_str,n);
  else
    {
      strcpy(name_str,"a1_asynchronous_stimulus");
      name_str[1] = num_stimuli++;
    }


  //  drive = MAX_DRIVE / 2;
  add_stimulus(this);

}

//========================================================================
dc_supply::dc_supply(char *n)
{

  snode = NULL;
  next = NULL;

  if(n)
    strcpy(name_str,n);
  else
    {
      strcpy(name_str,"v1_supply");
      name_str[1] = num_stimuli++;
    }


  drive = MAX_DRIVE / 2;
  add_stimulus(this);

}

//========================================================================
//


IOPIN::IOPIN(IOPORT *i, unsigned int b)
{
  iop = i;
  iobit=b;
  state = 0;
  l2h_threshold = 100;
  h2l_threshold = -100;
  drive = 0;
  snode = NULL;
  //  cout << "IOPIN constructor called \n";

  if(iop) {
    iop->attach_iopin(this,b);

    strcpy(name_str, iop->name());
    char bs[2];
    bs[0] = iobit+'0';
    bs[1] = 0;
    strcat(name_str,bs);
  }

  add_stimulus(this);


}

IOPIN::IOPIN(void)
{

  cout << "IOPIN default constructor\n";

}

void IOPIN::put_state_value(int new_state)
{
  if(iop)
    iop->setbit_value(iobit, new_state &1);
  //put_state(new_state);
  if(xref)
    xref->update();
}

void IOPIN::attach(Stimulus_Node *s)
{

  iop->attach_node(s,iobit);

  snode = s;
}

//========================================================================
//
IO_input::IO_input(IOPORT *i, unsigned int b)
  : IOPIN(i,b)
{

  state = 0;
  drive = 0;

}

IO_input::IO_input(void)
{
  cout << "IO_input default constructor\n";


}
void IO_input::toggle(void)
{
  if(iop) {
    iop->setbit(iobit, 1^iop->get_bit(iobit));
    if(iop->xref)
      iop->xref->update();
    state = iop->get_bit(iobit);
  }
  else
    state ^= 1;

  if(xref)
    xref->update();
}

/*************************************
 *  int IO_input::get_voltage(guint64 current_time)
 *
 *  If this iopin has a stimulus attached to it then
 * the voltage will be dictated by the stimulus. Otherwise,
 * the voltage is determined by the state of the ioport register
 * that is inside the pic. For an input (like this), the pic code
 * that is being simulated can not change the state of the I/O pin.
 * However, the user has the ability to modify the state of
 * this register either by writing directly to it in the cli,
 * or by clicking in one of many places in the gui.
 */
int IO_input::get_voltage(guint64 current_time)
{
  // The last time the stimulus to which this node is/maybe attached,
  // the drive was updated.

  if(snode)
    return drive;
  else
    return ( (iop->value & (1<<iobit)) ? drive : -drive);

}

void IO_input::put_state( int new_digital_state)
{
  //cout << "IO_input::put_state() new_state = " << new_digital_state <<'\n';

  if( (new_digital_state != 0) && (state < h2l_threshold)) {

    //cout << " driving I/O line high \n";
    state = l2h_threshold + 1;
    iop->setbit(iobit,1);

  } 
  else if((new_digital_state == 0) && (state > l2h_threshold)) {

    //cout << " driving I/O line low \n";
    state = h2l_threshold - 1;
    iop->setbit(iobit,0);

  }
  //else cout << " no change in IO_input state\n";

}

// this IO_input is attached to a node that has just been updated.
// The node is now trying to update this stimulus.

void IO_input::put_node_state( int new_state)
{
  //cout << "IO_input::put_node_state() " << " node = " << name() << " new_state = " << new_state <<'\n';


  // No need to proceed if we already in the new_state.

  if(new_state == state)
    return;


  if(iop) {

    // If the I/O pin to which this stimulus is mapped is at a logic 
    // high AND the new state is below the high-to-low threshold
    // then we need to drive the I/O line low.
    //
    // Similarly, if the I/O line is low and the new_state is above
    // the low-to-high threshold, we need to drive it low.

    if(iop->get_bit(iobit)) {
      if(new_state < h2l_threshold)
	iop->setbit(iobit,0);
    } else {
      if(new_state > l2h_threshold)
	iop->setbit(iobit,1);
    }
  }

  state = new_state;
}

//========================================================================
//
IO_bi_directional::IO_bi_directional(IOPORT *i, unsigned int b)
  : IO_input(i,b)
{
  //  source = new source_stimulus();

  state = 0;
  drive = MAX_DRIVE / 2;
  driving = 0;

  //sprintf(name_str,"%s%n",iop->name_str,iobit);
  //cout << name_str;
  // cout << "IO_bi_directional\n";
}


void IO_bi_directional::put_state( int new_digital_state)
{
  //cout << "IO_bi_directional::put_state() new_state = " << new_digital_state <<'\n';

  // If the bi-directional pin is an output then driving is TRUE.
  if(driving) {

    // If the new state to which the stimulus is being set is different than
    // the current state of the bit in the ioport (to which this stimulus is
    // mapped), then we need to update the ioport.

    if((new_digital_state!=0) ^ ( iop->value & (1<<iobit))) {

      iop->setbit(iobit,new_digital_state);

      // If this stimulus is attached to a node, then let the node be updated
      // with the new state as well.
      if(snode)
	snode->update(0);
      // Note that this will auto magically update
      // the io port.


    }

  }
  else {

    // The bi-directional pin is configured as an input. So let the parent
    // input class handle it.
    IO_input::put_state(new_digital_state);

  }

}


IO_bi_directional::IO_bi_directional(void)
{
  cout << "IO_bi_directional constructor shouldn't be called\n";
}

IO_bi_directional_pu::IO_bi_directional_pu(IOPORT *i, unsigned int b)
  : IO_bi_directional(i, b)
{

  pull_up_resistor = new resistor();
  pull_up_resistor->drive = 10;    // %%% FIX ME %%%


  state = 0;
  drive  = MAX_DRIVE / 2;
  driving = 0;

  //  sprintf(name_str,"%s%n",iop->name_str,iobit);
  //cout << name_str;

}

int IO_bi_directional::get_voltage(guint64 current_time)
{
  //cout << "Getting new state of a bi-di IO pin "<< iobit<<'\n';

  if(driving || !snode)
    {
      if( iop->value & (1<<iobit))
	{
	  //cout << " high\n";
	  return drive;
	}
      else
	{
	  //cout << " low\n";
	  return -drive;
	}
    }
  else
    {
      // This node is not driving (because it's configured
      // as an input). There is a stimulus attached to it, so
      // don't upset the 'node summing'. I guess we could return
      // a input leakage value...
      return 0;
    }

}


//---------------
//::update_direction(unsigned int new_direction)
//
//  This is called when a new value is written to the tris register
// with which this bi-direction pin is associated.

void IO_bi_directional::update_direction(unsigned int new_direction)
{

  //cout << "IO_bi_direction::update_direction\n";

  if(new_direction)
    driving = 1;
  else
    driving = 0;

}

//---------------
//void IO_bi_directional::change_direction(unsigned int new_direction)
//
//  This is called by the gui to change the direction of an 
// io pin. 

void IO_bi_directional::change_direction(unsigned int new_direction)
{

  //cout << __FUNCTION__ << '\n';

  iop->tris->setbit(iobit, new_direction & 1);

  if(xref)
    xref->update();
}

int IO_bi_directional_pu::get_voltage(guint64 current_time)
{
  //cout << "Getting new state of a bi-di-pu IO pin "<< iobit;

  if(driving | !snode)
    {
      if( iop->value & (1<<iobit))
	{
	  //cout << " high\n";
	  return drive;
	}
      else
	{
	  //cout << " low\n";
	  return -drive;
	}
    }
  else
    {
      //cout << " pulled up\n";
      return (pull_up_resistor->get_voltage(current_time));
    }

}


IO_open_collector::IO_open_collector(IOPORT *i, unsigned int b)
  : IO_input(i,b)
{

  drive = MAX_DRIVE / 2;

  state = 0;

  //  sprintf(name_str,"%s%n",iop->name_str,iobit);
  //cout << name_str;
  strcpy(name_str, iop->name());
  char bs[2];
  bs[0] = iobit+'0';
  bs[1] = 0;
  strcat(name_str,bs);
  add_stimulus(this);
}


int IO_open_collector::get_voltage(guint64 current_time)
{
  //cout << "Getting new state of an open collector IO pin port "<< iop->name() << " bit " << iobit<<'\n';

  if(driving )
    {
      if( iop->value & (1<<iobit))
	{
	  //cout << "high\n";
	  return 0;
	}
      else
	{
	  //cout << "low\n";
	  return (-MAX_DRIVE/2);
	}
    }
  else
    {
      //cout << "open collector is configured as an input\n";
      if(snode)
	return 0;
      else
	return ( (iop->value & (1<<iobit)) ? drive : (-drive));
    }
}

void IO_open_collector::update_direction(unsigned int new_direction)
{
  //cout << "IO_open_collector::" << __FUNCTION__ << " to new direction " << new_direction << '\n';
  if(new_direction)
    driving = 1;
  else
    driving = 0;

}

//---------------
//void IO_open_collector::change_direction(unsigned int new_direction)
//
//  This is called by the gui to change the direction of an 
// io pin. 

void IO_open_collector::change_direction(unsigned int new_direction)
{

  //cout << "IO_open_collector::" << __FUNCTION__ << '\n';

  iop->tris->setbit(iobit, new_direction & 1);

  if(xref)
    xref->update();
}

//*****************************************************************
// *** KNOWN CHANGE ***
//  Support functions that will get replaced by the CORBA interface.
//  

source_stimulus *last_stimulus=NULL;

void create_stimulus(int type, char *name)
{
  //cout << "Got request to create a stimulus \n";

  asynchronous_stimulus *asy;
  square_wave *sqw;
  triangle_wave *tri;

  switch(type)
    {
    case NEW_SQW:
      sqw = new square_wave(0,0,0,name);
      last_stimulus = sqw;
      break;

    case NEW_ASY:
      asy = new asynchronous_stimulus(name);
      last_stimulus = asy;
      break;

    case NEW_TRI:
      tri = new triangle_wave(0,0,0,name);
      last_stimulus = tri;
      break;
    }
  last_stimulus->period = 0;
  last_stimulus->duty   = 0;
  last_stimulus->phase  = 0;
  last_stimulus->initial_state  = 0;
  last_stimulus->start_cycle    = 0;

}

void stimorb_period(unsigned int _period)
{
  if(last_stimulus)
    last_stimulus->period = _period;
}

void stimorb_duty(unsigned int _duty)
{
  if(last_stimulus)
    last_stimulus->duty = _duty;
}

void stimorb_phase(unsigned int _phase)
{
  if(last_stimulus)
    last_stimulus->phase = _phase;
}

void stimorb_initial_state(unsigned int _initial_state)
{
  if(last_stimulus)
    last_stimulus->initial_state = _initial_state;
}

void stimorb_start_cycle(unsigned int _start_cycle)
{
  if(last_stimulus)
    last_stimulus->start_cycle = _start_cycle;
}

void stimorb_name(char *_name)
{
  //cout << "changing name to " << _name << '\n';

  //cout << "before " << last_stimulus->name();
  if(last_stimulus)
    strcpy(last_stimulus->name_str,_name);
  //cout << " after " << last_stimulus->name() << '\n';

}


void stimorb_asy(int digital, pic_processor *cpu,vector<StimulusDataType> temp_array )
{
  if(!last_stimulus)
    return;
  asynchronous_stimulus *asy;

  asy  =  (asynchronous_stimulus *) last_stimulus;

  if(temp_array.size())
    {

      asy->digital = digital;
      asy->max_states =  temp_array.size()/2;
      asy->transition_cycles = new guint64[ asy->max_states];
      asy->values            = new int[ asy->max_states];

      for(int j=0; j<=2*asy->max_states; j++)
	{
	  StimulusDataType dp = temp_array[j];
	  int new_data;

	  switch(dp.data_type) {

	  case STIMULUS_DPT_INT:
	    new_data = (dp.data_point.i & 0xfffffff);
	    break;

	  case STIMULUS_DPT_FLOAT:
	    new_data = (int)MAX_ANALOG_DRIVE * dp.data_point.f;
	    break;

	  default:
	    new_data = 0;
	    break;
	  }

	  if(j&1)
	    asy->values[j>>1] = new_data;
	  else
	    asy->transition_cycles[j>>1] = new_data;
	}

      asy->cpu = cpu;

      //      if (options_entered & STIM_IOPORT)
      //	is->ioport->attach_stimulus(asy, bit_pos);

      //cout << "Starting asynchronous stimulus\n";
      asy->start();
    }
}

// Char list.
// Here's a singly linked-list of char *'s.

struct char_list {
  char *name;
  char_list *next;
};

void stimorb_attach(char *node, char_list *stimuli)
{
  if(verbose&2)
    cout << " doing an attach (stimuli.cc) node: " << node << '\n';

  string s = string(node);
  Stimulus_Node *sn = find_node (s);

  if(sn)
    {
      int i=2;
	
      stimulus *st;
      while(stimuli)
	{
	  s = string(stimuli->name);
	  st = find_stimulus(s);
	  if(st) {
	    sn->attach_stimulus(st);
	    if(verbose&2)
	      cout << " attaching stimulus: " << s << '\n';
	  }
	  stimuli = stimuli->next;
	}
    }

}
