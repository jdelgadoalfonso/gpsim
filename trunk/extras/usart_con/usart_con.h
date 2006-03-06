/*
   Copyright (C) 1998-2006 T. Scott Dattalo

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


#ifndef __USART_CON_H__
#define __USART_CON_H__

/* IN_MODULE should be defined for modules */
#define IN_MODULE

#include <gpsim/modules.h>


class RCREG;
class RxBaudRateAttribute;
class RxBuffer;

class USARTModule : public Module
{
 public:
  void CreateGraphics(void);

  // Inheritances from the Package class
  virtual void create_iopin_map();

  USARTModule(const char *new_name);
  ~USARTModule();

  static Module *USART_construct(const char *new_name=NULL);

  virtual void new_rx_edge(unsigned int);
  virtual void newRxByte(unsigned int);
  virtual void get(char *, int len);
private:
  RxBaudRateAttribute *m_RxBaud;

  RxBuffer *m_RxBuffer;

  RCREG *m_rcreg;
};
#endif //  __USART_CON_H__
