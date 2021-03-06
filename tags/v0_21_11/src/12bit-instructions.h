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


#ifndef __12BIT_INSTRUCTIONS_H__
#define __12BIT_INSTRUCTIONS_H__

#include "pic-instructions.h"

//---------------------------------------------------------
class ADDWF : public Register_op
{
public:

  ADDWF(Processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);
  static instruction *construct(Processor *new_cpu, unsigned int new_opcode)
    {return new ADDWF(new_cpu,new_opcode);}
};

//---------------------------------------------------------

class ANDLW : public Literal_op
{

public:
  ANDLW(Processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);
  static instruction *construct(Processor *new_cpu, unsigned int new_opcode)
    {return new ANDLW(new_cpu,new_opcode);}

};

//---------------------------------------------------------
class ANDWF : public Register_op
{
public:

  ANDWF(Processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);
  static instruction *construct(Processor *new_cpu, unsigned int new_opcode)
    {return new ANDWF(new_cpu,new_opcode);}

};

//---------------------------------------------------------
class BCF : public Bit_op
{
public:

  BCF(Processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);

  static instruction *construct(Processor *new_cpu, unsigned int new_opcode)
    {return new BCF(new_cpu,new_opcode);}

};

//---------------------------------------------------------
class BSF : public Bit_op
{
public:

  BSF(Processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);

  static instruction *construct(Processor *new_cpu, unsigned int new_opcode)
    {return new BSF(new_cpu,new_opcode);}

};

//---------------------------------------------------------
class BTFSC : public Bit_op
{
public:

  BTFSC(Processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);

  static instruction *construct(Processor *new_cpu, unsigned int new_opcode)
    {return new BTFSC(new_cpu,new_opcode);}

};

//---------------------------------------------------------
class BTFSS : public Bit_op
{
public:

  BTFSS(Processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);

  static instruction *construct(Processor *new_cpu, unsigned int new_opcode)
    {return new BTFSS(new_cpu,new_opcode);}

};

//---------------------------------------------------------
class CALL : public instruction
{
public:
  unsigned int destination;

  CALL(Processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);
  virtual char *name(char *str,int len);
  virtual bool isBase() { return true;}

  static instruction *construct(Processor *new_cpu, unsigned int new_opcode)
    {return new CALL(new_cpu,new_opcode);}
};

//---------------------------------------------------------
class CLRF : public Register_op
{
public:

  CLRF(Processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);
  virtual char *name(char *str,int len);
  static instruction *construct(Processor *new_cpu, unsigned int new_opcode)
    {return new CLRF(new_cpu,new_opcode);}
};

//---------------------------------------------------------
class CLRW : public instruction
{
public:

  CLRW(Processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);
  virtual bool isBase() { return true;}
  static instruction *construct(Processor *new_cpu, unsigned int new_opcode)
    {return new CLRW(new_cpu,new_opcode);}

};

//---------------------------------------------------------
class CLRWDT : public instruction
{
public:

  CLRWDT(Processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);
  virtual bool isBase() { return true;}
  static instruction *construct(Processor *new_cpu, unsigned int new_opcode)
    {return new CLRWDT(new_cpu,new_opcode);}

};

//---------------------------------------------------------
class COMF : public Register_op
{
public:

  COMF(Processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);
  static instruction *construct(Processor *new_cpu, unsigned int new_opcode)
    {return new COMF(new_cpu,new_opcode);}

};

//---------------------------------------------------------
class DECF : public Register_op
{
public:

  DECF(Processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);
  static instruction *construct(Processor *new_cpu, unsigned int new_opcode)
    {return new DECF(new_cpu,new_opcode);}

};

//---------------------------------------------------------
class DECFSZ : public Register_op
{
public:

  DECFSZ(Processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);
  static instruction *construct(Processor *new_cpu, unsigned int new_opcode)
    {return new DECFSZ(new_cpu,new_opcode);}

};

//---------------------------------------------------------
class GOTO : public instruction
{
public:
  unsigned int destination;

  GOTO(Processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);
  virtual bool isBase() { return true;}
  virtual char *name(char *str,int len);
  static instruction *construct(Processor *new_cpu, unsigned int new_opcode)
    {return new GOTO(new_cpu,new_opcode);}

};

//---------------------------------------------------------
class INCF : public Register_op
{
public:

  INCF(Processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);
  static instruction *construct(Processor *new_cpu, unsigned int new_opcode)
    {return new INCF(new_cpu,new_opcode);}

};

