/*
   Copyright (C) 2000 T. Scott Dattalo

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


/* IN_MODULE should be defined for modules */
#define IN_MODULE

#include "lcd.h"

/*
  lcdengine.cc

  This is where the behavioral simulation of the LCD is performed.
  The LCD is simulated with a state machine. The states are defined

  POWERON,
  ST_INITIALIZED,
  ST_COMMAND_PH0,
  ST_STATUS_READ,
  ST_DATA_READ,
  
*/



/****************************************************************
 *
 * Now the details for simulating the LCD 
 */

/*
 * LCD Command "Set Display Data RAM Address" = 10000000
 */

static const int LCD_CMD_SET_DDRAM  = 0x80;
static const int LCD_MASK_SET_DDRAM = 0x80;

/*
 * LCD Command "Set Display Character Generator RAM Address" = 01aaaaaa
 */

static const int LCD_CMD_SET_CGRAM  = 0x40;
static const int LCD_MASK_SET_CGRAM = 0xc0;


/*
 * LCD Command Function Set =  001dnfxx
 *  d = 1 for 8-bit interface or 0 for 4-bit interface
 *  n = for 2 line displays, n=1 allows both lines to be displayed
 *      while n=0 only allows the first.
 * f = font size. f=1 is for 5x11 dots while f=0 is for 5x8 dots.
 */

static const int LCD_CMD_FUNC_SET  = 0x20;    // LCD Command "Function Set"
static const int LCD_MASK_FUNC_SET = 0xe0;    // 
static const int LCD_4bit_MODE     = 0x00;    // d=0
static const int LCD_8bit_MODE     = 0x10;    // d=1
static const int LCD_1_LINE        = 0x00;    // n=0
static const int LCD_2_LINES       = 0x08;    // n=1
static const int LCD_SMALL_FONT    = 0x00;    // f=0
static const int LCD_LARGE_FONT    = 0x04;    // f=1

/*
 * LCD Command "Cursor Display" = 0001sdxx
 *  s = 1 Sets cursor-move or display-shift
 *  d = 1 Shift right 0 = shift left
 */

static const int LCD_CMD_CURSOR_DISPLAY   = 0x10;   // LCD Command "Cursor Display"
static const int LCD_MASK_CURSOR_DISPLAY  = 0xf0;   // 

/*
 * LCD Command Display Control = 00001dcb
 *  d = 1 turn display on or 0 to turn display off
 *  c = 1 turn cursor on or 0 to turn cursor off
 *  b = 1 blinking cursor or 0 non-blinking cursor
 */

static const int LCD_CMD_DISPLAY_CTRL  = 0x08;    // LCD Command "Display Control"
static const int LCD_MASK_DISPLAY_CTRL = 0xf8;    // 
static const int LCD_DISPLAY_OFF       = 0x00;    // d=0
static const int LCD_DISPLAY_ON        = 0x04;    // d=1
static const int LCD_CURSOR_OFF        = 0x00;    // c=0
static const int LCD_CURSOR_ON         = 0x02;    // c=1
static const int LCD_BLINK_OFF         = 0x00;    // b=0
static const int LCD_BLINK_ON          = 0x01;    // b=1


/*
 * LCD Command "Entry Mode" = 000001is
 *  i = 1 to increment or 0 to decrement the DDRAM address after each DDRAM access.
 *  s = 1 to scroll the display in the direction specified by the i-bit when the
 *       cursor reaches the edge of the display window.
 */

static const int LCD_CMD_ENTRY_MODE  = 0x04;    // LCD Command "Entry Mode"
static const int LCD_MASK_ENTRY_MODE = 0xfc;    // 
static const int LCD_DEC_CURSOR_POS  = 0x00;    // i=0
static const int LCD_INC_CURSOR_POS  = 0x02;    // i=1
static const int LCD_NO_SCROLL       = 0x00;    // s=0
static const int LCD_SCROLL          = 0x01;    // s=1

