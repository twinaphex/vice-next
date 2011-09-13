/******************************************************************************* 
 *  -- mousedrv.h - Mouse driver for Playstation 3
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

#ifndef VICE_MOUSEDRV_H
#define VICE_MOUSEDRV_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif 

extern int mousedrv_resources_init(void);
extern int mousedrv_cmdline_options_init(void);
extern void mousedrv_init(void);
extern void mousedrv_destroy(void);

extern void mousedrv_mouse_changed(void);
extern void update_mouse(void);

extern BYTE mousedrv_get_x(void);
extern BYTE mousedrv_get_y(void);

extern int _mouse_available;
extern int _mouse_x, _mouse_y;
extern int _mouse_coords_dirty;

#ifdef __cplusplus
}
#endif

#endif
