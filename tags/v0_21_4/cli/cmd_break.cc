/*
   Copyright (C) 1999 T. Scott Dattalo

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


#include <iostream>
#include <iomanip>
#include <string>

#include "command.h"
#include "cmd_break.h"

#include "../src/pic-processor.h"
#include "../src/symbol_orb.h"
#include "../src/operator.h"

cmd_break c_break;

#define CYCLE       1
#define EXECUTION   2
#define WRITE       3
#define READ        4
#define STK_OVERFLOW  7
#define STK_UNDERFLOW 8
#define WDT           9

static cmd_options cmd_break_options[] =
{
  {"c",   CYCLE,        OPT_TT_BITFLAG},
  {"e",   EXECUTION,    OPT_TT_BITFLAG},
  {"w",   WRITE,        OPT_TT_BITFLAG},
  {"r",   READ,         OPT_TT_BITFLAG},
  {"so",  STK_OVERFLOW, OPT_TT_BITFLAG},
  {"su",  STK_UNDERFLOW,OPT_TT_BITFLAG},
  {"wdt", WDT,          OPT_TT_BITFLAG},
  {0,0,0}
};


cmd_break::cmd_break(void)
{ 
  name = "break";

  brief_doc = string("Set a break point");

  long_doc = string ("break [c | e addr | w [break_expr] | r [break_expr] | so | su | wdt ] \n"
    "\n"
    "  options:\n"
    "\tc          - cycle\n"
    "\te          - execution\n"
    "\t             addr - address value or symbol\n"
    "\tw          - write\n"
    "\tr          - read\n"
    "\tbreak_expr - r [& m] == v\n"
    "\t             r - register address or symbol\n"
    "\t             m - bit mask value to mask value in the register\n"
    "\t             v - integer value on which to break\n"
    "\tso         - stack over flow\n"
    "\tsu         - stack under flow\n"
    "\twdt        - wdt timeout\n"
    "\t           - no argument, display the break points that are set.\n"
    "  Examples:\n"
    "\tbreak e 0x20       // set an execution break point at address 0x20\n"
    "\tbreak w 8 == 0     // break if a zero is written to register 8\n"
    "\tbreak w 9 & 0x30 == 0xf0 // break if '3' is written to the upper nibble or reg 9\n"
    "\tbreak c 1000000    // break on the one million'th cycle\n"
    "\tbreak              // display all of the break points\n"
    "\n");

  op = cmd_break_options; 
}


void cmd_break::list(void)
{
  if(cpu)
    bp.dump();
}

const char *TOO_FEW_ARGS="missing register or location\n";
const char *TOO_MANY_ARGS="too many arguments\n";

void cmd_break::set_break(cmd_options *co, Value *pValue)
{
  Integer * pAddress = dynamic_cast<Integer*>(pValue);
  if (pAddress != NULL) { 
    gint64 iAddress;
    pAddress->get(iAddress);
    set_break(co->value, iAddress);
    return;
  }
  register_symbol* pRegSymbol = dynamic_cast<register_symbol*>(pValue);
  if (pRegSymbol) {
    set_break(co->value, pRegSymbol->getReg()->address);
    return;
  }
  printf("Syntax error: param %s does not represent a memory address\n",
    pValue->name().c_str());
}

void cmd_break::set_break(cmd_options *co, Expression *pExpr)
{

  if (!co) {
    list();
    return;
  }

  int bit_flag = co->value;
  if (!pExpr) {
    set_break(bit_flag);
    return;
  }
  ComparisonOperator *pCompareExpr = dynamic_cast<ComparisonOperator *>(pExpr);
  if (pCompareExpr != NULL) {
     
    Register * pReg = NULL;
    int  uMask = cpu->register_mask();
    LiteralSymbol* pLeftSymbol = dynamic_cast<LiteralSymbol*>(pCompareExpr->getLeft());
    if (pLeftSymbol != NULL) {
      register_symbol *pRegSym = dynamic_cast<register_symbol*>(pLeftSymbol->evaluate());
      pReg = pRegSym->getReg();
      delete pRegSym;
    }
    else {
      OpAnd* pLeftOp = dynamic_cast<OpAnd*>(pCompareExpr->getLeft());
      if (pLeftOp != NULL) {
        pLeftSymbol = dynamic_cast<LiteralSymbol*>(pLeftOp->getLeft());
        register_symbol *pRegSym = dynamic_cast<register_symbol*>(pLeftSymbol->evaluate());
        pReg = pRegSym->getReg();

        LiteralSymbol* pRightSymbol = dynamic_cast<LiteralSymbol*>(pLeftOp->getRight());
        Integer *pInteger = dynamic_cast<Integer*>(pRightSymbol->evaluate());
        gint64 i64;
        pInteger->get(i64);
        uMask = (int)i64;
        delete pRegSym;
        delete pInteger;
      }
    }
    if (pReg != NULL) {
      LiteralInteger* pInteger = dynamic_cast<LiteralInteger*>((LiteralInteger*)pCompareExpr->getRight());
      if (pInteger != NULL) {
        int uValue;
        Value *pInt = pInteger->evaluate();
        pInt->get(uValue);
        delete pInt;
        set_break(bit_flag, pReg->address, uValue, uMask);
      }
      else {
        cout << pCompareExpr->show() << " of type " << pCompareExpr->showType() <<
          " not allowed\n";
      }
    }
    else {
      cout << pCompareExpr->getLeft()->show() << " of type " << pCompareExpr->getLeft()->showType() <<
        " not allowed\n";
    }
  }
  else {
    cout << pExpr->show() << " of type " << pExpr->showType() <<
      " not allowed\n";
  }
  delete pExpr;
}

void cmd_break::set_break(cmd_options *co)
{

  if (!co) {
    list();
    return;
  }

  int bit_flag = co->value;
  set_break(bit_flag);
}

void cmd_break::set_break(Value *v)
{
  if(v)
    v->set_break();
}

void cmd_break::set_break(int bit_flag)
{

  if(!cpu)
    return;

  int b;

  switch(bit_flag) {

  case STK_OVERFLOW:
    b = bp.set_stk_overflow_break(cpu);

    if(b < MAX_BREAKPOINTS)
      cout << "break when stack over flows.  " <<
        "bp#: " << b << '\n';

    break;

  case STK_UNDERFLOW:
    b = bp.set_stk_underflow_break(cpu);

    if(b < MAX_BREAKPOINTS)
      cout << "break when stack under flows.  " <<
        "bp#: " << b << '\n';

    break;


  case WDT:
    b = bp.set_wdt_break(cpu);

    if(b < MAX_BREAKPOINTS)
      cout << "break when wdt times out.  " <<
        "bp#: " << b << '\n';

    break;

  default:
    cout << TOO_FEW_ARGS;
    break;
  }
}


void cmd_break::set_break(int bit_flag, guint64 v)
{

  if(!cpu)
    return;

  int b;
  unsigned int value = (unsigned int)v;

  switch(bit_flag) {

  case CYCLE:
    b = bp.set_cycle_break(cpu,value);

    if(b < MAX_BREAKPOINTS)
      cout << "break at cycle: " << value << " break #: " <<  b << '\n';
    else
      cout << "failed to set cycle break\n";

    break;

  case EXECUTION:
    b = bp.set_execution_break(cpu, value);
    if(b < MAX_BREAKPOINTS) {
	    cout << "break at address: " << value << " break #: " << b << '\n';
    }
    else
      cout << "failed to set execution break (check the address)\n";

    break;

  case WRITE:

    b = bp.set_write_break(cpu, value);
    if(b < MAX_BREAKPOINTS)
      cout << "break when register " << value << " is written. bp#: " << b << '\n';

    break;

  case READ:
    b = bp.set_read_break(cpu, value);
    if(b < MAX_BREAKPOINTS)
      cout << "break when register " << value << " is read.\n" << 
	      "bp#: " << b << '\n';

    break;

  case STK_OVERFLOW:
  case STK_UNDERFLOW:
  case WDT:
    cout << TOO_MANY_ARGS;
  }
}

void cmd_break::set_break(int bit_flag,
			  guint64 r,
			  guint64 v,
			  guint64 m)
{

  if(!cpu)
    return;

  int b = MAX_BREAKPOINTS;
  const char *str = "err";
  unsigned int reg = (unsigned int)r;
  unsigned int value = (unsigned int)v;
  unsigned int mask = (unsigned int)m;

  switch(bit_flag) {

  case CYCLE:
  case EXECUTION:
  case STK_OVERFLOW:
  case STK_UNDERFLOW:
  case WDT:
    cout << TOO_MANY_ARGS;
    break;

  case READ:
    b = bp.set_read_value_break(cpu, reg,value,mask);
    str = "read from";
    break;

  case WRITE:
    b = bp.set_write_value_break(cpu, reg,value,mask);
    str = "written to";
    break;
  }

  if(b<MAX_BREAKPOINTS) {
    cout << "break when ";
    if(mask == 0 || mask == 0xff)
      cout << (value&0xff);
    else {
      cout << "bit pattern ";
      for(unsigned int ui=0x80; ui; ui>>=1) {
        if(ui & mask) {
          if(ui & value)
            cout << '1';
          else
            cout << '0';
        }
        else {
	        cout << 'X';
        }
      }
    }
    cout << " is " << str <<" register " << reg << '\n' << 
	    "bp#: " << b << '\n';
  }
}