/*
 * LCD Command "Cursor Home" = 0000001x
 */

static const int LCD_CMD_CURSOR_HOME   = 0x02;   // LCD Command "Cursor Home"
static const int LCD_MASK_CURSOR_HOME  = 0xfe;   // 

// LCD Command Clear Display = 00000001
static const int LCD_CMD_CLEAR_DISPLAY  = 0x01;
static const int LCD_MASK_CLEAR_DISPLAY = 0xff;




//
// PowerON
//
// Power has just been applied to the LCD. Now it's waiting
// for a command to switch it into either 4 or 8 bit mode
//
/*
void LcdDisplay::STPowerON( Event e)
{

  switch (e) {
  case EWC:
    start_command();
    break;

  default:
    break;
  }
}
*/

void LcdDisplay::latch_data(void)
{
    if ( debug & LCD_DEBUG_TRACE_PORT )
        cout << "LCD:latch_data reads " << data_port->get() << "\n";

  if(in_8bit_mode())
    data_latch = data_port->get() & 0xff;
  else {
    // 4-bit mode.
    data_latch = ( (data_latch << 4) | ((data_port->get() & 0xf0)>>4) ) & 0xff;
    data_latch_phase ^= 1;
  }
}

void LcdDisplay::start_data(void)
{

  newState(ST_COMMAND_PH0);
}

void LcdDisplay::send_status(void)
{
  unsigned short status;
  

  status = ( cursor.row * 0x40 ) + cursor.col;

  if ( busyTimer.isBusy() ) {
    status |= 0x80;
  }

  if(in_8bit_mode()) {
    data_port->update_pin_directions(true);
    data_port->put ( status );
  } else {
    data_port->update_pin_directions(true);

    if (data_latch_phase & 1 )
      data_port->put ( status );
    else
      data_port->put ( status << 4 );

    data_latch_phase ^= 1;
  }

  newState(ST_STATUS_READ);
}

void LcdDisplay::read_data(void)
{
  unsigned char data = 0;
  bool bump_cursor = false;

  if (in_cgram)
    data = cgram[cgram_cursor];
  else
    data = ch_data[cursor.row][cursor.col];

  if (debug)
    cout << "Read data: 0x" << data << endl;

  if(in_8bit_mode()) {
    data_port->update_pin_directions(true);
    data_port->put ( data );
    bump_cursor = true;
  } else {
    data_port->update_pin_directions(true);

    if (data_latch_phase & 1 ) {
      data_port->put ( data & 0xf0 );
    } else {
      data_port->put ( data << 4 );
      bump_cursor = true;
    }

    data_latch_phase ^= 1;
  }

  if (debug)
    cout << "bump_cursor=" << (bump_cursor ? "true" : "false") << endl;
  if (bump_cursor) {
    if (in_cgram) {
      cgram_cursor = (cgram_cursor + 1) & CGRAM_MASK;
    } else {
      // FIXME: cursor behavior should be emulated more carefully
      // this requires some experimentation with an actual device (and
      // probably a more accurate modelling of DDRAM). For now, treat
      // DDRAM as a circular buffer sized rows*columns
      cursor.col++;
      if (cursor.col >= cols) {
        cursor.col = 0;
        cursor.row++;
        if (cursor.row >= rows)
          cursor.row = 0;
      }
    }
  }

  newState(ST_DATA_READ);
}

void LcdDisplay::release_port(void)
{
  data_port->update_pin_directions(false);
  newState(ST_INITIALIZED);
}

