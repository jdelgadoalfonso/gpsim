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
//    P16C56


#include <stdio.h>
#include <iostream>
#include <string>

#include "packages.h"
#include "p16x5x.h"
#include "pic-ioports.h"

#include "symbol.h"


void P16C54::create_iopin_map()
{
#ifdef USE_PIN_MODULE_FOR_TOCKI
    IOPIN * tockipin;
#endif

  package = new Package(18);
  if(!package)
    return;

  // Now Create the package and place the I/O pins

  package->assign_pin(17, m_porta->addPin(new IO_bi_directional("porta0"),0));
  package->assign_pin(18, m_porta->addPin(new IO_bi_directional("porta1"),1));
  package->assign_pin( 1, m_porta->addPin(new IO_bi_directional("porta2"),2));
  package->assign_pin( 2, m_porta->addPin(new IO_bi_directional("porta3"),3));
#ifdef USE_PIN_MODULE_FOR_TOCKI
  // RCP - attempt to add TOCKI without port register
  tockipin = new IOPIN("tocki");
  m_tocki->setPin ( tockipin );
  package->assign_pin( 3, tockipin );
  // RCP - End new code
#else
  package->assign_pin( 3, m_tocki->addPin(new IOPIN("tocki"),0));
#endif
  package->assign_pin( 4, 0);
  package->assign_pin( 5, 0);
  package->assign_pin( 6, m_portb->addPin(new IO_bi_directional("portb0"),0));
  package->assign_pin( 7, m_portb->addPin(new IO_bi_directional("portb1"),1));
  package->assign_pin( 8, m_portb->addPin(new IO_bi_directional("portb2"),2));
  package->assign_pin( 9, m_portb->addPin(new IO_bi_directional("portb3"),3));
  package->assign_pin(10, m_portb->addPin(new IO_bi_directional("portb4"),4));
  package->assign_pin(11, m_portb->addPin(new IO_bi_directional("portb5"),5));
  package->assign_pin(12, m_portb->addPin(new IO_bi_directional("portb6"),6));
  package->assign_pin(13, m_portb->addPin(new IO_bi_directional("portb7"),7));
  package->assign_pin(14, 0);
  package->assign_pin(15, 0);
  package->assign_pin(16, 0);
}


void P16C55::create_iopin_map()
{
  package = new Package(28);
  if(!package)
    return;

  // Now Create the package and place the I/O pins

  package->assign_pin( 6, m_porta->addPin(new IO_bi_directional("porta0"),0));
  package->assign_pin( 7, m_porta->addPin(new IO_bi_directional("porta1"),1));
  package->assign_pin( 8, m_porta->addPin(new IO_bi_directional("porta2"),2));
  package->assign_pin( 9, m_porta->addPin(new IO_bi_directional("porta3"),3));
#ifdef USE_PIN_MODULE_FOR_TOCKI
  // RCP - attempt to add TOCKI without port register
  tockipin = new IOPIN("tocki");
  m_tocki->setPin ( tockipin );
  package->assign_pin( 1, tockipin );
  // RCP - End new code
#else
  package->assign_pin( 1, m_tocki->addPin(new IOPIN("tocki"),0));
#endif
  package->assign_pin( 2, 0);
  package->assign_pin( 3, 0);
  package->assign_pin( 4, 0);
  package->assign_pin( 5, 0);

  package->assign_pin(10, m_portb->addPin(new IO_bi_directional("portb0"),0));
  package->assign_pin(11, m_portb->addPin(new IO_bi_directional("portb1"),1));
  package->assign_pin(12, m_portb->addPin(new IO_bi_directional("portb2"),2));
  package->assign_pin(13, m_portb->addPin(new IO_bi_directional("portb3"),3));
  package->assign_pin(14, m_portb->addPin(new IO_bi_directional("portb4"),4));
  package->assign_pin(15, m_portb->addPin(new IO_bi_directional("portb5"),5));
  package->assign_pin(16, m_portb->addPin(new IO_bi_directional("portb6"),6));
  package->assign_pin(17, m_portb->addPin(new IO_bi_directional("portb7"),7));

  package->assign_pin(18, m_portc->addPin(new IO_bi_directional("portc0"),0));
  package->assign_pin(19, m_portc->addPin(new IO_bi_directional("portc1"),1));
  package->assign_pin(20, m_portc->addPin(new IO_bi_directional("portc2"),2));
  package->assign_pin(21, m_portc->addPin(new IO_bi_directional("portc3"),3));
  package->assign_pin(22, m_portc->addPin(new IO_bi_directional("portc4"),4));
  package->assign_pin(23, m_portc->addPin(new IO_bi_directional("portc5"),5));
  package->assign_pin(24, m_portc->addPin(new IO_bi_directional("portc6"),6));
  package->assign_pin(25, m_portc->addPin(new IO_bi_directional("portc7"),7));

  package->assign_pin(26, 0);
  package->assign_pin(27, 0);
  package->assign_pin(28, 0);

}



