/******************************************************************************* 
 *  -- ui.h -       Basic debug functions for Playstation 3, used to genearte
 *                  HUD syle overlay of status info
 *
 *     VICE PS3 -   Commodore 64 emulator for the Playstation 3
 *                  ported from the original VICE distribution
 *                  located at http://sourceforge.net/projects/vice-emu/
 *
 *
 *  Copyright (C) 2010
 *       TimRex
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ********************************************************************************/

#ifndef VICE_UI_H_
#define VICE_UI_H_

#include "vice.h"

enum Emulator_Modes
{
	MODE_MENU,
	MODE_OSK,
	MODE_EMULATION,
	MODE_EXIT
};


struct drive_hid {
    unsigned int led_pwm1;
    unsigned int led_pwm2;
    int          led_color;
    unsigned int drive_base;
    unsigned int half_track_number;
};

struct tape_hid {
    int status;  // 0 or 1 - denotes if an image is loaded - doesn't work with T64 images, only TAP images
    int motor;   // 0 or 1
    int control; // 0 thru 6
    int counter;
};

struct hid {
    int   display_speed;
    int   display_drives;
    struct drive_hid drive8;
    struct drive_hid drive9;
    struct drive_hid drive10;
    struct drive_hid drive11;
    struct tape_hid tape;
    float speed;
    float frame_rate;
    int   warp_enabled;
};
extern struct hid ui;

extern int ui_resources_init(void);

/* ------------------------------------------------------------------------- */
/* Prototypes */

struct video_canvas_s;
struct palette_s;

void ui_display_speed(float percent, float framerate, int warp_flag);
void ui_dispatch_events(void);
extern void archdep_ui_init(int argc, char *argv[]);

int ui_callback(void); // used by ui_callback_alarm


// Extern'ed from emulator.cpp
int cellInit(void);
# endif
