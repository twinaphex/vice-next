/******************************************************************************* 
 *  -- kbd.cpp      Keyboard driver for Playstation 3
 *
 *     VICE PS3 -   Commodore 64 emulator for the Playstation 3
 *                  ported from the original VICE distribution
 *                  located at http://sourceforge.net/projects/vice-emu/
 *
 *  Copyright (C) 2010
 *
 *      Author    TimRex
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


#include "vice.h"

#include <stdio.h>
#include <cell/keyboard.h>

#include "kbd.h"
#include "keyboard.h"

#include "ps3debug.h"

extern "C" {
#include "log.h"
}

#define MAX_KEYBD 2

OSKUtil *osk;
uint8_t old_status[MAX_KEYBD];

void kbd_arch_init(void)
{
	if (cellKbInit(1) != CELL_OK)
	{
		log_warning (LOG_DEFAULT, "WARNING: Keyboard failed to initialise");
		return;
	}

	if (cellKbSetCodeType (0, CELL_KB_CODETYPE_ASCII) != CELL_KB_OK)
	{
		log_warning (LOG_DEFAULT, "Unable to set KB_CODETYPE_ASCII");
		return;
	}

	// Packet mode or Character mode?
	//CELL_KB_RMODE_PACKET
	if (cellKbSetReadMode (0, CELL_KB_RMODE_INPUTCHAR) != CELL_KB_OK)
	{
		log_warning (LOG_DEFAULT, "WARNING: Unable to set CELL_KB_RMODE_INPUTCHAR");
		return;
	}
}

void kbd_arch_destroy(void)
{
	cellKbEnd();
	return;
}


static CellKbInfo status; 
uint32_t old_info = 0;

#define MAX_KEYS 8

//Unlikely we'll ever want to register more than 8 keys at a time
static uint16_t keysdown[MAX_KEYS];
static unsigned int mkey;

#define L_CTRL  300
#define L_SHIFT 301
#define L_ALT   302
#define L_WIN   303

#define R_CTRL  304
#define R_SHIFT 305
#define R_ALT   306
#define R_WIN   307

void kbd_process(void)
{
	static CellKbData kdata;

	if (cellKbGetInfo (&status) != CELL_KB_OK)
		return;

	if((status.info & CELL_KB_INFO_INTERCEPTED) && (!(old_info & CELL_KB_INFO_INTERCEPTED)))
	{
		log_message (LOG_DEFAULT, "INFO: Lost Keyboard\n");
		old_info = status.info;
	}
	else if((!(status.info & CELL_KB_INFO_INTERCEPTED)) && (old_info & CELL_KB_INFO_INTERCEPTED))
	{
		log_message (LOG_DEFAULT, "INFO: Found Keyboard\n");
		old_info = status.info;
	}
	if ( (old_info != CELL_KB_STATUS_DISCONNECTED) && (status.info == CELL_KB_STATUS_DISCONNECTED))
	{
		log_message (LOG_DEFAULT, "Keyboard has disconnected\n");
		old_info = status.info;
		return;
	}

	if (old_status == CELL_KB_STATUS_DISCONNECTED)
		log_message (LOG_DEFAULT, "Keyboard has connected\n");

	if (cellKbRead (0, &kdata) != CELL_KB_OK)
		return;

	if ( (kdata.len == 0) )   // even the mkey shows kdata.len == 1
		return;

	// First, check for modifier keys and apply them.

	if ( !(mkey & CELL_KB_MKEY_L_CTRL) && (kdata.mkey & CELL_KB_MKEY_L_CTRL))
	{
		keyboard_key_pressed (L_CTRL);
	}
	if ((mkey & CELL_KB_MKEY_L_CTRL) && !(kdata.mkey & CELL_KB_MKEY_L_CTRL))
	{
		keyboard_key_released (L_CTRL);
	}

	if ( !(mkey & CELL_KB_MKEY_L_SHIFT) && (kdata.mkey & CELL_KB_MKEY_L_SHIFT))
	{
		keyboard_key_pressed (L_SHIFT);
	}
	if ((mkey & CELL_KB_MKEY_L_SHIFT) && !(kdata.mkey & CELL_KB_MKEY_L_SHIFT)) {
		keyboard_key_released (L_SHIFT);
	}

	if ( !(mkey & CELL_KB_MKEY_L_ALT) && (kdata.mkey & CELL_KB_MKEY_L_ALT)) {
		keyboard_key_pressed (L_ALT);
	}
	if ((mkey & CELL_KB_MKEY_L_ALT) && !(kdata.mkey & CELL_KB_MKEY_L_ALT)) {
		keyboard_key_released (L_ALT);
	}

	if ( !(mkey & CELL_KB_MKEY_L_WIN) && (kdata.mkey & CELL_KB_MKEY_L_WIN)) {
		keyboard_key_pressed (L_WIN);
	}
	if ((mkey & CELL_KB_MKEY_L_WIN) && !(kdata.mkey & CELL_KB_MKEY_L_WIN)) {
		keyboard_key_released (L_WIN);
	}

	if ( !(mkey & CELL_KB_MKEY_R_CTRL) && (kdata.mkey & CELL_KB_MKEY_R_CTRL)) {
		keyboard_key_pressed (R_CTRL);
	}
	if ((mkey & CELL_KB_MKEY_R_CTRL) && !(kdata.mkey & CELL_KB_MKEY_R_CTRL)) {
		keyboard_key_released (R_CTRL);
	}

	if ( !(mkey & CELL_KB_MKEY_R_SHIFT) && (kdata.mkey & CELL_KB_MKEY_R_SHIFT)) {
		keyboard_key_pressed (R_SHIFT);
	}
	if ((mkey & CELL_KB_MKEY_R_SHIFT) && !(kdata.mkey & CELL_KB_MKEY_R_SHIFT)) {
		keyboard_key_released (R_SHIFT);
	}

	if ( !(mkey & CELL_KB_MKEY_R_ALT) && (kdata.mkey & CELL_KB_MKEY_R_ALT)) {
		keyboard_key_pressed (R_ALT);
	}
	if ((mkey & CELL_KB_MKEY_R_ALT) && !(kdata.mkey & CELL_KB_MKEY_R_ALT)) {
		keyboard_key_released (R_ALT);
	}

	if ( !(mkey & CELL_KB_MKEY_R_WIN) && (kdata.mkey & CELL_KB_MKEY_R_WIN)) {
		keyboard_key_pressed (R_WIN);
	}
	if ((mkey & CELL_KB_MKEY_R_WIN) && !(kdata.mkey & CELL_KB_MKEY_R_WIN)) {
		keyboard_key_released (R_WIN);
	}

	mkey = kdata.mkey;


	if (kdata.len == 0)
		return;

	int i, j;
	uint16_t kcode;
	for (j = 0; j < kdata.len; j++)
	{
		debug_printf ("kdata.len is %d, mkey is %x\n", kdata.len, kdata.mkey);

		if (kdata.keycode[j] == 0x8039)    //caps lock
			continue;

		if (kdata.keycode[j] & CELL_KB_KEYPAD)
			kcode = kdata.keycode[j] & ~CELL_KB_KEYPAD;
		else
			kcode = kdata.keycode[j];

		debug_printf ("orig keycode = 0x%x\n", kdata.keycode[j]);

		if (kcode == 0x00)
		{
			//debug_printf_quick ("Detected 0x00. Releasing ALL keys\n");
			// Release all keys
			for (i=0; i<MAX_KEYS; i++)
			{
				if (keysdown[i] != 0x00)
				{
					debug_printf_quick ("detected keyUP kcode   '%d'\n", keysdown[i]);
					keyboard_key_released((signed long)keysdown[i]);
					keysdown[i] = 0x00;
				}
			}
			continue;
		}
	}

	// Find the keys that need to be released
	for (i=0; i<MAX_KEYS; i++)
	{
		bool found=false;

		if (keysdown[i] == 0x00)
			continue;

		for (j=0; j < kdata.len; j++) {
			if (kdata.keycode[j] & CELL_KB_KEYPAD)
				kcode = kdata.keycode[j] & ~CELL_KB_KEYPAD;
			else 
				kcode = kdata.keycode[j];

			if (keysdown[i] == kcode)
				found=true;
		}

		if (found==false)
		{
			// This key isn't pressed anymore.
			debug_printf_quick ("detected keyUP kcode   '%d'\n", keysdown[i]);
			keyboard_key_released((signed long)keysdown[i]);
			keysdown[i] = 0x00;
		}
	}


	// Find the keys that need to be pressed
	for (j = 0; j < kdata.len; j++)
	{
		bool found=false;
		if (kdata.keycode[j] & CELL_KB_KEYPAD)
			kcode = kdata.keycode[j] & ~CELL_KB_KEYPAD;
		else 
			kcode = kdata.keycode[j];

		for (i=0; i<MAX_KEYS; i++)
		{
			if (kcode == keysdown[i])
			{
				found=true;
				break;
			}
		}

		if (found)
			continue;
		else
		{
			// find a slot to record it, and press the key
			for (i=0; i < MAX_KEYS; i++)
			{
				if (keysdown[i]==0)
				{
					keysdown[i] = kcode;
					debug_printf_quick ("detected keyDOWN kcode '%d'\n", keysdown[i]);
					keyboard_key_pressed((signed long)keysdown[i]);
					break;
				}
			}
		}
	}

}


signed long kbd_arch_keyname_to_keynum(char *keyname)
{
	return (signed long)atoi(keyname);
}

const char *kbd_arch_keynum_to_keyname(signed long keynum)
{
	static char keyname[20];

	memset(keyname, 0, 20);

	sprintf(keyname, "%li", keynum);

	return keyname;
}

void kbd_initialize_numpad_joykeys(int* joykeys)
{
}

#define MAX_BUFFER 64
static int inputbuffer[MAX_BUFFER];
int osk_active_bufferlen=0;

void osk_kbd_append_buffer (char *keystring)
{
	for (unsigned int i=0; i < strlen(keystring); i++)
	{
		if (osk_active_bufferlen >= MAX_BUFFER) {
			log_warning (LOG_DEFAULT, "WARNING: Keystring inputbufer overflow\n");
			return;
		}

		inputbuffer[osk_active_bufferlen++] = keystring[i];
	}
}

void osk_kbd_append_buffer_char (int keycode)
{
	debug_printf ("appending char keycode %d\n", keycode);
	inputbuffer[osk_active_bufferlen++] = keycode;
}


void osk_kbd_type_key(void)
{
	// This will be called once per vsync.

	static int keystroke=0;    // represents the position in the buffer
	static int keydown=0;
	static int countdown=8;    // The number of cycles the key should be held down in order to register a keystroke

	if (keystroke > osk_active_bufferlen)
	{
		// End of the buffer. 
		// Reset the buffer
		osk_active_bufferlen=0;
		keystroke = 0;
	}

	if (osk_active_bufferlen == 0)
		return;

	if (!keydown)
	{
		// Press a key

		switch (inputbuffer[keystroke])
		{
			case '\n':
				keyboard_key_pressed((signed long) 10 );    // Enter
				break;
			default:
				keyboard_key_pressed((signed long) inputbuffer[keystroke] );
				break;
		}
		keydown=1;
	}
	else 
	{
		if (countdown == 0)
		{
			// Release a key
			switch (inputbuffer[keystroke])
			{
				case '\n':
					keyboard_key_released((signed long) 10 );    // Enter
					break;
				default:
					keyboard_key_released((signed long) inputbuffer[keystroke]);
					break;
			}
			keydown=0;
			// Advance to the next keystroke
			keystroke++;

			// Reset the countdown for the next character
			countdown=8;
		}
		else
			countdown--;
	}
}

