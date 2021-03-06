/*
   Copyright (C) 1998 Scott Dattalo

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
#include <string>
#include <assert.h>

#include "../config.h"
#include "cmd_gpsim.h"
#include "ioports.h"
#include "trace.h"

#include "stimuli.h"

#include "xref.h"
//#define DEBUG
#if defined(DEBUG)
#define Dprintf(arg) {printf("%s:%d ",__FILE__,__LINE__); printf arg; }
#else
#define Dprintf(arg) {}
#endif

//--------------------------------------------------
// 
//--------------------------------------------------
PeripheralSignalSource::PeripheralSignalSource(PinModule *_pin)
  : m_pin(_pin), m_cState('?')
{
  assert(m_pin);
}

// getState is called when the PinModule is attempting to
// update the output state for the I/O Pin.

char PeripheralSignalSource::getState()
{
  return m_cState;
}

/// putState is called when the peripheral output source
/// wants to change the output state.
void PeripheralSignalSource::putState(const char new3State)
{
  if (new3State != m_cState) {
    m_cState = new3State;
    m_pin->updatePinModule();
  }
}

void PeripheralSignalSource::toggle()
{
  switch (m_cState) {
  case '1':
  case 'W':
    putState('0');
    break;
  case '0':
  case 'w':
    putState('1');
    break;
  }
}

//-------------------------------------------------------------------
//
//                 ioports.cc
//
// The ioport infrastructure for gpsim is provided here. The class
// taxonomy for the IOPORT class is:
//
//  file_register 
//     |-> sfr_register
//            |-> IOPORT
//                  |-> PORTA
//                  |-> PORTB
//                  |-> PORTC
//                  |-> PORTD
//                  |-> PORTE
//                  |-> PORTF
// 
// Each I/O port has an associated array of I/O pins which provide an
// interface to the virtual external world of the stimuli.
//
//-------------------------------------------------------------------

class SignalSource : public SignalControl
{
public:
  SignalSource(PortRegister *_reg, unsigned int bitPosition)
    : m_register(_reg), m_bitMask(1<<bitPosition)
  {
  }
  char getState()
  {
    // return m_register ? 
    //  (((m_register->getDriving()&m_bitMask)!=0)?'1':'0') : 'Z';
    char r = m_register ? (((m_register->getDriving()&m_bitMask)!=0)?'1':'0') : 'Z';
    /**/
    Dprintf(("SignalSource::getState() %s  bitmask:0x%x state:%c\n",
	     (m_register?m_register->name().c_str():"NULL"),
	     m_bitMask,r));
    /**/
    return r;
  }
private:
  PortRegister *m_register;
  unsigned int  m_bitMask;
};


PortSink::PortSink(PortRegister *portReg, unsigned int iobit)
  : m_PortRegister(portReg), m_iobit(iobit)
{
  assert (m_PortRegister);
}

void PortSink::setSinkState(char cNewSinkState)
{
  Dprintf((" PortSink::setSinkState:bit=%d,val=%c\n",m_iobit,cNewSinkState));

  m_PortRegister->setbit(m_iobit,cNewSinkState);
}
//------------------------------------------------------------------------
PortRegister::PortRegister(unsigned int numIopins, unsigned int _mask)
  : sfr_register(),
    PortModule(numIopins),
    mEnableMask(_mask),  
    drivingValue(0), rvDrivenValue(0,0)

{

}

void PortRegister::setEnableMask(unsigned int newEnableMask)
{
  //unsigned int maskDiff = getEnableMask() ^ newEnableMask;
  unsigned int oldEnableMask = getEnableMask();

  for (unsigned int i=0, m=1; i<mNumIopins; i++, m<<= 1)
    if ((newEnableMask & m) && ! (oldEnableMask & m )) 
    {
      PinModule *pmP = PortModule::getIOpins(i); 
      if (!pmP)
      {
          pmP = new PinModule(this,i);
          PortModule::addPinModule(pmP,i);
          pmP->setDefaultSource(new SignalSource(this, i));
          pmP->addSink(new PortSink(this, i));
      }
      else
      {
        if (pmP->getSourceState() == '?')
	{
           pmP->setDefaultSource(new SignalSource(this, i));
           pmP->addSink(new PortSink(this, i));
	}
      }
    }

  mEnableMask = newEnableMask;
}

