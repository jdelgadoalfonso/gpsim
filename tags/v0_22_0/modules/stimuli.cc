/*
   Copyright (C) 2006 Scott Dattalo

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


/*
  stimuli.cc

  This module provides extended stimuli for gpsim.


  pulsegen - This a stimulus designed to synthesize square waves.
    It has the following attributes:

    .set
    .clear
    .delete
    .period

    The set and clear attributes generate the edge states.
    For example if the module is instantiated as PG, then

    gpsim> PG.set = 0x1000
    gpsim> PG.clear = 0x2000

    generate a rising edge at cycle 0x1000 and a falling edge at
    cycle 0x2000. Any number of edges can be specified and they
    may be in any order. The delete attribute removes an edge:

    gpsim> PG.delete = 0x2000  # remove the edge at cycle 0x2000

    The period attribute tells how many cycles there are in a
    rollover. If period is 0, then the pulsegen is not periodic.
    If there are edges beyond the period time, then those will be
    ignored.

    To be added:

    .start - specify a cycle to start
    .vhi - voltage for high drive
    .vlo - voltage for low drive
    .rth - output resitance
    .cth - output capacitance.


  pwlgen - piecewise linear generator.
    <not implemented>

  filegen - time and values are specified in a file.
    <not implemented>


*/

/* IN_MODULE should be defined for modules */
#define IN_MODULE

#include "../src/gpsim_time.h"
#include "stimuli.h"
#include "../config.h"
#include "../src/pic-ioports.h"
#include "../src/symbol.h"
#include "../src/trace.h"
#include "../src/processor.h"

namespace ExtendedStimuli {


  //----------------------------------------------------------------------
  // Attributes


  //
  class PulseAttribute : public Integer
  {
  public:
    PulseAttribute(PulseGen *pParent, const char *_name, const char * desc, double voltage);
    virtual void set(gint64);
  private:
    PulseGen *m_pParent;
    double    m_voltage;
  };

  class PulsePeriodAttribute : public Integer
  {
  public:
    PulsePeriodAttribute(PulseGen *pParent, const char *_name, const char * desc);
    virtual void set(gint64);
  private:
    PulseGen *m_pParent;
  };



  //----------------------------------------------------------------------
  //----------------------------------------------------------------------
  PulseAttribute::PulseAttribute(PulseGen *pParent,
				 const char *_name, const char * desc,
				 double voltage)
    : Integer(_name,0,desc), m_pParent(pParent), m_voltage(voltage)
  {
  }

  void PulseAttribute::set(gint64 i)
  {
    Integer::set(i);
    ValueStimulusData vsd;
    vsd.time = i;
    vsd.v = new Float(m_voltage);
    m_pParent->put_data(vsd);
  }

  //----------------------------------------------------------------------
  PulsePeriodAttribute::PulsePeriodAttribute(PulseGen *pParent,
					     const char *_name, const char * desc)
    : Integer(_name,0,desc), m_pParent(pParent)
  {
  }

  void PulsePeriodAttribute::set(gint64 i)
  {
    Integer::set(i);
    m_pParent->update_period();
  }

  //----------------------------------------------------------------------
  // StimulusBase
  //----------------------------------------------------------------------
  StimulusBase::StimulusBase(const char *_name, const char *_desc)
    : Module(_name,_desc)
  {
    // Default module attributes.
    initializeAttributes();

    // The I/O pin
    m_pin = new IO_bi_directional((name() + ".pin").c_str());
    m_pin->update_direction(IOPIN::DIR_OUTPUT,true);


  }

  //----------------------------------------------------------------------
  //----------------------------------------------------------------------

  void StimulusBase::callback_print()
  {
    printf("ExtendedStimulus:%s CallBack ID %d\n",name().c_str(),CallBackID);
  }


  void StimulusBase::create_iopin_map()
  {
    create_pkg(1);
    assign_pin(1, m_pin);
  }



  //----------------------------------------------------------------------
  // FileGen Module
  //----------------------------------------------------------------------
  //FileGen::FileGen


  //----------------------------------------------------------------------
  // PulseGen Module
  //----------------------------------------------------------------------