//---------------------------------------------------------
class INCFSZ : public Register_op
{
public:

  INCFSZ(Processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);
  static instruction *construct(Processor *new_cpu, unsigned int new_opcode)
    {return new INCFSZ(new_cpu,new_opcode);}

};

//---------------------------------------------------------

class IORLW : public Literal_op
{

public:
  IORLW(Processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);
  static instruction *construct(Processor *new_cpu, unsigned int new_opcode)
    {return new IORLW(new_cpu,new_opcode);}

};

//---------------------------------------------------------
class IORWF : public Register_op
{
public:

  IORWF(Processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);
  static instruction *construct(Processor *new_cpu, unsigned int new_opcode)
    {return new IORWF(new_cpu,new_opcode);}

};

//---------------------------------------------------------
class MOVF : public Register_op
{
public:

  MOVF(Processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);
  virtual void debug(void);
  static instruction *construct(Processor *new_cpu, unsigned int new_opcode)
    {return new MOVF(new_cpu,new_opcode);}

};

//---------------------------------------------------------

class MOVLW : public Literal_op
{
public:
  MOVLW(Processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);
  static instruction *construct(Processor *new_cpu, unsigned int new_opcode)
    {return new MOVLW(new_cpu,new_opcode);}

};

//---------------------------------------------------------
class MOVWF : public Register_op
{
public:

  MOVWF(Processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);
  virtual char *name(char *str,int len);
  static instruction *construct(Processor *new_cpu, unsigned int new_opcode)
    {return new MOVWF(new_cpu,new_opcode);}

};

//---------------------------------------------------------
class NOP : public instruction
{
public:

  NOP(Processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);
  virtual bool isBase() { return true;}
  static instruction *construct(Processor *new_cpu, unsigned int new_opcode)
    {return new NOP(new_cpu,new_opcode);}

};

//---------------------------------------------------------
class OPTION : public instruction
{
public:

  OPTION(Processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);
  virtual bool isBase() { return true;}
  static instruction *construct(Processor *new_cpu, unsigned int new_opcode)
    {return new OPTION(new_cpu,new_opcode);}

};

//---------------------------------------------------------

class RETLW : public Literal_op
{
public:

  RETLW(Processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);
  static instruction *construct(Processor *new_cpu, unsigned int new_opcode)
    {return new RETLW(new_cpu,new_opcode);}

};

//---------------------------------------------------------
class RLF : public Register_op
{
public:

  RLF(Processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);
  static instruction *construct(Processor *new_cpu, unsigned int new_opcode)
    {return new RLF(new_cpu,new_opcode);}

};

//---------------------------------------------------------
class RRF : public Register_op
{
public:

  RRF(Processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);
  static instruction *construct(Processor *new_cpu, unsigned int new_opcode)
    {return new RRF(new_cpu,new_opcode);}

};


//---------------------------------------------------------
class SLEEP : public instruction
{
public:

  SLEEP(Processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);
  virtual bool isBase() { return true;}
  static instruction *construct(Processor *new_cpu, unsigned int new_opcode)
    {return new SLEEP(new_cpu,new_opcode);}

};

//---------------------------------------------------------
class SUBWF : public Register_op
{
public:

  SUBWF(Processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);
  static instruction *construct(Processor *new_cpu, unsigned int new_opcode)
    {return new SUBWF(new_cpu,new_opcode);}

};


//---------------------------------------------------------
class SWAPF : public Register_op
{
public:

  SWAPF(Processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);

  static instruction *construct(Processor *new_cpu, unsigned int new_opcode)
    {return new SWAPF(new_cpu,new_opcode);}

};


//---------------------------------------------------------
class TRIS : public Register_op
{
public:
  Register *reg;

  TRIS(Processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);
  virtual char *name(char *str,int len);
  static instruction *construct(Processor *new_cpu, unsigned int new_opcode)
    {return new TRIS(new_cpu,new_opcode);}

};

//---------------------------------------------------------

class XORLW : public Literal_op
{

public:

  XORLW(Processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);
  static instruction *construct(Processor *new_cpu, unsigned int new_opcode)
    {return new XORLW(new_cpu,new_opcode);}

};

//---------------------------------------------------------
class XORWF : public Register_op
{
public:

  XORWF(Processor *new_cpu, unsigned int new_opcode);
  virtual void execute(void);
  static instruction *construct(Processor *new_cpu, unsigned int new_opcode)
    {return new XORWF(new_cpu,new_opcode);}

};


#endif  /*  __12BIT_INSTRUCTIONS_H__ */
