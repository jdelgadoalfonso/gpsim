/*
   Copyright (C) 1998 T. Scott Dattalo
   Copyright (C) 2006 Roy R Rankin

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
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string>
#include <list>
#include <vector>
#include <sstream>

#include <math.h>

#include "../config.h"
#include "pic-processor.h"
#include "stimuli.h"
#include "stimulus_orb.h"
#include "symbol.h"
#include "interface.h"
#include "errors.h"
#include "cmd_gpsim.h"


static char num_nodes = 'a';
static char num_stimuli = 'a';
void  gpsim_set_break_delta(guint64 delta, TriggerObject *f=0);


extern Processor *active_cpu;
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

void Stimulus_Node::new_name(const char *cPname)
{
  // JRH - Perhaps this could be migrated into gpsimObject
  const char *cPoldName = name().c_str();
  if(name_str.empty()) {
    // Assume never in symbol table.
    // Every named stimulus goes into the symbol table.
    gpsimObject::new_name(cPname);
    symbol_table.add_stimulus_node(this);
    return;
  }
  if(symbol_table.Exist(cPoldName)) {
    // The symbol is in the symbol table. Since the
    // symbol table is ordered we need to let the
    // symbol table rename the object to maintain
    // ordering. Yuk.
    // Note that rename() will call Stimulus_Node::new_name()
    // after the symbol is removed. This recursive
    // call will then enter the branch that calls
    // gpsimObject::new_name(). The simulus with
    // its new name is added into the symbol table.
    symbol_table.rename(cPoldName,cPname);
  }
  else {
    gpsimObject::new_name(cPname);
  }

}
void Stimulus_Node::new_name(string &rName)
{
  new_name(rName.c_str());
}

double Stimulus_Node::get_nodeVoltage()
{
  if (future_cycle) // RC calculation in progress, get current value
    callback();
  return(voltage);
}

void dump_node_list(void)
{
  cout << "Node List\n";
  Symbol_Table &ST = get_symbol_table();
  Symbol_Table::node_symbol_iterator it;
  Symbol_Table::node_symbol_iterator itEnd = ST.endNodeSymbol();
  for(it = ST.beginNodeSymbol(); it != itEnd; it++) {
    Stimulus_Node *t = (*it)->getNode();
    cout << t->name() << " voltage = " << t->get_nodeVoltage() << "V\n";
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

void dump_bus_list(void)
{
  dump_node_list();
}


void add_bus(char *bus_name)
{

  /*
  Stimulus_Node *sn = find_node(string(node_name));

  if(sn)
    cout << "Warning node `" << node_name << "' is already in the node list.\n(You can't have duplicate nodes in the node list.)\n";
  else
    sn = new Stimulus_Node(node_name);
  */
  cout << "add_bus -- not supported\n";
}


void dump_stimulus_list(void)
{
  cout << "Stimulus List\n";
  Symbol_Table &ST = get_symbol_table();
  Symbol_Table::stimulus_symbol_iterator it;
  Symbol_Table::stimulus_symbol_iterator itEnd = ST.endStimulusSymbol();
  for(it = ST.beginStimulusSymbol(); it != itEnd; it++) {
    stimulus *t = (*it)->getStimulus();
    if(t) {
      cout << t->name();
      //if(t->snode)
      // cout << " attached to " << t->snode->name();
      t->show();
      cout << '\n';
    }
  }

}


//========================================================================

Stimulus_Node::Stimulus_Node(const char *n)
  : TriggerObject(0)
{

  stimuli = 0;
  nStimuli = 0;
  voltage = 0;
  warned  = 0;
  current_time_constant = 0.0;
  delta_voltage = 0.0;
  cap_start_cycle = 0;
  future_cycle = 0;
  minThreshold = 0.1; // volts
  min_time_constant = 1000; // in cycles
  bSettling = false;  
  if(n)
    {
      new_name(n);
    }
  else
    {
      char name_str[100];
      snprintf(name_str,sizeof(name_str),"node%d",num_nodes);
      num_nodes++;    // %%% FIX ME %%%
      new_name(name_str);
    }

  gi.node_configuration_changed(this);
}

Stimulus_Node::~Stimulus_Node()
{
  stimulus *sptr;

  sptr = stimuli;
  while(sptr) {
    sptr->detach(this);
    sptr = sptr->next;
  }

  Value *vpNodeSym = symbol_table.remove(name());
  if(vpNodeSym != NULL)
    delete vpNodeSym;
}

Stimulus_Node * Stimulus_Node::construct(const char * psName)
{
  Stimulus_Node *sn = get_symbol_table().findNode(psName);
  if(sn) {
    cout << "Warning node `" << psName
         << "' is already in the node list.\n"
            "(You can't have duplicate nodes in the node list.)\n";
    return NULL;
  }
  else {
    sn = new Stimulus_Node(psName);
  }
  return sn;
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
      int nTotalStimuliConnected = 1;

      while(searching)
      {
        if(s == sptr)
          return;      // The stimulus is already attached to this node.

        nTotalStimuliConnected++;
        if(sptr->next == 0)
        {
          sptr->next = s;
          // s->next = 0;  This is done below
          searching=0;
        }
        sptr = sptr->next;
      }

      nStimuli = nTotalStimuliConnected;
    }
  else
    {
      stimuli = s;     // This is the first stimulus attached to this node.
      nStimuli = 1;
    }

  // If we reach this point, then it means that the stimulus that we're
  // trying to attach has just been placed at the end of the the stimulus
  // list for this node. So we need to 0 terminate the singly-linked list.

  s->next = 0;

  // Now tell the stimulus to attach itself to the node too
  // (If it hasn't already.)

  s->attach(this);
  
  gi.node_configuration_changed(this);

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

  if(stimuli) {
    if(s == stimuli) {

      // This was the first stimulus in the list.

      stimuli = s->next;
      s->detach(this);
      nStimuli--;

    } else {

      sptr = stimuli;

      do {
        if(s == sptr->next) {

          sptr->next = s->next;
          s->detach(this);
          nStimuli--;
          //gi.node_configuration_changed(this);
          return;
        }

        sptr = sptr->next;
      } while(sptr);

    } 
  }
}

//------------------------------------------------------------------------
//
// Stimulus_Node::update(guint64 current_time)
//
// update() is called whenever a stimulus attached to this node changes
// states. 

void Stimulus_Node::update(guint64 current_time)
{
  // So far, 'update' only applies to the current time. 
  update();
}

