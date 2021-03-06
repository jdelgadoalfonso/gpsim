/*
   Copyright (C) 1998 Scott Dattalo

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


#include <stdio.h>
#include <iostream.h>
#include <iomanip.h>

#include "16bit-registers.h"
#include "16bit-processors.h"
#include "interface.h"
#include "xref.h"

//--------------------------------------------------
// member functions for the BSR class
//--------------------------------------------------
//
void  BSR::put(unsigned int new_value)
{
  value = new_value & 0x0f;
  trace.register_write(address,value);

  cpu->register_bank = &cpu->registers[ value << 8 ];


}

void  BSR::put_value(unsigned int new_value)
{

  put(new_value);

  if(xref)
  {
      xref->update();
      cpu->indf.xref->update();
    }



}


//--------------------------------------------------
// member functions for the FSR class
//--------------------------------------------------
//
void  FSRL::put(unsigned int new_value)
{
  value = new_value & 0xff;
  trace.register_write(address,value);

  //  iam->fsr_delta = 0;
  iam->update_fsr_value();


}

void  FSRL::put_value(unsigned int new_value)
{

  put(new_value);

  if(xref)
    {
	xref->update();
  
	cpu->indf.xref->update();
    }


}

void  FSRH::put(unsigned int new_value)
{
  value = new_value & 0x0f;
  trace.register_write(address,value);

  //  iam->fsr_delta = 0;
  iam->update_fsr_value();

}

void  FSRH::put_value(unsigned int new_value)
{

  put(new_value);

  if(update_object)
    {

	xref->update();
  
	cpu->indf.xref->update();

    }
}

void  INDF16::put(unsigned int new_value)
{
  trace.register_write(address,new_value);

  iam->fsr_value += iam->fsr_delta;
  iam->fsr_delta = 0;

  iam->put(new_value);

}

void INDF16::put_value(unsigned int new_value)
{
  put(new_value);

  if(xref)
      xref->update();


}

unsigned int INDF16::get(void)
{

  trace.register_read(address,value);

  iam->fsr_value += iam->fsr_delta;
  iam->fsr_delta = 0;

  return(iam->get());
}

unsigned int INDF16::get_value(void)
{
  return(iam->get_value());
}

//------------------------------------------------
// PREINC
unsigned int PREINC::get(void)
{

  trace.register_read(address,value);
  iam->preinc_fsr_value();

  return(iam->get());
}

unsigned int PREINC::get_value(void)
{
  return(iam->get_value());
}

void PREINC::put(unsigned int new_value)
{
  trace.register_write(address,new_value);

  iam->preinc_fsr_value();
  iam->put(new_value);
}

void PREINC::put_value(unsigned int new_value)
{
  put(new_value);

  if(xref)
      xref->update();

}

//------------------------------------------------
// POSTINC
unsigned int POSTINC::get(void)
{

  trace.register_read(address,value);
  iam->postinc_fsr_value();

  return(iam->get());
}

unsigned int POSTINC::get_value(void)
{
  return(iam->get_value());
}

void POSTINC::put(unsigned int new_value)
{
  trace.register_write(address,new_value);

  iam->postinc_fsr_value();
  iam->put(new_value);
}

void POSTINC::put_value(unsigned int new_value)
{
  put(new_value);


  if(xref)
      xref->update();


}


//------------------------------------------------
// POSTDEC
unsigned int POSTDEC::get(void)
{

  trace.register_read(address,value);
  iam->postdec_fsr_value();

  return(iam->get());
}

unsigned int POSTDEC::get_value(void)
{
  return(iam->get_value());
}

void POSTDEC::put(unsigned int new_value)
{
  trace.register_write(address,new_value);

  iam->postdec_fsr_value();
  iam->put(new_value);
}

void POSTDEC::put_value(unsigned int new_value)
{
  put(new_value);

  if(xref)
      xref->update();

}


//------------------------------------------------
// PLUSW
unsigned int PLUSW::get(void)
{

  trace.register_read(address,value);

  int destination = iam->plusw_fsr_value();
  if(destination > 0)
    return(cpu->registers[destination]->get());
  else
    return 0;

}

unsigned int PLUSW::get_value(void)
{

  int destination = iam->plusw_fsr_value();
  if(destination > 0)
    return(cpu->registers[destination]->get_value());
  else
    return 0;

}

void PLUSW::put(unsigned int new_value)
{
  trace.register_write(address,new_value);

  int destination = iam->plusw_fsr_value();
  if(destination > 0)
    cpu->registers[destination]->put(new_value);
}

void PLUSW::put_value(unsigned int new_value)
{
  int destination = iam->plusw_fsr_value();
  if(destination > 0)
    cpu->registers[destination]->put_value(new_value);


  if(xref)
    {
	xref->update();
      if(destination > 0)
	  cpu->registers[destination]->xref->update();
    }

}

//------------------------------------------------

void Indirect_Addressing::init(_16bit_processor *new_cpu)
{

  fsrl.iam = this;
  fsrh.iam = this;
  indf.iam = this;
  preinc.iam = this;
  postinc.iam = this;
  postdec.iam = this;
  plusw.iam = this;

  cpu = new_cpu;

}

/*
 * put - Each of the indirect registers associated with this
 * indirect addressing class will call this routine to indirectly
 * write data.
 */
