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


#include <stdio.h>
#include <iostream>
#include <string>


#include "packages.h"
#include "stimuli.h"
#include "symbol.h"

#include "p16x6x.h"
#include "intcon.h"


void P16C61::create_sfr_map(void)
{
  if(verbose)
    cout << "creating c61 registers\n";

  add_file_registers(0x0c, 0x2f, 0x80);

 
  add_sfr_register(indf,   0x80);
  add_sfr_register(indf,   0x00);

  add_sfr_register(&tmr0,  0x01);
  add_sfr_register(&option_reg,  0x81, RegisterValue(0xff,0));

  add_sfr_register(pcl,    0x02, RegisterValue(0,0));
  add_sfr_register(status, 0x03, RegisterValue(0x18,0));
  add_sfr_register(fsr,     0x04);
  alias_file_registers(0x02,0x04,0x80);

  add_sfr_register(porta,   0x05);
  add_sfr_register(&trisa,  0x85, RegisterValue(0x3f,0));

  add_sfr_register(portb,   0x06);
  add_sfr_register(&trisb,  0x86, RegisterValue(0xff,0));

  add_sfr_register(pclath, 0x8a, RegisterValue(0,0));
  add_sfr_register(pclath, 0x0a, RegisterValue(0,0));

  add_sfr_register(&intcon_reg, 0x8b, RegisterValue(0,0));
  add_sfr_register(&intcon_reg, 0x0b, RegisterValue(0,0));

  intcon = &intcon_reg;

}


void P16C61::create_symbols(void)
{

  symbol_table.add_ioport(portb);
  symbol_table.add_ioport(porta);


}

void P16C61::create(void)
{

  create_iopin_map();

  _14bit_processor::create();

  P16C61::create_sfr_map();

}

Processor * P16C61::construct(void)
{

  P16C61 *p = new P16C61;

  if(verbose)
    cout << " c61 construct\n";

  p->create();
  p->create_invalid_registers ();
  p->pic_processor::create_symbols();

  p->new_name("p16c61");
  symbol_table.add_module(p,p->name().c_str());

  return p;

}

P16C61::P16C61(void)
{
  if(verbose)
    cout << "c61 constructor, type = " << isa() << '\n';

}


//------------------------------------------------------------------------
//
//

void P16C62::create_iopin_map(void)
{
  package = new Package(28);
  if(!package)
    return;


  porta = new PORTA;
  portb = new PORTB;
  portc = new PORTC;

  // ---- Complete the initialization for the I/O Ports

  // Build the links between the I/O Ports and their tris registers.
  porta->tris = &trisa;
  trisa.port = porta;

  portb->tris = &trisb;
  trisb.port = portb;

  portc->tris = &trisc;
  trisc.port = portc;


  // And give them a more meaningful name.
  trisa.new_name("trisa");
  trisb.new_name("trisb");
  trisc.new_name("trisc");

  // Define the valid I/O pins.
  porta->valid_iopins = 0x3f;
  portb->valid_iopins = 0xff;
  portc->valid_iopins = 0xff;


  // Now Create the package and place the I/O pins


  package->assign_pin(1, 0);

  package->assign_pin(2, new IO_bi_directional(porta, 0));
  package->assign_pin(3, new IO_bi_directional(porta, 1));
  package->assign_pin(4, new IO_bi_directional(porta, 2));
  package->assign_pin(5, new IO_bi_directional(porta, 3));
  package->assign_pin(6, new IO_open_collector(porta, 4));
  package->assign_pin(7, new IO_bi_directional(porta, 5));

  package->assign_pin(8, 0); //VSS
  package->assign_pin(9, 0);  // OSC
  package->assign_pin(10, 0); // OSC

  package->assign_pin(11, new IO_bi_directional(portc, 0));
  package->assign_pin(12, new IO_bi_directional(portc, 1));
  package->assign_pin(13, new IO_bi_directional(portc, 2));
  package->assign_pin(14, new IO_bi_directional(portc, 3));
  package->assign_pin(15, new IO_bi_directional(portc, 4));
  package->assign_pin(16, new IO_bi_directional(portc, 5));
  package->assign_pin(17, new IO_bi_directional(portc, 6));
  package->assign_pin(18, new IO_bi_directional(portc, 7));


  package->assign_pin(19, 0); //VSS
  package->assign_pin(20, 0); //VDD

  package->assign_pin(21, new IO_bi_directional_pu(portb, 0));
  package->assign_pin(22, new IO_bi_directional_pu(portb, 1));
  package->assign_pin(23, new IO_bi_directional_pu(portb, 2));
  package->assign_pin(24, new IO_bi_directional_pu(portb, 3));
  package->assign_pin(25, new IO_bi_directional_pu(portb, 4));
  package->assign_pin(26, new IO_bi_directional_pu(portb, 5));
  package->assign_pin(27, new IO_bi_directional_pu(portb, 6));
  package->assign_pin(28, new IO_bi_directional_pu(portb, 7));

}

