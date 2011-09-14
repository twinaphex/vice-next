/******************************************************************************* 
 *  -- joy.cpp      Pad driver for Playstation 3
 *
 *     VICE PS3 -   Commodore 64 emulator for the Playstation 3
 *                  ported from the original VICE distribution
 *                  located at http://sourceforge.net/projects/vice-emu/
 *
 *  Copyright (C) 2010
 *
 *      Original Author :   Bernhard Kuhn <kuhn@eikon.e-technik.tu-muenchen.de>
 *                          Ulmer Lionel <ulmer@poly.polytechnique.fr>
 *
 *      Updated by          TimRex
 *
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

#include "cellframework/input/cellInput.h"

#include "vice.h"
#include "emu-ps3.hpp"
#include "joy.h"
#include "kbd.h"
#include "osk.h"
#include "keyboard.h"
#include "ps3debug.h"

extern "C" {
#include "autostart.h"
#include "machine.h"
#include "resources.h"
#include "videoarch.h"
#include "vsync.h"
#include "ui.h"
#include "main.h"
}

#include "in_game_menu.h"
/* ------------------------------------------------------------------------- */


// Default joystick mapping
int  joymap_index[2][MAX_JOYMAP_CONTROLS] =  {
                {JOYMAP_FIRE, OSK_SPACE, OSK_Y, OSK_N, OSK_F1, OSK_F3, OSK_F5, OSK_F7},
                {JOYMAP_FIRE, OSK_SPACE, OSK_Y, OSK_N, OSK_F1, OSK_F3, OSK_F5, OSK_F7} 
};

// Keymap structures, for keys that are mappable to joystick controls
// These are defined seperate from regular keycodes (ps3_joy.ckm) to avoid differences
// in keyboard layouts (symbolic, positional, german)

struct keys keymap[] = {
    { "None",          0 },  // unused as keymap
    { "Joystick Fire", 0 },  // unused as keymap
    { "Joystick Up",   0 },  // unused as keymap
    { "In-Game Menu",  0 },  // unused as keymap

    { "Key F1",    90000 },
    { "Key F2",    90001 },
    { "Key F3",    90002 },
    { "Key F4",    90003 },
    { "Key F5",    90004 },
    { "Key F6",    90005 },
    { "Key F7",    90006 },
    { "Key F8",    90007 },
    { "RUN/STOP",  90008 },
    { "RESTORE",   90009 },
    { "CBM",       90010 },
    { "Key Y",     90011 },
    { "Key N",     90012 },
    { "<SPACE>",   90013 },
    { "<Return>",  90014 },
};

int get_joymap_value (int controller, int button)
{
    return joymap_index[controller][button];
}

int get_joymap_keycode (int index)
{
    return keymap[index].keycode;
}




/* Joystick devices.  */

int joystick_init_cmdline_options(void)
{
	return 0;
}

void joystick_close(void)
{
	return;
}

int joystick_arch_init_resources(void)
{
	return 0;
}

int joy_arch_init(void)
{
	return 0;
}

bool joyswap=false;


