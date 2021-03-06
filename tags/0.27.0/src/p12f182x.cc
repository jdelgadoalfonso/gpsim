/*
   Copyright (C) 2013 Roy R. Rankin

This file is part of the libgpsim library of gpsim

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, see 
<http://www.gnu.org/licenses/lgpl-2.1.html>.
*/


//
// p12f182x
//
//  This file supports:
//    PIC12F1822
//    PIC16F1823
//
//Note: unlike most other 12F processors these have extended 14bit instructions

#include <stdio.h>
#include <iostream>
#include <string>

#include "../config.h"
#include "symbol.h"

#include "stimuli.h"
#include "eeprom.h"

#include "p12f182x.h"

#include "pic-ioports.h"

#include "packages.h"


//#define DEBUG
#if defined(DEBUG)
#include "../config.h"
#define Dprintf(arg) {printf("%s:%d ",__FILE__,__LINE__); printf arg; }
#else
#define Dprintf(arg) {}
#endif

APFCON::APFCON(Processor *pCpu, const char *pName, const char *pDesc)
    : sfr_register(pCpu,pName,pDesc),
	m_usart(0), m_ssp(0), m_t1gcon(0)
{
      int j;
      mValidBits=0xef;
      for(j =0; j <8; j++)
      {
	m_bitPin[0][j] = NULL;
	m_bitPin[1][j] = NULL;
      }
}

void APFCON::put(unsigned int new_value)
{
    unsigned int old_value = value.get();
    unsigned int diff = (new_value ^ old_value) & mValidBits;

    trace.raw(write_trace.get() | value.get());
    new_value &= mValidBits;
    value.put(new_value);

    for(int i = 0; i < 8; i++)
    {
	unsigned int bit = 1<<i;
	if(diff & bit)
        {

	    if (m_bitPin[(new_value & bit)== bit][i] == 0)
	    {
		fprintf(stderr, "APFCON::put File bug report m_bitPin[%d][%d] not set\n", (new_value & bit)== bit, i);
		assert(m_bitPin[(new_value & bit)== bit][i]);
	    }

	    switch(i)
	    {
	    case 0:
		assert(m_ccpcon);
		m_ccpcon->setIOPin1(m_bitPin[(new_value & bit)== bit][i]);
		break;

	    case 1:
		assert(m_ccpcon);
		m_ccpcon->setIOPin2(m_bitPin[(new_value & bit)== bit][i]);
		break;

	    case 2:
		assert(m_usart);
		m_usart->set_TXpin(m_bitPin[(new_value & bit)== bit][i]);
		break;

	    case 3:
		assert(m_t1gcon);
		m_t1gcon->setGatepin(m_bitPin[(new_value & bit)== bit][i]);
		break;

	    case 4:	// not used
		break;

	    case 5:
		assert(m_ssp);
		m_ssp->set_ssPin(m_bitPin[(new_value & bit)== bit][i]);
		break;

	    case 6:
		assert(m_ssp);
		m_ssp->set_sdoPin(m_bitPin[(new_value & bit)== bit][i]);
		break;

	    case 7:
		assert(m_usart);
		m_usart->set_RXpin(m_bitPin[(new_value & bit)== bit][i]);
		break;
	    }
        }
    }
}


// Does not match any of 3 versions in pir.h, pir.cc
// If required by any other porcessors should be moved there
//
class PIR1v1822 : public PIR
{
public:

  enum {
    TMR1IF  = 1<<0,
    TMR2IF  = 1<<1,
    CCP1IF  = 1<<2,
    SSPIF   = 1<<3,
    TXIF    = 1<<4,
    RCIF    = 1<<5,
    ADIF    = 1<<6,
    TMR1GIF = 1<<7
  };

//------------------------------------------------------------------------

PIR1v1822(Processor *pCpu, const char *pName, const char *pDesc,INTCON *_intcon, PIE *_pie)
  : PIR(pCpu,pName,pDesc,_intcon, _pie,0)
{
  valid_bits = TMR1IF | TMR2IF | CCP1IF | SSPIF | TXIF | RCIF | ADIF | TMR1GIF;
  writable_bits = TMR1IF | TMR2IF | CCP1IF | SSPIF | ADIF | TMR1GIF;

}
  virtual void set_tmr1if()
  {
    trace.raw(write_trace.get() | value.get());
    value.put(value.get() | TMR1IF);
    if( value.get() & pie->value.get() )
      setPeripheralInterrupt();
  }
  virtual void set_tmr1gif()
  {
    trace.raw(write_trace.get() | value.get());
    value.put(value.get() | TMR1GIF);
    if( value.get() & pie->value.get() )
      setPeripheralInterrupt();
  }
  virtual void set_tmr2if()
  {
    trace.raw(write_trace.get() | value.get());
    value.put(value.get() | TMR2IF);
    if( value.get() & pie->value.get() )
      setPeripheralInterrupt();
  }

  void set_sspif(void)
  {
    trace.raw(write_trace.get() | value.get());
    value.put(value.get() | SSPIF);
    if( value.get() & pie->value.get() )
      setPeripheralInterrupt();
  }



};

class PIR2v1822 : public PIR
{
public:

