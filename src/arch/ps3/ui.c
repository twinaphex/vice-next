/******************************************************************************* 
 *  -- ui.h -       Basic debug functions for Playstation 3, used to genearte
 *                  HUD syle overlay of status info
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
#include "ui.h"
#include "types.h"
#include <stdio.h>
#include "resources.h"
#include "uiapi.h"

struct hid ui;

static int set_display_drive_indicators(int val, void *param);
static int set_display_framerate(int val, void *param);
static int ps3_set_pal60 (int val, void *param);
static int ps3_set_font_size (int val, void *param);
static int ps3_set_aspect (int val, void *param);
static int ps3_set_hw_filter (int val, void *param);
static int ps3_set_overscan (int val, void *param);
static int ps3_set_overscan_amt (int val, void *param);
static int ps3_set_shader(const char *val, void *param);
static int ps3_set_rom_dir(const char *val, void *param);


static int *drive_active_led;

static int ps3_resolution;
static int ps3_pal60;
static int ps3_font_size;
static int ps3_aspect;
static int ps3_hw_filter;
static int ps3_overscan;
static int ps3_overscan_amt;

static char *ps3_shader = NULL; 
static char *ps3_path_rom_dir = NULL; 


static const resource_int_t resources_int[] = {
    { "DisplayDriveIndicators", 0, RES_EVENT_NO, NULL,
      &ui.display_drives, set_display_drive_indicators, NULL },
    { "DisplayFrameRate", 0, RES_EVENT_NO, NULL,
      &ui.display_speed, set_display_framerate, NULL },
    { "PS3Pal60", 0, RES_EVENT_NO, NULL,
      &ps3_pal60, ps3_set_pal60, NULL },
    { "PS3FontSize", 100, RES_EVENT_NO, NULL,
      &ps3_font_size, ps3_set_font_size, NULL },
    { "PS3KeepAspect", 0, RES_EVENT_NO, NULL,
      &ps3_aspect, ps3_set_aspect, NULL },
    { "PS3HardwareFilter", 0, RES_EVENT_NO, NULL,
      &ps3_hw_filter, ps3_set_hw_filter, NULL },
    { "PS3Overscan", 0, RES_EVENT_NO, NULL,
      &ps3_overscan, ps3_set_overscan, NULL },
    { "PS3OverscanAmount", 0, RES_EVENT_NO, NULL,
      &ps3_overscan_amt, ps3_set_overscan_amt, NULL },
    { NULL }
};


static const resource_string_t resources_string[] = {
    { "PS3Shader", "", RES_EVENT_NO, NULL,
      &ps3_shader, ps3_set_shader, NULL },
    { "PS3PathRomDir", "/", RES_EVENT_NO, NULL,
      &ps3_path_rom_dir, ps3_set_rom_dir, NULL },
    { NULL }
};


static int set_display_drive_indicators(int val, void *param)
{
    ui.display_drives=val;
    return 0;
}

static int set_display_framerate(int val, void *param)
{
    ui.display_speed=val;
    return 0;
}

static int ps3_set_pal60 (int val, void *param)
{
    ps3_pal60 = val;
    return 0;
}

static int ps3_set_font_size (int val, void *param)
{
    ps3_font_size = val;
    return 0;
}

static int ps3_set_aspect (int val, void *param)
{
	ps3_aspect = val;
	return 0;
}

static int ps3_set_hw_filter (int val, void *param)
{
	ps3_hw_filter = val;
	return 0;
}

static int ps3_set_overscan (int val, void *param)
{
	ps3_overscan = val;
	return 0;
}

static int ps3_set_overscan_amt (int val, void *param)
{
	ps3_overscan_amt = val;
	return 0;
}

static int ps3_set_shader(const char *val, void *param)
{
	util_string_set(&ps3_shader, val);
	return 0;
}

static int ps3_set_rom_dir(const char *val, void *param)
{
	util_string_set(&ps3_path_rom_dir, val);
	return 0;
}

int ui_callback(void)
{
	static int one_more_redraw=0;

	if (Emulator_GetMode() == MODE_EXIT) {
		// handled by our atexit handler
		//Emulator_Shutdown();
		exit(0);
	}


	// The callback only works correclty if the screen continuously updates.
	// So we need to be sure the screen is updating, since the emulators vsync only occurs if
	// something needs to be updated on-screen

	// Maybe we can fix this on the emulator side, but for now.. Let's check when the last redraw occurred
	// And force a redraw if it's been more than, however long...

	cellSysutilCheckCallback();

	if (is_sysutil_drawing())
	{
		one_more_redraw=15; // timing dependent on the frequency of this callback
		// 5 wasn't quite enough. 8 nearly enough.

		// We need to force a redraw, if we haven't seen one for a while
		// This ensures the sysutil_callback draws regularly enough,
		// since the C64 implementation only draws if it needs to.
		sysutil_callback_redraw();
	}
	else if (one_more_redraw)
	{
		// Force one more redraw, after the XMB has left.
		// Ensures the screen is no longer dimmed.  
		one_more_redraw--;
		force_redraw();
	}



	return 0;
}


void ui_error(const char *format,...)
{
	char *tmp;
	va_list args;

	va_start(args, format);
	debug_printf (format, args);
	va_end(args);
}

void ui_display_drive_current_image(unsigned int drive_number, const char *image)
{
}

void ui_update_menus(void)
{
    /* needed */
}