void Indirect_Addressing::put(unsigned int new_value)
{
  /*  unsigned int midbits;

  if( ((fsr_value & 0xfc7) == 0xfc3) || ((fsr_value & 0xfc4) == 0xfc4))
    {
      midbits = (fsr_value >> 3) & 0x7;
      if(midbits >= 3 && midbits <= 5)
	return;
    }
  */
  if(is_indirect_register(fsr_value))
    return;

  cpu->registers[get_fsr_value()]->put(new_value);

}

/*
 * get - Each of the indirect registers associated with this
 * indirect addressing class will call this routine to indirectly
 * retrieve data.
 */
unsigned int Indirect_Addressing::get(void)
{
  //  unsigned int midbits;

  // See the comment in Indirect_Addressing::put about fsr address checking
  if(is_indirect_register(fsr_value))
    return 0;
  else
    /*
  if( ((fsr_value & 0xfc7) == 0xfc3) || ((fsr_value & 0xfc4) == 0xfc4))
    {
      midbits = (fsr_value >> 3) & 0x7;
      if(midbits >= 3 && midbits <= 5)
	return 0;
    }
    */

  return cpu->registers[get_fsr_value()]->get();

}

/*
 * get - Each of the indirect registers associated with this
 * indirect addressing class will call this routine to indirectly
 * retrieve data.
 */
unsigned int Indirect_Addressing::get_value(void)
{
  /*
  unsigned int midbits;

  See the comment in Indirect_Addressing::put about fsr address checking

  if( ((fsr_value & 0xfc7) == 0xfc3) || ((fsr_value & 0xfc4) == 0xfc4))
    {
      midbits = (fsr_value >> 3) & 0x7;
      if(midbits >= 3 && midbits <= 5)
	return 0;
    }
  */
  if(is_indirect_register(fsr_value))
    return 0;
  else
    return cpu->registers[get_fsr_value()]->get_value();

}

void Indirect_Addressing::put_fsr(unsigned int new_fsr)
{

  fsrl.put(new_fsr & 0xff);
  fsrh.put((new_fsr>>8) & 0x0f);

}


/*
 * update_fsr_value - This routine is called by the FSRL and FSRH
 * classes. It's purpose is to update the 16-bit (actually 12-bit)
 * address formed by the concatenation of FSRL and FSRH.
 *
 */

void Indirect_Addressing::update_fsr_value(void)
{

  if(current_cycle != cpu->cycles.value)
    {
      fsr_value = (fsrh.value << 8) |  fsrl.value;
      fsr_delta = 0;
    }
}