//------------------------------------------------------------------------
// refresh() - compute the Thevenin voltage and Thevenin impedance
//
void Stimulus_Node::refresh()
{
  if(stimuli) {

    stimulus *sptr = stimuli;

    initial_voltage = get_nodeVoltage();

    switch (nStimuli) {

    case 0:
      // hmm, strange nStimuli is 0, but the stimuli pointer is non null.
      break;

    case 1:
      // Only one stimulus is attached.
      finalVoltage = sptr->get_Vth();   // RP - was just voltage
      Zth =  sptr->get_Zth();
      break;

    case 2:
      // 2 stimuli are attached to the node. This is the typical case
      // and we'll optimize for it.
      {
      stimulus *sptr2 = sptr ? sptr->next : 0;
      if(!sptr2)
        break;     // error, nStimuli is two, but there aren't two stimuli
      
      double V1,Z1,C1;
      double V2,Z2,C2;
      sptr->getThevenin(V1,Z1,C1);
      sptr2->getThevenin(V2,Z2,C2);
      finalVoltage = (V1*Z2  + V2*Z1) / (Z1+Z2);
      Zth = Z1*Z2/(Z1+Z2);
      Cth = C1+C2;
      
      }
      break;

    default:
      {
      /*
        There are 3 or more stimuli connected to this node. Recall
        that these are all in parallel. The Thevenin voltage and 
        impedance for this is:

        Thevenin impedance:
        Zt = 1 / sum(1/Zi)

        Thevenin voltage:

        Vt = sum( Vi / ( ((Zi - Zt)/Zt) + 1) )
        = sum( Vi * Zt /Zi)
        = Zt * sum(Vi/Zi)
      */

      double conductance=0.0;	// Thevenin conductance.
      Cth=0;
      finalVoltage=0.0; 

      //cout << "multi-node summing:\n";
      while(sptr) {

	double V1,Z1,C1;
	sptr->getThevenin(V1,Z1,C1);
        /*
        cout << " N: " <<sptr->name() 
        << " V=" << V1
        << " Z=" << Z1
        << " C=" << C1 << endl;
        */

        double Cs = 1 / Z1;
        finalVoltage += V1 * Cs;
        conductance += Cs;
        Cth += C1;
        sptr = sptr->next;
      }
      Zth = 1.0/conductance;
      finalVoltage *= Zth;
      }
    }

    current_time_constant = Cth * Zth;
    if(((guint64)(current_time_constant*get_cycles().instruction_cps()) 
      < min_time_constant) 
      ||
      (fabs(finalVoltage - voltage) < minThreshold))
    {
      if (verbose)
        cout << "Stimulus_Node::refresh " << name() << " use DC " <<
		finalVoltage << " as current_time_constant=" <<
          current_time_constant << endl;

      if (future_cycle)	// callback is active
      {
	get_cycles().clear_break(this);
      }

      voltage = finalVoltage;
      future_cycle = 0;

    } else {
	settlingTimeStep = (guint64) (0.11 * 
	  get_cycles().instruction_cps() * current_time_constant);
	voltage = initial_voltage;

        if (verbose)
	  cout << "Stimulus_Node::refresh " << name() << " settlingTimeStep=" 
	    << settlingTimeStep << " voltage=" << voltage << " Finalvoltage=" 
	    << finalVoltage << endl;
  /*
	If future_cycle is not 0 we are in the middle of an RC
	calculation, but an input condition has changed.
  */

      if (future_cycle)
      {
	callback();
      }
      else
      {
	cap_start_cycle = get_cycles().value; 
        future_cycle = cap_start_cycle + settlingTimeStep;
        get_cycles().set_break(future_cycle,this);
      }
    }

  }


}

//------------------------------------------------------------------------
// updateStimuli
// 
// drive all the stimuli connected to this node.

void Stimulus_Node::updateStimuli()
{
  stimulus *sptr = stimuli;

  while(sptr) {
    sptr->set_nodeVoltage(voltage);
    sptr = sptr->next;
  }
}

void Stimulus_Node::update()
{

  if(stimuli) {

    refresh();
    updateStimuli();
  }
}

void Stimulus_Node::set_nodeVoltage(double v)
{
  voltage = v;
  updateStimuli();
}

//------------------------------------------------------------------------
void Stimulus_Node::callback()
{

    if (verbose)
	callback_print();

    initial_voltage = voltage;
    double Time_Step;
    double expz;
      //
      // increase time step as capacitor charges more slowly as final
      // voltage is approached.
      //
                                                                            
      //
      // The following is an exact calculation, assuming no circuit
      // changes,  regardless of time step.
      //
      Time_Step = (get_cycles().value - cap_start_cycle)/
        (get_cycles().instruction_cps()*current_time_constant);
      expz = exp(-Time_Step);
      voltage = finalVoltage* (1.-expz) + voltage * expz;

      if (verbose)
  	cout << "\tVoltage was " << initial_voltage << "V now "
          << voltage << "V\n";

      if (fabs(finalVoltage - voltage) < minThreshold) {
    	voltage = finalVoltage;
        future_cycle = 0;
	if (verbose)
          cout << "\t" << name() << 
	    " Final voltage " << finalVoltage << " reached at " 
	    << get_cycles().value << " cycles\n";
      } 
      else if(get_cycles().value >= future_cycle) // got here via break
      {
        settlingTimeStep  = (guint64) (1.5 * settlingTimeStep);
        cap_start_cycle = get_cycles().value;
	future_cycle = cap_start_cycle + settlingTimeStep;
    	get_cycles().set_break(future_cycle, this);
        if (verbose)
	  cout << "\tBreak reached at " << cap_start_cycle << 
	    " cycles, next break set for " 
	    << future_cycle << " delta=" << settlingTimeStep << endl;
      }
      else	// updating value before break don't increase step size
      {
        cap_start_cycle = get_cycles().value;
        get_cycles().reassign_break(future_cycle, 
		cap_start_cycle + settlingTimeStep, this);
	future_cycle = get_cycles().value + settlingTimeStep;
        if (verbose)
	  cout << "\tcallback called at " << cap_start_cycle << 
	    " cycles, next break set for " << future_cycle << " delta=" 
	    << settlingTimeStep << endl;
      }

  updateStimuli();
}

//------------------------------------------------------------------------
void Stimulus_Node::callback_print()
{
  cout << "Node: " << name() ;
  TriggerObject::callback_print();
}
//------------------------------------------------------------------------
void Stimulus_Node::time_constant(double new_tc)
{
  min_time_constant = (unsigned int)(new_tc*get_cycles().instruction_cps());
}

