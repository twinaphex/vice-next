/*
 * videoarch.h
 *
 * Written by
 *  Mathias Roslund <vice.emu@amidog.se>
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

#ifndef VICE_VIDEOARCH_H
#define VICE_VIDEOARCH_H

#include "video.h"

void set_last_redraw(void);
unsigned long get_last_redraw(void);
void force_redraw(void);


struct palette_s;
struct video_draw_buffer_callback_s;

struct video_canvas_s {
	struct video_canvas_s *next; /* linked list */
	unsigned int initialized;
	unsigned int created;
	unsigned int width, height, depth, bytes_per_line;
	unsigned int use_triple_buffering;
	struct video_render_config_s *videoconfig;
	struct draw_buffer_s *draw_buffer;
	struct viewport_s *viewport;
	struct geometry_s *geometry;
	struct palette_s *palette;

	struct raster_s *parent_raster;	// Much like the SDL port, the PS3 needs to force a repaint occasionally

	struct os_s *os; /* AmigaOS specific */
	struct video_draw_buffer_callback_s *video_draw_buffer_callback;
	int current_fullscreen;
	int window_left;
	int window_top;
	int waiting_for_resize;
};

typedef struct video_canvas_s video_canvas_t;

extern BYTE *screenbuffer;

extern video_canvas_t *last_canvas;

#endif
