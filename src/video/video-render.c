/*
 * video-render.c - Implementation of framebuffer to physical screen copy
 *
 * Written by
 *  John Selck <graham@cruise.de>
 *  Dag Lem <resid@nimrod.no>
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

#include <stdio.h>

#include "render1x1.h"
#include "render1x1pal.h"
#include "render2x2pal.h"
#include "renderyuv.h"
#include "types.h"
#include "video-render.h"
#include "video-resources.h"
#include "video.h"

static void(*render_1x2_func)(video_render_config_t *, const BYTE *, BYTE *,
                              unsigned int, const unsigned int,
                              const unsigned int, const unsigned int,
                              const unsigned int, const unsigned int,
                              const unsigned int, const unsigned int,
                              int);

static void(*render_2x2_func)(video_render_config_t *, const BYTE *, BYTE *,
                              unsigned int, const unsigned int,
                              const unsigned int, const unsigned int,
                              const unsigned int, const unsigned int,
                              const unsigned int, const unsigned int,
                              int);

static void(*render_pal_func)(video_render_config_t *, BYTE *, BYTE *,
                              int, int, int, int,
                              int, int, int, int, int, viewport_t *);

static void(*render_crt_func)(video_render_config_t *, BYTE *, BYTE *,
                              int, int, int, int,
                              int, int, int, int, int, viewport_t *);


/* this function is the interface to the outer world */

int video_render_get_fake_pal_state(void)
{
	return video_resources.delayloop_emulation;
}

void video_render_initconfig(video_render_config_t *config)
{
	int i;

	config->rendermode = VIDEO_RENDER_NULL;
	config->doublescan = 0;

	for (i = 0; i < 256; i++)
		config->color_tables.physical_colors[i] = 0;
}

void video_render_setphysicalcolor(video_render_config_t *config, int index, DWORD color, int depth)
{
	/* duplicated colours are used by the double size 8/16 bpp renderers. */
	//hardcode for depth = 16
	#if 0
	switch (depth)
	{
		case 8:
			color &= 0x000000FF;
			color = color | (color << 8);
			break;
		case 16:
	#endif
			color &= 0x0000FFFF;
			color = color | (color << 16);
	#if 0
			break;
	}
	#endif
	config->color_tables.physical_colors[index] = color;
}

void video_render_main(video_render_config_t *config, BYTE *src, BYTE *trg, int width, int height, int xs, int ys, int xt, int yt, int pitchs, int pitcht, int depth, viewport_t *viewport)
{
	const video_render_color_tables_t *colortab;
	int rendermode;

#if 0
	log_debug("w:%i h:%i xs:%i ys:%i xt:%i yt:%i ps:%i pt:%i d%i",
			width, height, xs, ys, xt, yt, pitchs, pitcht, depth);

#endif
	if (width <= 0)
		return; /* some render routines don't like invalid width */

	rendermode = config->rendermode;
	colortab = &config->color_tables;

	switch (rendermode)
	{
		case VIDEO_RENDER_NULL:
			break;

		case VIDEO_RENDER_PAL_1X1:
		case VIDEO_RENDER_PAL_2X2:
			(*render_pal_func)(config, src, trg, width, height, xs, ys, xt, yt,
					pitchs, pitcht, depth, viewport);
			return;

		case VIDEO_RENDER_CRT_1X1:
		case VIDEO_RENDER_CRT_1X2:
		case VIDEO_RENDER_CRT_2X2:
			(*render_crt_func)(config, src, trg, width, height, xs, ys, xt, yt,
					pitchs, pitcht, depth, viewport);
			return;

		case VIDEO_RENDER_RGB_1X1:
			//hardcode for depth = 16
			#if 0
			switch (depth) {
				case 8:
					render_08_1x1_04(colortab, src, trg, width, height,
							xs, ys, xt, yt, pitchs, pitcht);
					return;
				case 16:
			#endif
					render_16_1x1_04(colortab, src, trg, width, height,
							xs, ys, xt, yt, pitchs, pitcht);
			#if 0
					return;
				case 24:
					render_24_1x1_04(colortab, src, trg, width, height,
							xs, ys, xt, yt, pitchs, pitcht);
					return;
				case 32:
					render_32_1x1_04(colortab, src, trg, width, height,
							xs, ys, xt, yt, pitchs, pitcht);
					return;
			}
			#endif
			return;

		case VIDEO_RENDER_RGB_1X2:
			(*render_1x2_func)(config, src, trg, width, height,
					xs, ys, xt, yt, pitchs, pitcht, depth);
			return;

		case VIDEO_RENDER_RGB_2X2:
			(*render_2x2_func)(config, src, trg, width, height,
					xs, ys, xt, yt, pitchs, pitcht, depth);
			return;
	}
}

void video_render_1x2func_set(void(*func)(video_render_config_t *,
                              const BYTE *, BYTE *,
                              unsigned int, const unsigned int,
                              const unsigned int, const unsigned int,
                              const unsigned int, const unsigned int,
                              const unsigned int, const unsigned int,
                              int))
{
	render_1x2_func = func;
}

void video_render_2x2func_set(void(*func)(video_render_config_t *,
                              const BYTE *, BYTE *,
                              unsigned int, const unsigned int,
                              const unsigned int, const unsigned int,
                              const unsigned int, const unsigned int,
                              const unsigned int, const unsigned int,
                              int))
{
	render_2x2_func = func;
}

void video_render_palfunc_set(void(*func)(video_render_config_t *,
                              BYTE *, BYTE *, int, int, int, int,
                              int, int, int, int, int, viewport_t *))
{
	render_pal_func = func;
}

void video_render_crtfunc_set(void(*func)(video_render_config_t *,
                              BYTE *, BYTE *, int, int, int, int,
                              int, int, int, int, int, viewport_t *))
{
	render_crt_func = func;
}
