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


//
// p16x6x
//
//  This file supports:
//    P16C61

#define PORT_EXPERIMENT true


#include <stdio.h>
#include <iostream>
#include <string>


#include "packages.h"
#include "stimuli.h"
#include "symbol.h"

#include "p16x6x.h"
#include "pic-ioports.h"
#include "intcon.h"

void P16C61::create(void)
{

  create_iopin_map();

  _14bit_processor::create();

  add_file_registers(0x0c, 0x2f, 0x80);
  Pic14Bit::create_sfr_map();

}

Processor * P16C61::construct(const char *name)
{

  P16C61 *p = new P16C61(name);

  if(verbose)
    cout << " c61 construct\n";

  p->create();
  p->create_invalid_registers ();
  p->create_symbols();

  symbol_table.add_module(p,p->name().c_str());

  return p;

}

P16C61::P16C61(const char *_name, const char *desc)
  
{
}


//------------------------------------------------------------------------
//
//

void P16C62::create_iopin_map(void)
{
  package = new Package(28);
  if(!package)
    return;

  package->assign_pin(1, 0);

  package->assign_pin( 2, m_porta->addPin(new IO_bi_directional("porta0"),0));
  package->assign_pin( 3, m_porta->addPin(new IO_bi_directional("porta1"),1));
  package->assign_pin( 4, m_porta->addPin(new IO_bi_directional("porta2"),2));
  package->assign_pin( 5, m_porta->addPin(new IO_bi_directional("porta3"),3));
  package->assign_pin( 6, m_porta->addPin(new IO_open_collector("porta4"),4));
  package->assign_pin( 7, m_porta->addPin(new IO_bi_directional("porta5"),5));


  package->assign_pin(8, 0); //VSS
  package->assign_pin(9, 0);  // OSC
  package->assign_pin(10, 0); // OSC

  package->assign_pin(11, m_portc->addPin(new IO_bi_directional("portc0"),0));
  package->assign_pin(12, m_portc->addPin(new IO_bi_directional("portc1"),1));
  package->assign_pin(13, m_portc->addPin(new IO_bi_directional("portc2"),2));
  package->assign_pin(14, m_portc->addPin(new IO_bi_directional("portc3"),3));
  package->assign_pin(15, m_portc->addPin(new IO_bi_directional("portc4"),4));
  package->assign_pin(16, m_portc->addPin(new IO_bi_directional("portc5"),5));
  package->assign_pin(17, m_portc->addPin(new IO_bi_directional("portc6"),6));
  package->assign_pin(18, m_portc->addPin(new IO_bi_directional("portc7"),7));

  package->assign_pin(19, 0); //VSS
  package->assign_pin(20, 0); //VDD
  package->assign_pin(21, m_portb->addPin(new IO_bi_directional_pu("portb0"),0));
  package->assign_pin(22, m_portb->addPin(new IO_bi_directional_pu("portb1"),1));
  package->assign_pin(23, m_portb->addPin(new IO_bi_directional_pu("portb2"),2));
  package->assign_pin(24, m_portb->addPin(new IO_bi_directional_pu("portb3"),3));
  package->assign_pin(25, m_portb->addPin(new IO_bi_directional_pu("portb4"),4));
  package->assign_pin(26, m_portb->addPin(new IO_bi_directional_pu("portb5"),5));
  package->assign_pin(27, m_portb->addPin(new IO_bi_directional_pu("portb6"),6));
  package->assign_pin(28, m_portb->addPin(new IO_bi_directional_pu("portb7"),7));

  if (hasSSP()) {
    ssp.initialize(
		get_pir_set(),    // PIR
	 	&(*m_portc)[3],   // SCK
		&(*m_porta)[5],   // SS
		&(*m_portc)[5],   // SDO
		&(*m_portc)[4],   // SDI
		m_trisc,         // I2C port
		SSP_TYPE_BSSP
  	);  
  }

  tmr1l.setIOpin(&(*m_portc)[0]);

}

