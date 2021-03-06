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
#include <iostream.h>

#include "uart.h"
#include "14bit-processors.h"
#include "14bit-tmrs.h"

#include "xref.h"

//-----------------------------------------------------------
// TXREG - USART Transmit Register


void _TXREG::put(unsigned int new_value)
{

  value = new_value & 0xff;

  if(verbose)
    cout << "txreg just got a new value\n";

  if(txsta->value & _TXSTA::TRMT)
    {
      // If the transmit buffer is empty, 
      // then transmit this new data now...

      txsta->start_transmitting();
    }
  else
    {
      // The transmit buffer is busy transmitting,
      // So clear the TXIF flag and wait.
      full();
    }
}

void _TXREG::put_value(unsigned int new_value)
{

  put(new_value);

  trace.register_write(address,value);

}

//-----------------------------------------------------------
// TXSTA - Transmit Register Status and Control

void _TXSTA::put_value(unsigned int new_value)
{

  value = (new_value & ~TRMT) | ( (bit_count) ? 0 : TRMT);

}

void _TXSTA::put(unsigned int new_value)
{

  put_value(new_value);

  trace.register_write(address,value);

}

void _TXSTA::start_transmitting(void)
{
  //cout << "starting a USART transmission\n";

  // Build the serial byte that's about to be transmitted.
  // I doubt the pic really does this way, but gpsim builds
  // the entire bit stream including the start bit, 8 data 
  // bits, optional 9th data bit and the stop, and places
  // this into the tsr register. But since the contents of
  // the tsr are inaccessible, I guess we'll never know.


  // The start bit, which is always low, occupies bit position
  // zero. The next 8 bits come from the txreg.

  tsr = txreg->value << 1;

  // Is this a 9-bit data transmission?
  if(value & TX9)
    {
      // Copy the stop bit and the 9th data bit to the tsr.
      tsr |= ( (value & TX9D) ? (3<<9) : (2<<9));
      bit_count = 11;
    }
  else
    {
      // The stop bit is always high.
      tsr |= (1<<9);
      bit_count = 10;
    }


  // Set a callback breakpoint at the next SPBRG edge

  cpu->cycles.set_break(spbrg->get_cpu_cycle(1), this);

  // The TSR now has data, so clear the Transmit Shift 
  // Register Status bit.

  value &= ~TRMT;
  trace.register_write(address,value);

  // Tell the TXREG that its data has been consumed.

  txreg->empty();

}

void _TXSTA::transmit_a_bit(void)
{


  if(bit_count)
    {

      //cout << "Transmit bit #" << bit_count << ": " << (tsr&1) << '\n';
      txpin->put_state(tsr&1);

      tsr >>= 1;

      --bit_count;

    }

}

void _TXSTA::callback(void)
{

  //cout << "TXSTA callback " << (cpu->cycles.value) << '\n';

  transmit_a_bit();

  if(!bit_count) {
    // tsr is empty.
    // If there is any more data in the TXREG, then move it to
    // the tsr and continue transmitting other wise set the TRMT bit

    if(txreg->is_empty())
      value |= TRMT;
    else
      start_transmitting();

  } else  {
    // bit_count is non zero which means there is still
    // data in the tsr that needs to be sent.

    if(value & BRGH) 
      cpu->cycles.set_break(spbrg->get_cpu_cycle(TOTAL_BRGH_STATES),this);
    else
      cpu->cycles.set_break(spbrg->get_cpu_cycle(TOTAL_BRGL_STATES),this);

  }

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

  diff = new_value ^ value;
  value = ( value & (RX9D | OERR | FERR) )   |  (new_value & ~(RX9D | OERR | FERR));

  // First check whether or not the serial port is being enabled
  if(value & diff & SPEN) {
    spbrg->start();
    // Make the tx line high when the serial port is enabled.
    txsta->txpin->put_state(1);
  }

  if(!(txsta->value & _TXSTA::SYNC)) {

    // Asynchronous receive.
    if( (value & (SPEN | CREN)) == (SPEN | CREN) ) {

      // The receiver is enabled. Now check to see if that just happened
      
      if(diff & (SPEN | CREN)) {
    
	// The serial port has just been enabled. 
	start_receiving();

	// If the rx line is low, then go ahead and start receiving now.
	if(!uart_port->get_bit(rx_bit))
	  receive_start_bit();
      }

    } else {
      // The serial port is not enabled.

      state = RCSTA_DISABLED;
    }

  } else
    cout << "not doing syncronous receptions yet\n";

  trace.register_write(address,value);
  
}