//------------------------------------------------------------------------
//
//

void P16C64::create_iopin_map(void)
{

  package = new Package(40);
  if(!package)
    return;

  porta = new PORTA;
  portb = new PORTB;
  portc = new PORTC;
  portd = new PORTD;
  porte = new PORTE;

  // ---- Complete the initialization for the I/O Ports

  // Build the links between the I/O Ports and their tris registers.
  porta->tris = &trisa;
  trisa.port = porta;

  portb->tris = &trisb;
  trisb.port = portb;

  portc->tris = &trisc;
  trisc.port = portc;

  portd->tris = &trisd;
  trisd.port = portd;

  porte->tris = &trise;
  trise.port = porte;


  // And give them a more meaningful name.
  trisa.new_name("trisa");
  trisb.new_name("trisb");
  trisc.new_name("trisc");
  trisd.new_name("trisd");
  trise.new_name("trise");

  // Define the valid I/O pins.
  porta->valid_iopins = 0x3f;
  portb->valid_iopins = 0xff;
  portc->valid_iopins = 0xff;
  portd->valid_iopins = 0xff;
  porte->valid_iopins = 0x07;


  // Now Create the package and place the I/O pins

  package->assign_pin(1, 0);

  package->assign_pin(2, new IO_bi_directional(porta, 0));
  package->assign_pin(3, new IO_bi_directional(porta, 1));
  package->assign_pin(4, new IO_bi_directional(porta, 2));
  package->assign_pin(5, new IO_bi_directional(porta, 3));
  package->assign_pin(6, new IO_open_collector(porta, 4));
  package->assign_pin(7, new IO_bi_directional(porta, 5));

  package->assign_pin(8, new IO_bi_directional(porte, 0));
  package->assign_pin(9, new IO_bi_directional(porte, 1));
  package->assign_pin(10, new IO_bi_directional(porte, 2));


  package->assign_pin(11, 0);
  package->assign_pin(12, 0);
  package->assign_pin(13, 0);
  package->assign_pin(14, 0);

  package->assign_pin(15, new IO_bi_directional(portc, 0));
  package->assign_pin(16, new IO_bi_directional(portc, 1));
  package->assign_pin(17, new IO_bi_directional(portc, 2));
  package->assign_pin(18, new IO_bi_directional(portc, 3));
  package->assign_pin(23, new IO_bi_directional(portc, 4));
  package->assign_pin(24, new IO_bi_directional(portc, 5));
  package->assign_pin(25, new IO_bi_directional(portc, 6));
  package->assign_pin(26, new IO_bi_directional(portc, 7));


  package->assign_pin(19, new IO_bi_directional(portd, 0));
  package->assign_pin(20, new IO_bi_directional(portd, 1));
  package->assign_pin(21, new IO_bi_directional(portd, 2));
  package->assign_pin(22, new IO_bi_directional(portd, 3));
  package->assign_pin(27, new IO_bi_directional(portd, 4));
  package->assign_pin(28, new IO_bi_directional(portd, 5));
  package->assign_pin(29, new IO_bi_directional(portd, 6));
  package->assign_pin(30, new IO_bi_directional(portd, 7));

  package->assign_pin(31, 0);
  package->assign_pin(32, 0);

  package->assign_pin(33, new IO_bi_directional_pu(portb, 0));
  package->assign_pin(34, new IO_bi_directional_pu(portb, 1));
  package->assign_pin(35, new IO_bi_directional_pu(portb, 2));
  package->assign_pin(36, new IO_bi_directional_pu(portb, 3));
  package->assign_pin(37, new IO_bi_directional_pu(portb, 4));
  package->assign_pin(38, new IO_bi_directional_pu(portb, 5));
  package->assign_pin(39, new IO_bi_directional_pu(portb, 6));
  package->assign_pin(40, new IO_bi_directional_pu(portb, 7));

}