//------------------------------------------------------------------------
//
//

void P16C64::create_iopin_map(void)
{

  package = new Package(40);
  if(!package)
    return;

  // Now Create the package and place the I/O pins

  package->assign_pin(1, 0);

  package->assign_pin( 2, m_porta->addPin(new IO_bi_directional("porta0"),0));
  package->assign_pin( 3, m_porta->addPin(new IO_bi_directional("porta1"),1));
  package->assign_pin( 4, m_porta->addPin(new IO_bi_directional("porta2"),2));
  package->assign_pin( 5, m_porta->addPin(new IO_bi_directional("porta3"),3));
  package->assign_pin( 6, m_porta->addPin(new IO_open_collector("porta4"),4));
  package->assign_pin( 7, m_porta->addPin(new IO_bi_directional("porta5"),5));

  package->assign_pin( 8, m_porte->addPin(new IO_bi_directional("porte0"),0));
  package->assign_pin( 9, m_porte->addPin(new IO_bi_directional("porte1"),1));
  package->assign_pin(10, m_porte->addPin(new IO_bi_directional("porte2"),2));

  package->assign_pin(11, 0);
  package->assign_pin(12, 0);
  package->assign_pin(13, 0);
  package->assign_pin(14, 0);

  package->assign_pin(15, m_portc->addPin(new IO_bi_directional("portc0"),0));
  package->assign_pin(16, m_portc->addPin(new IO_bi_directional("portc1"),1));
  package->assign_pin(17, m_portc->addPin(new IO_bi_directional("portc2"),2));
  package->assign_pin(18, m_portc->addPin(new IO_bi_directional("portc3"),3));
  package->assign_pin(23, m_portc->addPin(new IO_bi_directional("portc4"),4));
  package->assign_pin(24, m_portc->addPin(new IO_bi_directional("portc5"),5));
  package->assign_pin(25, m_portc->addPin(new IO_bi_directional("portc6"),6));
  package->assign_pin(26, m_portc->addPin(new IO_bi_directional("portc7"),7));


  package->assign_pin(19, m_portd->addPin(new IO_bi_directional("portd0"),0));
  package->assign_pin(20, m_portd->addPin(new IO_bi_directional("portd1"),1));
  package->assign_pin(21, m_portd->addPin(new IO_bi_directional("portd2"),2));
  package->assign_pin(22, m_portd->addPin(new IO_bi_directional("portd3"),3));
  package->assign_pin(27, m_portd->addPin(new IO_bi_directional("portd4"),4));
  package->assign_pin(28, m_portd->addPin(new IO_bi_directional("portd5"),5));
  package->assign_pin(29, m_portd->addPin(new IO_bi_directional("portd6"),6));
  package->assign_pin(30, m_portd->addPin(new IO_bi_directional("portd7"),7));

  package->assign_pin(31, 0);
  package->assign_pin(32, 0);

  package->assign_pin(33, m_portb->addPin(new IO_bi_directional_pu("portb0"),0));
  package->assign_pin(34, m_portb->addPin(new IO_bi_directional_pu("portb1"),1));
  package->assign_pin(35, m_portb->addPin(new IO_bi_directional_pu("portb2"),2));
  package->assign_pin(36, m_portb->addPin(new IO_bi_directional_pu("portb3"),3));
  package->assign_pin(37, m_portb->addPin(new IO_bi_directional_pu("portb4"),4));
  package->assign_pin(38, m_portb->addPin(new IO_bi_directional_pu("portb5"),5));
  package->assign_pin(39, m_portb->addPin(new IO_bi_directional_pu("portb6"),6));
  package->assign_pin(40, m_portb->addPin(new IO_bi_directional_pu("portb7"),7));

  if (hasSSP()) {
    ssp.initialize(
		get_pir_set(),    // PIR
		&(*m_portc)[3],   // SCK
		&(*m_porta)[5],   // SS
		&(*m_portc)[5],   // SDO
		&(*m_portc)[4],   // SDI
		m_trisc,         // I2C port
		SSP_TYPE_BSSP
		   ); 
  }
  psp.initialize(get_pir_set(),    // PIR
		m_portd,	   // Parallel port
		m_trisd,	   // Parallel tris
		m_trise,	   // Control tris
		&(*m_porte)[0],	   // NOT RD
		&(*m_porte)[1],	   // NOT WR
		&(*m_porte)[2]);   // NOT CS

  tmr1l.setIOpin(&(*m_portc)[0]);
}


