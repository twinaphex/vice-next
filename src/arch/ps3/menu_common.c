/******************************************************************************* 
 *  -- menu_common.c - Common routines for menu functionality
 *
 *     VICE PS3 -   Commodore 64 emulator for the Playstation 3
 *                  ported from the original VICE distribution
 *                  located at http://sourceforge.net/projects/vice-emu/
 *
 *
 *  Copyright (C) 2010
 *  Created on: Dec 24, 2010
 *      Author   TimRex
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

#include <stdbool.h>
#include <sys/timer.h>

float menu_vert_scroll_speed = 0;
bool  menu_ignore_analog = false;

void attenuate_scroll_speed(void) 
{
	float vert_scroll_factor = 0;
	int sleep_gain = 0;
	int sleep_countdown = 0;


	if (sleep_countdown > 0)
	{
		sleep_countdown -= 5;
		sys_timer_usleep (1000 * 5 );  // sleep for 5 ms at a time

		// Start ignoring analogue controls
		menu_ignore_analog = true;
	}
	else
	{
		// re-set the scroll sleep timer
		vert_scroll_factor = 1 - menu_vert_scroll_speed; // inversely proportional
		sleep_gain = (int) 70 * vert_scroll_factor;
		sleep_countdown = (8 + sleep_gain);  // 8ms minimum (fastest),   78ms maximum (slowest)

		// Start listening to analogue controls again
		menu_ignore_analog = false;
	}

	// The sleep counter should be aborted upon any keypress that IS NOT an analogue keypress.
	// This way, the sleep won't prevent other keypresses from being ignored.
}


