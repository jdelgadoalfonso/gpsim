/*
   Copyright (C) 1998-2000 T. Scott Dattalo

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

#ifndef __PIC_PROCESSORS_H__
#define __PIC_PROCESSORS_H__
#include <glib.h>
#include <stdio.h>

#include "gpsim_classes.h"
#include "modules.h"
#include "processor.h"
#include "program_files.h"

#include "pic-registers.h"
#include "14bit-registers.h"
#include "trigger.h"

class EEPROM;
class instruction;
class Register;
class sfr_register;
class pic_register;
class ConfigMemory;
class ResetTraceType;
class InterruptTraceType;

enum PROCESSOR_TYPE
{
  _PIC_PROCESSOR_,
  _14BIT_PROCESSOR_,
  _12BIT_PROCESSOR_,
  _16BIT_PROCESSOR_,
  _P10F200_,
  _P10F202_,
  _P12C508_,
  _P12C509_,
  _P16C84_,
  _P16CR83_,
  _P16CR84_,
  _P12CE518_,
  _P12CE519_,
  _P16F83_,
  _P16F84_,
  _P16C71_,
  _P16C712_,
  _P16C716_,
  _P16C54_,
  _P16C55_,
  _P16C56_,
  _P16C61_,
  _P16C62_,
  _P16C62A_,
  _P16CR62_,
  _P16F627_,
  _P16F628_,
  _P16F648_,
  _P16C63_,
  _P16C64_,
  _P16C64A_,
  _P16CR64_,
  _P16C65_,
  _P16C65A_,
  _P16C72_,
  _P16C73_,
  _P16C74_,
  _P16F73_,
  _P16F74_,
  _P16F87_,
  _P16F88_,
  _P16F818_,
  _P16F819_,
  _P16F871_,
  _P16F873_,
  _P16F873A_,
  _P16F874_,
  _P16F874A_,
  _P16F876_,
  _P16F876A_,
  _P16F877_,
  _P16F877A_,
  _P17C7xx_,
  _P17C75x_,
  _P17C752_,
  _P17C756_,
  _P17C756A_,
  _P17C762_,
  _P17C766_,
  _P18Cxx2_,
  _P18C2x2_,
  _P18C242_,
  _P18F242_,
  _P18F248_,
  _P18F448_,
  _P18C252_,
  _P18F252_,
  _P18C442_,
  _P18C452_,
  _P18F442_,
  _P18F452_,
  _P18F1220_,
  _P18F1320_,
  _P18F2455_,
};

// Configuration modes.  DELETE THIS...
//  The configuration mode bits are the config word bits remapped.
//  The remapping removes processor dependent bit definitions.
class ConfigMode {
 public:

  enum {
    CM_FOSC0 = 1<<0,    // FOSC0 and  FOSC1 together define the PIC clock
    CM_FOSC1 = 1<<1,    // All PICs todate have these two bits, but the
                        // ones with internal oscillators use them differently
    CM_WDTE =  1<<2,    // Watch dog timer enable
    CM_CP0 =   1<<3,    // Code Protection
    CM_CP1 =   1<<4,
    CM_PWRTE = 1<<5,    // Power on/Reset timer enable
    CM_BODEN = 1<<6,    // Brown out detection enable
    CM_CPD =   1<<7,
    CM_MCLRE = 1<<8,    // MCLR enable

    CM_FOSC1x = 1<<31,   // Hack for internal oscillators
  };

  int config_mode;
  int valid_bits;
  ConfigMode() { 
    config_mode = 0xffff; 
    valid_bits = CM_FOSC0 | CM_FOSC1 | CM_WDTE;
  };

  virtual void set_config_mode(int new_value) { config_mode = new_value & valid_bits;};
  virtual void set_valid_bits(int new_value) { valid_bits = new_value;};
  void set_fosc0(){config_mode |= CM_FOSC0;};
  void clear_fosc0(){config_mode &= ~CM_FOSC0;};
  bool get_fosc0(){return (config_mode & CM_FOSC0);};
  void set_fosc1(){config_mode |= CM_FOSC1;};
  void clear_fosc1(){config_mode &= ~CM_FOSC1;};
  bool get_fosc1(){return (0 != (config_mode & CM_FOSC1));};
  bool get_fosc1x(){return (0 != (config_mode & CM_FOSC1x));};
  void set_fosc01(int v) 
  { 
    config_mode = (config_mode & ~(CM_FOSC0 | CM_FOSC1)) |
      (v & (CM_FOSC0 | CM_FOSC1));
  }

  void set_cp0()  {config_mode |= CM_CP0;  valid_bits |= CM_CP0;};
  void clear_cp0(){config_mode &= ~CM_CP0; valid_bits |= CM_CP0;};
  bool get_cp0()  {return (0 != (config_mode & CM_CP0));};
  void set_cp1()  {config_mode |= CM_CP1;  valid_bits |= CM_CP1;};
  void clear_cp1(){config_mode &= ~CM_CP1; valid_bits |= CM_CP1;};
  bool get_cp1()  {return (0 != (config_mode & CM_CP1));};

  void enable_wdt()  {config_mode |= CM_WDTE;};
  void disable_wdt() {config_mode &= ~CM_WDTE;};
  void set_wdte(bool b) { config_mode = b ? (config_mode | CM_WDTE) : (config_mode & ~CM_WDTE); }
  bool get_wdt()     {return (0 != (config_mode & CM_WDTE));};

  void set_mclre(bool b) { config_mode = b ? (config_mode | CM_MCLRE) : (config_mode & ~CM_MCLRE); }
  bool get_mclre()     {return (0 != (config_mode & CM_MCLRE));};

  void enable_pwrte()   {config_mode |= CM_PWRTE;  valid_bits |= CM_PWRTE;};
  void disable_pwrte()  {config_mode &= ~CM_PWRTE; valid_bits |= CM_PWRTE;};
  void set_pwrte(bool b) { config_mode = b ? (config_mode | CM_PWRTE) : (config_mode & ~CM_PWRTE); }
  bool get_pwrte()      {return (0 != (config_mode & CM_PWRTE));};
  bool is_valid_pwrte() {return (0 != (valid_bits & CM_PWRTE));};

  virtual void print();

};


//---------------------------------------------------------
// Watch Dog Timer
//

class WDT : public TriggerObject
{
public:
  WDT(pic_processor *, double _timeout);
  void put(unsigned int new_value);
  virtual void initialize(bool enable);
  void set_timeout(double);
  virtual void set_prescale(unsigned int);
  virtual void reset(RESET_TYPE r);
  void clear();
  virtual void callback();
  virtual void update();
  virtual void callback_print();
  void set_breakpoint(unsigned int bpn);
  bool hasBreak() { return breakpoint != 0;}

protected:
  pic_processor *cpu;           // The cpu to which this wdt belongs.

  unsigned int
    value,
    breakpoint,
    prescale;
  guint64
    future_cycle;

  double timeout;   // When no prescaler is assigned
  bool   wdte;
  bool   warned;


};

/*==================================================================
 * FIXME - move these global references somewhere else
 */
