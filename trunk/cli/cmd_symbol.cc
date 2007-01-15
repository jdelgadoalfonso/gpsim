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
#include "cmd_symbol.h"
#include "../src/cmd_gpsim.h"
#include "../src/symbol.h"
#include "../src/symbol_orb.h"
#include "../src/ValueCollections.h"

cmd_symbol c_symbol;

static cmd_options cmd_symbol_options[] =
{
  {0,0,0}
};


cmd_symbol::cmd_symbol()
  : command("symbol",0)
{ 

  brief_doc = string("Add or display symbols");

  long_doc = string ("symbol [<symbol_name>]\n"
    "symbol <symbol_name>=<value>\n"
    "\n"
    "\tIf no options are supplied, the entire symbol table will be\n"
    "\tdisplayed. If only the symbol_name is provided, then only\n"
    "\tthat symbol will be displayed.\n"
    "\tIf a symbol_name that does not currently exist is equated\n"
    "\tto a value, then a new symbol will be added to the symbol table.\n"
    "\tThe type of symbol will be derived. To force a string value double\n"
    "\tdouble quote the value.\n"
    "\n"
    "\tValid symbol types:\n"
    "\t  Integer, Float, Boolean and String\n"
    "\n"
    "Examples:\n"
    "\tsymbol                     // display the symbol table\n"
    "\tsymbol GpsimIsGreat=true   // create a new constant symbol\n"
    "\n");

  op = cmd_symbol_options; 
}


void cmd_symbol::dump_all(void)
{
  get_symbol_table().dump_all();
}

void cmd_symbol::dump_one(const char *sym_name)
{
  string sName(sym_name);
  get_symbol_table().dump_filtered(sName);
}

void cmd_symbol::dump_one(Value *s)
{
  if(s)
    cout << s->toString() << endl;
}

void cmd_symbol::add_one(const char *sym_name, Expression *expr)
{
  Value * pVal = expr->evaluate();
  if (pVal) {
    pVal->new_name(sym_name);
    pVal->setClearableSymbol(false);
    pVal->set_description("Derived from the command line.");
    if (!get_symbol_table().add(pVal))
      delete pVal;
  }
}

void cmd_symbol::EvaluateAndDisplay(Expression *pExpr) {
  try {
    Value * pValue = pExpr->evaluate();
    GetUserInterface().DisplayMessage("%s\n", pValue->toString().c_str());
  }
  catch(Error *pMessage)  {
    GetUserInterface().DisplayMessage("%s\n", pMessage->toString().c_str());
  }
}

void cmd_symbol::dump(Value *s, ExprList_t*e) {
  IndexedSymbol sym(s, e);
  cout << sym.toString() << endl;
}

void cmd_symbol::Set(Value *s, ExprList_t*e, Expression *pExpr) {
  try {
    IIndexedCollection *pCollection = dynamic_cast<IIndexedCollection*>(s);
    if(pCollection == NULL) {
      GetUserInterface().DisplayMessage("%s is not an indexed symbol\n",
        s->name().c_str());
    }
    else {
      pCollection->SetAt(e, pExpr);
    }
  }
  catch(Error Message)  {
    GetUserInterface().DisplayMessage("%s\n", Message.toString().c_str());
  }
}