/*
 * preinc_fsr_value - This member function pre-increments the current
 * fsr_value. If the preinc access is a read-modify-write instruction
 * (e.g. bcf preinc0,1 ) then the increment operation should occur
 * only once. 
 */

void Indirect_Addressing::preinc_fsr_value(void)
{

  if(current_cycle != cpu->cycles.value)
    {
      fsr_value += (fsr_delta+1);
      fsr_delta = 0;
      current_cycle = cpu->cycles.value;
      put_fsr(fsr_value);
    }

}

void Indirect_Addressing::postinc_fsr_value(void)
{

  if(current_cycle != cpu->cycles.value)
    {
      fsr_value += fsr_delta;
      fsr_delta = 1;
      current_cycle = cpu->cycles.value;
      put_fsr(fsr_value+1);
      
    }
}

void Indirect_Addressing::postdec_fsr_value(void)
{

  if(current_cycle != cpu->cycles.value)
    {
      fsr_value += fsr_delta;
      fsr_delta = -1;
      current_cycle = cpu->cycles.value;
      put_fsr(fsr_value-1);
      
    }

}

int Indirect_Addressing::plusw_fsr_value(void)
{

  fsr_value += fsr_delta;
  fsr_delta = 0;

  unsigned int destination = (fsr_value + cpu->W.value) & _16BIT_REGISTER_MASK;
  if(is_indirect_register(destination))
    return -1;
  else
    return destination;

}

//------------------------------------------------
void Fast_Stack::init(_16bit_processor *new_cpu)
{
  cpu = new_cpu;
}

void Fast_Stack::push(void)
{
  w = cpu->W.value;
  status = cpu->status.value;
  bsr = cpu->bsr.value;

}

void Fast_Stack::pop(void)
{
  cout << "popping fast stack\n";
  cpu->W.put(w);
  cpu->status.put(status);
  cpu->bsr.put(bsr);

}

//--------------------------------------------------
// put_value - Change the program counter without affecting the cycle counter
//             (This is what's called if the user changes the pc.)

void Program_Counter16::put_value(unsigned int new_value)
{
  value = new_value & memory_size_mask;
  cpu->pcl.value = value & 0xff;
  cpu->pclath.value = (new_value >> 8) & 0xff;
  //  cpu16->pclatu.value = (new_value >> 16) & 0xff;

  if(xref)
  {
      cpu->pcl.xref->update();
      cpu->pclath.xref->update();
      //update_object(cpu16->pclatu.gui_xref,cpu->pclatu.value);
      xref->update();
    }

}

//------------------------------------------------
// TOSL
unsigned int TOSL::get(void)
{
  value = stack->get_tos() & 0xff;
  trace.register_read(address,value);
  return(value);

}

unsigned int TOSL::get_value(void)
{

  value = stack->get_tos() & 0xff;
  return(value);

}

void TOSL::put(unsigned int new_value)
{
  trace.register_write(address,new_value);

  stack->put_tos( stack->get_tos() & 0xffffff00 | new_value & 0xff);
}

void TOSL::put_value(unsigned int new_value)
{

  stack->put_tos( stack->get_tos() & 0xffffff00 | new_value & 0xff);

  if(xref)
      xref->update();

}


//------------------------------------------------
// TOSH
unsigned int TOSH::get(void)
{
  value = (stack->get_tos() >> 8) & 0xff;
  trace.register_read(address,value);
  return(value);

}

unsigned int TOSH::get_value(void)
{

  value = (stack->get_tos() >> 8) & 0xff;
  return(value);

}

void TOSH::put(unsigned int new_value)
{
  trace.register_write(address,new_value);

  stack->put_tos( stack->get_tos() & 0xffff00ff | ( (new_value & 0xff) << 8));
}

void TOSH::put_value(unsigned int new_value)
{

  stack->put_tos( stack->get_tos() & 0xffff00ff | ( (new_value & 0xff) << 8));

  if(xref)
      xref->update();

}


//------------------------------------------------
// TOSU
unsigned int TOSU::get(void)
{
  value = (stack->get_tos() >> 16) & 0x1f;
  trace.register_read(address,value);
  return(value);

}

