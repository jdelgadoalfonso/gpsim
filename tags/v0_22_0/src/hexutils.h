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

#if !defined(__HEXUTILS_H__)
#define __HEXUTILS_H__

#include "program_files.h"


/*
 *  IntelHexProgramFileType
 *  Note that the code is in hexutils.cc
 *
 */

class IntelHexProgramFileType : public ProgramFileType {
private:
  unsigned char checksum;

  int           readihex16 (Processor **pProcessor, FILE * file);
  int           getachar (FILE * file);
  unsigned char getbyte  (FILE * file);
  unsigned int  getword  (FILE *file);

public:
  IntelHexProgramFileType();

  // ProgramFileType overrides
  virtual int  LoadProgramFile(Processor **pProcessor, const char *pFilename,
                               FILE *pFile, const char *pProcessorName);
};



#endif // __HEXUTILS_H__