#include "cmd_gpsim.h"
extern guint64 gui_update_rate; // The rate (in simulation cycles) at which the gui is updated

/*==================================================================
 *
 * Here are the base class declarations for the pic processors
 */

/*
 * First, forward-declare a few class references
 */

enum IOPIN_TYPES
{
  INPUT_ONLY,          // e.g. MCLR
  BI_DIRECTIONAL,      // most iopins
  BI_DIRECTIONAL_PU,   // same as bi_directional, but with pullup resistor. e.g. portb
  OPEN_COLLECTOR       // bit4 in porta on the 18 pin midrange devices.
};

/*
 * Define a base class processor for the pic processor family
 *
 * All pic processors are derived from this class.
 */

class pic_processor : public Processor
{
public:



  unsigned int config_word;      // as read from hex or cod file
  ConfigMode   *config_modes;    // processor dependent configuration bits.

  unsigned int pll_factor;       // 2^pll_factor is the speed boost the PLL adds 
                                 // to the instruction execution rate.

  WDT          wdt;

  INDF         *indf;
  FSR          *fsr;
  Stack        *stack;

  Status_register *status;
  WREG         *W;
  PCL          *pcl;
  PCLATH       *pclath;
  PCHelper     *m_PCHelper;
  TMR0         tmr0;
  int          num_of_gprs;

  EEPROM      *eeprom;       // set to NULL for PIC's that don't have a data EEPROM

  bool LoadProgramFile(const char *pFilename,
		       FILE *pFile,
		       const char *pProcessorName
		       );

