/*
   Copyright (C) 2001 Salvador E. Tropea

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


#include <iostream.h>
#include <iomanip.h>
#include <string>
#include <vector>

#include "command.h"
#include "cmd_frequency.h"
#include "../src/pic-processor.h"

static cmd_options cmd_frequency_options[] =
{
  NULL,0,0
};

cmd_frequency::cmd_frequency(void)
{ 
  name = "frequency";

  brief_doc = string("Set the clock frequency");

  long_doc = string ("\nfrequency [value]\n\
\tThis command sets the clock frequency. By default gpsim uses 4 MHz
\tas clock. The clock frequency is used to compute time in seconds.
\tUse this command to adjust this value.
\tIf no value is provided this command prints the current clock.
\tNote that PICs have an instruction clock that's a forth of the
\texternal clock. This value is the external clock.
\n");

  op = cmd_frequency_options;
}


void cmd_frequency::set(double value)
{

  if(!have_cpu(1))
    return;

  if(value<0) {
    cout << "Error: the clock must be a positive value.\n";
    return;
  }
  cpu->set_frequency(value);
}

void cmd_frequency::print()
{
  if(!have_cpu(1))
    return;

  cout << "Clock frequency: " << cpu->time_to_cycles(1.0)/1e6 << " MHz.\n";
}

cmd_frequency frequency;