//------------------------------------------------------------------------
stimulus::stimulus(const char *cPname,double _Vth, double _Zth)
  : snode(NULL), next(NULL), Vth(_Vth), Zth(_Zth)
{
  if(cPname && *cPname)
    new_name(cPname);

  snode = 0;
  bDrivingState = false;
  bDriving = false;
  next = 0;

  Cth = 0;     // Farads
  nodeVoltage = 0.0; // Volts
}
void stimulus::new_name(const char *cPname)
{
  const char *cPoldName = name().c_str();
  if(name_str.empty() && cPname != NULL && *cPname != 0) {
    // Assume never in symbol table.
    // Every named stimulus goes into the symbol table.
    gpsimObject::new_name(cPname);
    symbol_table.add_stimulus(this);
    return;
  }
  if(symbol_table.Exist(cPoldName)) {
    // The symbol is in the symbol table. Since the
    // symbol table is ordered we need to let the
    // symbol table rename the object to maintain
    // ordering. Yuk.
    // Note that rename() will call stimulus::new_name()
    // after the symbol is removed. This recursive
    // call will then enter the branch that calls
    // gpsimObject::new_name(). The simulus with
    // its new name is added into the symbol table.
    symbol_table.rename(cPoldName,cPname);
  }
  else {
    gpsimObject::new_name(cPname);
  }

}
void stimulus::new_name(string &rName)
{
  new_name(rName.c_str());
}

stimulus::~stimulus(void)
{
  if(snode)
    snode->detach_stimulus(this);

  Value *vpNodeSym = symbol_table.remove(name());
  if(vpNodeSym != NULL)
    delete vpNodeSym;
}

void stimulus::show()
{
  GetUserInterface().DisplayMessage(toString().c_str());
}

string stimulus::toString() 
{
  ostringstream s;

  s << " stimulus ";
  if(snode)
    s << " attached to " << snode->name();
  s << endl
    << " Vth=" << get_Vth() << "V"
    << " Zth=" << get_Zth() << " ohms"
    << " Cth=" << get_Cth() << "F"
    << " nodeVoltage= " << get_nodeVoltage() << "V"
    << endl 
    << " Driving=" << getDriving()
    << " drivingState=" << getDrivingState()
    << " drivenState=" << getDrivenState()
    << " bitState=" << getBitChar();

  return s.str();
}
void stimulus::attach(Stimulus_Node *s)
{
  detach(snode);
  snode = s;
}
void stimulus::detach(Stimulus_Node *s)
{
  if(snode == s)
    snode = 0; 
}

void   stimulus::getThevenin(double &v, double &z, double &c)
{
  v = get_Vth();
  z = get_Zth();
  c = get_Cth();
}

//========================================================================
// 
PinMonitor::PinMonitor()
{
}

PinMonitor::~PinMonitor()
{
}

void PinMonitor::addSink(SignalSink *new_sink)
{
  if(new_sink)
    sinks.push_back(new_sink);
}

void PinMonitor::removeSink(SignalSink *pSink)
{
  if(pSink)
    sinks.remove(pSink);
}

//========================================================================


square_wave::square_wave(unsigned int p, unsigned int dc, unsigned int ph, const char *n)
{
      
  //cout << "creating sqw stimulus\n";

  if(n)
    new_name(n);
  else
    {
      char name_str[100];
      snprintf(name_str,sizeof(name_str),"s%d_square_wave",num_stimuli);
      num_stimuli++;
      new_name(name_str);
    }


  period = p;   // cycles
  duty   = dc;  // # of cycles over the period for which the sq wave is high
  phase  = ph;  // phase of the sq wave wrt the cycle counter
  time   = 0;   // simulation time
  snode = 0;
  next = 0;

}

double square_wave::get_Vth()
{
  guint64 current_time = get_cycles().value;

  if(verbose & 1)
    cout << "Getting new state of the square wave.\n";

  if( ((current_time+phase) % period) <= duty)
    return  Vth;
  else
    return  0.0;
}


//========================================================================
//
// triangle_wave

triangle_wave::triangle_wave(unsigned int p, unsigned int dc, unsigned int ph, const char *n)
{
      
  //cout << "creating sqw stimulus\n";

  if(n)
    new_name(n);
  else
    {
      char name_str[100];
      snprintf(name_str,sizeof(name_str),"s%d_triangle_wave",num_stimuli);
      num_stimuli++;
      new_name(name_str);
    }

  if(p==0)  //error
    p = 1;

  // copy the square wave stuff
  period = p;   // cycles
  duty   = dc;  // # of cycles over the period for which the sq wave is high
  phase  = ph;  // phase of the sq wave wrt the cycle counter
  time   = 0;   // simulation time
  snode = 0;
  next = 0;

  //cout << "duty cycle " << dc << " period " << p << " drive " << drive << '\n';

  // calculate the slope and the intercept for the two lines comprising
  // the triangle wave:

  if(duty)
    m1 = Vth/duty;
  else
    m1 = Vth/period;   // m1 will not be used if the duty cycle is zero

  b1 = 0;

  if(period != duty)
    m2 = Vth/(duty - period);
  else
    m2 = Vth;

  b2 = -m2 * period;

  //cout << "m1 = " << m1 << " b1 = " << b1 << '\n';
  //cout << "m2 = " << m2 << " b2 = " << b2 << '\n';

}

double triangle_wave::get_Vth()
{
  guint64 current_time = get_cycles().value;

  //cout << "Getting new state of the triangle wave.\n";

  guint64 t = (current_time+phase) % period;

  double ret_val;

  if( t <= duty)
    ret_val = b1 + m1 * t;
  else
    ret_val = b2 + m2 * t;
  
  //  cout << "Triangle wave: t = " << t << " value = " << ret_val << '\n';
  return ret_val;

}

//========================================================================
//
// Event

Event::Event(void)
{
  current_state = 0;
}

//========================================================================
//
void Event::callback(void)
{


  // If there's a node attached to this stimulus, then update it.
  if(snode)
    snode->update();

  // If the event is inactive.

  if(current_state == 0) {
    get_cycles().set_break_delta(1,this);
    current_state = 1;
  } else {
    current_state = 0;
  }

}

void source_stimulus::callback_print(void)
{
  cout << "stimulus " << name() << " CallBack ID " << CallBackID << '\n';

}


void source_stimulus::callback(void)
{
  cout << "shouldn't be called\n";
}

void source_stimulus::show()
{
  stimulus::show();
}
void source_stimulus::put_period(Value *pValue)
{
  if (pValue)
    pValue->get(period);
}
void source_stimulus::put_duty(Value *pValue)
{
  if (pValue)
    pValue->get(duty);
}
void source_stimulus::put_phase(Value *pValue)
{
  if (pValue)
    pValue->get(phase);
}
void source_stimulus::put_initial_state(Value *pValue)
{
  if (pValue)
    pValue->get(initial_state);
}
void source_stimulus::put_start_cycle(Value *pValue)
{
  if (pValue)
    pValue->get(start_cycle);
}