unsigned int TOSU::get_value(void)
{

  value = (stack->get_tos() >> 16) & 0x1f;
  return(value);

}

void TOSU::put(unsigned int new_value)
{
  trace.register_write(address,new_value);

  stack->put_tos( stack->get_tos() & 0xffe0ffff | ( (new_value & 0x1f) << 16));

}

void TOSU::put_value(unsigned int new_value)
{

  stack->put_tos( stack->get_tos() & 0xffe0ffff | ( (new_value & 0x1f) << 16));

  if(xref)
      xref->update();


}


//------------------------------------------------
// STKPTR
unsigned int STKPTR::get(void)
{

  trace.register_read(address,value);
  return(value);

}

unsigned int STKPTR::get_value(void)
{

  return(value);

}

void STKPTR::put(unsigned int new_value)
{
  trace.register_write(address,new_value);

  value = new_value;
}

void STKPTR::put_value(unsigned int new_value)
{

  value = new_value;

  if(xref)
      xref->update();

}


//--------------------------------------------------
//
Stack16::Stack16(void)
{
  stkptr.stack = this;
  tosl.stack = this;
  tosh.stack = this;
  tosu.stack = this;

}

void Stack16::push(unsigned int address)
{
  contents[stkptr.value & Stack16_MASK] = address;

  stkptr.value++;

  if((stkptr.value & Stack16_MASK) == 0)
    {
      // check the STVREN bit
      // if(STVREN) {reset(stack_over_flow); return;}
      stkptr.value |= 0x80;
    }

  stkptr.value &= 0xdf;

}

unsigned int Stack16::pop(void)
{

  if(stkptr.value & Stack16_MASK)
    return(contents[ (--stkptr.value) & Stack16_MASK]);
  else
    {
      // check the STVREN bit
      // if(STVREN) {reset(stack_over_flow); return;}
      stkptr.value |= 0x5f;
      return(contents[0]);
    }
}

void Stack16::reset(void)
{
  stkptr.value = 0;
}

unsigned int Stack16::get_tos(void)
{

  return (contents[stkptr.value & Stack16_MASK]);

}

void Stack16::put_tos(unsigned int new_tos)
{

  contents[stkptr.value & Stack16_MASK] = new_tos;

}

//--------------------------------------------------
// member functions for the T0CON base class
//--------------------------------------------------
T0CON::T0CON(void)
{
  break_point = 0;
  por_value = 0xff;
  wdtr_value = 0xff;
  new_name("t0con");
}

void T0CON::put(unsigned int new_value)
{

  unsigned int old_value = value;
  value = new_value;

  // First, check the tmr0 clock source bit to see if we are  changing from
  // internal to external (or vice versa) clocks.
  if( (value ^ old_value) & T0CS)
    cpu16->tmr0l.new_clock_source();

  // %%%FIX ME%%% - can changing the state of TOSE cause the timer to
  // increment if tmr0 is being clocked by an external clock?

  // Now check the rest of the tmr0 bits.
  if( (value ^ old_value) & (T0SE | PSA | PS2 | PS1 | PS0))
    cpu16->tmr0l.new_prescale();

  if( (value ^ old_value) & (BIT6 | BIT7))
    cpu->option_new_bits_6_7(value & (BIT6 | BIT7));

  trace.register_write(address,value);


}

//--------------------------------------------------

void INTCON_16::initialize(void)
{

  tmr0l = &cpu16->tmr0l;
  rcon  = &cpu16->rcon;
  intcon2  = &cpu16->intcon2;

}

//----------------------------------------------------------------------
// void INTCON_16::clear_gies(void)
//
//  This routine clears the global interrupt enable bit(s). If priority
// interrupts are used (IPEN in RCON is set) then the appropriate gie
// bit (either giel or gieh) is cleared.
//
// This routine is called from 16bit_processor::interrupt().
//
//----------------------------------------------------------------------