void PortRegister::put(unsigned int new_value)
{
  trace.raw(write_trace.get() | value.data);

  put_value(new_value);
}
void PortRegister::put_value(unsigned int new_value)
{
  Dprintf(("PortRegister::put_value old=0x%x:new=0x%x\n",value.data,new_value));

  unsigned int diff = mEnableMask & (new_value ^ value.data);
  drivingValue = new_value & mEnableMask;
  value.data = drivingValue;

  if(diff) {
    // If no stimuli are connected to the Port pins, then the driving
    // value and the driven value are the same. If there are external
    // stimuli (or perhaps internal peripherals) overdriving or overriding
    // this port, then the call to updatePort() will update 'drivenValue'
    // to its proper value. In either case, calling updatePort ensures 
    // the drivenValue is updated properly
    
    updatePort();
  }
}
//------------------------------------------------------------------------
// PortRegister::updateUI()  UI really means GUI.
// We just pass control to the update method, which is defined in gpsimValue.

void PortRegister::updateUI()
{
  update();
}
//------------------------------------------------------------------------
// PortRegister::setbit
//
// This method is called whenever a stimulus changes the state of
// an I/O pin associated with the port register. 3-state logic is
// used.
// FIXME -  rvDrivenValue and value are always the same, so why have
// FIXME -  both?

void PortRegister::setbit(unsigned int bit_number, char new3State)
{
  if(bit_number <= bit_mask) {

    trace.raw(write_trace.get()  | value.data);
    trace.raw(write_trace.geti() | value.init);

    Dprintf(("PortRegister::setbit() %s bit=%d,val=%c\n",name().c_str(), bit_number,new3State));

    if (new3State=='1' || new3State=='W') {
      rvDrivenValue.data |= (1<<bit_number);
      rvDrivenValue.init &= ~(1<<bit_number);
    } else if (new3State=='0' || new3State=='w') {
      rvDrivenValue.data &= ~(1<<bit_number);
      rvDrivenValue.init &= ~(1<<bit_number);
    } else 
      // Not a 0 or 1, so it must be unknown.
      rvDrivenValue.init |= (1<<bit_number);

    value = rvDrivenValue;
  }

}

unsigned int PortRegister::get()
{
  trace.raw(read_trace.get()  | rvDrivenValue.data);
  trace.raw(read_trace.geti() | rvDrivenValue.init);

  return rvDrivenValue.data;
}
unsigned int PortRegister::get_value()
{
  return rvDrivenValue.data;
}
void PortRegister::putDrive(unsigned int new_value)
{
  put(new_value);
}
unsigned int PortRegister::getDriving()
{
  return drivingValue;
}
//========================================================================
//========================================================================
static PinModule AnInvalidPinModule;

PortModule::PortModule(unsigned int numIopins)
  : mNumIopins(numIopins)
{

  iopins = new PinModule *[mNumIopins];
  for (unsigned int i=0; i<mNumIopins; i++)
    iopins[i] = &AnInvalidPinModule;
  //iopins[i] = new PinModule(this,i);

}
PortModule::~PortModule()
{
  for (unsigned int i=0; i<mNumIopins; i++)
    delete iopins[i];

  delete iopins;
}

PinModule &PortModule::operator [] (unsigned int iPinNumber)
{
  if (iPinNumber < mNumIopins)
    return *iopins[iPinNumber];

  // error...
  return AnInvalidPinModule;
}

PinModule * PortModule::getIOpins(unsigned int iPinNumber)
{

  if (iPinNumber < mNumIopins && iopins[iPinNumber] != &AnInvalidPinModule)
    return iopins[iPinNumber];

  // error...
  return (PinModule *)0;
}


void PortModule::updatePort()
{
  for (unsigned int i=0; i<mNumIopins; i++) 
    if (iopins[i])
      iopins[i]->updatePinModule();
}
void PortModule::updateUI()
{
  // hmmm nothing 
}