//========================================================================
//
IOPIN::IOPIN(const char *_name,
	     double _Vth, 
	     double _Zth,
	     double _ZthWeak,
	     double _ZthFloating
	     )

  : stimulus(_name,_Vth, _Zth),
    ZthWeak(_ZthWeak), ZthFloating(_ZthFloating)
{
  if(verbose)
    cout << "IOPIN default constructor\n";

  l2h_threshold = 2.0;      // PICs are CMOS and use CMOS-like thresholds
  h2l_threshold = 1.0;
  bDrivenState = false;
  cForcedDrivenState = 'Z';
  snode = 0;
  m_monitor=0;

}

void IOPIN::setMonitor(PinMonitor *new_pinMonitor)
{
  if (!m_monitor && new_pinMonitor)
    m_monitor = new_pinMonitor;
}


IOPIN::~IOPIN()
{
}


void IOPIN::attach(Stimulus_Node *s)
{
  snode = s;
}

void IOPIN::show()
{
  stimulus::show();
}

//--------------------
// set_nodeVoltage()
//
// 
void IOPIN::set_nodeVoltage(double new_nodeVoltage)
{
  if(verbose & 1)
    cout << name()<< " set_nodeVoltage old="<<nodeVoltage <<" new="<<new_nodeVoltage<<endl;
  
  nodeVoltage = new_nodeVoltage;

  if( nodeVoltage < h2l_threshold) {

    // The voltage is below the low threshold
    setDrivenState(false);
  } 
  else if(nodeVoltage > l2h_threshold) {

    // The voltage is above the high threshold
    setDrivenState(true);

  }  else {
    // The voltage is between the low and high thresholds,
    // so do nothing
  }

  //setDrivenState(getBitChar());
  if (m_monitor)
    m_monitor->set_nodeVoltage(nodeVoltage);
}

//------------------------------------------------------------
// putState - called by peripherals when they wish to
// drive an I/O pin to a new state.

void IOPIN::putState(bool new_state)
{
  if(new_state != bDrivingState) {
    bDrivingState = new_state;
    Vth = bDrivingState ? 5.0 : 0.3;
    
    if(verbose & 1)
      cout << name()<< " putState= " 
	   << (new_state ? "high" : "low") << endl;
    
    // If this pin is tied to a node, then update the node.
    // Note that when the node is updated, then the I/O port
    // (if there is one) holding this I/O pin will get updated.
    // If this pin is not tied to a node, then try to update
    // the I/O port directly.

    if(snode)
      snode->update();
  }
  if(m_monitor)
    m_monitor->putState(new_state?'1':'0');

}

//------------------------------------------------------------
bool IOPIN::getState()
{
  return getDriving() ? getDrivingState() : getDrivenState();
}

void IOPIN::setDrivingState(bool new_state)
{ 
  bDrivingState = new_state;

  if(m_monitor)
    m_monitor->setDrivingState(bDrivingState?'1':'0');

  if(verbose & 1)
    cout << name()<< " setDrivingState= " 
	 << (new_state ? "high" : "low") << endl;
}

void IOPIN::setDrivingState(char new3State)
{ 
  bDrivingState = new3State=='1';

  if(m_monitor)
    m_monitor->setDrivingState(new3State);

}

bool IOPIN::getDrivingState(void)
{
  return bDrivingState;
}


bool IOPIN::getDrivenState()
{
  return bDrivenState;
}

//------------------------------------------------------------------------
// setDrivenState
//
// An stimulus attached to this pin is driving us to a new state.
// This state will be recorded and propagate up to anything 
// monitoring this pin.

void IOPIN::setDrivenState(bool new_state)
{
  bDrivenState = new_state;

  if(verbose & 1)
    cout << name()<< " setDrivenState= " 
	 << (new_state ? "high" : "low") << endl;

  // Propagate the new state to those things monitoring this pin.
  // (note that the 3-state value is what's propagated).
  if(m_monitor)
  {
    m_monitor->setDrivenState(getBitChar());
    if(verbose & 16)
      cout << name() << " setting state of monitor to " << getBitChar() << endl;
  }
}

//------------------------------------------------------------------------
// forceDrivenState() - allows the 'driven state' to be manipulated whenever
// there is no snode attached. The primary purpose of this is to allow the
// UI to toggle I/O pin states.
// 
void IOPIN::forceDrivenState(char newForcedState)
{
  if (cForcedDrivenState != newForcedState) {

    cForcedDrivenState = newForcedState;

    bDrivenState = cForcedDrivenState=='1' || cForcedDrivenState=='W';
    
    if(m_monitor) {
      m_monitor->setDrivenState(getBitChar());
      m_monitor->updateUI();
    }
  }
}

char IOPIN::getForcedDrivenState()
{
  return cForcedDrivenState;
}

void IOPIN::toggle()
{
  putState(getState() ^ true);
}

/*************************************
 *  int IOPIN::get_Vth()
 *
 * If this iopin has a stimulus attached to it then
 * the voltage will be dictated by the stimulus. Otherwise,
 * the voltage is determined by the state of the ioport register
 * that is inside the pic. For an input (like this), the pic code
 * that is being simulated can not change the state of the I/O pin.
 * However, the user has the ability to modify the state of
 * this register either by writing directly to it in the cli,
 * or by clicking in one of many places in the gui.
 */
double IOPIN::get_Vth()
{
  return Vth;

}

char IOPIN::getBitChar()
{
  if(!snode)
    return getForcedDrivenState();      // RCP - Changed to match IO_bi_directional
//  was  return 'Z';  // High impedance - unknown state.

  if(snode->get_nodeZth() > ZthFloating)
    return 'Z';

  if(snode->get_nodeZth() > ZthWeak)
    return getDrivenState() ? 'W' : 'w';

  return getDrivenState() ? '1' : '0';
}
void IOPIN::newGUIname(const char *s)
{
                                                                                
  if(s)
  {
    gui_name_updated = true;
    gui_name = string(s);
  }
}
string &IOPIN::GUIname(void) const
{
  return (string &)gui_name;
}
//========================================================================
//
IO_bi_directional::IO_bi_directional(const char *_name,
				     double _Vth, 
				     double _Zth,
				     double _ZthWeak,
				     double _ZthFloating,
				     double _VthIn,
				     double _ZthIn)
  : IOPIN(_name, _Vth, _Zth, _ZthWeak, _ZthFloating),
    ZthIn(_ZthIn), VthIn(_VthIn)
{
}


void IO_bi_directional::set_nodeVoltage( double new_nodeVoltage)
{
  IOPIN::set_nodeVoltage(new_nodeVoltage);
}

