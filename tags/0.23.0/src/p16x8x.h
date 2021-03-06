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
along with gpsim; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#ifndef __P16X8X_H__
#define __P16X8X_H__

#include "14bit-processors.h"
#include "intcon.h"

class P16X8X : public Pic14Bit
{
public:
  P16X8X(const char *_name=0, const char *desc=0);
  virtual void create_sfr_map();
  virtual void set_out_of_range_pm(unsigned int address, unsigned int value);
  virtual void create_iopin_map();
  virtual void create(int ram_top);
  virtual unsigned int register_memory_size () const { return 0x100; }
};

class P16C84 : public P16X8X
{
public:

  P16C84(const char *_name=0, const char *desc=0);

  virtual PROCESSOR_TYPE isa(){return _P16C84_;};
  virtual void create(int ram_top);

  virtual unsigned int program_memory_size() const { return 0x400; }
  static Processor *construct(const char *name);
};

class P16F84 : public P16X8X
{
public:

  virtual PROCESSOR_TYPE isa(){return _P16F84_;};

  virtual void create(int ram_top);
  virtual unsigned int program_memory_size() const { return 0x400; };

  P16F84(const char *_name=0, const char *desc=0);
  static Processor *construct(const char *name);
};

class P16CR84 : public P16F84
{
public:

  virtual PROCESSOR_TYPE isa(){return _P16CR84_;};

  P16CR84(const char *_name=0, const char *desc=0);
  static Processor *construct(const char *name);
};



class P16F83 : public P16X8X
{
public:

  virtual PROCESSOR_TYPE isa(){return _P16F83_;};

  virtual unsigned int program_memory_size() const { return 0x200; };
  virtual void create(int ram_top);

  P16F83(const char *_name=0, const char *desc=0);
  static Processor *construct(const char *name);
};

class P16CR83 : public P16F83
{
public:

  virtual PROCESSOR_TYPE isa(){return _P16CR83_;};

  P16CR83(const char *_name=0, const char *desc=0);
  static Processor *construct(const char *name);
};


#endif
