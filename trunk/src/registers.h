/*
   Copyright (C) 1998-2003 Scott Dattalo

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

#ifndef __REGISTERS_H__
#define __REGISTERS_H__

#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <glib.h>

class symbol;
class XrefObject;
class Processor;

#include "gpsim_classes.h"
#include "value.h"


//---------------------------------------------------------
// RegisterValue class
//
// This class is used to represent the value of registers.
// It also defines which bits have been initialized and which
// are valid.
//

class RegisterValue {
 public:

  unsigned int data;  // The actual numeric value of the register.
  unsigned int init;  // bit mask of initialized bits.

  RegisterValue(void)
    {
      data = 0;
      init = 0xff;  // assume 8-bit wide, unitialized registers
    }

  RegisterValue(unsigned int d, unsigned int i) : 
    data(d), init(i)
    {
    }

  bool initialized(void)
    {
      return init == 0;
    }

  unsigned int get(void)
    {
      return data;
    }

  void put(unsigned int d)
    {
      data = d;
    }

  void operator = (RegisterValue rv)
    {
      data = rv.data;
      init = rv.init;
    }


};


//---------------------------------------------------------
// Base class for a file register.
class Register : public gpsimValue
{
public:

  enum REGISTER_TYPES
  {
    INVALID_REGISTER,
    GENERIC_REGISTER,
    FILE_REGISTER,
    SFR_REGISTER,
    BP_REGISTER
  };

  RegisterValue value;

  unsigned int address;

  // If non-zero, the alias_mask describes all address at which
  // this file register appears. The assumption (that is true so
  // far for all pic architectures) is that the aliased register
  // locations differ by one bit. For example, the status register
  // appears at addresses 0x03 and 0x83 in the 14-bit core. 
  // Consequently, alias_mask = 0x80 and address (above) is equal
  // to 0x03.

  unsigned int alias_mask;

  unsigned int por_value;  // power on reset value

  unsigned int bit_mask;   // = 7 for 8-bit registers, = 15 for 16-bit registers.

  symbol *symbol_alias;

  guint64 read_access_count;
  guint64 write_access_count;



  Register(void);
  virtual ~Register(void);


  // Register access functions
  virtual unsigned int get(void);
  virtual void put(unsigned int new_value);

  
  /* put_value is the same as put(), but some extra stuff like
   * interfacing to the gui is done. (It's more efficient than
   * burdening the run time performance with (unnecessary) gui
   * calls.)
   */

  virtual void put_value(unsigned int new_value);

  /* same as get(), but no trace is performed */
  virtual unsigned int get_value(void) { return(value.get()); }

  /* getRV and putRV are the accessor functions that get the
   * the fule RegisterValue object (minus the value.valid bits)
   */
  virtual RegisterValue getRV(void) { return value;}
  virtual void putRV(RegisterValue rv)
    { 
      value.data = rv.data;
      value.init = rv.init;
    }

  // In the Register class, the 'Register *get()' returns a
  // pointer to itself. Derived classes may return something
  // else (e.g. a break point may be pointing to the register
  // it replaced and will return that instead).

  virtual Register *getReg(void)
    {
      return this;
    }
  

  virtual REGISTER_TYPES isa(void) {return GENERIC_REGISTER;};
  virtual void reset(RESET_TYPE r) { return; };

  /* 
     setbit functions are not really intended for general purpose
     registers (although they can be). Instead, they provide place
     holders which are over-ridden by IO ports. The purpose is to
     provide an abstract way in which break points can be set
     on individual IO pin changes.
  */
  virtual void setbit(unsigned int bit_number, bool new_value);
  virtual void setbit_value(unsigned int bit_number, bool new_value);

  /*
    like setbit, getbit is used mainly for breakpoints.
   */
  virtual int get_bit(unsigned int bit_number);
  virtual int get_bit_voltage(unsigned int bit_number);

  /*
    Breakpoint objects will overload this function and return true.
   */

  virtual bool hasBreak(void)
    { 
      return false;
    }

  /*
    When a breakpoint is set on this register, that object (which is derived
    from the register class) will save a pointer to itself here:
  */
  virtual bool replacingWith(Register *replacer)
    {
      replacedBy = replacer;
      return true;
    }
 protected:

  /*
    If a breakpoint gets set on a register, then a copy of the pointer to it will
    get stored here.
  */

  Register *replacedBy;
};


//---------------------------------------------------------
// define a special 'invalid' register class. Accessess to
// to this class' value get 0

class InvalidRegister : public Register
{
public:

  InvalidRegister(void);
  InvalidRegister(unsigned int at_address);

  void put(unsigned int new_value);
  unsigned int get(void);
  virtual REGISTER_TYPES isa(void) {return INVALID_REGISTER;};
  virtual Register *getReg(void)
    {
      return 0;
    }
  

};


//---------------------------------------------------------
// Base class for a special function register.
class sfr_register : public Register
{
public:
  sfr_register() : Register(){}
  unsigned int wdtr_value; // wdt or mclr reset value

  virtual REGISTER_TYPES isa(void) {return SFR_REGISTER;};
  virtual void initialize(void) {return;};

  virtual void reset(RESET_TYPE r) {
    switch (r) {

    case POR_RESET:
      value.put(por_value);
      break;

    case WDT_RESET:
      value.put(wdtr_value);
      break;
    case SOFT_RESET:
      break;
    }

  }
};



//---------------------------------------------------------
// Program Counter
//

class Program_Counter : public gpsimValue
{
public:
  unsigned int value;              /* pc's current value */
  unsigned int memory_size_mask; 
  unsigned int pclath_mask;        /* pclath confines PC to banks */
  unsigned int instruction_phase;

  Program_Counter(void);
  virtual void increment(void);
  virtual void skip(void);
  virtual void jump(unsigned int new_value);
  virtual void interrupt(unsigned int new_value);
  virtual void computed_goto(unsigned int new_value);
  virtual void new_address(unsigned int new_value);
  virtual void put_value(unsigned int new_value);
  virtual unsigned int get_value(void)
    {
      return value;
    }

  // get_raw_value -- on the 16-bit cores, get_value is multiplied by 2
  // whereas get_raw_value isn't. The raw value of the program counter
  // is used as an index into the program memory.
  virtual unsigned int get_raw_value(void)
    {
      return value;
    }

  virtual void set_phase(int phase)
    { 
      instruction_phase = phase;
    }
  virtual int get_phase(void) 
    {
      return instruction_phase; 
    }
  
  void set_reset_address(unsigned int _reset_address)
    {
      reset_address = _reset_address;
    }
  unsigned int get_reset_address(void) 
    {
      return reset_address;
    }

  void reset(void);

  virtual unsigned int get_next(void);

private:
  unsigned int reset_address;      /* Value pc gets at reset */
  
};

#endif // __REGISTERS__