double IO_bi_directional::get_Vth()
{
  if(getDriving())
    return getDrivingState() ? Vth : 0;

  
  //return getDrivingState() ? VthIn : 0;
  return VthIn;

}


double IO_bi_directional::get_Zth()
{
  return getDriving() ? Zth : ZthIn;

}

/*
   getBitChar() returns bit status as follows
     Input pin
	1> Pin considered floating, 
	   return 'Z'
	2> Weak Impedance on pin, 
	   return 'W" if high or 'w' if low
	3> Pin being driven externally
	   return '1' node voltage high '0' if low
     Output pin
	1> Node voltage opposite driven value
	   return 'X' if node voltage high or 'x' if inode voltage low
	2> Node voltage same as driven value
	   return '1' node voltage high '0' if low
*/

char IO_bi_directional::getBitChar()
{
  if(!snode && !getDriving() )
    return getForcedDrivenState();

  if(snode) {
                                                                                
    if (!getDriving())		// input pin
    {
      if(snode->get_nodeZth() > ZthFloating)
        return 'Z';
                                                                                
      if(snode->get_nodeZth() > ZthWeak)
        return getDrivenState() ? 'W' : 'w';
     }
    else if(getDrivenState() != getDrivingState())
        return getDrivenState() ? 'X' : 'x';
  }


  return getDrivenState() ? '1' : '0';
}


//---------------
//::update_direction(unsigned int new_direction)
//
//  This is called when a new value is written to the tris register
// with which this bi-direction pin is associated.

void IO_bi_directional::update_direction(unsigned int new_direction, bool refresh)
{

  setDriving(new_direction ? true : false);

  // If this pin is not associated with an IO Port, but it's tied
  // to a stimulus, then we need to update the stimulus.
  if(refresh && snode)
    snode->update();
}

IO_bi_directional_pu::IO_bi_directional_pu(const char *_name,
					   double _Vth, 
					   double _Zth,
					   double _ZthWeak,
					   double _ZthFloating,
					   double _VthIn,
					   double _ZthIn,
					   double _Zpullup)
  : IO_bi_directional(_name, _Vth, _Zth, _ZthWeak,
		      _ZthFloating, _VthIn, _ZthIn),
    Zpullup(_Zpullup)
{
  Vpullup = Vth;
  bPullUp = false;
}

IO_bi_directional_pu::~IO_bi_directional_pu(void)
{

}

void IO_bi_directional_pu::update_pullup(char new_state, bool refresh)
{
  bool bNewPullupState = new_state == '1' || new_state == 'W';
  if (bPullUp != bNewPullupState) {
    bPullUp = bNewPullupState;
    if (refresh) { 
      if (snode)
	snode->update();
      else
	setDrivenState(bPullUp);
    }
  }
}

double IO_bi_directional_pu::get_Zth()
{
  return getDriving() ? Zth : (bPullUp ? Zpullup : ZthIn);
}

double IO_bi_directional_pu::get_Vth()
{
  
  /**/
  if(verbose & 1)
    cout << name() << " get_Vth PU "
	 << " driving=" << getDriving()
	 << " DrivingState=" << getDrivingState()
	 << " bDrivenState=" << bDrivenState
	 << " Vth=" << Vth
	 << " VthIn=" << VthIn
	 << " bPullUp=" << bPullUp << endl;
  /**/  

  // If the pin is configured as an output, then the driving voltage
  // depends on the pin state. If the pin is an input, and the pullup resistor
  // is enabled, then the pull-up resistor will 'drive' the output. The
  // open circuit voltage in this case will be Vth (the thevenin voltage, 
  // which is assigned to be same as the processor's supply voltage).

  if(getDriving())
    return getDrivingState() ? Vth : 0;
  else
    return bPullUp ? Vpullup : VthIn;

}

/*
   getBitChar() returns bit status as follows
     Input pin
	1> Pin considered floating, 
	   return 'Z'
	2> Weak Impedance on pin, 
	   return 'W" if high or 'w' if low
	3> Pin being driven externally
	   return '1' node voltage high '0' if low
     Output pin
	1> Node voltage opposite driven value
	   return 'X' if node voltage high or 'x' if inode voltage low
	2> Node voltage same as driven value
	   return '1' node voltage high '0' if low
*/

char IO_bi_directional_pu::getBitChar()
{
  if(!snode && !getDriving() ) {
    char cForced=getForcedDrivenState();
    return (cForced=='Z' && bPullUp) ? 'W' : cForced;
  }

  if(snode) {
                                                                                
    if (!getDriving())		// input pin
    {
      if(snode->get_nodeZth() > ZthFloating)
        return 'Z';
                                                                                
      if(snode->get_nodeZth() > ZthWeak)
        return getDrivenState() ? 'W' : 'w';
     }
    else if(getDrivenState() != getDrivingState())
        return getDrivenState() ? 'X' : 'x';
  }


  return getDrivenState() ? '1' : '0';
}

IO_open_collector::IO_open_collector(const char *_name)
  : IO_bi_directional_pu(_name)
{
}


double IO_open_collector::get_Vth()
{
  /**/
  if(verbose & 1)
    cout << name() << " get_Vth OC"
	 << " driving=" << getDriving()
	 << " DrivingState=" << getDrivingState()
	 << " bDrivenState=" << bDrivenState
	 << " Vth=" << Vth
	 << " VthIn=" << VthIn
	 << " bPullUp=" << bPullUp << endl;
  /**/  

  if(getDriving() && !getDrivingState())
    return 0.0;

  return bPullUp ? Vpullup : VthIn;
}


double IO_open_collector::get_Zth()
{
  if(getDriving() && !getDrivingState())
    return Zth;

  return bPullUp ? Zpullup : ZthIn;

}
char IO_open_collector::getBitChar()
{
  if(!snode && !getDriving() ){
    char cForced=getForcedDrivenState();
    return (cForced=='Z' && bPullUp) ? 'W' : cForced;
  }

  if(snode) {

    if(snode->get_nodeZth() > ZthFloating)
      return bPullUp ? 'W' : 'Z';

    if(getDriving() && getDrivenState() && !getDrivingState())
      return 'X';

    if(snode->get_nodeZth() > ZthWeak)
      return getDrivenState() ? 'W' : 'w';
    else
      return getDrivenState() ? '1' : '0';
  }

  return getDrivingState() ? 'W' : '0';
}

//========================================================================

//========================================================================
//
// ValueStimulus
//
// A Value stimulus is a stream of data that can change values at
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