//---------------------------------------------------------
//
//  P16x6x::create_sfr_map(void) - Here's where all of the 
//  registers are defined for a p16c63 and greater...

void P16X6X_processor::create_sfr_map()
{

  if(verbose)
    cout << "P16X6X_processor::create_sfr_map\n";

  Pic14Bit::create_sfr_map();

  // P16x63 and higher have porta5
  m_porta->setEnableMask(0x3f);
  m_porta->setTris(m_trisa);

  // The 16c62,c64 have general purpose registers
  // at addresses 20-7f and a0-bf
  add_file_registers(0x20, 0x7f, 0);
  add_file_registers(0xa0, 0xbf, 0);


  add_sfr_register(pir1,   0x0c, RegisterValue(0,0),"pir1");
  add_sfr_register(&pie1,   0x8c, RegisterValue(0,0));

  add_sfr_register(&tmr1l,  0x0e, RegisterValue(0,0),"tmr1l");
  add_sfr_register(&tmr1h,  0x0f, RegisterValue(0,0),"tmr1h");

  add_sfr_register(&pcon,   0x8e, RegisterValue(0,0),"pcon");

  add_sfr_register(&t1con,  0x10, RegisterValue(0,0));
  add_sfr_register(&tmr2,   0x11, RegisterValue(0,0));
  add_sfr_register(&t2con,  0x12, RegisterValue(0,0));
  add_sfr_register(&pr2,    0x92, RegisterValue(0xff,0));

  if( hasSSP() ) {
    add_sfr_register(&ssp.sspbuf,  0x13, RegisterValue(0,0),"sspbuf");
    add_sfr_register(&ssp.sspcon,  0x14, RegisterValue(0,0),"sspcon");
    add_sfr_register(&ssp.sspadd,  0x93, RegisterValue(0,0),"sspadd");
    add_sfr_register(&ssp.sspstat, 0x94, RegisterValue(0,0),"sspstat");
    tmr2.ssp_module = &ssp;
  }

  add_sfr_register(&ccpr1l,  0x15, RegisterValue(0,0));
  add_sfr_register(&ccpr1h,  0x16, RegisterValue(0,0));
  add_sfr_register(&ccp1con, 0x17, RegisterValue(0,0));

  // get_pir_set()->set_pir1(get_pir1());
  pir_set_def.set_pir1(pir1);

  intcon = &intcon_reg;
  intcon_reg.set_pir_set(get_pir_set());

  // Maybe there's a better place for this, but let's go ahead and link all
  // of the registers together (there's probably a better way too) :

  tmr1l.tmrh = &tmr1h;
  tmr1l.t1con = &t1con;
  tmr1l.setInterruptSource(new InterruptSource(pir1, PIR1v1::TMR1IF));
  tmr1l.ccpcon = &ccp1con;

  tmr1h.tmrl  = &tmr1l;

  t1con.tmrl  = &tmr1l;

  t2con.tmr2  = &tmr2;
  tmr2.pir_set   = get_pir_set();
  tmr2.pr2    = &pr2;
  tmr2.t2con  = &t2con;
  tmr2.ccp1con = &ccp1con;
  tmr2.ccp2con = &ccp2con;
  pr2.tmr2    = &tmr2;


  ccp1con.setCrosslinks(&ccpr1l, get_pir_set(), &tmr2);
  ccp1con.setIOpin(&((*m_portc)[2]));
  ccpr1l.ccprh  = &ccpr1h;
  ccpr1l.tmrl   = &tmr1l;
  ccpr1h.ccprl  = &ccpr1l;

  //  portc->ccp1con = &ccp1con;

  ccpr1l.new_name("ccpr1l");
  ccpr1h.new_name("ccpr1h");
  ccp1con.new_name("ccp1con");

  if (pir1) {
    pir1->set_intcon(&intcon_reg);
    pir1->set_pie(&pie1);
  }
  pie1.pir    = pir1;
  pie1.new_name("pie1");


}
//--------------------------------------------------