  enum {
    BCLIF   = 1<<3,
    EEIF    = 1<<4,
    C1IF    = 1<<5,
    C2IF    = 1<<6, // not 12f1822
    OSFIF   = 1<<7
  };

//------------------------------------------------------------------------

PIR2v1822(Processor *pCpu, const char *pName, const char *pDesc,INTCON *_intcon, PIE *_pie)
  : PIR(pCpu,pName,pDesc,_intcon, _pie,0)
{
  valid_bits = BCLIF | EEIF | C1IF | OSFIF;
  writable_bits = BCLIF | EEIF | C1IF | OSFIF;

}

  void set_bclif(void)
  {
    trace.raw(write_trace.get() | value.get());
    value.put(value.get() | BCLIF);
    if( value.get() & pie->value.get() )
      setPeripheralInterrupt();
  }
  virtual void set_eeif()
  {
    trace.raw(write_trace.get() | value.get());
    value.put(value.get() | EEIF);
    if( value.get() & pie->value.get() )
      setPeripheralInterrupt();
  }
  void set_c1if(void)
  {
    trace.raw(write_trace.get() | value.get());
    value.put(value.get() | C1IF);
    if( value.get() & pie->value.get() )
      setPeripheralInterrupt();
  }
  void set_c2if(void)
  {
    trace.raw(write_trace.get() | value.get());
    value.put(value.get() | C2IF);
    if( value.get() & pie->value.get() )
      setPeripheralInterrupt();
  }
  void set_osfif(void)
  {
    trace.raw(write_trace.get() | value.get());
    value.put(value.get() | OSFIF);
    if( value.get() & pie->value.get() )
      setPeripheralInterrupt();
  }
};
//========================================================================


P12F1822::P12F1822(const char *_name, const char *desc)
  : _14bit_e_processor(_name,desc), 
    comparator(this),
    pie1(this,"PIE1", "Peripheral Interrupt Enable"),
    pie2(this,"PIE2", "Peripheral Interrupt Enable"),
    t2con(this, "t2con", "TMR2 Control"),
    pr2(this, "pr2", "TMR2 Period Register"),
    tmr2(this, "tmr2", "TMR2 Register"),
    t1con_g(this, "t1con", "TMR1 Control Register"),
    tmr1l(this, "tmr1l", "TMR1 Low"),
    tmr1h(this, "tmr1h", "TMR1 High"),
    ccp1con(this, "ccp1con", "Capture Compare Control"),
    ccpr1l(this, "ccpr1l", "Capture Compare 1 Low"),
    ccpr1h(this, "ccpr1h", "Capture Compare 1 High"),
    fvrcon(this, "fvrcon", "Voltage reference control register", 0xbf, 0x40),
    borcon(this, "borcon", "Brown-out reset control register"),
    ansela(this, "ansela", "Analog Select"),
    adcon0(this,"adcon0", "A2D Control 0"),
    adcon1(this,"adcon1", "A2D Control 1"),
    adresh(this,"adresh", "A2D Result High"),
    adresl(this,"adresl", "A2D Result Low"),
    osccon(this, "osccon", "Oscillator Control Register"),
    osctune(this, "osctune", "Oscillator Tunning Register"),
    oscstat(this, "oscstat", "Oscillator Status Register"),
    wdtcon(this, "wdtcon", "Watch dog timer control", 0x3f),
    usart(this),
    ssp(this),
    apfcon(this, "apfcon", "Alternate Pin Function Control Register"),
    pwm1con(this, "pwm1con", "Enhanced PWM Control Register"),
    ccp1as(this, "ccp1as", "CCP1 Auto-Shutdown Control Register"),
    pstr1con(this, "pstr1con", "Pulse Sterring Control Register"),
    cpscon0(this, "cpscon0", " Capacitive Sensing Control Register 0"),
    cpscon1(this, "cpscon1", " Capacitive Sensing Control Register 1"),
    sr_module(this)


{
  m_iocaf = new IOC(this, "iocaf", "Interrupt-On-Change flag Register");
  m_iocap = new IOC(this, "iocap", "Interrupt-On-Change positive edge");
  m_iocan = new IOC(this, "iocan", "Interrupt-On-Change negative edge");
  m_porta = new PicPortIOCRegister(this,"porta","", intcon, m_iocap, m_iocan, m_iocaf, 8,0x3f);
  m_trisa = new PicTrisRegister(this,"trisa","", m_porta, false, 0x37);
  m_lata  = new PicLatchRegister(this,"lata","",m_porta, 0x37);
  m_daccon0 = new DACCON0(this, "daccon0", "DAC Voltage reference register 0", 0xec, 32);
  m_daccon1 = new DACCON1(this, "daccon1", "DAC Voltage reference register 1", 0x1f, m_daccon0);
  m_cpu_temp = new CPU_Temp("cpu_temperature", 30., "CPU die temperature");

  tmr0.set_cpu(this, m_porta, 4, option_reg);
  tmr0.start(0);
  tmr0.set_t1gcon(&t1con_g.t1gcon);
  cpscon1.m_cpscon0 = &cpscon0;
  cpscon0.m_tmr0 = &tmr0;
  cpscon0.m_t1con_g = &t1con_g;



  m_wpua = new WPU(this, "wpua", "Weak Pull-up Register", m_porta, 0x37);

  pir1 = new PIR1v1822(this,"pir1","Peripheral Interrupt Register",intcon, &pie1);
  pir2 = new PIR2v1822(this,"pir2","Peripheral Interrupt Register",intcon, &pie2);

  comparator.cmxcon0[0] = new CMxCON0(this, "cm1con0", " Comparator C1 Control Register 0", 0, &comparator);
  comparator.cmxcon1[0] = new CMxCON1(this, "cm1con1", " Comparator C1 Control Register 1", 0, &comparator);
  comparator.cmout = new CMOUT(this, "cmout", "Comparator Output Register");
}