//---------------------------------------------------------
//
//  P16x6x::create_sfr_map(void) - Here's where all of the 
//  registers are defined for a p16c63 and greater...

void P16X6X_processor::create_sfr_map(void)
{

  if(verbose)
    cout << "P16X6X_processor::create_sfr_map\n";

  // The 16c62,c64 have general purpose registers
  // at addresses 20-7f and a0-bf
  add_file_registers(0x20, 0x7f, 0);
  add_file_registers(0xa0, 0xbf, 0);

 
  add_sfr_register(indf,   0x80);
  add_sfr_register(indf,   0x00);

  add_sfr_register(&tmr0,  0x01);
  add_sfr_register(&option_reg,  0x81, RegisterValue(0xff,0));

  add_sfr_register(pcl,    0x02, RegisterValue(0,0));
  add_sfr_register(status, 0x03, RegisterValue(0x18,0));
  add_sfr_register(fsr,     0x04);
  alias_file_registers(0x02,0x04,0x80);

  add_sfr_register(pclath, 0x8a, RegisterValue(0,0));
  add_sfr_register(pclath, 0x0a, RegisterValue(0,0));

  add_sfr_register(&intcon_reg, 0x8b, RegisterValue(0,0));
  add_sfr_register(&intcon_reg, 0x0b, RegisterValue(0,0));

  add_sfr_register(get_pir1(),   0x0c, RegisterValue(0,0));
  add_sfr_register(&pie1,   0x8c, RegisterValue(0,0));

  add_sfr_register(&tmr1l,  0x0e, RegisterValue(0,0));
  add_sfr_register(&tmr1h,  0x0f, RegisterValue(0,0));

  add_sfr_register(&pcon,   0x8e, RegisterValue(0,0));

  add_sfr_register(&t1con,  0x10, RegisterValue(0,0));
  add_sfr_register(&tmr2,   0x11, RegisterValue(0,0));
  add_sfr_register(&t2con,  0x12, RegisterValue(0,0));
  add_sfr_register(&pr2,    0x92, RegisterValue(0xff,0));

  if( init_ssp ) {
	add_sfr_register(ssp.sspbuf, 0x13, RegisterValue(0,0),"sspbuf");
	add_sfr_register(ssp.sspcon, 0x14, RegisterValue(0,0),"sspcon");
	add_sfr_register(ssp.sspadd, 0x93, RegisterValue(0,0),"sspadd");
	add_sfr_register(ssp.sspstat, 0x94, RegisterValue(0,0),"sspstat");
  }

  add_sfr_register(&ccpr1l,  0x15, RegisterValue(0,0));
  add_sfr_register(&ccpr1h,  0x16, RegisterValue(0,0));
  add_sfr_register(&ccp1con, 0x17, RegisterValue(0,0));

  // get_pir_set()->set_pir1(get_pir1());
  pir_set_def.set_pir1(&pir1_reg);

  intcon = &intcon_reg;
  intcon_reg.set_pir_set(get_pir_set());

  // Maybe there's a better place for this, but let's go ahead and link all
  // of the registers together (there's probably a better way too) :

  tmr1l.tmrh = &tmr1h;
  tmr1l.t1con = &t1con;
  tmr1l.pir_set  = get_pir_set();
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


  ccp1con.ccprl = &ccpr1l;
  ccp1con.pir_set   = get_pir_set();
  ccp1con.tmr2  = &tmr2;
  ccpr1l.ccprh  = &ccpr1h;
  ccpr1l.tmrl   = &tmr1l;
  ccpr1h.ccprl  = &ccpr1l;

  //  portc->ccp1con = &ccp1con;

  ccpr1l.new_name("ccpr1l");
  ccpr1h.new_name("ccpr1h");
  ccp1con.new_name("ccp1con");

  get_pir1()->intcon = &intcon_reg;
  get_pir1()->pie    = &pie1;
  pie1.pir    = get_pir1();
  pie1.new_name("pie1");


}

