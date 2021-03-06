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

#include <string>
#include "14bit-processors.h"


#ifndef __SYMBOL_H__
#define __SYMBOL_H__

int load_symbol_file(pic_processor **, char *);
void display_symbol_file_error(int);

#include "interface.h"
/*enum SYMBOL_TYPE
{
  SYMBOL_BASE_CLASS,
  SYMBOL_IOPORT,
  SYMBOL_STIMULUS_NODE,
  SYMBOL_STIMULUS,
  SYMBOL_LINE_NUMBER,
  SYMBOL_CONSTANT,
  SYMBOL_REGISTER,
  SYMBOL_ADDRESS
};*/


class symbol;
class Stimulus_Node;
class stimulus;

class symbol_type
{
public:
  SYMBOL_TYPE type;
  char * name_str;
};

extern symbol_type symbol_types[];

class Symbol_Table
{
public:

  void add_ioport(pic_processor *cpu, IOPORT *ioport);
  void add_stimulus_node(Stimulus_Node *stimulus_node);
  void add_stimulus(stimulus *s);
  void add_line_number(pic_processor *cpu, int address);
  void add_constant(pic_processor *cpu, char *, int );
  void add_register(pic_processor *cpu, file_register *reg);
  void add_address(pic_processor *cpu, char *, int );
  void add(pic_processor *cpu, char *symbol_name, char *symbol_type, int value);
  void dump_all(void);
  void dump_one(char *s);
  void dump_one(string *s);
  symbol * find(char *s);
  symbol * find(string *s);

};

extern Symbol_Table symbol_table;

class symbol
{
public:

  string name_str;
  pic_processor *cpu;

  virtual SYMBOL_TYPE isa(void) { return SYMBOL_BASE_CLASS;};
  char * type_name(void) { return symbol_types[isa()].name_str;};
  string *name(void) { return &name_str;};
  void new_name(string *new_name_str) {name_str = *new_name_str;};
  virtual void print(void);
  virtual int get_value(void){return 0;};
  symbol(void);

};


class constant_symbol : public symbol
{
public:

  int val;
  virtual SYMBOL_TYPE isa(void) { return SYMBOL_CONSTANT;};
  virtual void print(void) {
    cout << *name() << " = 0x" << hex << val <<'\n';
  }
  virtual int get_value(void){return val;};
};


class ioport_symbol : public symbol
{
public:

  IOPORT *ioport;
  virtual SYMBOL_TYPE isa(void) { return SYMBOL_IOPORT;};
};

class node_symbol : public symbol
{
public:

  Stimulus_Node *stimulus_node;
  virtual SYMBOL_TYPE isa(void) { return SYMBOL_STIMULUS_NODE;};
};

class register_symbol : public symbol
{
public:

  file_register *reg;
  virtual SYMBOL_TYPE isa(void) { return SYMBOL_REGISTER;};
  virtual void print(void) {
    if(reg)
      cout << *name() << hex << " [0x" << reg->address << "] = 0x" << reg->value <<'\n';
  }
  virtual int get_value(void) {
    if(reg)
      return reg->address;
  }

};

class stimulus_symbol : public symbol
{
public:

  stimulus *s;
  virtual SYMBOL_TYPE isa(void) { return SYMBOL_STIMULUS;};
};

class address_symbol : public symbol
{
public:
  int val;

  virtual SYMBOL_TYPE isa(void) { return SYMBOL_ADDRESS;};
  virtual void print(void) {
    cout << *name() << " at address 0x" << hex << val <<'\n';
  }
  virtual int get_value(void){return val;};

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
};
#endif  //  __SYMBOL_H__