void INTCON_16::clear_gies(void)
{

  if(cpu16->interrupt_vector == INTERRUPT_VECTOR_HI)
    put(get() & ~GIEH);
  else
    put(get() & ~GIEL);


}


//----------------------------------------------------------------------
// void INTCON_16::clear_gies(void)
//
//----------------------------------------------------------------------

void INTCON_16::set_gies(void)
{

  get();   // Update the current value of intcon
           // (and emit 'register read' trace).

  if(rcon->value & RCON::IPEN)
    {
      // Interrupt priorities are being used.

      if(0 == (value & GIEH))
	{
	  // GIEH is cleared, so we need to set it

	  put(value | GIEH);
	  return;

	}
      else
	{
	  // GIEH is set. This means high priority interrupts are enabled.
	  // So we most probably got here because of an RETFIE instruction
	  // after handling a low priority interrupt. We could check to see
	  // if GIEL is low before calling put(), but it's not necessary.
	  // So we'll just blindly re-enable giel, and continue with the
	  // simulation.

	  put(value | GIEL);
	  return;

	}
    }
  else
    {

      // Interrupt priorities are not used, so re-enable GIEH (which is in
      // the same bit-position as GIE on the mid-range core).

      put(value | GIEH);
      return;

    }


}


//----------------------------------------------------------------------
// void INTCON_16::put(unsigned int new_value)
//
//  Here's were the 18cxxx interrupt logic is primarily handled. 
//
// inputs: new_value - 
// outputs: none
//
//----------------------------------------------------------------------

void INTCON_16::put(unsigned int new_value)
{

  value = new_value;
  trace.register_write(address,value);

  // Now let's see if there's a pending interrupt
  // if IPEN is set in RCON, then interrupt priorities
  // are being used. (In other words, there are two
  // interrupt priorities on the 18cxxx core. If a
  // low priority interrupt is being serviced, it's
  // possible for a high priority interrupt to interject.

  if(rcon->value & RCON::IPEN)
    {
      unsigned int i1;

      // Use interrupt priorities

      if( 0 == (value & GIEH))
	return;    // Interrupts are disabled

      // Now we just go through the interrupt logic of the 18cxxx
      // First we check the high priorities and then we check the
      // low ones. When ever we detect an interrupt, then the 
      // bp.interrupt flag is set (which will cause the interrupt
      // to be handled at the high level) and additional checks
      // are aborted.

      // If TO, INT, or RB flags are set AND their correspond
      // interrupts are enabled, then the lower three bits of
      // i1 will reflect this. Note that INTF does NOT have an
      // associated priority bit!

      i1 =  ( (value>>3)&value) & (T0IF | INTF | RBIF);

      if(i1 & ( (intcon2->value & (T0IF | RBIF)) | INTF))
	{
	  cpu16->interrupt_vector = INTERRUPT_VECTOR_HI;
	  trace.interrupt();
	  bp.set_interrupt();
	  return;
	}


      // If we reach here, then there are no high priority
      // interrupts pending. So let's check for the low priority
      // ones.

      if(i1 & ( (~intcon2->value & (T0IF | RBIF)) | INTF))
	{
	  cpu16->interrupt_vector = INTERRUPT_VECTOR_LO;
	  trace.interrupt();
	  bp.set_interrupt();
	  return;
	}


    }
  else
    {
      // ignore interrupt priorities

      cpu16->interrupt_vector = INTERRUPT_VECTOR_HI;
      if(value & GIE) 
	{
	  if( ( (value>>3)&value) & (T0IF | INTF | RBIF) )
	    {
	      trace.interrupt();
	      bp.set_interrupt();
	    }
	  else if(value & XXIE)
	    {
	      if(check_peripheral_interrupt())
		{
		  trace.interrupt();
		  bp.set_interrupt();
		}
	    }

	}

    }
}

//--------------------------------------------------
void TMR0H::put(unsigned int new_value)
{
  value = new_value & 0xff;
  trace.register_write(address,value);

}