void PortModule::updatePin(unsigned int iPinNumber)
{
  if (iPinNumber < mNumIopins)
    iopins[iPinNumber]->updatePinModule();
}

SignalSink *PortModule::addSink(SignalSink *new_sink, unsigned int iPinNumber)
{
  if (iPinNumber < mNumIopins)
    iopins[iPinNumber]->addSink(new_sink);
  return new_sink;
}

IOPIN *PortModule::addPin(IOPIN *new_pin, unsigned int iPinNumber)
{
  if (iPinNumber < mNumIopins) {
    // If there is not a PinModule for this pin, then add one.
    if (iopins[iPinNumber] == &AnInvalidPinModule)
      iopins[iPinNumber] = new PinModule(this,iPinNumber);

    iopins[iPinNumber]->setPin(new_pin);
  }
  return new_pin;
}

void PortModule::addPinModule(PinModule *newModule, unsigned int iPinNumber)
{
  if (iPinNumber < mNumIopins  && iopins[iPinNumber] == &AnInvalidPinModule)
    iopins[iPinNumber] = newModule;
}

IOPIN *PortModule::getPin(unsigned int iPinNumber)
{
  if (iPinNumber < mNumIopins) {
    return &iopins[iPinNumber]->getPin();
  }
  return 0;
}

//------------------------------------------------------------------------
// PinModule

PinModule::PinModule()
  : PinMonitor(),
    m_cLastControlState('?'), m_cLastSinkState('?'),
    m_cLastSourceState('?'), m_cLastPullupControlState('?'),
    m_defaultSource(0), m_activeSource(0),
    m_defaultControl(0), m_activeControl(0),
    m_defaultPullupControl(0), m_activePullupControl(0),
    m_pin(0), m_port(0), m_pinNumber(0)
{

}

PinModule::PinModule(PortModule *_port, unsigned int _pinNumber, IOPIN *_pin)
  : PinMonitor(),
    m_cLastControlState('?'), m_cLastSinkState('?'),
    m_cLastSourceState('?'), m_cLastPullupControlState('?'),
    m_defaultSource(0), m_activeSource(0),
    m_defaultControl(0), m_activeControl(0),
    m_defaultPullupControl(0), m_activePullupControl(0),
    m_pin(_pin), m_port(_port), m_pinNumber(_pinNumber),
    m_bForcedUpdate(false)
{
  setPin(m_pin);
}

void PinModule::setPin(IOPIN *new_pin)
{
  // Replace our pin only if this one is valid and we don't have one already.
  if (!m_pin && new_pin) {
    m_pin = new_pin;
    m_pin->setMonitor(this);
    m_cLastControlState = getControlState();
    m_cLastSourceState = getSourceState();
  }

}
void PinModule::refreshPinOnUpdate(bool bForcedUpdate)
{
  m_bForcedUpdate = bForcedUpdate;
}

void PinModule::updatePinModule()
{
  if (!m_pin)
    return;

  bool bStateChange=m_bForcedUpdate;

  Dprintf(("PinModule::updatePinModule():%s enter cont=%c,source=%c,pullup%c\n",
	   (m_pin ? m_pin->name().c_str() : "NOPIN"),
	    m_cLastControlState,m_cLastSourceState,m_cLastPullupControlState));

  char cCurrentControlState = getControlState();

  if (cCurrentControlState != m_cLastControlState) {
    m_cLastControlState = cCurrentControlState;
    m_pin->update_direction((cCurrentControlState=='1') ? IOPIN::DIR_INPUT : IOPIN::DIR_OUTPUT,
			    false);
    bStateChange = true;
  }

  char cCurrentSourceState = getSourceState();

  if (cCurrentSourceState != m_cLastSourceState) {
    m_cLastSourceState = cCurrentSourceState;
    m_pin->setDrivingState(cCurrentSourceState);
    bStateChange = true;
  }

  char cCurrentPullupControlState = getPullupControlState();

  if (cCurrentPullupControlState != m_cLastPullupControlState) {
    m_cLastPullupControlState = cCurrentPullupControlState;
    m_pin->update_pullup(m_cLastPullupControlState,false);
    bStateChange = true;
  }  

  if (bStateChange) {

    Dprintf(("PinModule::updatePinModule() exit cont=%c,source=%c,pullup%c\n",
	     m_cLastControlState,m_cLastSourceState,
	     m_cLastPullupControlState));

    if (m_pin->snode)
      m_pin->snode->update();
    else
      setDrivenState(cCurrentSourceState);
  }

}

