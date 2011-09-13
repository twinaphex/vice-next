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


#include <assert.h>
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
#include "ps3debug.h"

//extern "C" int main_program(int argc, char **argv);
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
static Emulator_Modes mode_switch = MODE_MENU;


void Emulator_SwitchMode(Emulator_Modes m)
{
	mode_switch = m;
}

Emulator_Modes Emulator_GetMode(void)
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

	debug_close();
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

void Emulator_RequestLoadROM(char* rom, bool forceReboot, bool compatibility_mode) 
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
		if (autostart_autodetect(current_rom, NULL, 0, AUTOSTART_MODE_RUN) < 0) {
			log_warning (LOG_DEFAULT, "autostart_autodetect failed for image : '%s'", current_rom);
		}
	}
	else
	{
		// Insert the disk image, don't force a reboot.
		// TODO : Allow device selection (drive 8, 9, 10, 11 or tape)
		// This hack assumes we're attaching a disk image to drive 8

		if (file_system_attach_disk(8, current_rom) < 0 && tape_image_attach(1, current_rom) < 0 ) {
			log_warning (LOG_DEFAULT, "could not attach image : %s to any disk/tape device", current_rom);
		}

		log_message (LOG_DEFAULT, "Attached disk image (%s) to device %d\n", current_rom, 8);
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


int cellInit(void)
{
	debug_init();
	sys_spu_initialize(6, 1);

	cellSysmoduleLoadModule(CELL_SYSMODULE_FS);
	cellSysmoduleLoadModule(CELL_SYSMODULE_IO);

	CellInput = new CellInputFacade();
	CellInput->Init();

	osk = new OSKUtil();
	if (!osk->Init()) {
		debug_printf ("WARNING: OSK could not be initialised\n");
		// TODO: handle this
	}

	Graphics = new PS3Graphics();
	Graphics->Init();  // width, height, depth

	//Graphics->SetOverscan(Settings.PS3OverscanEnabled,(float)Settings.PS3OverscanAmount/100);
	// TODO : We used to use saved overscan values.. but we haven't loaded them for VICE yet.
	Graphics->SetOverscan(false,(float)0);

	Graphics->InitDbgFont();

	return 0;
}


int main (void)
{
	cellSysutilRegisterCallback(0, sysutil_exit_callback, NULL);
	cellInit();


	// Start running Vice.
	// When it init's the UI, if willcall back here for the 'menu' function below
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



int save_screenshot(struct video_canvas_s *canvas)
{
	const char *md5filename;

	FILE *fd;
	static unsigned int len;
	static unsigned int isdir;


	if (current_rom == NULL)
		return -1;

	if (screenbuffer == NULL)
		return -1;

	if (archdep_stat(VICE_SCREENSHOT_DIR, &len, &isdir) != 0) {
		debug_printf ("archdep_mkstemp_fd creating VICE_TMPDIR : %s\n", VICE_SCREENSHOT_DIR);
		archdep_mkdir(VICE_SCREENSHOT_DIR, 0755);
	}

	// Write the screendump to a hash lookup file of the currently running image file
	// Get MD5 of 'current_rom'

	char *digest;
	digest = md5_sum_file(current_rom);

	if (digest) {
		md5filename = util_concat (VICE_SCREENSHOT_DIR, digest, NULL);

		// save screenshot to VICE_SCREENSHOT_DIR
		debug_printf ("Writing screenshot to %s with width=%d height=%d\n", md5filename, canvas->width, canvas->height);

		size_t size;
		//size = Graphics->RetrieveDumpSize();
		size = canvas->width * canvas->height * canvas->depth / 8;


		fd = (FILE *) gzopen(md5filename, MODE_WRITE"9");
		if (fd == NULL) {
			log_error (LOG_DEFAULT, "Failed to open file for writing %s\n", md5filename);
			lib_free(md5filename);
			return -1;
		}

		int bytes_wrote = gzwrite (fd, screenbuffer, size);

		if (bytes_wrote < size) {
			log_error (LOG_DEFAULT, "Failed to write screenshot, wrote %d bytes of total %d.\n", bytes_wrote, size);
			lib_free(md5filename);
			return -1;
		}

		if (gzclose(fd) != 0) {
			log_warning (LOG_DEFAULT, "fclose failed");
		}

		lib_free(md5filename);
		return 0;
	}
	return -1;
}
 
#define BUFFER_SIZE   384 * 272 * 8 / 2

unsigned char * load_screenshot(const char *filename, long *bytes)
{
	char *digest;
	static unsigned char *pPixels;
	const char *md5filename;
	FILE *fd = NULL;

	// Get MD5 of 'current_rom'
	digest = md5_sum_file(filename);

	if (digest) {
		md5filename = util_concat (VICE_SCREENSHOT_DIR, digest, NULL);

		debug_printf ("Loading screenshot from %s\n", md5filename);
		// read pPixels from VICE_SCREENSHOT_DIR
		fd = (FILE *) gzopen (md5filename, "r");

		if (fd == NULL) {
			debug_printf ("Failed to gzopen file %s\n", md5filename);
			return NULL;
			lib_free (md5filename);
		}

		// TODO optimize this
		if (pPixels == NULL) {
			pPixels = (unsigned char *) lib_malloc (BUFFER_SIZE);
		}

		// The uncompressed buffer size of any screenshot should be exactly BUFFER_SIZE
		// That is, the size of the canvas width * height * depth

		// This might be different for NTSC machines?
		// Ideally it shouldn't matter, we should allocate this dynamically... oh well.


		*bytes = gzread (fd, pPixels, BUFFER_SIZE);

		if (!gzeof(fd)) {
			debug_printf ("fread returned %d uncompressed bytes from file. Buffer exhausted or gzread failed\n", *bytes);
			lib_free (md5filename);
			return NULL;
		} 

		if (gzclose(fd) != 0) {
			debug_printf ("WARNING: fclose failed\n");
		}


		long compressed_filesize = -1;
		fd = fopen (md5filename, "r");
		if ( (fd != NULL) && (fseek(fd, 0L, SEEK_END) != -1) ) {
			compressed_filesize = ftell (fd);
		}

		lib_free (md5filename);
		debug_printf ("read file of %d bytes (%d compressed)\n", *bytes, compressed_filesize);

	} else {
		return NULL;
	}
	return pPixels;
}

extern "C" int menu(Emulator_Modes mode)
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

				// The C64 only redraws if it needs to, so we force one here to clena up after the menu
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