void LcdDisplay::execute_command(void)
{
  if(debug)
    cout << "execute command:  ";

  //
  // Determine the command type
  //

  if( (data_latch & LCD_MASK_SET_DDRAM) ==  LCD_CMD_SET_DDRAM) {
    if(debug)
      cout << "LCD_CMD_SET_DDRAM\n";
    write_ddram_address(data_latch & 0x7f);
    busyTimer.set(39e-6);	// busy for 39 usec after set DDRAM addr
  }
  else if( (data_latch & LCD_MASK_SET_CGRAM) ==  LCD_CMD_SET_CGRAM) {
    if(debug)
      cout << "LCD_CMD_SET_CGRAM\n";
    write_cgram_address(data_latch & 0x3f);
  }
  else if( (data_latch & LCD_MASK_FUNC_SET) == LCD_CMD_FUNC_SET) {

    if(debug)
      cout << "LCD_CMD_FUNC_SET\n";

    //
    // Check the bits in the command
    //

    if(data_latch & LCD_8bit_MODE)
      set_8bit_mode();
    else
      set_4bit_mode();

    if(data_latch & LCD_2_LINES)
      set_2line_mode();
    else
      set_1line_mode();

    if(data_latch & LCD_LARGE_FONT)
      set_large_font_mode();
    else
      set_small_font_mode();

    busyTimer.set(39e-6);	// busy for 39 usec after DDRAM write
  }
  else if( (data_latch & LCD_MASK_CURSOR_DISPLAY) ==  LCD_CMD_CURSOR_DISPLAY) {
    cout << "LCD_CMD_CURSOR_DISPLAY\n";
    cout << "NOT SUPPORTED\n";
  }
  else if( (data_latch & LCD_MASK_DISPLAY_CTRL) == LCD_CMD_DISPLAY_CTRL) {

    if(debug)
      cout << "LCD_CMD_DISPLAY_CTRL\n";

    if(data_latch & LCD_DISPLAY_ON)
      set_display_on();
    else
      set_display_off();

    if(data_latch & LCD_CURSOR_ON)
      set_cursor_on();
    else
      set_cursor_off();

    if(data_latch & LCD_BLINK_ON)
      set_blink_on();
    else
      set_blink_off();
  }
  else if( (data_latch & LCD_MASK_ENTRY_MODE) == LCD_CMD_ENTRY_MODE) {
    if ((data_latch & ~LCD_MASK_ENTRY_MODE) != LCD_INC_CURSOR_POS) {
      cout << "LCD_CMD_ENTRY_MODE\n";
      cout << "NOT SUPPORTED\n";
    } else if(debug) {
      cout << "LCD_CMD_ENTRY_MODE cursorpos=inc scroll=no\n";
    }
  }
  else if( (data_latch & LCD_MASK_CURSOR_HOME) ==  LCD_CMD_CURSOR_HOME) {
    if(debug)
      cout << "LCD_CMD_CURSOR_HOME\n";
    move_cursor(0,0);
  }
  else if( (data_latch & LCD_MASK_CLEAR_DISPLAY) == LCD_CMD_CLEAR_DISPLAY) {
    if(debug)
      cout << "LCD_CMD_CLEAR_DISPLAY\n";
    clear_display();
    busyTimer.set(1350e-6);	// busy for 1.3 msec after clear screen
  }
  else
    cout << "UNKOWN command : 0x" << hex << data_latch << '\n';

  // If the lcd is in '4-bit' mode, then this flag
  // will tell us which four bits are being written.
  data_latch_phase = 1;

}


//--------------------------------------------------
//
//   new_command
//
// The state machine will call `new_cmmand' when it enters
// the eWC state.

void LcdDisplay::new_command(void)
{

  if(in_8bit_mode())
    execute_command();

  else {
    // 4-bit mode 
    if(data_latch_phase & 1) 
      execute_command();
  }

  newState(ST_INITIALIZED);
}


//--------------------------------------------------
//
//   new_data
//

void LcdDisplay::new_data(void)
{

  if(in_8bit_mode())
    write_data(data_latch);

  else {
    // 4-bit mode 
    if(data_latch_phase & 1) 
      write_data(data_latch);
  }

  newState(ST_INITIALIZED);
}

//--------------------------------------------------
//
//   newState
//