void _RCSTA::put_value(unsigned int new_value)
{

  put(new_value);

  if(xref)
    xref->update();
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

  if(bit_count)
    {

      // If we're waiting for the start bit and this isn't it then
      // we don't need to look any further
      if( (state == RCSTA_WAITING_FOR_START) && bit)
	return;

      // Copy the bit into the Receive Shift Register
      if(bit)
	rsr |= 1<<9;

      //cout << "Receive bit #" << bit_count << ": " << (rsr&(1<<9)) << '\n';

      rsr >>= 1;

      if(--bit_count == 0)
	{
	  // rsr is full.

	  // If the rxreg has data from a previous reception then
	  // we have a receiver overrun error.
	  //cout << "rcsta.rsr is full\n";

	  if((value & RX9) == 0)
	    rsr >>= 1;

	  // copy the rsr to the fifo
	  rcreg->push( rsr & 0xff);

	  // If we're continuously receiving, then set up for the next byte.
	  // FIXME -- may want to set a half bit delay before re-starting...
	  if(value & CREN)
	    start_receiving();
	  else
	    state = RCSTA_DISABLED;

	}

    }

}

void _RCSTA::start_receiving(void)
{
  //cout << "The USART is starting to receive data\n";

  rsr = 0;
  sample = 0;

  // Is this a 9-bit data reception?
  if(value & RX9)
    {
      bit_count = 10;
    }
  else
    {
      bit_count = 9;
    }

  state = RCSTA_WAITING_FOR_START;

}

void _RCSTA::set_callback_break(unsigned int spbrg_edge)
{
  //  last_cycle = cpu->cycles.value;

  cpu->cycles.set_break(spbrg->get_cpu_cycle(spbrg_edge), this);

}
void _RCSTA::receive_start_bit(void)
{

  //cout << "USART received a start bit\n";

  if(txsta->value & _TXSTA::BRGH)
    set_callback_break(BRGH_FIRST_MID_SAMPLE);
  else
    set_callback_break(BRGL_FIRST_MID_SAMPLE);

  sample = 0;
  state = RCSTA_WAITING_MID1;

}

void _RCSTA::callback(void)
{

  //cout << "RCSTA callback " << (cpu->cycles.value) << '\n';

  switch(state) {
  case RCSTA_WAITING_MID1:
    if(uart_port->get_bit(rx_bit))
      sample++;

    if(txsta->value & _TXSTA::BRGH)
      set_callback_break(BRGH_SECOND_MID_SAMPLE - BRGH_FIRST_MID_SAMPLE);
    else
      set_callback_break(BRGL_SECOND_MID_SAMPLE - BRGL_FIRST_MID_SAMPLE);

    state = RCSTA_WAITING_MID2;

    break;

  case RCSTA_WAITING_MID2:
    if(uart_port->get_bit(rx_bit))
      sample++;

    if(txsta->value & _TXSTA::BRGH)
      set_callback_break(BRGH_THIRD_MID_SAMPLE - BRGH_SECOND_MID_SAMPLE);
    else
      set_callback_break(BRGL_THIRD_MID_SAMPLE - BRGL_SECOND_MID_SAMPLE);

    state = RCSTA_WAITING_MID3;

    break;

  case RCSTA_WAITING_MID3:
    if(uart_port->get_bit(rx_bit))
      sample++;

    receive_a_bit( (sample>=2));
    sample = 0;

    // If this wasn't the last bit then go ahead and set a break for the next bit.
    if(state==RCSTA_WAITING_MID3) {
      if(txsta->value & _TXSTA::BRGH)
	set_callback_break(TOTAL_BRGH_STATES -(BRGH_THIRD_MID_SAMPLE - BRGH_FIRST_MID_SAMPLE));
      else
	set_callback_break(TOTAL_BRGL_STATES -(BRGL_THIRD_MID_SAMPLE - BRGL_FIRST_MID_SAMPLE));

      state = RCSTA_WAITING_MID1;
    }

    break;

  default:
    cout << "Error RCSTA callback with bad state\n";

  }

}

//-----------------------------------------------------------
// RCREG
//
void _RCREG::push(unsigned int new_value)
{
  if(fifo_sp >= 2)
    {
      rcsta->value |= _RCSTA::OERR;
      if(verbose)
	cout << "receive overrun\n";
    }
  else
    {
      //cout << "pushing uart reception onto rcreg stack\n";
      fifo_sp++;
      oldest_value = value;
      value = new_value;
      trace.register_write(address,value);
    }

}

void _RCREG::pop(void)
{

  if(fifo_sp == 0)
    return;

  if(--fifo_sp == 1)
    value = oldest_value;

}

unsigned int _RCREG::get_value(void)
{

  return value;

}

unsigned int _RCREG::get(void)
{

  pop();
  trace.register_read(address,value);
  return value;
}


//-----------------------------------------------------------
// SPBRG - Serial Port Baud Rate Generator
//
// The SPBRG is essentially a continuously running programmable
// clock. (Note that this will slow the simulation down if the
// serial port is not used. Perhaps gpsim needs some kind of
// pragma type thing to disable cpu intensive peripherals...)

