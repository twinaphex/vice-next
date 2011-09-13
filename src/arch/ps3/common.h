/******************************************************************************* 
 *  -- common.h -   common header for Vice PS3
 *
 *     VICE PS3 -   Commodore 64 emulator for the Playstation 3
 *                  ported from the original VICE distribution
 *                  located at http://sourceforge.net/projects/vice-emu/
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


#ifdef EMU_C64
#define VICE_USRDIR "/dev_hdd0/game/VICE90000/USRDIR/"
#define VICE_TMPDIR "/dev_hdd0/tmp/game/VICE90000/"
#define VICE_SCREENSHOT_DIR "/dev_hdd0/game/VICE90000/USRDIR/SCREENS/"
#define VICE_SNAPSHOT_DIR "/dev_hdd0/game/VICE90000/USRDIR/SNAPSHOT/"
#define SCREEN_RENDER_TEXTURE_WIDTH  384
#define SCREEN_RENDER_TEXTURE_HEIGHT 272
#define SCREEN_RENDER_TEXTURE_PITCH  384*2

#elif EMU_C128
#define VICE_USRDIR "/dev_hdd0/game/VICE90001/USRDIR/"
#define VICE_TMPDIR "/dev_hdd0/tmp/game/VICE90001/"
#define VICE_SCREENSHOT_DIR "/dev_hdd0/game/VICE90001/USRDIR/SCREENS/"
#define VICE_SNAPSHOT_DIR "/dev_hdd0/game/VICE90001/USRDIR/SNAPSHOT/"
#define SCREEN_RENDER_TEXTURE_WIDTH  384
#define SCREEN_RENDER_TEXTURE_HEIGHT 272
#define SCREEN_RENDER_TEXTURE_PITCH  384*2

#elif EMU_VIC20
#define VICE_USRDIR "/dev_hdd0/game/VICE90002/USRDIR/"
#define VICE_TMPDIR "/dev_hdd0/tmp/game/VICE90002/"
#define VICE_SCREENSHOT_DIR "/dev_hdd0/game/VICE90002/USRDIR/SCREENS/"
#define VICE_SNAPSHOT_DIR "/dev_hdd0/game/VICE90002/USRDIR/SNAPSHOT/"
#define SCREEN_RENDER_TEXTURE_WIDTH  448
#define SCREEN_RENDER_TEXTURE_HEIGHT 284
#define SCREEN_RENDER_TEXTURE_PITCH  448*2

#elif EMU_PLUS4
#define VICE_USRDIR "/dev_hdd0/game/VICE90003/USRDIR/"
#define VICE_TMPDIR "/dev_hdd0/tmp/game/VICE90003/"
#define VICE_SCREENSHOT_DIR "/dev_hdd0/game/VICE90003/USRDIR/SCREENS/"
#define VICE_SNAPSHOT_DIR "/dev_hdd0/game/VICE90003/USRDIR/SNAPSHOT/"
#define SCREEN_RENDER_TEXTURE_WIDTH  384
#define SCREEN_RENDER_TEXTURE_HEIGHT 272
#define SCREEN_RENDER_TEXTURE_PITCH  384*2
#endif
  
#define SYS_CONFIG_FILE VICE_USRDIR "vice64.conf"