//--------------------------------------------------
void TMR0H::put_value(unsigned int new_value)
{
  value = new_value & 0xff;
}

unsigned int TMR0H::get(void)
{
  trace.register_read(address,value);
  return(value);
}

unsigned int TMR0H::get_value(void)
{
  return(value);
}

//--------------------------------------------------
// get_prescale
//
//  If the prescaler is assigned to the WDT (and not TMR0)
//    then return 0
//  other wise
//    then return the Prescale select bits (plus 1)
//
unsigned int TMR0_16::get_prescale(void)
{

  if(t0con->value & (0x8))
    return 0;
  else
    return ((t0con->value & 7) + 1);
}

void TMR0_16::set_t0if(void)
{
  intcon->set_t0if();
}

unsigned int TMR0_16::get_t0cs(void)
{
 return t0con->value & 0x20;
}

void TMR0_16::initialize(void)
{
  t0con = &cpu16->t0con;
  intcon = &cpu16->intcon;
  tmr0h  = &cpu16->tmr0h;

}

//--------------------------------------------------
// member functions for the _TMR0 base class
//--------------------------------------------------
/*
_TMR0::_TMR0(void)
{
  break_point = 0;
  value=0;
  synchronized_cycle=0;
  prescale=1;
  new_name("tmr0");
}
*/
/*
void _TMR0::start(void)
{

  value = 0;
  cout << "TMRO::start\n";

  synchronized_cycle = cpu->cycles.value.lo;    // + 2 ???

  prescale = 1 << get_prescale();

  last_cycle = synchronized_cycle - value * prescale;
  future_cycle = last_cycle + 256 * prescale;

  cout << "_TMR0::start   last_cycle = " << 
    hex << last_cycle << " future_cycle = " << future_cycle << '\n';

  cpu->cycles.set_break(future_cycle, 0, this);
  cpu->cycles.dump_breakpoints();

}
*/

// %%%FIX ME%%% 
void TMR0_16::increment(void)
{
  //  cout << "_TMR0 increment because of external clock ";

  if(--prescale_counter == 0)
    {
      prescale_counter = prescale;

      if(t0con->value & T0CON::T08BIT)
	{
	  if(++value == 256)
	    {
	      value = 0;
	      set_t0if();
	    }
	}
      else
	{
	  if(++value == 256)
	    {
	      value = 0;
	      if(tmr0h->value++ == 256)
		{
		  tmr0h->put(0);
		  set_t0if();
		}

	    }
	}

      trace.register_write(address,value);
    }
  //  cout << value << '\n';
}

void TMR0_16::put(unsigned int new_value)
{
  if(get_t0cs())
    {
      cout << "_TMR0::put external clock...\n";
    }


  // Note, anytime something is written to TMR0, the prescaler, if it's
  // assigned to tmr0, is also cleared. This is implicitly handled by
  // saving the value of cpu's cycle counter and associating that value
  // with the tmr rollover.

  value = new_value & 0xff;

  prescale = 1 << get_prescale();
  prescale_counter = prescale;

  synchronized_cycle = cpu->cycles.value + 2;

  last_cycle = synchronized_cycle - value * prescale;


  unsigned int fc;

  if(t0con->value & T0CON::T08BIT)
    fc = last_cycle + 0x100 * prescale;
  else
    fc = last_cycle + 0x10000 * prescale;


  if(future_cycle)
    cpu->cycles.reassign_break(future_cycle, fc, this);
  else
    cpu->cycles.set_break(fc, this);

  future_cycle = fc;

  trace.register_write(address,value);

}

