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

#include <iostream>
#include <iomanip>
#include <assert.h>

#include <map>
#include "../config.h"
#include "pic-processor.h"
#include "14bit-processors.h"
#include "trace.h"
#include "trace_orb.h"
#include "xref.h"

#define MODE "0x" << hex

Trace trace;               /* Instantiate the trace buffer class.
			    * This is where *everything* including the
			    * kitchen sink gets stored in a trace buffer.
			    * Since everything is stored here, it gets
			    * rather difficult to post process traced info
			    * efficiently. So this buffer is primarily used
			    * to record program flow that the user may post
			    * analyze by dumping its contents.
			    */

// create an instance of inline get_trace() method by taking its address
static Trace &(*dummy_trace)(void) = get_trace;

/*Trace trace_log_buffer;   * The trace_log_buffer is a special trace
			    * buffer intended for logging events that will
			    * ultimately be written to a file. Each logged
			    * event is individually time tagged making it
			    * easy to post process.
			    */
TraceLog trace_log;
ProfileKeeper profile_keeper;

//========================================================================
traceValue::traceValue(void)
{
  
}
unsigned int traceValue::get_value(void)
{
  return trace.trace_index;
}

/****************************************************************************
 *
 *   gpsim Trace
 *
 
   General:
  
   gpsim traces almost everything simulated: instructions executed,
   register reads/writes, clock cycles, special register accesses
   break points, instruction skips, external modules, and a few
   other miscellaneous things. 

   The tracing subsystem is implemented as a C++ class. In theory,
   multiple traces could be instantiated, but (currently) there is
   one global trace instantiated and all the pieces of gpsim make
   direct references to it.

   How can gpsim trace every thing and still be the fastest
   microcontroller simulator? Well, gpsim writes trace
   information into a giant circular buffer. So one optimization
   is that there are no array bounds to check. Another optimization
   is that most of the trace operations are C++ inline functions.
   A third optimization is that the trace operations are efficiently
   encoded into 32-bit words. The one exception is the cycle counter
   trace, it takes two 32-bit words (see the cycle counter trace 
   comment below).

   The upper 8-bits of the trace word are reserved for describing
   the trace type. The upper two bits of these 8-bits are reservered
   for encoding the cycle counter. The lower 6-bits allow 64 enumerated
   types to be encoded. Only a small portion of these are currently
   used. The lower 24-bits of the 32-bit trace word store the 
   information we wish to trace. For example, for register reads and
   writes, there are 8-bits of data and (upto) 16-bits of address.


   Details

   Each trace takes exactly one 32-bit word. The upper 8-bits define
   the trace type and the lower 24 are the trace value. For example,
   a register write will get traced with a 32 bit encoded like:

      TTAAAAVV

   TT - Register write trace type
   AAAA - 4-hexdigit address
   VV - 2-hexdigit (8-bit) value
  
   The cycle counter is treated slightly differently. Since it is a
   64-bit object, it has to be split across at least two trace
   entries. The upper few bits of the cycle counter aren't
   traced. (This is a bug for simulations that run for several
   centuries!) The trace member function is_cycle_trace() describes
   the cycle counter encoding in detail.

   Trace frames.

   A trace frame is defined to be the contents of the trace buffer
   corresponding to a single time quantum. 


****************************************************************************/


/*
  Trace Logging

  gpsim supports two modes of trace logging. The first mode is the
  "log all" mode. In this mode, the entire trace buffer is periodically
  written to a file. The second mode is a "log selective". In this mode
  individual trace operations are written to a file.

*/


TraceRawLog::TraceRawLog(void)
{
  log_filename = 0;
  log_file = 0;
}

TraceRawLog::~TraceRawLog(void)
{
  if(log_file) {
    log();
    fclose(log_file);
  }

}

void TraceRawLog::log(void)
{
  if(log_file) {
    unsigned int i;
    for(i=0; i<trace.trace_index; i++)
      fprintf(log_file,"%08X\n",trace[i]);

    trace.trace_index = 0;
  }

}

void TraceRawLog::enable(char *fname)
{
  if(!fname) {
    cout << "Trace logging - invalid file name\n";
    return;
  }

  log_filename = strdup(fname);
  log_file = fopen(fname,"w");
  if(log_file) {
    trace.bLogging = true;

    cout << "Trace logging enabled to file " << fname << endl;
  } else 
    cout << "Trace logging: could not open: " << fname << endl;

}

void TraceRawLog::disable(void)
{
  log();

  if(trace.cpu)
    trace.cpu->save_state(log_file);

  if(log_filename) {
    free(log_filename);
    log_filename = 0;

  }
  fclose(log_file);
  log_file = 0;

  cout << "Trace logging disabled\n";

  trace.bLogging = false;
}


//========================================================================
// Experimental trace code

class WTraceObject : public TraceObject
{
public:
  RegisterValue from;
  RegisterValue to;

  WTraceObject(Processor *_cpu, RegisterValue trv) 
    : TraceObject(_cpu), from(trv)
  {
    to = cpu_pic->W->trace_state;
    cpu_pic->W->trace_state = from;

  }

  virtual void print(void)
  {
    fprintf(stdout, " W: was (0x%04X,0x%04X) is (0x%04X,0x%04X)\n",
	      from.data,from.init, to.data, to.init);

  }
};


//========================================================================
// TraceFrame
TraceFrame::TraceFrame( ) 
{
  cycle_time = 0;
}

