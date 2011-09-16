/*
 * vsidui.c - Implementation of the VSID UI.
 *
 * Written by
 *  Dag Lem <resid@nimrod.no>
 * based on c64ui.c written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Andr� Fachat <fachat@physik.tu-chemnitz.de>
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#include "vice.h"

#include "machine.h"


int vsid_ui_init(void)
{
	return 0;
}

void vsid_ui_display_name(const char *name)
{
	#ifdef CELL_DEBUG
	printf("Name: %s", name);
	#endif
}

void vsid_ui_display_author(const char *author)
{
	#ifdef CELL_DEBUG
	printf("Author: %s", author);
	#endif
}

void vsid_ui_display_copyright(const char *copyright)
{
	#ifdef CELL_DEBUG
	printf("Copyright by: %s", copyright);
	#endif
}

void vsid_ui_display_sync(int sync)
{
	#ifdef CELL_DEBUG
	printf("Using %s sync", sync == MACHINE_SYNC_PAL ? "PAL" : "NTSC");
	#endif
}

void vsid_ui_display_sid_model(int model)
{
	#ifdef CELL_DEBUG
	printf("Using %s emulation", model == 0 ? "MOS6581" : "MOS8580");
	#endif
}

void vsid_ui_set_default_tune(int nr)
{
	#ifdef CELL_DEBUG
	printf("Default Tune: %i", nr);
	#endif
}

void vsid_ui_display_tune_nr(int nr)
{
	#ifdef CELL_DEBUG
	printf("Playing Tune: %i", nr);
	#endif
}

void vsid_ui_display_nr_of_tunes(int count)
{
	#ifdef CELL_DEBUG
	printf("Number of Tunes: %i", count);
	#endif
}

void vsid_ui_display_time(unsigned int sec)
{
}

void vsid_ui_display_irqtype(const char *irq)
{
}

void vsid_ui_close(void)
{
}

void vsid_ui_setdrv(char* driver_info_text)
{
}
