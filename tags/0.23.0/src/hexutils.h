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
public:
  IntelHexProgramFileType();
  int readihex16 (Processor *pProcessor, FILE * file);
  inline void writeihex16(Register **fr, gint32 size, FILE *file, gint32 offset)
	{ writeihexN(2, fr, size, file, offset); }
  inline void writeihex8(Register **fr, gint32 size, FILE *file, gint32 offset)
	{ writeihexN(1, fr, size, file, offset); }

  inline int readihex16(Register **fr, gint32 size, FILE *file, gint32 offset)
	{ return readihexN(2, fr, size, file, offset); }
  inline int readihex8(Register **fr, gint32 size, FILE *file, gint32 offset)
	{ return readihexN(1, fr, size, file, offset); }


  // ProgramFileType overrides
  virtual int  LoadProgramFile(Processor **pProcessor, const char *pFilename,
                               FILE *pFile, const char *pProcessorName);
private:
  unsigned char checksum;
  bool isBigEndian;

  int           getachar (FILE * file);
  unsigned char getbyte  (FILE * file);
  unsigned int  getword  (FILE *file);
  void putachar (FILE * file, unsigned char c);
  void write_be_word(FILE * file, int w);
  void write_le_word(FILE * file, int w);
  int read_be_word(FILE * file);
  int read_le_word(FILE * file);
  // Compute checksum for extended address record
  inline int ext_csum(gint32 add) { 
	return ((-(6 + (add & 0xff) + ((add >> 8) & 0xff)) & 0xff));
  }
  void writeihexN(int bytes_per_word, Register **fr, gint32 size, FILE *file, gint32 out_base);
  int readihexN (int bytes_per_word, Register **fr, gint32 size, FILE * file, gint32 offset);
  // The following do the same function of ntohs and htons
  // these save having to include networking includes
  inline int ntoh16(int w) { return isBigEndian ? w : ((w >> 8) & 0xff) | ((w & 0xff) << 8);}
  inline int hton16(int w) { return isBigEndian ? w : ((w >> 8) & 0xff) | ((w & 0xff) << 8);}

};



#endif // __HEXUTILS_H__
