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
class I2C_EE_PIN : public IO_open_collector
{
public:

  I2C_EE *eeprom;

  I2C_EE_PIN (I2C_EE *_eeprom, char *_name) 
    : IO_open_collector((IOPORT *)0,1,_name) { 

    eeprom = _eeprom;

    bDrivingState = true;
    bDrivenState = true;

    // Make the pin an output.
    update_direction(IO_bi_directional::DIR_OUTPUT,true);

  };

  //
  void setDrivingState(bool new_state) {

    //cout << "eeprom sda setDrivingState " << new_state << '\n';

    bDrivingState = new_state;
    bDrivenState = new_state;

    if(snode)
      snode->update();

  }

};


class I2C_EE_SDA : public I2C_EE_PIN
{
public:

  I2C_EE_SDA (I2C_EE *_eeprom, char *_name) 
    : I2C_EE_PIN (_eeprom,_name)
  {
  }

  void setDrivenState(bool new_dstate) {

    bool diff = new_dstate ^ bDrivenState;

    //cout << "eeprom sda setDrivenState " << new_dstate << '\n';
    if( eeprom && diff ) {

      //cout << "      ... that's an edge, so call handler\n";
      bDrivenState = new_dstate;
      eeprom->new_sda_edge(new_dstate);
    }

  }

};
class I2C_EE_SCL : public I2C_EE_PIN
{
public:

  I2C_EE_SCL (I2C_EE *_eeprom, char *_name) 
    : I2C_EE_PIN (_eeprom,_name)
  {
  }