void P16X6X_processor::create_symbols()
{
  Pic14Bit::create_symbols();
}


//--------------------------------------------------

P16X6X_processor::P16X6X_processor(const char *_name, const char *_desc)
  : Pic14Bit(_name,_desc)
{
  

  if(verbose)
    cout << "generic 16X6X constructor, type = " << isa() << '\n';


  m_portc = new PicPortRegister("portc",8,0xff);
  m_trisc = new PicTrisRegister("trisc",m_portc);

  pir1 = new PIR1v1(&intcon_reg, &pie1);
  pir2 = new PIR2v1(&intcon_reg, &pie2);

}

P16X6X_processor::~P16X6X_processor()
{
  
}
/*******************************************************************
 *
 *        Definitions for the various P16x6x processors
 *
 */


P16C62::P16C62(const char *_name, const char *desc)
  : P16X6X_processor(_name,desc)
{
  if(verbose)
    cout << "c62 constructor, type = " << isa() << '\n';
  
}



void P16C62::create_sfr_map()
{
  if(verbose)
    cout << "creating c62 registers\n";

  P16X6X_processor::create_sfr_map();

  add_sfr_register(m_portc, 0x07);
  add_sfr_register(m_trisc, 0x87, RegisterValue(0xff,0));

  //1((PORTC*)portc)->ccp1con = &ccp1con;

}

void P16C62::create_symbols(void)
{

  if(verbose)
    cout << "creating c62 symbols\n";

  P16X6X_processor::create_symbols();
}


void  P16C62::create(void)
{

  if(verbose)
    cout << " c62 create \n";

  create_iopin_map();
  
  _14bit_processor::create();

  P16C62::create_sfr_map();

  // Build the links between the I/O Pins and the internal peripherals
  //1ccp1con.iopin = portc->pins[2];

}

Processor * P16C62::construct(const char *name)
{

  P16C62 *p = new P16C62(name);

  cout << " c62 construct\n";

  p->create();
  p->create_invalid_registers ();
  p->create_symbols();

  symbol_table.add_module(p,p->name().c_str());

  return p;

}

//------------------------------------------------------------------------
//
//

void P16C63::create_sfr_map(void)
{

  if(verbose)
    cout << "creating c63 registers\n";

  P16C62::create_sfr_map();

  add_file_registers(0xc0, 0xff, 0);

  add_sfr_register(pir2,    0x0d, RegisterValue(0,0),"pir2");
  add_sfr_register(&pie2,   0x8d, RegisterValue(0,0));

  add_sfr_register(&ccpr2l, 0x1b, RegisterValue(0,0));
  add_sfr_register(&ccpr2h, 0x1c, RegisterValue(0,0));
  add_sfr_register(&ccp2con, 0x1d, RegisterValue(0,0));

  // get_pir_set()->set_pir2(get_pir2());
  pir_set_def.set_pir2(pir2);

  ccp2con.setCrosslinks(&ccpr2l, get_pir_set(), &tmr2);
  ccp2con.setIOpin(&((*m_portc)[1]));
  ccpr2l.ccprh  = &ccpr2h;
  ccpr2l.tmrl   = &tmr1l;
  ccpr2h.ccprl  = &ccpr2l;

  usart.initialize(get_pir_set(),&(*m_portc)[6], &(*m_portc)[7],
		   new _TXREG(&usart), new _RCREG(&usart));

  add_sfr_register(&usart.rcsta, 0x18, RegisterValue(0,0),"rcsta");
  add_sfr_register(&usart.txsta, 0x98, RegisterValue(2,0),"txsta");
  add_sfr_register(&usart.spbrg, 0x99, RegisterValue(0,0),"spbrg");
  add_sfr_register(usart.txreg,  0x19, RegisterValue(0,0),"txreg");
  add_sfr_register(usart.rcreg,  0x1a, RegisterValue(0,0),"rcreg");

  ccpr2l.new_name("ccpr2l");
  ccpr2h.new_name("ccpr2h");
  ccp2con.new_name("ccp2con");

  if (pir2) {
    pir2->set_intcon(&intcon_reg);
    pir2->set_pie(&pie2);
  }

  pie2.pir    = get_pir2();
  pie2.new_name("pie2");


}

