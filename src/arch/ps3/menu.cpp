/******************************************************************************* 
 *  -- menu.cpp - Menu interface for Vice Playstation 3
 *
 *     VICE PS3 -   Commodore 64 emulator for the Playstation 3
 *                  ported from the original VICE distribution
 *                  located at http://sourceforge.net/projects/vice-emu/
 *
 *
 *  Copyright (C) 2010
 *  Created on: Oct 10, 2010
 *      Adapted for Vice by:  TimRex
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


#include <string.h>
#include <stack>
#include <vector>
#include <cell/pad.h>
#include <cell/audio.h>
#include <cell/sysmodule.h>
#include <cell/cell_fs.h>
#include <cell/dbgfont.h>
#include <sysutil/sysutil_sysparam.h>

#include "cellframework/fileio/FileBrowser.hpp"
#include "cellframework/input/cellInput.h"

#include "ps3video.hpp"
#include "emu-ps3.hpp"
#include "joy.h"
#include "osk.h"
#include "kbd.h"

extern "C" {
#include "sid.h"
#include "siddefs.h"
#include "common.h"
#include "util.h"
#include "lib.h"
#include "machine.h"
#include "videoarch.h"
#include "resources.h"
#include "mouse.h"
#include "attach.h"
#include "tape.h"
#include "datasette.h"
#include "ui.h"
#include "zfile.h"
}

#include "menu.hpp"

#include "common.h"

#define MIN(x,y) ((x) < (y) ? (x) : (y))
#define MAX(x,y) ((x) > (y) ? (x) : (y))

#define NUM_ENTRY_PER_PAGE 19
#define MAX_PATH_LENGTH 1024

// forward declaration
void do_controller_settings(void);
void do_datasette_controls(void);

// is the menu running
bool menuRunning = false;

// menu to render
typedef void (*curMenuPtr)();

//
std::stack<curMenuPtr> menuStack;

// main file browser->for rom browser
FileBrowser* browser = NULL;

// tmp file browser->for everything else
FileBrowser* tmpBrowser = NULL;

int16_t currently_selected_setting = 0;
int16_t currently_selected_vice_setting = 0;
int16_t currently_selected_path_setting = 0;
int16_t currently_selected_osk_entry = MIN_KEY_INDEX;
int16_t currently_selected_controller_setting = 0;
int16_t currently_selected_datasette_control = 0;

#define FILEBROWSER_DELAY              100000
#define FILEBROWSER_DELAY_DIVIDED_BY_3 33333
#define SETTINGS_DELAY                 150000	

#define ROM_EXTENSIONS "d64|d71|d80|d81|d82|g64||g41|x64|t64|tap|prg|p00|crt|bin|D64|D71|D81|D82|G64|G41|X64|T64|TAP|PRG|P00|CRT|BIN|zip|ZIP|gz|GZ|d6z|D6Z|d7z|D7Z|d8z|D8Z|g6z|G6Z|g4z|G4Z|x6z|X6Z"

void MenuStop()
{
	menuRunning = false;
}

bool MenuIsRunning()
{
	return menuRunning;
}

static bool selection_changed=false;

void UpdateBrowser(FileBrowser* b)
{
	if (CellInput->WasButtonPressed(0, CTRL_DOWN) | (CellInput->IsAnalogPressedDownPercentage(0, CTRL_LSTICK) > 0.10) )    // down to next setting
	{
		if(b->GetCurrentEntryIndex() < b->get_current_directory_file_count()-1)
		{
			b->IncrementEntry();
			selection_changed=true;
			sys_timer_usleep(FILEBROWSER_DELAY);
		}
	}
	if (CellInput->WasButtonPressed(0,CTRL_UP) | (CellInput->IsAnalogPressedUpPercentage(0,CTRL_LSTICK) > 0.10) )
	{
		if(b->GetCurrentEntryIndex() > 0)
		{
			b->DecrementEntry();
			selection_changed=true;
			sys_timer_usleep(FILEBROWSER_DELAY);
		}
	}
	if (CellInput->WasButtonPressed(0,CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK))
	{
		b->GotoEntry(MIN(b->GetCurrentEntryIndex()+5, b->get_current_directory_file_count()-1));
		selection_changed=true;
	}
	if (CellInput->WasButtonPressed(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
	{
		if (b->GetCurrentEntryIndex() <= 5)
		{
			b->GotoEntry(0);
			selection_changed=true;
		}
		else
		{
			b->GotoEntry(b->GetCurrentEntryIndex()-5);
			selection_changed=true;
		}
		sys_timer_usleep(FILEBROWSER_DELAY);
	}
	if (CellInput->WasButtonPressed(0,CTRL_R1) && !(CellInput->IsButtonPressed(0,CTRL_TRIANGLE)) )
	{
		b->GotoEntry(MIN(b->GetCurrentEntryIndex()+NUM_ENTRY_PER_PAGE, b->get_current_directory_file_count()-1));
		selection_changed=true;
		sys_timer_usleep(FILEBROWSER_DELAY);
	}
	if (CellInput->WasButtonPressed(0,CTRL_L1) && !(CellInput->IsButtonPressed(0,CTRL_TRIANGLE)) )
	{
		if (b->GetCurrentEntryIndex() <= NUM_ENTRY_PER_PAGE)
		{
			b->GotoEntry(0);
			selection_changed=true;
		}
		else
		{
			b->GotoEntry(b->GetCurrentEntryIndex()-NUM_ENTRY_PER_PAGE);
			selection_changed=true;
		}
		sys_timer_usleep(FILEBROWSER_DELAY);
	}

	if (CellInput->WasButtonPressed(0, CTRL_CIRCLE))
	{
		b->PopDirectory();
		selection_changed=true;
	}
}

void RenderBrowser(FileBrowser* b)
{
	uint32_t file_count = b->get_current_directory_file_count();
	int current_index = b->GetCurrentEntryIndex();

	int page_number = current_index / NUM_ENTRY_PER_PAGE;
	int page_base = page_number * NUM_ENTRY_PER_PAGE;
	float currentX = 0.09f;
	float currentY = 0.09f;
	float ySpacing = 0.035f;

	for (int i = page_base; i < file_count && i < page_base + NUM_ENTRY_PER_PAGE; ++i)
	{
		currentY = currentY + ySpacing;
		cellDbgFontPuts(currentX, currentY, Emulator_GetFontSize(),
				i == current_index ? RED : b->_cur[i]->d_type == CELL_FS_TYPE_DIRECTORY ? GREEN : WHITE,
				b->_cur[i]->d_name);
		cellDbgFontDraw();
	}

	// Locate any screenshots for the currently selected file
	if (selection_changed) {
		selection_changed=false;

		#if 0
		if (b->_cur[current_index]->d_type == CELL_FS_TYPE_REGULAR) {
			// TODO check file extension

			std::string ext = FileBrowser::GetExtension(b->_cur[current_index]->d_name);
			std::string path = b->GetCurrentDirectoryInfo().dir + "/" + b->_cur[current_index]->d_name;

			if ( ((ext == "zip") || (ext == "ZIP")) && (zipfile_entries(path.c_str()) != 1) ) {
				// Do nothing
				// If we have a zipfile with multiple entries, we can use zipfile browsing support.
				// If it only has a single entry, we can use this file as it is
			}
			else {
				// We're processing a regular file, or a zipfile having only a single entry embedded
				// gzip files by definition on ly have a single entry, and thus treated as regular files also.

			}
		}
		#endif
	}


	cellDbgFontDraw();
}

void do_shaderChoice()
{
	char path[MAX_PATH_LENGTH];

	if (tmpBrowser == NULL)
	{
		char *tmppath = util_concat(VICE_USRDIR, "shaders", NULL);
		tmpBrowser = new FileBrowser(tmppath, "cg|CG");
		lib_free(tmppath);
	}

	if (CellInput->UpdateDevice(0) == CELL_PAD_OK)
	{
		UpdateBrowser(tmpBrowser);

		if (CellInput->WasButtonPressed(0,CTRL_CROSS))
		{
			if(tmpBrowser->IsCurrentADirectory())
			{
                                const char * separatorslash = (strcmp(tmpBrowser->get_current_directory_name(),"/") == 0) ? "" : "/";
				snprintf(path, sizeof(path), "%s%s%s", tmpBrowser->get_current_directory_name(), separatorslash, tmpBrowser->get_current_filename());
				tmpBrowser->PushDirectory(path, CELL_FS_TYPE_REGULAR | CELL_FS_TYPE_DIRECTORY, "cg|CG");
			}
			else if (tmpBrowser->IsCurrentAFile())
			{
				snprintf(path, sizeof(path), "%s/%s", tmpBrowser->get_current_directory_name(), tmpBrowser->get_current_filename());

				//load shader
				Graphics->LoadFragmentShader(path);
				Graphics->SetSmooth(false);
				resources_set_int("PS3HardwareFilter", false);
				menuStack.pop();

				// Display the screen render for a moment, so we know we want this shader
				// render was previously saved before the menu was launched.

				// TODO  On initial launch, the screen dump will be blank. We need a default image here.
				Graphics->DumpScreen();
				psglSwap();
				sys_timer_usleep (1000 * 1000 * 2);
			}
		}

		if (CellInput->WasButtonHeld(0, CTRL_TRIANGLE)) {
			menuStack.pop();
		}
	}

	cellDbgFontPuts(0.09f, 0.88f, Emulator_GetFontSize(), YELLOW, "CROSS    - Select shader");
	cellDbgFontPuts(0.09f, 0.92f, Emulator_GetFontSize(), PURPLE, "TRIANGLE - return to settings");
	cellDbgFontDraw();

	RenderBrowser(tmpBrowser);
}

void do_pathChoice()
{
	char path[MAX_PATH_LENGTH];

	if (tmpBrowser == NULL)
                tmpBrowser = new FileBrowser("/\0", "empty");

	if (CellInput->UpdateDevice(0) == CELL_PAD_OK)
	{
		UpdateBrowser(tmpBrowser);
		if (CellInput->WasButtonPressed(0,CTRL_SQUARE))
		{
			if(tmpBrowser->IsCurrentADirectory())
			{
				snprintf(path, sizeof(path), "%s/%s", tmpBrowser->get_current_directory_name(), tmpBrowser->get_current_filename());
				switch(currently_selected_path_setting)
				{
					case SETTING_PATH_DEFAULT_ROM_DIRECTORY:
						resources_set_string("PS3PathRomDir", path);
						break;
				}
				menuStack.pop();
			}
		}
		if (CellInput->WasButtonPressed(0, CTRL_TRIANGLE))
		{
			snprintf(path, sizeof(path), VICE_USRDIR);
			switch(currently_selected_path_setting)
			{
				case SETTING_PATH_DEFAULT_ROM_DIRECTORY:
					resources_set_string("PS3PathRomDir", path);
					break;
			}
			menuStack.pop();
		}
		if (CellInput->WasButtonPressed(0,CTRL_CROSS))
		{
			if(tmpBrowser->IsCurrentADirectory())
			{
                                const char * separatorslash = (strcmp(tmpBrowser->get_current_directory_name(),"/") == 0) ? "" : "/";
                                snprintf(path, sizeof(path), "%s%s%s", tmpBrowser->get_current_directory_name(), separatorslash, tmpBrowser->get_current_filename());
                                tmpBrowser->PushDirectory(path, CELL_FS_TYPE_REGULAR | CELL_FS_TYPE_DIRECTORY, "empty");
			}
		}
	}

	cellDbgFontPuts(0.05f, 0.88f, Emulator_GetFontSize(), YELLOW, "CROSS    - enter directory");
	cellDbgFontPuts(0.05f, 0.92f, Emulator_GetFontSize(), BLUE,   "SQUARE   - select directory as path");
	cellDbgFontPuts(0.55f, 0.92f, Emulator_GetFontSize(), PURPLE, "TRIANGLE - return to settings");
	cellDbgFontDraw();

	RenderBrowser(tmpBrowser);
}

// void do_path_settings()
// // called from ROM menu by pressing the SELECT button
// // return to ROM menu by pressing the CIRCLE button
void do_path_settings()
{
	static const char *footer = NULL;
	const char *res_string;

	if(CellInput->UpdateDevice(0) == CELL_OK)
	{
		// back to ROM menu if CIRCLE is pressed
		if (CellInput->WasButtonPressed(0, CTRL_L1) | CellInput->WasButtonPressed(0, CTRL_CIRCLE))
		{
			menuStack.pop();
			return;
		}

		if (CellInput->WasButtonPressed(0, CTRL_R1))
		{
			menuStack.push(do_controller_settings);
			return;
		}


		if (CellInput->WasButtonPressed(0,CTRL_DOWN) | (CellInput->IsAnalogPressedDownPercentage(0,CTRL_LSTICK) > 0.10) )
		{
			// down to next setting
			currently_selected_path_setting++;
			if (currently_selected_path_setting >= MAX_NO_OF_PATH_SETTINGS)
			{
				currently_selected_path_setting = 0;
			}
		}

		if (CellInput->WasButtonPressed(0,CTRL_START))
		{
			MenuStop();
			Emulator_StartROMRunning();
			return;
		}

		if (CellInput->WasButtonPressed(0,CTRL_UP) | (CellInput->IsAnalogPressedUpPercentage(0,CTRL_LSTICK) > 0.10) )
		{
			// up to previous setting
			currently_selected_path_setting--;
			if (currently_selected_path_setting < 0)
			{
				currently_selected_path_setting = MAX_NO_OF_PATH_SETTINGS-1;
			}
		}
		switch(currently_selected_path_setting)
		{
			case SETTING_PATH_DEFAULT_ROM_DIRECTORY:
				if(CellInput->WasButtonPressed(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_CROSS))
				{
					menuStack.push(do_pathChoice);
					tmpBrowser = NULL;
				}
				if(CellInput->IsButtonPressed(0, CTRL_TRIANGLE))
				{
					resources_set_string("PS3PathRomDir", "/");
				}
				break;
			case SETTING_PATH_SAVE_SETTINGS:
				if(CellInput->WasButtonPressed(0, CTRL_CROSS) | CellInput->IsButtonPressed(0, CTRL_TRIANGLE))
				{
					resources_save(NULL);
				}
				break;
			case SETTING_PATH_DEFAULT_ALL:
				if(CellInput->WasButtonPressed(0, CTRL_CROSS) | CellInput->IsButtonPressed(0, CTRL_TRIANGLE))
				{
					resources_set_string("PS3PathRomDir", "/");
				}
				break;
			default:
				break;
		} // end of switch
	}

	float yPos = 0.13;
	float ySpacing = 0.04;

	cellDbgFontPuts (0.09f, 0.05f, Emulator_GetFontSize(), BLUE,   "GENERAL");
	cellDbgFontPuts (0.26f,  0.05f, Emulator_GetFontSize(), BLUE,  "VICE");
	cellDbgFontPuts (0.44f, 0.05f, Emulator_GetFontSize(), PURPLE, "PATHS");
	cellDbgFontPuts (0.61f, 0.05f, Emulator_GetFontSize(), BLUE,   "CONTROLS");
	cellDbgFontPuts (0.78f, 0.05f, Emulator_GetFontSize(), BLUE,   "DATASETTE");
	cellDbgFontDraw();

	resources_get_string ("PS3PathRomDir", &res_string);
	cellDbgFontPuts (0.09f, yPos, Emulator_GetFontSize(), currently_selected_path_setting == SETTING_PATH_DEFAULT_ROM_DIRECTORY ? YELLOW : WHITE, "Startup ROM Directory");
	cellDbgFontPuts (0.5f,  yPos, Emulator_GetFontSize(), res_string == "/" ? YELLOW : GREEN, res_string);


	yPos += ySpacing;
	yPos += ySpacing;
	cellDbgFontPuts (0.09f, yPos, Emulator_GetFontSize(), currently_selected_path_setting == SETTING_PATH_SAVE_SETTINGS ? YELLOW : GREEN, "SAVE SETTINGS");
	cellDbgFontDraw();


	yPos += ySpacing;
	cellDbgFontPrintf(0.09f, yPos, Emulator_GetFontSize(), currently_selected_path_setting == SETTING_PATH_DEFAULT_ALL ? YELLOW : GREEN, "DEFAULT");

	cellDbgFontPuts(0.05f, 0.88f, Emulator_GetFontSize(), YELLOW, "UP/DOWN   - select    X/LEFT/RIGHT - change          TRIANGLE - default");

	if (footer == NULL)
		footer = util_concat("L1/CIRCLE - back      START        - return to ", machine_name, "   R1       - forward", NULL);

	cellDbgFontPuts(0.05f, 0.92f, Emulator_GetFontSize(), YELLOW, footer);
	cellDbgFontDraw();
}

// // called from ROM menu by pressing the SELECT button
// // return to ROM menu by pressing the CIRCLE button
void do_controller_settings()
{
	static const char *footer = NULL;
	bool dirty=false;


	if(CellInput->UpdateDevice(0) == CELL_OK)
	{
		// back to ROM menu if CIRCLE is pressed
		if (CellInput->WasButtonPressed(0, CTRL_L1) | CellInput->WasButtonPressed(0, CTRL_CIRCLE))
		{
			menuStack.pop();
			return;
		}

		if (CellInput->WasButtonPressed(0, CTRL_R1))
		{
			menuStack.push(do_datasette_controls);
			return;
		}

		if (CellInput->WasButtonPressed(0,CTRL_DOWN) | (CellInput->IsAnalogPressedDownPercentage(0,CTRL_LSTICK) > 0.10) )
		{
			currently_selected_controller_setting++;
			if (currently_selected_controller_setting >= MAX_NO_OF_CONTROLLER_SETTINGS)
			{
				currently_selected_controller_setting = 0;
			}
		}

		if (CellInput->WasButtonPressed(0,CTRL_START))
		{
			MenuStop();
			Emulator_StartROMRunning();
			return;
		}

		if (CellInput->WasButtonPressed(0,CTRL_UP) | (CellInput->IsAnalogPressedUpPercentage(0,CTRL_LSTICK) > 0.10) )
		{
			currently_selected_controller_setting--;
			if (currently_selected_controller_setting < 0)
			{
				currently_selected_controller_setting = MAX_NO_OF_CONTROLLER_SETTINGS-1;
			}
		}

		switch(currently_selected_controller_setting)
		{
			case SETTING_CONTROLLER_1_CROSS:
				if(CellInput->WasButtonPressed(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
				{
					if (joymap_index[0][CROSS] > 0) {
						joymap_index[0][CROSS]--;
						dirty=true;
					}
				}
				if(CellInput->WasButtonPressed(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK) | CellInput->WasButtonPressed(0,CTRL_CROSS))
				{
					if (joymap_index[0][CROSS] < MAX_JOYMAP_INDEX) {
						joymap_index[0][CROSS]++;
						dirty=true;
					}
				}
				if(CellInput->IsButtonPressed(0, CTRL_TRIANGLE))
				{
					joymap_index[0][CROSS] = JOYMAP_FIRE;
					dirty=true;
				}
				break;
			case SETTING_CONTROLLER_1_CIRCLE:
				if(CellInput->WasButtonPressed(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
				{
					if (joymap_index[0][CIRCLE] > 0) {
						joymap_index[0][CIRCLE]--;
						dirty=true;
					}
				}
				if(CellInput->WasButtonPressed(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK) | CellInput->WasButtonPressed(0,CTRL_CROSS))
				{
					if (joymap_index[0][CIRCLE] < MAX_JOYMAP_INDEX) {
						joymap_index[0][CIRCLE]++;
						dirty=true;
					}
				}
				if(CellInput->IsButtonPressed(0, CTRL_TRIANGLE))
				{
					joymap_index[0][CIRCLE] = OSK_N;
					dirty=true;
				}
				break;
			case SETTING_CONTROLLER_1_SQUARE:
				if(CellInput->WasButtonPressed(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
				{
					if (joymap_index[0][SQUARE] > 0) {
						joymap_index[0][SQUARE]--;
						dirty=true;
					}
				}
				if(CellInput->WasButtonPressed(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK) | CellInput->WasButtonPressed(0,CTRL_CROSS))
				{
					if (joymap_index[0][SQUARE] < MAX_JOYMAP_INDEX) {
						joymap_index[0][SQUARE]++;
						dirty=true;
					}
				}
				if(CellInput->IsButtonPressed(0, CTRL_TRIANGLE))
				{
					joymap_index[0][SQUARE] = OSK_SPACE;
					dirty=true;
				}
				break;
			case SETTING_CONTROLLER_1_TRIANGLE:
				if(CellInput->WasButtonPressed(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
				{
					if (joymap_index[0][TRIANGLE] > 0) {
						joymap_index[0][TRIANGLE]--;
						dirty=true;
					}
				}
				if(CellInput->WasButtonPressed(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK) | CellInput->WasButtonPressed(0,CTRL_CROSS))
				{
					if (joymap_index[0][TRIANGLE] < MAX_JOYMAP_INDEX) {
						joymap_index[0][TRIANGLE]++;
						dirty=true;
					}
				}
				if(CellInput->IsButtonPressed(0, CTRL_TRIANGLE))
				{
					joymap_index[0][TRIANGLE] = OSK_Y;
					dirty=true;
				}
				break;
			case SETTING_CONTROLLER_1_L1:
				if(CellInput->WasButtonPressed(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
				{
					if (joymap_index[0][L1] > 0) {
						joymap_index[0][L1]--;
						dirty=true;
					}
				}
				if(CellInput->WasButtonPressed(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK) | CellInput->WasButtonPressed(0,CTRL_CROSS))
				{
					if (joymap_index[0][L1] < MAX_JOYMAP_INDEX) {
						joymap_index[0][L1]++;
						dirty=true;
					}
				}
				if(CellInput->IsButtonPressed(0, CTRL_TRIANGLE))
				{
					joymap_index[0][L1] = OSK_F1;
					dirty=true;
				}
				break;

			case SETTING_CONTROLLER_1_L2:
				if(CellInput->WasButtonPressed(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
				{
					if (joymap_index[0][L2] > 0) {
						joymap_index[0][L2]--;
						dirty=true;
					}
				}
				if(CellInput->WasButtonPressed(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK) | CellInput->WasButtonPressed(0,CTRL_CROSS))
				{
					if (joymap_index[0][L2] < MAX_JOYMAP_INDEX) {
						joymap_index[0][L2]++;
						dirty=true;
					}
				}
				if(CellInput->IsButtonPressed(0, CTRL_TRIANGLE))
				{
					joymap_index[0][L2] = OSK_F3;
					dirty=true;
				}
				break;
			case SETTING_CONTROLLER_1_R1:
				if(CellInput->WasButtonPressed(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
				{
					if (joymap_index[0][R1] > 0) {
						joymap_index[0][R1]--;
						dirty=true;
					}
				}
				if(CellInput->WasButtonPressed(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK) | CellInput->WasButtonPressed(0,CTRL_CROSS))
				{
					if (joymap_index[0][R1] < MAX_JOYMAP_INDEX) {
						joymap_index[0][R1]++;
						dirty=true;
					}
				}
				if(CellInput->IsButtonPressed(0, CTRL_TRIANGLE))
				{
					joymap_index[0][R1] = OSK_F5;
					dirty=true;
				}
				break;
			case SETTING_CONTROLLER_1_R2:
				if(CellInput->WasButtonPressed(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
				{
					if (joymap_index[0][R2] > 0) {
						joymap_index[0][R2]--;
						dirty=true;
					}
				}
				if(CellInput->WasButtonPressed(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK) | CellInput->WasButtonPressed(0,CTRL_CROSS))
				{
					if (joymap_index[0][R2] < MAX_JOYMAP_INDEX) {
						joymap_index[0][R2]++;
						dirty=true;
					}
				}
				if(CellInput->IsButtonPressed(0, CTRL_TRIANGLE))
				{
					joymap_index[0][R2] = OSK_F7;
					dirty=true;
				}
				break;
			case SETTING_CONTROLLER_2_CROSS:
				if(CellInput->WasButtonPressed(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
				{
					if (joymap_index[1][CROSS] > 0) {
						joymap_index[1][CROSS]--;
						dirty=true;
					}
				}
				if(CellInput->WasButtonPressed(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK) | CellInput->WasButtonPressed(0,CTRL_CROSS))
				{
					if (joymap_index[1][CROSS] < MAX_JOYMAP_INDEX) {
						joymap_index[1][CROSS]++;
						dirty=true;
					}
				}
				if(CellInput->IsButtonPressed(0, CTRL_TRIANGLE))
				{
					joymap_index[1][CROSS] = JOYMAP_FIRE;
					dirty=true;
				}
				break;
			case SETTING_CONTROLLER_2_CIRCLE:
				if(CellInput->WasButtonPressed(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
				{
					if (joymap_index[1][CIRCLE] > 0) {
						joymap_index[1][CIRCLE]--;
						dirty=true;
					}
				}
				if(CellInput->WasButtonPressed(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK) | CellInput->WasButtonPressed(0,CTRL_CROSS))
				{
					if (joymap_index[1][CIRCLE] < MAX_JOYMAP_INDEX) {
						joymap_index[1][CIRCLE]++;
						dirty=true;
					}
				}
				if(CellInput->IsButtonPressed(0, CTRL_TRIANGLE))
				{
					joymap_index[1][CIRCLE] = OSK_N;
					dirty=true;
				}
				break;
			case SETTING_CONTROLLER_2_SQUARE:
				if(CellInput->WasButtonPressed(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
				{
					if (joymap_index[1][SQUARE] > 0) {
						joymap_index[1][SQUARE]--;
						dirty=true;
					}
				}
				if(CellInput->WasButtonPressed(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK) | CellInput->WasButtonPressed(0,CTRL_CROSS))
				{
					if (joymap_index[1][SQUARE] < MAX_JOYMAP_INDEX) {
						joymap_index[1][SQUARE]++;
						dirty=true;
					}
				}
				if(CellInput->IsButtonPressed(0, CTRL_TRIANGLE))
				{
					joymap_index[1][SQUARE] = OSK_SPACE;
					dirty=true;
				}
				break;
			case SETTING_CONTROLLER_2_TRIANGLE:
				if(CellInput->WasButtonPressed(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
				{
					if (joymap_index[1][TRIANGLE] > 0) {
						joymap_index[1][TRIANGLE]--;
						dirty=true;
					}
				}
				if(CellInput->WasButtonPressed(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK) | CellInput->WasButtonPressed(0,CTRL_CROSS))
				{
					if (joymap_index[1][TRIANGLE] < MAX_JOYMAP_INDEX) {
						joymap_index[1][TRIANGLE]++;
						dirty=true;
					}
				}
				if(CellInput->IsButtonPressed(0, CTRL_TRIANGLE))
				{
					joymap_index[1][TRIANGLE] = OSK_Y;
					dirty=true;
				}
				break;
			case SETTING_CONTROLLER_2_L1:
				if(CellInput->WasButtonPressed(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
				{
					if (joymap_index[1][L1] > 0) {
						joymap_index[1][L1]--;
						dirty=true;
					}
				}
				if(CellInput->WasButtonPressed(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK) | CellInput->WasButtonPressed(0,CTRL_CROSS))
				{
					if (joymap_index[1][L1] < MAX_JOYMAP_INDEX) {
						joymap_index[1][L1]++;
						dirty=true;
					}
				}
				if(CellInput->IsButtonPressed(0, CTRL_TRIANGLE))
				{
					joymap_index[1][L1] = OSK_F1;
					dirty=true;
				}
				break;

			case SETTING_CONTROLLER_2_L2:
				if(CellInput->WasButtonPressed(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
				{
					if (joymap_index[1][L2] > 0) {
						joymap_index[1][L2]--;
						dirty=true;
					}
				}
				if(CellInput->WasButtonPressed(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK) | CellInput->WasButtonPressed(0,CTRL_CROSS))
				{
					if (joymap_index[1][L2] < MAX_JOYMAP_INDEX) {
						joymap_index[1][L2]++;
						dirty=true;
					}
				}
				if(CellInput->IsButtonPressed(0, CTRL_TRIANGLE))
				{
					joymap_index[1][L2] = OSK_F3;
					dirty=true;
				}
				break;
			case SETTING_CONTROLLER_2_R1:
				if(CellInput->WasButtonPressed(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
				{
					if (joymap_index[1][R1] > 0) {
						joymap_index[1][R1]--;
						dirty=true;
					}
				}
				if(CellInput->WasButtonPressed(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK) | CellInput->WasButtonPressed(0,CTRL_CROSS))
				{
					if (joymap_index[1][R1] < MAX_JOYMAP_INDEX) {
						joymap_index[1][R1]++;
						dirty=true;
					}
				}
				if(CellInput->IsButtonPressed(0, CTRL_TRIANGLE))
				{
					joymap_index[1][R1] = OSK_F5;
					dirty=true;
				}
				break;
			case SETTING_CONTROLLER_2_R2:
				if(CellInput->WasButtonPressed(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
				{
					if (joymap_index[1][R2] > 0) {
						joymap_index[1][R2]--;
						dirty=true;
					}
				}
				if(CellInput->WasButtonPressed(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK) | CellInput->WasButtonPressed(0,CTRL_CROSS))
				{
					if (joymap_index[1][R2] < MAX_JOYMAP_INDEX) {
						joymap_index[1][R2]++;
						dirty=true;
					}
				}
				if(CellInput->IsButtonPressed(0, CTRL_TRIANGLE))
				{
					joymap_index[1][R2] = OSK_F7;
					dirty=true;
				}
				break;
			case SETTING_CONTROLLER_DEFAULT_ALL:
				if(CellInput->WasButtonPressed(0, CTRL_CROSS) | CellInput->IsButtonPressed(0, CTRL_TRIANGLE))
				{
					joymap_index[0][TRIANGLE] = OSK_Y;
					joymap_index[1][TRIANGLE] = OSK_Y;
					joymap_index[0][SQUARE]   = OSK_N;
					joymap_index[1][SQUARE]   = OSK_N;
					joymap_index[0][CIRCLE]   = OSK_SPACE;
					joymap_index[1][CIRCLE]   = OSK_SPACE;
					joymap_index[0][CROSS]    = JOYMAP_FIRE;
					joymap_index[1][CROSS]    = JOYMAP_FIRE;
					joymap_index[0][L1]       = OSK_F1;
					joymap_index[1][L1]       = OSK_F1;
					joymap_index[0][L2]       = OSK_F3;
					joymap_index[1][L2]       = OSK_F3;
					joymap_index[0][R1]       = OSK_F5;
					joymap_index[1][R1]       = OSK_F5;
					joymap_index[0][R2]       = OSK_F7;
					joymap_index[1][R2]       = OSK_F7;
				}
				break;
			default:
				break;
		} // end of switch
	}

	float yPos = 0.13;
	float ySpacing = 0.04;

	cellDbgFontPuts (0.09f, 0.05f, Emulator_GetFontSize(), BLUE,   "GENERAL");
	cellDbgFontPuts (0.26f,  0.05f, Emulator_GetFontSize(), BLUE,  "VICE");
	cellDbgFontPuts (0.44f, 0.05f, Emulator_GetFontSize(), BLUE,   "PATHS");
	cellDbgFontPuts (0.61f, 0.05f, Emulator_GetFontSize(), PURPLE, "CONTROLS");
	cellDbgFontPuts (0.78f, 0.05f, Emulator_GetFontSize(), BLUE,   "DATASETTE");
	cellDbgFontDraw();


	cellDbgFontPuts (0.09f, yPos, Emulator_GetFontSize(), currently_selected_controller_setting == SETTING_CONTROLLER_1_CROSS ? YELLOW : WHITE, "Controller 1 CROSS");
	cellDbgFontPuts (0.5f,  yPos, Emulator_GetFontSize(), joymap_index[0][CROSS] == JOYMAP_FIRE  ? GREEN : RED, keymap[joymap_index[0][CROSS]].keyname);

	yPos += ySpacing;
	cellDbgFontPuts (0.09f, yPos, Emulator_GetFontSize(), currently_selected_controller_setting == SETTING_CONTROLLER_1_SQUARE ? YELLOW : WHITE, "Controller 1 SQUARE");
	cellDbgFontPuts (0.5f,  yPos, Emulator_GetFontSize(), joymap_index[0][SQUARE] == OSK_SPACE  ? GREEN : RED, keymap[joymap_index[0][SQUARE]].keyname);

	yPos += ySpacing;
	cellDbgFontPuts (0.09f, yPos, Emulator_GetFontSize(), currently_selected_controller_setting == SETTING_CONTROLLER_1_TRIANGLE ? YELLOW : WHITE, "Controller 1 TRIANGLE");
	cellDbgFontPuts (0.5f,  yPos, Emulator_GetFontSize(), joymap_index[0][TRIANGLE] == OSK_Y  ? GREEN : RED, keymap[joymap_index[0][TRIANGLE]].keyname);

	yPos += ySpacing;
	cellDbgFontPuts (0.09f, yPos, Emulator_GetFontSize(), currently_selected_controller_setting == SETTING_CONTROLLER_1_CIRCLE ? YELLOW : WHITE, "Controller 1 CIRCLE");
	cellDbgFontPuts (0.5f,  yPos, Emulator_GetFontSize(), joymap_index[0][CIRCLE] == OSK_N  ? GREEN : RED, keymap[joymap_index[0][CIRCLE]].keyname);

	yPos += ySpacing;
	cellDbgFontPuts (0.09f, yPos, Emulator_GetFontSize(), currently_selected_controller_setting == SETTING_CONTROLLER_1_L1 ? YELLOW : WHITE, "Controller 1 L1");
	cellDbgFontPuts (0.5f,  yPos, Emulator_GetFontSize(), joymap_index[0][L1] == OSK_F1  ? GREEN : RED, keymap[joymap_index[0][L1]].keyname);

	yPos += ySpacing;
	cellDbgFontPuts (0.09f, yPos, Emulator_GetFontSize(), currently_selected_controller_setting == SETTING_CONTROLLER_1_L2 ? YELLOW : WHITE, "Controller 1 L2");
	cellDbgFontPuts (0.5f,  yPos, Emulator_GetFontSize(), joymap_index[0][L2] == OSK_F3  ? GREEN : RED, keymap[joymap_index[0][L2]].keyname);

	yPos += ySpacing;
	cellDbgFontPuts (0.09f, yPos, Emulator_GetFontSize(), currently_selected_controller_setting == SETTING_CONTROLLER_1_R1 ? YELLOW : WHITE, "Controller 1 R1");
	cellDbgFontPuts (0.5f,  yPos, Emulator_GetFontSize(), joymap_index[0][R1] == OSK_F5  ? GREEN : RED, keymap[joymap_index[0][R1]].keyname);

	yPos += ySpacing;
	cellDbgFontPuts (0.09f, yPos, Emulator_GetFontSize(), currently_selected_controller_setting == SETTING_CONTROLLER_1_R2 ? YELLOW : WHITE, "Controller 1 R2");
	cellDbgFontPuts (0.5f,  yPos, Emulator_GetFontSize(), joymap_index[0][R2] == OSK_F7  ? GREEN : RED, keymap[joymap_index[0][R2]].keyname);

	yPos += ySpacing;
	cellDbgFontPuts (0.09f, yPos, Emulator_GetFontSize(), currently_selected_controller_setting == SETTING_CONTROLLER_2_CROSS ? YELLOW : WHITE, "Controller 2 CROSS");
	cellDbgFontPuts (0.5f,  yPos, Emulator_GetFontSize(), joymap_index[1][CROSS] == JOYMAP_FIRE  ? GREEN : RED, keymap[joymap_index[1][CROSS]].keyname);

	yPos += ySpacing;
	cellDbgFontPuts (0.09f, yPos, Emulator_GetFontSize(), currently_selected_controller_setting == SETTING_CONTROLLER_2_SQUARE ? YELLOW : WHITE, "Controller 2 SQUARE");
	cellDbgFontPuts (0.5f,  yPos, Emulator_GetFontSize(), joymap_index[1][SQUARE] == OSK_SPACE  ? GREEN : RED, keymap[joymap_index[1][SQUARE]].keyname);

	yPos += ySpacing;
	cellDbgFontPuts (0.09f, yPos, Emulator_GetFontSize(), currently_selected_controller_setting == SETTING_CONTROLLER_2_TRIANGLE ? YELLOW : WHITE, "Controller 2 TRIANGLE");
	cellDbgFontPuts (0.5f,  yPos, Emulator_GetFontSize(), joymap_index[1][TRIANGLE] == OSK_Y  ? GREEN : RED, keymap[joymap_index[1][TRIANGLE]].keyname);

	yPos += ySpacing;
	cellDbgFontPuts (0.09f, yPos, Emulator_GetFontSize(), currently_selected_controller_setting == SETTING_CONTROLLER_2_CIRCLE ? YELLOW : WHITE, "Controller 2 CIRCLE");
	cellDbgFontPuts (0.5f,  yPos, Emulator_GetFontSize(), joymap_index[1][CIRCLE] == OSK_N  ? GREEN : RED, keymap[joymap_index[1][CIRCLE]].keyname);

	yPos += ySpacing;
	cellDbgFontPuts (0.09f, yPos, Emulator_GetFontSize(), currently_selected_controller_setting == SETTING_CONTROLLER_2_L1 ? YELLOW : WHITE, "Controller 1 L1");
	cellDbgFontPuts (0.5f,  yPos, Emulator_GetFontSize(), joymap_index[1][L1] == OSK_F1  ? GREEN : RED, keymap[joymap_index[1][L1]].keyname);

	yPos += ySpacing;
	cellDbgFontPuts (0.09f, yPos, Emulator_GetFontSize(), currently_selected_controller_setting == SETTING_CONTROLLER_2_L2 ? YELLOW : WHITE, "Controller 1 L2");
	cellDbgFontPuts (0.5f,  yPos, Emulator_GetFontSize(), joymap_index[1][L2] == OSK_F3  ? GREEN : RED, keymap[joymap_index[1][L2]].keyname);

	yPos += ySpacing;
	cellDbgFontPuts (0.09f, yPos, Emulator_GetFontSize(), currently_selected_controller_setting == SETTING_CONTROLLER_2_R1 ? YELLOW : WHITE, "Controller 1 R1");
	cellDbgFontPuts (0.5f,  yPos, Emulator_GetFontSize(), joymap_index[1][R1] == OSK_F5  ? GREEN : RED, keymap[joymap_index[1][R1]].keyname);

	yPos += ySpacing;
	cellDbgFontPuts (0.09f, yPos, Emulator_GetFontSize(), currently_selected_controller_setting == SETTING_CONTROLLER_2_R2 ? YELLOW : WHITE, "Controller 1 R2");
	cellDbgFontPuts (0.5f,  yPos, Emulator_GetFontSize(), joymap_index[1][R2] == OSK_F7  ? GREEN : RED, keymap[joymap_index[1][R2]].keyname);



	yPos += ySpacing;
	cellDbgFontPuts (0.09f, yPos, Emulator_GetFontSize(), currently_selected_controller_setting == SETTING_CONTROLLER_SAVE_SETTINGS ? YELLOW : GREEN, "SAVE SETTINGS");
	cellDbgFontDraw();


	yPos += ySpacing;
	cellDbgFontPrintf(0.09f, yPos, Emulator_GetFontSize(), currently_selected_controller_setting == SETTING_CONTROLLER_DEFAULT_ALL ? YELLOW : GREEN, "DEFAULT");


	if (footer == NULL)
		footer = util_concat("L1/CIRCLE - back      START        - return to ", machine_name, "   R1       - forward", NULL);

	cellDbgFontPuts(0.05f, 0.88f, Emulator_GetFontSize(), YELLOW, "UP/DOWN   - select    X/LEFT/RIGHT - change          TRIANGLE - default");
	cellDbgFontPuts(0.05f, 0.92f, Emulator_GetFontSize(), YELLOW, footer);
	cellDbgFontDraw();
}

#define MAX_NO_OF_DATASETTE_CONTROLS 7

const char *datasette_control_string[MAX_NO_OF_DATASETTE_CONTROLS] = { 
    "Press STOP",
    "Press PLAY",
    "Press FFD",
    "Press REW",
    "Press REC",
    "Reset Counter", 
    "Reset"};


// void do_datasette_controls()
// // called from ROM menu by pressing the SELECT button
// // return to ROM menu by pressing the CIRCLE button
void do_datasette_controls(void)
{
	static const char *footer = NULL;

	if(CellInput->UpdateDevice(0) == CELL_OK)
	{
		// back to ROM menu if CIRCLE is pressed
		if (CellInput->WasButtonPressed(0, CTRL_L1) | CellInput->WasButtonPressed(0, CTRL_CIRCLE))
		{
			menuStack.pop();
			return;
		}

		if (CellInput->WasButtonPressed(0,CTRL_DOWN) | (CellInput->IsAnalogPressedDownPercentage(0,CTRL_LSTICK) > 0.10) )
		{
			currently_selected_datasette_control++;
			if (currently_selected_datasette_control >= MAX_NO_OF_DATASETTE_CONTROLS)
			{
				currently_selected_datasette_control = 0;
			}
		}

		if (CellInput->WasButtonPressed(0,CTRL_START))
		{
			MenuStop();
			Emulator_StartROMRunning();
			return;
		}


		if (CellInput->WasButtonPressed(0,CTRL_UP) | (CellInput->IsAnalogPressedUpPercentage(0,CTRL_LSTICK) > 0.10) )
		{
			currently_selected_datasette_control--;
			if (currently_selected_datasette_control < 0)
			{
				currently_selected_datasette_control = MAX_NO_OF_DATASETTE_CONTROLS-1;
			}
		}

		if (CellInput->WasButtonPressed(0,CTRL_CROSS))
		{
			datasette_control (currently_selected_datasette_control);
			MenuStop();
			Emulator_StartROMRunning();
			return;
		}
	}

	float yPos = 0.13;
	float ySpacing = 0.04;

	cellDbgFontPuts (0.09f, 0.05f, Emulator_GetFontSize(), BLUE,   "GENERAL");
	cellDbgFontPuts (0.26f,  0.05f, Emulator_GetFontSize(), BLUE,  "VICE");
	cellDbgFontPuts (0.44f, 0.05f, Emulator_GetFontSize(), BLUE,   "PATHS");
	cellDbgFontPuts (0.61f, 0.05f, Emulator_GetFontSize(), BLUE,   "CONTROLS");
	cellDbgFontPuts (0.78f, 0.05f, Emulator_GetFontSize(), PURPLE, "DATASETTE");
	cellDbgFontDraw();


	for (int i=0; i < MAX_NO_OF_DATASETTE_CONTROLS; i++) {
		if (i > 0)
			yPos += ySpacing;

		cellDbgFontPuts (0.09f, yPos, Emulator_GetFontSize(), currently_selected_datasette_control == i ? YELLOW : WHITE, datasette_control_string[i]);
	}


	if (footer == NULL)
		footer = util_concat("L1/CIRCLE - back      START        - return to ", machine_name, NULL);

	cellDbgFontPuts(0.05f, 0.88f, Emulator_GetFontSize(), YELLOW, "UP/DOWN   - select    X");
	cellDbgFontPuts(0.05f, 0.92f, Emulator_GetFontSize(), YELLOW, footer);
	cellDbgFontDraw();
}

void inline process_drive_change (const char *res, int default_type)
{
	int drive_type_index=0;
	int res_value;                                        
	resources_get_int(res, &res_value);

	for (int i = 0; i <= MAX_DRIVE_TYPES; i++) {
		// find the resource value in the list of values
		if (drive_type[i].id == res_value) {
			drive_type_index=i;
			break;
		}
	}
	if(CellInput->WasButtonPressed(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
	{
		if (drive_type_index > 0) {
			drive_type_index--;
			res_value = drive_type[drive_type_index].id;
			resources_set_int(res, res_value);
		}
	}
	if(CellInput->WasButtonPressed(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK) | CellInput->WasButtonPressed(0,CTRL_CROSS))
	{
		if (drive_type_index < MAX_DRIVE_TYPES) {
			drive_type_index++;
			res_value = drive_type[drive_type_index].id;
			resources_set_int(res, res_value);
		}
	}
	if(CellInput->IsButtonPressed(0, CTRL_TRIANGLE))
	{
		res_value = default_type;
		resources_set_int(res, res_value);
	}
}


void do_vice_settings()
{
	static const char *footer = NULL;
	int res_value;
	int model_index=0;

	int sid_engine;
	int sid_model;
	int SidEngineModel;

	bool SidOptionPassbandDisabled = false;
	bool MouseDisabled = false;


	resources_get_int("SidEngine", &res_value);
	if (res_value == SID_ENGINE_FASTSID) {
		SidOptionPassbandDisabled = true;
	} else if (res_value == SID_ENGINE_RESID) {
		int sampling_mode;
		if (resources_get_int("SidResidSampling", &sampling_mode) == 0) {
			if (sampling_mode == SAMPLE_FAST) {
				SidOptionPassbandDisabled = true;
			}
			if (sampling_mode == SAMPLE_INTERPOLATE) {
				SidOptionPassbandDisabled = true;
			}
			if (sampling_mode == SAMPLE_RESAMPLE_INTERPOLATE) {
				SidOptionPassbandDisabled = false;
			}
		}
	} else if (res_value == SID_ENGINE_RESID_FP) {
		int sampling_mode;
		if (resources_get_int("SidResidSampling", &sampling_mode) == 0) {
			if (sampling_mode == SAMPLE_FAST) {
				SidOptionPassbandDisabled = true;
			}
			if (sampling_mode == SAMPLE_INTERPOLATE) {
				SidOptionPassbandDisabled = true;
			}
			if (sampling_mode == SAMPLE_RESAMPLE_INTERPOLATE) {
				SidOptionPassbandDisabled = false;
			}
		}
	}

	if (resources_get_int("Mouse", &res_value) == 0)
		MouseDisabled = !res_value;


	if(CellInput->UpdateDevice(0) == CELL_PAD_OK)
	{
		// back to ROM menu if CIRCLE is pressed
		if (CellInput->WasButtonPressed(0, CTRL_L1) | CellInput->WasButtonPressed(0, CTRL_CIRCLE))
		{
			menuStack.pop();
			return;
		}

		if (CellInput->WasButtonPressed(0, CTRL_R1))
		{
			menuStack.push(do_path_settings);
			return;
		}

		if (CellInput->WasButtonPressed(0,CTRL_DOWN) | (CellInput->IsAnalogPressedDownPercentage(0,CTRL_LSTICK) > 0.10) )
		{
			currently_selected_vice_setting++;
			if (currently_selected_vice_setting >= MAX_NO_OF_VICE_SETTINGS)
			{
				currently_selected_vice_setting = 0;
			}
		}
		if (CellInput->WasButtonPressed(0,CTRL_UP) | (CellInput->IsAnalogPressedUpPercentage(0,CTRL_LSTICK) > 0.10) )
		{
			currently_selected_vice_setting--;
			if (currently_selected_vice_setting < 0)
			{
				currently_selected_vice_setting = MAX_NO_OF_VICE_SETTINGS-1;
			}
		}

		if (CellInput->WasButtonPressed(0,CTRL_START))
		{
			MenuStop();
			Emulator_StartROMRunning();
			return;
		}

		bool dirty=false;
		switch(currently_selected_vice_setting)
		{
			// display framerate on/off
			case SETTING_VICE_DISPLAY_FRAMERATE:
				resources_get_int("DisplayFrameRate", &res_value);
				if(CellInput->WasButtonPressed(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_CROSS))
				{
					res_value = !res_value;
					resources_set_int("DisplayFrameRate", res_value);
				}
				if(CellInput->IsButtonPressed(0, CTRL_TRIANGLE))
				{
					res_value = false;
					resources_set_int("DisplayFrameRate", res_value);
				}
				break;
			case SETTING_VICE_DISPLAY_DRIVE_INDICATORS:
				resources_get_int("DisplayDriveIndicators", &res_value);
				if(CellInput->WasButtonPressed(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_CROSS))
				{
					res_value = !res_value;
					resources_set_int("DisplayDriveIndicators", res_value);
				}
				if(CellInput->IsButtonPressed(0, CTRL_TRIANGLE))
				{
					res_value = true;
					resources_set_int("DisplayDriveIndicators", res_value);
				}
				break;
			case SETTING_VICE_SID_FILTERS:
				resources_get_int("SidFilters", &res_value);
				if(CellInput->WasButtonPressed(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_CROSS))
				{
					res_value = !res_value;
					resources_set_int("SidFilters", res_value);
				}
				if(CellInput->IsButtonPressed(0, CTRL_TRIANGLE))
				{
					res_value = true;
					resources_set_int("SidFilters", res_value);
				}
				break;
			case SETTING_VICE_SID_MODEL:
				dirty=false;

				resources_get_int("SidEngine", &sid_engine);
				resources_get_int("SidModel", &sid_model);
				SidEngineModel = (sid_engine << 8) | sid_model;

				for (int i=MIN_ALL_SID_MODELS; i <= MAX_ALL_SID_MODELS; i++)
				{
					if (ui_sid_engine_model_id[i] == SidEngineModel)
					{
						model_index=i;
						break;
					}
				}

				if(CellInput->WasButtonPressed(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
				{
					if (model_index > MIN_ALL_SID_MODELS) {
						model_index--;
						SidEngineModel = ui_sid_engine_model_id[model_index];
						dirty=true;
					}
				}
				if(CellInput->WasButtonPressed(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK) | CellInput->WasButtonPressed(0,CTRL_CROSS))
				{
					if (model_index < MAX_ALL_SID_MODELS) {
						model_index++;
						SidEngineModel = ui_sid_engine_model_id[model_index];
						dirty=true;
					}
				}
				if(CellInput->IsButtonPressed(0, CTRL_TRIANGLE))
				{
					SidEngineModel = SID_RESIDFP_6581R4AR_3789;
					dirty=true;
				}

				if (dirty==false)
					break;


				sid_engine = SidEngineModel >> 8;
				sid_model  = SidEngineModel & 0xff;

				for (int i=MIN_ALL_SID_MODELS; i <= MAX_ALL_SID_MODELS; i++)
				{
					if (ui_sid_engine_model_id[i] == SidEngineModel)
					{
						model_index=i;
						break;
					}
				}

				resources_set_int("SidEngine", sid_engine);
				if (model_index < MIN_RESID_MODELS)
					resources_set_int("SidResidSampling", SAMPLE_FAST);
				else if (model_index < MIN_RESID_FP_MODELS)
					resources_set_int("SidResidSampling", SAMPLE_INTERPOLATE);
				else
					resources_set_int("SidResidSampling", SAMPLE_RESAMPLE_INTERPOLATE);

				resources_set_int("SidModel", sid_model);

				//sid_set_engine_model(sid_engine, sid_model);

				break;
			case SETTING_VICE_SID_RESID_PASSBAND:
				if (SidOptionPassbandDisabled)
					break;

				resources_get_int("SidResidPassband", &res_value);
				if(CellInput->WasButtonPressed(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
				{
					if (res_value >= 30)
					{
						res_value -= 10;
						resources_set_int("SidResidPassband", res_value);
					}
				}
				if(CellInput->WasButtonPressed(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK) | CellInput->WasButtonPressed(0,CTRL_CROSS))
				{
					if (res_value <= 80)
					{
						res_value += 10;
						resources_set_int("SidResidPassband", res_value);
					}
				}
				if(CellInput->IsButtonPressed(0, CTRL_TRIANGLE))
				{
					res_value = 90;
					resources_set_int("SidResidPassband", res_value);
				}
				break;
			case SETTING_VICE_MOUSE_SUPPORT:
				resources_get_int("Mouse", &res_value);
				if(CellInput->WasButtonPressed(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_CROSS))
				{
					res_value = !res_value;
					resources_set_int("Mouse", res_value);
				}
				if(CellInput->IsButtonPressed(0, CTRL_TRIANGLE))
				{
					res_value = false;
					resources_set_int("Mouse", res_value);
				}
				break;
			case SETTING_VICE_MOUSE_TYPE:
				if (MouseDisabled)
					break;

				resources_get_int("Mousetype", &res_value);
				if(CellInput->WasButtonPressed(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_CROSS))
				{
					res_value = (res_value==MOUSE_TYPE_1351) ? MOUSE_TYPE_NEOS : MOUSE_TYPE_1351;
					resources_set_int("Mousetype", res_value);
				}
				if(CellInput->IsButtonPressed(0, CTRL_TRIANGLE))
				{
					res_value = MOUSE_TYPE_1351;
					resources_set_int("Mousetype", res_value);
				}
				break;
			case SETTING_VICE_SWAP_JOYSTICKS:
				if(CellInput->WasButtonPressed(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_CROSS))
				{
					joyswap=!joyswap;
				}
				if(CellInput->IsButtonPressed(0, CTRL_TRIANGLE))
				{
					joyswap=false;
				}
				break;
			case SETTING_VICE_KEYMAP:
				resources_get_int("KeymapIndex", &res_value);
				if(CellInput->WasButtonPressed(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
				{
					if (res_value > 0) {
						res_value--;
						resources_set_int("KeymapIndex", res_value);
					}
				}
				if(CellInput->WasButtonPressed(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK))
				{
					if (res_value < 1) {
						res_value++;
						resources_set_int("KeymapIndex", res_value);
					}
				}
				if(CellInput->IsButtonPressed(0, CTRL_TRIANGLE))
				{
					res_value = 0;
					resources_set_int("KeymapIndex", res_value);
				}
				break;
			case SETTING_VICE_DRIVE8_TYPE:
				process_drive_change("Drive8Type", DRIVE_TYPE_1541);
				break;
			case SETTING_VICE_DRIVE9_TYPE:
				process_drive_change("Drive9Type", DRIVE_TYPE_NONE);
				break;
			case SETTING_VICE_DRIVE10_TYPE:
				process_drive_change("Drive10Type", DRIVE_TYPE_NONE);
				break;
			case SETTING_VICE_DRIVE11_TYPE:
				process_drive_change("Drive11Type", DRIVE_TYPE_NONE);
				break;
			case SETTING_VICE_HARD_RESET:
				if (CellInput->WasButtonPressed(0, CTRL_CROSS))
				{
					machine_trigger_reset(MACHINE_RESET_MODE_HARD);
				}
				break;
			case SETTING_VICE_SAVE_SETTINGS:
				if(CellInput->WasButtonPressed(0, CTRL_TRIANGLE) | CellInput->WasButtonPressed(0, CTRL_CROSS))
				{
					resources_save(NULL);
				}
				break;
			case SETTING_VICE_DEFAULT_ALL:
				if(CellInput->IsButtonPressed(0, CTRL_TRIANGLE) | CellInput->WasButtonPressed(0, CTRL_CROSS))
				{
					resources_set_int ("DisplayFrameRate", false);
					resources_set_int ("DisplayDriveIndicators", true);
					resources_set_int ("SidFilters", true);
					resources_set_int ("SidEngine", SID_ENGINE_RESID_FP);
					resources_set_int ("SidResidSampling", SAMPLE_RESAMPLE_INTERPOLATE);
					resources_set_int ("SidEngine", (SID_RESIDFP_6581R4AR_3789 >> 8) );
					resources_set_int ("SidModel",  (SID_RESIDFP_6581R4AR_3789 & 0xff) );
					resources_set_int ("SidResidPassband", 90);
					resources_set_int ("Mouse", false);
					resources_set_int ("Mousetype", MOUSE_TYPE_1351);
					joyswap=false;
					resources_set_int ("Drive8Type", DRIVE_TYPE_1541);
					resources_set_int ("Drive9Type", DRIVE_TYPE_NONE);
					resources_set_int ("Drive10Type", DRIVE_TYPE_NONE);
					resources_set_int ("Drive11Type", DRIVE_TYPE_NONE);
				}
				break;

			default:
				break;
		} // end of switch
	}

	float yPos=0.05f;
	float ybrk=0.04f;

	cellDbgFontPuts (0.09f, 0.05f, Emulator_GetFontSize(), BLUE,    "GENERAL");
	cellDbgFontPuts (0.26f,  0.05f, Emulator_GetFontSize(), PURPLE, "VICE");
	cellDbgFontPuts (0.44f, 0.05f, Emulator_GetFontSize(), BLUE,    "PATHS");
	cellDbgFontPuts (0.61f, 0.05f, Emulator_GetFontSize(), BLUE,    "CONTROLS");
	cellDbgFontPuts (0.78f, 0.05f, Emulator_GetFontSize(), BLUE,    "DATASETTE");
	cellDbgFontDraw();

	yPos+=ybrk;
	yPos+=ybrk;
	resources_get_int("DisplayFrameRate", &res_value);
	cellDbgFontPuts (0.09f, yPos, Emulator_GetFontSize(), currently_selected_vice_setting == SETTING_VICE_DISPLAY_FRAMERATE ? YELLOW : WHITE, "Display framerate");
	cellDbgFontPuts (0.5f,  yPos, Emulator_GetFontSize(), res_value == false ? GREEN : RED, res_value == true ? "ON" : "OFF");
	cellDbgFontDraw();

	yPos+=ybrk;
	resources_get_int("DisplayDriveIndicators", &res_value);
	cellDbgFontPuts (0.09f, yPos, Emulator_GetFontSize(), currently_selected_vice_setting == SETTING_VICE_DISPLAY_DRIVE_INDICATORS ? YELLOW : WHITE, "Show Disk/Tape Activity");
	cellDbgFontPuts (0.5f,  yPos, Emulator_GetFontSize(), res_value == false ? GREEN : RED, res_value == true ? "ON" : "OFF");
	cellDbgFontDraw();

	yPos+=ybrk;
	resources_get_int("SidFilters", &res_value);
	cellDbgFontPuts (0.09f, yPos, Emulator_GetFontSize(), currently_selected_vice_setting == SETTING_VICE_SID_FILTERS ? YELLOW : WHITE, "SID Filtering Enabled");
	cellDbgFontPuts (0.5f,  yPos, Emulator_GetFontSize(), res_value == true ? GREEN : RED, res_value == true ? "ON" : "OFF");
	cellDbgFontDraw();


	yPos+=ybrk;
	resources_get_int("SidEngine", &res_value);
	cellDbgFontPuts (0.09f, yPos, Emulator_GetFontSize(), currently_selected_vice_setting == SETTING_VICE_SID_ENGINE ? YELLOW : WHITE, "SID Engine");

	switch (res_value) {
		case SID_ENGINE_FASTSID:
			cellDbgFontPuts        (0.5f, yPos, Emulator_GetFontSize(), GRAY, "FastSID   (low quality)");
			break;
		case SID_ENGINE_RESID:
			cellDbgFontPuts        (0.5f, yPos, Emulator_GetFontSize(), GRAY, "ReSID     (medium quality)");
			break;
		case SID_ENGINE_RESID_FP:
			cellDbgFontPuts        (0.5f, yPos, Emulator_GetFontSize(), GRAY, "ReSID-FP  (highest quality)");
			break;
	}
	cellDbgFontDraw();




	yPos+=ybrk;
	resources_get_int("SidResidSampling", &res_value);
	cellDbgFontPuts (0.09f, yPos, Emulator_GetFontSize(), currently_selected_vice_setting == SETTING_VICE_SID_RESID_SAMPLING ? YELLOW : WHITE, "ReSID Sampling Mode");
	cellDbgFontDraw();

	switch (res_value) {
		case SAMPLE_FAST:
			cellDbgFontPuts (0.5f, yPos, Emulator_GetFontSize(), GRAY, "Fast (lowest quality)");
			break;
		case SAMPLE_INTERPOLATE:
			cellDbgFontPuts (0.5f, yPos, Emulator_GetFontSize(), GRAY, "Interpolate (medium quality)");
			break;
		case SAMPLE_RESAMPLE_INTERPOLATE:
			cellDbgFontPuts (0.5f, yPos, Emulator_GetFontSize(), GRAY, "Resampling (highest quality)");
			break;
	}



	yPos+=ybrk;
	resources_get_int("SidEngine", &sid_engine);
	resources_get_int("SidModel", &sid_model);
	SidEngineModel = (sid_engine << 8) | sid_model;
	for (int i=MIN_ALL_SID_MODELS; i <= MAX_ALL_SID_MODELS; i++)
	{
		if (ui_sid_engine_model_id[i] == SidEngineModel)
		{
			model_index=i;
			break;
		}
	}
	cellDbgFontPuts (0.09f, yPos, Emulator_GetFontSize(), currently_selected_vice_setting == SETTING_VICE_SID_MODEL ? YELLOW : WHITE, "SID Model");
	cellDbgFontPuts (0.5f,  yPos, Emulator_GetFontSize(), ui_sid_engine_model_id[model_index] == SID_RESIDFP_6581R4AR_3789 ? GREEN : RED, ui_sid_engine_model[model_index]);
	cellDbgFontDraw();

	yPos+=ybrk;
	resources_get_int("SidResidPassband", &res_value);
	cellDbgFontPuts (0.09f, yPos, Emulator_GetFontSize(), currently_selected_vice_setting == SETTING_VICE_SID_RESID_PASSBAND ? YELLOW : WHITE, "ReSID Passband Filter (20-90)");
	cellDbgFontPrintf(0.5f, yPos, Emulator_GetFontSize(), SidOptionPassbandDisabled==true ? GRAY : (res_value == 90 ? GREEN : RED), "%d", res_value);
	cellDbgFontDraw();


	yPos+=ybrk;
	resources_get_int("Mouse", &res_value);
	cellDbgFontPuts (0.09f, yPos, Emulator_GetFontSize(), currently_selected_vice_setting == SETTING_VICE_MOUSE_SUPPORT ? YELLOW : WHITE, "Mouse Enabled");
	cellDbgFontPuts (0.5f,  yPos, Emulator_GetFontSize(), res_value == false ? GREEN : RED, res_value == true ? "ON":"OFF");
	cellDbgFontDraw();


	yPos+=ybrk;
	resources_get_int("Mousetype", &res_value);
	cellDbgFontPuts (0.09f, yPos, Emulator_GetFontSize(), currently_selected_vice_setting == SETTING_VICE_MOUSE_TYPE ? YELLOW : WHITE, "Mouse Type");
	cellDbgFontPuts (0.5f,  yPos, Emulator_GetFontSize(), MouseDisabled==true ? GRAY : (res_value == MOUSE_TYPE_1351 ? GREEN : RED), res_value == MOUSE_TYPE_1351 ? "CBM 1351" : "NEOS");
	cellDbgFontDraw();


	yPos+=ybrk;
	cellDbgFontPuts (0.09f, yPos, Emulator_GetFontSize(), currently_selected_vice_setting == SETTING_VICE_SWAP_JOYSTICKS ? YELLOW : WHITE, "Swap Joysticks");
	cellDbgFontPuts (0.5f,  yPos, Emulator_GetFontSize(), joyswap==false ? GREEN : RED, joyswap == true ? "ON" : "OFF");
	cellDbgFontDraw();

	yPos+=ybrk;
	cellDbgFontPuts (0.09f, yPos, Emulator_GetFontSize(), currently_selected_vice_setting == SETTING_VICE_KEYMAP ? YELLOW : WHITE, "Keyboard Mapping");
	resources_get_int("KeymapIndex", &res_value);
	cellDbgFontPuts (0.5f,  yPos, Emulator_GetFontSize(), res_value == 0 ? GREEN : RED, res_value == 0 ? "Symbolic" : "Positional");
	cellDbgFontDraw();


	yPos+=ybrk;
	cellDbgFontPuts (0.09f, yPos, Emulator_GetFontSize(), currently_selected_vice_setting == SETTING_VICE_DRIVE8_TYPE ? YELLOW : WHITE, "Drive 8");
	resources_get_int("Drive8Type", &res_value);
	// find the resource value in the list of values
	for (int i = 0; i <= MAX_DRIVE_TYPES; i++)
	{
		if (drive_type[i].id == res_value)
		{
			cellDbgFontPuts (0.5f, yPos, Emulator_GetFontSize(), res_value == DRIVE_TYPE_1541 ? GREEN : RED, drive_type[i].name);
			break;
		}
	}
	cellDbgFontDraw();

	yPos+=ybrk;
	resources_get_int("Drive9Type", &res_value);
	cellDbgFontPuts (0.09f, yPos, Emulator_GetFontSize(), currently_selected_vice_setting == SETTING_VICE_DRIVE9_TYPE ? YELLOW : WHITE, "Drive 9");
	// find the resource value in the list of values
	for (int i = 0; i <= MAX_DRIVE_TYPES; i++) {
		if (drive_type[i].id == res_value) {
			cellDbgFontPuts (0.5f, yPos, Emulator_GetFontSize(), res_value == DRIVE_TYPE_NONE ? GREEN : RED, drive_type[i].name);
			break;
		}
	}
	cellDbgFontDraw();

	yPos+=ybrk;
	resources_get_int("Drive10Type", &res_value);
	cellDbgFontPuts (0.09f, yPos, Emulator_GetFontSize(), currently_selected_vice_setting == SETTING_VICE_DRIVE10_TYPE ? YELLOW : WHITE, "Drive 10");
	// find the resource value in the list of values
	for (int i = 0; i <= MAX_DRIVE_TYPES; i++) {
		if (drive_type[i].id == res_value) {
			cellDbgFontPuts (0.5f, yPos, Emulator_GetFontSize(), res_value == DRIVE_TYPE_NONE ? GREEN : RED, drive_type[i].name);
			break;
		}
	}
	cellDbgFontDraw();

	yPos+=ybrk;
	resources_get_int("Drive11Type", &res_value);
	cellDbgFontPuts (0.09f, yPos, Emulator_GetFontSize(), currently_selected_vice_setting == SETTING_VICE_DRIVE11_TYPE ? YELLOW : WHITE, "Drive 11");
	// find the resource value in the list of values
	for (int i = 0; i <= MAX_DRIVE_TYPES; i++) {
		if (drive_type[i].id == res_value) {
			cellDbgFontPuts (0.5f, yPos, Emulator_GetFontSize(), res_value == DRIVE_TYPE_NONE ? GREEN : RED, drive_type[i].name);
			break;
		}
	}
	cellDbgFontDraw();

	yPos+=ybrk;
	yPos+=ybrk;
	cellDbgFontPuts (0.09f, yPos, Emulator_GetFontSize(), currently_selected_vice_setting == SETTING_VICE_HARD_RESET ? YELLOW : GREEN, "ISSUE HARD RESET");
	cellDbgFontDraw();


	yPos+=ybrk;
	cellDbgFontPuts (0.09f, yPos, Emulator_GetFontSize(), currently_selected_vice_setting == SETTING_VICE_SAVE_SETTINGS ? YELLOW : GREEN, "SAVE SETTINGS");
	cellDbgFontDraw();


	yPos+=ybrk;
	cellDbgFontPrintf(0.09f, yPos, Emulator_GetFontSize(), currently_selected_vice_setting == SETTING_VICE_DEFAULT_ALL ? YELLOW : GREEN, "DEFAULT");
	cellDbgFontDraw();


	if (footer == NULL)
		footer = util_concat("L1/CIRCLE - back      START        - return to ", machine_name, "   R1       - forward", NULL);

	cellDbgFontPuts(0.05f, 0.88f, Emulator_GetFontSize(), YELLOW,             "UP/DOWN   - select    X/LEFT/RIGHT - change          TRIANGLE - default");
	cellDbgFontPuts(0.05f, 0.92f, Emulator_GetFontSize(), YELLOW, footer);


	cellDbgFontDraw();
}

void do_general_settings(void)
{
	static const char *footer = NULL;
	int res_int;
	int overscan_enabled;
	int overscan_amount;

	if(CellInput->UpdateDevice(0) == CELL_PAD_OK)
	{
		// back to ROM menu if CIRCLE is pressed
		if (CellInput->WasButtonPressed(0, CTRL_CIRCLE))
		{
			menuStack.pop();
			return;
		}

		if (CellInput->WasButtonPressed(0, CTRL_R1))
		{
			menuStack.push(do_vice_settings);
			return;
		}

		if (CellInput->WasButtonPressed(0,CTRL_DOWN) | (CellInput->IsAnalogPressedDownPercentage(0,CTRL_LSTICK) > 0.10) )
		{
			currently_selected_setting++;
			if (currently_selected_setting >= MAX_NO_OF_SETTINGS)
			{
				currently_selected_setting = 0;
			}
		}

		if (CellInput->WasButtonPressed(0,CTRL_UP) | (CellInput->IsAnalogPressedUpPercentage(0,CTRL_LSTICK) > 0.10) )
		{
			currently_selected_setting--;
			if (currently_selected_setting < 0)
			{
				currently_selected_setting = MAX_NO_OF_SETTINGS-1;
			}
		}

		if (CellInput->WasButtonPressed(0,CTRL_START))
		{
			MenuStop();
			Emulator_StartROMRunning();
			return;
		}

		switch(currently_selected_setting)
		{
			// display framerate on/off
			case SETTING_CHANGE_RESOLUTION:
				resources_get_int ("PS3Pal60", &res_int);
				if(CellInput->WasButtonPressed(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK))
				{
					Graphics->NextResolution();
				}
				if(CellInput->WasButtonPressed(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
				{
					Graphics->PreviousResolution();
				}
				if(CellInput->WasButtonPressed(0, CTRL_CROSS))
				{
					if (Graphics->GetCurrentResolution() == CELL_VIDEO_OUT_RESOLUTION_576)
					{
						if(Graphics->CheckResolution(CELL_VIDEO_OUT_RESOLUTION_576))
						{
							Graphics->SetPAL60Hz(res_int);
							Graphics->SwitchResolution(Graphics->GetCurrentResolution(), res_int);
						}
					}
					else
					{
						Graphics->SetPAL60Hz(false);
						Graphics->SwitchResolution(Graphics->GetCurrentResolution(), false);
					}
				}
				if(CellInput->IsButtonPressed(0, CTRL_TRIANGLE))
				{
					Graphics->SwitchResolution(Graphics->GetInitialResolution(), res_int);
				}
				break;
			case SETTING_PAL60_MODE:
				resources_get_int ("PS3Pal60", &res_int);
				if(CellInput->WasButtonPressed(0, CTRL_RIGHT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK) | CellInput->WasButtonPressed(0,CTRL_CROSS) | CellInput->WasButtonPressed(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK))
				{
					if (Graphics->GetCurrentResolution() == CELL_VIDEO_OUT_RESOLUTION_576)
					{
						if(Graphics->CheckResolution(CELL_VIDEO_OUT_RESOLUTION_576))
						{
							res_int = ! res_int;
							resources_set_int ("PS3Pal60", res_int);
							Graphics->SetPAL60Hz(res_int);
							Graphics->SwitchResolution(Graphics->GetCurrentResolution(), res_int);
						}
					}

				}
				break;
			case SETTING_SHADER:
				if(CellInput->WasButtonPressed(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_CROSS))
				{
					menuStack.push(do_shaderChoice);
					tmpBrowser = NULL;
				}
				if(CellInput->IsButtonPressed(0, CTRL_TRIANGLE))
				{
					Graphics->LoadFragmentShader(DEFAULT_SHADER_FILE);
					Graphics->SetSmooth(false);
					resources_set_int ("PS3HardwareFilter", false);
				}
				break;
			case SETTING_FONT_SIZE:
				resources_get_int ("PS3FontSize", &res_int);
				if(CellInput->WasButtonPressed(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK) | CellInput->WasButtonPressed(0,CTRL_CROSS))
				{
					if(res_int > -100)
						res_int--;
				}
				if(CellInput->WasButtonPressed(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK) | CellInput->WasButtonPressed(0,CTRL_CROSS))
				{
					if((res_int < 100))
						res_int++;
				}
				if(CellInput->IsButtonPressed(0, CTRL_TRIANGLE))
				{
					res_int = 100;
				}
				resources_set_int ("PS3FontSize", res_int);
				break;
			case SETTING_KEEP_ASPECT_RATIO:
				resources_get_int ("PS3KeepAspect", &res_int);
				if(CellInput->WasButtonPressed(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK) | CellInput->WasButtonPressed(0,CTRL_CROSS))
				{
					res_int = !res_int;
					Graphics->SetAspectRatio(res_int);
				}
				if(CellInput->IsButtonPressed(0, CTRL_TRIANGLE))
				{
					res_int = false;
					Graphics->SetAspectRatio(res_int);
				}
				resources_set_int ("PS3KeepAspect", res_int);
				break;
			case SETTING_HW_TEXTURE_FILTER:
				resources_get_int ("PS3HardwareFilter", &res_int);
				if(CellInput->WasButtonPressed(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK) | CellInput->WasButtonPressed(0,CTRL_CROSS))
				{
					res_int = !res_int;
					Graphics->SetSmooth(res_int);
				}
				if(CellInput->IsButtonPressed(0, CTRL_TRIANGLE))
				{
					res_int = false;
					Graphics->SetSmooth(res_int);
				}
				resources_set_int ("PS3HardwareFilter", res_int);
				break;
			case SETTING_HW_OVERSCAN_AMOUNT:
				resources_get_int ("PS3OverscanAmount", &overscan_amount);
				resources_get_int ("PS3Overscan", &overscan_enabled);
				if(CellInput->WasButtonPressed(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0,CTRL_LSTICK) | CellInput->WasButtonPressed(0,CTRL_CROSS))
				{
					if(overscan_amount > -40)
					{
						overscan_amount--;
						overscan_enabled = true;
						Graphics->SetOverscan(overscan_enabled, (float)overscan_amount/100);
					}
					if(overscan_amount == 0)
					{
						overscan_enabled = false;
						Graphics->SetOverscan(overscan_enabled, (float)overscan_amount/100);
					}
				}
				if(CellInput->WasButtonPressed(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0,CTRL_LSTICK) | CellInput->WasButtonPressed(0,CTRL_CROSS))
				{
					if((overscan_amount < 40))
					{
						overscan_amount++;
						overscan_enabled = true;
						Graphics->SetOverscan(overscan_enabled, (float)overscan_amount/100);
					}
					if(overscan_amount == 0)
					{
						overscan_enabled = false;
						Graphics->SetOverscan(overscan_enabled, (float)overscan_amount/100);
					}
				}
				if(CellInput->IsButtonPressed(0, CTRL_TRIANGLE))
				{
					overscan_amount = 0;
					overscan_enabled = false;
					Graphics->SetOverscan(overscan_enabled, (float)overscan_amount/100);
				}
				resources_set_int ("PS3Overscan", overscan_enabled);
				resources_set_int ("PS3OverscanAmount", overscan_amount);
				break;
			case SETTING_SAVE_SETTINGS:
				if(CellInput->WasButtonPressed(0, CTRL_LEFT) | CellInput->IsAnalogPressedLeft(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_RIGHT) | CellInput->IsAnalogPressedRight(0, CTRL_LSTICK) | CellInput->WasButtonPressed(0, CTRL_CROSS))
				{
					resources_save(NULL);
				}
				if(CellInput->IsButtonPressed(0, CTRL_TRIANGLE))
				{
					resources_save(NULL);
				}
				break;
			case SETTING_DEFAULT_ALL:
				if(CellInput->WasButtonPressed(0, CTRL_CROSS) | CellInput->IsButtonPressed(0, CTRL_TRIANGLE))
				{
					resources_set_int ("PS3KeepAspect", false);
					resources_set_int ("PS3HardwareFilter", false);
					resources_set_int ("PS3Overscan", false);
					resources_set_int ("PS3OverscanAmount", 0);
					resources_set_int ("PS3KeepAspect", false);
					resources_set_int ("PS3Pal60", false);
					resources_set_int ("PS3FontSize", 100);

					Graphics->SetAspectRatio(false);
					Graphics->SetSmooth(false);
					Graphics->SetOverscan(false, (float)0);
					Graphics->SetPAL60Hz(false);
					//Settings.RSoundEnabled = false;
					//Settings.RSoundServerIPAddress = "0.0.0.0";
				}
				break;
			default:
				break;
		} // end of switch
	}

	cellDbgFontPuts (0.09f, 0.05f, Emulator_GetFontSize(), PURPLE, "GENERAL");
	cellDbgFontPuts (0.26f,  0.05f, Emulator_GetFontSize(), BLUE,  "VICE");
	cellDbgFontPuts (0.44f, 0.05f, Emulator_GetFontSize(), BLUE,   "PATHS");
	cellDbgFontPuts (0.61f, 0.05f, Emulator_GetFontSize(), BLUE,   "CONTROLS");
	cellDbgFontPuts (0.78f, 0.05f, Emulator_GetFontSize(), BLUE,   "DATASETTE");

	float yPos = 0.13;
	float ySpacing = 0.04;

	cellDbgFontPuts(0.09f, yPos, Emulator_GetFontSize(), currently_selected_setting == SETTING_CHANGE_RESOLUTION ? YELLOW : WHITE, "Resolution");

	switch(Graphics->GetCurrentResolution())
	{
		case CELL_VIDEO_OUT_RESOLUTION_480:
			cellDbgFontPrintf(0.5f, yPos, Emulator_GetFontSize(), Graphics->GetInitialResolution() == CELL_VIDEO_OUT_RESOLUTION_480 ? GREEN : RED, "720x480 (480p)");
			break;
		case CELL_VIDEO_OUT_RESOLUTION_720:
			cellDbgFontPrintf(0.5f, yPos, Emulator_GetFontSize(), Graphics->GetInitialResolution() == CELL_VIDEO_OUT_RESOLUTION_720 ? GREEN : RED, "1280x720 (720p)");
			break;
		case CELL_VIDEO_OUT_RESOLUTION_1080:
			cellDbgFontPrintf(0.5f, yPos, Emulator_GetFontSize(), Graphics->GetInitialResolution() == CELL_VIDEO_OUT_RESOLUTION_1080 ? GREEN : RED, "1920x1080 (1080p)");
			break;
		case CELL_VIDEO_OUT_RESOLUTION_576:
			cellDbgFontPrintf(0.5f, yPos, Emulator_GetFontSize(), Graphics->GetInitialResolution() == CELL_VIDEO_OUT_RESOLUTION_576 ? GREEN : RED, "720x576 (576p)");
			break;
		case CELL_VIDEO_OUT_RESOLUTION_1600x1080:
			cellDbgFontPrintf(0.5f, yPos, Emulator_GetFontSize(), Graphics->GetInitialResolution() == CELL_VIDEO_OUT_RESOLUTION_1600x1080 ? GREEN : RED, "1600x1080");
			break;
		case CELL_VIDEO_OUT_RESOLUTION_1440x1080:
			cellDbgFontPrintf(0.5f, yPos, Emulator_GetFontSize(), Graphics->GetInitialResolution() == CELL_VIDEO_OUT_RESOLUTION_1440x1080 ? GREEN : RED, "1440x1080");
			break;
		case CELL_VIDEO_OUT_RESOLUTION_1280x1080:
			cellDbgFontPrintf(0.5f, yPos, Emulator_GetFontSize(), Graphics->GetInitialResolution() == CELL_VIDEO_OUT_RESOLUTION_1280x1080 ? GREEN : RED, "1280x1080");
			break;
		case CELL_VIDEO_OUT_RESOLUTION_960x1080:
			cellDbgFontPrintf(0.5f, yPos, Emulator_GetFontSize(), Graphics->GetInitialResolution() == CELL_VIDEO_OUT_RESOLUTION_960x1080 ? GREEN : RED, "960x1080");
			break;
	}
	cellDbgFontDraw();

	yPos += ySpacing;

	resources_get_int ("PS3Pal60", &res_int);
	cellDbgFontPuts   (0.09f, yPos, Emulator_GetFontSize(), currently_selected_setting == SETTING_PAL60_MODE ? YELLOW : WHITE,    "PAL60 Mode (576p only)");
	cellDbgFontPrintf (0.5f,  yPos, Emulator_GetFontSize(), res_int == true ? RED : GREEN, res_int == true ? "ON" : "OFF");

	yPos += ySpacing;
	cellDbgFontPuts(0.09f, yPos, Emulator_GetFontSize(), currently_selected_setting == SETTING_SHADER ? YELLOW : WHITE, "Selected shader");
	cellDbgFontPrintf(0.5f, yPos, Emulator_GetFontSize(), GREEN, "%s", Graphics->GetFragmentShaderPath().substr(Graphics->GetFragmentShaderPath().find_last_of('/')).c_str());

	yPos += ySpacing;
	resources_get_int ("PS3FontSize", &res_int);
	cellDbgFontPuts(0.09f,  yPos, Emulator_GetFontSize(), currently_selected_setting == SETTING_FONT_SIZE ? YELLOW : WHITE, "Font size");
	cellDbgFontPrintf(0.5f, yPos, Emulator_GetFontSize(), res_int == 100 ? GREEN : RED, "%f", Emulator_GetFontSize());

	yPos += ySpacing;
	resources_get_int ("PS3KeepAspect", &res_int);
	cellDbgFontPuts(0.09f, yPos, Emulator_GetFontSize(), currently_selected_setting == SETTING_KEEP_ASPECT_RATIO ? YELLOW : WHITE, "Aspect Ratio");
	cellDbgFontPrintf(0.5f, yPos, Emulator_GetFontSize(), (res_int == false) ? GREEN : RED, "%s", res_int == true ? "Scaled" : "Stretched");
	cellDbgFontDraw();

	yPos += ySpacing;
	resources_get_int ("PS3HardwareFilter", &res_int);
	cellDbgFontPuts(0.09f, yPos, Emulator_GetFontSize(), currently_selected_setting == SETTING_HW_TEXTURE_FILTER ? YELLOW : WHITE, "Hardware Filtering");
	cellDbgFontPrintf(0.5f, yPos, Emulator_GetFontSize(), res_int == false ? GREEN : RED, "%s", res_int == true ? "Linear interpolation" : "Point filtering");

	yPos += ySpacing;
	resources_get_int ("PS3OverscanAmount", &res_int);
	cellDbgFontPuts   (0.09f, yPos, Emulator_GetFontSize(), currently_selected_setting == SETTING_HW_OVERSCAN_AMOUNT ? YELLOW : WHITE, "Overscan");
	cellDbgFontPrintf (0.5f,  yPos, Emulator_GetFontSize(), res_int == 0 ? GREEN : RED, "%f", (float)res_int/100);


	/*
	   yPos += ySpacing;
	   cellDbgFontPuts(0.09f, yPos, Emulator_GetFontSize(), currently_selected_setting == SETTING_RSOUND_ENABLED ? YELLOW : WHITE, "Sound");
	   cellDbgFontPuts(0.5f, yPos, Emulator_GetFontSize(), Settings.RSoundEnabled == false ? GREEN : RED, Settings.RSoundEnabled == true ? "RSound" : "Normal");

	   yPos += ySpacing;
	   cellDbgFontPuts(0.09f, yPos, Emulator_GetFontSize(), currently_selected_setting == SETTING_RSOUND_SERVER_IP_ADDRESS ? YELLOW : WHITE, "RSound Server IP Address");
	   cellDbgFontPuts(0.5f, yPos, Emulator_GetFontSize(), strcmp(Settings.RSoundServerIPAddress,"0.0.0.0") ? RED : GREEN, Settings.RSoundServerIPAddress);
	 */

	yPos += ySpacing;
	yPos += ySpacing;
	cellDbgFontPuts  (0.09f, yPos, Emulator_GetFontSize(), currently_selected_setting == SETTING_SAVE_SETTINGS ? YELLOW : GREEN, "SAVE SETTINGS");
	cellDbgFontDraw();

	yPos += ySpacing;
	cellDbgFontPrintf(0.09f, yPos, Emulator_GetFontSize(), currently_selected_setting == SETTING_DEFAULT_ALL ? YELLOW : GREEN, "DEFAULT");
	cellDbgFontDraw();


	if (footer == NULL)
		footer = util_concat ("CIRCLE    - back      START        - return to ", machine_name, "   R1       - forward", NULL);

	cellDbgFontPuts(0.05f, 0.88f, Emulator_GetFontSize(), YELLOW,              "UP/DOWN   - select    X/LEFT/RIGHT - change          TRIANGLE - default");
	cellDbgFontPuts(0.05f, 0.92f, Emulator_GetFontSize(), YELLOW, footer);

	cellDbgFontDraw();
}

