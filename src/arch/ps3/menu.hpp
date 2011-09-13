/******************************************************************************* 
 *  -- menu.h - Menu interface for Vice Playstation 3
 *
 *     VICE PS3 -   Commodore 64 emulator for the Playstation 3
 *                  ported from the original VICE distribution
 *                  located at http://sourceforge.net/projects/vice-emu/
 *
 *
 *  Copyright (C) 2010
 *  Created on: Oct 10, 2010
 *      Author:     halsafar
 *      Updated by  TimRex
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


#ifndef MENU_H_
#define MENU_H_

#include "colors.h"
#include "drive.h"

#define MAX_PATH 1024

// if you add more settings to the screen, remember to change this value to the correct number
// PATH - Total amount of Path settings
#define MAX_NO_OF_SETTINGS      9
#define MAX_NO_OF_PATH_SETTINGS 3
#define MAX_NO_OF_VICE_SETTINGS 18


//GENERAL  - setting constants
#define SETTING_CHANGE_RESOLUTION 0
#define SETTING_PAL60_MODE 1
#define SETTING_SHADER 2
#define SETTING_FONT_SIZE 3
#define SETTING_KEEP_ASPECT_RATIO 4
#define SETTING_HW_TEXTURE_FILTER 5
#define SETTING_HW_OVERSCAN_AMOUNT 6
//#define SETTING_RSOUND_ENABLED 7
//#define SETTING_RSOUND_SERVER_IP_ADDRESS 8
#define SETTING_SAVE_SETTINGS 7
#define SETTING_DEFAULT_ALL 8

// PATH
#define SETTING_PATH_DEFAULT_ROM_DIRECTORY 0
#define SETTING_PATH_SAVE_SETTINGS 1
#define SETTING_PATH_DEFAULT_ALL 2

// CONTROLLER
#define SETTING_CONTROLLER_1_CROSS        0
#define SETTING_CONTROLLER_1_SQUARE       1
#define SETTING_CONTROLLER_1_TRIANGLE     2
#define SETTING_CONTROLLER_1_CIRCLE       3
#define SETTING_CONTROLLER_1_L1           4
#define SETTING_CONTROLLER_1_L2           5
#define SETTING_CONTROLLER_1_R1           6
#define SETTING_CONTROLLER_1_R2           7
#define SETTING_CONTROLLER_2_CROSS        8
#define SETTING_CONTROLLER_2_SQUARE       9
#define SETTING_CONTROLLER_2_TRIANGLE     10
#define SETTING_CONTROLLER_2_CIRCLE       11
#define SETTING_CONTROLLER_2_L1           12
#define SETTING_CONTROLLER_2_L2           13
#define SETTING_CONTROLLER_2_R1           14
#define SETTING_CONTROLLER_2_R2           15
#define SETTING_CONTROLLER_SAVE_SETTINGS  16
#define SETTING_CONTROLLER_DEFAULT_ALL    17
#define MAX_NO_OF_CONTROLLER_SETTINGS     18


// VICE
#define SETTING_VICE_DISPLAY_FRAMERATE 0        // bool
#define SETTING_VICE_DISPLAY_DRIVE_INDICATORS 1 // bool
#define SETTING_VICE_SID_FILTERS 2              // bool
#define SETTING_VICE_SID_ENGINE 3               // enum  0, 1, 7  - SID_ENGINE_FASTSID, SID_ENGINE_RESID, SID_ENGINE_RESID_FP
#define SETTING_VICE_SID_RESID_SAMPLING 4       // enum  0, 1, 2  /  0 - Fast, 1 - Interpolate, 2 - Resampling
#define SETTING_VICE_SID_MODEL 5                // enum
#define SETTING_VICE_SID_RESID_PASSBAND 6       //  range 20-90
#define SETTING_VICE_MOUSE_SUPPORT 7    
#define SETTING_VICE_MOUSE_TYPE 8   
#define SETTING_VICE_SWAP_JOYSTICKS 9   
#define SETTING_VICE_KEYMAP 10   
#define SETTING_VICE_DRIVE8_TYPE 11
#define SETTING_VICE_DRIVE9_TYPE 12
#define SETTING_VICE_DRIVE10_TYPE 13
#define SETTING_VICE_DRIVE11_TYPE 14

#define SETTING_VICE_HARD_RESET 15
#define SETTING_VICE_SAVE_SETTINGS 16
#define SETTING_VICE_DEFAULT_ALL 17

// OSK defines

// ReSID defines


enum { VICE_RESID_FASTSID, VICE_RESID_INTERPOLATION, VICE_RESID_RESAMP};


#define MIN_ALL_SID_MODELS 0
#define MAX_ALL_SID_MODELS 14

#define MIN_SID_MODELS 0
#define MAX_SID_MODELS 1
#define MIN_RESID_MODELS 2
#define MAX_RESID_MODELS 4
#define MIN_RESID_FP_MODELS 5
#define MAX_RESID_FP_MODELS 14

static const int   ui_sid_engine_model_id[] = {
    SID_FASTSID_6581,
    SID_FASTSID_8580,
    SID_RESID_6581,
    SID_RESID_8580,
    SID_RESID_8580D,
    SID_RESIDFP_6581R3_4885,
    SID_RESIDFP_6581R3_0486S, 
    SID_RESIDFP_6581R3_3984,
    SID_RESIDFP_6581R4AR_3789,
    SID_RESIDFP_6581R3_4485,
    SID_RESIDFP_6581R4_1986S,  
    SID_RESIDFP_8580R5_3691,
    SID_RESIDFP_8580R5_3691D,  
    SID_RESIDFP_8580R5_1489,
    SID_RESIDFP_8580R5_1489D  
};

static const char *ui_sid_engine_model[] = {
    ("FastSID   6581"),
    ("FastSID   8580"),
    ("ReSID     6581"),
    ("ReSID     8580"),
    ("ReSID     8580 + digi boost"),
    ("ReSID-FP  6581R3 4885"),
    ("ReSID-FP  6581R3 0486S"),
    ("ReSID-FP  6581R3 3984"),
    ("ReSID-FP  6581R4AR 3789"),
    ("ReSID-FP  6581R3 4485"),
    ("ReSID-FP  6581R4 1986S"),
    ("ReSID-FP  8580R5 3691"),
    ("ReSID-FP  8580R5 3691 + digi boost"),
    ("ReSID-FP  8580R5 1489"),
    ("ReSID-FP  8580R5 1489 + digi boost"),
    NULL
};




#ifdef EMU_C64
#define MAX_DRIVE_TYPES 5
static struct _drive_type {
    const char *name;
    int id;
} drive_type[] = {
    { "None", DRIVE_TYPE_NONE },
    { "1541", DRIVE_TYPE_1541 },
    { "1541-II", DRIVE_TYPE_1541II },
    { "1570", DRIVE_TYPE_1570 },
    { "1571", DRIVE_TYPE_1571 },
    { "1581", DRIVE_TYPE_1581 },
    { NULL, 0 }
};
#endif
#ifdef EMU_C128
#define MAX_DRIVE_TYPES 13
static struct _drive_type {
    const char *name;
    int id;
} drive_type[] = {
    { "None", DRIVE_TYPE_NONE },
    { "1541", DRIVE_TYPE_1541 },
    { "1541-II", DRIVE_TYPE_1541II },
    { "1570", DRIVE_TYPE_1570 },
    { "1571", DRIVE_TYPE_1571 },
    { "1571CR", DRIVE_TYPE_1571CR },
    { "1581", DRIVE_TYPE_1581 },
    { "2031", DRIVE_TYPE_2031 },
    { "2040", DRIVE_TYPE_2040 },
    { "3040", DRIVE_TYPE_3040 },
    { "4040", DRIVE_TYPE_4040 },
    { "1001", DRIVE_TYPE_1001 },
    { "8050", DRIVE_TYPE_8050 },
    { "8250", DRIVE_TYPE_8250 },
    { NULL, 0 }
};
#endif
#ifdef EMU_VIC20
#define MAX_DRIVE_TYPES 6
static struct _drive_type {
    const char *name;
    int id;
} drive_type[] = {
    { "None", DRIVE_TYPE_NONE },
    { "1541", DRIVE_TYPE_1541 },
    { "1541-II", DRIVE_TYPE_1541II },
    { "1570", DRIVE_TYPE_1570 },
    { "1571", DRIVE_TYPE_1571 },
    { "1581", DRIVE_TYPE_1581 },
    { NULL, 0 }
};
#endif
#ifdef EMU_PLUS4
#define MAX_DRIVE_TYPES 6
static struct _drive_type {
    const char *name;
    int id;
} drive_type[] = {
    { "None", DRIVE_TYPE_NONE },
    { "1541", DRIVE_TYPE_1541 },
    { "1541-II", DRIVE_TYPE_1541II },
    { "1551", DRIVE_TYPE_1551 },
    { "1570", DRIVE_TYPE_1570 },
    { "1571", DRIVE_TYPE_1571 },
    { "1581", DRIVE_TYPE_1581 },
    { NULL, 0 }
};
#endif

void MenuMainLoop(void);

void MenuStop();
bool MenuIsRunning();
char* MenuGetSelectedROM();
char* MenuGetCurrentPath();

char* do_pathmenu(uint16_t is_for_shader_or_dir_selection, const char * pathname = "/");
#endif /* MENU_H_ */
