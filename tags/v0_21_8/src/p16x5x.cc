/*
   Copyright (C) 2000 T. Scott Dattalo, Daniel Schudel

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
// p16x5x
//
//  This file supports:
//    P16C54
//    P16C55


#include <stdio.h>
#include <iostream>
#include <string>

#include "packages.h"
#include "p16x5x.h"

#include "symbol.h"


void P16C54::create_iopin_map(void)
{

  package = new Package(18);
  if(!package)
    return;

  // ---- This is probably going to be moved:
  porta = new PORTA;
  portb = new PIC_IOPORT;	// just a straight IO port, no fancy bits
  portb->new_name("portb");	// but we need to set the name

  // ---- Complete the initialization for the I/O Ports

  // Build the links between the I/O Ports and their tris registers.
  porta->tris = &trisa;
  trisa.port = porta;

  portb->tris = &trisb;
  trisb.port = portb;

  // And give them a more meaningful name.
  trisa.new_name("trisa");

  trisb.new_name("trisb");

  // Define the valid I/O pins.
  porta->valid_iopins = 0x1f;
  portb->valid_iopins = 0xff;

  // Now Create the package and place the I/O pins

  //  create_pkg(18);

  package->assign_pin(17, new IO_bi_directional(porta, 0));
  package->assign_pin(18, new IO_bi_directional(porta, 1));
  package->assign_pin(1, new IO_bi_directional(porta, 2));
  package->assign_pin(2, new IO_bi_directional(porta, 3));
  package->assign_pin(3, new IO_open_collector(porta, 4));
  package->assign_pin(4, 0);
  package->assign_pin(5, 0);
  package->assign_pin(6, new IO_bi_directional_pu(portb, 0));
  package->assign_pin(7, new IO_bi_directional_pu(portb, 1));
  package->assign_pin(8, new IO_bi_directional_pu(portb, 2));
  package->assign_pin(9, new IO_bi_directional_pu(portb, 3));
  package->assign_pin(10, new IO_bi_directional_pu(portb, 4));
  package->assign_pin(11, new IO_bi_directional_pu(portb, 5));
  package->assign_pin(12, new IO_bi_directional_pu(portb, 6));
  package->assign_pin(13, new IO_bi_directional_pu(portb, 7));
  package->assign_pin(14, 0);
  package->assign_pin(15, 0);
  package->assign_pin(16, 0);



}


void P16C55::create_iopin_map(void)
{
  package = new Package(28);
  if(!package)
    return;

  // ---- This is probably going to be moved:
  porta = new PORTA;
  portb = new PIC_IOPORT;	// just a straight IO port, no fancy bits
  portb->new_name("portb");	// but we need to set the name
  portc = new PIC_IOPORT;	// just a straight IO port, no fancy bits
  portc->new_name("portc");	// but we need to set the name

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
  porta->valid_iopins = 0x1f;
  portb->valid_iopins = 0xff;
  portc->valid_iopins = 0xff;

  // Now Create the package and place the I/O pins

  package->assign_pin(6, new IO_bi_directional(porta, 0));
  package->assign_pin(7, new IO_bi_directional(porta, 1));
  package->assign_pin(8, new IO_bi_directional(porta, 2));
  package->assign_pin(9, new IO_bi_directional(porta, 3));
  package->assign_pin(1, new IO_open_collector(porta, 4));
  package->assign_pin(2, 0);
  package->assign_pin(3, 0);
  package->assign_pin(4, 0);
  package->assign_pin(5, 0);
  package->assign_pin(10, new IO_bi_directional_pu(portb, 0));
  package->assign_pin(11, new IO_bi_directional_pu(portb, 1));
  package->assign_pin(12, new IO_bi_directional_pu(portb, 2));
  package->assign_pin(13, new IO_bi_directional_pu(portb, 3));
  package->assign_pin(14, new IO_bi_directional_pu(portb, 4));
  package->assign_pin(15, new IO_bi_directional_pu(portb, 5));
  package->assign_pin(16, new IO_bi_directional_pu(portb, 6));
  package->assign_pin(17, new IO_bi_directional_pu(portb, 7));
  package->assign_pin(18, new IO_bi_directional_pu(portc, 0));
  package->assign_pin(19, new IO_bi_directional_pu(portc, 1));
  package->assign_pin(20, new IO_bi_directional_pu(portc, 2));
  package->assign_pin(21, new IO_bi_directional_pu(portc, 3));
  package->assign_pin(22, new IO_bi_directional_pu(portc, 4));
  package->assign_pin(23, new IO_bi_directional_pu(portc, 5));
  package->assign_pin(24, new IO_bi_directional_pu(portc, 6));
  package->assign_pin(25, new IO_bi_directional_pu(portc, 7));
  package->assign_pin(26, 0);
  package->assign_pin(27, 0);
  package->assign_pin(28, 0);

}



void P16C54::create_sfr_map(void)
{
  if(verbose)
    cout << "creating c54 registers\n";

  add_file_registers(0x07, 0x1f, 0x00);

 
  add_sfr_register(indf,   0x00);

  add_sfr_register(&tmr0,  0x01);

  add_sfr_register(pcl,    0x02, RegisterValue(0,0));
  add_sfr_register(status, 0x03, RegisterValue(0x18,0));
  add_sfr_register(fsr,     0x04);
//alias_file_registers(0x02,0x04,0x80);

  add_sfr_register(porta,   0x05);
  add_sfr_register(portb,   0x06);

  add_sfr_register(&option_reg,  0xffffffff, RegisterValue(0xff,0));
  add_sfr_register(&trisa,  0xffffffff, RegisterValue(0x1f,0));
  add_sfr_register(&trisb,  0xffffffff, RegisterValue(0xff,0));

  pic_processor::create_symbols();

}


void P16C54::create_symbols(void)
{

  symbol_table.add_ioport(portb);
  symbol_table.add_ioport(porta);


}

void P16C54::create(void)
{

  create_iopin_map();

  _12bit_processor::create();

  create_sfr_map();

}

Processor * P16C54::construct(void)
{

  P16C54 *p = new P16C54;

  if(verbose)
    cout << " c54 construct\n";

  p->pc->set_reset_address(0x1ff);

  p->create();

  p->new_name("p16c54");
  symbol_table.add_module(p,p->name().c_str());

  return p;

}

P16C54::P16C54(void)
{
  if(verbose)
    cout << "c54 constructor, type = " << isa() << '\n';

}

void P16C54::tris_instruction(unsigned int tris_register)
{

   switch (tris_register)
   {
      case 5:
        trisa.put(W->value.get());
        trace.write_TRIS(trisa.value.get());
        break;
      case 6:
        trisb.put(W->value.get());
        trace.write_TRIS(trisb.value.get());
        break;
      default:
        cout << __FUNCTION__ << ": Unknown TRIS register " << tris_register << endl;
        break;
   }
}





void P16C55::create_sfr_map(void)
{
  if(verbose)
    cout << "creating c55 registers\n";

  add_file_registers(0x08, 0x1f, 0x00);

 
  add_sfr_register(indf,   0x00);

  add_sfr_register(&tmr0,  0x01);

  add_sfr_register(pcl,    0x02, RegisterValue(0,0));
  add_sfr_register(status, 0x03, RegisterValue(0x18,0));
  add_sfr_register(fsr,     0x04);
//alias_file_registers(0x02,0x04,0x80);

  add_sfr_register(porta,   0x05);
  add_sfr_register(portb,   0x06);
  add_sfr_register(portc,   0x07);

  add_sfr_register(&option_reg,  0xffffffff, RegisterValue(0xff,0));
  add_sfr_register(&trisa,  0xffffffff, RegisterValue(0x0f,0));
  add_sfr_register(&trisb,  0xffffffff, RegisterValue(0xff,0));
  add_sfr_register(&trisc,  0xffffffff, RegisterValue(0xff,0));

  pic_processor::create_symbols();

}


void P16C55::create_symbols(void)
{

  symbol_table.add_ioport(portc);
  symbol_table.add_ioport(portb);
  symbol_table.add_ioport(porta);


}

void P16C55::create(void)
{
  
  create_iopin_map();
  
  _12bit_processor::create();
  
  create_sfr_map();

}

Processor * P16C55::construct(void)
{

  P16C55 *p = new P16C55;

  if(verbose)
    cout << " c55 construct\n";

  p->pc->set_reset_address(0x1ff);

  p->create();

  p->new_name("p16c55");
  symbol_table.add_module(p,p->name().c_str());

  return p;

}

P16C55::P16C55(void)
{
  if(verbose)
    cout << "c55 constructor, type = " << isa() << '\n';

}

void P16C55::tris_instruction(unsigned int tris_register)
{

   switch (tris_register)
   {
      case 5:
        trisa.put(W->value.get());
        trace.write_TRIS(trisa.value.get());
        break;
      case 6:
        trisb.put(W->value.get());
        trace.write_TRIS(trisb.value.get());
        break;
      case 7:
        trisc.put(W->value.get());
        trace.write_TRIS(trisc.value.get());
        break;
      default:
        cout << __FUNCTION__ << ": Unknown TRIS register " << tris_register << endl;
        break;
   }
}