ValueStimulus::ValueStimulus(const char *n)
  : source_stimulus() 
{
  initial.time = 0;
  initial.v = 0;
  current = 0;

  if(n)
    new_name(n);
  else
    {
      char name_str[100];
      snprintf(name_str,sizeof(name_str),"s%d_asynchronous_stimulus",num_stimuli);
      num_stimuli++;
      new_name(name_str);
    }

}

ValueStimulus::~ValueStimulus()
{
  delete initial.v;
  delete current;

  for(sample_iterator = samples.begin();
      sample_iterator != samples.end();
      ++sample_iterator) {
	  
    delete (*sample_iterator).v;
  }

}

void ValueStimulus::show()
{
  // print the electrical stuff
  stimulus::show();

  cout << "\n  states = " << samples.size() << '\n';

  list<ValueStimulusData>::iterator si;

  for(si = samples.begin();
      si != samples.end();
      ++si) {

    //double d;
    //(*si).v->get(d);
    cout << "    t=" << dec << (*si).time
	 <<  ",v=" << (*si).v->toString()
	 << '\n';

  }

  if (initial.v)
    cout << "  initial=" << initial.v->toString() << '\n';

  cout
    << "  period=" << period << '\n'
    << "  start_cycle=" << start_cycle << '\n'
    << "  Next break cycle=" << future_cycle << '\n';

}

void ValueStimulus::callback()
{
  guint64 current_cycle = future_cycle;

  current = next_sample.v;

  if(verbose & 1)
    cout << "asynchro cycle " << current_cycle << "  state " << current->toString() << '\n';

  // If there's a node attached to this stimulus, then update it.
  if(snode)
    snode->update();

  ValueStimulusData *n = getNextSample();

  if(n) {
    next_sample = *n;

    if(verbose & 1) {
      cout << "  current_sample (" << next_sample.time << "," 
	   << next_sample.v->toString() << ")\n";
      cout << " start cycle " << start_cycle << endl;
    }

    // get the cycle when the data will change next

    future_cycle = next_sample.time + start_cycle;
      

    if(future_cycle <= current_cycle) {
    
      // There's an error in the data. Set a break on the next simulation cycle
      // and see if it can be resolved.

      future_cycle = current_cycle+1;
    }

    get_cycles().set_break(future_cycle, this);
  } else
    future_cycle = 0;

  if(verbose & 1)
    cout <<"  next transition = " << future_cycle << '\n';
}

void ValueStimulus::put_initial_state(Value *pValue)
{
  if (pValue && !initial.v) {
    initial.time = 0;
    initial.v = pValue->copy();
  }
}

void ValueStimulus::put_data(ValueStimulusData &data_point)
{
  ValueStimulusData *sample = new ValueStimulusData;
  sample->time = data_point.time;
  sample->v = data_point.v;
  samples.push_back(*sample);

}

double ValueStimulus::get_Vth()
{
  double v=initial_state;
  if(current) {
    try {
      current->get(v);
      if(digital && v >0.0)
	v = 5.0;
    }

    catch (Error *err) {
      if(err) {
	cout << "Warning stimulus: " << name() << " failed on: "<< err->toString() << endl;
	delete err;
      }
    }

  }
  return v;
}


void ValueStimulus::start()
{

  if(verbose & 1)
    cout << "Starting asynchronous stimulus\n";

  if(period) {

    // Create a data point for the rollover condition.
    // If an initial value was supplied when the stimulus was created, then
    // that's what we'll use for the rollover. Otherwise, we'll create 
    // a rollover based on 'initial_state' (which should be a default value).

    ValueStimulusData vSample;
    vSample.time = period;
    vSample.v = initial.v ? initial.v : new Float(initial_state);
    put_data(vSample);
  }

  sample_iterator = samples.begin();

  if(sample_iterator != samples.end()) {


    if(digital)
      initial_state = (initial_state > 0.0) ? Vth : 0.0;

    current       = initial.v;
    next_sample   = *sample_iterator;
    future_cycle  = next_sample.time;// + start_cycle;

    get_cycles().set_break(future_cycle, this);

  }



  if(verbose & 1)
    cout << "asy should've been started\n";

}


ValueStimulusData *ValueStimulus::getNextSample()
{

  ++sample_iterator;

  if(sample_iterator == samples.end()) {

    // We've gone through all of the data. Now let's try to start over

    sample_iterator = samples.begin();

    // If the period is zero then we don't want to 
    // regenerate the data stream.

    if(period == 0)
      return 0;

    start_cycle += period;

    if(verbose & 1) {
      cout << "  asynchronous stimulus rolled over\n"
	   << "   next start_cycle " << start_cycle << "  period " << period << '\n';
    }
  }

  return &(*sample_iterator);
}
//------------------------------------------------------------------------
AttributeStimulus::AttributeStimulus(const char *n)
  : ValueStimulus(n), attr(0)
{
}
/*
AttributeStimulus::~AttributeStimulus()
{
  ValueStimulus::~ValueStimulus();

}
*/
void AttributeStimulus::show()
{
  if (attr)
    cout << "\nDriving Attribute:" << attr->name() << endl;
  ValueStimulus::show();
}

void AttributeStimulus::callback()
{
  guint64 current_cycle = future_cycle;

  current = next_sample.v;

  if(verbose & 1)
    cout << "asynchro cycle " << current_cycle << "  state " << current->toString() << '\n';

  // If there's a node attached to this stimulus, then update it.
  if(attr)
    attr->set(current);

  ValueStimulusData *n = getNextSample();

  if(n) {
    next_sample = *n;

    if(verbose & 1) {
      cout << "  current_sample (" << next_sample.time << "," 
	   << next_sample.v->toString() << ")\n";
      cout << " start cycle " << start_cycle << endl;
    }

    // get the cycle when the data will change next

    future_cycle = next_sample.time + start_cycle;
      

    if(future_cycle <= current_cycle) {
    
      // There's an error in the data. Set a break on the next simulation cycle
      // and see if it can be resolved.

      future_cycle = current_cycle+1;
    }

    get_cycles().set_break(future_cycle, this);
  } else
    future_cycle = 0;

  if(verbose & 1)
    cout <<"  next transition = " << future_cycle << '\n';
}

void AttributeStimulus::setClientAttribute(Value *v)
{
  if(attr)
    cout << "overwriting target attribute in AttributeStimulus\n";

  attr = v;

  if((bool)verbose && v)
    cout << " attached " << name() << " to attribute: " << v->name() << endl;
}

//========================================================================
// 
// helper functions follow here