P12F1822::~P12F1822()
{
    unassignMCLRPin();
    delete_file_registers(0x20, 0x7f);
    delete_file_registers(0xa0, 0xbf);

    delete_sfr_register(m_iocap);
    delete_sfr_register(m_iocan);
    delete_sfr_register(m_iocaf);
    delete_sfr_register(m_daccon0);
    delete_sfr_register(m_daccon1);
    delete_sfr_register(m_trisa);
    delete_sfr_register(m_porta);
    delete_sfr_register(m_lata);

    delete_sfr_register(m_wpua);
    remove_sfr_register(&tmr0);

    remove_sfr_register(&tmr1l);
    remove_sfr_register(&tmr1h);
    remove_sfr_register(&t1con_g);
    remove_sfr_register(&t1con_g.t1gcon);

    remove_sfr_register(&tmr2);
    remove_sfr_register(&pr2);
    remove_sfr_register(&t2con);
    remove_sfr_register(&cpscon0);
    remove_sfr_register(&cpscon1);
    remove_sfr_register(&ssp.sspbuf);
    remove_sfr_register(&ssp.sspadd);
    remove_sfr_register(&ssp.ssp1msk);
    remove_sfr_register(&ssp.sspstat);
    remove_sfr_register(&ssp.sspcon);
    remove_sfr_register(&ssp.sspcon2);
    remove_sfr_register(&ssp.ssp1con3);
    remove_sfr_register(&ccpr1l);
    remove_sfr_register(&ccpr1h);
    remove_sfr_register(&ccp1con);
    remove_sfr_register(&pwm1con);
    remove_sfr_register(&ccp1as);
    remove_sfr_register(&pstr1con);
    remove_sfr_register(&pie1);
    remove_sfr_register(&pie2);
    remove_sfr_register(&adresl);
    remove_sfr_register(&adresh);
    remove_sfr_register(&adcon0);
    remove_sfr_register(&adcon1);
    remove_sfr_register(&borcon);
    remove_sfr_register(&fvrcon);
    remove_sfr_register(&sr_module.srcon0);
    remove_sfr_register(&sr_module.srcon1);
    remove_sfr_register(&apfcon );
    remove_sfr_register(&ansela);
    remove_sfr_register(get_eeprom()->get_reg_eeadr());
    remove_sfr_register(get_eeprom()->get_reg_eeadrh());
    remove_sfr_register(get_eeprom()->get_reg_eedata());
    remove_sfr_register(get_eeprom()->get_reg_eedatah());
    remove_sfr_register(get_eeprom()->get_reg_eecon1());
    remove_sfr_register(get_eeprom()->get_reg_eecon2());
    remove_sfr_register(&usart.spbrg);
    remove_sfr_register(&usart.spbrgh);
    remove_sfr_register(&usart.rcsta);
    remove_sfr_register(&usart.txsta);
    remove_sfr_register(&usart.baudcon);
    remove_sfr_register(&ssp.sspbuf);
    remove_sfr_register(&ssp.sspadd);
    remove_sfr_register(&ssp.ssp1msk);
    remove_sfr_register(&ssp.sspstat);
    remove_sfr_register(&ssp.sspcon);
    remove_sfr_register(&ssp.sspcon2);
    remove_sfr_register(&ssp.ssp1con3);
    remove_sfr_register(&ccpr1l);
    remove_sfr_register(&ccpr1h);
    remove_sfr_register(&ccp1con);
    remove_sfr_register(&pwm1con);
    remove_sfr_register(&ccp1as);
    remove_sfr_register(&pstr1con);
    remove_sfr_register(&osctune);
    remove_sfr_register(&osccon);
    remove_sfr_register(&oscstat);

    remove_sfr_register(comparator.cmxcon0[0]); 
    remove_sfr_register(comparator.cmxcon1[0]); 
    remove_sfr_register(comparator.cmout); 
    delete_sfr_register(usart.rcreg);
    delete_sfr_register(usart.txreg);
    delete_sfr_register(pir1);
    delete_sfr_register(pir2);
    delete e;
    delete m_cpu_temp;
}
Processor * P12F1822::construct(const char *name)
{

  P12F1822 *p = new P12F1822(name);

  p->create(0x7f, 256);
  p->create_invalid_registers ();
  p->create_symbols();
  return p;

}

