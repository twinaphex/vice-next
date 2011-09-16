/*
 * soundps3.c - Implementation of the PS3 audio device
 *
 * Written by
 *  TimRex <timwrecks@gmail.com>
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

#include <stdlib.h>
#include <sys/timer.h>
#include "audio/rsound.hpp"

extern "C" {
#include "lib.h"
}
#include "sound.h"

#define AUDIO_BLOCK_SAMPLES (256)
#define AUDIO_CHANNELS (2)

static size_t stereo_pbuf_size=0; 
static SWORD *stereo_pbuf=NULL; 

static Audio::Stream<int16_t> *CellAudio = NULL;
static int num_channels;

/*
 * PS3 Audio
 *  1 Packet = 1 Frame 
 *  1 Frame  = 1 or 2 Samples (SWORD)  1:mono SID, 2:stereo SID
 *  1 Slice  = n Frames
 *
 * VICE Audio:
 *  1 Fragment = n Frames     (n=fragsize)
 *  Soundbuffer = m Fragments (m=fragnum)
 */


int ps3_audio_init(const char *param, int *speed, int *fragsize, int *fragnr, int *channels)
{
	#ifdef CELL_DEBUG
	printf("PS3 Audio : ps3_audio_init called with speed=%d, fragsiz=%d, fragnr=%d, channels=%d\n", *speed, *fragsize, *fragnr, *channels);
	#endif

	// Must use these values.
	//*fragnr *=4;
	//*fragsize=256;
	//*speed = 44100;

	*speed = 48000;


	*fragsize=1024;
	*fragnr = 20;

	num_channels =* channels;

	#ifdef CELL_DEBUG
	printf("PS3 Audio : forcing settings to speed=%d, fragsiz=%d, fragnr=%d, channels=%d\n", *speed, *fragsize, *fragnr, *channels);
	#endif

	if(CellAudio)
	{
		delete CellAudio;
	}
	//if((Settings.RSoundEnabled) && (strlen(Settings.RSoundServerIPAddress) > 0))
	//	{
	//		CellAudio = new Audio::RSound<int16_t>(Settings.RSoundServerIPAddress, AUDIO_INPUT_RATE);
	//	}
	//	else
	//	{

	size_t buflen = (*fragsize) * (*fragnr) ;

	#ifdef CELL_DEBUG
	printf("PS3 Audio : setting up audioport with buffer size %d\n", buflen);
	#endif

	// Promote mono audio to stereo
	CellAudio = new Audio::AudioPort<int16_t>(2, *speed, buflen);

	stereo_pbuf = NULL;

	CellAudio->unpause();
	return 0;
}

int ps3_audio_write(SWORD *pbuf, size_t nr)
{
	SWORD *stereo_ptr; 
	SWORD *mono_ptr; 

	if (nr == 0)
		return 0;

	// Audioport requires 256 frames of audio at a time.

	if (nr >= (AUDIO_BLOCK_SAMPLES * AUDIO_CHANNELS))
	{
		if (num_channels == 1)
		{
			// PS3 requires stereo audio.
			// fudge this.
			if (stereo_pbuf)
			{
				if (stereo_pbuf_size != (sizeof(SWORD) * nr * 2))
				{
					stereo_pbuf_size = sizeof(SWORD) * nr * 2;
					stereo_pbuf = (SWORD *) lib_realloc(stereo_pbuf, stereo_pbuf_size);
				}
			}
			else
			{
				stereo_pbuf_size = sizeof(SWORD) * nr * 2;
				stereo_pbuf = (SWORD *) lib_malloc(stereo_pbuf_size);
			}

			stereo_ptr = stereo_pbuf;
			mono_ptr = pbuf;

			for (size_t i=0; i<nr; i++)
			{
				*stereo_ptr++ = *mono_ptr;
				*stereo_ptr++ = *mono_ptr++;
			}
			CellAudio->write(stereo_pbuf, nr*2);
		}
		else
		{
			CellAudio->write(pbuf, nr);
		}


	}
	return 0;
}

int ps3_audio_bufferspace(void)
{
	return CellAudio->write_avail();
}

void ps3_audio_close(void)
{
	if (stereo_pbuf)
	{
		lib_free(stereo_pbuf);
		stereo_pbuf = NULL;
	}
	CellAudio->pause();
	delete CellAudio;
	CellAudio = NULL;
}

int ps3_audio_suspend(void)
{
	if (CellAudio)
		CellAudio->pause();
	return 0;
}

int ps3_audio_resume(void)
{
	if (CellAudio)
		CellAudio->unpause();
	return 0;
}

static sound_device_t ps3_audio_device =
{
	"ps3",
	ps3_audio_init,
	ps3_audio_write,
	NULL,
	NULL,
	ps3_audio_bufferspace,
	ps3_audio_close,
	ps3_audio_suspend,
	ps3_audio_resume,
	1
};

int sound_init_ps3_device(void)
{
	return sound_register_device(&ps3_audio_device);
}