void do_ROMMenu()
{
	static const char *footer = NULL;
	char rom_path[MAX_PATH_LENGTH];
	char newpath[MAX_PATH_LENGTH];
	int res_int;

	if (CellInput->UpdateDevice(0) == CELL_PAD_OK)
	{
		UpdateBrowser(browser);

		if (CellInput->WasButtonPressed(0,CTRL_SELECT))
		{
			menuStack.push(do_general_settings);
		}

		if (CellInput->WasButtonPressed(0,CTRL_CROSS))
		{
			if(browser->IsCurrentADirectory())
			{
				//if 'filename' is in fact '..' - then pop back directory instead of adding '..' to filename path
				const char * separatorslash = (strcmp(browser->get_current_directory_name(),"/") == 0) ? "" : "/";
				snprintf(newpath, sizeof(newpath), "%s%s%s", browser->get_current_directory_name(), separatorslash, browser->get_current_filename());
				browser->PushDirectory(newpath, CELL_FS_TYPE_REGULAR | CELL_FS_TYPE_DIRECTORY, ROM_EXTENSIONS);
			}
			else if (browser->IsCurrentAFile())
			{
				// load game (standard controls), go back to main loop
				snprintf(rom_path, sizeof(rom_path), "%s/%s", browser->get_current_directory_name(), browser->get_current_filename());

				MenuStop();

				// switch emulator to emulate mode
				Emulator_StartROMRunning();


				Emulator_RequestLoadROM(rom_path, true, false);

				return;
			}
		}
		if (CellInput->WasButtonPressed(0,CTRL_SQUARE))
		{
			if (browser->IsCurrentAFile())
			{
				// Load the disk image.
				// Reset the emulator with FastLoad disabled

				snprintf(rom_path, sizeof(rom_path), "%s/%s", browser->get_current_directory_name(), browser->get_current_filename());
				MenuStop();

				// switch emulator to emulate mode
				Emulator_StartROMRunning();
				Emulator_RequestLoadROM(rom_path, true, true);
				return;
			}
		}
		/*
		   if (CellInput->WasButtonPressed(0,CTRL_TRIANGLE) )
		   {
		   if (browser->IsCurrentAFile())
		   {
		// Load without any disk image. Do "not" reset the machine
		rom_path = browser->GetCurrentDirectoryInfo().dir + "/" + browser->GetCurrentEntry()->d_name;
		MenuStop();

		// switch emulator to emulate mode
		Emulator_StartROMRunning();
		Emulator_RequestLoadROM((char*)rom_path, false, true);
		return;
		}
		}
		 */

		if (CellInput->IsButtonPressed(0,CTRL_TRIANGLE))
		{ 
			if (browser->IsCurrentAFile())
			{
				int drive_count=0;
				resources_get_int("Drive8Type", &res_int);
				drive_count+= (res_int == DRIVE_TYPE_NONE) ? 0:1;
				resources_get_int("Drive9Type", &res_int);
				drive_count+= (res_int == DRIVE_TYPE_NONE) ? 0:1;
				resources_get_int("Drive10Type", &res_int);
				drive_count+= (res_int == DRIVE_TYPE_NONE) ? 0:1;
				resources_get_int("Drive11Type", &res_int);
				drive_count+= (res_int == DRIVE_TYPE_NONE) ? 0:1;

				if ( (drive_count == 1) || (CellInput->WasButtonPressed(0,CTRL_L1)) )
				{
					// Insert the disk into Drive8
					snprintf(rom_path, sizeof(rom_path), "%s/%s", browser->get_current_directory_name(), browser->get_current_filename());
					if (file_system_attach_disk(8, rom_path) < 0)
					{
#ifdef CELL_DEBUG
						printf("could not attach image to device 8 : %s\n", rom_path);
#endif
					}
#ifdef CELL_DEBUG
					printf(LOG_DEFAULT, "Attached disk image to device %d\n", 8);
#endif
				} 
				else if (CellInput->WasButtonPressed(0,CTRL_L2))
				{
					// Insert the disk into Drive9
					snprintf(rom_path, sizeof(rom_path), "%s/%s", browser->get_current_directory_name(), browser->get_current_filename());
					if (file_system_attach_disk(9, rom_path) < 0)
					{
#ifdef CELL_DEBUG
						printf("could not attach image to device 9 : %s\n", rom_path);
#endif
					}
#ifdef CELL_DEBUG
					printf("Attached disk image to device %d\n", 9);
#endif
				}
				else if (CellInput->WasButtonPressed(0,CTRL_L2))
				{
					// Insert the disk into Drive10
					snprintf(rom_path, sizeof(rom_path), "%s/%s", browser->get_current_directory_name(), browser->get_current_filename());
					if (file_system_attach_disk(10, rom_path) < 0)
					{
#ifdef CELL_DEBUG
						printf("could not attach image to device 10 : %s\n", rom_path);
#endif
					}
#ifdef CELL_DEBUG
					printf("Attached disk image to device %d\n", 10);
#endif
				}
				else if (CellInput->WasButtonPressed(0,CTRL_L2))
				{
					// Insert the disk into Drive11
					snprintf(rom_path, sizeof(rom_path), "%s/%s", browser->get_current_directory_name(), browser->get_current_filename());
					if (file_system_attach_disk(11, rom_path) < 0)
					{
#ifdef CELL_DEBUG
						printf("could not attach image to device 11 : %s\n", rom_path);
#endif
					}
#ifdef CELL_DEBUG
					printf("Attached disk image to device %d\n", 11);
#endif
				}

			}
		}

		if (CellInput->WasButtonPressed(0,CTRL_START))
		{
			// Don't load any new rom. Go back to the emulator as is
			MenuStop();
			Emulator_StartROMRunning();
			return;
		}
	}


	const char *filenameptr=NULL;
	const char *filepathptr=NULL;

	float yPos=0.000f;
	float ybrk=0.035f;

	resources_get_int("Drive8Type", &res_int);
	if (res_int != DRIVE_TYPE_NONE)
	{
		yPos+=ybrk;
		cellDbgFontPuts(0.65f, yPos, Emulator_GetFontSize(), PURPLE, "Drive 8");
		filepathptr = file_system_get_disk_name(8);
		if (filepathptr != NULL) {
			filenameptr = strrchr(filepathptr, '/');
			if (filenameptr != NULL)
				cellDbgFontPrintf(0.755f, yPos, Emulator_GetFontSize(), CYAN, "%s", ++filenameptr);
			else 
				cellDbgFontPrintf(0.755f, yPos, Emulator_GetFontSize(), CYAN, "%s", filepathptr);
		}
	}
	resources_get_int("Drive9Type", &res_int);
	if (res_int != DRIVE_TYPE_NONE)
	{
		yPos+=ybrk;
		cellDbgFontPuts(0.65f, yPos, Emulator_GetFontSize(), PURPLE,  "Drive 9");
		filepathptr = file_system_get_disk_name(9);
		if (filepathptr != NULL) {
			filenameptr = strrchr(filepathptr, '/');
			if (filenameptr != NULL)
				cellDbgFontPrintf(0.755f, yPos, Emulator_GetFontSize(),  CYAN, "%s", ++filenameptr);
			else
				cellDbgFontPrintf(0.755f, yPos, Emulator_GetFontSize(),  CYAN, "%s", filepathptr);
		}
	}
	resources_get_int("Drive10Type", &res_int);
	if (res_int != DRIVE_TYPE_NONE) {
		yPos+=ybrk;
		cellDbgFontPuts(0.65f, yPos, Emulator_GetFontSize(), PURPLE, "Drive 10");
		filepathptr = file_system_get_disk_name(10);
		if (filepathptr != NULL) {
			filenameptr = strrchr(filepathptr, '/');
			if (filenameptr != NULL)
				cellDbgFontPrintf(0.755f, yPos, Emulator_GetFontSize(),  CYAN, "%s", ++filenameptr);
			else
				cellDbgFontPrintf(0.755f, yPos, Emulator_GetFontSize(),  CYAN, "%s", filepathptr);
		}
	}
	resources_get_int("Drive11Type", &res_int);
	if (res_int != DRIVE_TYPE_NONE) {
		yPos+=ybrk;
		cellDbgFontPuts(0.65f, yPos, Emulator_GetFontSize(), PURPLE,  "Drive 11");
		filepathptr = file_system_get_disk_name(11);
		if (filepathptr != NULL) {
			filenameptr = strrchr(filepathptr, '/');
			if (filenameptr != NULL)
				cellDbgFontPrintf(0.755f, yPos, Emulator_GetFontSize(),  CYAN, "%s", ++filenameptr);
			else
				cellDbgFontPrintf(0.755f, yPos, Emulator_GetFontSize(),  CYAN, "%s", filepathptr);
		}
	}
	yPos+=ybrk;
	cellDbgFontPuts(0.65f, yPos, Emulator_GetFontSize(), PURPLE,  "Tape");
	filepathptr = tape_get_file_name();
	if (filepathptr != NULL) {
		filenameptr = strrchr(filepathptr, '/');
		if (filenameptr != NULL)
			cellDbgFontPrintf(0.755f, yPos, Emulator_GetFontSize(),  CYAN, "%s", ++filenameptr);
		else
			cellDbgFontPrintf(0.755f, yPos, Emulator_GetFontSize(),  CYAN, "%s", filepathptr);
	}
	cellDbgFontDraw();



	if (footer == NULL)
		footer = util_concat ("START  - Return to ", machine_name, NULL);

	cellDbgFontPuts(0.09f, 0.88f, Emulator_GetFontSize(), BLUE,                "SELECT - settings screen");
	cellDbgFontPuts(0.09f, 0.92f, Emulator_GetFontSize(), PURPLE, footer);
	cellDbgFontDraw();

	cellDbgFontPuts(0.44f, 0.84f, Emulator_GetFontSize(), GREEN,  " X - Reset with image (FastLoad)");
	cellDbgFontPuts(0.44f, 0.88f, Emulator_GetFontSize(), YELLOW, "[] - Reset with image (SlowLoad + TDE)");
	cellDbgFontPuts(0.44f, 0.92f, Emulator_GetFontSize(), CYAN ,  "/\\ + L1/L2/R1/R2 - Attach to D8/9/10/11");
	cellDbgFontDraw();

	RenderBrowser(browser);
}