void P12F1822::create_symbols()
{
  pic_processor::create_symbols();
  addSymbol(Wreg);
  addSymbol(m_cpu_temp);


}
void P12F1822::create_sfr_map()
{

  pir_set_2_def.set_pir1(pir1);
  pir_set_2_def.set_pir2(pir2);


  //add_sfr_register(indf,    0x00);
  add_file_registers(0xa0, 0xbf, 0x00);
  add_sfr_register(m_porta, 0x0c);
  add_sfr_register(pir1,    0x11, RegisterValue(0,0),"pir1");
  add_sfr_register(pir2,    0x12, RegisterValue(0,0),"pir2");
  add_sfr_register(&tmr0,   0x15);

  add_sfr_register(&tmr1l,  0x16, RegisterValue(0,0),"tmr1l");
  add_sfr_register(&tmr1h,  0x17, RegisterValue(0,0),"tmr1h");
  add_sfr_register(&t1con_g,  0x18, RegisterValue(0,0));
  add_sfr_register(&t1con_g.t1gcon, 0x19, RegisterValue(0,0));

  add_sfr_register(&tmr2,   0x1a, RegisterValue(0,0));
  add_sfr_register(&pr2,    0x1b, RegisterValue(0,0));
  add_sfr_register(&t2con,  0x1c, RegisterValue(0,0));
  add_sfr_register(&cpscon0,  0x1e, RegisterValue(0,0));
  add_sfr_register(&cpscon1,  0x1f, RegisterValue(0,0));


  add_sfr_register(m_trisa, 0x8c, RegisterValue(0x3f,0));

  pcon.valid_bits = 0xcf;
  add_sfr_register(option_reg,  0x95, RegisterValue(0xff,0));
  add_sfr_register(&osctune,    0x98, RegisterValue(0,0));
  add_sfr_register(&osccon,     0x99, RegisterValue(0x38,0));
  add_sfr_register(&oscstat,    0x9a, RegisterValue(0,0));

  intcon_reg.set_pir_set(get_pir_set());


  tmr1l.tmrh = &tmr1h;
  tmr1l.t1con = &t1con_g;
  tmr1l.setInterruptSource(new InterruptSource(pir1, PIR1v1::TMR1IF));
  
  tmr1h.tmrl  = &tmr1l;
  t1con_g.tmrl  = &tmr1l;
  t1con_g.t1gcon.set_tmrl(&tmr1l);
  t1con_g.t1gcon.set_pir_set(get_pir_set());

  tmr1l.setIOpin(&(*m_porta)[5]);
  t1con_g.t1gcon.setGatepin(&(*m_porta)[3]);

  add_sfr_register(&pie1,   0x91, RegisterValue(0,0));
  add_sfr_register(&pie2,   0x92, RegisterValue(0,0));
  add_sfr_register(&adresl, 0x9b);
  add_sfr_register(&adresh, 0x9c);
  add_sfr_register(&adcon0, 0x9d, RegisterValue(0x00,0));
  add_sfr_register(&adcon1, 0x9e, RegisterValue(0x00,0));


  usart.initialize(pir1,
	&(*m_porta)[0], // TX pin
	&(*m_porta)[1], // RX pin
	new _TXREG(this,"txreg", "USART Transmit Register", &usart), 
        new _RCREG(this,"rcreg", "USART Receiver Register", &usart));

  usart.set_eusart(true);

  add_sfr_register(m_lata,    0x10c);
  add_sfr_register(comparator.cmxcon0[0], 0x111, RegisterValue(0x04,0)); 
  add_sfr_register(comparator.cmxcon1[0], 0x112, RegisterValue(0x00,0)); 
  add_sfr_register(comparator.cmout,      0x115, RegisterValue(0x00,0)); 
  add_sfr_register(&borcon,   0x116, RegisterValue(0x80,0));
  add_sfr_register(&fvrcon,   0x117, RegisterValue(0x00,0));
  add_sfr_register(m_daccon0, 0x118, RegisterValue(0x00,0));
  add_sfr_register(m_daccon1, 0x119, RegisterValue(0x00,0));
  add_sfr_register(&sr_module.srcon0, 0x11a, RegisterValue(0x00,0));
  add_sfr_register(&sr_module.srcon1, 0x11b, RegisterValue(0x00,0));
  add_sfr_register(&apfcon ,  0x11d, RegisterValue(0x00,0));
  add_sfr_register(&ansela,   0x18c, RegisterValue(0x17,0));
  add_sfr_register(get_eeprom()->get_reg_eeadr(),   0x191);
  add_sfr_register(get_eeprom()->get_reg_eeadrh(),   0x192);
  add_sfr_register(get_eeprom()->get_reg_eedata(),  0x193);
  add_sfr_register(get_eeprom()->get_reg_eedatah(),  0x194);
  add_sfr_register(get_eeprom()->get_reg_eecon1(),  0x195, RegisterValue(0,0));
  add_sfr_register(get_eeprom()->get_reg_eecon2(),  0x196);
  add_sfr_register(usart.rcreg,    0x199, RegisterValue(0,0),"rcreg");
  add_sfr_register(usart.txreg,    0x19a, RegisterValue(0,0),"txreg");
  add_sfr_register(&usart.spbrg,   0x19b, RegisterValue(0,0),"spbrgl");
  add_sfr_register(&usart.spbrgh,  0x19c, RegisterValue(0,0),"spbrgh");
  add_sfr_register(&usart.rcsta,   0x19d, RegisterValue(0,0),"rcsta");
  add_sfr_register(&usart.txsta,   0x19e, RegisterValue(2,0),"txsta");
  add_sfr_register(&usart.baudcon, 0x19f,RegisterValue(0x40,0),"baudcon");

  add_sfr_register(m_wpua,       0x20c, RegisterValue(0x3f,0),"wpua");
  add_sfr_register(&ssp.sspbuf,  0x211, RegisterValue(0,0),"ssp1buf");
  add_sfr_register(&ssp.sspadd,  0x212, RegisterValue(0,0),"ssp1add");
  add_sfr_register(&ssp.ssp1msk, 0x213, RegisterValue(0xff,0),"ssp1msk");
  add_sfr_register(&ssp.sspstat, 0x214, RegisterValue(0,0),"ssp1stat");
  add_sfr_register(&ssp.sspcon,  0x215, RegisterValue(0,0),"ssp1con");
  add_sfr_register(&ssp.sspcon2, 0x216, RegisterValue(0,0),"ssp1con2");
  add_sfr_register(&ssp.ssp1con3, 0x217, RegisterValue(0,0),"ssp1con3");
  add_sfr_register(&ccpr1l,      0x291, RegisterValue(0,0));
  add_sfr_register(&ccpr1h,      0x292, RegisterValue(0,0));
  add_sfr_register(&ccp1con,     0x293, RegisterValue(0,0));
  add_sfr_register(&pwm1con,     0x294, RegisterValue(0,0));
  add_sfr_register(&ccp1as,      0x295, RegisterValue(0,0));
  add_sfr_register(&pstr1con,    0x296, RegisterValue(1,0));

  add_sfr_register(m_iocap, 0x391, RegisterValue(0,0),"iocap");
  add_sfr_register(m_iocan, 0x392, RegisterValue(0,0),"iocan");
  add_sfr_register(m_iocaf, 0x393, RegisterValue(0,0),"iocaf");


  tmr2.ssp_module = &ssp;

    ssp.initialize(
	get_pir_set(),    // PIR
        &(*m_porta)[1],   // SCK
        &(*m_porta)[3],   // SS
        &(*m_porta)[0],   // SDO
        &(*m_porta)[2],    // SDI
          m_trisa,          // i2c tris port
	SSP_TYPE_MSSP1
    );
    apfcon.set_usart(&usart);
    apfcon.set_ssp(&ssp);
    apfcon.set_t1gcon(&t1con_g.t1gcon);
    apfcon.set_pins(0, &(*m_porta)[2], &(*m_porta)[5]); //CCP1/P1A
    apfcon.set_pins(1, &(*m_porta)[0], &(*m_porta)[4]); //P1B
    apfcon.set_pins(2, &(*m_porta)[0], &(*m_porta)[4]); //USART TX Pin
    apfcon.set_pins(3, &(*m_porta)[4], &(*m_porta)[3]); //tmr1 gate
    apfcon.set_pins(5, &(*m_porta)[3], &(*m_porta)[0]); //SSP SS
    apfcon.set_pins(6, &(*m_porta)[0], &(*m_porta)[4]); //SSP SDO
    apfcon.set_pins(7, &(*m_porta)[1], &(*m_porta)[5]); //USART RX Pin
    if (pir1) {
    	pir1->set_intcon(intcon);
    	pir1->set_pie(&pie1);
    }
    pie1.setPir(pir1);
    pie2.setPir(pir2);
    t2con.tmr2 = &tmr2;
    tmr2.pir_set   = get_pir_set();
    tmr2.pr2    = &pr2;
    tmr2.t2con  = &t2con;
    tmr2.add_ccp ( &ccp1con );
//  tmr2.add_ccp ( &ccp2con );
    pr2.tmr2    = &tmr2;

    ccp1as.setIOpin(0, 0, &(*m_porta)[2]);
    ccp1as.link_registers(&pwm1con, &ccp1con);

    ccp1con.setIOpin(&(*m_porta)[2], &(*m_porta)[0]);
    ccp1con.pstrcon = &pstr1con;
    ccp1con.pwm1con = &pwm1con;
    ccp1con.setCrosslinks(&ccpr1l, pir1, PIR1v1822::CCP1IF, &tmr2, &ccp1as);
    ccpr1l.ccprh  = &ccpr1h;
    ccpr1l.tmrl   = &tmr1l;
    ccpr1h.ccprl  = &ccpr1l;


    ansela.config(0x17, 0);
    ansela.setValidBits(0x17);
    ansela.setAdcon1(&adcon1);

    adcon0.setAdresLow(&adresl);
    adcon0.setAdres(&adresh);
    adcon0.setAdcon1(&adcon1);
    adcon0.setIntcon(intcon);
    adcon0.setA2DBits(10);
    adcon0.setPir(pir1);
    adcon0.setChannel_Mask(0x1f);
    adcon0.setChannel_shift(2);
    adcon0.setGo(1);

    adcon1.setAdcon0(&adcon0); 
    adcon1.setNumberOfChannels(32); // not all channels are used
    adcon1.setIOPin(0, &(*m_porta)[0]);
    adcon1.setIOPin(1, &(*m_porta)[1]);
    adcon1.setIOPin(2, &(*m_porta)[2]);
    adcon1.setIOPin(3, &(*m_porta)[4]);
    adcon1.setValidBits(0xf3);
    adcon1.setVrefHiConfiguration(0, 1);
    adcon1.set_FVR_chan(0x1f);

    comparator.cmxcon1[0]->set_OUTpin(&(*m_porta)[2]);
    comparator.cmxcon1[0]->set_INpinNeg(&(*m_porta)[1], &(*m_porta)[4]);
    comparator.cmxcon1[0]->set_INpinPos(&(*m_porta)[0]);
    comparator.cmxcon0[0]->setBitMask(0xf7);
    comparator.cmxcon1[0]->setBitMask(0xf1);
    comparator.assign_pir_set(get_pir_set());
    comparator.assign_t1gcon(&t1con_g.t1gcon);
    comparator.assign_sr_module(&sr_module);
    fvrcon.set_adcon1(&adcon1);
    fvrcon.set_cpscon0(&cpscon0);
    fvrcon.set_daccon0(m_daccon0);
    fvrcon.set_cmModule(&comparator);
    fvrcon.set_VTemp_AD_chan(0x1d);
    fvrcon.set_FVRAD_AD_chan(0x1f);

    m_daccon0->set_adcon1(&adcon1);
    m_daccon0->set_cpscon0(&cpscon0);
    m_daccon0->set_cmModule(&comparator);
    m_daccon0->set_FVRCDA_AD_chan(0x1e);
    m_daccon0->setDACOUT(&(*m_porta)[0]);

    cpscon0.set_pin(0, &(*m_porta)[0]);
    cpscon0.set_pin(1, &(*m_porta)[1]);
    cpscon0.set_pin(2, &(*m_porta)[2]);
    cpscon0.set_pin(3, &(*m_porta)[4]);


    sr_module.setPins(&(*m_porta)[1], &(*m_porta)[2], &(*m_porta)[5]);

    osccon.set_osctune(&osctune);
    osccon.set_oscstat(&oscstat);
    osctune.set_osccon((OSCCON *)&osccon);
}

