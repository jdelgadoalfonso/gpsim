/*
   Copyright (C) 1998,1999 Scott Dattalo

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

#include "../config.h"
#include "stimuli.h"
#include "uart.h"
#include "14bit-processors.h"
#include "14bit-tmrs.h"

#define p_cpu ((Processor *)cpu)


//#define DEBUG
#if defined(DEBUG)
#define Dprintf(arg) {printf("%s:%d-%s() ",__FILE__,__LINE__,__FUNCTION__); printf arg; }
#else
#define Dprintf(arg) {}
#endif

//--------------------------------------------------
// 
//--------------------------------------------------
class TXSignalSource : public SignalControl
{
public:
  TXSignalSource(_TXSTA *_txsta)
    : m_txsta(_txsta)
  {
    assert(m_txsta);
  }
  char getState()
  {
    return m_txsta->getState();
  }
private:
  _TXSTA *m_txsta;
};

//--------------------------------------------------
//
//--------------------------------------------------

class RXSignalSink : public SignalSink
{
public:
  RXSignalSink(_RCSTA *_rcsta)
    : m_rcsta(_rcsta)
  {
    assert(_rcsta);
  }

  void setSinkState(char new3State)
  {
    m_rcsta->setState(new3State);
  }
private:
  _RCSTA *m_rcsta;
};

//-----------------------------------------------------------
_RCSTA::_RCSTA()
  : rcreg(0), spbrg(0), txsta(0), m_PinModule(0), m_sink(0), m_cRxState('?')
{
}

//-----------------------------------------------------------
_TXSTA::_TXSTA()
  : txreg(0), spbrg(0), m_PinModule(0),  m_source(0), m_cTxState('?')
{
}

//-----------------------------------------------------------
_RCREG::_RCREG()
 : m_rcsta(0), pir_set(0)
{
}

_TXREG::_TXREG()
 : m_txsta(0), pir_set(0)
{
}


_SPBRG::_SPBRG()
  : txsta(0), rcsta(0)
{
}

//-----------------------------------------------------------
// TXREG - USART Transmit Register


void _TXREG::put(unsigned int new_value)
{

  trace.raw(write_trace.get() | value.get());
  value.put(new_value & 0xff);

  Dprintf(("txreg just got a new value:0x%x\n",new_value));

  // The transmit register has data,
  // so clear the TXIF flag

  full();

  if(m_txsta &&
     ( (m_txsta->value.get() & (_TXSTA::TRMT | _TXSTA::TXEN)) == (_TXSTA::TRMT | _TXSTA::TXEN)))
    {
      // If the transmit buffer is empty and the transmitter is enabled
      // then transmit this new data now...

      m_txsta->start_transmitting();
    }

}

void _TXREG::put_value(unsigned int new_value)
{

  put(new_value);

  update();
}

//-----------------------------------------------------------
// TXSTA - setIOpin - assign the I/O pin associated with the
// the transmitter.


void _TXSTA::setIOpin(PinModule *newPinModule)
{
  if (!m_source) {
    m_source = new TXSignalSource(this);
    m_PinModule = newPinModule;
  }

}
//-----------------------------------------------------------
// TXSTA - putTXState - update the state of the TX output pin
//

void _TXSTA::putTXState(char newTXState)
{

  m_cTxState = newTXState;

  if (m_PinModule)
    m_PinModule->updatePinModule();

}

//-----------------------------------------------------------
// TXSTA - Transmit Register Status and Control

void _TXSTA::put_value(unsigned int new_value)
{

  put(new_value);

  update();

}

void _TXSTA::put(unsigned int new_value)
{

  unsigned int old_value = value.get();

  trace.raw(write_trace.get() | value.get());

  // The TRMT bit is controlled entirely by hardware.
  // It is high if the TSR has any data.

  value.put((new_value & ~TRMT) | ( (bit_count) ? 0 : TRMT));

  Dprintf((" TXSTA value=0x%x\n",value.get()));


  if( (old_value ^ value.get()) & TXEN) {

    // The TXEN bit has changed states.
    //
    // If transmitter is being enabled and the transmit register
    // has some data that needs to be sent, then start a
    // transmission.
    // If the transmitter is being disabled, then abort any
    // transmission.

    if(value.get() & TXEN) {
      Dprintf(("TXSTA - enabling transmitter\n"));
      if (m_PinModule)
	m_PinModule->setSource(m_source);
      if(txreg) {
	txreg->empty();
#if 0
	if(txreg->is_empty()) {
	  txreg->empty();
	} else {
          Dprintf(("start_transmitting\n"));
	  start_transmitting();
	}
#endif
      }
    } else {
      stop_transmitting();
      if (m_PinModule)
	m_PinModule->setSource(0);

    }
  }
}
//------------------------------------------------------------
//
char _TXSTA::getState()
{

  return m_cTxState;
}


// _TXSTA::stop_transmitting()
//
void _TXSTA::stop_transmitting()
{
  Dprintf(("stopping a USART transmission\n"));

  bit_count = 0;
  value.put(value.get() | TRMT);

  // It's not clear from the documentation as to what happens
  // to the TXIF when we are aborting a transmission. According
  // to the documentation, the TXIF is set when the TXEN bit
  // is set. In other words, when the Transmitter is enabled
  // the txreg is emptied (and hence TXIF set). But what happens
  // when TXEN is cleared? Should we clear TXIF too?
  // 
  // There is one sentence that says when the transmitter is
  // disabled that the whole transmitter is reset. So I interpret
  // this to mean that the TXIF gets cleared. I could be wrong
  // (and I don't have a way to test it on a real device).
  // 
  // Another interpretation is that TXIF retains it state 
  // through changing TXEN. However, when SPEN (serial port) is
  // set then the whole usart is reinitialized and TXIF will
  // get set.
  //
  //  txreg->full();   // Clear TXIF

}

void _TXSTA::start_transmitting()
{
  Dprintf(("starting a USART transmission\n"));

  // Build the serial byte that's about to be transmitted.
  // I doubt the pic really does this way, but gpsim builds
  // the entire bit stream including the start bit, 8 data 
  // bits, optional 9th data bit and the stop, and places
  // this into the tsr register. But since the contents of
  // the tsr are inaccessible, I guess we'll never know.
  //
  // (BTW, if you look carefully you may puzzle over why
  //  there appear to be 2 stop bits in the packet built
  //  below. Well, it's because the way gpsim implements
  //  the transmit logic. The second stop bit doesn't
  //  actually get transmitted - it merely causes the first
  //  stop bit to get transmitted before the TRMT bit is set.
  //  [Recall that the TRMT bit indicates when the tsr 
  //  {transmit shift register} is empty. It's not tied to
  //  an interrupt pin, so the pic application software
  //  most poll this bit. This bit is set after the STOP
  //  bit is transmitted.] This is a cheap trick that saves
  //  one comparison in the callback code.)

  // The start bit, which is always low, occupies bit position
  // zero. The next 8 bits come from the txreg.
  if(!txreg)
    return;

  tsr = txreg->value.get() << 1;

  // Is this a 9-bit data transmission?
  if(value.get() & TX9)
    {
      // Copy the stop bit and the 9th data bit to the tsr.
      // (See note above for the reason why two stop bits 
      // are appended to the packet.)

      tsr |= ( (value.get() & TX9D) ? (7<<9) : (6<<9));
      bit_count = 12;  // 1 start, 9 data, 1 stop , 1 delay
    }
  else
    {
      // The stop bit is always high. (See note above
      // for the reason why two stop bits are appended to
      // the packet.)
      tsr |= (3<<9);
      bit_count = 11;  // 1 start, 8 data, 1 stop , 1 delay
    }


  // Set a callback breakpoint at the next SPBRG edge

  if(cpu)
    get_cycles().set_break(spbrg->get_cpu_cycle(1), this);

  // The TSR now has data, so clear the Transmit Shift 
  // Register Status bit.

  trace.raw(write_trace.get() | value.get());
  value.put(value.get() & ~TRMT);

  // Tell the TXREG that its data has been consumed.

  txreg->empty();

}

void _TXSTA::transmit_a_bit()
{


  if(bit_count) {

    Dprintf(("Transmit bit #%x: bit val:%d time:0x%llx\n",bit_count, (tsr&1),get_cycles().value));

    putTXState(tsr&1 ? '1' : '0');

    tsr >>= 1;

    --bit_count;

  }

}

void _TXSTA::callback()
{
  Dprintf(("TXSTA callback - time:%llx\n",get_cycles().value));

  transmit_a_bit();

  if(!bit_count) {

    // tsr is empty.
    // If there is any more data in the TXREG, then move it to
    // the tsr and continue transmitting other wise set the TRMT bit

    // (See the note above about the 'extra' stop bit that was stuffed
    // into the tsr register. 

    if(txreg && txreg->is_empty())
      value.put(value.get() | TRMT);
    else
      start_transmitting();

  } else  {
    // bit_count is non zero which means there is still
    // data in the tsr that needs to be sent.

    if(cpu) {
      get_cycles().set_break(spbrg->get_cpu_cycle(1),this);
    }
  }

}

void _TXSTA::callback_print()
{
  cout << "TXSTA " << name() << " CallBack ID " << CallBackID << '\n';
}

bool _TXREG::is_empty()
{
  if (pir_set)
    return(pir_set->get_txif());
  return true;
}
void _TXREG::empty()
{
  Dprintf(("txreg::empty - setting TXIF\n"));
  if (pir_set)
    pir_set->set_txif();

}
void _TXREG::full()
{
  Dprintf(("txreg::full - clearing TXIF\n"));
  if(pir_set)
    pir_set->clear_txif();
}
void _TXREG::assign_pir_set(PIR_SET *new_pir_set)
{
  pir_set = new_pir_set;
}

//-----------------------------------------------------------
// Receiver portion of the USART
//-----------------------------------------------------------
//
// First RCSTA -- Receiver Control and Status 
// The RCSTA class controls the usart reception. The PIC usarts have
// two modes: synchronous and asynchronous.
// Asynchronous reception:
//   Asynchronous reception means that there is no external clock
// available for telling the usart when to sample the data. Sampling
// timing is all based upon the PIC's oscillator. The SPBRG divides
// this clock down to a frequency that's appropriate to the data
// being received. (e.g. 9600 baud defines the rate at which data
// will be sent to the pic - 9600 bits per second.) The start bit,
// which is a high to low transition on the receive line, defines
// when the usart should start sampling the incoming data.
//   The pic usarts sample asynchronous data three times in "approximately
// the middle" of each bit. The data sheet is not exactly specific
// on what's the middle. Consequently, gpsim takes a stab/ educated
// guess on when these three samples are to be taken. Once the
// three samples are taken, then simple majority summing determines
// the sample e.g. if two out of three of the samples are high, then
// then the data bit is considered high.
//
//-----------------------------------------------------------
// RCSTA::put
//
void _RCSTA::put(unsigned int new_value)
{
  unsigned int diff;

  diff = new_value ^ value.get();
  trace.raw(write_trace.get() | value.get());
  value.put( ( value.get() & (RX9D | OERR | FERR) )   |  (new_value & ~(RX9D | OERR | FERR)));

  if(!txsta || !txsta->txreg)
    return;
  // First check whether or not the serial port is being enabled
  if(diff & SPEN) {

    if(value.get() & SPEN) {
      spbrg->start();
      // Make the tx line high when the serial port is enabled.
      txsta->putTXState('1');
      txsta->txreg->empty();
    } else {

      // Completely disable the usart:

      txsta->stop_transmitting();
      txsta->txreg->full();         // Turn off TXIF
      stop_receiving();

      return;
    }

  }

  if(!(txsta->value.get() & _TXSTA::SYNC)) {

    // Asynchronous receive.
    if( (value.get() & (SPEN | CREN)) == (SPEN | CREN) ) {

      // The receiver is enabled. Now check to see if that just happened
      
      if(diff & (SPEN | CREN)) {
    
	// The serial port has just been enabled. 
	start_receiving();

	// If the rx line is low, then go ahead and start receiving now.
	//if(uart_port && !uart_port->get_bit(rx_bit))
	if (m_cRxState == '0' || m_cRxState == 'w')
	  receive_start_bit();
      }

    } else {
      // The serial port is not enabled.

      state = RCSTA_DISABLED;
    }

  } else
    cout << "not doing syncronous receptions yet\n";

}

void _RCSTA::put_value(unsigned int new_value)
{

  put(new_value);

  update();
}

//-----------------------------------------------------------
// RCSTA - setIOpin - assign the I/O pin associated with the
// the receiver.


void _RCSTA::setIOpin(PinModule *newPinModule)
{
  if (!m_sink) {
    m_sink = new RXSignalSink(this);
    m_PinModule = newPinModule;
    if (m_PinModule)
      m_PinModule->addSink(m_sink);
  }

}

//-----------------------------------------------------------
// RCSTA - setState
// This gets called whenever there's a change detected on the RX pin.
// The usart is only interested in those changes when it is waiting
// for the start bit. Otherwise, the rcsta callback function will sample
// the rx pin (if we're receiving).


void _RCSTA::setState(char new_RxState)
{
  m_cRxState = new_RxState;

  if( (state == RCSTA_WAITING_FOR_START) && (m_cRxState =='0' || m_cRxState=='w'))
    receive_start_bit();

}
//-----------------------------------------------------------
// RCSTA::receive_a_bit(unsigned int bit)
//
// A new bit needs to be copied to the the Receive Shift Register.
// If the receiver is receiving data, then this routine will copy
// the incoming bit to the rsr. If this is the last bit, then a 
// check will be made to see if we need to set up for the next 
// serial byte.
// If this is not the last bit, then the receive state machine.

void _RCSTA::receive_a_bit(unsigned int bit)
{

  // If we're waiting for the start bit and this isn't it then
  // we don't need to look any further
  Dprintf(("receive_a_bit state:%d bit:%d time:0x%llx\n",state,bit,get_cycles().value));

  if( state == RCSTA_MAYBE_START) {
    if (bit)
      state = RCSTA_WAITING_FOR_START;
    else
      state = RCSTA_RECEIVING;
    return;
  }
  if (bit_count == 0) {
    // we should now have the stop bit
    if (bit) {
      // got the stop bit
      // If the rxreg has data from a previous reception then
      // we have a receiver overrun error.
      // cout << "rcsta.rsr is full\n";

      if((value.get() & RX9) == 0)
        rsr >>= 1;

      // copy the rsr to the fifo
      if(rcreg)
        rcreg->push( rsr & 0xff);

      Dprintf(("_RCSTA::receive_a_bit received 0x%02X\n",rsr & 0xff));

    } else {
      //not stop bit; discard the data and go back receiving
    }
    // If we're continuously receiving, then set up for the next byte.
    // FIXME -- may want to set a half bit delay before re-starting...
    if(value.get() & CREN)
      start_receiving();
    else
      state = RCSTA_DISABLED;
    return;
  }


  // Copy the bit into the Receive Shift Register
  if(bit)
    rsr |= 1<<9;

  //cout << "Receive bit #" << bit_count << ": " << (rsr&(1<<9)) << '\n';

  rsr >>= 1;
  bit_count--;

}

void _RCSTA::stop_receiving()
{

  rsr = 0;
  bit_count = 0;
  state = RCSTA_DISABLED;

}

void _RCSTA::start_receiving()
{
  //cout << "The USART is starting to receive data\n";

  rsr = 0;
  sample = 0;

  // Is this a 9-bit data reception?
  if(value.get() & RX9)
    {
      bit_count = 9;
    }
  else
    {
      bit_count = 8;
    }

  state = RCSTA_WAITING_FOR_START;

}
void _RCSTA::overrun()
{
  value.put(value.get() | _RCSTA::OERR);
}

void _RCSTA::set_callback_break(unsigned int spbrg_edge)
{
  //  last_cycle = cycles.value;

  if(cpu && spbrg)
    get_cycles().set_break(get_cycles().value + (spbrg->value.get() + 1) * spbrg_edge, this);

}
void _RCSTA::receive_start_bit()
{
  Dprintf(("USART received a start bit\n"));

  if((value.get() & (CREN | SREN)) == 0) {
    Dprintf(("  but not enabled\n"));
    return;
  }
  
  if(txsta && (txsta->value.get() & _TXSTA::BRGH))
    set_callback_break(BRGH_FIRST_MID_SAMPLE);
  else
    set_callback_break(BRGL_FIRST_MID_SAMPLE);

  sample = 0;
  sample_state = RCSTA_WAITING_MID1;
  state = RCSTA_MAYBE_START;
}

//------------------------------------------------------------
void _RCSTA::callback()
{

  Dprintf(("RCSTA callback. time:0x%llx\n",cycles.value));

  // A bit is sampled 3 times.

  switch(sample_state) {
  case RCSTA_WAITING_MID1:
    if (m_cRxState == '1' || m_cRxState == 'W')
      sample++;

    if(txsta && (txsta->value.get() & _TXSTA::BRGH))
      set_callback_break(BRGH_SECOND_MID_SAMPLE - BRGH_FIRST_MID_SAMPLE);
    else
      set_callback_break(BRGL_SECOND_MID_SAMPLE - BRGL_FIRST_MID_SAMPLE);

    sample_state = RCSTA_WAITING_MID2;

    break;

  case RCSTA_WAITING_MID2:
    if (m_cRxState == '1' || m_cRxState == 'W')
      sample++;

    if(txsta && (txsta->value.get() & _TXSTA::BRGH))
      set_callback_break(BRGH_THIRD_MID_SAMPLE - BRGH_SECOND_MID_SAMPLE);
    else
      set_callback_break(BRGL_THIRD_MID_SAMPLE - BRGL_SECOND_MID_SAMPLE);

    sample_state = RCSTA_WAITING_MID3;

    break;

  case RCSTA_WAITING_MID3:
    if (m_cRxState == '1' || m_cRxState == 'W')
      sample++;

    receive_a_bit( (sample>=2));
    sample = 0;

    // If this wasn't the last bit then go ahead and set a break for the next bit.
    if(state==RCSTA_RECEIVING) {
      if(txsta && (txsta->value.get() & _TXSTA::BRGH))
	set_callback_break(TOTAL_BRGH_STATES -(BRGH_THIRD_MID_SAMPLE - BRGH_FIRST_MID_SAMPLE));
      else
	set_callback_break(TOTAL_BRGL_STATES -(BRGL_THIRD_MID_SAMPLE - BRGL_FIRST_MID_SAMPLE));

      sample_state = RCSTA_WAITING_MID1;
    }

    break;

  default:
    //cout << "Error RCSTA callback with bad state\n";
    // The receiver was probably disabled in the middle of a reception.
    ;
  }

}

//-----------------------------------------------------------
void _RCSTA::callback_print()
{
  cout << "RCSTA " << name() << " CallBack ID " << CallBackID << '\n';
}

//-----------------------------------------------------------
// RCREG
//
void _RCREG::push(unsigned int new_value)
{
  trace.raw(write_trace.get() | value.get());

  if(fifo_sp >= 2) {

    if (m_rcsta)
      m_rcsta->overrun();
      
    Dprintf(("receive overrun\n"));

  } else {

    Dprintf(("pushing uart reception onto rcreg stack, value received=0x%x\n",new_value));
    fifo_sp++;
    oldest_value = value.get();
    value.put(new_value);
  }

  if(pir_set)
    pir_set->set_rcif();

}

void _RCREG::pop()
{

  if(fifo_sp == 0)
    return;

  if(--fifo_sp == 1)
    value.put(oldest_value);

  if(fifo_sp == 0 && pir_set)
    pir_set->clear_rcif();

}

unsigned int _RCREG::get_value()
{

  return value.get();

}

unsigned int _RCREG::get()
{

  pop();
  trace.raw(read_trace.get() | value.get());
  return value.get();
}
void _RCREG::assign_pir_set(PIR_SET *new_pir_set)
{
  pir_set = new_pir_set;
}


//-----------------------------------------------------------
// SPBRG - Serial Port Baud Rate Generator
//
// The SPBRG is essentially a continuously running programmable
// clock. (Note that this will slow the simulation down if the
// serial port is not used. Perhaps gpsim needs some kind of
// pragma type thing to disable cpu intensive peripherals...)

void _SPBRG::get_next_cycle_break()
{
  unsigned int cpi = (cpu) ? p_cpu->get_ClockCycles_per_Instruction() : 4;


  if(txsta && (txsta->value.get() & _TXSTA::SYNC))
    {
      // Synchronous mode
      future_cycle = last_cycle + (value.get() + 1)*4/cpi;
    }
  else
    {
      // Asynchronous mode
      if(txsta && (txsta->value.get() & _TXSTA::BRGH))
	future_cycle = last_cycle + (value.get() + 1)*16/cpi;
      else
	future_cycle = last_cycle + (value.get() + 1)*64/cpi;
    }

  if(cpu)
    get_cycles().set_break(future_cycle, this);

  Dprintf(("SPBRG::callback next break at 0x%llx\n",future_cycle));
  
}

void _SPBRG::start()
{

  if(cpu)
    last_cycle = get_cycles().value;
  start_cycle = last_cycle;

  get_next_cycle_break();

  Dprintf((" SPBRG::start   last_cycle:0x%llx: future_cycle:0x%llx\n",last_cycle,future_cycle));


}

//--------------------------
//guint64 _SPBRG::get_last_cycle()
//
// Get the cpu cycle corresponding to the last edge of the SPBRG
//

guint64 _SPBRG::get_last_cycle()
{

  // There's a chance that a SPBRG break point exists on the current
  // cpu cycle, but has not yet been serviced.
  if(cpu)
    return( (get_cycles().value == future_cycle) ? future_cycle : last_cycle);
  else
    return 0;
}
//--------------------------
//guint64 _SPBRG::get_cpu_cycle(unsigned int edges_from_now)
//
//  When the SPBRG is enabled, it becomes a free running counter
// that's synchronous with the cpu clock. The frequency of the
// counter depends on the mode of the usart:
//
//  Synchronous mode:
//    baud = cpu frequency / 4 / (spbrg.value + 1)
//
//  Asynchronous mode:
//   high frequency:
//     baud = cpu frequency / 16 / (spbrg.value + 1)
//   low frequency:
//     baud = cpu frequency / 64 / (spbrg.value + 1)
//
// What this routine will do is return the cpu cycle corresponding
// to a (rising) edge of the spbrg clock. 

guint64 _SPBRG::get_cpu_cycle(unsigned int edges_from_now)
{

  unsigned int cpi = (cpu) ? p_cpu->get_ClockCycles_per_Instruction() : 4;

  // There's a chance that a SPBRG break point exists on the current
  // cpu cycle, but has not yet been serviced. 
  guint64 cycle = (get_cycles().value == future_cycle) ? future_cycle : last_cycle;

  if(txsta && (txsta->value.get() & _TXSTA::SYNC))
    // Synchronous mode
    return ( edges_from_now * (value.get() + 1)*4/cpi + cycle);

  else  {
      // Asynchronous mode
    if(txsta && (txsta->value.get() & _TXSTA::BRGH))
      return ( edges_from_now * (value.get() + 1)*16/cpi + cycle);
    else
      return ( edges_from_now * (value.get() + 1)*64/cpi + cycle);
  }

}
void _SPBRG::callback()
{

  last_cycle = get_cycles().value;

  Dprintf(("SPBRG rollover at cycle:0x%llx\n",last_cycle));

  if(rcsta && (rcsta->value.get() & _RCSTA::SPEN))
    {

      // If the serial port is enabled, then set another 
      // break point for the next clock edge.
      get_next_cycle_break();

    }
}
//--------------------------------------------------
/*
bool TXREG_14::is_empty()
{
  return(pir_set->get_txif());
}

void TXREG_14::empty()
{
  Dprintf(("txreg::empty - setting TXIF\n"));
  pir_set->set_txif();
}

void TXREG_14::full()
{
  Dprintf(("txreg::full - clearing TXIF\n"));
  if(pir_set)
    pir_set->clear_txif();
}

void RCREG_14::push(unsigned int new_value)
{

  _RCREG::push(new_value);

  pir_set->set_rcif();

}

void RCREG_14::pop()
{

  _RCREG::pop();
  if(fifo_sp == 0)
    pir_set->clear_rcif();

}
*/

//--------------------------------------------------
// member functions for the USART
//--------------------------------------------------
void USART_MODULE::initialize(PIR_SET *pir_set,
			      PinModule *tx_pin, PinModule *rx_pin,
			      _TXREG *_txreg, _RCREG *_rcreg)
{
  assert(_txreg && _rcreg);

  spbrg.txsta = &txsta;
  spbrg.rcsta = &rcsta;

  txreg = _txreg;
  txreg->assign_pir_set(pir_set);
  txreg->assign_txsta(&txsta);

  rcreg = _rcreg;
  rcreg->assign_pir_set(pir_set);
  rcreg->assign_rcsta(&rcsta);

  txsta.txreg = txreg;
  txsta.spbrg = &spbrg;
  txsta.bit_count = 0;
  txsta.setIOpin(tx_pin);

  rcsta.rcreg = rcreg;
  rcsta.spbrg = &spbrg;
  rcsta.txsta = &txsta;
  rcsta.setIOpin(rx_pin);

}

//--------------------------------------------------
USART_MODULE::USART_MODULE()
  : txreg(0), rcreg(0)
{
}

//--------------------------------------------------
USART_MODULE14::USART_MODULE14()
{
}
