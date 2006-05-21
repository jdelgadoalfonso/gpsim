/*
   Copyright (C) 1998-2000 T. Scott Dattalo

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

#ifndef __P16F87X_H__
#define __P16F87X_H__

#include "p16x7x.h"

#include "eeprom.h"
#include "comparator.h"

class IOPORT;


class P16F871 : public P16C64   // The 74 has too much RAM and too many CCPs
{
 public:
  // XXX
  // This pir1_2, pir2_2 stuff is not particularly pretty.  It would be
  // better to just tell C++ to redefine pir1 and pir2 and PIR1v2 and
  // PIR2v2, but C++ only supports covariance in member function return
  // values.
  PIR1v2 pir1_2_reg;
  PIR2v2 pir2_2_reg;
  PIR_SET_2 pir_set_2_def;

  virtual PIR *get_pir1() { return (&pir1_2_reg); }
  virtual PIR *get_pir2() { return (&pir2_2_reg); }
  virtual PIR_SET *get_pir_set() { return (&pir_set_2_def); }

  ADCON0_withccp adcon0;
  ADCON1 adcon1;
  ADRES  adres;
  ADRES  adresl;

  USART_MODULE14 usart;

  // That now brings us up to spec with the 74 as far as we need to be

  
  virtual void set_out_of_range_pm(unsigned int address, unsigned int value);

  virtual PROCESSOR_TYPE isa(void){return _P16F871_;};
  virtual unsigned int program_memory_size(void) const { return 0x0800; };
  virtual void create_symbols(void);
  void create_sfr_map(void);
  void create(void);
  virtual unsigned int register_memory_size () const { return 0x200;};

  P16F871(void);
  static Processor *construct(void);

  virtual void set_eeprom(EEPROM *ep) {
    // use set_eeprom_wide as P16F871 expect a wide EEPROM
    assert(0);

  }
  virtual void set_eeprom_wide(EEPROM_WIDE *ep) {
    eeprom = ep;
  }
  virtual EEPROM_WIDE *get_eeprom(void) { return ((EEPROM_WIDE *)eeprom); }

  virtual bool hasSSP() { return false;}

private:

};




class P16F873 : public P16C73
{

 public:

  ADRES  adresl;

  virtual void set_out_of_range_pm(unsigned int address, unsigned int value);

  virtual PROCESSOR_TYPE isa(void){return _P16F873_;};
  virtual unsigned int program_memory_size(void) const { return 0x1000; };
  virtual void create_symbols(void);
  void create_sfr_map(void);
  void create(void);
  virtual unsigned int register_memory_size () const { return 0x200;};

  P16F873(void);

  virtual void set_eeprom(EEPROM *ep) {
    // use set_eeprom_wide as P16F873 expect a wide EEPROM
    assert(0);
  }
  virtual void set_eeprom_wide(EEPROM_WIDE *ep) {
    eeprom = ep;
  }
  virtual EEPROM_WIDE *get_eeprom(void) { return ((EEPROM_WIDE *)eeprom); }
  static Processor *construct(void);

private:

};


class P16F873A : public P16F873
{
 public:
 COMPARATOR_MODULE comparator;
  virtual PROCESSOR_TYPE isa(void){return _P16F873A_;};
//  virtual void create_symbols(void);
  void create_sfr_map(void);
  void create(void);

  P16F873A(void);
  static Processor *construct(void);
};


class P16F876 : public P16F873
{
 public:
  virtual PROCESSOR_TYPE isa(void){return _P16F876_;};
  virtual unsigned int program_memory_size(void) const { return 0x2000; };
  virtual void create_symbols(void);
  void create_sfr_map(void);
  void create(void);
  virtual unsigned int register_memory_size () const { return 0x200;};

  P16F876(void);
  static Processor *construct(void);
};

class P16F876A : public P16F873A
{
 public:
 COMPARATOR_MODULE comparator;
  virtual PROCESSOR_TYPE isa(void){return _P16F876A_;};
  virtual unsigned int program_memory_size(void) const { return 0x2000; };
//  virtual void create_symbols(void);
  void create_sfr_map(void);
  void create(void);
  virtual unsigned int register_memory_size () const { return 0x200;};

  P16F876A(void);
  static Processor *construct(void);
};


class P16F874 : public P16C74
{
 public:
 COMPARATOR_MODULE comparator;

  ADRES  adresl;

  virtual void set_out_of_range_pm(unsigned int address, unsigned int value);

  virtual PROCESSOR_TYPE isa(void){return _P16F874_;};
  virtual unsigned int program_memory_size(void) const { return 0x1000; };
  virtual void create_symbols(void);
  void create_sfr_map(void);
  void create(void);
  virtual unsigned int register_memory_size () const { return 0x200;};

  P16F874(void);
  static Processor *construct(void);

  virtual void set_eeprom(EEPROM *ep) {
    // use set_eeprom_wide as P16F873 expect a wide EEPROM
    assert(0);
  }
  virtual void set_eeprom_wide(EEPROM_WIDE *ep) {
    eeprom = ep;
  }
  virtual EEPROM_WIDE *get_eeprom(void) { return ((EEPROM_WIDE *)eeprom); }
};

class P16F877 : public P16F874
{
 public:
  virtual PROCESSOR_TYPE isa(void){return _P16F877_;};
  virtual unsigned int program_memory_size(void) const { return 0x2000; };
  virtual void create_symbols(void);
  void create_sfr_map(void);
  void create(void);

  P16F877(void);
  static Processor *construct(void);
};

class P16F874A : public P16F874
{
 public:
 COMPARATOR_MODULE comparator;

  virtual void set_out_of_range_pm(unsigned int address, unsigned int value);

  virtual PROCESSOR_TYPE isa(void){return _P16F874A_;};
  virtual unsigned int program_memory_size(void) const { return 0x1000; };
  virtual void create_symbols(void);
  void create_sfr_map(void);
  void create(void);
  virtual unsigned int register_memory_size () const { return 0x200;};

  P16F874A(void);
  static Processor *construct(void);

};

class P16F877A : public P16F874A
{
 public:
 COMPARATOR_MODULE comparator;
  virtual PROCESSOR_TYPE isa(void){return _P16F877A_;};
  virtual unsigned int program_memory_size(void) const { return 0x2000; };
  virtual void create_symbols(void);
  void create_sfr_map(void);
  void create(void);

  P16F877A(void);
  static Processor *construct(void);
};

#endif