//--------------------------------------------------

P16X6X_processor::P16X6X_processor(void)
{
  

  if(verbose)
    cout << "generic 16X6X constructor, type = " << isa() << '\n';

  init_ssp = false;

}

/*******************************************************************
 *
 *        Definitions for the various P16x6x processors (finally)
 *
 */




void P16C62::create_sfr_map(void)
{
   if(verbose)
    cout << "creating c62 registers\n";


  add_sfr_register(porta,   0x05);
  add_sfr_register(&trisa,  0x85, RegisterValue(0x3f,0));

  add_sfr_register(portb,   0x06);
  add_sfr_register(&trisb,  0x86, RegisterValue(0xff,0));

  add_sfr_register(portc,   0x07);
  add_sfr_register(&trisc,  0x87, RegisterValue(0xff,0));

  ((PORTC*)portc)->ccp1con = &ccp1con;

}

void P16C62::create_symbols(void)
{

  if(verbose)
    cout << "creating c62 symbols\n";

  symbol_table.add_ioport(porta);
  symbol_table.add_ioport(portb);
  symbol_table.add_ioport(portc);

}


void  P16C62::create(void)
{

  if(verbose)
    cout << " c62 create \n";

  create_iopin_map();
  
  _14bit_processor::create();

  P16X6X_processor::create_sfr_map();
  P16C62::create_sfr_map();

  // Build the links between the I/O Pins and the internal peripherals
  ccp1con.iopin = portc->pins[2];

}

Processor * P16C62::construct(void)
{

  P16C62 *p = new P16C62;

  cout << " c62 construct\n";

  p->create();
  p->create_invalid_registers ();
  p->pic_processor::create_symbols();

  p->ssp.initialize_14(p,p->get_pir_set(),p->portc,3,4,5,p->porta,5,SSP_TYPE_BSSP);

  p->new_name("p16c62");
  symbol_table.add_module(p,p->name().c_str());

  return p;

}

P16C62::P16C62(void)
{
  if(verbose)
    cout << "c62 constructor, type = " << isa() << '\n';

  init_ssp = true;
  
}

//------------------------------------------------------------------------
//
//

