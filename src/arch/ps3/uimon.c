/*
 * uimon.c - Monitor access interface.
 *
 * Written by
 *  Spiro Trikaliotis <Spiro.Trikaliotis@gmx.de>
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#include "vice.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "console.h"
#include "lib.h"
#include "monitor.h"
#include "uimon.h"


int uimon_out(const char *buffer)
{
	return 0;
}

char *uimon_get_in(char **ppchCommandLine, const char *prompt)
{
	return NULL;
}

void uimon_notify_change(void)
{
}



struct console_s *uimon_window_open(void)
{
}

void uimon_window_close(void)
{
}

void uimon_window_suspend(void)
{
}

struct console_s *uimon_window_resume(void)
{
}


void uimon_set_interface(monitor_interface_t **monitor_interface_init, int count)
{
}

void fullscreen_resume(void)
{
}