unsigned int TMR0_16::get_value(void)
{
  // If the _TMR0 is being read immediately after being written, then
  // it hasn't had enough time to synchronize with the PIC's clock.
  if(cpu->cycles.value <= synchronized_cycle)
    return value;

  if(get_t0cs())
    return(value);

  int new_value = (cpu->cycles.value - last_cycle)/ prescale;

  value = new_value & 0xff;

  tmr0h->put_value((new_value >> 8)&0xff);

  return(value);
  
}
/*
//----------------------------------------------
unsigned int _TMR0::get(void)
{
  value = get_value();
  trace.register_read(address, value);
}
*/
void TMR0_16::new_prescale(void)
{
  cout << "_TMR0 new_prescale\n";

  if(get_t0cs())
    {
      cout << "external clock...\n";

      //prescale = 1 << (cpu->option_reg.get_psa() ? 0 : (1+cpu->option_reg.get_prescale()));
      prescale = 1 << get_prescale();
      prescale_counter = prescale;
    }
  else
    {
      // Get the current value of TMR0
      int new_value = (cpu->cycles.value - last_cycle)/prescale;
      cout << "cycles " << cpu->cycles.value  << " old prescale " << prescale;

      //prescale = 1 << (cpu->option_reg.get_psa() ? 0 : (1+cpu->option_reg.get_prescale()));
      prescale = 1 << get_prescale();

      cout << " new prescale " << prescale;

      // Now compute the 'last_cycle' as though if TMR0 had been running on the 
      // new prescale all along. Recall, 'last_cycle' records the value of the cpu's
      // cycle counter when tmr0 last rolled over.

      last_cycle = cpu->cycles.value - new_value * prescale;
      cout << " effective last_cycle " << last_cycle << '\n';

      if(cpu->cycles.value <= synchronized_cycle)
	last_cycle += (synchronized_cycle - cpu->cycles.value);

      guint64 fc;

      if(t0con->value & T0CON::T08BIT)
	fc = last_cycle + 0x100 * prescale;
      else
	fc = last_cycle + 0x10000 * prescale;

      cout << "moving break from " << future_cycle << " to " << fc << '\n';

      cpu->cycles.reassign_break(future_cycle, fc, this);

    }
}
/*
void _TMR0::new_clock_source(void)
{

  cout << "_TMR0:new_clock_source changed to the ";
  if(get_t0cs())
    {
      cout << "external\n";
      //      cpu->cycles.
    }
  else
    {
      cout << "internal\n";
      put(value);    // let _TMR0::put() set a cycle counter break point
    }
}
*/
// TMR0 callback is called when the cycle counter hits the break point that
// was set in TMR0::put. The cycle counter will clear the break point, so
// we don't need to worry about it. At this point, TMR0 is rolling over.

void TMR0_16::callback(void)
{

  //  cout<<"_TMR0 rollover: " << hex << cpu->cycles.value.lo << '\n';

  // If tmr0 is being clocked by the external clock, then at some point
  // the simulated code must have switched from the internal clock to
  // external clock. The cycle break point was still set, so just ignore it.
  if(get_t0cs())
    {
      future_cycle = 0;  // indicates that tmr0 no longer has a break point
      return;
    }

  value = 0;
  synchronized_cycle = cpu->cycles.value;
  last_cycle = synchronized_cycle;

  if(t0con->value & T0CON::T08BIT)
    {
      // 8-bit mode
      future_cycle = last_cycle + 0x100*prescale;

    }
  else
    {
      // 16-bit mode
      tmr0h->put_value(0);
      future_cycle = last_cycle + 0x10000*prescale;
    }

  cpu->cycles.set_break(future_cycle, this);
  set_t0if();

}



//--------------------------------------------------

bool TXREG_16::is_empty(void)
{
  return(pir1->get_TXIF());
}

void TXREG_16::empty(void)
{
  pir1->set_TXIF();
}

void TXREG_16::full(void)
{
  pir1->clear_TXIF();
}

void RCREG_16::push(unsigned int new_value)
{

  _RCREG::push(new_value);

  pir1->set_RCIF();

}

