/******************************************************************************* 
 *  -- ps3debug.cpp - Basic debug functions for Playstation 3
 *
 *     VICE PS3 -     Commodore 64 emulator for the Playstation 3
 *                    ported from the original VICE distribution
 *                    located at http://sourceforge.net/projects/vice-emu/
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

#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 
#include <unistd.h>

#include "netdbg/net.h"
#include "ps3debug.h"


#define DEBUGGERY

static bool netdebug_active=false;

int debug_init(void)
{
#ifdef DEBUGGERY
    net_init();   
    netdebug_active=true;
#endif
}

void debug_printf (const char *format, ...)
{
#ifdef DEBUGGERY
    char buffer[4096];

    va_list args;

    va_start(args,format);
    vsnprintf (buffer, 4096, format, args);
    net_send (0, buffer);
    va_end(args);
#endif
}

void debug_printf_quick (const char *format, ...)
{
#ifdef DEBUGGERY
    char buffer[4096];

    va_list args;

    va_start(args,format);
    vsnprintf (buffer, 4096, format, args);
    net_send (0, buffer);
    va_end(args);
#endif
}

void debug_close(void)
{
#ifdef DEBUGGERY
    netdebug_active=false;
    net_shutdown();
#endif
}