void PinModule::setDefaultControl(SignalControl *newDefaultControl)
{
  if(!m_defaultControl && newDefaultControl) {
    m_defaultControl = newDefaultControl;
    setControl(m_defaultControl);
  }
  else
	delete newDefaultControl;
}
void PinModule::setControl(SignalControl *newControl)
{
  m_activeControl = newControl ? newControl : m_defaultControl;
}

void PinModule::setDefaultSource(SignalControl *newDefaultSource)
{
  if(!m_defaultSource && newDefaultSource) {
    m_defaultSource = newDefaultSource;
    setSource(m_defaultSource);
  }
}
void PinModule::setSource(SignalControl *newSource)
{
  m_activeSource = newSource ? newSource : m_defaultSource;
}

void PinModule::setDefaultPullupControl(SignalControl *newDefaultPullupControl)
{
  if(!m_defaultPullupControl && newDefaultPullupControl) {
    m_defaultPullupControl = newDefaultPullupControl;
    setPullupControl(m_defaultPullupControl);
  }
}
void PinModule::setPullupControl(SignalControl *newPullupControl)
{
  m_activePullupControl = newPullupControl ? newPullupControl : m_defaultPullupControl;
}

char PinModule::getControlState()
{
  return m_activeControl ? m_activeControl->getState() : '?';
}
char PinModule::getSourceState()
{
  return m_activeSource ? m_activeSource->getState() : '?';
}
char PinModule::getPullupControlState()
{
  return m_activePullupControl ? m_activePullupControl->getState() : '?';
}


void PinModule::setDrivenState(char new3State)
{
  m_cLastSinkState = new3State;

  list <SignalSink *> :: iterator ssi;
  for (ssi = sinks.begin(); ssi != sinks.end(); ++ssi)
    (*ssi)->setSinkState(new3State);
}

void PinModule::setDrivingState(char new3State)
{
  //printf("PinModule::%s -- does nothing\n",__FUNCTION__);
}

void PinModule::set_nodeVoltage(double)
{
  //printf("PinModule::%s -- does nothing\n",__FUNCTION__);
}
void PinModule::putState(char)
{
  //printf("PinModule::%s -- does nothing\n",__FUNCTION__);
}
void PinModule::setDirection()
{
  //printf("PinModule::%s -- does nothing\n",__FUNCTION__);
}

void PinModule::updateUI()
{
  m_port->updateUI();
}



// The IOPORT class is deprecated.

#if defined(OLD_IOPORT_DESIGN)

//-------------------------------------------------------------------
//
// IOPORT::update_stimuli
//
//   input: none
//  return: the states of the stimuli that are driving this ioport
//
//  This member function will update each node that is attached to the
//  iopins of this port. If there are no pins attached, 0 is returned.
//  
//
//-------------------------------------------------------------------
int IOPORT::update_stimuli(void)
{

  unsigned int v = value.get();

  return v ^ get_value();
}



//-------------------------------------------------------------------
//-------------------------------------------------------------------
double IOPORT::get_bit_voltage(unsigned int bit_number)
{

  double v;

  if(pins[bit_number]) {
    if(pins[bit_number]->snode) {
      cout << "Warning IOPORT::get_bit_voltage has changed\n";
      v = pins[bit_number]->snode->get_nodeVoltage();
    }
    else
      v = pins[bit_number]->get_Vth();
  }
  else
    v = (value.get() &  (1<<bit_number)) ?  5.0 : 0.0;


  return v;
}      

//-------------------------------------------------------------------
//-------------------------------------------------------------------
bool IOPORT::get_bit(unsigned int bit_number)
{
  //cout << "get_bit, latch " << internal_latch << " bit " << bit_number << endl;
  return (internal_latch &  (1<<bit_number )) ? true : false;

}