void _SPBRG::get_next_cycle_break(void)
{

  if(txsta->value & _TXSTA::SYNC)
    {
      // Synchronous mode
      future_cycle = last_cycle + (value + 1)*4;
    }
  else
    {
      // Asynchronous mode
      if(txsta->value & _TXSTA::BRGH)
	future_cycle = last_cycle + (value + 1)*16;
      else
	future_cycle = last_cycle + (value + 1)*64;
    }

  cpu->cycles.set_break(future_cycle, this);


}

void _SPBRG::start(void)
{

  if(verbose)
    cout << "SPBRG::start\n";

  last_cycle = cpu->cycles.value;
  start_cycle = last_cycle;

  get_next_cycle_break();

  if(verbose)
    cout << " SPBRG::start   last_cycle = " << 
      hex << last_cycle << " future_cycle = " << future_cycle << '\n';

  if(verbose)
    cpu->cycles.dump_breakpoints();

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

  // There's a chance that a SPBRG break point exists on the current
  // cpu cycle, but has not yet been serviced. 
  guint64 cycle = (cpu->cycles.value == future_cycle) ? future_cycle : last_cycle;

  if(txsta->value & _TXSTA::SYNC)
    {
      // Synchronous mode
      return ( edges_from_now * (value + 1)*4 + cycle);
    }
  else
    {
      // Asynchronous mode
      if(txsta->value & _TXSTA::BRGH)
	return ( edges_from_now * (value + 1)*16 + cycle);
      else
	return ( edges_from_now * (value + 1)*64 + cycle);
    }


#if 0

const guint64 SPBRG_SYNC_MASK = ~((guint64) 3);
const guint64 SPBRG_ASYNC_LO  = ~((guint64) 63);
const guint64 SPBRG_ASYNC_HI = ~((guint64) 15);
  
  guint64 cycle;

  // Get the number of cycles the sprbg has been active
  cycle = (cpu->cycles.value - start_cycle);

  if(txsta->value & _TXSTA::SYNC)
    {
      // Synchronous mode
      cycle = (cycle & SPBRG_SYNC_MASK) + edges_from_now * (value + 1)*4 + start_cycle;
    }
  else
    {
      // Asynchronous mode
      if(txsta->value & _TXSTA::BRGH)
	cycle = (cycle & SPBRG_ASYNC_HI) + edges_from_now * (value + 1)*16 + start_cycle;
      else
	cycle = (cycle & SPBRG_ASYNC_LO) + edges_from_now * (value + 1)*64 + start_cycle;
    }

  return cycle;
#endif
}
void _SPBRG::callback(void)
{

  last_cycle = cpu->cycles.value;

  //cout << "SPBRG rollover at cycle " << last_cycle << '\n';

  if(rcsta->value & _RCSTA::SPEN)
    {

      // If the serial port is enabled, then set another 
      // break point for the next clock edge.
      get_next_cycle_break();

    }
}
//--------------------------------------------------

bool TXREG_14::is_empty(void)
{
  return(pir1->get_txif());
}

void TXREG_14::empty(void)
{
  pir1->set_txif();
}

void TXREG_14::full(void)
{
  pir1->clear_txif();
}

void RCREG_14::push(unsigned int new_value)
{

  _RCREG::push(new_value);

  pir1->set_rcif();

}

void RCREG_14::pop(void)
{

  _RCREG::pop();
  if(fifo_sp == 0)
    pir1->clear_rcif();

}


//--------------------------------------------------
// member functions for the USART
//--------------------------------------------------
void USART_MODULE14::initialize(_14bit_processor *new_cpu, PIR1 *pir1, IOPORT *uart_port)
{
  cpu = new_cpu;

  spbrg.txsta = &txsta;
  spbrg.rcsta = &rcsta;

  txreg.pir1 = pir1;
  txreg.txsta = &txsta;

  txsta.txreg = &txreg;
  txsta.spbrg = &spbrg;
  txsta.txpin = uart_port->pins[6];
  txsta.bit_count = 0;

  rcsta.rcreg = &rcreg;
  rcsta.spbrg = &spbrg;
  rcsta.txsta = &txsta;
  rcsta.uart_port = uart_port;
  rcsta.rx_bit = 7;

  rcreg.rcsta = &rcsta;
  rcreg.pir1 = pir1;

  //  spbrg.start();

}

// This gets called whenever there's a change detected on the RX pin.
// The usart is only interested in those changes when it is waiting
// for the start bit. Otherwise, the rcsta callback function will sample
// the rx pin (if we're receiving).

void   USART_MODULE14::new_rx_edge(unsigned int bit)
{
  if( (rcsta.state == _RCSTA::RCSTA_WAITING_FOR_START) && !bit)
    rcsta.receive_start_bit();
}
