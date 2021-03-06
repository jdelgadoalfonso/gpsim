/*
   Copyright (C) 1998-2002 T. Scott Dattalo

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

#ifndef __P16F62X_H__
#define __P16F62X_H__

#include "p16x6x.h"

#include "eeprom.h"
#include "comparator.h"

/***************************************************************************
 *
 * Include file for:  P16F627, P16F628, P16F648
 *
 *
 * The F62x devices are quite a bit different from the other PICs. The class
 * heirarchy is similar to the 16F84.
 * 
 *
 ***************************************************************************/

class P16F62x : public P16X6X_processor
{
public:
  P16F62x(const char *_name=0, const char *desc=0);

  USART_MODULE usart;
  COMPARATOR_MODULE comparator;

  virtual void set_out_of_range_pm(unsigned int address, unsigned int value);

  virtual PROCESSOR_TYPE isa(){return _P16F627_;};
  virtual void create_symbols();
  virtual unsigned int register_memory_size () const { return 0x200;};

  virtual unsigned int program_memory_size() { return 0; };

  virtual void create_sfr_map();

  // The f628 (at least) I/O pins depend on the Fosc Configuration bits.
  virtual bool set_config_word(unsigned int address, unsigned int cfg_word);

  /*
  virtual int get_pin_count(){return Package::get_pin_count();};
  virtual char *get_pin_name(unsigned int pin_number) {return Package::get_pin_name(pin_number);};
  virtual int get_pin_state(unsigned int pin_number) {return Package::get_pin_state(pin_number);};
  virtual IOPIN *get_pin(unsigned int pin_number) {return Package::get_pin(pin_number);};
  */

  virtual void create(int ram_top, unsigned int eeprom_size);
  virtual void create_iopin_map();

  virtual void set_eeprom(EEPROM *ep) {
    // Use set_eeprom_pir as P16F62x expects to have a PIR capable EEPROM
    assert(0);
  }
  virtual void set_eeprom_pir(EEPROM_PIR *ep) {
    eeprom = ep;
  }
  virtual EEPROM_PIR *get_eeprom() { return ((EEPROM_PIR *)eeprom); }
};

class P16F627 : public P16F62x
{
public:

  virtual PROCESSOR_TYPE isa(){return _P16F627_;};

  virtual unsigned int program_memory_size() const { return 0x1000; };

  P16F627(const char *_name=0, const char *desc=0);
  static Processor *construct(const char *name);
};

class P16F628 : public P16F627
{
public:

  virtual PROCESSOR_TYPE isa(){return _P16F628_;};

  virtual unsigned int program_memory_size() const { return 0x2000; };

  P16F628(const char *_name=0, const char *desc=0);
  static Processor *construct(const char *name);
};

class P16F648 : public P16F628
{
public:

  virtual PROCESSOR_TYPE isa(){return _P16F648_;};

  virtual unsigned int program_memory_size() const { return 0x4000; };
  virtual void create_sfr_map();

  P16F648(const char *_name=0, const char *desc=0);
  static Processor *construct(const char *name);
};

#endif