//-------------------------------------------------------------------
void P12F1822::set_out_of_range_pm(unsigned int address, unsigned int value)
{

  if( (address>= 0x2100) && (address < 0x2100 + get_eeprom()->get_rom_size()))
      get_eeprom()->change_rom(address - 0x2100, value);
}

void P12F1822::create_iopin_map()
{

  package = new Package(8);
  if(!package)
    return;

  // Now Create the package and place the I/O pins
  package->assign_pin(7, m_porta->addPin(new IO_bi_directional_pu("porta0"),0));
  package->assign_pin(6, m_porta->addPin(new IO_bi_directional_pu("porta1"),1));
  package->assign_pin(5, m_porta->addPin(new IO_bi_directional_pu("porta2"),2));
  package->assign_pin(4, m_porta->addPin(new IO_bi_directional_pu("porta3"),3));
  package->assign_pin(3, m_porta->addPin(new IO_bi_directional_pu("porta4"),4));
  package->assign_pin(2, m_porta->addPin(new IO_bi_directional_pu("porta5"),5));

  package->assign_pin( 1, 0);	// Vdd
  package->assign_pin( 8, 0);	// Vss


}




void  P12F1822::create(int ram_top, int eeprom_size)
{

  create_iopin_map();

  e = new EEPROM_EXTND(this, pir2);
  set_eeprom(e);

  pic_processor::create();


  e->initialize(eeprom_size, 16, 16, 0x8000);
  e->set_intcon(intcon);
  e->get_reg_eecon1()->set_valid_bits(0xff);

  add_file_registers(0x20, ram_top, 0x00);
  _14bit_e_processor::create_sfr_map();
  P12F1822::create_sfr_map();
  // Set DeviceID
  if (m_configMemory && m_configMemory->getConfigWord(6))
      m_configMemory->getConfigWord(6)->set(0x2700);
}