  Module *PulseGen::construct(const char *new_name)
  {
    PulseGen *pPulseGen = new PulseGen(new_name);
    pPulseGen->create_iopin_map();
    return pPulseGen;
  }

  //----------------------------------------------------------------------
  //----------------------------------------------------------------------

  PulseGen::PulseGen(const char *_name=0)
    : StimulusBase(_name, "\
Pulse Generator\n\
 Attributes:\n\
 .set - time when the pulse will drive high\n\
 .clear - time when the pulse will drive low\n\
 .period - time the pulse stream is repeated\n\
"), m_future_cycle(0), m_start_cycle(0)
  {
    // Attributes for the pulse generator.
    m_set = new PulseAttribute(this, "set","r/w cycle time when ouput will be driven high", 5.0);
    m_clear = new PulseAttribute(this, "clear","r/w cycle time when ouput will be driven low",0.0);
    m_period = new PulsePeriodAttribute(this,
					"period","r/w cycle time to specify pulse stream repeat rate");

    add_attribute(m_set);
    add_attribute(m_clear);
    add_attribute(m_period);

    sample_iterator = samples.end();
  }

  PulseGen::~PulseGen()
  {
    delete m_pin;
    delete m_set;
    delete m_clear;
    delete m_period;
  }

  //----------------------------------------------------------------------
  //----------------------------------------------------------------------
  void PulseGen::callback()
  {
    //guint64 currCycle = get_cycles().get();

    if (sample_iterator != samples.end()) {

      double d;
      (*sample_iterator).v->get(d);
      m_pin->putState(d > 2.5);

      ++sample_iterator;

      // If we reached the end of a non-periodic waveform
      // then we're done.

      if (sample_iterator == samples.end()  &&
	  m_period->getVal() == 0)
	return;

      // If this is a periodic pulse stream and either
      //     a) we reached the end of the sequence
      //     b) we have more data but it exceeds the period
      // then
      //   start the stream over.
      if (m_period->getVal() && ((sample_iterator == samples.end()  ||
				  (*sample_iterator).time > m_period->getVal()))) {

	sample_iterator = samples.begin();
	m_start_cycle += m_period->getVal();
      }

      m_future_cycle = m_start_cycle + (*sample_iterator).time;
      get_cycles().set_break(m_future_cycle, this);
    }

  }

  //------------------------------------------------------------
  // Set a callback break point at the cycle specified
  // Point the sample_iterator to the appropiate sample

  void  PulseGen::setBreak(guint64 next_cycle,list<ValueStimulusData>::iterator si)
  {

    if (m_future_cycle) {
      get_cycles().clear_break(this);
      m_future_cycle = 0;
      sample_iterator = samples.end();
    }

    if (next_cycle > get_cycles().get()) {
      get_cycles().set_break(next_cycle, this);
      m_future_cycle = next_cycle;
      sample_iterator = si;
    }
  }


  //------------------------------------------------------------
  // cycleIsInFuture - find_if helper predicate.
  //
  // The stl find_if() algorithm takes a pointer to a function whose
  // job is to compare a list element to a reference value. Stroustrop
  // calls this function a predicate.

  static guint64 current_cycle=0;
  static bool cycleIsInFuture(ValueStimulusData &data_point)
  {
    return data_point.time > current_cycle;
  }

  void PulseGen::update_period()
  {
    // If the period is 0 then force the start to 0.
    if (m_period->getVal() == 0)
      m_start_cycle = 0;

    // Find the next sample that will generate an edge.
    list<ValueStimulusData>::iterator si;
    current_cycle = get_cycles().get() -  m_start_cycle;
    si = find_if(samples.begin(), samples.end(), cycleIsInFuture);

    if (si == samples.end() && m_period->getVal())
      setBreak(m_start_cycle + m_period->getVal(), samples.begin());
  }

