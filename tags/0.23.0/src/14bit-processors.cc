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
#include "14bit-processors.h"
#include "pic-ioports.h"
#include "pic-registers.h"
#include <string>
#include "stimuli.h"
#include "packages.h"

//========================================================================
// Generic Configuration word for the midrange family.

class Generic14bitConfigWord : public ConfigWord
{
public:
  Generic14bitConfigWord(_14bit_processor *pCpu)
    : ConfigWord("CONFIG", 0x3fff, "Configuration Word", pCpu, 0x2007)
  {
    assert(pCpu);
    pCpu->wdt.initialize(true);
  }

  enum {
    FOSC0  = 1<<0,
    FOSC1  = 1<<1,
    WDTEN  = 1<<2,
    PWRTEN = 1<<3
  };

  virtual void set(gint64 v)
  {
    gint64 oldV = getVal();

    Integer::set(v);
    if (m_pCpu) {

      gint64 diff = oldV ^ v;

      if (diff & WDTEN)
        m_pCpu->wdt.initialize((v & WDTEN) == WDTEN);

      m_pCpu->config_modes->set_fosc01(v & (FOSC0 | FOSC1));
      m_pCpu->config_modes->set_wdte((v&WDTEN)==WDTEN);
      m_pCpu->config_modes->set_pwrte((v&PWRTEN)==PWRTEN);

    }

  }

  virtual string toString()
  {
    gint64 i64;
    get(i64);
    int i = i64 &0xfff;

    char buff[256];

    snprintf(buff,sizeof(buff),
             "$%3x\n"
             " FOSC=%d - Clk source = %s\n"
             " WDTEN=%d - WDT is %s\n"
             " PWRTEN=%d - Power up timer is %s\n",
             i,
             i&(FOSC0|FOSC1), (i&FOSC0 ? (i&FOSC1 ? "EXTRC":"XT"):(i&FOSC1 ? "INTRC":"LP")),
             (i&WDTEN?1:0), ((i&WDTEN) ? "enabled" : "disabled"),
             (i&PWRTEN?1:0), ((i&PWRTEN) ? "disabled" : "enabled"));

    return string(buff);
  }

};


//-------------------------------------------------------------------
_14bit_processor::_14bit_processor(const char *_name, const char *_desc)
  : pic_processor(_name,_desc), intcon(0)
{
  pc = new Program_Counter("pc", "Program Counter", this);
  pc->set_trace_command(); //trace.allocateTraceType(new PCTraceType(this,1)));
  option_reg = new OPTION_REG(this,"option_reg");
  stack = new Stack();
}

_14bit_processor::~_14bit_processor()
{
  delete_sfr_register(fsr);
  delete_sfr_register(option_reg);
  delete pc; pc=0;
}

//-------------------------------------------------------------------
//
//
//    create
//
//  The purpose of this member function is to 'create' those things
// that are unique to the 14-bit core processors.

void _14bit_processor :: create ()
{

  if(verbose)
    cout << "_14bit_processor create, type = " << isa() << '\n';

  pic_processor::create();
  fsr = new FSR(this, "fsr", "File Select Register for indirect addressing");
}



//-------------------------------------------------------------------
void _14bit_processor::interrupt ()
{

  bp.clear_interrupt();

  stack->push(pc->value);
  intcon->clear_gie();

  pc->interrupt(INTERRUPT_VECTOR);

}

//-------------------------------------------------------------------
void _14bit_processor::save_state()
{
  pic_processor::save_state();

  option_reg->put_trace_state(option_reg->value);
}

//-------------------------------------------------------------------
void _14bit_processor::option_new_bits_6_7(unsigned int bits)
{
  cout << "14bit, option bits 6 and/or 7 changed\n";
}
//-------------------------------------------------------------------
void _14bit_processor::put_option_reg(unsigned int val)
{
  option_reg->put(val);
}


//------------------------------------------------------------------
// Fetch the rom contents at a particular address.
unsigned int _14bit_processor::get_program_memory_at_address(unsigned int address)
{
  unsigned int uIndex = map_pm_address2index(address);


  if (uIndex < program_memory_size())
    return  program_memory[uIndex] ? program_memory[uIndex]->get_opcode() : 0xffffffff;

  return get_config_word(address);
}

//-------------------------------------------------------------------
void _14bit_processor::create_config_memory()
{
  m_configMemory = new ConfigMemory(this,1);
  m_configMemory->addConfigWord(0,new Generic14bitConfigWord(this));
}

//-------------------------------------------------------------------