void LcdDisplay::newState(State s)
{
  previous_state = current_state;
  current_state = s;
}

//--------------------------------------------------
//
//   revertState
//

void LcdDisplay::revertState(void)
{
  current_state = previous_state;
}

//--------------------------------------------------
static void debug_events(LcdDisplay *lcd, ControlLineEvent e, State s)
{
  if(!lcd)
    return;


  cout << "Event:  " 
       << lcd->getEventName(lcd->last_event) 
       << "->" << lcd->getEventName(e) 
       << endl;
  cout << " from State:  " 
       << lcd->getStateName(s) << '\n';


}

//--------------------------------------------------
//
//   advanceState
//

void LcdDisplay::advanceState( ControlLineEvent e)
{

  if(debug)
    debug_events(this, e, current_state);

  int eventTransition = last_event ^ e;

#define E_transition   (EWD ^ eWD)
#define RW_transition  (EWD ^ ERD)
#define CD_transition  (EWD ^ EWC)

  if(e == DataChange)
    return;

  if(!(eventTransition & E_transition))
    return;

  switch(current_state) {
  case ST_INITIALIZED:
  case POWERON:
    switch(e) {
    case  ERC:  // Active Read Command 
      send_status();
      break;
    case  EWD:  // Active Write Data
      start_data();
      break;
    case  EWC:  // Active Write Command
      start_data();
      break;
    case  ERD:
      read_data();
      break;

    case  eRD:
    case  eRC:
    case  eWD:
    case  eWC:
      if(eventTransition & E_transition) {
	debug_events(this, e, current_state);
	cout << "?? unhandled state transition\n";
	newState(ST_INITIALIZED);
      }
      break;
    }

    break;

  case ST_COMMAND_PH0:
    switch(e) {
    case  eWD:
      latch_data();
      new_data();
      break;
    case  eWC:
      latch_data();
      new_command();
      break;
    case  eRD:
    case  eRC:
    case  ERD:
    case  ERC:
    case  EWD:
    case  EWC:
      debug_events(this, e, current_state);
      cout << "?? unhandled state transition\n";
      newState(ST_INITIALIZED);
      break;
    }

    break;

  case ST_STATUS_READ:
    switch(e) {
    case  eRD:
    case  eRC:
    case  eWD:
    case  eWC:
      release_port();
      break;

    case  ERD:
    case  ERC:
    case  EWD:
    case  EWC:
      debug_events(this, e, current_state);
      cout << "?? unhandled state transition\n";
      newState(ST_INITIALIZED);
      break;
    }

  case ST_DATA_READ:
    switch(e) {
    case  eRD:
    case  eRC:
    case  eWD:
    case  eWC:
      release_port();
      break;

    case  ERD:
    case  ERC:
    case  EWD:
    case  EWC:
      debug_events(this, e, current_state);
      cout << "?? unhandled state transition\n";
      newState(ST_INITIALIZED);
      break;
    }
    
  default:
    
    break;

  }

  last_event = e;

  if(debug) {
    cout << " to State:  " << getStateName(current_state) << '\n';

    viewInternals(0);

  }


}

char * LcdDisplay::getStateName(State s)
{

  switch(s) {
  case POWERON:
    return "power on";
  case ST_INITIALIZED:
    return "initialized";
  case ST_COMMAND_PH0:
    return "command start";
  case ST_STATUS_READ:
    return "reading status";
  case ST_DATA_READ:
    return "reading data";

  default:
    break;
  }


  return "unknown state";
}

char *LcdDisplay::getEventName( ControlLineEvent e)
{
  switch(e) {
  case  eRD:
    return "eRD";
  case  eRC:
    return "eRC";
  case  eWD:
    return "eWD";
  case  eWC:
    return "eWC";
  case  ERD:
    return "ERD";
  case  ERC:
    return "ERC";
  case  EWD:
    return "EWD";
  case  EWC:
    return "EWC";
  case  DataChange:
    return "Data Change";
  default:
    break;
  }

  return "unknown state";
}