TraceFrame::~TraceFrame()
{
  list <TraceObject *> :: iterator toIter;

  toIter = traceObjects.begin();
  while(toIter != traceObjects.end()) {
    delete *toIter;
    ++toIter;
  }
}

void TraceFrame::add(TraceObject *to)
{
  traceObjects.push_back(to);
}

void TraceFrame::print() 
{
  list <TraceObject *> :: iterator toIter;

  printf("=== Trace Frame 0x%016LX\n",cycle_time);
  for(toIter = traceObjects.begin();
      toIter != traceObjects.end();
      ++toIter) 
    (*toIter)->print_frame(this);
}


void Trace::addFrame(TraceFrame *newFrame)
{
  current_frame = newFrame;
  traceFrames.push_back(newFrame);
}

void Trace::addToCurrentFrame(TraceObject *to)
{

  if(current_frame)
    current_frame->add(to);

}
void Trace::deleteTraceFrame(void)
{
  list <TraceFrame *> :: iterator tfIter;

  for(tfIter = traceFrames.begin();
      tfIter != traceFrames.end();
      ++tfIter) {
    TraceFrame *tf = *tfIter;
    delete tf;
  }
  traceFrames.clear();
  current_frame = 0;
  current_cycle_time = 0;
}

void Trace::printTraceFrame(void)
{
  list <TraceFrame *> :: reverse_iterator tfIter;

  for(tfIter = traceFrames.rbegin();
      tfIter != traceFrames.rend();
      ++tfIter) {
    (*tfIter)->print();
  }
}




//========================================================================
// TraceObject
//
// Base class for decode traces.
//
TraceObject::TraceObject()
{
  throw "TraceObject";
}
TraceObject::TraceObject(Processor *_cpu)
  : cpu(_cpu) 
{
}
void TraceObject::print_frame(TraceFrame *tf)
{
  // by default, a trace object doesn't know how to print a frame
  // special trace objects derived from this class will be designated
  // printers.
}


//========================================================================
// RegisterTraceObject
//
RegisterTraceObject::RegisterTraceObject() 
  : reg(0)
{
  throw "RegisterTraceObject";
}
RegisterTraceObject::RegisterTraceObject(Processor *_cpu,
					 Register *_reg,
					 RegisterValue trv) 
  : TraceObject(_cpu), reg(_reg), from(trv)
{
  if(reg) {
    to = reg->trace_state;
    reg->trace_state = from;
  }
}
void RegisterTraceObject::print(void)
{
  if(reg)
    fprintf(stdout, "  Wrote: (0x%04X,0x%04X) to %s(0x%04X) was (0x%04X,0x%04X) \n",
	    to.data, to.init, reg->name().c_str(), reg->address, from.data,from.init);
}

//========================================================================
PCTraceObject::PCTraceObject(Processor *_cpu, unsigned int _address) 
  : TraceObject(_cpu), address(_address)
{
}

void PCTraceObject::print(void)
{
  char a_string[50];

  unsigned addr = address & 0xffff;

  fprintf(stdout,"0x%04X 0x%04X %s\n",
	  address &0xffffff,
	  cpu->pma->get_opcode(addr),
	  (*cpu->pma)[addr].name(a_string,sizeof(a_string)));


}
void PCTraceObject::print_frame(TraceFrame *tf)
{
  if(!tf)
    return;

  list <TraceObject *> :: iterator toIter;

  printf("0x%016LX %s ",tf->cycle_time,cpu->name().c_str());
  for(toIter = tf->traceObjects.begin();
      toIter != tf->traceObjects.end();
      ++toIter) 
    (*toIter)->print();

}



//========================================================================

TraceType::TraceType(unsigned int t, unsigned int s)
  : type(t), size(s)
{

}

//========================================================================

RegisterTraceType::RegisterTraceType(Processor *_cpu, 
				     unsigned int t,
				     unsigned int s)
  : ProcessorTraceType(_cpu,t,s)
{
    
}

TraceObject *RegisterTraceType::decode(unsigned int tbi)
{

  unsigned int tv = trace.get(tbi);
  RegisterValue rv = RegisterValue(tv & 0xff, 0);
  unsigned int address = (tv >> 8) & 0xfff;

  printf(" RegisterTraceType: tv=0x%x\n",tv);

  RegisterTraceObject *rto = new RegisterTraceObject(cpu, cpu->rma.get_register(address), rv);
  trace.addToCurrentFrame(rto);

  return rto;
}


//========================================================================
TraceObject * WTraceType::decode(unsigned int tbi)
{

  unsigned int tv = trace.get(tbi);
  printf(" WTraceType: 0x%x\n",tv);

  RegisterValue rv = RegisterValue(tv & 0xff,0);
  WTraceObject *wto = new WTraceObject(cpu, rv);
  trace.addToCurrentFrame(wto);

  return wto;
}

//========================================================================
PCTraceType::PCTraceType(Processor *_cpu, 
			 unsigned int t,
			 unsigned int s)
  : ProcessorTraceType(_cpu,t,s)
{
}

TraceObject *PCTraceType::decode(unsigned int tbi)
{

  unsigned int tv = trace.get(tbi);
  printf(" PCTraceType: 0x%x\n",tv);


  PCTraceObject *pcto = new PCTraceObject(cpu, tv);
  trace.addToCurrentFrame(pcto);

  if((tv & (3<<22)) == (1<<22))
    trace.current_cycle_time -= 2;
  else
    trace.current_cycle_time -= 1;

  trace.current_frame->cycle_time = trace.current_cycle_time;

  return pcto;
}


