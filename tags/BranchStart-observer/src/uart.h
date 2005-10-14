/*
   Copyright (C) 1998,1999 T. Scott Dattalo

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

#include <iostream.h>
#include <stdio.h>


class invalid_file_register;   // Forward reference


#ifndef __USART_H__
#define __USART_H__

#include "pic-processor.h"
#include "14bit-registers.h"

class _TXSTA;   // Forward references
class _SPBRG;
class _RCSTA;
class PIR1;
class  _14bit_processor;
class IOPORT;


#define TOTAL_BRGH_STATES      16
#define TOTAL_BRGL_STATES      64


class _TXREG : public sfr_register
{
 public:

  _TXSTA  *txsta;

  virtual void put(unsigned int);
  virtual void put_value(unsigned int);
  virtual bool is_empty(void)=0;
  virtual void empty(void)=0;
  virtual void full(void)=0;

};

class _TXSTA : public sfr_register, public BreakCallBack
{
public:
  _TXREG  *txreg;
  _SPBRG  *spbrg;
  IOPIN   *txpin;

  unsigned int tsr;
  unsigned int bit_count;

  enum {
    TX9D = 1<<0,
    TRMT = 1<<1,
    BRGH = 1<<2,
    SYNC = 1<<4,
    TXEN = 1<<5,
    TX9  = 1<<6,
    CSRC = 1<<7
  };

  virtual void put(unsigned int new_value);
  virtual void put_value(unsigned int new_value);

  virtual void transmit_a_bit(void);
  virtual void start_transmitting(void);
  void callback(void);

};

class _RCREG : public sfr_register
{
 public:

  _RCSTA  *rcsta;


  unsigned int oldest_value;  /* rcreg has a 2-deep fifo. The oldest received
			       * value is stored here, while the most recent
			       * is stored in sfr_register.value . */

  unsigned int fifo_sp;       /* fifo stack pointer */

  virtual unsigned int get(void);
  virtual unsigned int get_value(void);
  virtual void push(unsigned int);
  virtual void pop(void);


};

class _RCSTA : public sfr_register, public BreakCallBack
{

 public:
  enum {
    RX9D = 1<<0,
    OERR = 1<<1,
    FERR = 1<<2,
    ADDEN = 1<<3,
    CREN = 1<<4,
    SREN = 1<<5,
    RX9  = 1<<6,
    SPEN = 1<<7
  };

  enum {
    RCSTA_DISABLED,
    RCSTA_WAITING_FOR_START,
    RCSTA_WAITING_MID1,
    RCSTA_WAITING_MID2,
    RCSTA_WAITING_MID3,
    RCSTA_RECEIVING
  };

  // The usart samples the middle of the bit three times and
  // produces a sample based on majority averaging. 
  // 

#define BRGH_FIRST_MID_SAMPLE  7
#define BRGH_SECOND_MID_SAMPLE 8
#define BRGH_THIRD_MID_SAMPLE  9

#define BRGL_FIRST_MID_SAMPLE  30
#define BRGL_SECOND_MID_SAMPLE 32
#define BRGL_THIRD_MID_SAMPLE  34

  _RCREG  *rcreg;
  _SPBRG  *spbrg;
  _TXSTA  *txsta;
  IOPORT  *uart_port;
  

  unsigned int rsr;
  unsigned int bit_count;
  unsigned int rx_bit;
  unsigned int sample,state;
  guint64 future_cycle, last_cycle;

  //  unsigned int new_bit;    //TEST!!!!


  virtual void put(unsigned int new_value);
  virtual void put_value(unsigned int new_value);
  void receive_a_bit(unsigned);
  void receive_start_bit(void);
  virtual void start_receiving(void);
  void set_callback_break(unsigned int spbrg_edge);
  void callback(void);

};
class _SPBRG : public sfr_register, public BreakCallBack
{
 public:
  _TXSTA *txsta;
  _RCSTA *rcsta;

  guint64 
    start_cycle,   // The cycle the SPBRG was started
    last_cycle,    // The cycle when the spbrg clock last changed
    future_cycle;  // The next cycle spbrg is predicted to change

  virtual void callback(void);
  void start(void);
  void get_next_cycle_break(void);
  guint64 get_cpu_cycle(unsigned int edges_from_now);
};

//---------------------------------------------------------------
class TXREG_14 : public _TXREG
{
 public:
  PIR1 *pir1;

  virtual bool is_empty(void);
  virtual void empty(void);
  virtual void full(void);

};

class RCREG_14 : public _RCREG
{
 public:
  PIR1 *pir1;

  virtual void push(unsigned int);
  virtual void pop(void);

};

//---------------------------------------------------------------
class USART_MODULE
{
public:

  //  pic_processor *cpu;

  _TXSTA       txsta;
  _RCSTA       rcsta;
  _SPBRG       spbrg;

  //  USART_MODULE(void);

  virtual  void  new_rx_edge(unsigned int)=0;
};

class USART_MODULE14 : public USART_MODULE
{
 public:

  _14bit_processor *cpu;

  TXREG_14     txreg;
  RCREG_14     rcreg;

  virtual void initialize(_14bit_processor *new_cpu,PIR1 *pir1, IOPORT *uart_port);
  virtual void new_rx_edge(unsigned int);

};

#endif