void P16C63::create_sfr_map(void)
{

  if(verbose)
    cout << "creating c63 registers\n";

  add_file_registers(0xc0, 0xff, 0);

  add_sfr_register(get_pir2(),   0x0d, RegisterValue(0,0));
  add_sfr_register(&pie2,   0x8d, RegisterValue(0,0));

  add_sfr_register(&ccpr2l, 0x1b, RegisterValue(0,0));
  add_sfr_register(&ccpr2h, 0x1c, RegisterValue(0,0));
  add_sfr_register(&ccp2con, 0x1d, RegisterValue(0,0));

  // get_pir_set()->set_pir2(get_pir2());
  pir_set_def.set_pir2(&pir2_reg);

  ccp2con.ccprl = &ccpr2l;
  ccp2con.pir_set   = get_pir_set();
  ccp2con.tmr2  = &tmr2;
  ccpr2l.ccprh  = &ccpr2h;
  ccpr2l.tmrl   = &tmr1l;
  ccpr2h.ccprl  = &ccpr2l;

  add_sfr_register(usart.rcsta, 0x18, RegisterValue(0,0),"rcsta");
  add_sfr_register(usart.txsta, 0x98, RegisterValue(2,0),"txsta");
  add_sfr_register(usart.spbrg, 0x99, RegisterValue(0,0),"spbrg");
  add_sfr_register(usart.txreg, 0x19, RegisterValue(0,0),"txreg");
  add_sfr_register(usart.rcreg, 0x1a, RegisterValue(0,0),"rcreg");
  usart.initialize_14(this,get_pir_set(),portc,7);

  //  cout << "portc HACK \n";
  int i;
  for(i=0; i<8; i++) {
    if(portc->pins[i]) {
      portc->pins[i]->iopp=&(this->registers[7]);
    }
  }

  /*
  add_sfr_register(ssp.sspbuf, 0x13, RegisterValue(0,0),"sspbuf");
  add_sfr_register(ssp.sspcon, 0x14, RegisterValue(0,0),"sspcon");
  add_sfr_register(ssp.sspadd, 0x93, RegisterValue(0,0),"sspadd");
  add_sfr_register(ssp.sspstat, 0x94, RegisterValue(0,0),"sspstat");
  ssp.initialize_14(this,get_pir_set(),portc,3,4,5,porta,5,SSP_TYPE_SSP);
  */

  ccpr2l.new_name("ccpr2l");
  ccpr2h.new_name("ccpr2h");
  ccp2con.new_name("ccp2con");

  get_pir2()->intcon = &intcon_reg;
  get_pir2()->pie    = &pie2;
  pie2.pir    = get_pir2();
  pie2.new_name("pie2");


  ((PORTC*)portc)->usart = &usart;
  ((PORTC*)portc)->ssp = &ssp;
  ((PORTA*)porta)->ssp = &ssp;
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

P16C63::P16C63(void)
{

  if(verbose)
    cout << "c63 constructor, type = " << isa() << '\n';

  init_ssp = true;

}

void P16C63::create(void)
{
  if(verbose)
    cout << " c63 create \n";

  P16C62::create();

  P16C63::create_sfr_map();

  // Build the links between the I/O Pins and the internal peripherals
  // ccp1con.iopin = portc.pins[2];
  ccp2con.iopin = portc->pins[1];

}

Processor * P16C63::construct(void)
{

  P16C63 *p = new P16C63;

  if(verbose)
    cout << " c63 construct\n";

  p->create();
  p->create_invalid_registers ();

  p->pic_processor::create_symbols();

  p->ssp.initialize_14(p,p->get_pir_set(),p->portc,3,4,5,p->porta,5,SSP_TYPE_BSSP);

  p->new_name("p16c63");
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


  add_sfr_register(porta,   0x05);
  add_sfr_register(&trisa,  0x85, RegisterValue(0x3f,0));

  add_sfr_register(portb,   0x06);
  add_sfr_register(&trisb,  0x86, RegisterValue(0xff,0));

  add_sfr_register(portc,   0x07);
  add_sfr_register(&trisc,  0x87, RegisterValue(0xff,0));

  add_sfr_register(portd,   0x08);
  add_sfr_register(&trisd,  0x88, RegisterValue(0xff,0));

  add_sfr_register(porte,   0x09);
  add_sfr_register(&trise,  0x89, RegisterValue(0x07,0));

  ((PORTC*)portc)->ccp1con = &ccp1con;

}

void P16C64::create_symbols(void)
{

  if(verbose)
    cout << "creating c64 symbols\n";

  symbol_table.add_ioport(porta);
  symbol_table.add_ioport(portb);
  symbol_table.add_ioport(portc);
  symbol_table.add_ioport(portd);
  symbol_table.add_ioport( porte);

}


void  P16C64::create(void)
{

  if(verbose)
    cout << " c64 create \n";

  create_iopin_map();
  
  _14bit_processor::create();

  P16X6X_processor::create_sfr_map();
  P16C64::create_sfr_map();

  // Build the links between the I/O Pins and the internal peripherals
  ccp1con.iopin = portc->pins[2];

}

Processor * P16C64::construct(void)
{

  P16C64 *p = new P16C64;

  cout << " c64 construct\n";

  p->create();
  p->create_invalid_registers ();
  p->pic_processor::create_symbols();

  p->ssp.initialize_14(p,p->get_pir_set(),p->portc,3,4,5,p->porta,5,SSP_TYPE_BSSP);

  p->new_name("p16c64");
  symbol_table.add_module(p,p->name().c_str());

  return p;

}

P16C64::P16C64(void)
{
  if(verbose)
    cout << "c64 constructor, type = " << isa() << '\n';

  init_ssp = true;
}

//------------------------------------------------------------------------
//
//

void P16C65::create_sfr_map(void)
{

  if(verbose)
    cout << "creating c65 registers\n";

  add_file_registers(0xc0, 0xff, 0);

  add_sfr_register(get_pir2(),   0x0d, RegisterValue(0,0));
  add_sfr_register(&pie2,   0x8d, RegisterValue(0,0));

  add_sfr_register(&ccpr2l, 0x1b, RegisterValue(0,0));
  add_sfr_register(&ccpr2h, 0x1c, RegisterValue(0,0));
  add_sfr_register(&ccp2con, 0x1d, RegisterValue(0,0));

  // get_pir_set()->set_pir2(&get_pir2());
  pir_set_def.set_pir2(&pir2_reg);

  ccp2con.ccprl = &ccpr2l;
  ccp2con.pir_set   = get_pir_set();
  ccp2con.tmr2  = &tmr2;
  ccpr2l.ccprh  = &ccpr2h;
  ccpr2l.tmrl   = &tmr1l;
  ccpr2h.ccprl  = &ccpr2l;

  add_sfr_register(usart.rcsta, 0x18, RegisterValue(0,0),"rcsta");
  add_sfr_register(usart.txsta, 0x98, RegisterValue(2,0),"txsta");
  add_sfr_register(usart.spbrg, 0x99, RegisterValue(0,0),"spbrg");
  add_sfr_register(usart.txreg, 0x19, RegisterValue(0,0),"txreg");
  add_sfr_register(usart.rcreg, 0x1a, RegisterValue(0,0),"rcreg");
  usart.initialize_14(this,get_pir_set(),portc,7);

  //  cout << "portc HACK \n";
  int i;
  for(i=0; i<8; i++) {
    if(portc->pins[i]) {
      portc->pins[i]->iopp=&(this->registers[7]);
    }
  }

  /*
  add_sfr_register(ssp.sspbuf, 0x13, RegisterValue(0,0),"sspbuf");
  add_sfr_register(ssp.sspcon, 0x14, RegisterValue(0,0),"sspcon");
  add_sfr_register(ssp.sspadd, 0x93, RegisterValue(0,0),"sspadd");
  add_sfr_register(ssp.sspstat, 0x94, RegisterValue(0,0),"sspstat");
  ssp.initialize_14(this,get_pir_set(),portc,3,4,5,porta,5,SSP_TYPE_SSP);
  */

  ccpr2l.new_name("ccpr2l");
  ccpr2h.new_name("ccpr2h");
  ccp2con.new_name("ccp2con");

  get_pir2()->intcon = &intcon_reg;
  get_pir2()->pie    = &pie2;
  pie2.pir    = get_pir2();
  pie2.new_name("pie2");


  ((PORTC*)portc)->usart = &usart;
  ((PORTC*)portc)->ssp = &ssp;
  ((PORTA*)porta)->ssp = &ssp;
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

P16C65::P16C65(void)
{

  if(verbose)
    cout << "c65 constructor, type = " << isa() << '\n';

  init_ssp = true;

}

void P16C65::create(void)
{
  if(verbose)
    cout << " c65 create \n";

  P16C64::create();

  P16C65::create_sfr_map();

  // Build the links between the I/O Pins and the internal peripherals
  // ccp1con.iopin = portc.pins[2];
  ccp2con.iopin = portc->pins[1];

}

Processor * P16C65::construct(void)
{

  P16C65 *p = new P16C65;

  if(verbose)
    cout << " c65 construct\n";

  p->create();
  p->create_invalid_registers ();

  p->pic_processor::create_symbols();

  p->ssp.initialize_14(p,p->get_pir_set(),p->portc,3,4,5,p->porta,5,SSP_TYPE_BSSP);

  p->new_name("p16c65");
  symbol_table.add_module(p,p->name().c_str());

  return p;

}