void ui_display_tape_current_image(const char *image)
{
    /* needed */
}

void ui_display_tape_counter(int counter)
{
    ui.tape.counter=counter;
    force_redraw();
}

void ui_display_tape_motor_status(int motor)
{
    ui.tape.motor=motor;
    force_redraw();
}

void ui_display_tape_control_status(int control)
{
    ui.tape.control=control;
    force_redraw();
}

void ui_set_tape_status(int tape_status)
{
    ui.tape.status=tape_status;
    force_redraw();
}

void ui_display_recording(int recording_status)
{
    /* needed */
}

void ui_display_playback(int playback_status, char *version)
{
    /* needed */
}

int ui_init(int *argc, char **argv)
{
    resources_load(NULL);
    resources_get_int("DisplayFrameRate", &ui.display_speed);
    resources_get_int("DisplayDriveIndicators", &ui.display_speed);
    return 0;
}



void archdep_ui_init(int argc, char *argv[])
{
    return;
}

int ui_init_finish(void)
{
    return 0;
}


void ui_enable_drive_status(ui_drive_enable_t enable, int *drive_led_color)
{
    drive_active_led = drive_led_color;
}

int ui_extend_image_dialog(void)
{
    /* needed */
}

void ui_display_drive_led(int drive_number, unsigned int led_pwm1, unsigned int led_pwm2)
{
	switch (drive_number)
	{
		case 0:
			ui.drive8.led_pwm1 = led_pwm1;
			ui.drive8.led_pwm2 = led_pwm2;
			ui.drive8.led_color= drive_active_led[0];
			break;
		case 1:
			ui.drive9.led_pwm1 = led_pwm1;
			ui.drive9.led_pwm2 = led_pwm2;
			ui.drive9.led_color= drive_active_led[1];
			break;
		case 2:
			ui.drive10.led_pwm1 = led_pwm1;
			ui.drive10.led_pwm2 = led_pwm2;
			ui.drive10.led_color= drive_active_led[2];
			break;
		case 3:
			ui.drive11.led_pwm1 = led_pwm1;
			ui.drive11.led_pwm2 = led_pwm2;
			ui.drive11.led_color= drive_active_led[3];
			break;
	}
	force_redraw();
}

void ui_display_drive_track(unsigned int drive_number, unsigned int drive_base, unsigned int half_track_number)
{
	switch (drive_number)
	{
		case 0:
			ui.drive8.drive_base = drive_base;
			ui.drive8.half_track_number = half_track_number;
			break;
		case 1:
			ui.drive9.drive_base = drive_base;
			ui.drive9.half_track_number = half_track_number;
			break;
		case 2:
			ui.drive10.drive_base = drive_base;
			ui.drive10.half_track_number = half_track_number;
			break;
		case 3:
			ui.drive11.drive_base = drive_base;
			ui.drive11.half_track_number = half_track_number;
			break;
	}
	force_redraw();
}

int ui_resources_init(void)
{
	if (resources_register_int(resources_int) < 0)
		return -1;

	if (resources_register_string(resources_string) < 0)
		return -1;

	return 0;
}

int ui_cmdline_options_init(void)
{
	/* needed */
}

int ui_init_finalize(void)
{
	int res_int;

	// Disable CartridgeReset here, in case the user quits before the machine has fully init'ed
	// there won't be anything to reset.
	// (yes.. cartridge reset is attempted even if the machine hasn't fully init'ed upon shutdown).

	resources_get_int("CartridgeReset", &res_int);
	resources_set_int("CartridgeReset", 0);

	// User can select an image to load here
	menu();

	// re-enable CartridgeReset 
	resources_set_int("CartridgeReset", res_int);
	return 0;
}

ui_jam_action_t ui_jam_dialog(const char *format,...)
{
	char *tmp;
	va_list args;

	va_start(args, format);
	debug_printf (format, args);
	va_end(args);

	/* Always hard reset.  */
	return UI_JAM_HARD_RESET;
}

void ui_shutdown(void)
{
    /* needed */
}

void ui_resources_shutdown(void)
{
    /* needed */
}

void _ui_menu_radio_helper(void)
{
    /* needed */
}

void ui_check_mouse_cursor(void)
{
    /* needed */
}

void ui_dispatch_events(void)
{
    /* needed */
}

void ui_display_speed(float speed, float frame_rate, int warp_enabled)
{
	ui.speed = speed;
	ui.frame_rate = frame_rate;
	ui.warp_enabled = warp_enabled;

	debug_printf_quick ("speed = %f, frame_rate = %f, warp_enabled = %d\n", speed, frame_rate, warp_enabled);
	force_redraw();
}

void ui_display_joyport(BYTE *joyport)
{
	/* needed */
}

void ui_display_event_time(unsigned int current, unsigned int total)
{
	/* needed */
}

void ui_display_volume(int vol)
{
}


char* ui_get_file(const char *format,...)
{
	return NULL;
}