  void setDrivenState(bool new_state) {

    bool diff = new_state ^ bDrivenState;

    //cout << "eeprom scl setDrivingState " << new_state << '\n';
    if( eeprom && diff ) {

      //cout << "      ... that's an edge, so call handler\n";
      bDrivenState = new_state;
      eeprom->new_scl_edge(bDrivenState);

    }

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

  bus_state = IDLE;
  ee_busy = false;

}

void I2C_EE::debug()
{

  if (!scl || !sda || !rom)
    return;

  const char *cPBusState=0;
  switch (bus_state) {
  case IDLE:
    cPBusState = "IDLE";
    break;
  case START:
    cPBusState = "START";
    break;
  case RX_CMD:
    cPBusState = "RX_CMD";
    break;
  case ACK_CMD:
    cPBusState = "ACK_CMD";
    break;
  case RX_ADDR:
    cPBusState = "RX_ADDR";
    break;
  case ACK_ADDR:
    cPBusState = "ACK_ADDR";
    break;
  case RX_DATA:
    cPBusState = "RX_DATA";
    break;
  case ACK_WR:
    cPBusState = "ACK_WR";
    break;
  case WRPEND:
    cPBusState = "WRPEND";
    break;
  case ACK_RD:
    cPBusState = "ACK_RD";
    break;
  case TX_DATA:
    cPBusState = "TX_DATA";
    break;
  }

  cout << "I2C EEPROM: current state="<<cPBusState<<endl;
  cout << " t=0x"<< hex <<get_cycles().value << endl;
  cout << "  scl drivenState="  << scl->getDrivenState()
       << " drivingState=" << scl->getDrivingState()
       << " direction=" << ((scl->get_direction()==IOPIN::DIR_INPUT) ?"IN":"OUT") 
       << endl;
  cout << "  sda drivenState="  << sda->getDrivenState()
       << " drivingState=" << sda->getDrivingState()
       << " direction=" << ((sda->get_direction()==IOPIN::DIR_INPUT) ?"IN":"OUT") 
       << endl;

  cout << "  bit_count:"<<bit_count
       << " ee_busy:"<<ee_busy
       << " xfr_addr:0x"<<hex<<xfr_addr
       << " xfr_data:0x"<<hex<<xfr_data
       << endl;

}
Register * I2C_EE::get_register(unsigned int address)
{

  if ( address < rom_size )
    return rom[address];
  return 0;

}


void I2C_EE::start_write(void)
{

  //cout << "I2C_EE::start_write() \n";

    get_cycles().set_break(get_cycles().value + I2C_EE_WRITE_TIME, this);

    rom[xfr_addr]->put ( xfr_data );
    ee_busy = true;

}


void I2C_EE::write_is_complete(void) 
{
}



void I2C_EE::callback()
{

  ee_busy = false;
  if(verbose)
    cout << "I2C_EE::callback() - write cycle is complete\n";
}

void I2C_EE::callback_print()
{
  cout << "Internal I2C-EEPROM\n";
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
  int curBusState = bus_state;
  if (verbose) {
    cout << "I2C_EE::new_scl_edge: "<<direction<<endl;
    debug();
  }
    if ( direction )
    {
        // Rising edge
        nxtbit = sda->getDrivenState();
        //cout << "I2C_EE SCL : Rising edge, data=" << nxtbit << "\n";
    }
    else
    {
        // Falling edge
        //cout << "I2C_EE SCL : Falling edge\n";
        switch ( bus_state )
        {
            case I2C_EE::IDLE :
                sda->setDrivingState ( true );
                break;

            case I2C_EE::START :
                sda->setDrivingState ( true );
                bus_state = I2C_EE::RX_CMD;
                break;

            case I2C_EE::RX_CMD :
                if ( shift_read_bit ( sda->getDrivenState() ) )
                {
                    if ( verbose )
                        cout << "I2C_EE : got command " << hex << xfr_data;
                    if ( ( xfr_data & 0xf0 ) == 0xA0 )
                    {
                        bus_state = I2C_EE::ACK_CMD;
                        if ( verbose )
                            cout << " - OK\n";
                        sda->setDrivingState ( false );
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
                sda->setDrivingState ( true );
                if ( xfr_data & 0x01 )
                {
                    // it's a read command
                    bus_state = I2C_EE::TX_DATA;
                    bit_count = 8;
                    xfr_data = rom[xfr_addr]->get();
                    sda->setDrivingState ( shift_write_bit() );
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
                if ( shift_read_bit ( sda->getDrivenState() ) )
                {
                    sda->setDrivingState ( false );
                    bus_state = I2C_EE::ACK_ADDR;
                    xfr_addr = xfr_data % rom_size;
                    if ( verbose )
                        cout << "I2C_EE : address set to " << hex << xfr_addr << 
                                "  (raw " << xfr_data << ", rom size " << rom_size << ")\n";
                }
                break;

            case I2C_EE::ACK_ADDR :
                sda->setDrivingState ( true );
                bus_state = I2C_EE::RX_DATA;
                bit_count = 0;
                xfr_data = 0;
                break;

            case I2C_EE::RX_DATA :
                if ( shift_read_bit ( sda->getDrivenState() ) )
                {
                    if ( verbose )
                        cout << "I2C_EE : data set to " << hex << xfr_data << "\n";
                    sda->setDrivingState ( false );
                    bus_state = I2C_EE::ACK_WR;
                }
                break;

            case I2C_EE::ACK_WR :
                sda->setDrivingState ( true );
                bus_state = I2C_EE::WRPEND;
                break;

            case I2C_EE::WRPEND :
                // We were about to do the write but got more data instead
                // of the expected stop bit
                xfr_data = sda->getDrivenState();
                bit_count = 1;
                bus_state = I2C_EE::RX_DATA;
                if ( verbose )
                    cout << "I2C_EE : write postponed by extra data\n";
                break;

            case I2C_EE::TX_DATA :
                if ( bit_count == 0 )
                {
                    sda->setDrivingState ( true );     // Release the bus
                    xfr_addr++;
                    xfr_addr %= rom_size;
                    bus_state = I2C_EE::ACK_RD;
                }
                else
                {
                    sda->setDrivingState ( shift_write_bit() );
                }
                break;

            case I2C_EE::ACK_RD :
                if ( sda->getDrivenState() == false )
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
                sda->setDrivingState ( true );     // Release the bus
                break;
        }
    }
    if (verbose && bus_state != curBusState) {
      cout << "I2C_EE::new_scl_edge() new bus state = " << bus_state << "\n";
      debug();
    }
}


void I2C_EE::new_sda_edge ( bool direction )
{
  if (verbose) {
    cout << "I2C_EE::new_sda_edge: direction:"<<direction<<endl;
    debug();
  }

    if ( scl->getDrivenState() )
    {
      int curBusState = bus_state;
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

    if (verbose && bus_state != curBusState) {
      cout << "I2C_EE::new_sda_edge() new bus state = " << bus_state << "\n";
      debug();
    }

    }
}


void I2C_EE::reset(RESET_TYPE by)
{

  switch(by)
    {
    case POR_RESET:
        bus_state = IDLE;
        ee_busy = false;
	break;
    default:
      break;
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

  scl = new I2C_EE_SCL ( this, "SCL" );
  sda = new I2C_EE_SDA ( this, "SDA" );

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