//-------------------------------------------------------------------
void P12F1822::enter_sleep()
{
    tmr0.sleep();
    tmr1l.sleep();
    pic_processor::enter_sleep();
}

//-------------------------------------------------------------------
void P12F1822::exit_sleep()
{
    if (m_ActivityState == ePASleeping)
    {
	tmr0.wake();
	tmr1l.wake();
        osccon.wake();
	pic_processor::exit_sleep();
    }
}

//-------------------------------------------------------------------
void P12F1822::option_new_bits_6_7(unsigned int bits)
{
	Dprintf(("P12F1822::option_new_bits_6_7 bits=%x\n", bits));
    m_porta->setIntEdge ( (bits & OPTION_REG::BIT6) == OPTION_REG::BIT6); 
    m_wpua->set_wpu_pu ( (bits & OPTION_REG::BIT7) != OPTION_REG::BIT7); 
}

void P12F1822::oscillator_select(unsigned int cfg_word1, bool clkout)
{
    unsigned int mask = 0x1f;

    osccon.set_config(cfg_word1 & (FOSC0|FOSC1|FOSC2), cfg_word1 & IESO);
    set_int_osc(false);
    switch(cfg_word1 & (FOSC0|FOSC1|FOSC2))
    {
    case 0:	//LP oscillator: low power crystal
    case 1:	//XT oscillator: Crystal/resonator 
    case 2:	//HS oscillator: High-speed crystal/resonator
        (m_porta->getPin(4))->newGUIname("OSC2");
        (m_porta->getPin(5))->newGUIname("OSC1");
	mask = 0x0f;

	break;

    case 3:	//EXTRC oscillator External RC circuit connected to CLKIN pin
        (m_porta->getPin(5))->newGUIname("CLKIN");
	mask = 0x1f;
	if(clkout) 
	{
	    (m_porta->getPin(4))->newGUIname("CLKOUT");
	    mask = 0x0f;
        }
	break;

    case 4:	//INTOSC oscillator: I/O function on CLKIN pin
        set_int_osc(true);
	mask = 0x3f;
	if(clkout) 
	{
	    (m_porta->getPin(4))->newGUIname("CLKOUT");
	    mask = 0x2f;
        }
        (m_porta->getPin(5))->newGUIname((m_porta->getPin(5))->name().c_str());
	break;

    case 5:	//ECL: External Clock, Low-Power mode (0-0.5 MHz): on CLKIN pin
	mask = 0x1f;
	if(clkout) 
	{
	    (m_porta->getPin(4))->newGUIname("CLKOUT");
	    mask = 0x0f;
        }
        (m_porta->getPin(5))->newGUIname("CLKIN");
	break;

    case 6:	//ECM: External Clock, Medium-Power mode (0.5-4 MHz): on CLKIN pin
	mask = 0x1f;
	if(clkout) 
	{
	    (m_porta->getPin(4))->newGUIname("CLKOUT");
	    mask = 0x0f;
        }
        (m_porta->getPin(5))->newGUIname("CLKIN");
	break;

    case 7:	//ECH: External Clock, High-Power mode (4-32 MHz): on CLKIN pin
	mask = 0x1f;
	if(clkout) 
	{
	    (m_porta->getPin(4))->newGUIname("CLKOUT");
	    mask = 0x0f;
        }
        (m_porta->getPin(5))->newGUIname("CLKIN");
	break;
    };
    ansela.setValidBits(0x17 & mask);
    m_porta->setEnableMask(mask);
}
void P12F1822::program_memory_wp(unsigned int mode)
{
	switch(mode)
	{
	case 3:	// no write protect
	    get_eeprom()->set_prog_wp(0x0);
	    break;

	case 2: // write protect 0000-01ff
	    get_eeprom()->set_prog_wp(0x0200);
	    break;

	case 1: // write protect 0000-03ff
	    get_eeprom()->set_prog_wp(0x0400);
	    break;

	case 0: // write protect 0000-07ff
	    get_eeprom()->set_prog_wp(0x0800);
	    break;

	default:
	    printf("%s unexpected mode %d\n", __FUNCTION__, mode);
	    break;
	}

}
//========================================================================