void do_OSKMenu()
{
	static const char *footer = NULL;
	if (CellInput->UpdateDevice(0) == CELL_PAD_OK)
	{
		if (CellInput->WasButtonPressed(0,CTRL_DOWN) | (CellInput->IsAnalogPressedDownPercentage(0,CTRL_LSTICK) > 0.10))
		{
			currently_selected_osk_entry++;
			if (currently_selected_osk_entry > MAX_KEY_INDEX)
				currently_selected_osk_entry = MIN_KEY_INDEX;
		}

		if (CellInput->WasButtonPressed(0,CTRL_UP) | (CellInput->IsAnalogPressedUpPercentage(0,CTRL_LSTICK) > 0.10) )
		{
			currently_selected_osk_entry--;
			if (currently_selected_osk_entry < MIN_KEY_INDEX)
				currently_selected_osk_entry = MAX_KEY_INDEX;
		}

		if (CellInput->WasButtonPressed(0,CTRL_START) | CellInput->WasButtonPressed(0,CTRL_CIRCLE) )
		{
			// Don't do anything, go back to the emulator as is
			menuStack.pop();
			MenuStop();
			Emulator_StartROMRunning();
			return;
		}

		if (CellInput->WasButtonPressed(0,CTRL_CROSS))
		{
			osk_kbd_append_buffer_char (keymap[currently_selected_osk_entry].keycode);

			menuStack.pop();
			MenuStop();
			// switch emulator to emulate mode
			Emulator_StartROMRunning();
			return;
		}
	}

	float yPos=0.05f;
	float ybrk=0.04f;

	yPos+=ybrk;

	for (int i=MIN_KEY_INDEX; i <= MAX_KEY_INDEX; i++)
	{
		yPos+=ybrk;
		cellDbgFontPuts (0.09f, yPos, Emulator_GetFontSize(), currently_selected_osk_entry == i ? YELLOW : WHITE, keymap[i].keyname);
	}
	cellDbgFontDraw();


	if (footer == NULL)
		footer = util_concat("START/CIRCLE  - return to ", machine_name, NULL);

	cellDbgFontPuts(0.05f, 0.88f, Emulator_GetFontSize(), YELLOW, "UP/DOWN       - navigate           X - select");
	cellDbgFontPuts(0.05f, 0.92f, Emulator_GetFontSize(), YELLOW, footer);
	cellDbgFontDraw();
}

void MenuMainLoop(void)
{
	const char *res_string;

	// create file browser->if null
	if (browser == NULL)
	{
		resources_get_string ("PS3PathRomDir", &res_string);
		browser = new FileBrowser(res_string, ROM_EXTENSIONS);
		browser->SetEntryWrap(false);
	}


	if (mode_switch == MODE_OSK)
		menuStack.push(do_OSKMenu);
	else
	{

		if (menuStack.empty())
		{
			menuStack.push(do_ROMMenu);
		}
	}

	// menu loop
	menuRunning = true;
	while (!menuStack.empty() && menuRunning)
	{
		glClear(GL_COLOR_BUFFER_BIT);

		menuStack.top()();

		psglSwap();

		cellSysutilCheckCallback();

		if (mode_switch == MODE_EXIT)
		{
			// Emulator_Shutdown will be called by our atexit handler
			exit(0);
		}
	}
}