//--------------------------------------------------
// member functions for the USART
//--------------------------------------------------
void USART_MODULE16::initialize(_16bit_processor *new_cpu)
{
  cpu = new_cpu;

  spbrg.txsta = &txsta;
  spbrg.rcsta = &rcsta;

  txreg.pir1 = &cpu->pir1;
  txreg.txsta = &txsta;

  txsta.txreg = &txreg;
  //txsta.spbrg = &spbrg;
  //txsta.txpin = uart_port->pins[6];
  //txsta.bit_count = 0;


  rcsta.rcreg = &rcreg;
  rcsta.spbrg = &spbrg;
  rcsta.txsta = &txsta;
  //rcsta.uart_port = uart_port;
  //rcsta.rx_bit = 7;

  rcreg.rcsta = &rcsta;
  rcreg.pir1 = &cpu->pir1;

}

void   USART_MODULE16::new_rx_edge(unsigned int bit)
{

  //  if( (rcsta.state == _RCSTA::RCSTA_WAITING_FOR_START) && !bit)
  //    rcsta.receive_start_bit();
}

//-------------------------------------------------------------------
//
//  Table Reads and Writes
//
// The 18cxxx family provides a peripheral that will allow the program
// memory to read and write to itself. 
//
//-------------------------------------------------------------------

void TBL_MODULE::initialize(_16bit_processor *new_cpu)
{
  cpu = new_cpu;

  /*
  tablat;
  tabptrl;
  tabptrh;
  tabptru;
  */


}

//-------------------------------------------------------------------
//  void TBL_MODULE::increment(void)
//
//  This function increments the 24-bit ptr that is formed by the
// concatenation of tabptrl,tabptrh, and tabptru. It is called by
// the TBLRD and TBLWT pic instructions when the auto-increment
// operand is specified (e.g. TBLWT *+ )
//
//
// Inputs:  none
// Outputs: none
//
//-------------------------------------------------------------------
void TBL_MODULE::increment(void)
{

  if(tabptrl.value >= 0xff) {
    tabptrl.put(0);
    if(tabptrh.value >= 0xff) {
      tabptrh.put(0);
      tabptru.put(tabptru.value + 1);
    }
  }
  else
    tabptrl.put(tabptrl.value + 1);


}
//-------------------------------------------------------------------
//-------------------------------------------------------------------
void TBL_MODULE::decrement(void)
{

  if(tabptrl.value == 0) {
    tabptrl.put(0xff);
    if(tabptrh.value = 0) {
      tabptrh.put(0xff);
      tabptru.put(tabptru.value - 1);
    }
  }
  else
    tabptrl.put(tabptrl.value - 1);

}
//-------------------------------------------------------------------
//-------------------------------------------------------------------
void TBL_MODULE::read(void)
{
  unsigned int tabptr,opcode;

  tabptr = 
    ( (tabptru.value & 0xff) << 16 ) |
    ( (tabptrh.value & 0xff) << 8 )  |
    ( (tabptrl.value & 0xff) << 0 );

  opcode = cpu->pma.get_opcode(tabptr>>1);

  //  cout << __FUNCTION__ << "() tabptr: 0x" << hex << tabptr << "opcode: 0x" << opcode << '\n';

  if(tabptr & 1)
    {
      tablat.put((opcode >> 8) & 0xff);
      internal_latch = (internal_latch & 0x00ff) | (opcode & 0xff00);
    }
  else
    {
      tablat.put((opcode >> 0) & 0xff);
      internal_latch = (internal_latch & 0xff00) | (opcode & 0x00ff);
    }

}
//-------------------------------------------------------------------
//-------------------------------------------------------------------
void TBL_MODULE::write(void)
{

  unsigned int tabptr,opcode;

  tabptr = 
    ( (tabptru.value & 0xff) << 16 ) |
    ( (tabptrh.value & 0xff) << 8 )  |
    ( (tabptrl.value & 0xff) << 0 );

  //  opcode = cpu->pma->get_opcode(tabptr);

  if(tabptr & 1)
    {
      // Long write
      internal_latch = (internal_latch & 0x00ff) | ((tablat.value<<8) & 0xff00);
      cpu->pma.put_opcode_start(tabptr>>1, internal_latch);
    }
  else
    {
      // Short Write
      internal_latch = (internal_latch & 0xff00) | (tablat.value & 0x00ff);
    }


}
