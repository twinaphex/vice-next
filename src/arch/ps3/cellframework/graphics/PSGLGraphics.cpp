/******************************************************************************* 
 *  -- Cellframework -  Open framework to abstract the common tasks related to
 *                      PS3 application development.
 *
 *  Copyright (C) 2010 - 2011
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

#include "PSGLGraphics.h"

#include <stdio.h>
#include <string.h>
#include <sys/sys_time.h>

#include "ps3debug.h"

PSGLGraphics::PSGLGraphics()
{
	psgl_device = NULL;
	psgl_context = NULL;
	gl_width = 0;
	gl_height = 0;
}

PSGLGraphics::~PSGLGraphics()
{

}

PSGLdeviceParameters PSGLGraphics::InitCommon(uint32_t resolutionId, uint16_t pal60Hz)
{
	PSGLinitOptions options =
	{
		enable: PSGL_INIT_MAX_SPUS | PSGL_INIT_INITIALIZE_SPUS | PSGL_INIT_HOST_MEMORY_SIZE,
		maxSPUs: 1,
		initializeSPUs: false,
		persistentMemorySize: 0,
		transientMemorySize: 0,
		errorConsole: 0,
		fifoSize: 0,
		hostMemorySize: 8 * 1024 * 1024
	};

	psglInit(&options);

	PSGLdeviceParameters params;
	params.enable = PSGL_DEVICE_PARAMETERS_COLOR_FORMAT | PSGL_DEVICE_PARAMETERS_DEPTH_FORMAT | PSGL_DEVICE_PARAMETERS_MULTISAMPLING_MODE;
	params.colorFormat = GL_ARGB_SCE;
	params.depthFormat = GL_NONE;
	params.multisamplingMode = GL_MULTISAMPLING_NONE_SCE;

	if(pal60Hz)
	{
		params.enable |= PSGL_DEVICE_PARAMETERS_RESC_PAL_TEMPORAL_MODE;
		params.rescPalTemporalMode = RESC_PAL_TEMPORAL_MODE_60_INTERPOLATE;
		params.enable |= PSGL_DEVICE_PARAMETERS_RESC_RATIO_MODE;
		params.rescRatioMode = RESC_RATIO_MODE_FULLSCREEN;
	}

	if(resolutionId != NULL)
	{
		//Resolution setting
		CellVideoOutResolution resolution;
		cellVideoOutGetResolution(resolutionId, &resolution);
		
		params.enable |= PSGL_DEVICE_PARAMETERS_WIDTH_HEIGHT;
		params.width = resolution.width;
		params.height = resolution.height;
	}

	return params;
}

void PSGLGraphics::Init()
{
	PSGLdeviceParameters myParams;
	myParams	= InitCommon(NULL, 0);

	psgl_device = psglCreateDeviceExtended(&myParams);


	psglGetDeviceDimensions(psgl_device, &gl_width, &gl_height); // Get the dimensions of the screen in question, and do stuff with it :)

	psgl_context = psglCreateContext(); // Create a context and bind it to the current display.
	

	psglMakeCurrent(psgl_context, psgl_device);

	psglResetCurrentContext();
}

void PSGLGraphics::Init(uint32_t resolutionId, uint16_t pal60Hz)
{
	PSGLdeviceParameters myParams;
	myParams	= InitCommon(resolutionId, pal60Hz);

	psgl_device = psglCreateDeviceExtended(&myParams);

	psglGetDeviceDimensions(psgl_device, &gl_width, &gl_height); // Get the dimensions of the screen in question, and do stuff with it :)

	psgl_context = psglCreateContext(); // Create a context and bind it to the current display.

	psglMakeCurrent(psgl_context, psgl_device);

	psglResetCurrentContext();
}

void PSGLGraphics::GetAllAvailableResolutions()
{
	if(CheckResolution(CELL_VIDEO_OUT_RESOLUTION_480))
	{
		AddResolution(CELL_VIDEO_OUT_RESOLUTION_480);
		initialResolution = CELL_VIDEO_OUT_RESOLUTION_480;
	}
	if(CheckResolution(CELL_VIDEO_OUT_RESOLUTION_576))
	{
		AddResolution(CELL_VIDEO_OUT_RESOLUTION_576);
		initialResolution = CELL_VIDEO_OUT_RESOLUTION_576;
	}
	if(CheckResolution(CELL_VIDEO_OUT_RESOLUTION_960x1080))
	{
		AddResolution(CELL_VIDEO_OUT_RESOLUTION_960x1080);
		initialResolution = CELL_VIDEO_OUT_RESOLUTION_960x1080;
	}
	if(CheckResolution(CELL_VIDEO_OUT_RESOLUTION_720))
	{
		AddResolution(CELL_VIDEO_OUT_RESOLUTION_720);
		initialResolution = CELL_VIDEO_OUT_RESOLUTION_720;
	}
	if(CheckResolution(CELL_VIDEO_OUT_RESOLUTION_1280x1080))
	{
		AddResolution(CELL_VIDEO_OUT_RESOLUTION_1280x1080);
		initialResolution = CELL_VIDEO_OUT_RESOLUTION_1280x1080;
	}
	if(CheckResolution(CELL_VIDEO_OUT_RESOLUTION_1440x1080))
	{
		AddResolution(CELL_VIDEO_OUT_RESOLUTION_1440x1080);
		initialResolution = CELL_VIDEO_OUT_RESOLUTION_1440x1080;
	}
	if(CheckResolution(CELL_VIDEO_OUT_RESOLUTION_1600x1080))
	{
		AddResolution(CELL_VIDEO_OUT_RESOLUTION_1600x1080);
		initialResolution = CELL_VIDEO_OUT_RESOLUTION_1600x1080;
	}
	if(CheckResolution(CELL_VIDEO_OUT_RESOLUTION_1080))
	{
		AddResolution(CELL_VIDEO_OUT_RESOLUTION_1080);
		initialResolution = CELL_VIDEO_OUT_RESOLUTION_1080;
	}
	currentResolution = supportedResolutions.size()-1;
}

int PSGLGraphics::CheckResolution(uint32_t resId)
{
	return cellVideoOutGetResolutionAvailability(CELL_VIDEO_OUT_PRIMARY, resId, CELL_VIDEO_OUT_ASPECT_AUTO,0);
}

int PSGLGraphics::AddResolution(uint32_t resId)
{
	supportedResolutions.push_back(resId);
}

void PSGLGraphics::NextResolution()
{
	if(currentResolution+1 < supportedResolutions.size())
	{
		currentResolution++;
	}
}

void PSGLGraphics::PreviousResolution()
{
	if(currentResolution > 0)
	{
		currentResolution--;
	}
}

void PSGLGraphics::SwitchResolution(uint32_t resId, uint16_t pal60Hz)
{
	if(CheckResolution(resId))
	{
		ChangeResolution(resId, pal60Hz);
	}
}

uint32_t PSGLGraphics::GetInitialResolution()
{
	return initialResolution;
}

uint32_t PSGLGraphics::GetCurrentResolution()
{
	return supportedResolutions[currentResolution];
}

int32_t PSGLGraphics::ChangeResolution(uint32_t resId, uint16_t pal60Hz)
{
	int32_t ret;

	DeinitDbgFont();
	psglDestroyContext(psgl_context);
	psglDestroyDevice(psgl_device);
	
	Init(resId, pal60Hz);
	InitDbgFont();
	SetResolution();
}

void PSGLGraphics::SetResolution()
{
		cellVideoOutGetState(CELL_VIDEO_OUT_PRIMARY, 0, &stored_video_state);
}

CellVideoOutState PSGLGraphics::GetVideoOutState()
{
	return stored_video_state;
}

void PSGLGraphics::Deinit()
{
	glFinish();
	this->DeinitDbgFont();

	psglDestroyContext(psgl_context);
	psglDestroyDevice(psgl_device);
#ifdef PS3_SDK_3_41
	//FIXME: It will crash here for 1.92 - termination of the PSGL library - works fine for 3.41
	psglExit();
#else
	//for 1.92
	gl_width = 0;
	gl_height = 0;
	psgl_context = NULL;
	psgl_device = NULL;
#endif
}

void PSGLGraphics::InitDbgFont()
{
	CellDbgFontConfig cfg;
	memset(&cfg, 0, sizeof(cfg));
	cfg.bufSize = 512;
	cfg.screenWidth = gl_width;
	cfg.screenHeight = gl_height;
	cellDbgFontInit(&cfg);
}


void PSGLGraphics::DeinitDbgFont()
{
	cellDbgFontExit();
}


GLfloat PSGLGraphics::GetDeviceAspectRatio()
{
	return psglGetDeviceAspectRatio(psgl_device);
}


GLint PSGLGraphics::GetResolutionWidth()
{
	return gl_width;
}


GLint PSGLGraphics::GetResolutionHeight()
{
	return gl_height;
}

//FIXME: classify this
void dprintf_console(const char* fmt, ...)
{
	glClear(GL_COLOR_BUFFER_BIT);

	va_list ap;

	va_start(ap, fmt);
	cellDbgFontConsoleVprintf(dbg_id, fmt, ap);
	va_end(ap);

	cellDbgFontConsoleEnable(dbg_id);
	cellDbgFontDraw();
	psglSwap();
}

void dprintf_noswap(float x, float y, float scale, const char* fmt, ...)
{
	char buffer[512];

	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buffer, 512, fmt, ap);
	cellDbgFontPuts(x, y, scale, 0xffffffff, buffer);
	va_end(ap);

	cellDbgFontDraw();
}

void write_fps(void)
{
	static float last_time = 0.0;
	float new_time = sys_time_get_system_time() / 1000000.0;
	float delta = new_time - last_time;
	last_time = new_time;

	dprintf_noswap(0.1, 0.1, 1.0, "FPS: %.2f\n", 1.0f/delta);
}


void gl_dprintf(float x, float y, float scale, const char* fmt, ...)
{
	glClear(GL_COLOR_BUFFER_BIT);

	char buffer[512];

	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buffer, 512, fmt, ap);
	cellDbgFontPuts(x, y, scale, 0xffffffff, buffer);
	va_end(ap);

	cellDbgFontDraw();
	psglSwap();
}
