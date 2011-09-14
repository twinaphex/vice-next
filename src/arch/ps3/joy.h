/******************************************************************************* 
 *  -- joy.h - Joystick support for Playstation 3 
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


#ifndef VICE_JOY_H
#define VICE_JOY_H

#ifdef __cplusplus
extern CellInputFacade *CellInput;
extern "C" {
#endif

// Joystick mapping

#define CROSS               0
#define SQUARE              1
#define TRIANGLE            2
#define CIRCLE              3
#define L1                  4
#define L2                  5
#define R1                  6
#define R2                  7
#define MAX_JOYMAP_CONTROLS 8

extern int joymap_index[2][MAX_JOYMAP_CONTROLS];

// Joystick KeyMapping
struct keys {
    const char* keyname;
    int         keycode;
};
extern struct keys keymap[];


extern int get_joymap_value (int controller, int button);
extern int get_joymap_keycode (int index);

extern int joy_arch_init(void);
extern void joystick_close(void);
extern void old_joystick_init(void);
extern void old_joystick_close(void);
extern void old_joystick(void);
extern void new_joystick_init(void);
extern void new_joystick_close(void);
extern void new_joystick(void);
extern int joystick_arch_init_resources(void);
extern int joystick_init_cmdline_options(void);

extern int joystick(void);

#ifdef __cplusplus
extern "C" {
extern bool joyswap;
}
#endif

extern int set_warp_mode(int warp);

#ifdef __cplusplus
}
#endif

#define JOYDEV_NONE      0
#define JOYDEV_NUMPAD    1
#define JOYDEV_KEYSET1   2
#define JOYDEV_KEYSET2   3
#define JOYDEV_ANALOG_0  4
#define JOYDEV_ANALOG_1  5
#define JOYDEV_ANALOG_2  6
#define JOYDEV_ANALOG_3  7
#define JOYDEV_ANALOG_4  8
#define JOYDEV_ANALOG_5  9
#define JOYDEV_DIGITAL_0 10
#define JOYDEV_DIGITAL_1 11
#define JOYDEV_USB_0     12
#define JOYDEV_USB_1     13

#endif

