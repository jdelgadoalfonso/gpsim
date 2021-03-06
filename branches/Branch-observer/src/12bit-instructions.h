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


#ifndef __12BIT_INSTRUCTIONS_H__
#define __12BIT_INSTRUCTIONS_H__

#include "pic-instructions.h"

//---------------------------------------------------------
class ADDWF : public Register_op
{
public:

  ADDWF(pic_processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);
  static instruction *construct(pic_processor *new_cpu, unsigned int new_opcode)
    {return new ADDWF(new_cpu,new_opcode);}
};

//---------------------------------------------------------

class ANDLW : public Literal_op
{

public:
  ANDLW(pic_processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);

};

//---------------------------------------------------------
class ANDWF : public Register_op
{
public:

  ANDWF(pic_processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);

};

//---------------------------------------------------------
class BCF : public Bit_op
{
public:

  BCF(pic_processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);

  static instruction *construct(pic_processor *new_cpu, unsigned int new_opcode)
    {return new BCF(new_cpu,new_opcode);}

};

//---------------------------------------------------------
class BSF : public Bit_op
{
public:

  BSF(pic_processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);

  static instruction *construct(pic_processor *new_cpu, unsigned int new_opcode)
    {return new BSF(new_cpu,new_opcode);}

};

//---------------------------------------------------------
class BTFSC : public Bit_op
{
public:

  BTFSC(pic_processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);

  static instruction *construct(pic_processor *new_cpu, unsigned int new_opcode)
    {return new BTFSC(new_cpu,new_opcode);}

};

//---------------------------------------------------------
class BTFSS : public Bit_op
{
public:

  BTFSS(pic_processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);

  static instruction *construct(pic_processor *new_cpu, unsigned int new_opcode)
    {return new BTFSS(new_cpu,new_opcode);}

};

//---------------------------------------------------------
class CALL : public instruction
{
public:
  unsigned int destination;

  CALL(pic_processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);
  char *name(char *str);
};

//---------------------------------------------------------
class CLRF : public Register_op
{
public:

  CLRF(pic_processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);
  char *name(char *str);
  static instruction *construct(pic_processor *new_cpu, unsigned int new_opcode)
    {return new CLRF(new_cpu,new_opcode);}
};

//---------------------------------------------------------
class CLRW : public instruction
{
public:

  CLRW(pic_processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);

};

//---------------------------------------------------------
class CLRWDT : public instruction
{
public:

  CLRWDT(pic_processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);
  static instruction *construct(pic_processor *new_cpu, unsigned int new_opcode)
    {return new CLRWDT(new_cpu,new_opcode);}

};

//---------------------------------------------------------
class COMF : public Register_op
{
public:

  COMF(pic_processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);
  static instruction *construct(pic_processor *new_cpu, unsigned int new_opcode)
    {return new COMF(new_cpu,new_opcode);}

};

//---------------------------------------------------------
class DECF : public Register_op
{
public:

  DECF(pic_processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);

};

//---------------------------------------------------------
class DECFSZ : public Register_op
{
public:

  DECFSZ(pic_processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);

};

//---------------------------------------------------------
class GOTO : public instruction
{
public:
  unsigned int destination;

  GOTO(pic_processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);
  char *name(char *str);
};

//---------------------------------------------------------
class INCF : public Register_op
{
public:

  INCF(pic_processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);

};

//---------------------------------------------------------
class INCFSZ : public Register_op
{
public:

  INCFSZ(pic_processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);

};

//---------------------------------------------------------

class IORLW : public Literal_op
{

public:
  IORLW(pic_processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);

};

//---------------------------------------------------------
class IORWF : public Register_op
{
public:

  IORWF(pic_processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);

};

//---------------------------------------------------------
class MOVF : public Register_op
{
public:

  MOVF(pic_processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);
  virtual void debug(void);

};

//---------------------------------------------------------

class MOVLW : public Literal_op
{
public:
  MOVLW(pic_processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);
  static instruction *construct(pic_processor *new_cpu, unsigned int new_opcode)
    {return new MOVLW(new_cpu,new_opcode);}

};

//---------------------------------------------------------
class MOVWF : public Register_op
{
public:

  MOVWF(pic_processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);
  char *name(char *str);
};

//---------------------------------------------------------
class NOP : public instruction
{
public:

  NOP(pic_processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);
  static instruction *construct(pic_processor *new_cpu, unsigned int new_opcode)
    {return new NOP(new_cpu,new_opcode);}

};

//---------------------------------------------------------
class OPTION : public instruction
{
public:

  OPTION(pic_processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);

};

//---------------------------------------------------------

class RETLW : public Literal_op
{
public:

  RETLW(pic_processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);
  static instruction *construct(pic_processor *new_cpu, unsigned int new_opcode)
    {return new RETLW(new_cpu,new_opcode);}

};

//---------------------------------------------------------
class RLF : public Register_op
{
public:

  RLF(pic_processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);

};

//---------------------------------------------------------
class RRF : public Register_op
{
public:

  RRF(pic_processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);

};


//---------------------------------------------------------
class SLEEP : public instruction
{
public:

  SLEEP(pic_processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);

};

//---------------------------------------------------------
class SUBWF : public Register_op
{
public:

  SUBWF(pic_processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);

};


//---------------------------------------------------------
class SWAPF : public Register_op
{
public:

  SWAPF(pic_processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);

  static instruction *construct(pic_processor *new_cpu, unsigned int new_opcode)
    {return new SWAPF(new_cpu,new_opcode);}

};


//---------------------------------------------------------
class TRIS : public Register_op
{
public:
  file_register *reg;

  TRIS(pic_processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);
  char *name(char *str);
};

//---------------------------------------------------------

class XORLW : public Literal_op
{

public:

  XORLW(pic_processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);

};

//---------------------------------------------------------
class XORWF : public Register_op
{
public:

  XORWF(pic_processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);

};


#endif  /*  __12BIT_INSTRUCTIONS_H__ */