  void add_sfr_register(Register *reg, unsigned int addr,
			RegisterValue por_value=RegisterValue(0,0),const char *new_name=0);
  void delete_sfr_register(Register **ppReg, unsigned int addr);
  void remove_sfr_register(Register *ppReg, unsigned int addr);

  void init_program_memory(unsigned int memory_size);
  void build_program_memory(int *memory,int minaddr, int maxaddr);

  virtual instruction * disasm ( unsigned int address,unsigned int inst)=0;
  virtual void create_config_memory() = 0;
  virtual void tris_instruction(unsigned int tris_register) {return;};
  virtual void create_symbols();
  virtual void run(bool refresh=true);
  virtual void finish();

  void sleep();
  virtual void enter_sleep();
  virtual void exit_sleep();
  virtual void step(unsigned int steps,bool refresh=true);
  virtual void step_over(bool refresh=true);
  virtual void step_cycle();

  virtual void step_one(bool refresh=true) {
    program_memory[pc->value]->execute();
  }

  // Take a snap shot of the internal state.
  virtual void save_state();

  virtual void interrupt() { return; };
  //// TEMPORARY - consolidate the various bp.set_interrupt() calls to one function:
  void BP_set_interrupt();
  void pm_write();

  virtual bool set_config_word(unsigned int address, unsigned int cfg_word);
  virtual unsigned int get_config_word(unsigned int address);
  virtual unsigned int config_word_address() const {return 0x2007;};
  virtual ConfigMode *create_ConfigMode() { return new ConfigMode; };
  virtual void reset(RESET_TYPE r);

  virtual void create();

  virtual PROCESSOR_TYPE isa(){return _PIC_PROCESSOR_;};
  virtual PROCESSOR_TYPE base_isa(){return _PIC_PROCESSOR_;};

  /* The program_counter class calls these two functions to get the upper bits of the PC
   * for branching (e.g. goto) or modify PCL instructions (e.g. addwf pcl,f) */
  virtual unsigned int get_pclath_branching_jump()=0;
  virtual unsigned int get_pclath_branching_modpcl()=0;

  virtual void option_new_bits_6_7(unsigned int)=0;
  virtual void put_option_reg(unsigned int) {}

  virtual void set_eeprom(EEPROM *e);
  virtual EEPROM *get_eeprom() { return (eeprom); }

  // Activity States reflect what the processor is currently doing
  // (The breakpoint class formally implemented this functionality).
  enum eProcessorActivityStates {
    ePAActive,      // Normal state
    ePAIdle,        // Processor is held in reset
    ePASleeping,    // Processor is sleeping
    ePAInterrupt,   // do we need this?
    ePAPMWrite      // Processor is busy performing a program memory write
  };
  eProcessorActivityStates getActivityState() { return m_ActivityState; }

  pic_processor(const char *_name=0, const char *desc=0);
  virtual ~pic_processor();

protected:
  ConfigMemory *m_configMemory;
  eProcessorActivityStates m_ActivityState;
  ResetTraceType *m_pResetTT;
  InterruptTraceType *m_pInterruptTT;
};


#define cpu_pic ( (pic_processor *)cpu)

//------------------------------------------------------------------------
// Base Class for configuration memory
//
// The configuration memory is only a tiny portion of the overall processor
// program memory space (only 1-word on the mid range devices). So, explicit
// attributes are created for each memory configuration word. Since the meaning
// of configuration memory varies from processor to processor, it is up to 
// each process to derive from this class.


class ConfigWord : public Integer
{
public:
  ConfigWord(const char *_name, unsigned int default_val, const char *desc,
             pic_processor *pCpu, unsigned int addr);
protected:
  pic_processor *m_pCpu;
  unsigned int m_addr;
};

class ConfigMemory
{
public:
  ConfigMemory(pic_processor *pCpu, unsigned int nWords);
  ~ConfigMemory();
  int addConfigWord(unsigned int addr, ConfigWord *);
  ConfigWord *getConfigWord(unsigned int addr);
protected:
  pic_processor *m_pCpu;
  ConfigWord **m_ConfigWords;
  int m_nConfigWords;
};

/*
class ConfigMemory : public Integer
{
public:
  ConfigMemory(const char *_name, unsigned int default_val, const char *desc,
	       pic_processor *pCpu, unsigned int addr);
protected:
  pic_processor *m_pCpu;
  unsigned int m_addr;
};
*/
#endif
