/*
   Copyright (C) 2004 Chris Emerson
   (based on push button by Carlos Ghirardelli)

Thiss file is part of gpsim.

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


#ifndef __ENC_H__
#define __ENC_H__

/* IN_MODULE should be defined for modules */
#define IN_MODULE

#include "../src/stimuli.h"
#include "../src/ioports.h"
#include "../src/symbol.h"
#include "../src/trace.h"
#include "../src/modules.h"

#include <gtk/gtk.h>

class Encoder : public Module, private TriggerObject
{
    void create_widget(Encoder *enc);
public:

    IOPORT *enc_port;
    IOPIN *a_pin;
    IOPIN *b_pin;

    Encoder(void);
    ~Encoder(void);


    void test(void);
    void update(void);

    /* Send clockwise pulses */
    void send_cw(void);
    /* Send anticlockwise pulses */
    void send_ccw(void);

    // Inheritances from the Package class
    virtual void create_iopin_map(void);

    // Inheritance from Module class
    const virtual char *type(void) { return ("encoder"); };
    static Module *construct(const char *new_name);

private:
    enum rotate_state {
	rot_detent,
	rot_moving_cw,
	rot_moving_ccw
    };
    rotate_state rs;
    void toggle_a();
    void toggle_b();
    void schedule_tick();
    void callback();
};

#endif //  __ENC_H__