//--------------------------------------------------------
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

  if(!node)
    return;

  string s(node);
  Symbol_Table &ST = get_symbol_table();
  Stimulus_Node *sn = ST.findNode(s);

  if(sn) {
    while(stimuli) {
      s = string(stimuli->name);
      stimulus *st = ST.findStimulus(s);
      if(st) {
        sn->attach_stimulus(st);
        if(verbose&2)
          cout << " attaching stimulus: " << s << '\n';
      }
      else
        cout << "Warning, stimulus: " << s << " not attached\n";

      stimuli = stimuli->next;
    }
    sn->update();
  }
  else {
    cout << "Warning: Node \"" << node << "\" was not found in the node list\n";
  }

}

void AttachStimulusToNode(Stimulus_Node *sn, string &sStimulusName);
void AttachStimulusToNode(Stimulus_Node *sn, string &sStimulusName,
                          stimulus *st);

//========================================================================
//  stimuli_attach(list <string> * sl)
//
//  Attach stimuli to a node
//
// The first item in the input list is the name of the node.
// The remaining items are the names of the stimuli.

void stimuli_attach(StringList_t *sl)
{
  if (!sl)
    return;

  list <string> :: iterator si;

  si = sl->begin();

  Symbol_Table &ST = get_symbol_table();
  Stimulus_Node *sn = ST.findNode((*si));

  if(sn) {
      for(++si; si != sl->end(); ++si)
      {
        AttachStimulusToNode(sn, *si);
      }
      sn->update();
  }
  else {
    cout << "Warning: Node \"" << (*si) << "\" was not found in the node list\n";
  }
}


void stimuli_attach(SymbolList_t *sl)
{
  if (!sl)
    return;

  SymbolList_t :: iterator si;

  // The first symbol is always the node name
  si = sl->begin();
  Symbol_Table &ST = get_symbol_table();
  Stimulus_Node *sn = ST.findNode((*si)->name());

  if(sn) {
    // All symbols thereafter are stimulus objects
    for(++si; si != sl->end(); ++si)
    {
      AttachStimulusToNode(sn, (*si)->name());
    }
    sn->update();
  }
  else {
    // The first symbol is not a node - so let's assume that 
    // we're performing a register stimulus.

    //cout << "Warning: Node \"" << (*si) << "\" was not found in the node list\n";

    stimulus *st;
    Value *v;
    if(sl->size() == 2) {
      st = ST.findStimulus((*si)->name());

      if(st) {
        ++si;
        v = *si;
      } else {
        v = *si;
        ++si;
        st = ST.findStimulus((*si)->name());
      }

      if(st) {
        AttributeStimulus *ast = dynamic_cast<AttributeStimulus *>(st);
        if(ast)
          ast->setClientAttribute(v);
      }
    }

  }
}

void stimuli_attach(Value *pNode, PinList_t *pPinList)
{
  bool bSuccess = true;
  Symbol_Table &ST = get_symbol_table();
  node_symbol *pNS = dynamic_cast<node_symbol*>(pNode); // ST.findNode(pNode->name());
  PinList_t::iterator si;
  if(pNS) {
    Stimulus_Node *sn = pNS->getNode();
    // All symbols thereafter are stimulus objects
    for(si = pPinList->begin();
        si != pPinList->end() && bSuccess; ++si)
    {
      Pin_t * pPinArgument = *si;
#if 0
      // don't have time to test out this new structure.
      stimulus * pStim = pPinArgument->GetStimulus();
      if(pStim != NULL) {
        // PinName symbol name only
        AttachStimulusToNode(sn, pStim->name(), pStim);
      }
      else {
        IOPIN * pPin = pPinArgument->GetIOPin();
        AttachStimulusToNode(sn, pPinArgument->m_sPin->name(), pPin);
      }
#else
      stimulus_symbol * pPinSymbol = dynamic_cast<stimulus_symbol*>(pPinArgument->m_sPin);
      stimulus * pPin = pPinSymbol == NULL ? NULL : pPinSymbol->getStimulus();
      if(pPin != NULL) {
        // PinName symbol name only
        AttachStimulusToNode(sn, pPin->name(), pPin);
      }
      else {
        Module *pMod = NULL;
        if(pPinArgument->m_iFlags & Pin_t::eActiveProc) {
          pMod = get_active_cpu();
        }
        else if ( pPinArgument->m_sModuleName ) {
          // this dynamic_cast always fails here
          pMod = dynamic_cast<Module*>(pPinArgument->m_sModuleName);
          if(pMod == NULL) {
            // but the dynamic_cast in findModule() succeeds
            pMod = ST.findModule(pPinArgument->m_sModuleName->name().c_str());
            if(pMod == NULL) {
              String *pModName = dynamic_cast<String*>(pPinArgument->m_sModuleName);
              // but the dynamic_cast in findModule() succeeds
              if(pModName != NULL) {
                pMod = ST.findModule(*pModName);
              }
            }
          }
        }
        if(pMod == NULL) {
          if (NULL == pPinArgument->m_sModuleName) {
            GetUserInterface().DisplayMessage(
              "attach error: did not find pin '%s'\n",
              pPinArgument->m_sPin->name().c_str());
          }
          else {
            GetUserInterface().DisplayMessage(
              "attach error: did not find module '%s'\n",
              pPinArgument->m_sModuleName->name().c_str());
          }
          bSuccess = false;
        }
        else {
          Integer *pPinInt = dynamic_cast<Integer*>(pPinArgument->m_sPin);

          if(pPinInt != NULL) {
            IOPIN *pPinObj = NULL;
            if(pPinArgument->m_iFlags & Pin_t::ePackageBased) {
              // ModName && Integer
              // Could be a literal int or a symbol
              pPinObj = pMod->get_pin(*pPinInt);
            }
            else /* if(pPinArgument->m_iFlags & Pin_t::ePortBased) */ {
              ioport_symbol *pIOPSym = dynamic_cast<ioport_symbol*>(pPinArgument->m_sPort);
              if(pIOPSym != NULL) {
                PortRegister * pPort = pIOPSym->getIOPort();
                pPinObj = pPort->getPin(*pPinInt);
              }
              else {
                bSuccess = false;
                GetUserInterface().DisplayMessage(
                  "attach error: did not find port '%s' in module '%s'\n",
                  pPinArgument->m_sPort->name().c_str());
              }
            }
            if(pPinObj != NULL) {
              AttachStimulusToNode(sn, pPinInt->name(), pPinObj);
            }
            else {
              bSuccess = false;
              GetUserInterface().DisplayMessage(
                  "attach error: did not find pin '%d' in module '%s'\n",
                  (int)*pPinInt, pMod->name().c_str());
            }
          }
          else {
            bSuccess = false;
            if(pPin == NULL && 
		strcmp(pPinArgument->m_sPin->showType().c_str(), "module_symbol")) {
              int iValue = -1;
              if(pPinArgument->m_sPin) {
                pPinArgument->m_sPin->get(iValue);
              }
              if(pPinArgument->m_sPort) {
                pPinArgument->m_sPort->get(iValue);
              }
              GetUserInterface().DisplayMessage(
                  "attach error: pin argument '%s'(%d) type(%s) is not of type Integer or stimulus\n",
                  pPinArgument->m_sPin->name().c_str(), iValue,
                  pPinArgument->m_sPin->showType().c_str());
            }
            else {
              GetUserInterface().DisplayMessage(
                  "attach error: pin argument '%s' type(%s) is not of type Integer or stimulus\n",
                  pPinArgument->m_sPin->name().c_str(),
                  pPinArgument->m_sPin->showType().c_str());
              }
          }
        }
      }
#endif
    }
    sn->update();
  }
  else {
    // The first symbol is not a node - so let's assume that 
    // we're performing a register stimulus.

    //cout << "Warning: Node \"" << (*si) << "\" was not found in the node list\n";
    si = pPinList->begin();
    stimulus *st;
    Value *v;
    if(pPinList->size() == 1) {
      // Might be a stimulus but probably not
      st = dynamic_cast<stimulus*>(pNode);
      if(st == NULL) {
        // Might be an attribute holding a stimulus
        st = ST.findStimulus(pNode->name());
      }
      Pin_t *pPin = dynamic_cast<Pin_t*>(*si);
      if(st) {
        // if pNode is a stimulus
        // then get whatever the the first Pin_t is
        v = pPin->GetValue();

      } else {
        v = pNode;
        st = ST.findStimulus(pPin->GetValue()->name());
      }

      if(st) {
        AttributeStimulus *ast = dynamic_cast<AttributeStimulus *>(st);
        if(ast) {
          ast->setClientAttribute(v);
        }
      }
    }
  }
}

