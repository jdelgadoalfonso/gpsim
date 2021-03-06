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
along with gpasm; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

//
// symbol.h
//

#include <iostream>
#include <string>
#include <list>
#include "gpsim_classes.h"
#include "value.h"

using namespace std;

#ifndef __SYMBOL_H__
#define __SYMBOL_H__

class Processor;

int load_symbol_file(Processor **, const char *);
void display_symbol_file_error(int);

//#include "gpsim_interface.h"
enum SYMBOL_TYPE
{
  SYMBOL_INVALID,
  SYMBOL_BASE_CLASS,
  SYMBOL_IOPORT,
  SYMBOL_STIMULUS_NODE,
  SYMBOL_STIMULUS,
  SYMBOL_LINE_NUMBER,
  SYMBOL_CONSTANT,
  SYMBOL_REGISTER,
  SYMBOL_ADDRESS,
  SYMBOL_SPECIAL_REGISTER,   // like W
  SYMBOL_PROCESSOR,
  SYMBOL_MODULE
};


class stimulus;
class Stimulus_Node;
class WREG;
class IOPORT;
class Processor;
class Register;
class Module;

class symbol : public gpsimValue
{
public:

  Module *cpu;

  virtual SYMBOL_TYPE isa(void) { return SYMBOL_BASE_CLASS;};
  virtual char * type_name(void) { return "unknown";}

  virtual string toString();
  virtual void print();
  virtual void put_value(unsigned int new_value) {}
  virtual unsigned int get_value() { return 0; }

  symbol(void);
  virtual ~symbol();
};

class Symbol_Table
{
public:
  void add(symbol*);

  void add_ioport(Processor *cpu, IOPORT *ioport);
  void add_stimulus_node(Stimulus_Node *stimulus_node);
  void add_stimulus(stimulus *s);
  void add_line_number(Processor *cpu, int address, char *symbol_name=0);
  void add_constant(Processor *cpu, char *, int );
  void add_register(Processor *cpu, Register *reg, char *symbol_name=0);
  void add_address(Processor *cpu, char *, int );
  void add_w(Processor *cpu, WREG *w );
  void add_module(Module * m, const char *module_name);
  void remove_module(Module * m);
  void add(Processor *cpu, char *symbol_name, char *symbol_type, int value);
  void dump_all(void);
  void dump_one(char *s);
  void dump_one(string *s);
  void dump_type(SYMBOL_TYPE symt);
  
  symbol * find(char *s);
  symbol * find(string *s);
  symbol * find(SYMBOL_TYPE symt, char *s);
};

#ifdef IN_MODULE
// we are in a module: don't access symbol_table object directly!
Symbol_Table &get_symbol_table(void);
#else
// we are in gpsim: use of get_symbol_table() is recommended,
// even if trace object can be accessed directly.
extern Symbol_Table symbol_table;

inline Symbol_Table &get_symbol_table(void)
{
  return symbol_table;
}
#endif



class constant_symbol : public symbol
{
public:

  int val;
  virtual SYMBOL_TYPE isa(void) { return SYMBOL_CONSTANT;};
  virtual char * type_name(void) { return "constant";}
  virtual void print(void);
  virtual unsigned int get_value(void){return val;};
};


class ioport_symbol : public symbol
{
public:

  IOPORT *ioport;
  virtual SYMBOL_TYPE isa(void) { return SYMBOL_IOPORT;};
  virtual char * type_name(void) { return "ioport";}
  virtual void put_value(unsigned int new_value);
};

class node_symbol : public symbol
{
public:

  Stimulus_Node *stimulus_node;
  virtual SYMBOL_TYPE isa(void) { return SYMBOL_STIMULUS_NODE;};
  virtual char * type_name(void) { return "node";}
  virtual void print(void);

};

class register_symbol : public symbol
{
public:

  Register *reg;
  virtual SYMBOL_TYPE isa(void) { return SYMBOL_REGISTER;};
  virtual char * type_name(void) { return "register";}
  virtual void print(void);
  virtual unsigned int get_value(void);
  virtual void put_value(unsigned int new_value);
};

class stimulus_symbol : public symbol
{
public:

  stimulus *s;
  virtual string &name(void);
  virtual SYMBOL_TYPE isa(void) { return SYMBOL_STIMULUS;};
};

class address_symbol : public symbol
{
public:
  int val;

  virtual SYMBOL_TYPE isa(void) { return SYMBOL_ADDRESS;};
  virtual char * type_name(void) { return "address";}
  virtual void print(void);
  virtual unsigned int get_value(void){return val;};

};

class line_number_symbol : public symbol
{
 public:

  int address,src_id,src_line,lst_id,lst_line,lst_page;

  void put_address(int new_address) {address = new_address;}
  void put_src_line(int new_src_line) {src_line = new_src_line;}
  void put_lst_line(int new_lst_line) {lst_line = new_lst_line;}
  void put_lst_page(int new_lst_page) {lst_page = new_lst_page;}

  virtual SYMBOL_TYPE isa(void) { return SYMBOL_LINE_NUMBER;};
  virtual char * type_name(void) { return "line_number";}
};

class module_symbol : public symbol
{
 public:
  virtual void print(void);      
  virtual SYMBOL_TYPE isa(void) { return SYMBOL_MODULE;};
  virtual char * type_name(void) { return "module";}

};

// Place W into the symbol table
class w_symbol : public symbol
{
 public:

  WREG *w;

  virtual SYMBOL_TYPE isa(void) { return SYMBOL_SPECIAL_REGISTER;};
  virtual char * type_name(void) { return "W";}

  virtual void print(void);
  virtual unsigned int get_value(void);
  virtual void put_value(unsigned int new_value);

};

class val_symbol : public symbol
{
 public:

  val_symbol(gpsimValue *);

  gpsimValue *val;

  virtual SYMBOL_TYPE isa(void) { return SYMBOL_SPECIAL_REGISTER;}
  virtual char * type_name(void);

  virtual string toString();
  virtual void print(void);
  virtual unsigned int get_value(void);
  virtual void put_value(unsigned int new_value);
  virtual string &name(void);

};

#endif  //  __SYMBOL_H__