P16F1823::P16F1823(const char *_name, const char *desc)
  : P12F1822(_name,desc), 
    anselc(this, "anselc", "Analog Select port c")
{
   
  m_portc = new PicPortBRegister(this,"portc","", intcon, 8,0x3f);
  m_trisc = new PicTrisRegister(this,"trisc","", m_portc, false, 0x3f);
  m_latc  = new PicLatchRegister(this,"latc","",m_portc, 0x3f);
  m_wpuc = new WPU(this, "wpuc", "Weak Pull-up Register", m_portc, 0x3f);
  comparator.cmxcon0[1] = new CMxCON0(this, "cm2con0", " Comparator C2 Control Register 0", 1, &comparator);
  comparator.cmxcon1[1] = new CMxCON1(this, "cm2con1", " Comparator C2 Control Register 1", 1, &comparator);
  cpscon1.mValidBits = 0x0f;
}
P16F1823::~P16F1823()
{
    delete_sfr_register(m_portc);
    delete_sfr_register(m_trisc);
    delete_sfr_register(m_latc);
    remove_sfr_register(comparator.cmxcon0[1]); 
    remove_sfr_register(comparator.cmxcon1[1]); 
    delete_sfr_register(m_wpuc);
    remove_sfr_register(&anselc);
}
void P16F1823::create_iopin_map()
{

  package = new Package(14);
  if(!package)
    return;

  // Now Create the package and place the I/O pins
  package->assign_pin(13, m_porta->addPin(new IO_bi_directional_pu("porta0"),0));
  package->assign_pin(12, m_porta->addPin(new IO_bi_directional_pu("porta1"),1));
  package->assign_pin(11, m_porta->addPin(new IO_bi_directional_pu("porta2"),2));
  package->assign_pin(4, m_porta->addPin(new IO_bi_directional_pu("porta3"),3));
  package->assign_pin(3, m_porta->addPin(new IO_bi_directional_pu("porta4"),4));
  package->assign_pin(2, m_porta->addPin(new IO_bi_directional_pu("porta5"),5));

  package->assign_pin(10, m_portc->addPin(new IO_bi_directional_pu("portc0"),0));
  package->assign_pin(9, m_portc->addPin(new IO_bi_directional_pu("portc1"),1));
  package->assign_pin(8, m_portc->addPin(new IO_bi_directional_pu("portc2"),2));
  package->assign_pin(7, m_portc->addPin(new IO_bi_directional_pu("portc3"),3));
  package->assign_pin(6, m_portc->addPin(new IO_bi_directional_pu("portc4"),4));
  package->assign_pin(5, m_portc->addPin(new IO_bi_directional_pu("portc5"),5));

  package->assign_pin( 1, 0);	// Vdd
  package->assign_pin( 14, 0);	// Vss


}


