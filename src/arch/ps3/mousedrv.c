/******************************************************************************* 
 *  -- mousedrv.c - Mouse driver for Playstation 3
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

#include <stdio.h>

#include "mouse.h"
#include "mousedrv.h"
#include "cell/mouse.h"

int ret;
int _mouse_available;
int _mouse_x, _mouse_y;
 
static CellMouseData data;

int mousedrv_resources_init(void)	{ return 0; }
int mousedrv_cmdline_options_init(void)	{ return 0; }
void mousedrv_mouse_changed(void)	{ }

void mousedrv_init(void)
{
	ret = cellMouseInit(1);
	#ifdef CELL_DEBUG
	if (ret != CELL_OK)
	{
		printf("cellMouseInit failed (%d)", ret);
	}
	#endif

	#ifdef CELL_DEBUG
	printf("Mouse is available.");
	#endif
	_mouse_available = 1;
}

void mousedrv_destroy(void)
{
	if (_mouse_available)
	{
		ret = cellMouseEnd();
		#ifdef CELL_DEBUG
		if (ret != CELL_OK)
		{
			printf("cellMouseEnd failed (%d)\n", ret);
		}
		#endif
	}
}

inline void update_mouse(void)
{ 
	static uint8_t old_status= 0;
	static uint32_t old_info = 0;

	static CellMouseInfo Info;
	static uint8_t old_buttons;


	if ((ret = cellMouseGetInfo (&Info)) != CELL_OK)
	{
#ifdef CELL_DEBUG
		printf("Error(%08X) : cellMouseGetInfo\n", ret);
#endif
		return;
	}

	if((Info.info & CELL_MOUSE_INFO_INTERCEPTED) && (!(old_info & CELL_MOUSE_INFO_INTERCEPTED)))
	{
#ifdef CELL_DEBUG
		printf("Lost mouse\n");
#endif
		old_info = Info.info;
	} else if((!(Info.info & CELL_MOUSE_INFO_INTERCEPTED)) && (old_info & CELL_MOUSE_INFO_INTERCEPTED))
	{
#ifdef CELL_DEBUG
		printf("Found mouse\n");
#endif
		old_info = Info.info;
	}

	if ( (old_status != CELL_MOUSE_STATUS_DISCONNECTED) && (Info.status[0] == CELL_MOUSE_STATUS_DISCONNECTED))
	{
#ifdef CELL_DEBUG
		printf("Mouse disconnected\n");
#endif
		old_info = Info.info;
		return;
	}

	if (Info.status[0] == CELL_MOUSE_STATUS_DISCONNECTED)
	{
		old_info = Info.info;
		return;
	}


	if ( (old_status == CELL_MOUSE_STATUS_DISCONNECTED) && (Info.status[0] != CELL_MOUSE_STATUS_DISCONNECTED))
	{
#ifdef CELL_DEBUG
		printf("New Mouse %d is connected: VENDOR_ID=%d PRODUCT_ID=%d\n", 0, Info.vendor_id[0], Info.product_id[0]);
#endif
	}


	ret = cellMouseGetData (0, &data);

	if (ret != CELL_OK)
	{
#ifdef CELL_DEBUG
		printf("Read Error(%08X) port id %d\n", ret , 0);
#endif
		old_info = Info.info;
		return;
	}


	if (data.update == CELL_MOUSE_DATA_UPDATE)
	{
		_mouse_x = (_mouse_x + data.x_axis) & 0xff;
		_mouse_y = (_mouse_y + data.y_axis) & 0xff;

		if (data.buttons & CELL_MOUSE_BUTTON_1)
			mouse_button_left(1);
		else
			mouse_button_left(0);

		if (data.buttons & CELL_MOUSE_BUTTON_2)
			mouse_button_right(1);
		else
			mouse_button_right(0);

		old_buttons = data.buttons;
	}
	else
	{
		if (old_buttons & CELL_MOUSE_BUTTON_1)
			mouse_button_left(1);
		else
			mouse_button_left(0);

		if (old_buttons & CELL_MOUSE_BUTTON_2)
			mouse_button_right(1);
		else
			mouse_button_right(0);
	}

	old_status = Info.status[0];
}



#define ACCEL 2

BYTE mousedrv_get_x(void)
{
	if (!_mouse_available || !_mouse_enabled)
		return 0xff;
	return (BYTE)(_mouse_x * ACCEL >> 1) & 0x7e;
}

BYTE mousedrv_get_y(void)
{
	if (!_mouse_available || !_mouse_enabled)
		return 0xff;
	return (BYTE)(~_mouse_y * ACCEL >> 1) & 0x7e;
}