void process_button (int pad_id, int pad_button, int *value) 
{
	int function;   // mapped controller funciton
	int keycode;    // keycode associated with that controller function

	// represents mapped button state for each pad controller
	static bool button_state[2][MAX_JOYMAP_CONTROLS];  

	int map_id=0;

	switch (pad_button)
	{
		case CTRL_CROSS:
			map_id = CROSS;
			break;
		case CTRL_CIRCLE:
			map_id = CIRCLE;
			break;
		case CTRL_SQUARE:
			map_id = SQUARE;
			break;
		case CTRL_TRIANGLE:
			map_id = TRIANGLE;
			break;
		case CTRL_L1:
			map_id = L1;
			break;
		case CTRL_L2:
			map_id = L2;
			break;
		case CTRL_R1:
			map_id = R1;
			break;
		case CTRL_R2:
			map_id = R2;
			break;
		default:
			return;
			break;
	}

	if (CellInput->IsButtonPressed(pad_id,pad_button))
	{
		function = get_joymap_value (pad_id,map_id);
		if (function == JOYMAP_FIRE)
			*value |= 16;
		else if (function == JOYMAP_UP)
			*value |= 1;
		else if (function == FUNC_IN_GAME_MENU)
			InGameMenuLoop();
		else
		{
			if (!button_state[pad_id][map_id])
			{
				keycode = get_joymap_keycode (function);
				if (keycode != 0)
				{
					button_state[pad_id][map_id] = true;
					keyboard_key_pressed(keycode);
				}
			}
		}
	}
	else
	{
		if (button_state[pad_id][map_id])
		{
			keycode = get_joymap_keycode (get_joymap_value(pad_id, map_id));
			if (keycode != 0)
			{
				button_state[pad_id][map_id] = false;
				keyboard_key_released(keycode);
			}
		}
	}
}