Processor * P16F1823::construct(const char *name)
{

  P16F1823 *p = new P16F1823(name);

  p->create(0x7f, 256);
  p->create_invalid_registers ();
  p->create_symbols();
  return p;

}


void  P16F1823::create(int ram_top, int eeprom_size)
{

  create_iopin_map();
  e = new EEPROM_EXTND(this, pir2);
  set_eeprom(e);

  pic_processor::create();


  e->initialize(eeprom_size, 16, 16, 0x8000);
  e->set_intcon(intcon);
  e->get_reg_eecon1()->set_valid_bits(0xff);

  add_file_registers(0x20, ram_top, 0x00);
  _14bit_e_processor::create_sfr_map();
  P12F1822::create_sfr_map();
  P16F1823::create_sfr_map();
  // Set DeviceID
  if (m_configMemory && m_configMemory->getConfigWord(6))
      m_configMemory->getConfigWord(6)->set(0x2720);

}
void P16F1823::create_sfr_map()
{
    add_sfr_register(m_portc, 0x0e);
    add_sfr_register(m_trisc, 0x8e, RegisterValue(0x3f,0));
    add_sfr_register(m_latc, 0x10e);
    add_sfr_register(comparator.cmxcon0[1], 0x113, RegisterValue(0x04,0)); 
    add_sfr_register(comparator.cmxcon1[1], 0x114, RegisterValue(0x00,0)); 
    add_sfr_register(&anselc, 0x18e, RegisterValue(0x0f,0));
    add_sfr_register(m_wpuc, 0x20e, RegisterValue(0x3f,0),"wpuc");

    anselc.config(0x0f, 4);
    anselc.setValidBits(0x0f);
    anselc.setAdcon1(&adcon1);
    ansela.setAnsel(&anselc);
    anselc.setAnsel(&ansela);
    adcon1.setIOPin(4, &(*m_portc)[0]);
    adcon1.setIOPin(5, &(*m_portc)[1]);
    adcon1.setIOPin(6, &(*m_portc)[2]);
    adcon1.setIOPin(7, &(*m_portc)[3]);

    ssp.set_sckPin(&(*m_portc)[0]);
    ssp.set_sdiPin(&(*m_portc)[1]);
    ssp.set_sdoPin(&(*m_portc)[2]);
    ssp.set_ssPin(&(*m_portc)[3]);
    ssp.set_tris(m_trisc);

    // Pin values for default APFCON
    usart.set_TXpin(&(*m_portc)[4]); // TX pin
    usart.set_RXpin(&(*m_portc)[5]);  // RX pin

    ccp1con.setIOpin(&(*m_portc)[5], &(*m_portc)[4], &(*m_portc)[3], &(*m_portc)[2]);
    apfcon.set_ValidBits(0xec);
    // pins 0,1 not used for p16f1823
    apfcon.set_pins(2, &(*m_portc)[4], &(*m_porta)[0]); //USART TX Pin
    // pin 3 defined in p12f1822
    apfcon.set_pins(5, &(*m_portc)[3], &(*m_porta)[3]); //SSP SS
    apfcon.set_pins(6, &(*m_portc)[2], &(*m_porta)[4]); //SSP SDO
    apfcon.set_pins(7, &(*m_portc)[5], &(*m_porta)[1]); //USART RX Pin
    comparator.cmxcon1[0]->set_INpinNeg(&(*m_porta)[1], &(*m_portc)[1],  &(*m_portc)[2],  &(*m_portc)[3]);
    comparator.cmxcon1[1]->set_INpinNeg(&(*m_porta)[1], &(*m_portc)[1],  &(*m_portc)[2],  &(*m_portc)[3]);
    comparator.cmxcon1[1]->set_INpinPos(&(*m_portc)[0]);
    comparator.cmxcon1[0]->set_OUTpin(&(*m_porta)[2]);
    comparator.cmxcon1[1]->set_OUTpin(&(*m_portc)[4]);
    comparator.cmxcon0[0]->setBitMask(0xf7);
    comparator.cmxcon0[1]->setBitMask(0xf7);
    comparator.cmxcon1[0]->setBitMask(0xf3);
    comparator.cmxcon1[1]->setBitMask(0xf3);


    cpscon0.set_pin(4, &(*m_portc)[0]);
    cpscon0.set_pin(5, &(*m_portc)[1]);
    cpscon0.set_pin(6, &(*m_portc)[2]);
    cpscon0.set_pin(7, &(*m_portc)[3]);
    sr_module.srcon1.set_ValidBits(0xff);
    sr_module.setPins(&(*m_porta)[1], &(*m_porta)[2], &(*m_portc)[4]);
}
