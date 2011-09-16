/******************************************************************************* 
 *  -- emu-ps3.cpp -Integration with menu.cpp to provide a frontend for Playstation 3
 *
 *     VICE PS3 -   Commodore 64 emulator for the Playstation 3
 *                  ported from the original VICE distribution
 *                  located at http://sourceforge.net/projects/vice-emu/
 *
 *
 *  Copyright (C) 2010
 *  Created on: Oct 15, 2010
 *      Updated by  TimRex
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/timer.h>
#include <sys/return_code.h>
#include <sys/process.h>
#include <cell/sysmodule.h>
#include <cell/cell_fs.h>
#include <cell/pad.h>
#include <cell/dbgfont.h>
#include <cell/audio.h>
#include <stddef.h>
#include <math.h>

#include <sysutil/sysutil_sysparam.h>
#include <sys/spu_initialize.h>

#include "kbd.h"

#include "common.h"
//#include "settings.h"
#include "emu-ps3.hpp"

#include "ps3video.hpp"
#include "audio/rsound.hpp"
 
#include "joy.h"
#include "mousedrv.h"
#include "machine.h"

extern "C" {
#include "videoarch.h"
#include "archdep.h"
#include "main.h"
#include "sid.h"
#include "util.h"
#include "lib.h"
#include "log.h"
#include "autostart.h"
#include "attach.h"
#include "tape.h"
#include "siddefs-fp.h"
#include "resources.h"
#include "mouse.h"
#include "lib/zlib/zlib.h"
}

#include "menu.hpp"

extern int ps3_audio_suspend(void);
extern int ps3_audio_resume(void);

CellInputFacade* CellInput;
PS3Graphics* Graphics;
SYS_PROCESS_PARAM(1001, 0x10000);


// is emulator loaded?
bool emulator_loaded = false;

// current rom being emulated
char* current_rom = NULL;

// mode the main loop is in
static uint32_t mode_switch = MODE_MENU;

void Emulator_SwitchMode(uint32_t m)
{
	mode_switch = m;
}

uint32_t Emulator_GetMode(void)
{
	return mode_switch;
}

void Emulator_Shutdown()
{
	cellSysutilUnregisterCallback(0);

	mousedrv_destroy();

	if (osk) {
		osk->Close();
		delete osk;
	}

	if (Graphics)
		delete Graphics;

	cellSysmoduleUnloadModule(CELL_SYSMODULE_AUDIO);
	cellSysmoduleUnloadModule(CELL_SYSMODULE_NET);
	cellSysmoduleUnloadModule(CELL_SYSMODULE_FS);
	cellSysmoduleUnloadModule(CELL_SYSMODULE_IO);
}

const char *get_current_rom(void)
{
	return current_rom;
}

void Emulator_StartROMRunning()
{
	Emulator_SwitchMode(MODE_EMULATION);
}

void Emulator_RequestLoadROM(const char* rom, bool forceReboot, bool compatibility_mode) 
{
	if (current_rom == NULL || strcmp(rom, current_rom) != 0)
	{
		if (current_rom != NULL)
		{
			free(current_rom);
		}

		current_rom = strdup(rom);
	}


	if (forceReboot)
	{

		// Tell the emulator to install 'current_rom' for autoload

		// Turn off warp for compatibility mode
		set_autostart_warp(!compatibility_mode, NULL);

		// Set True Drive Emulation (TDE) for compatibility mode
		resources_set_int("DriveTrueEmulation", (int) compatibility_mode);

		// name, program_name, program_number, run/load
		if (autostart_autodetect(current_rom, NULL, 0, AUTOSTART_MODE_RUN) < 0)
		{
			#ifdef CELL_DEBUG
			printf("WARNING: autostart_autodetect failed for image : '%s'", current_rom);
			#endif
		}
	}
	else
	{
		// Insert the disk image, don't force a reboot.
		// TODO : Allow device selection (drive 8, 9, 10, 11 or tape)
		// This hack assumes we're attaching a disk image to drive 8

		if (file_system_attach_disk(8, current_rom) < 0 && tape_image_attach(1, current_rom) < 0 )
		{
			#ifdef CELL_DEBUG
			printf("WARNING: could not attach image : %s to any disk/tape device", current_rom);
			#endif
		}

		#ifdef CELL_DEBUG
		printf("Attached disk image (%s) to device %d\n", current_rom, 8);
		#endif
	}
}


/*
 * Callback for PS3 System operations
 */

