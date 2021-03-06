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
// p16x8x
//
//  This file supports:
//    PIC16C84
//    PIC16CR84
//    PIC16F84
//    PIC16F83
//    PIC16CR83
//

#include <stdio.h>
#include <iostream>
#include <string>

#include "../config.h"
#include "symbol.h"

#include "stimuli.h"
#include "eeprom.h"

#include "p16x8x.h"

#include "packages.h"

void P16C8x::create_iopin_map(void)
{

  package = new Package(18);
  if(!package)
    return;

  // ---- This is probably going to be moved:
  porta = new PORTA;
  portb = new PORTB;

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


void P16C8x::create_sfr_map(void)
{
 
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

  add_sfr_register(get_eeprom()->get_reg_eedata(),  0x08);
  add_sfr_register(get_eeprom()->get_reg_eecon1(),  0x88, RegisterValue(0,0));

  add_sfr_register(get_eeprom()->get_reg_eeadr(),   0x09);
  add_sfr_register(get_eeprom()->get_reg_eecon2(),  0x89);

  add_sfr_register(pclath, 0x8a, RegisterValue(0,0));
  add_sfr_register(pclath, 0x0a, RegisterValue(0,0));

  add_sfr_register(&intcon_reg, 0x8b, RegisterValue(0,0));
  add_sfr_register(&intcon_reg, 0x0b, RegisterValue(0,0));

  intcon = &intcon_reg;


}

void P16C8x::create_symbols(void)
{
  symbol_table.add_ioport(portb);
  symbol_table.add_ioport(porta);

}

void P16C8x::set_out_of_range_pm(unsigned int address, unsigned int value)
{

  if( (address>= 0x2100) && (address < 0x2100 + get_eeprom()->get_rom_size()))
    {
      get_eeprom()->change_rom(address - 0x2100, value);
    }
}

void  P16C8x::create(int ram_top)
{
  EEPROM *e;
  create_iopin_map();

  _14bit_processor::create();

  e = new EEPROM;
  e->set_cpu(this);
  e->initialize(EEPROM_SIZE);

  //ema.set_cpu(this);
  //ema.set_Registers(e->rom, EEPROM_SIZE);
  e->set_intcon(&intcon_reg);

  set_eeprom(e);

  add_file_registers(0x0c, ram_top, 0x80);
  P16C8x::create_sfr_map();

}

//========================================================================
//
// Pic 16C84 
//

Processor * P16C84::construct(void)
{

  P16C84 *p = new P16C84;

  p->P16C8x::create(0x2f);
  p->create_invalid_registers ();
  p->pic_processor::create_symbols();
  p->new_name("p16c84");
  symbol_table.add_module(p,p->name().c_str());

  return p;

}

P16C84::P16C84(void)
{
}



//========================================================================
//
// Pic 16F84 
//


Processor * P16F84::construct(void)
{

  P16F84 *p = new P16F84;

  p->P16C8x::create(0x4f);
  p->create_invalid_registers ();
  p->pic_processor::create_symbols();
  p->new_name("p16f84");
  symbol_table.add_module(p,p->name().c_str());

  return p;

}

P16F84::P16F84(void)
{
}

//========================================================================
//
// Pic 16F83
//

P16F83::P16F83(void)
{
  name_str = "p16f83";
}

Processor * P16F83::construct(void)
{

  P16F83 *p = new P16F83;

  p->P16C8x::create(0x2f);
  p->create_invalid_registers ();
  p->pic_processor::create_symbols();
  p->new_name("p16f83");
  symbol_table.add_module(p,p->name().c_str());

  return p;

}

Processor * P16CR83::construct(void)
{
  return 0;

}

Processor * P16CR84::construct(void)
{
  return 0;
}