void P16C54::create_sfr_map()
{
  if(verbose)
    cout << "creating c54 registers\n";

  add_file_registers(0x07, 0x1f, 0x00);

 
  add_sfr_register(indf,   0x00);

  add_sfr_register(&tmr0,  0x01);

  add_sfr_register(pcl,    0x02, RegisterValue(0,0));
  add_sfr_register(status, 0x03, RegisterValue(0x18,0));
  add_sfr_register(fsr,    0x04);

  add_sfr_register(m_porta, 0x05);
  add_sfr_register(m_portb, 0x06);

  add_sfr_register(&option_reg,  0xffffffff, RegisterValue(0xff,0));
  add_sfr_register(m_trisa,  0xffffffff, RegisterValue(0x1f,0));
  add_sfr_register(m_trisb,  0xffffffff, RegisterValue(0xff,0));
#ifndef USE_PIN_MODULE_FOR_TOCKI
  add_sfr_register(m_tocki,  0xffffffff, RegisterValue(0x01,0));
  add_sfr_register(m_trist0, 0xffffffff, RegisterValue(0x01,0));
#endif

}


void P16C54::create_symbols()
{

  pic_processor::create_symbols();
  symbol_table.add_register(m_porta);
  symbol_table.add_register(m_trisa);

  symbol_table.add_register(m_portb);
  symbol_table.add_register(m_trisb);
}

void P16C54::create()
{

  create_iopin_map();
  _12bit_processor::create();

}

Processor * P16C54::construct()
{

  P16C54 *p = new P16C54;

  if(verbose)
    cout << " c54 construct\n";

  p->new_name("p16c54");

  p->pc->set_reset_address(0x1ff);

  p->create();
  p->create_invalid_registers();
  p->create_sfr_map();
  p->create_symbols();

  symbol_table.add_module(p,p->name().c_str());

  return p;

}

P16C54::P16C54()
{
  if(verbose)
    cout << "c54 constructor, type = " << isa() << '\n';

  m_porta = new PicPortRegister("porta",8,0x1f);
  m_trisa = new PicTrisRegister("trisa", m_porta);

  m_portb = new PicPortRegister("portb",8,0xff);
  m_trisb = new PicTrisRegister("trisb", m_portb);

#ifdef USE_PIN_MODULE_FOR_TOCKI
//  RCP - Attempt to assign TOCKI without a port register
  m_tocki = new PinModule();
  cout << "c54 contructor assigning tmr0\n";
  tmr0.set_cpu(this, m_tocki);
#else
  m_tocki = new PicPortRegister("tockiport",8,0x01);
  m_trist0 = new PicTrisRegister("trist0", m_tocki);
//  cout << "c54 contructor assigning tmr0 to tocki register\n";
  tmr0.set_cpu(this, m_tocki, 0);
#endif
  tmr0.start(0);

}

void P16C54::tris_instruction(unsigned int tris_register)
{

   switch (tris_register)
   {
      case 5:
        m_trisa->put(W->value.get());
        trace.write_TRIS(m_trisa->value.get());
        break;
      case 6:
        m_trisb->put(W->value.get());
        trace.write_TRIS(m_trisb->value.get());
        break;
      default:
        cout << __FUNCTION__ << ": Unknown TRIS register " << tris_register << endl;
        break;
   }
}





void P16C55::create_sfr_map()
{
  if(verbose)
    cout << "creating c55 registers\n";

  P16C54::create_sfr_map();

  add_sfr_register(m_portc, 0x07);
  add_sfr_register(m_trisc,  0xffffffff, RegisterValue(0xff,0));

}


void P16C55::create_symbols()
{
  P16C54::create_symbols();

  symbol_table.add_register(m_portc);
  symbol_table.add_register(m_trisc);

}

void P16C55::create()
{
  P16C54::create();
}

Processor * P16C55::construct()
{

  P16C55 *p = new P16C55;

  p->new_name("p16c55");

  if(verbose)
    cout << " c55 construct\n";

  p->pc->set_reset_address(0x1ff);

  p->create();
  p->create_invalid_registers();
  p->create_sfr_map();
  p->create_symbols();

  symbol_table.add_module(p,p->name().c_str());

  return p;

}

P16C55::P16C55()
{
  if(verbose)
    cout << "c55 constructor, type = " << isa() << '\n';

  m_portc = new PicPortRegister("portc",8,0xff);
  m_trisc = new PicTrisRegister("trisc", m_portc);

}

void P16C55::tris_instruction(unsigned int tris_register)
{

   switch (tris_register)
   {
      case 5:
        m_trisa->put(W->value.get());
        trace.write_TRIS(m_trisa->value.get());
        break;
      case 6:
        m_trisb->put(W->value.get());
        trace.write_TRIS(m_trisb->value.get());
        break;
      case 7:
        m_trisc->put(W->value.get());
        trace.write_TRIS(m_trisc->value.get());
        break;
      default:
        cout << __FUNCTION__ << ": Unknown TRIS register " << tris_register << endl;
        break;
   }
}



Processor * P16C56::construct()
{

  P16C56 *p = new P16C56;

  p->new_name("p16c56");

  if(verbose)
    cout << " c56 construct\n";

  p->pc->set_reset_address(0x3ff);

  p->create();
  p->create_invalid_registers();
  p->create_sfr_map();
  p->create_symbols();

  symbol_table.add_module(p,p->name().c_str());

  return p;
}


P16C56::P16C56()
{
  if(verbose)
    cout << "c56 constructor, type = " << isa() << '\n';

}