//-------------------------------------------------------------------
//  IOPORT::get_value(void)
//
//   inputs:  none
//  returns:  the current state of the ioport
//
// If there are stimuli attached to the iopins, then their current
// state is obtained. If there aren't any attached, then the last
// value written to the ioport is obtained.
//
//-------------------------------------------------------------------

unsigned int IOPORT::get_value(void)
{

  // Update the stimuli - if there are any

  unsigned int current_value = value.get();
  
  unsigned int i=0;
  unsigned int m=1;

  for(i=0; i<num_iopins; i++, m<<=1) {
    if(pins[i] && pins[i]->snode) {

      double v = pins[i]->snode->get_nodeVoltage();

      if(current_value & m) {
	// this io bit is currently a high
	if(v <= pins[i]->get_h2l_threshold())
	  current_value ^= m;
      } else
	if (v > pins[i]->get_l2h_threshold())
	  current_value ^= m;
    }
  }

  value.put(current_value);

  return(value.get());
}
//-------------------------------------------------------------------
//  IOPORT::get(void)
//
//   inputs:  none
//  returns:  the current state of the ioport
//
// get is identical to get_value except that tracing is performed.
//
//-------------------------------------------------------------------

unsigned int IOPORT::get(void)
{

  trace.raw(read_trace.get() | value.get());
  return get_value();
}

//-------------------------------------------------------------------
//  IOPORT::put(unsigned int new_value)
//
//  inputs:  new_value - 
//                       
//  returns: none
//
//  The I/O Port is updated with the new value. If there are any stimuli
// attached to the I/O pins then they will be updated as well.
//
//-------------------------------------------------------------------

void IOPORT::put(unsigned int new_value)
{


  // The I/O Ports have an internal latch that holds the state of the last
  // write, even if the I/O pins are configured as inputs. If the tris port
  // changes an I/O pin from an input to an output, then the contents of this
  // internal latch will be placed onto the external I/O pin.

  internal_latch = new_value;

  trace.raw(write_trace.get() | value.get());

  unsigned int current_value = value.get();

  value.put(new_value);

  if(stimulus_mask && (current_value != new_value)) {

    unsigned int diff = current_value ^ new_value;

    // Update all I/O pins that have stimuli attached to
    // them and their state is being changed by this put() operation.

    for(unsigned int i = 0; i<num_iopins; i++,diff>>=1)
      if((diff&1) && pins[i] && pins[i]->snode)
	pins[i]->snode->update();
  }

}

//-------------------------------------------------------------------
// void IOPORT::put_value(unsigned int new_value)
//
//  When there's a gui initiated change to the IO port, we'll pass
// though here. There are three things that we do. First, we update
// the I/O port the way the gui asks us. Note however, that it's 
// possible that the gui's requested will go un-honored (if for example,
// we try to force an output to change states or if there's a stimulus
// driving the bus already). 
//   Next, after updating the IO port (and all of it's connected stimuli),
// we'll call the gui to update its windows. This is done through the
// xref->update call.
//   Finally, we'll check all of the I/O pins that have changed as a
// result of the IO port update and individually call each of their 
// cross references.
//
//-------------------------------------------------------------------
void IOPORT::put_value(unsigned int new_value)
{
  unsigned int i,j;
  unsigned int old_value = value.get();
  unsigned int diff;

 
  value.put(new_value);

  // Update the stimuli - if there are any
  if(stimulus_mask)
    update_stimuli();

  
  update();
  
  // Find the pins that have changed states
  diff = (old_value ^ value.get()) & valid_iopins;

  // Update the cross references for each pin that has changed.
  for(i=0,j=1; i<num_iopins; i++,j<<=1) {

    if((j & diff) && pins[i])
      pins[i]->update();

  }

}

//-------------------------------------------------------------------
//-------------------------------------------------------------------
void IOPORT::setbit(unsigned int bit_number, bool new_value)
{

  int bit_mask = 1<<bit_number;
  unsigned int current_value = value.get();
  bool current_bit_value = (current_value & bit_mask) ? true : false;

  if( current_bit_value != new_value)
    {
      trace_register_write();
      value.put(current_value ^ bit_mask);

      internal_latch = (current_value & bit_mask) | (internal_latch & ~bit_mask);
    }

}