bool _14bit_processor::set_config_word(unsigned int address,unsigned int cfg_word)
{

  if((address == config_word_address()) && config_modes) {

    config_word = cfg_word;

    if (m_configMemory && m_configMemory->getConfigWord(0))
      m_configMemory->getConfigWord(0)->set((int)cfg_word);

    return true;
  }

  return false;

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
Pic14Bit::Pic14Bit(const char *_name, const char *_desc)
  : _14bit_processor(_name,_desc),
    intcon_reg(this,"intcon","Interrupt Control"),
    m_MCLR(0), m_MCLRMonitor(0)
{
  m_porta = new PicPortRegister(this,"porta","", 8,0x1f);
  m_trisa = new PicTrisRegister(this,"trisa","", m_porta, false);

  tmr0.set_cpu(this, m_porta, 4, option_reg);
  tmr0.start(0);

  m_portb = new PicPortBRegister(this,"portb","",&intcon_reg,8,0xff);
  m_trisb = new PicTrisRegister(this,"trisb","", m_portb, false);
}

//-------------------------------------------------------------------
Pic14Bit::~Pic14Bit()
{
  //delete m_MCLR; <-- this is a package pin
  delete m_MCLRMonitor;

  delete_sfr_register(m_portb);
  delete_sfr_register(m_trisb);

  delete_sfr_register(m_porta);
  delete_sfr_register(m_trisa);
}
//-------------------------------------------------------------------
void Pic14Bit::create_symbols()
{
  pic_processor::create_symbols();

  addSymbol(W);

}

//-------------------------------------------------------------------
void Pic14Bit::create_sfr_map()
{

  add_sfr_register(indf,    0x00);
  alias_file_registers(0x00,0x00,0x80);
  //add_sfr_register(indf,    0x00);

  add_sfr_register(&tmr0,   0x01);
  add_sfr_register(option_reg,  0x81, RegisterValue(0xff,0));

  add_sfr_register(pcl,     0x02, RegisterValue(0,0));
  add_sfr_register(status,  0x03, RegisterValue(0x18,0));
  add_sfr_register(fsr,     0x04);
  alias_file_registers(0x02,0x04,0x80);

  add_sfr_register(m_porta, 0x05);
  add_sfr_register(m_trisa, 0x85, RegisterValue(0x3f,0));

  add_sfr_register(m_portb, 0x06);
  add_sfr_register(m_trisb, 0x86, RegisterValue(0xff,0));

  add_sfr_register(pclath,  0x0a, RegisterValue(0,0));
  //add_sfr_register(pclath,  0x8a, RegisterValue(0,0));

  add_sfr_register(&intcon_reg, 0x0b, RegisterValue(0,0));
  //add_sfr_register(&intcon_reg, 0x8b, RegisterValue(0,0));
  alias_file_registers(0x0a,0x0b,0x80);

  intcon = &intcon_reg;


}
//-------------------------------------------------------------------
void Pic14Bit::option_new_bits_6_7(unsigned int bits)
{
  //1 ((PORTB *)portb)->rbpu_intedg_update(bits);
  m_portb->setRBPU( (bits & (1<<7)) == (1<<7));
  m_portb->setIntEdge((bits & (1<<6)) == (1<<6));
}

//-------------------------------------------------------------------
class MCLRPinMonitor : public PinMonitor
{
public:
  MCLRPinMonitor(pic_processor *pCpu);
  ~MCLRPinMonitor() {}

  virtual void setDrivenState(char);
  virtual void setDrivingState(char) {}
  virtual void set_nodeVoltage(double) {}
  virtual void putState(char) {}
  virtual void setDirection() {}
private:
  pic_processor *m_pCpu;
  char m_cLastResetState;
};

MCLRPinMonitor::MCLRPinMonitor(pic_processor *pCpu)
  : m_pCpu(pCpu),
    m_cLastResetState('I')  // I is not a valid state. It's used here for 'I'nitialization
{
}


void MCLRPinMonitor::setDrivenState(char newState)
{
  if (newState =='0' || newState =='w') {
    m_cLastResetState = '0';
    m_pCpu->reset(MCLR_RESET);
  }

  if (newState =='1' || newState =='W') {
    if (m_cLastResetState == '0')
      m_pCpu->reset(EXIT_RESET);

    m_cLastResetState = 1;
  }

}
//-------------------------------------------------------------------
void Pic14Bit::createMCLRPin(int pkgPinNumber)
{
  if (m_MCLR) {
    cout << "BUG?: assigning multiple MCLR pins: " << __FILE__ << __LINE__ << endl;
  }
  if(package) {
    m_MCLR = new IO_open_collector("MCLR");
    package->assign_pin(pkgPinNumber,m_MCLR);

    m_MCLRMonitor = new MCLRPinMonitor(this);
    m_MCLR->setMonitor(m_MCLRMonitor);
  }
}