int joystick(void)
{
	static bool key_cursorup    = false;
	static bool key_cursordown  = false;
	static bool key_cursorleft  = false;
	static bool key_cursorright = false;


	static bool warp_mode=false;

	// osk_active_bufferlen == The OSK is entering characters, don't interrupt it.
	// autostart_in_progress() == 
	// Autostart is running. Don't allow the joystick to interrupt.
	// In particular this can break the actual autostart script that tests the basic prompt due to interference

	if (osk_active_bufferlen || autostart_in_progress())
	{
		return 0;
	} 

	uint8_t pads_connected = CellInput->NumberPadsConnected();
	for (uint8_t i = 0; i < pads_connected; ++i)
	{
		int value = 0;

		// Do PS3 pads
		CellInput->UpdateDevice(i);

		// Set the joystick values

		if (CellInput->IsButtonPressed(i,CTRL_LEFT) | CellInput->IsAnalogPressedLeft(i,CTRL_LSTICK))
			value |= 4;

		if (CellInput->IsButtonPressed(i,CTRL_RIGHT) | CellInput->IsAnalogPressedRight(i,CTRL_LSTICK))
			value |= 8;

		if (CellInput->IsButtonPressed(i,CTRL_UP) | CellInput->IsAnalogPressedUp(i,CTRL_LSTICK))
			value |= 1;

		if (CellInput->IsButtonPressed(i,CTRL_DOWN) | CellInput->IsAnalogPressedDown(i,CTRL_LSTICK))
			value |= 2;


		// Process re-definable controls
		process_button (i, CTRL_CROSS, &value);
		process_button (i, CTRL_CIRCLE, &value);
		process_button (i, CTRL_SQUARE, &value);
		process_button (i, CTRL_TRIANGLE, &value);
		process_button (i, CTRL_L1, &value);
		process_button (i, CTRL_L2, &value);
		process_button (i, CTRL_R1, &value);
		process_button (i, CTRL_R2, &value);


		// Emulator cursor keys
		if (CellInput->IsAnalogPressedUp(i,CTRL_RSTICK))
		{
			if (!key_cursorup)
			{
				keyboard_key_pressed((signed long)  32850);  // Cursor Up key pressed
				key_cursorup = true;
			}
		}
		else
		{
			if (key_cursorup)
			{
				keyboard_key_released((signed long) 32850);  // Cursor Up key released
				key_cursorup = false;
			}
		}

		if (CellInput->IsAnalogPressedDown(i,CTRL_RSTICK))
		{
			if (!key_cursordown) {
				keyboard_key_pressed((signed long)  32849);  // Cursor Down key pressed
				key_cursordown = true;
			}
		}
		else
		{
			if (key_cursordown)
			{
				keyboard_key_released((signed long) 32849);  // Cursor Down key released
				key_cursordown = false;
			}
		}

		if (CellInput->IsAnalogPressedLeft(i,CTRL_RSTICK)) {
			if (!key_cursorleft) {
				keyboard_key_pressed((signed long)  32848);  // Cursor Left key pressed
				key_cursorleft = true;
			}
		} else {
			if (key_cursorleft) {
				keyboard_key_released((signed long) 32848);  // Cursor Left key released
				key_cursorleft = false;
			}
		}

		if (CellInput->IsAnalogPressedRight(i,CTRL_RSTICK)) {
			if (!key_cursorright) {
				keyboard_key_pressed((signed long)  32847);  // Cursor Right key pressed
				key_cursorright = true;
			}
		} else {
			if (key_cursorright) {
				keyboard_key_released((signed long) 32847);  // Cursor Right key released
				key_cursorright = false;
			}
		}

		if ((CellInput->IsButtonPressed(0,CTRL_R2) && CellInput->IsButtonPressed(0,CTRL_L2)) || (CellInput->IsButtonPressed(1,CTRL_R2) && CellInput->IsButtonPressed(1,CTRL_L2)))
		{
			if (!warp_mode)
			{
				// Disable sound (makes warp mode infinitely faster)
				resources_set_int("Sound", 0);

				// Enable Warp Mode
				resources_set_int("WarpMode", 1);
				warp_mode=true;
			}
		}
		else
		{
			if (warp_mode)
			{
				resources_set_int("WarpMode", 0);
				resources_set_int("Sound", 1);
				warp_mode=false;
			}
		}


		if (CellInput->IsButtonPressed(i,CTRL_L1) && CellInput->IsButtonPressed(i,CTRL_R1) && CellInput->IsButtonPressed(i,CTRL_L2) && CellInput->IsButtonPressed(i,CTRL_R2) )
			machine_trigger_reset(MACHINE_RESET_MODE_HARD);


		/*
		// Swap joysticks
		if (CellInput->WasButtonPressed(i,CTRL_SELECT)) {
		// Do nothing
		}
		// Required because keyloop kills us
		if (CellInput->WasButtonReleased(i,CTRL_SELECT)) {
		if (pads_connected == 1)
		// Only allow us to swap joystick ports if there is only one controller
		// Otherwise, multiplayer madness and much punching will ensue.
		joyswap=!joyswap;
		}
		 */

		if (CellInput->WasButtonPressed(i,CTRL_SELECT))
		{
			InGameMenuLoop();
		}

		if(CellInput->WasButtonPressed(i,CTRL_START))
		{
			menu(MODE_MENU);
			// Stop the clock (effectively, tells the emulator we were paused)
			vsync_suspend_speed_eval();
			// Apply any changes as necessary.
		}

		if(CellInput->WasButtonPressed(i,CTRL_L3))
		{
			// Vice OSK
			menu(MODE_OSK);
			// Stop the clock (effectively, tells the emulator we were paused)
			vsync_suspend_speed_eval();
			// Apply any changes as necessary.
		}

		if(CellInput->WasButtonPressed(i,CTRL_R3))
		{
			debug_printf_quick ("OSK starting\n");
			if (!osk->Start(L"Characters entered here will be relayed to the emulator ", L""))
				debug_printf ("WARNING: OSK could not start\n");

			debug_printf_quick ("OSK started\n");
			// Just in case. This ensures we check to see if the screen has updated, and if not.. force one
			// The OSK fails to draw if the screen doesn't update.
			debug_printf_quick ("OSK callback check\n");
		}

		cellSysutilCheckCallback();
		if (is_sysutil_drawing())
			sysutil_callback_redraw();

		ui_callback();


		// pad 0 becomes port 2
		// pad 1 becomes port 1
		if (joyswap)
			joystick_set_value_absolute( ((i+1) % 2 ) + 1, value);  
		else
			joystick_set_value_absolute(i+1, value); 

		// pad 0 becomes port 1
		// pad 1 becomes port 2
	}

	return 0;
}