//-------------------------------------------------------------------
//-------------------------------------------------------------------
void IOPORT::change_pin_direction(unsigned int bit_number, bool new_direction)
{

  cout << " IOPORT::" << __FUNCTION__ <<'(' << bit_number << ',' << new_direction << ") doesn't do anything.\n";

}

//-------------------------------------------------------------------
// getIO(unsigned int pin_number)
//  return the I/O pin at the bit position requested.
//-------------------------------------------------------------------
IOPIN *IOPORT::getIO(unsigned int pin_number)
{
  if(pins && pin_number < num_iopins)
    return pins[pin_number];

  return 0;
}
//-------------------------------------------------------------------
// attach_iopin
//   This will store a pointer to the iopin that is associated with
// one of the bits of the I/O port.
//
//-------------------------------------------------------------------
IOPIN *IOPORT::addPin(IOPIN * new_pin, unsigned int bit_position)
{

  if(bit_position < num_iopins)

    pins[bit_position] = new_pin;

  else
    cout << "Warning: iopin pin number ("<<bit_position 
	 <<") is invalid for " << name() << ". Max iopins " << num_iopins << '\n';

  if(verbose)
    cout << "attaching iopin to ioport " << name() << '\n';

  return new_pin;
}
//-------------------------------------------------------------------
// attach_iopin
//   This will store a pointer to the iopin that is associated with
// one of the bits of the I/O port.
//
//-------------------------------------------------------------------
void IOPORT::attach_iopin(IOPIN * new_pin, unsigned int bit_position)
{

  if(bit_position < num_iopins)

    pins[bit_position] = new_pin;

  else
    cout << "Warning: iopin pin number ("<<bit_position 
	 <<") is invalid for " << name() << ". Max iopins " << num_iopins << '\n';

  if(verbose)
    cout << "attaching iopin to ioport " << name() << '\n';
}

//-------------------------------------------------------------------
//-------------------------------------------------------------------
void IOPORT::attach_stimulus(stimulus *new_stimulus, unsigned int bit_position)
{

  if(pins  && (bit_position < num_iopins) && pins[bit_position]) {

    stimulus_mask |= (1<<bit_position);

    if(pins[bit_position]->snode == 0)
      {
	// If this I/O pin is not attached to a node yet, 
	// then create a node and attach it.

	pins[bit_position]->snode = new Stimulus_Node();
	pins[bit_position]->snode->attach_stimulus(pins[bit_position]);
      }

    // attach the new stimulus to the same node as this I/O pin's

    pins[bit_position]->snode->attach_stimulus(new_stimulus);
  }

}

//-------------------------------------------------------------------
//-------------------------------------------------------------------
void IOPORT::attach_node(Stimulus_Node *new_node, unsigned int bit_position)
{

  if(pins[bit_position])
    {
      stimulus_mask |= (1<<bit_position);
      //      pins[bit_position]->snode == new_node;
    }
  else
    cout << "Error: attaching node to a non-existing I/O pin.\n";

}

//-------------------------------------------------------------------
// trace_register_write
//   - a wrapper for trace.register_write
// This provides an option for IOPORTs derived from the IOPORT class 
// to override the behavior of IOPORT traces.
//-------------------------------------------------------------------
void IOPORT::trace_register_write(void)
{
  trace.raw(write_trace.get() | value.get());
  //trace.register_write(address,value.get());
}

IOPORT::IOPORT(unsigned int _num_iopins)
  : sfr_register()
{
  stimulus_mask = 0;
  num_iopins = _num_iopins;
  valid_iopins = (1<<num_iopins) - 1;
  address = 0;
  value.put(0);
  internal_latch = 0;

  pins = (IOPIN **) new char[sizeof (IOPIN *) * num_iopins];

  for(unsigned int i=0; i<num_iopins; i++)
    pins[i] = 0;

  new_name("ioport");
}

IOPORT::~IOPORT()
{
    for(unsigned int i=0; i<num_iopins; i++)
    {
	if(pins[i] != 0)
	    delete pins[i];
    }
    delete pins;
}

#endif