//========================================================================

#define TRACE_INSTRUCTION       (1<< (INSTRUCTION >> 24))
#define TRACE_PROGRAM_COUNTER   (1<< (PROGRAM_COUNTER >> 24))
#define TRACE_CYCLE_INCREMENT   (1<< (CYCLE_INCREMENT >> 24))
#define TRACE_ALL (0xffffffff)

//
// The trace_map is an STL map object that associates dynamically
// created trace types with a unique number. The simulation engine
// uses the number as a 'command' for tracing information of a 
// specific type. This number along with information specific to
// to the trace type is written into the trace buffer. When the
// simulation is halted and the trace buffer is parsed, the
// trace type can be extracted. This can then be used as an input
// to the trace_map to access an object that can further process
// the traced information.
//
// Here's an example:
//
// The pic_processor class during construction will request a trace
// type for tracing 8-bit register writes. 
//
map <unsigned int, TraceType *> trace_map;

Trace::Trace(void)
{

  for(trace_index = 0; trace_index < TRACE_BUFFER_SIZE; trace_index++)
    trace_buffer[trace_index] = NOTHING;

  trace_index = 0;
  string_cycle = 0;

  lastTraceType = LAST_TRACE_TYPE;

  xref = new XrefObject(&trace_value);

}

Trace::~Trace(void)
{
  if(xref)
    delete xref;

}

//--------------------------------------------------------------
// is_cycle_trace(unsigned int index)
//
//  Given an index into the trace buffer, this function determines
// if the trace is a cycle counter trace.
//
// INPUT: index - index into the trace buffer
//        *cvt_cycle - a pointer to where the cycle will be decoded
//                     if the trace entry is a cycle trace.
// RETURN: 0 - trace is not a cycle counter
//         1 - trace is the high integer of a cycle trace
//         2 - trace is the low integer of a cycle trace

int Trace::is_cycle_trace(unsigned int index, guint64 *cvt_cycle)
{

  if(!(get(index) & (CYCLE_COUNTER_LO | CYCLE_COUNTER_HI)))
    return 0;

  // Cycle counter

  // A cycle counter occupies two consecutive trace buffer entries.
  // We have to determine if the current entry (pointed to by index) is
  // the high or low integer of the cycle counter.
  //
  // The upper two bits of the trace are used to decode the two 32-bit
  // integers that comprise the cycle counter. The encoding algorithm is
  // optimized for speed:
  // CYCLE_COUNTER_LO is defined as 1<<31
  // CYCLE_COUNTER_HI is defined as 1<<30
  //
  //   trace[i] = low 32 bits of cycle counter | CYCLE_COUNTER_LO
  //   trace[i+1] = upper 32 bits of "    " | CYCLE_COUNTER_HI | bit 31 of cycle counter
  //
  // The low 32-bits are always saved in the trace buffer with the msb (CYCLE_COUNTER_LO)
  // set. However, notice that this bit may've already been set prior to calling trace().
  // So we need to make sure that we don't lose it. This is done by copying it along
  // with the high 32-bits of the cycle counter into the next trace buffer location. The
  // upper 2 bits of the cycle counter are assumed to always be zero (if they're not, gpsim
  // has been running for a loooonnnggg time!). Bit 30 (CYCLE_COUNTER_HIGH) is always
  // set in the high 32 bit trace. While bit 31 gets the copy of bit 31 that was over
  // written in the low 32 bit trace.
  //
  // Here are some examples:
  //                                                upper 2 bits 
  //    cycle counter    |  trace[i]    trace[i+1]    [i]   [i+1]
  //---------------------+----------------------------------------
  //         0x12345678  |  0x92345678  0x40000000    10     01
  //         0x44445555  |  0xc4445555  0x40000000    11     01
  // 0x1111222233334444  |  0xb3334444  0x51112222    10     01
  //         0x9999aaaa  |  0x9999aaaa  0xc0000000    10     11
  //         0xccccdddd  |  0xccccdddd  0xc0000000    11     11
  //         0xccccddde  |  0xccccddde  0xc0000000    11     11
  //
  // Looking at the upper two bits of trace buffer, we can make these
  // observations:
  //
  // 00 - not a cycle counter trace
  // 10 - current index points at the low int of a cycle counter
  // 01 - current index points at the high int of a cycle counter
  // 11 - if traces on either side of the current index are the same
  //      then the current index points to a low int else it points to a high int

  int j = index;                         // Assume that the index is pointing to the low int.
  int k = (j + 1) & TRACE_BUFFER_MASK;   // and that the next entry is the high int.

  if( (get(j) & CYCLE_COUNTER_LO) &&
      (get(k) & CYCLE_COUNTER_HI) )
    {
      if(get(j) & CYCLE_COUNTER_HI)
	{
	  // The upper two bits of the current trace are set. This means that
	  // the trace is either the high 32 bits or the low 32 bits of the cycle 
	  // counter. This ambiguity is resolved by examining the trace buffer on
	  // either side of the current index. If the entry immediately proceeding
	  // this one is not a cycle counter trace, then we know that we're pointing
	  // at the low 32 bits. If the proceeding entry IS a cycle counter trace then
	  // we have two consecutive cycle traces (we already know that the entry
	  // immediately following the current trace index is a cycle counter trace).
	  // Now we know that if  have consecutive cycle traces, then they differ by one
	  // count. We only need to look at the low 32 bits of these consecutive
	  // traces to ascertain this.
	  int i = (index - 1) &  TRACE_BUFFER_MASK;   // previous index
	  if( (get(i) & (CYCLE_COUNTER_HI | CYCLE_COUNTER_LO)) &&
	      (((get(k) - get(i)) & 0x7fffffff) == 1) )
	    return 1;
	}

      // The current index points to the low int and the next entry is
      // the high int.
      // extract the ~64bit cycle counter from the trace buffer.
      if(cvt_cycle) {
	*cvt_cycle = get(k)&0x3fffffff;
	*cvt_cycle = (*cvt_cycle << 32) | 
	  ((get(j)&0x7fffffff) | (get(k)&0x80000000 ));
      }

      return 2;

    }

  //printf("trace error??? in cycle trace\n");

  return 1;
}

