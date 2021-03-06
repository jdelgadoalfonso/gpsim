/*
   Copyright (C) 2006 T. Scott Dattalo

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

#if !defined(__CLOCK_PHASE_H__)
#define __CLOCK_PHASE_H__

#define CLOCK_EXPERIMENTS


#if defined(CLOCK_EXPERIMENTS)
/*
  Clock Phase

  The Clock Phase base class takes an external clock source as its
  input and uses this to control a module's simulation state. For
  example, the clock input on a microcontroller drives all of the
  digital state machines. On every edge of the clock, there is digital
  logic that can potentially change states. The Clock Phase base class
  can be thought of the "logic" that responds to the clock input and
  redirects control to the state machines inside of a processor.
*/

class Processor;
class ClockPhase
{
public:
  ClockPhase();
  virtual ~ClockPhase();
  virtual ClockPhase *advance()=0;
  void setNextPhase(ClockPhase *pNextPhase) { m_pNextPhase = pNextPhase; }
  ClockPhase *getNextPhase() { return m_pNextPhase; }
protected:
  ClockPhase *m_pNextPhase;
};


/*
  The Processor Phase base class is a Clock Phase class that contains a
  pointer to a Processor object. It's the base class from which all of
  the processor's various Phase objects are derived.
*/
class ProcessorPhase : public ClockPhase
{
public:
  ProcessorPhase(Processor *pcpu);
  virtual ~ProcessorPhase();
protected:
  Processor *m_pcpu;
};

/*
  The Execute 1 Cycle class is a Processor Phase class designed to
  execute a single instruction. 

*/
class phaseExecute1Cycle : public ProcessorPhase
{
public:
  phaseExecute1Cycle(Processor *pcpu);
  virtual ~phaseExecute1Cycle();
  virtual ClockPhase *advance();
};

class phaseExecute2ndHalf : public ProcessorPhase
{
public:
  phaseExecute2ndHalf(Processor *pcpu);
  virtual ~phaseExecute2ndHalf();
  virtual ClockPhase *advance();
  ClockPhase *firstHalf(unsigned int uiPC);
protected:
  unsigned int m_uiPC;
};

class phaseExecuteInterrupt : public ProcessorPhase
{
public:
  phaseExecuteInterrupt(Processor *pcpu);
  virtual ~phaseExecuteInterrupt();
  virtual ClockPhase *advance();

  ClockPhase *firstHalf(unsigned int uiPC);
protected:
  unsigned int m_uiPC;
};

// phaseIdle - when a processor is idle, the current
// clock source can be handled by this class.

class phaseIdle : public ProcessorPhase
{
public:
  phaseIdle(Processor *pcpu);
  virtual ~phaseIdle();
  virtual ClockPhase *advance();
protected:
};

////// TEMPORARY ////////
// These will be moved into the Processor class.
extern ClockPhase *mCurrentPhase;
extern phaseExecute1Cycle *mExecute1Cycle;
extern phaseExecute2ndHalf *mExecute2ndHalf;
extern phaseExecuteInterrupt *mExecuteInterrupt;
extern phaseIdle *mIdle;

#endif // defined(CLOCK_EXPERIMENTS)

#endif  //if !defined(__CLOCK_PHASE_H__)
