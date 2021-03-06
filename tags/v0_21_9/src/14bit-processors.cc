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
#include <iostream>
#include <iomanip>

#include "../config.h"
//#include "14bit-registers.h"
//#include "14bit-instructions.h"
#include "14bit-processors.h"

#include <string>
#include "stimuli.h"


//-------------------------------------------------------------------
//
pic_processor * _14bit_processor::construct(void)
{

  cout << " Can't create a generic 14bit processor\n";

  return 0;

}
//-------------------------------------------------------------------
_14bit_processor::_14bit_processor(void)
{
  pc = new Program_Counter();
  pc->set_trace_command(trace.allocateTraceType(new PCTraceType(this,0,1)));

}
//-------------------------------------------------------------------
//
// 
//    create
//
//  The purpose of this member function is to 'create' those things
// that are unique to the 14-bit core processors.

void _14bit_processor :: create (void)
{

  if(verbose)
    cout << "_14bit_processor create, type = " << isa() << '\n';

  pic_processor::create();
  fsr = new FSR;
  fsr->new_name("fsr");

}



//-------------------------------------------------------------------
void _14bit_processor::interrupt (void)
{
  
  bp.clear_interrupt();

  stack->push(pc->value);
  intcon->clear_gie();

  pc->interrupt(INTERRUPT_VECTOR);

}

//-------------------------------------------------------------------
void _14bit_processor::por(void)
{
  pic_processor::por();
}

//-------------------------------------------------------------------
void _14bit_processor::option_new_bits_6_7(unsigned int bits)
{
  cout << "14bit, option bits 6 and/or 7 changed\n";
}

#if 0
//-------------------------------------------------------------------
class PortBSink;

class PortBIntEdgeSink :: public SignalSink
{
public:
  PortBIntEdgeSink(PortBSink *, unsigned int iobit);
  virtual void setSinkState(bool);
private:
  PortBSink    *m_PortBSink;
  unsigned int  m_bitMask;
};
class PortBSink
{
public:
  PortBSink(PortRegister *portReg);
  void setSink(unsigned int, bool);
  void setPullups(bool);
private:
  PortRegister *m_port;
  bool m_bPullupState;
};

//------------------------------------------------------------------------
PortBIntEdgeSink::PortBIntEdgeSink(PortBSink *_PortBSink, unsigned int iobit)
  : m_PortBSink(_PortBSink), m_bitMask(1<<iobit)
{
}

void PortBIntEdgeSink::setSinkState(bool bMewState)
{
  m_PortBSink->setSink(m_bitBask, bNewState);
}

//------------------------------------------------------------------------
PortBSink::PortBSink(PicPortRegister *portReg)
  : m_port(portReg),
    m_bPullupState(false)
{
  assert (portReg);

  portReg->addSink(new PortBIntEdgeSink(this, 0), 0);

  unsigned int mask = portReg->getEnableMask();
  for (unsigned int i=0, m=1; mask; i++, m<<= 1)
    if (mask & m) {
      mask ^= m;
      portReg->addSink(new PortBPinSink(this, i), i);
    }

}

void PortBSink::setPullups(bool new_pullupState)
{
  unsigned int mask = portReg->getEnableMask();
  for (unsigned int i=0, m=1; mask; i++, m<<= 1)
    if (mask & m) {
      mask ^= m;
      (*portReg)[i].update_pullup(new_pullupState);
    }
}
#endif

//-------------------------------------------------------------------
Pic14Bit::Pic14Bit()
{
  m_porta = new PicPortRegister("porta",8,0x1f);
  m_trisa = new PicTrisRegister(m_porta);
  m_trisa->new_name("trisa");

  tmr0.set_cpu(this, m_porta, 4);
  tmr0.start(0);


  m_portb = new PicPortBRegister("portb",8,0xff);
  m_trisb = new PicTrisRegister(m_portb);
  m_trisb->new_name("trisb");

}

//-------------------------------------------------------------------
void Pic14Bit::create_symbols(void)
{
  pic_processor::create_symbols();
  symbol_table.add_register(m_portb);
  symbol_table.add_register(m_porta);
  symbol_table.add_register(m_trisb);
  symbol_table.add_register(m_trisa);

}

//-------------------------------------------------------------------
void Pic14Bit::create_sfr_map(void)
{
 
  add_sfr_register(indf,    0x80);
  add_sfr_register(indf,    0x00);

  add_sfr_register(&tmr0,   0x01);
  add_sfr_register(&option_reg,  0x81, RegisterValue(0xff,0));

  add_sfr_register(pcl,     0x02, RegisterValue(0,0));
  add_sfr_register(status,  0x03, RegisterValue(0x18,0));
  add_sfr_register(fsr,     0x04);
  alias_file_registers(0x02,0x04,0x80);

  add_sfr_register(m_porta, 0x05);
  add_sfr_register(m_trisa, 0x85, RegisterValue(0x3f,0));

  add_sfr_register(m_portb, 0x06);
  add_sfr_register(m_trisb, 0x86, RegisterValue(0xff,0));

  add_sfr_register(pclath,  0x8a, RegisterValue(0,0));
  add_sfr_register(pclath,  0x0a, RegisterValue(0,0));

  add_sfr_register(&intcon_reg, 0x8b, RegisterValue(0,0));
  add_sfr_register(&intcon_reg, 0x0b, RegisterValue(0,0));

  intcon = &intcon_reg;


}
//-------------------------------------------------------------------
void Pic14Bit::option_new_bits_6_7(unsigned int bits)
{
  //1 ((PORTB *)portb)->rbpu_intedg_update(bits);
  m_portb->setRBPU( (bits & (1<<7)) == (1<<7));
  m_portb->setIntEdge((bits & (1<<6)) == (1<<6));
}