void LcdDisplay::viewInternals(int verbosity)
{

  cout << "Lcd Display: " << name() << '\n';
  cout << "Current state: " << getStateName(current_state) << '\n';
  cout << "Last event: " << getEventName(last_event) << '\n';

  if(verbosity>1) {

    cout << "  " << ( (in_8bit_mode()) ? '8' : '4') << "bit mode\n";
    cout << "  " << ( (in_2line_mode()) ? '2' : '1') << "-line mode\n";
    cout << "  " << ( (in_large_font_mode()) ? "large" : "small") << " font\n";
    cout << "  " << " Control = 0x" << hex << control_port->value.get() << endl;
    cout << "  " << " Data = 0x" << hex << data_port->value.get() << endl;

  }

}

void LcdDisplay::InitStateMachine(void)
{

  ControlEvents[0].init(eWC, "eWC");
  ControlEvents[1].init(eWD, "eWD");
  ControlEvents[2].init(eRC, "eRC");
  ControlEvents[3].init(eRD, "eRD");
  ControlEvents[4].init(EWC, "EWC");
  ControlEvents[5].init(EWD, "EWD");
  ControlEvents[6].init(ERC, "ERC");
  ControlEvents[7].init(ERD, "ERD");

  current_state = POWERON;

  int j;
  
  for(j=0; j<cols; j++)
    ch_data[0][j] = 255;

  for(j=0; j<cols; j++)
    ch_data[1][j] = 0;

  if(debug) test();

}
static void debug_print(int success, char const *m)
{
  if (success)
    cout << "SUCCESS: ";
  else
    cout << " FAILED: ";

  cout << m;

}
void LcdDisplay::test(void)
{
  const unsigned int EWC=4;
  const unsigned int eWC=0;
  const unsigned int EWD=5;
  const unsigned int eWD=0;

  set_8bit_mode();

  data_port->put(LCD_CMD_FUNC_SET | LCD_8bit_MODE);
  control_port->put(4);
  control_port->put(0);
  debug_print(in_8bit_mode(), " setting 8-bit mode\n");

  data_port->put(LCD_CMD_FUNC_SET | LCD_4bit_MODE);
  control_port->put(4);
  control_port->put(0);
  debug_print(in_4bit_mode(), " setting 4-bit mode\n");


  data_port->put(LCD_CMD_FUNC_SET | LCD_4bit_MODE | LCD_2_LINES | LCD_SMALL_FONT);
  control_port->put(4);
  control_port->put(0);

  data_port->put((LCD_CMD_FUNC_SET | LCD_4bit_MODE | LCD_2_LINES | LCD_SMALL_FONT)<<4);
  control_port->put(4);
  control_port->put(0);
  debug_print(in_2line_mode(), " setting 2-line mode\n");

  data_port->put(LCD_CMD_DISPLAY_CTRL | LCD_DISPLAY_ON); //LCD_CURSOR_OFF | LCD_BLINK_OFF
  control_port->put(4);
  control_port->put(0);

  data_port->put((LCD_CMD_DISPLAY_CTRL | LCD_DISPLAY_ON)<<4);
  control_port->put(4);
  control_port->put(0);
  debug_print(display_is_on(), " turning on display\n");

   
  data_port->put(LCD_CMD_CLEAR_DISPLAY);
  control_port->put(4);
  control_port->put(0);

  data_port->put(LCD_CMD_CLEAR_DISPLAY << 4);
  control_port->put(4);
  control_port->put(0);

  char *s ="ASHLEY & AMANDA";
  int l = strlen(s);

  for(int i=0; i<l; i++) {
    data_port->put(s[i]);
    control_port->put(5);
    control_port->put(1);

    data_port->put(s[i] << 4);
    control_port->put(5);
    control_port->put(1);
  }

  set_8bit_mode();

  viewInternals(0xff);
}
