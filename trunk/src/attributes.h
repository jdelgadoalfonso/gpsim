/*
   Copyright (C) 1998-2005 T. Scott Dattalo

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

#if !defined(__ATTRIBUTES_H__)
#define __ATTRIBUTES_H__

#include "value.h"

class Processor;

/// gpsim attributes
///


/// WarnModeAttribute
class WarnModeAttribute : public Boolean
{
protected:
  Processor *cpu;
public:
  WarnModeAttribute(Processor *_cpu);
  virtual void set(Value *v);
  virtual void get(bool &b);
};


/// SafeModeAttribute
class SafeModeAttribute : public Boolean
{
protected:
  Processor *cpu;
public:
  SafeModeAttribute(Processor *_cpu);
  ~SafeModeAttribute();
  virtual void set(Value *v);
  virtual void get(bool &b);
};

/// UnknownModeAttribute
class UnknownModeAttribute : public Boolean
{
protected:
  Processor *cpu;
public:
  UnknownModeAttribute(Processor *_cpu);
  virtual void set(Value *v);
  virtual void get(bool &b);
};


/// BreakOnResetAttribute
class BreakOnResetAttribute : public Boolean
{
protected:
  Processor *cpu;
public:
  BreakOnResetAttribute(Processor *_cpu);
  virtual void set(Value *v);
  virtual void get(bool &b);
};
#endif //if !defined(__ATTRIBUTES_H__)