//------------------------------------------------------------------------
//
// dump1 - decode a single trace buffer item.
//
//
// RETURNS 2 if the trace item takes two trace entries, otherwise returns 1.

int Trace::dump1(unsigned index, char *buffer, int bufsize)
{
  char a_string[50];
  Register *r;

  guint64 cycle;
  int return_value = is_cycle_trace(index,&cycle);

  if(bufsize)
    buffer[0] = 0;   // 0 terminate just in case no string is created

  if(return_value == 2) {
    /*
    if(trace_flag & TRACE_PROGRAM_COUNTER)
      snprintf(buffer, bufsize," cycle: 0x%llx" , cycle);
    */
    return(return_value);

  }
  
  return_value = 1;

  switch (type(index))
    {
    case NOTHING:
      snprintf(buffer, bufsize,"  empty trace cycle\n");
      break;
    case INSTRUCTION:
      if(trace_flag & TRACE_INSTRUCTION)
	snprintf(buffer, bufsize," instruction: 0x%04x",
		 get(index)&0xffff);
      break;
      /*
    case PROGRAM_COUNTER_2C:
    case PROGRAM_COUNTER:
      if(trace_flag & TRACE_PROGRAM_COUNTER) {
	int address  = cpu->map_pm_index2address(get(index)&0xffff);
	snprintf(buffer, bufsize," pc: 0x%04x %s", 
		 address ,
		 (*cpu->pma)[address].name(a_string,sizeof(a_string)));
      }
      break;
    case PC_SKIP:
      {
	int address  = cpu->map_pm_index2address(get(index)&0xffff);
	snprintf(buffer, bufsize,"  skipped: %04x %s",
		 address ,
		 (*cpu->pma)[address].name(a_string,sizeof(a_string)));
      }
      break;
      */
    case REGISTER_READ_VAL:
    case REGISTER_READ:
      r = cpu->registers[(get(index)>>8) & 0xfff];
      snprintf(buffer, bufsize,"   read: 0x%02x from %s",
	       get(index)&0xff, r->name().c_str());
      break;
    case REGISTER_WRITE_VAL:
    case REGISTER_WRITE:
      r = cpu->registers[(get(index)>>8) & 0xfff];
      snprintf(buffer, bufsize,"  wrote: 0x%02x to %s",
	       get(index)&0xff, r->name().c_str());
      break;
    case REGISTER_READ_16BITS:
      r = cpu->registers[(get(index)>>16) & 0xff];
      snprintf(buffer, bufsize,"   read: 0x%04x from %s",
	       get(index)&0xffff, r->name().c_str());
      break;
    case REGISTER_WRITE_16BITS:
      r = cpu->registers[(get(index)>>16) & 0xff];
      snprintf(buffer, bufsize,"  wrote: 0x%04x to %s",
	       get(index)&0xffff, r->name().c_str());
      break;
    case READ_W:
      snprintf(buffer, bufsize,"   read: 0x%02x from W",
	       get(index)&0xff);
      break;
    case WRITE_W:
      snprintf(buffer, bufsize,"  wrote: 0x%02x to W",
	       get(index)&0xff);
      break;
    case WRITE_TRIS:
      snprintf(buffer, bufsize,"  wrote: 0x%02x to TRIS",
	       get(index)&0xff);
      break;
    case WRITE_OPTION:
      snprintf(buffer, bufsize,"  wrote: 0x%02x to OPTION",
	       get(index)&0xff);
      break;
    case BREAKPOINT:
      snprintf(buffer, bufsize,"BREAK: ");
      bp.dump_traced(get(index) & 0xffffff);
      break;
    case INTERRUPT:
      snprintf(buffer, bufsize," interrupt");
      break;
    case _RESET:
      switch( (RESET_TYPE) (get(index)&0xff))
	{
	case POR_RESET:
	  snprintf(buffer, bufsize," POR");
	  break;
	case WDT_RESET:
	  snprintf(buffer, bufsize," WDT reset");
	  break;
	case SOFT_RESET:
	  snprintf(buffer, bufsize,"SOFT reset");
	  break;
	default:
	  snprintf(buffer, bufsize,"unknown reset");
	}
      break;

    case OPCODE_WRITE:
      if(type(index-1) == OPCODE_WRITE)
	snprintf(buffer, bufsize," wrote opcode: 0x%04x to pgm memory: 0x%05x",
	       get(index)&0xffff,
	       get(index - 1) & 0xffffff);

      break;

    case CYCLE_INCREMENT:
      if(trace_flag & TRACE_CYCLE_INCREMENT)
	snprintf(buffer, bufsize," cycle increment");
      break;

    case MODULE_TRACE2:
      return_value = 2;
    case MODULE_TRACE1:

      // fall through

    default:
      if(type(index) != CYCLE_COUNTER_HI) {
	if(cpu)
	  return_value = cpu->trace_dump1(index,buffer,bufsize);
      }
    }

  return return_value;

}