void sysutil_exit_callback (uint64_t status, uint64_t param, void *userdata)
{
	(void) param;
	(void) userdata;

	switch (status)
	{
		case CELL_SYSUTIL_REQUEST_EXITGAME:
			Emulator_SwitchMode(MODE_EXIT);
			break;
		case CELL_SYSUTIL_DRAWING_BEGIN:
			sysutil_drawing (1);
			break;
		case CELL_SYSUTIL_DRAWING_END:
			sysutil_drawing (0);
			break;
		case CELL_SYSUTIL_OSKDIALOG_FINISHED:
			osk->Stop();
			osk_kbd_append_buffer ((char *)osk->OutputString());
			break;
		case CELL_SYSUTIL_OSKDIALOG_INPUT_ENTERED:
			break;
		case CELL_SYSUTIL_OSKDIALOG_INPUT_CANCELED:
			break;
		case CELL_OSKDIALOG_INPUT_DEVICE_KEYBOARD:
			break;
		case CELL_OSKDIALOG_INPUT_DEVICE_PAD:
			break;
		case CELL_SYSUTIL_OSKDIALOG_DISPLAY_CHANGED:
			break;
	}

}

void sysutil_callback_redraw(void)
{
	if (Graphics->TimeSinceLastDraw() >= 20000)
	{
		// Refresh the display. 
		Graphics->Refresh();
	}
}

int main (void)
{
	cellSysutilRegisterCallback(0, sysutil_exit_callback, NULL);
	sys_spu_initialize(6, 1);

	cellSysmoduleLoadModule(CELL_SYSMODULE_FS);
	cellSysmoduleLoadModule(CELL_SYSMODULE_IO);

	CellInput = new CellInputFacade();
	CellInput->Init();

	osk = new OSKUtil();
	if (!osk->Init())
	{
		#ifdef CELL_DEBUG
		printf("WARNING: OSK could not be initialised\n");
		#endif
		// TODO: handle this
	}

	Graphics = new PS3Graphics();
	Graphics->Init();  // width, height, depth

	//Graphics->SetOverscan(Settings.PS3OverscanEnabled,(float)Settings.PS3OverscanAmount/100);
	// TODO : We used to use saved overscan values.. but we haven't loaded them for VICE yet.
	Graphics->SetOverscan(false,(float)0);

	Graphics->InitDbgFont();


	// Start running Vice.
	// When it inits the UI, it will call back here for the 'menu' function below
	char  arg0[] = "vice";
	char* argv[] = { &arg0[0], NULL };
	int   argc   = 1;

	emulator_loaded = true;

	main_program (argc, &argv[0]);

	emulator_loaded = false;

	ps3_audio_suspend();
	machine_shutdown();
	Emulator_Shutdown();
	exit(0);
}

extern "C" int menu(uint32_t mode)
{
	// TODO : Maybe we should pause the emulator?

	ps3_audio_suspend();
	Graphics->ScreenDump();

	Emulator_SwitchMode(mode);

	while(1)
	{
		switch(mode_switch)
		{
			case MODE_MENU:
			case MODE_OSK:
				MenuMainLoop();
				break;
			case MODE_EMULATION:
				// Break out and return to Vice.
				Graphics->DestroyDump();
				ps3_audio_resume();

				// The C64 only redraws if it needs to, so we force one here to clean up after the menu
				force_redraw();
				return 0;
				break;
			case MODE_EXIT:
				Graphics->DestroyDump();
				// letting this be handled by atexit
				ps3_audio_suspend();
				machine_shutdown();
				Emulator_Shutdown();
				exit(0);
		}
	}

	// We should never get here
	return 0;
}

float Emulator_GetFontSize()
{
	int res_int;
	resources_get_int("PS3FontSize", &res_int);
	return res_int/100.0;
}