  void PulseGen::update()
  {

    if (samples.empty())
      return;  // There are no samples

    current_cycle = get_cycles().get();

    list<ValueStimulusData>::iterator si;

    if (current_cycle == 0) {

      // The simulation hasn't started yet.

      // Point to the second sample
      si = samples.begin();
      ++si;

      // If the next sample *is* the second one then we've been here before
      // (and that means we've already handled the first sample)
      if (sample_iterator == si)
	return;

      if (si == samples.end()) {
	si = samples.begin();
	sample_iterator = si;
	double d;
	(*si).v->get(d);
	m_pin->putState(d > 2.5);
      }

      sample_iterator = si;
      --si;
      double d;
      (*si).v->get(d);
      m_pin->putState(d > 2.5);


      setBreak((*sample_iterator).time, sample_iterator);

      return;

    }

    current_cycle -= m_start_cycle;

    si = find_if(samples.begin(), samples.end(), cycleIsInFuture);

    if (si == sample_iterator)
      return;

    setBreak(m_start_cycle + (*si).time, si);

  }
  void PulseGen::put_data(ValueStimulusData &data_point)
  {
    list<ValueStimulusData>::iterator si;
    si = find(samples.begin(), samples.end(), data_point);
    if (si == samples.end()) {
      samples.push_back(data_point);
      samples.sort();
    } else {
      delete (*si).v;
      (*si).v = data_point.v;
    }

    update();
  }


  string PulseGen::toString()
  {
    ostringstream sOut;

    sOut << "pulsegen toString method" << hex;

    list<ValueStimulusData>::iterator si;
    if (m_period->getVal())
      sOut << endl <<"period 0x" << m_period->getVal();
    if (m_start_cycle)
      sOut << endl << "start  0x" << m_start_cycle;

    si = samples.begin();
    while (si != samples.end()) {
      sOut << endl;

      double d;
      (*si).v->get(d);
      sOut << "  {0x" << (*si).time << ',' << d << '}';
      if (si == sample_iterator)
	sOut << " <-- Next at cycle 0x" << (m_start_cycle + (*si).time);
      ++si;
    }
    sOut << ends;
    return sOut.str();
  }


  //----------------------------------------------------------------------
  // File Stimulus
  //----------------------------------------------------------------------
  class FileNameAttribute : public String
  {
  public:
    FileNameAttribute(FileStimulus *, const char *_name, const char * desc);

    virtual void set(Value *);

    const char *getLine();
    const FILE *fp() { return m_pFile; }
  private:
    FileStimulus *m_Parent;
    FILE *m_pFile;
    enum  {
      eBuffSize = 1024
    };
    char m_buff[eBuffSize];
  };
  //=1024;
  FileNameAttribute::FileNameAttribute(FileStimulus *pParent,
				       const char *_name,
				       const char * _desc)
    : String(_name,"",_desc), m_Parent(pParent), m_pFile(0)
  {

  }

  void FileNameAttribute::set(Value *pV)
  {
    if (m_pFile)
      return;

    String::set(pV);

    m_pFile = fopen( getVal(), "r");

    m_Parent->newFile();
  }

  const char *FileNameAttribute::getLine()
  {

    m_buff[0]=0;
    if (m_pFile)
      fgets(m_buff, eBuffSize, m_pFile);
    return m_buff;
  }
  //----------------------------------------------------------------------
  //----------------------------------------------------------------------
  FileStimulus::FileStimulus(const char *_name)
    : StimulusBase(_name, "\
File Stimulus\n\
 Attributes:\n\
 .file - file name\n\
"), m_future_cycle(0)
  {
    // Attributes for the pulse generator.
    m_file = new FileNameAttribute(this, "file","name of file or pipe supplying data");

    add_attribute(m_file);

  }


  FileStimulus::~FileStimulus()
  {

  }

  void FileStimulus::newFile()
  {
    if (m_future_cycle) {
      get_cycles().clear_break(this);
      m_future_cycle = 0;
    }

    parse(m_file->getLine());

  }

  void FileStimulus::parse(const char *cP)
  {
    if (!cP)
      return;
    guint64 t;
    float v;

    //fscanf(m_file,"%" PRINTF_INT64_MODIFIER "i %g",&t, &v);
    sscanf(cP,"%" PRINTF_INT64_MODIFIER "i %g",&t, &v);

    cout << "  read 0x" << hex << t << "," << v << endl;

  }

  void FileStimulus::callback()
  {
  }


  string FileStimulus::toString()
  {

    ostringstream sOut;

    sOut << "fileStimulus toString method" << endl;

    sOut << ends;
    return sOut.str();

  }