//------------------------------------------------------------------
void Trace::enableLogging(char *fname)
{
  if(fname)
    logger.enable(fname);
}

void Trace::disableLogging()
{
  logger.disable();
}

//------------------------------------------------------------------
// int Trace::dump(unsigned int n=0)
//

int Trace::dump(unsigned int n, FILE *out_stream)
{

  char a_string[50];
  int  frame_index=-1;
  bool found_pc = false;
  bool found_frame = false;

  if(!cpu)
    return 0;

  if(!n)
    n = 5;
  n++;

  if(!out_stream)
    return 0;


  unsigned int i = tbi(trace_index-2);
  unsigned int k = tbi(trace_index-1);
  guint64 cycle=0;

  if(trace.is_cycle_trace(i,&cycle) !=  2)
    return 0;

  unsigned int frame_start = tbi(trace_index-2);
  unsigned int frame_end = trace_index;
  k = frame_start;

  unsigned int cycle_delta = 0;

  static bool bMapIsInitialized = false;

  if(!bMapIsInitialized) {
    //trace_map[PROGRAM_COUNTER] = new PCTraceType(cpu,PROGRAM_COUNTER,1);
    //cpu->pc->set_trace_command(PROGRAM_COUNTER);
    trace_map[REGISTER_WRITE] = new RegisterTraceType(cpu,REGISTER_WRITE,1);
    trace_map[WRITE_W] = new WTraceType(cpu,WRITE_W,1);

    bMapIsInitialized = true;
  }


  // Save the state of the CPU here. 
  cpu->save_state();


  //
  // Decode the trace buffer
  //
  // Starting at the end of the trace buffer, step backwards
  // and count 'n' trace frames.

  current_frame = 0;

  while(traceFrames.size()<n && inRange(k,frame_end,frame_start)) {

    printf("k=%d  ",k);
    map<unsigned int, TraceType *>::iterator tti = trace_map.find(type(k));

    if(tti != trace_map.end()) {
      TraceType *tt = (*tti).second;

      if(tt) {
	if(tt->isFrameBoundary() )
	  addFrame(new TraceFrame( ));
	   
	tt->decode(k);
      } 


    } else if(is_cycle_trace(k,&cycle) == 2) {
      current_cycle_time = cycle;
      printf(" Cycle Trace type 0x%Lx\n",cycle);
      //printf(" ignoring cycle trace\n");
    } else {

      if(type(k) == 0)
	break;

      if(is_cycle_trace(tbi(k-1),0) != 2)
	printf(" could not decode 0x%x\n",get(k));
    }

    k = tbi(k-1);
  }


  cout <<"Trace frame: " << traceFrames.size() << endl;
  printTraceFrame();
  deleteTraceFrame();
  /*
  try {
    for(i=0;i<n && k!=frame_end;i++) {

      found_pc = false;


      cycle -= cycle_delta;
      cycle_delta = 0;

      do {

	k = tbi(k-1);
	switch (type(k)) {
	case PROGRAM_COUNTER_2C:
	  cycle_delta++;

	  // fall through

	case PROGRAM_COUNTER:
	  cycle_delta++;
	  found_pc = true;
	  found_frame = true;
	  break;
	}
      } while (!((k==frame_end) || found_pc));


      if(found_pc)
	frame_start = k;

    }
  }
  catch (const char * err) {
    cout << err << endl;
    return 0;
  }

  if(!found_frame)
    return 0;


  trace_flag = TRACE_ALL & ~(TRACE_CYCLE_INCREMENT | 
			     CYCLE_COUNTER_LO      | 
			     CYCLE_COUNTER_HI      |
			     TRACE_INSTRUCTION     |
			     TRACE_PROGRAM_COUNTER);

  for(i=0; i<n; i++) {

    unsigned int j = tbi(frame_start + 1);
    found_pc = false;

    while (!((j==frame_end) || found_pc)) {
      if( type(j) == PROGRAM_COUNTER   || type(j) == PROGRAM_COUNTER_2C) {
	frame_index = j;
	break;
      }
      j = tbi(j+1);
    }
    
    if(j != frame_end) {

      printf("Trace frame: 0x%x - 0x%x\n",frame_start,frame_index);

      int address  = cpu->map_pm_index2address(trace_buffer[frame_index]&0xffff);

      fprintf(out_stream,"0x%016LX  %s  0x%04X  0x%04X  %s\n",
	      cycle,
	      cpu->name().c_str(),
	      address,
	      cpu->pma->get_opcode(address), //trace_buffer[instruction_index]&0xffff,
	      (*cpu->pma)[address].name(a_string,sizeof(a_string))
	      );

      cycle++;
      if( type(j) == PROGRAM_COUNTER_2C) 
	cycle++;

      int j = tbi(frame_start + 1);

      while (inRange(j,frame_start, frame_index)) {
	// Decode the next item in the trace buffer.
	j = tbi(j + dump1(j,string_buffer, sizeof(string_buffer)));

	// If the item was decoded, then print it out.
	if(*string_buffer)
	  printf("%s\n",string_buffer);
      }

      frame_start = frame_index;
    }
    else 
      i = n; // abort the loop.
  }
  */
  return n;
}
//------------------------------------------------------------------------
unsigned int Trace::allocateTraceType(TraceType *tt)
{

  if(tt) {
    trace_map[lastTraceType] = tt;
    tt->type = lastTraceType;
    lastTraceType += (1<<24);

    return tt->type;
  }
  return 0;
}
//---------------------------------------------------------
// dump_raw
// mostly for debugging, 
void Trace::dump_raw(int n)
{
  if(!n)
    return;

  const int BUFFER_SIZE = 50;

  char buffer[BUFFER_SIZE];

  unsigned int i = (trace_index - n)  & TRACE_BUFFER_MASK;

  trace_flag = TRACE_ALL;

  do {
    printf("%04X: ",i);
    if(is_cycle_trace(i,0))
      printf("%08X:%08X",trace_buffer[i], trace_buffer[(i+1) & TRACE_BUFFER_MASK]);
    else
      printf("%08X         ",trace_buffer[i]);

    i = (i + dump1(i,buffer,BUFFER_SIZE)) & TRACE_BUFFER_MASK;

    if(buffer[0]) 
      printf("%s",buffer);
    putc('\n',stdout);

  } while((i!=trace_index) && (i!=((trace_index+1)&TRACE_BUFFER_MASK)));
    putc('\n',stdout);
    putc('\n',stdout);
}

