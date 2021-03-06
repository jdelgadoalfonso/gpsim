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
#include "12bit-processors.h"

#include <string>
#include "stimuli.h"

extern unsigned int config_word;


//-------------------------------------------------------------------
_12bit_processor::_12bit_processor(void)
{
  pc = new Program_Counter();

  pc->set_trace_command(trace.allocateTraceType(new PCTraceType(this,0,1)));

}

void _12bit_processor::create_symbols(void)
{
  cout << "12bit create symbols\n";
}

void _12bit_processor::por(void)
{
  pic_processor::por();
}

void _12bit_processor::reset(RESET_TYPE r)
{
  pic_processor::reset(r);
  
}
//-------------------------------------------------------------------

void _12bit_processor::set_config_word(unsigned int address,unsigned int cfg_word)
{
  config_word = cfg_word;

  // Clear all of the configuration bits in config_modes and then
  // reset each of them based on the config bits in cfg_word:
  //config_modes &= ~(CM_WDTE);
  //config_modes |= ( (cfg_word & WDTE) ? CM_WDTE : 0);
  //cout << " setting cfg_word and cfg_modes " << hex << config_word << "  " << config_modes << '\n';

  if((address == config_word_address()) && config_modes)
    config_modes->config_mode = (config_modes->config_mode & ~7) | (cfg_word & 7);

  if(verbose && config_modes)
    config_modes->print();

}

void _12bit_processor::create(void)
{

  if(verbose)
    cout << "_12bit_processor create, type = " << isa() << '\n';

  pa_bits = 0;                 // Assume only one code page (page select bits in status)

  pic_processor::create();

  fsr = new FSR_12(fsr_register_page_bits(), fsr_valid_bits());
  fsr->new_name("fsr");


  // Sigh. Hack, hack,... manually assign indf bits
  indf->fsr_mask = 0x1f;
  indf->base_address_mask1 = 0x0;
  indf->base_address_mask2 = 0x1f;

  stack->stack_mask = 1;        // The 12bit core only has 2 stack positions

  tmr0.set_cpu(this);
  tmr0.start(0);

}

//-------------------------------------------------------------------
void _12bit_processor::option_new_bits_6_7(unsigned int bits)
{

  //portb.rbpu_intedg_update(bits);

  cout << "12bit, option bits 6 and/or 7 changed\n";

}

//-------------------------------------------------------------------
void _12bit_processor::dump_registers (void)
{


  pic_processor::dump_registers();

  cout << "option = " << option_reg.value.get() << '\n';

}

