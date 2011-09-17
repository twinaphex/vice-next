/******************************************************************************* 
 *  -- video.cpp    Graphics handling interface for Vice, based on the cellframework graphics library
 *
 *     VICE PS3 -   Commodore 64 emulator for the Playstation 3
 *                  ported from the original VICE distribution
 *                  located at http://sourceforge.net/projects/vice-emu/
 *
 *
 *  Copyright (C) 2010
 *      Author:      TimRex
 *      Based on existing drivers for MSDOS and SDL environments
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

#include <string>
#include <sys/sys_time.h>

#include "cmdline.h"
#include "palette.h"
#include "ui.h"
#include "emu-ps3.hpp"

#include "common.h"

extern "C" {
#include "lib.h"
#include "util.h"
#include "machine.h"
#include "raster.h"
#include "video.h"
#include "videoarch.h"

unsigned char *screenbuffer = NULL;

video_canvas_t *last_canvas;

#define MAX_CANVAS_NUM 2
video_canvas_t *active_canvas = NULL;
static video_canvas_t *canvaslist[MAX_CANVAS_NUM];

/* ------------------------------------------------------------------------- */

int sysutil_drawing = 0;
static int last_redraw = 0;

static void canvas_change_palette(video_canvas_t *c)
{
}

void set_last_redraw()
{
	last_redraw = sys_time_get_system_time();
}

unsigned long get_last_redraw(void)
{
	return last_redraw;
}


int video_arch_resources_init(void)
{
	return 0;
}

void video_arch_resources_shutdown(void)
{
	return;
}

/* ------------------------------------------------------------------------- */

/* Video-specific command-line options.  */

int video_init_cmdline_options(void)
{
    return 0;
}

/* ------------------------------------------------------------------------- */

int video_init(void)
{
	last_canvas = NULL;
	active_canvas = NULL;

	for (int i = 0; i < MAX_CANVAS_NUM; i++)
		canvaslist[i] = NULL;

	return 0;
}

void video_shutdown(void)
{    
	active_canvas = NULL;

	for (int i = 0; i < MAX_CANVAS_NUM; i++)
		canvaslist[i] = NULL;

	return;
}

void video_arch_canvas_init(struct video_canvas_s *canvas)
{
	canvas->video_draw_buffer_callback = NULL;
	active_canvas = canvas;

	#ifdef CELL_DEBUG
	printf("canvas init width wants to be : %d\ncanvas init height wants to be : %d\n", canvas->width, canvas->height);
	#endif
}

void video_canvas_resize(video_canvas_t *canvas, unsigned int width, unsigned int height)
{
	#ifdef CELL_DEBUG
	printf("canvas width resize wants to be : %d\ncanvas height resize wants to be : %d\n", width, height);
	#endif

	if ((screenbuffer != NULL) && (canvas->width == width) && (canvas->height == height))
	{
		#ifdef CELL_DEBUG
		printf("no realloc necessary\n");
		#endif
	}
	else
		screenbuffer = (unsigned char *) lib_realloc(screenbuffer, width * height * canvas->depth / 8);

	canvas->width = width;
	canvas->height = height;
	canvas->bytes_per_line = canvas->width * canvas->depth / 8;

	video_canvas_set_palette(canvas, canvas->palette);
	Graphics->PSGLReInit (width, height, canvas->depth);
}

/* Note: `mapped' is ignored.  */
video_canvas_t *video_canvas_create(video_canvas_t *canvas, unsigned int *width, unsigned int *height, int mapped)
{
	int next_canvas = 0;
	static int vicii_setup = 0;

	canvas->videoconfig->rendermode = VIDEO_RENDER_RGB_1X1;

#ifdef CELL_DEBUG
	printf("canvas width wants to be : %d\ncanvas height wants to be : %d\ncanvas depth wants to be : %d\n", canvas->width, canvas->height, canvas->depth);
#endif

	canvas->depth = 16;

#ifdef CELL_DEBUG
	printf("canvas set to %d x %d\n", canvas->width, canvas->height);
#endif

	canvas->bytes_per_line = canvas->width * (canvas->depth / 8);

	screenbuffer = (unsigned char *) lib_malloc(canvas->width * canvas->height * (canvas->depth / 8));
	video_canvas_set_palette(canvas, canvas->palette);

	while (canvaslist[next_canvas] != NULL && next_canvas < MAX_CANVAS_NUM - 1)
		next_canvas++;

	canvaslist[next_canvas] = canvas;

	// TODO This hack ensures the C128 always uses the VICII canvas and never the VDC canvas

	if (machine_class == VICE_MACHINE_C128)
	{
#ifdef CELL_DEBUG
		printf("forcing C128 canvas as canvas[1]\n");
#endif
		active_canvas=canvaslist[1];
	}
	else
		active_canvas=canvaslist[0];

	return canvas;
}


void video_canvas_destroy(video_canvas_t *canvas)
{
	if (canvas == NULL)
		return;

	// TODO free screenbuffer / overlaybuffer
	//canvas_free_bitmaps(canvas);

	for (int i = 0; i < MAX_CANVAS_NUM; i++)
	{
		if (canvaslist[i] == canvas)
			canvaslist[i] = NULL;
	}
}



/* ------------------------------------------------------------------------- */


//inline void video_canvas_refresh(video_canvas_t *canvas,
void video_canvas_refresh(video_canvas_t *canvas, unsigned int xs, unsigned int ys, unsigned int xi, unsigned int yi, unsigned int w, unsigned int h)
{
	// TODO This version will NOT draw the VDC
	if (active_canvas != canvas)
		return;

	/* this is a hack for F7 change between VICII and VDC */
	if (active_canvas != canvas)
	{
		active_canvas = canvas;
		//canvas_update_colors(canvas);
		//clear(screen);
	}

	if (last_canvas != canvas)
		last_canvas = canvas;

	video_canvas_render(canvas, (unsigned char *)screenbuffer, w, h, xs, ys, xi, yi, canvas->bytes_per_line, canvas->depth);

	set_last_redraw();
	Graphics->Draw (canvas->width, canvas->height, (std::uint16_t *)screenbuffer);
	psglSwap();
}

void fullscreen_capability(cap_fullscreen_t *cap_fullscreen)
{
}

struct GLpalette_s {
	unsigned char r;
	unsigned char g;
	unsigned char b;
} colors[256];

static int makecol_RGB555BE(int r, int g, int b)
{
	int c = ((r & 0xf8) << 7) | ((g & 0xf8) << 2) | ((b & 0xf8) >> 3);
	return c;
}

int video_canvas_set_palette(struct video_canvas_s *canvas, struct palette_s *palette)
{
	unsigned int i, col;
	//canvas->palette = palette;
	canvas_change_palette(canvas);

	canvas->palette = palette;

	for (i = 0; i < palette->num_entries; i++)
		video_render_setphysicalcolor(canvas->videoconfig, i, makecol_RGB555BE(palette->entries[i].red, palette->entries[i].green, palette->entries[i].blue), canvas->depth);

	return 0;
}

void force_redraw (void)
{
	if (active_canvas->parent_raster != NULL)
		raster_force_repaint(active_canvas->parent_raster);
}

#ifdef __cplusplus
}
#endif
