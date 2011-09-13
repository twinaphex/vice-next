/******************************************************************************* 
 *  -- vsyncarch.c -End-of-frame handling for Playstation 3
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

#include "vice.h"

#include "kbdbuf.h"
#include "ui.h"
#include "vsyncapi.h"
#include "videoarch.h"

#include "mouse.h"
#include "joy.h"

#include <unistd.h>
#include <sys/time.h>

/* hook to ui event dispatcher */
static void_hook_t ui_dispatch_hook;
static warp_mode=0;

/* ------------------------------------------------------------------------- */

/* Number of timer units per second. */
signed long vsyncarch_frequency(void)
{
	/* Microseconds resolution. */
	return 1000000;
}


static unsigned long warp_start=0;
static unsigned long warp_offset=0;
static int           warp_multiplier=2;

int set_warp_mode(int warp)
{
	warp_mode = warp;

	if (warp_mode)
	{
		sound_set_warp_mode(1);
		// 0 means don't limit speed. Also means don't redraw.
		// Only other way to speed up is to start dropping frames...  ?
		set_relative_speed(0, NULL);

		warp_start = sys_time_get_system_time();
	}
	else
	{
		sound_set_warp_mode(0);
		set_relative_speed(100, NULL);
	}
}

/* Get time in timer units. */

unsigned long vsyncarch_gettime(void)
{
	return sys_time_get_system_time();
}

void vsyncarch_init(void)
{
	vsync_set_event_dispatcher(ui_dispatch_events);
}

/* Display speed (percentage) and frame rate (frames per second). */
void vsyncarch_display_speed(double speed, double frame_rate, int warp_enabled)
{
	ui_display_speed((float)speed, (float)frame_rate, warp_enabled);
}

/* Sleep a number of timer units. */

void vsyncarch_sleep(signed long delay)
{
	usleep(delay);
}

void vsyncarch_presync(void)
{
	kbdbuf_flush();

	/* Update mouse */
	update_mouse();

	// Check the joystick for input
	joystick();

	// Check for any OSK based keyboard input
	osk_kbd_type_key();

	// Check the keyboard for input
	kbd_process();
}

void_hook_t vsync_set_event_dispatcher(void_hook_t hook)
{
	void_hook_t t = ui_dispatch_hook;

	ui_dispatch_hook = hook;
	return t;
}

void vsyncarch_postsync(void)
{
	(*ui_dispatch_hook)();
}
