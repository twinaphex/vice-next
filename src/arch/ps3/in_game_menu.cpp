/******************************************************************************* 
 *  -- in_game_menu.cpp - In-Game Menu interface for Vice Playstation 3
 *
 *     VICE PS3 -   Commodore 64 emulator for the Playstation 3
 *                  ported from the original VICE distribution
 *                  located at http://sourceforge.net/projects/vice-emu/
 *
 *
 *  Copyright (C) 2010
 *  Created on: Dec 23, 2010
 *  Author:      TimRex
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


#include <stack>
#include <vector>
#include <cell/pad.h>
#include <cell/audio.h>
#include <cell/sysmodule.h>
#include <cell/cell_fs.h>
#include <cell/dbgfont.h>
#include <sysutil/sysutil_msgdialog.h>

#include "cellframework/input/cellInput.h"

#include "emu-ps3.hpp"
#include "joy.h"
#include "osk.h"
#include "kbd.h"

extern "C" {
#include "machine.h"
}

#include "in_game_menu.h"
#include "ui_snapshot.h"

#define OPTION_SWAP_JOYSTICKS 0
#define OPTION_HARD_RESET     1
#define OPTION_LOAD_SNAPSHOT  2
#define OPTION_SAVE_SNAPSHOT  3
#define OPTION_OSK_C64        4
#define OPTION_OSK_NATIVE     5
#define OPTION_CANCEL         6
#define MAX_NO_OF_IN_GAME_OPTIONS 7
 
const char *in_game_option_text[] = {
    "Swap Joysticks",
    "Issue Hard Reset",
    "Load Snapshot",
    "Save Snapshot",
    "On-Screen-Keyboard (C64 keys)",
    "On-Screen-Keyboard (Native)",
    "Back To Emulator" 
};


// is the menu running
bool ingame_menu_running = false;


static void callback_dialog_yes_no ( int button_type, void *userdata )
{
	switch ( button_type )
	{
		case CELL_MSGDIALOG_BUTTON_YES:
			switch ( (int) userdata )
			{
				case OPTION_LOAD_SNAPSHOT:
					load_snapshot(get_current_rom());
					ingame_menu_running=false;
					break;

				case OPTION_SAVE_SNAPSHOT:
					save_snapshot(get_current_rom());
					ingame_menu_running=false;
					break;

			}

		case CELL_MSGDIALOG_BUTTON_NO:
		case CELL_MSGDIALOG_BUTTON_ESCAPE:
		case CELL_MSGDIALOG_BUTTON_NONE:
			break;
	}
}

void DialogYesNo (const char *message, int selection)
{
	static int current_selection;

	unsigned int type =   CELL_MSGDIALOG_TYPE_SE_TYPE_NORMAL
		| CELL_MSGDIALOG_TYPE_BG_INVISIBLE
		| CELL_MSGDIALOG_TYPE_BUTTON_TYPE_YESNO
		| CELL_MSGDIALOG_TYPE_DISABLE_CANCEL_ON
		| CELL_MSGDIALOG_TYPE_DEFAULT_CURSOR_NO;

	current_selection = selection;

	int ret = cellMsgDialogOpen2 (type, message, callback_dialog_yes_no, (void*) current_selection, NULL );
	if ( ret != CELL_OK ) {
		if ( ret == (int)CELL_SYSUTIL_ERROR_BUSY )
		{
			#ifdef CELL_DEBUG
			printf("WARN  : cellMsgDialogOpen2() = 0x%x (CELL_SYSUTIL_ERROR_BUSY) ... Retry.\n", ret);
			#endif
			return;
		}
		return;
	}
}

int16_t currently_selected_option = 0;
int16_t last_selected_option = -1;
bool snapshot_avail = false;
const char *romfile = NULL;
const char *rompath;

void do_ingame_menu()
{
	if(CellInput->UpdateDevice(0) == CELL_OK)
	{
		// back to ROM menu if CIRCLE is pressed
		if (CellInput->WasButtonPressed(0, CTRL_CIRCLE) | CellInput->WasButtonPressed(0, CTRL_SELECT) | CellInput->WasButtonPressed(0, CTRL_START))
		{
			ingame_menu_running=false;
			return;
		}

		if (CellInput->WasButtonPressed(0, CTRL_DOWN) | (CellInput->IsAnalogPressedDownPercentage(0, CTRL_LSTICK) > 0.10))    // down to next setting
		{
			currently_selected_option++;

			// The order of these is important

			if (currently_selected_option == OPTION_LOAD_SNAPSHOT) {
				// If there is no snapshot, this option is disabled. SKip it.
				if ( (rompath == NULL) || (!snapshot_avail) )
					currently_selected_option++;
			}
			if (currently_selected_option == OPTION_SAVE_SNAPSHOT) {
				// If there is no currently loaded rom image, this option is disabled. SKip it.
				if (rompath == NULL)
					currently_selected_option++;
			}

			if (currently_selected_option >= MAX_NO_OF_IN_GAME_OPTIONS)
			{
				currently_selected_option = 0;
			}
		}

		if (CellInput->WasButtonPressed(0, CTRL_UP) | (CellInput->IsAnalogPressedUpPercentage(0, CTRL_LSTICK) > 0.10))    // up to previous setting
		{
			currently_selected_option--;

			// The order of these is important

			if (currently_selected_option == OPTION_SAVE_SNAPSHOT)
			{
				// If there is no currently loaded rom image, this option is disabled. SKip it.
				if (rompath == NULL)
					currently_selected_option--;
			}
			if (currently_selected_option == OPTION_LOAD_SNAPSHOT)
			{
				// If there is no snapshot, this option is disabled. SKip it.
				if ( (rompath == NULL) || (!snapshot_avail) )
					currently_selected_option--;
			}

			if (currently_selected_option < 0)
			{
				currently_selected_option = MAX_NO_OF_IN_GAME_OPTIONS-1;
			}
		}

		if (CellInput->WasButtonPressed(0, CTRL_CROSS))
		{
			switch(currently_selected_option)
			{
				case OPTION_SWAP_JOYSTICKS:
					joyswap=!joyswap;
					ingame_menu_running=false;
					return;
					break;
				case OPTION_HARD_RESET:
					machine_trigger_reset(MACHINE_RESET_MODE_HARD);
					ingame_menu_running=false;
					return;
					break;
				case OPTION_LOAD_SNAPSHOT:
					DialogYesNo ("The current state will be lost. Are you sure?", OPTION_LOAD_SNAPSHOT);
					break;

				case OPTION_SAVE_SNAPSHOT:
					if (snapshot_avail)
					{
						DialogYesNo ("An existing snapshot will be overwritten\nAre you sure?", OPTION_SAVE_SNAPSHOT);
						break;
					}

					save_snapshot(rompath);
					ingame_menu_running=false;
					return;
				case OPTION_OSK_C64:
					// TODO Add C64 OSK here.  Need to refactor the OSK menu so be more like this one
					break;
				case OPTION_OSK_NATIVE:
					if (!osk->Start(L"Characters entered here will be relayed to the emulator ", L""))
					{
						#ifdef CELL_DEBUG
						printf("WARNING: OSK could not start\n");
						#endif
					}
					ingame_menu_running=false;
					return;
					break;
				case OPTION_CANCEL:
					ingame_menu_running=false;
					return;
					break;
			}
		}
	}

	float yPos = 0.13;
	float ySpacing = 0.04;


	for (int i=0; i < MAX_NO_OF_IN_GAME_OPTIONS; i++)
	{
		yPos += ySpacing;

		switch (i)
		{
			case OPTION_LOAD_SNAPSHOT:
				if (snapshot_avail)
				{
					cellDbgFontPuts (0.09f, yPos, Emulator_GetFontSize(), (currently_selected_option == i ? YELLOW : WHITE), in_game_option_text[i]);
					cellDbgFontPuts (0.5f, yPos, Emulator_GetFontSize(),  GRAY, romfile);
				}
				else
				{
					cellDbgFontPuts (0.09f, yPos, Emulator_GetFontSize(), GRAY, in_game_option_text[i]);
					cellDbgFontPuts (0.5f, yPos, Emulator_GetFontSize(),  GRAY, "No snapshot available");
				}
				break;

			case OPTION_SAVE_SNAPSHOT:
				cellDbgFontPuts (0.09f, yPos, Emulator_GetFontSize(), (rompath == NULL) ? GRAY : ( (currently_selected_option == i ? YELLOW : WHITE) ), in_game_option_text[i]);

				if (snapshot_avail)
				{
					cellDbgFontPrintf (0.5f, yPos, Emulator_GetFontSize(), GRAY, "Snapshot exists for %s", romfile);
				}
				break;

			default:
				cellDbgFontPuts (0.09f, yPos, Emulator_GetFontSize(), currently_selected_option == i ? YELLOW : WHITE, in_game_option_text[i]);
				break;
		}

	}
	cellDbgFontDraw();
}


void InGameMenuLoop(void)
{
	// menu loop
	ingame_menu_running = true;

	rompath = get_current_rom();
	if (rompath != NULL)
	{
		romfile = strrchr (rompath, '/');
		if (romfile == NULL)
			romfile=rompath;
		else
			romfile++;
	}
	snapshot_avail = snapshot_exists(rompath);

	while (ingame_menu_running)
	{
		// TODO Only clear a portion of the screen.. OR simply call "force-refresh" and
		// draw directly over the top...  ?

		glClear(GL_COLOR_BUFFER_BIT);

		do_ingame_menu();
		psglSwap();

		cellSysutilCheckCallback();

		if (mode_switch == MODE_EXIT)
		{
			// Emulator_Shutdown will be called by our atexit handler
			exit(0);
		}
	}
}

