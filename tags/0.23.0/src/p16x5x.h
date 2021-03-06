/*
   Copyright (C) 2000,2001 T. Scott Dattalo, Daniel Schudel, Robert Pearce

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


//
// p16x5x
//
//  This file supports:
//    P16C54
//    P16C55
//    P16C56

#ifndef __P16X5X_H__
#define __P16X5X_H__

#include "packages.h"
#include "stimuli.h"
#include "12bit-processors.h"

class PicPortRegister;
class PicTrisRegister;
class PicLatchRegister;


class P16C54 : public  _12bit_processor
{
public:
  PicPortRegister  *m_porta;
  PicTrisRegister  *m_trisa;

  PicPortRegister  *m_portb;
  PicTrisRegister  *m_trisb;

#ifdef USE_PIN_MODULE_FOR_TOCKI
  PinModule    *m_tocki;
#else
  PicPortRegister  *m_tocki;
  PicTrisRegister  *m_trist0;
#endif
  
  virtual PROCESSOR_TYPE isa(){return _P16C54_;};
  virtual void create_symbols();

  virtual unsigned int program_memory_size() const { return 0x200; };
  virtual unsigned int register_memory_size() const { return 0x20; };
  virtual unsigned int config_word_address() const {return 0xFFF;};

  virtual void create_sfr_map();

  virtual void option_new_bits_6_7(unsigned int bits) {}

  P16C54(const char *_name=0, const char *desc=0);
  virtual ~P16C54();
  void create();
  virtual void create_iopin_map();

  static Processor *construct(const char *name);
  virtual void tris_instruction(unsigned int tris_register);

  virtual unsigned int fsr_valid_bits()
    {
      return 0x1f;  // Only 32 register addresses 
    }

  virtual unsigned int fsr_register_page_bits()
    {
      return 0;     // Only one register page.
    }


};

class P16C55 : public  P16C54
{
public:

  PicPortRegister  *m_portc;
  PicTrisRegister  *m_trisc;

  virtual PROCESSOR_TYPE isa(){return _P16C55_;};
  virtual void create_symbols();

  virtual unsigned int program_memory_size() const { return 0x200; };
  virtual unsigned int register_memory_size() const { return 0x20; };
  virtual unsigned int config_word_address() const {return 0xFFF;};

  virtual void create_sfr_map();

  P16C55(const char *_name=0, const char *desc=0);
  virtual ~P16C55();
  virtual void create();
  virtual void create_iopin_map();

  static Processor *construct(const char *name);
  virtual void tris_instruction(unsigned int tris_register);

};

class P16C56 : public  P16C54
{
public:

  virtual PROCESSOR_TYPE isa(){return _P16C56_;};

  virtual unsigned int program_memory_size() const { return 0x400; };
  virtual unsigned int register_memory_size() const { return 0x20; };
  virtual unsigned int config_word_address() const {return 0xFFF;};

  P16C56(const char *_name=0, const char *desc=0);

  static Processor *construct(const char *name);

};

#endif