  //----------------------------------------------------------------------
  //----------------------------------------------------------------------

  class RegisterAddressAttribute : public Integer
  {
  public:
    RegisterAddressAttribute(Register *pReg, const char *_name, const char * desc);
    virtual void set(gint64);
  private:
    Register *m_replaced;
    const int InvalidAddress;
  };

  RegisterAddressAttribute::RegisterAddressAttribute(Register *pReg,const char *_name, const char * desc)
    : Integer(_name,0xffffffff,desc),
      m_replaced(pReg),
      InvalidAddress(0xffffffff)
  {
    m_replaced->address = InvalidAddress;
  }

  void RegisterAddressAttribute::set(gint64 i)
  {

    Processor *pcpu = get_active_cpu();
    if (pcpu && m_replaced) {

      if (m_replaced->address != InvalidAddress)
	pcpu->rma.removeRegister(m_replaced->address,m_replaced);

      m_replaced->set_cpu(pcpu);

      m_replaced->address = i & 0xffffffff;
      if (!pcpu->rma.insertRegister(m_replaced->address,m_replaced))
	  m_replaced->address = InvalidAddress;

      gint64 insertAddress = m_replaced->address;

      Integer::set(insertAddress);
    }
  }


  //----------------------------------------------------------------------
  //----------------------------------------------------------------------
  static void buildTraceType(Register *pReg, unsigned int baseType)
  {
    RegisterValue rv;

    rv = RegisterValue(baseType + (0<<8), baseType + (1<<8));
    pReg->set_write_trace(rv);
    rv = RegisterValue(baseType + (2<<8), baseType + (3<<8));
    pReg->set_read_trace(rv);

  }



  //----------------------------------------------------------------------
  //----------------------------------------------------------------------

  Module *PortStimulus::construct(const char *new_name)
  {
    PortStimulus *pPortStimulus = new PortStimulus(new_name);
    pPortStimulus->create_iopin_map();
    return pPortStimulus;
  }

  //----------------------------------------------------------------------
  //----------------------------------------------------------------------

  PortStimulus::PortStimulus(const char *_name)
    : Module(_name, "\
Port Stimulus\n\
 Attributes:\n\
 .port - port name\n\
 .tris - tris name\n\
 .lat  - latch name\n\
")
  {
    mPort  = new PicPortRegister((name()+".port").c_str(),8,0xff);
    mTris  = new PicTrisRegister((name()+".tris").c_str(),mPort);
    mLatch = new PicLatchRegister((name()+".lat").c_str(),mPort);
    mLatch->setEnableMask(0xff);

    mPortAddress = new RegisterAddressAttribute(mPort, "portAdr","Port register address");
    mTrisAddress = new RegisterAddressAttribute(mTris, "trisAdr","Tris register address");
    mLatchAddress = new RegisterAddressAttribute(mLatch, "latAdr","Latch register address");

    get_symbol_table().add_register(mPort);
    get_symbol_table().add_register(mTris);
    get_symbol_table().add_register(mLatch);

    add_attribute(mPortAddress);
    add_attribute(mTrisAddress);
    add_attribute(mLatchAddress);

    // FIXME - probably want something better than the generic module trace

    ModuleTraceType *mMTT = new ModuleTraceType(this,1," Port Stimulus");
    get_trace().allocateTraceType(mMTT);

    buildTraceType(mPort, mMTT->type());
    buildTraceType(mTris, mMTT->type() + (4<<8));
    buildTraceType(mLatch, mMTT->type() + (8<<8));

  }
  //----------------------------------------------------------------------
  //----------------------------------------------------------------------

  void PortStimulus::callback_print()
  {
    printf("PortStimulus:%s CallBack ID %d\n",name().c_str(),CallBackID);
  }


  void PortStimulus::create_iopin_map()
  {

    create_pkg(8);

    for (int i=0; i<8; i++) {
      char pinNumber = '1'+i;
      IO_bi_directional *ppin;

      ppin = new IO_bi_directional((name() + ".p" + pinNumber).c_str());
      ppin->update_direction(IOPIN::DIR_OUTPUT,true);
      assign_pin(i+1, mPort->addPin(ppin,i));
    }
  }


} // end of namespace ExtendedStimuli