void P16C63::create_symbols(void)
{

  if(verbose)
    cout << "creating c63 symbols\n";

  // There's nothing to create...

}


//------------------------------------------------------------------------
//
// P16C63 constructor
// 
// Note: Since the 'C63 is derived from the 'C62. So before this constructor
// is called, the C62 constructor will be called. Most of the initialization
// is done within the 'C62 constructor.

P16C63::P16C63(const char *_name, const char *desc)
  : P16C62(_name,desc)
{

  if(verbose)
    cout << "c63 constructor, type = " << isa() << '\n';

}

void P16C63::create(void)
{
  if(verbose)
    cout << " c63 create \n";

  P16C62::create();

  P16C63::create_sfr_map();

  // Build the links between the I/O Pins and the internal peripherals
  //1ccp2con.iopin = portc->pins[1];

}

Processor * P16C63::construct(const char *name)
{

  P16C63 *p = new P16C63(name);

  if(verbose)
    cout << " c63 construct\n";

  p->create();
  p->create_invalid_registers ();

  p->create_symbols();

  symbol_table.add_module(p,p->name().c_str());

  return p;

}


//----------------------------------------------------------
//
//

void P16C64::create_sfr_map(void)
{
   if(verbose)
    cout << "creating c64 registers\n";

  pir_set_2_def.set_pir1(&pir1_2_reg);


  P16X6X_processor::create_sfr_map();

  add_sfr_register(m_portc, 0x07);
  add_sfr_register(m_trisc, 0x87, RegisterValue(0xff,0));

  add_sfr_register(m_portd, 0x08);
  add_sfr_register(m_trisd, 0x88, RegisterValue(0xff,0));

  add_sfr_register(m_porte, 0x09);
  add_sfr_register(m_trise, 0x89, RegisterValue(0x07,0));

  //1((PORTC*)portc)->ccp1con = &ccp1con;

}

void P16C64::create_symbols(void)
{

  if(verbose)
    cout << "creating c64 symbols\n";

  P16X6X_processor::create_symbols();

  symbol_table.add_register(m_portd);
  symbol_table.add_register(m_porte);

  symbol_table.add_register(m_trisd);
  symbol_table.add_register(m_trise);

}


void  P16C64::create(void)
{

  if(verbose)
    cout << " c64 create \n";

  create_iopin_map();
  
  _14bit_processor::create();

  //P16X6X_processor::create_sfr_map();
  P16C64::create_sfr_map();

  // Build the links between the I/O Pins and the internal peripherals
  //1ccp1con.iopin = portc->pins[2];

}

Processor * P16C64::construct(const char *name)
{

  P16C64 *p = new P16C64(name);

  p->create();
  p->create_invalid_registers ();
  p->create_symbols();

  symbol_table.add_module(p,p->name().c_str());

  return p;

}