//
// dump_last_instruction(void)

void Trace::dump_last_instruction(void)
{
  dump(1,stdout);
}


/*****************************************************************
 *
 *         Logging
 */
TraceLog::TraceLog(void)
{
  logging = 0;
  log_filename = 0;
  cpu = 0;
  log_file = 0;
  lxtp=0;
  last_trace_index = 0;
  items_logged = 0;
  buffer.trace_flag = TRACE_ALL;

}

TraceLog::~TraceLog(void)
{

  disable_logging();
    
  close_logfile();

}

void TraceLog::callback(void)
{
  int n = 0;
  get_trace().cycle_counter(get_cycles().value);

  if((log_file||lxtp) && logging) {
    if(last_trace_index < get_trace().trace_index) { 
      for (unsigned int c=last_trace_index; c<get_trace().trace_index; c++)
        if ((get_trace().trace_buffer[c] & 0xff000000) == Trace::INSTRUCTION)
	  n++;
    } else {
      for (unsigned int c=last_trace_index; c<=TRACE_BUFFER_MASK; c++)
        if ((get_trace().trace_buffer[c] & 0xff000000) == Trace::INSTRUCTION)
	  n++;
      for (unsigned int c=0; c<get_trace().trace_index; c++)
        if ((get_trace().trace_buffer[c] & 0xff000000) == Trace::INSTRUCTION)
	  n++;
    }

    //trace.dump(n, log_file, watch_reg);
    if(file_format==TRACE_FILE_FORMAT_ASCII)
	trace.dump(n, log_file);

    last_trace_index = trace.trace_index;
    get_cycles().set_break(get_cycles().value + 1000,this);
  }

}

void TraceLog::open_logfile(const char *new_fname, int format)
{

    if(!new_fname)
    {
	switch(format)
	{
	case TRACE_FILE_FORMAT_LXT:
	    new_fname = "gpsim.lxt";
            break;
	case TRACE_FILE_FORMAT_ASCII:
	    new_fname = "gpsim.log";
	    break;
	}
    }

  if(log_filename) {
    //
    // Looks like there's a log file open and now we
    // want to open a different one.
    //

    if(strcmp(new_fname, log_filename) == 0 ) 
      return;  // the file with this name is already opened


    close_logfile();

  }

  file_format=format;

  switch(file_format)
  {
  case TRACE_FILE_FORMAT_ASCII:
      log_file = fopen(new_fname, "w");
      lxtp=0;
      break;
  case TRACE_FILE_FORMAT_LXT:
      lxtp = lt_init(new_fname);
      lt_set_timescale(lxtp, -8);
      lt_set_clock_compress(lxtp);
      lt_set_initial_value(lxtp, 'X');
      log_file=0;
      break;
  }

  log_filename = strdup(new_fname);
  items_logged = 0;
}

void TraceLog::close_logfile(void)
{

  if(log_filename) {
      switch(file_format)
      {
      case TRACE_FILE_FORMAT_ASCII:
	  write_logfile();
	  fclose(log_file);
	  break;
      case TRACE_FILE_FORMAT_LXT:
	  lt_close(lxtp);
	  break;
      }

      free(log_filename);
      log_file = 0;
      log_filename = 0;
  }
}

void TraceLog::write_logfile(void)
{

  unsigned int i,j;
  char buf[256];

  if(log_file) {

    buffer.trace_flag = TRACE_ALL;

    // Loop through the trace buffer and decode each entry.
    // Note that the second loop counter, j, keeps tabs on first, i.

    for(i=0,j=0; i<buffer.trace_index && j<buffer.trace_index; j++) {
      buf[0] = 0;
      i = (i + buffer.dump1(i,buf, sizeof(buf))) & TRACE_BUFFER_MASK;

      if(buf[0]) {
	items_logged++;
	fprintf(log_file,"%s\n", buf);
      } else {
	cout << " write_logfile: ERROR, couldn't decode trace buffer\n";
	return;
      }
    }

    buffer.trace_index = 0;
  }

}

