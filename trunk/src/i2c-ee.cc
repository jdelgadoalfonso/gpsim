/*
   Copyright (C) 1998-2003 Scott Dattalo
                 2004 Rob Pearce

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

#include <assert.h>

#include <iostream>
#include <iomanip>
using namespace std;

#include <glib.h>

#include "trace.h"
#include "pic-processor.h"
#include "stimuli.h"
#include "i2c-ee.h"


// I2C EEPROM Peripheral
//
//  This object emulates the I2C EEPROM peripheral on the 12CE51x
//
//  It's main purpose is to provide a means by which the port pins
//  may communicate.
// 


//--------------------------------------------------------------
//
//

class I2C_EE_SCL : public IO_input
{
public:

  I2C_EE *eeprom;

  I2C_EE_SCL (I2C_EE *_eeprom,
	       IOPORT *i, 
	       unsigned int b, 
	       char *opt_name=NULL) : IO_input(i,b,opt_name) { 

    eeprom = _eeprom;

    // Let the pin think it's in the high state. If this is wrong,
    // then the I/O pin driving it will correct it.
    // Note, may want to add a flag that indicates if the pin
    // has ever been driven at all. This way, we can capture the
    // first edge. Or we could add another parameter to the constructor.

    digital_state = 1;

  };

  virtual void put_node_state(int new_state) {

//    cout << "I2C_EE_SCL put_node_state " << new_state << '\n';

    state = new_state;

    if( state < h2l_threshold) {
      state = l2h_threshold + 1;
      put_digital_state(0);

    } else {

      if(state > l2h_threshold) {
	state = h2l_threshold - 1;
	put_digital_state(1);
      }
    }

  }

  void put_digital_state(bool new_dstate) { 
    bool diff = new_dstate ^ digital_state;
    digital_state = new_dstate;

//    cout << "eeprom scl put_digital_state " << digital_state << '\n';
    if( eeprom && diff ) {

      //  cout << "      ... that's an edge, so call handler\n";
      eeprom->new_scl_edge(digital_state);

      if(iop) // this check should not be necessary...
	iop->setbit(iobit,digital_state);


    }

  }

  //virtual void put_state( int new_state);
  virtual int get_voltage(guint64 current_time) {
    return 42;
  }
};



//--------------------------------------------------------------
//
//

class I2C_EE_SDA : public IO_open_collector
{
public:

  I2C_EE *eeprom;
  bool read_state;

  I2C_EE_SDA (I2C_EE *_eeprom,
	       IOPORT *i, 
	       unsigned int b, 
	       char *opt_name=NULL) : IO_open_collector(i,b,opt_name) { 

    eeprom = _eeprom;

    digital_state = 1;
    read_state = 1;
    update_direction(0);   // Make the pin an output.

  };

  virtual void put_node_state(int new_state) {

//    cout << "I2C_EE_SDA put_node_state " << new_state << '\n';

    state = new_state;

    if ( state < h2l_threshold )
    {
      state = l2h_threshold + 1;
      put_digital_state(0);

    }
    else if(state > l2h_threshold)
    {
	state = h2l_threshold - 1;
	put_digital_state(1);
    }
//    else
//        cout << "     no change, l2h = " << l2h_threshold << 
//                "  h2l = " << h2l_threshold << "\n";

  }

  void put_digital_state(bool new_dstate) { 


    bool diff = new_dstate ^ read_state;
    read_state = new_dstate;

//    cout << "I2C_EE_SDA put_digital_state " << read_state << '\n';
    if( eeprom && diff ) {

//        cout << "      ... that's an edge, so call handler\n";
      eeprom->new_sda_edge(read_state);

//      if(iop) // this check should not be necessary...
//	iop->setbit(iobit,digital_state);
    }


  }

  void put_state( int new_digital_state) {

    Register *port = get_iop();

    if(port) {
      // If the new state to which the stimulus is being set is different than
      // the current state of the bit in the ioport (to which this stimulus is
      // mapped), then we need to update the ioport.

      if ( new_digital_state!=digital_state ) {

	port->setbit(iobit,new_digital_state);

	digital_state = new_digital_state;
//        cout << "I2C_EE SDA : wrote " << digital_state << "\n";
        
	// If this stimulus is attached to a node, then let the node be updated
	// with the new state as well.
	if(snode)
	  snode->update(0);
	// Note that this will auto magically update
	// the io port.

      }
    }
  }

  virtual int get_voltage(guint64 current_time) {
//    cout << "I2C_EE_SDA::" <<__FUNCTION__ <<  
//            " digital state=" << digital_state << '\n';
    
    if(digital_state)
      return l2h_threshold * 5; // BODGE ALERT !!!  should be a pull-up resistor
    else
      return -drive;
  }

};





//----------------------------------------------------------
//
// I2C EE PROM
//
// There are many conditions that need to be verified against a real part:
//    1) what happens if 
//       > the simulator 
//    2) what happens if a RD is initiated while data is being written?
//       > the simulator ignores the read
//    3) what happens if 
//       > the simulator 

I2C_EE::I2C_EE(void)
{

  rom_size = 0;
  name_str = 0;
  cpu = 0;
}

Register * I2C_EE::get_register(unsigned int address)
{

  if ( address < rom_size )
    return rom[address];
  return 0;

}


void I2C_EE::start_write(void)
{

    cycles.set_break(cycles.value + I2C_EE_WRITE_TIME, this);

    rom[xfr_addr]->put ( xfr_data );
    ee_busy = true;

}


void I2C_EE::write_is_complete(void) 
{

//  assert(intcon != 0);

//  eecon1.value = (eecon1.value  & (~eecon1.WR)) | eecon1.EEIF;

//  intcon->peripheral_interrupt();


}



void I2C_EE::callback(void)
{

    ee_busy = false;
    cout << "I2C_EE::callback() - write cycle is complete\n";
//  switch ( state ) {
//  case I2C_EE::EEREAD:
    //cout << "eeread\n";

//    eecon2.unarm();
//    eedata.value = rom[eeadr.value]->get();
//    eecon1.value &= (~EECON1::RD);
//    break;

//  default:
//    cout << "I2C_EE::callback() bad eeprom state " << state << '\n';
//  }
}



bool I2C_EE::shift_read_bit ( bool x )
{
    xfr_data = ( xfr_data << 1 ) | ( x != 0 );
    bit_count++;
    if ( bit_count == 8 )
        return true;
    else
        return false;
}


bool I2C_EE::shift_write_bit ( void )
{
    bool bit;

    bit_count--;
    bit = ( xfr_data >> bit_count ) & 1;
//    cout << "I2C_EE : send bit " << bit_count << " = " << bit << "\n";
    return bit;
}


void I2C_EE::new_scl_edge ( bool direction )
{
    if ( direction )
    {
        // Rising edge
        nxtbit = sda->read_state;
//        cout << "I2C_EE SCL : Rising edge, data=" << nxtbit << "\n";
    }
    else
    {
        // Falling edge
//        cout << "I2C_EE SCL : Falling edge\n";
        switch ( bus_state )
        {
            case I2C_EE::IDLE :
                sda->put_state ( 1 );
                break;

            case I2C_EE::START :
                sda->put_state ( 1 );
                bus_state = I2C_EE::RX_CMD;
                break;

            case I2C_EE::RX_CMD :
                if ( shift_read_bit ( sda->read_state ) )
                {
                    if ( verbose )
                        cout << "I2C_EE : got command " << hex << xfr_data;
                    if ( ( xfr_data & 0xf0 ) == 0xA0 )
                    {
                        bus_state = I2C_EE::ACK_CMD;
                        if ( verbose )
                            cout << " - OK\n";
                        sda->put_state ( 0 );
                    }
                    else
                    {
                        // not for us
                        bus_state = I2C_EE::IDLE;
                        if ( verbose )
                            cout << " - not for us\n";
                    }
                }
                break;
                
            case I2C_EE::ACK_CMD :
                sda->put_state ( 1 );
                if ( xfr_data & 0x01 )
                {
                    // it's a read command
                    bus_state = I2C_EE::TX_DATA;
                    bit_count = 8;
                    xfr_data = rom[xfr_addr]->get();
                    sda->put_state ( shift_write_bit() );
                }
                else
                {
                    // it's a write command
                    bus_state = I2C_EE::RX_ADDR;
                    bit_count = 0;
                    xfr_data = 0;
                }
                break;

            case I2C_EE::RX_ADDR :
                if ( shift_read_bit ( sda->read_state ) )
                {
                    sda->put_state ( 0 );
                    bus_state = I2C_EE::ACK_ADDR;
                    xfr_addr = xfr_data % rom_size;
                    if ( verbose )
                        cout << "I2C_EE : address set to " << hex << xfr_addr << 
                                "  (raw " << xfr_data << ", rom size " << rom_size << ")\n";
                }
                break;

            case I2C_EE::ACK_ADDR :
                sda->put_state ( 1 );
                bus_state = I2C_EE::RX_DATA;
                bit_count = 0;
                xfr_data = 0;
                break;

            case I2C_EE::RX_DATA :
                if ( shift_read_bit ( sda->read_state ) )
                {
                    if ( verbose )
                        cout << "I2C_EE : data set to " << hex << xfr_data << "\n";
                    sda->put_state ( 0 );
                    bus_state = I2C_EE::ACK_WR;
                }
                break;

            case I2C_EE::ACK_WR :
                sda->put_state ( 1 );
                bus_state = I2C_EE::WRPEND;
                break;

            case I2C_EE::WRPEND :
                // We were about to do the write but got more data instead
                // of the expected stop bit
                xfr_data = sda->read_state;
                bit_count = 1;
                bus_state = I2C_EE::RX_DATA;
                if ( verbose )
                    cout << "I2C_EE : write postponed by extra data\n";
                break;

            case I2C_EE::TX_DATA :
                if ( bit_count == 0 )
                {
                    sda->put_state ( 1 );     // Release the bus
                    xfr_addr++;
                    xfr_addr %= rom_size;
                    bus_state = I2C_EE::ACK_RD;
                }
                else
                {
                    sda->put_state ( shift_write_bit() );
                }
                break;

            case I2C_EE::ACK_RD :
                if ( sda->read_state == 0 )
                {
                    // The master has asserted ACK, so we send another byte
                    bus_state = I2C_EE::TX_DATA;
                    bit_count = 0;
                    xfr_data = rom[xfr_addr]->get();
                }
                else
                    bus_state = I2C_EE::IDLE;   // Actually a limbo state
                break;

            default :
                sda->put_state ( 1 );     // Release the bus
                break;
        }
    }
//    cout << "I2C_EE new bus state = " << bus_state << "\n";
}


void I2C_EE::new_sda_edge ( bool direction )
{
    if ( scl->digital_state )
    {
        if ( direction )
        {
            // stop bit
            if ( verbose&2 )
                cout << "I2C_EE SDA : Rising edge in SCL high => stop bit\n";
            if ( bus_state == I2C_EE::WRPEND )
            {
                if ( verbose&2 )
                    cout << "I2C_EE : write is pending - commence...\n";
                start_write();
                bus_state = I2C_EE::IDLE;   // Should be busy
            }
            else
                bus_state = I2C_EE::IDLE;
        }
        else
        {
            // start bit
            if ( verbose&2 )
                cout << "I2C_EE SDA : Falling edge in SCL high => start bit\n";
            if ( ee_busy )
            {
                if ( verbose&2 )
                    cout << "             Device is busy - ignoring start bit\n";
            }
            else
            {
                bus_state = I2C_EE::START;
                bit_count = 0;
                xfr_data = 0;
            }
        }
//        cout << "I2C_EE new bus state = " << bus_state << "\n";
    }
}


void I2C_EE::reset(RESET_TYPE by)
{

  switch(by)
    {
    case POR_RESET:
        bus_state = IDLE;
        ee_busy = false;
    }

}

void I2C_EE::initialize(unsigned int new_rom_size)
{

  rom_size = new_rom_size;

  // Create the rom

  rom = (Register **) new char[sizeof (Register *) * rom_size];
  assert(rom != 0);

  // Initialize the rom

  char str[100];
  for (unsigned int i = 0; i < rom_size; i++)
    {

      rom[i] = new Register;
      rom[i]->address = i;
      rom[i]->value.put(0);
      rom[i]->alias_mask = 0;

      sprintf (str, "eeprom reg 0x%02x", i);
      rom[i]->new_name(str);

    }

  port = new IOPORT ( 2 );
  scl = new I2C_EE_SCL ( this, port, 0, "SCL" );
  sda = new I2C_EE_SDA ( this, port, 1, "SDA" );

  if(cpu) {
    cpu->ema.set_cpu(cpu);
    cpu->ema.set_Registers(rom, rom_size);
  }

}


void I2C_EE::attach ( Stimulus_Node *_scl, Stimulus_Node *_sda )
{
  _scl->attach_stimulus ( scl );
  _sda->attach_stimulus ( sda );
}


void I2C_EE::dump(void)
{
  unsigned int i, j, reg_num,v;

  cout << "     " << hex;

  // Column labels
  for (i = 0; i < 16; i++)
    cout << setw(2) << setfill('0') <<  i << ' ';

  cout << '\n';

  for (i = 0; i < rom_size/16; i++)
    {
      cout << setw(2) << setfill('0') <<  i << ":  ";

      for (j = 0; j < 16; j++)
	{
	  reg_num = i * 16 + j;
	  if(reg_num < rom_size)
	    {
	      v = rom[reg_num]->get_value();
	      cout << setw(2) << setfill('0') <<  v << ' ';
	    }
	  else
	    cout << "-- ";
	}
      cout << "   ";

      for (j = 0; j < 16; j++)
	{
	  reg_num = i * 16 + j;
	  if(reg_num < rom_size)
	    {
	      v = rom[reg_num]->get_value();
	      if( (v >= ' ') && (v <= 'z'))
		cout.put(v);
	      else
		cout.put('.');
	    }
	}

      cout << '\n';

    }
}


