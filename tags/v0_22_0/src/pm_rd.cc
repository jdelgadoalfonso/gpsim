/*
   Copyright (C) 1998-2003 Scott Dattalo
                 2003 Mike Durian
                 2006 Roy Rankin
                 2006 David Barnett

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

#include <assert.h>

#include <iostream>
#include <iomanip>
using namespace std;

#include <glib.h>

#include "trace.h"
#include "pic-processor.h"
#include "pm_rd.h"


//------------------------------------------------------------------------
//
// PM-related registers


void PMCON1::put(unsigned int new_value)
{
  trace.raw(write_trace.get() | value.get());

  new_value &= valid_bits;
  
  bool rd_rise = (bool)(new_value & ~value.get() & RD);
  value.put((value.get() & RD) | new_value);

  if (rd_rise)
    pm_rd->start_read();
}

unsigned int PMCON1::get(void)
{
  trace.raw(read_trace.get() | value.get());

  return(value.get());
}

PMCON1::PMCON1(void)
{
  new_name("pmcon1");
  valid_bits = PMCON1_VALID_BITS;
}

unsigned int PMDATA::get(void)
{
  trace.raw(read_trace.get() | value.get());
  return(value.get());
}

void PMDATA::put(unsigned int new_value)
{
  trace.raw(write_trace.get() | value.get());
  value.put(new_value);
}

PMDATA::PMDATA(void) {}


unsigned int PMADR::get(void)
{
  trace.raw(read_trace.get() | value.get());

  return(value.get());
}

void PMADR::put(unsigned int new_value)
{

  trace.raw(write_trace.get() | value.get());
  value.put(new_value);

}


PMADR::PMADR(void) {}

// ----------------------------------------------------------

PM_RD::PM_RD(void)
{
  cpu = NULL;
  pmcon1.set_pm(this);

  pmadr.new_name("pmadr");
  pmadrh.new_name("pmadrh");
  pmdata.new_name("pmdata");
  pmdath.new_name("pmdath");
}

void PM_RD::start_read(void)
{
  rd_adr = pmadr.value.get() | (pmadrh.value.get() << 8);

  get_cycles().set_break(get_cycles().value + READ_CYCLES, this);
}

void PM_RD::callback(void)
{
  // read program memory
  if(pmcon1.value.get() & PMCON1::RD) {
    int opcode = cpu->pma->get_opcode(rd_adr);
    pmdata.value.put(opcode & 0xff);
    pmdath.value.put((opcode>>8) & 0xff);
    pmcon1.value.put(pmcon1.value.get() & (~PMCON1::RD));
  }
}