P16C64::P16C64(const char *_name, const char *desc)
  : P16X6X_processor(_name,desc), 
  pir1_2_reg(&intcon_reg,&pie1)
{
  if(verbose)
    cout << "c64 constructor, type = " << isa() << '\n';

  pir1 = &pir1_2_reg;


  m_portd = new PicPSP_PortRegister("portd",8,0xff);
  m_trisd = new PicTrisRegister("trisd",(PicPortRegister *)m_portd);

  m_porte = new PicPortRegister("porte",8,0x07);
  m_trise =  new PicPSP_TrisRegister("trise",m_porte);

}
P16C64::~P16C64()
{
}
//------------------------------------------------------------------------
//
//

void P16C65::create_sfr_map(void)
{

  if(verbose)
    cout << "creating c65 registers\n";

  //P16C64::create_sfr_map();

  add_file_registers(0xc0, 0xff, 0);

  add_sfr_register(pir2,    0x0d, RegisterValue(0,0),"pir2");
  add_sfr_register(&pie2,   0x8d, RegisterValue(0,0));

  add_sfr_register(&ccpr2l, 0x1b, RegisterValue(0,0));
  add_sfr_register(&ccpr2h, 0x1c, RegisterValue(0,0));
  add_sfr_register(&ccp2con, 0x1d, RegisterValue(0,0));

  // get_pir_set()->set_pir2(&get_pir2());
  pir_set_def.set_pir2(pir2);


  ccp2con.setCrosslinks(&ccpr2l, get_pir_set(), &tmr2);
  ccp2con.setIOpin(&((*m_portc)[1]));

  ccpr2l.ccprh  = &ccpr2h;
  ccpr2l.tmrl   = &tmr1l;
  ccpr2h.ccprl  = &ccpr2l;

  usart.initialize(get_pir_set(),&(*m_portc)[6], &(*m_portc)[7],
		   new _TXREG(&usart), new _RCREG(&usart));

  add_sfr_register(&usart.rcsta, 0x18, RegisterValue(0,0),"rcsta");
  add_sfr_register(&usart.txsta, 0x98, RegisterValue(2,0),"txsta");
  add_sfr_register(&usart.spbrg, 0x99, RegisterValue(0,0),"spbrg");
  add_sfr_register(usart.txreg, 0x19, RegisterValue(0,0),"txreg");
  add_sfr_register(usart.rcreg, 0x1a, RegisterValue(0,0),"rcreg");

  ccpr2l.new_name("ccpr2l");
  ccpr2h.new_name("ccpr2h");
  ccp2con.new_name("ccp2con");

  if (pir2) {
    pir2->set_intcon(&intcon_reg);
    pir2->set_pie(&pie2);
  }

  pie2.pir    = get_pir2();
  pie2.new_name("pie2");

}

void P16C65::create_symbols(void)
{

  if(verbose)
    cout << "creating c65 symbols\n";

  // There's nothing to create...

}


//------------------------------------------------------------------------
//
// P16C65 constructor
// 
// Note: Since the 'C65 is derived from the 'C64. So before this constructor
// is called, the C64 constructor will be called. Most of the initialization
// is done within the 'C64 constructor.

P16C65::P16C65(const char *_name, const char *desc)
  : P16C64(_name,desc)
{

  if(verbose)
    cout << "c65 constructor, type = " << isa() << '\n';
}

void P16C65::create(void)
{
  if(verbose)
    cout << " c65 create \n";

  P16C64::create();

  P16C65::create_sfr_map();

  // Build the links between the I/O Pins and the internal peripherals
  // ccp1con.iopin = portc.pins[2];
  //1ccp2con.iopin = portc->pins[1];

}

Processor * P16C65::construct(const char *name)
{

  P16C65 *p = new P16C65(name);

  if(verbose)
    cout << " c65 construct\n";

  p->create();
  p->create_invalid_registers ();

  p->create_symbols();

  symbol_table.add_module(p,p->name().c_str());

  return p;

}