void TraceLog::enable_logging(const char *new_fname, int format)
{

  if(logging)
    return;

  if(!cpu) {
    if(get_active_cpu()) {
      cpu = (pic_processor *)get_active_cpu();
    } else
      cout << "Warning: Logging can't be enabled until a cpu has been selected.";
  }

  buffer.cpu = cpu;
  open_logfile(new_fname, format);

  last_trace_index = buffer.trace_index;
  // cycles.set_break(cycles.value + 1000,this);
  logging = 1;

}

void TraceLog::disable_logging(void)
{

  if(!logging)
    return;

  close_logfile();
  logging = 0;


}

void TraceLog::status(void)
{

  if(logging) {
      cout << "Logging to file: " << log_filename;
      switch(file_format)
      {
      case TRACE_FILE_FORMAT_LXT:
	  cout << "in LXT mode" << endl;
	  break;
      case TRACE_FILE_FORMAT_ASCII:
      default:
	  cout << "in ASCII mode" << endl;
	  break;
      }

    // note that there's the cycle counter is traced for every
    // item that is in the log buffer, so the actual events that
    // triggered a log is the total buffer size divided by 2.

    int total_items = (buffer.trace_index + items_logged)/2;
    if(total_items) {
      cout << "So far, it contains " << hex << "0x" << total_items << " logged events\n";
    } else {
      cout << "Nothing has been logged yet\n";
    }

    int first = 1;

    for(int i = 0; i<MAX_BREAKPOINTS; i++) {
      if( 
	 (bp.break_status[i].type == bp.NOTIFY_ON_REG_READ) ||
	 (bp.break_status[i].type == bp.NOTIFY_ON_REG_WRITE) ||
	 (bp.break_status[i].type == bp.NOTIFY_ON_REG_READ_VALUE) ||
	 (bp.break_status[i].type == bp.NOTIFY_ON_REG_WRITE_VALUE)
	 ) {

	if(first) 
	  cout << "Log triggers:\n";
	first = 0;

	bp.dump1(i);
      }
    }

  } else {
    cout << "Logging is disabled\n";
  }

}

void TraceLog::switch_cpus(Processor *pcpu)
{
  cpu = pcpu;
}

void TraceLog::lxt_trace(unsigned int address, unsigned int value, guint64 cc)
{
    char *name;

    name = (char *)cpu->registers[address]->name().c_str();

    lt_set_time(lxtp, (int)(get_cycles().value*4.0e8*cpu->period));

    symp=lt_symbol_find(lxtp, name);
    if(symp==0)
    {
	symp=lt_symbol_add(lxtp,
			   name,         // name
			   0,            // rows
			   7,            // msb
			   0,            // lsb
			   LT_SYM_F_BITS //flags
			  );
        assert(symp!=0);
    }
    lt_emit_value_int(lxtp, symp, 0, value);
}

void TraceLog::register_read(unsigned int address, unsigned int value, guint64 cc)
{
    switch(file_format)
    {
    case TRACE_FILE_FORMAT_ASCII:
	buffer.cycle_counter(cc);
	buffer.register_read(address, value);
	break;
    case TRACE_FILE_FORMAT_LXT:
	lxt_trace(address, value, cc);
	break;
    }
}

void TraceLog::register_write(unsigned int address, unsigned int value, guint64 cc)
{
    switch(file_format)
    {
    case TRACE_FILE_FORMAT_ASCII:
	buffer.cycle_counter(cc);
	buffer.register_write(address, value);
	if(buffer.near_full())
	    write_logfile();
	break;
    case TRACE_FILE_FORMAT_LXT:
	lxt_trace(address, value, cc);
	break;
    }
}

void TraceLog::register_read_value(unsigned int address, unsigned int value, guint64 cc)
{
    switch(file_format)
    {
    case TRACE_FILE_FORMAT_ASCII:
	buffer.cycle_counter(cc);
	//buffer.register_read_value(address, value);
        break;
    case TRACE_FILE_FORMAT_LXT:
	lxt_trace(address, value, cc);
	break;
    }
}

void TraceLog::register_write_value(unsigned int address, unsigned int value, guint64 cc)
{
    switch(file_format)
    {
    case TRACE_FILE_FORMAT_ASCII:
	buffer.cycle_counter(cc);
	//buffer.register_write_value(address, value);
        break;
    case TRACE_FILE_FORMAT_LXT:
	lxt_trace(address, value, cc);
	break;
    }
}



/*****************************************************************
 *
 *         Profiling
 */
ProfileKeeper::ProfileKeeper(void)
{
  enabled = 0;
  cpu = 0;
  last_trace_index = 0;
}

ProfileKeeper::~ProfileKeeper(void)
{

  disable_profiling();
}

void ProfileKeeper::catchup(void)
{
    Register *r;
    if(!enabled)
        return;
    for(unsigned int i=last_trace_index; i!=trace.trace_index; i = (i+1)& TRACE_BUFFER_MASK)
    {
	switch (trace.trace_buffer[i] & 0xff000000)
	{
	case Trace::INSTRUCTION:
	    instruction_address=trace_pc_value;
	    cpu->program_memory[instruction_address]->cycle_count++;
	    trace_pc_value++;
	    break;
	    /*
	case Trace::PROGRAM_COUNTER:
	case Trace::PC_SKIP:
	    trace_pc_value=trace.trace_buffer[i]&0xffff;
	    break;
	    */
	case Trace::CYCLE_INCREMENT:
	    cpu->program_memory[instruction_address]->cycle_count++;
	    break;
	case Trace::REGISTER_READ:
	    r = cpu->registers[(trace.trace_buffer[i]>>8) & 0xfff];
	    if(r->isa() == Register::FILE_REGISTER)
	    {
		r->read_access_count++;
	    }
	    break;
	case Trace::REGISTER_WRITE:
	    r = cpu->registers[(trace.trace_buffer[i]>>8) & 0xfff];
	    if(r->isa() == Register::FILE_REGISTER)
	    {
		r->write_access_count++;
	    }
	    break;
	}
    }

    last_trace_index = trace.trace_index;
}