void AttachStimulusToNode(Stimulus_Node *sn, string &sStimulusName) {
  stimulus *st = get_symbol_table().findStimulus(sStimulusName);
  AttachStimulusToNode(sn, sStimulusName, st);
}

void AttachStimulusToNode(Stimulus_Node *sn, string &sStimulusName, stimulus *st) {
  if(st) {
    // attach each found stimulus to the node
    sn->attach_stimulus(st);
    if(verbose&2) {
      if(sStimulusName.empty() || sStimulusName == st->name()) {
        GetUserInterface().DisplayMessage(
          "attach stimulus: %s to node: %s\n",
          st->name().c_str(), sn->name().c_str());
      }
      else {
        GetUserInterface().DisplayMessage(
          "attach stimulus: %s(%s) to node: %s\n",
          sStimulusName.c_str(), st->name().c_str(), sn->name().c_str());
      }
    }
  }
  else {
    GetUserInterface().DisplayMessage(
      "attach warning: %s(%s) not attached to %s\n",
      sStimulusName.c_str(), st->name().c_str(), sn->name().c_str());
  }
}

stimulus *Pin_t::GetStimulus() {
  stimulus_symbol * pPinSymbol = NULL;
  if(m_sPin) {
    pPinSymbol = dynamic_cast<stimulus_symbol*>(m_sPin);
  }
  if(m_sPort) {
    pPinSymbol = dynamic_cast<stimulus_symbol*>(m_sPort);
  }
  stimulus * pPin = pPinSymbol == NULL ? NULL : pPinSymbol->getStimulus();
  // PinName symbol name only
  if(pPin == NULL) {
    int iPinNumber = -1;
    if(pPinSymbol) {
      pPinSymbol->get(iPinNumber);
      GetUserInterface().DisplayMessage(
        "attach error: pin argument '%s'(%d) type(%s) is not of type Integer or stimulus\n",
        pPinSymbol->name().c_str(), iPinNumber,
        pPinSymbol->showType().c_str());
    }
  }
  return pPin;
}

IOPIN *Pin_t::GetIOPin() {
  bool bSuccess = true;
  Module *pMod;
  Symbol_Table &ST = get_symbol_table();
  if(m_iFlags & Pin_t::eActiveProc) {
    pMod = get_active_cpu();
  }
  else {
    // this dynamic_cast always fails here
    pMod = dynamic_cast<Module*>(m_sModuleName);
    if(pMod == NULL) {
      // but the dynamic_cast in findModule() succeeds
      pMod = ST.findModule(m_sModuleName->name().c_str());
      if(pMod == NULL) {
        String *pModName = dynamic_cast<String*>(m_sModuleName);
        // but the dynamic_cast in findModule() succeeds
        if(pModName != NULL) {
          pMod = ST.findModule(*pModName);
        }
      }
    }
  }
  if(pMod == NULL) {
    if (NULL == m_sModuleName) {
      GetUserInterface().DisplayMessage(
        "attach error: did not find pin '%s'\n",
        m_sPin->name().c_str());
    }
    else {
      GetUserInterface().DisplayMessage(
        "attach error: did not find module '%s'\n",
        m_sModuleName->name().c_str());
    }
    bSuccess = false;
  }
  else {
    Integer *pPinInt = dynamic_cast<Integer*>(m_sPin);

    if(pPinInt != NULL) {
      IOPIN *pPinObj = NULL;
      if(m_iFlags & Pin_t::ePackageBased) {
        // ModName && Integer
        // Could be a literal int or a symbol
        pPinObj = pMod->get_pin(*pPinInt);
      }
      else /* if(m_iFlags & Pin_t::ePortBased) */ {
        ioport_symbol *pIOPSym = dynamic_cast<ioport_symbol*>(m_sPort);
        if(pIOPSym != NULL) {
          PortRegister * pPort = pIOPSym->getIOPort();
          pPinObj = pPort->getPin(*pPinInt);
        }
        else {
          bSuccess = false;
          GetUserInterface().DisplayMessage(
            "attach error: did not find port '%s' in module '%s'\n",
            m_sPort->name().c_str());
        }
      }
      if(pPinObj != NULL) {
        return pPinObj;
//        AttachStimulusToNode(sn, pPinInt->name(), pPinObj);
      }
      else {
        bSuccess = false;
        GetUserInterface().DisplayMessage(
            "attach error: did not find pin '%d' in module '%s'\n",
            (int)*pPinInt,
            m_sModuleName->name().c_str());
      }
    }
    else {
      bSuccess = false;
      GetUserInterface().DisplayMessage(
          "attach error: pin argument '%s' type(%s) is not of type Integer\n",
          m_sPin->name().c_str(),
          m_sPin->showType().c_str());
    }
  }
  return (IOPIN*)NULL;
}

Value * Pin_t::GetValue() {

  if(m_sPin)
    return m_sPin;
  if(m_sPort)
    return m_sPort;
  return NULL;
}

