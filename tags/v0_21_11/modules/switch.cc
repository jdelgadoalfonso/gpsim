/*
   Copyright (C) 2002 Ralf Forsberg

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

  switch.cc

  This is a module that displays a togglebutton on the screen and
  puts the togglebutton state on its output pin.

*/

/* IN_MODULE should be defined for modules */
#define IN_MODULE

#include <time.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <errno.h>
#include <stdlib.h>
#include <string>
#include <iostream>

#include "../config.h"    // get the definition for HAVE_GUI
#ifdef HAVE_GUI

#include <gtk/gtk.h>
#include "../src/packages.h"
#include "switch.h"


//--------------------------------------------------------------
// create_iopin_map 
//
//  This is where the information for the Module's package is defined.
// Specifically, the I/O pins of the module are created.

void Switch::create_iopin_map(void)
{


    // Create an I/O port to which the I/O pins can interface
    //   The module I/O pins are treated in a similar manner to
    //   the pic I/O pins. Each pin has a unique pin number that
    //   describes it's position on the physical package. This
    //   pin can then be logically grouped with other pins to define
    //   an I/O port.


    switch_port = new IOPORT(1);
    switch_port->value.put(0);
    switch_port->valid_iopins = 0x01;


    // Here, we name the port `pin'. So in gpsim, we will reference
    //   the bit positions as U1.pin0, U1.pin1, ..., where U1 is the
    //   name of the logic gate (which is assigned by the user and
    //   obtained with the name() member function call).

    char *pin_name = (char*)name().c_str();   // Get the name of this switch
    if(pin_name) {
	switch_port->new_name(pin_name);
    }



    // Define the physical package.
    //   The Package class, which is a parent of all of the modules,
    //   is responsible for allocating memory for the I/O pins.
    //


    create_pkg(1);


    // Define the I/O pins and assign them to the package.
    //   There are two things happening here. First, there is
    //   a new I/O pin that is being created. For the binary
    //   indicator, both pins are inputs. The second thing is
    //   that the pins are "assigned" to the package. If we
    //   need to reference these newly created I/O pins (like
    //   below) then we can call the member function 'get_pin'.

    assign_pin(1, new IO_bi_directional(switch_port, 0,"out"));
    package->set_pin_position(1,2.5); // Position pin on middle right side of package

    // Create an entry in the symbol table for the new I/O pins.
    // This is how the pins are accessed at the higher levels (like
    // in the CLI).

    switch_pin = get_pin(1);
    if(switch_pin)
    {
	get_symbol_table().add_stimulus(switch_pin);
	switch_pin->update_direction(1,true);
	if(switch_pin->snode)
	    switch_pin->snode->update(0);
    }

}

//--------------------------------------------------------------
// GUI
static void
toggle_cb (GtkToggleButton *button, Switch *sw)
{
    int state = gtk_toggle_button_get_active(button);

    sw->switch_port->put_value(state);
}


void Switch::create_widget(Switch *sw)
{
    GtkWidget *box1;
    GtkWidget *button;

    box1 = gtk_vbox_new (FALSE, 0);

    button = gtk_toggle_button_new_with_label ((char*)sw->name().c_str());
    gtk_container_set_border_width (GTK_CONTAINER (button), 5);
    gtk_signal_connect (GTK_OBJECT (button), "toggled",
			GTK_SIGNAL_FUNC (toggle_cb), (gpointer)sw);
    gtk_widget_show(button);
    gtk_box_pack_start (GTK_BOX (box1), button, FALSE, FALSE, 0);

    // Tell gpsim which widget to use in breadboard.
    sw->set_widget(box1);
}

//--------------------------------------------------------------
// construct
Module * Switch::construct(const char *_new_name=0)
{

//    cout << " Switch constructor\n";

    Switch *switchP = new Switch ;
    switchP->new_name(_new_name);
    switchP->create_iopin_map();

    switchP->create_widget(switchP);

    return switchP;

}

Switch::Switch(void)
{

  name_str = strdup("Switch");
  
  //interface_id = gpsim_register_interface((gpointer)this);
}

Switch::~Switch(void)
{
//    cout << "Switch destructor\n";

    delete switch_port;

}
#endif // HAVE_GUI