void ProfileKeeper::callback(void)
{
    if(enabled)
    {
        catchup();
	get_cycles().set_break(get_cycles().value + 1000,this);
    }
}

void ProfileKeeper::enable_profiling(void)
{

    if(enabled)
	return;

    if(!cpu) {
	if(get_active_cpu())
	    cpu = get_active_cpu();
	else
	    cout << "Warning: Profiling can't be enabled until a cpu has been selected.";
    }

    last_trace_index = trace.trace_index;
    get_cycles().set_break(get_cycles().value + 1000,this);
    enabled = 1;
}

void ProfileKeeper::disable_profiling(void)
{

    if(!enabled)
	return;

    enabled = 0;
}

void ProfileKeeper::switch_cpus(Processor *pcpu)
{
    cpu = pcpu;
}

//*****************************************************************
// *** KNOWN CHANGE ***
//  Support functions that will get replaced by the CORBA interface.
//  

//--------------------------------------------
void trace_dump_all(void)
{
  trace.dump(0, stdout);

}
//--------------------------------------------
void trace_dump_n(int numberof)
{
  trace.dump(numberof,stdout);
}
//--------------------------------------------
void trace_dump_raw(int numberof)
{
  trace.dump_raw(numberof);
}
//--------------------------------------------
void trace_enable_logging(char *file, int format)
{
  if (file)
    trace_log.enable_logging(file, format);
  else
    trace_log.disable_logging();
}

void trace_watch_register(int reg) {
  //trace_log.watch_reg = reg;
}



//--------------------------------------------------
//  BoolEventBuffer::event(bool state)
//
//    Record a 0/1 event (e.g. the state of an I/O line).
//    returns false if this event has filled the buffer
//    or if the buffer is full. Note, an event will get lost
//    if the callee attempts to save it in a full buffer.

inline bool BoolEventBuffer::event(bool state)
{

  // If the new event is different than the most recently logged one
  // then we need to log this event. (Note that the event is implicitly
  // logged in the "index". I.e. 1 events are at odd indices.

  if(state ^ (index & 1) ^ !bInitialState)  {

    if(index < max_events) {
      buffer[index++] = get_cycles().value - start_time;
      return true;
    }

    return false;
	
  }

  return true;
}

//--------------------------------------------------
//
// BoolEventBuffer::get_index
//
// given an event time, get_index will perform a binary
// search for it in the event buffer.
//

unsigned int BoolEventBuffer::get_index(guint64 event_time) 
{
  guint32 start_index, end_index, search_index, bstep;
  guint64 time_offset;

  end_index = index;
  start_index = 0;

  bstep = (max_events+1) >> 1;
  search_index = start_index + bstep;

  bstep >>= 1;

  time_offset = event_time - start_time;

  // Binary search for the event time:
  do {
    if(time_offset == buffer[search_index])
      return search_index;

    if(time_offset < buffer[search_index])
      search_index = search_index - bstep;
    else
      search_index = search_index + bstep;

    //cout << hex << "search index "<< search_index << "  buffer[search_index] " << buffer[search_index] << '\n';
    bstep >>= 1;

  } while(bstep);

  if(time_offset >= buffer[search_index])
    return search_index;
  else
    return (--search_index);

}

//--------------------------------------------------
//
// BoolEventBuffer::activate
//
// 
void BoolEventBuffer::activate(bool _initial_state)
{

  // If the buffer is activated already or the buffer is full,
  // then we can't activate it.

  if(isActive() || isFull())
    return;

  // Save the time for this initial event
  start_time = get_cycles().value;
  bInitialState = _initial_state;

  index = 0;  // next state gets stored at the first position in the buffer.

  bActive = true;

  future_cycle = start_time + (1<<31);
  get_cycles().set_break(future_cycle, this);

}
//--------------------------------------------------
void BoolEventBuffer::deactivate(void)
{

  bActive = false;

  if(future_cycle)
    get_cycles().clear_break(this);

  future_cycle = 0;

}
//--------------------------------------------------
void BoolEventBuffer::callback(void)
{
  future_cycle = 0;

  if(isActive())
    deactivate();
}
void BoolEventBuffer::callback_print(void)
{
  cout << "BoolEventBuffer\n";
}

//--------------------------------------------------
// BoolEventBuffer -- constructor
//
BoolEventBuffer::BoolEventBuffer(bool _initial_state, guint32 _max_events)
{

  max_events = _max_events;

  // Make sure that max_events is an even power of 2
  if(max_events & (max_events - 1)) {
    max_events <<= 1;
    while(1) {
      if(max_events && (max_events & (max_events-1)))
	max_events &= max_events - 1;
      else
	break;

    }
  } else if(!max_events)
    max_events = 4096;
    
  max_events--;  // make the max_events a mask

  buffer = new guint64[max_events];

  activate(_initial_state);
}

BoolEventBuffer::~BoolEventBuffer(void)
{

  delete [] buffer;
  
}
