/******************************************************************************* 
 *  -- ps3video.cpp    Interface to cellframework library Graphics.h
 *
 *     VICE PS3 -   Commodore 64 emulator for the Playstation 3
 *                  ported from the original VICE distribution
 *                  located at http://sourceforge.net/projects/vice-emu/
 *
 *
 *  Copyright (C) 2010
 *  Created on: Nov 14, 2010
 *      Adapted for Vice by:  TimRex
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

#include <sys/sys_time.h>
#include <string.h>

#include "ps3video.hpp"

#include "common.h"
#include "emu-ps3.hpp"
#include "colors.h"

extern "C" {
#include "resources.h"
#include "util.h"
#include "lib.h"
#include "drive.h"
#include "datasette.h"
#include "tape.h"
#include "ui.h"
}

static unsigned char *pPixels;
static const char* active_shader=NULL;
//static int offset=0;
static unsigned int last_redraw=0;

unsigned int PS3Graphics::TimeSinceLastDraw()
{
	return abs(sys_time_get_system_time() - last_redraw);
}

const char* PS3Graphics::GetCurrentShader()
{
	return active_shader;
}

int32_t PS3Graphics::PSGLReInit(int width, int height, int depth)
{
	context_width = width;
	context_height = height;

#ifdef CELL_DEBUG
	printf("ReInit called with width=%d, height=%d, depth=%d\n", width, height, depth);
#endif

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glBindBuffer(GL_TEXTURE_REFERENCE_BUFFER_SCE, vbo[0]);

	glBufferData(GL_TEXTURE_REFERENCE_BUFFER_SCE, width * height * (depth / 8), NULL, GL_STREAM_DRAW);
	glTextureReferenceSCE(GL_TEXTURE_2D, 1, width, height, 0, GL_RGB5_A1, width * (depth / 8), 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	return CELL_OK;
}

void PS3Graphics::ScreenDump (void)
{
	pPixels = new unsigned char[context_width * context_height * 2];

	unsigned char *buffer = (unsigned char *) glMapBuffer(GL_TEXTURE_REFERENCE_BUFFER_SCE, GL_READ_ONLY);
	if (buffer != NULL)
		memcpy (pPixels, buffer, context_width * context_height * 2);
	glUnmapBuffer(GL_TEXTURE_REFERENCE_BUFFER_SCE);
}


void PS3Graphics::DumpScreen (void)
{
	Draw (context_width, context_height, (std::uint16_t*) pPixels);
}


void PS3Graphics::DestroyDump (void)
{
	delete []pPixels;
}

unsigned char* PS3Graphics::RetrieveDump (void)
{
	return pPixels;
}

int PS3Graphics::RetrieveDumpSize (void)
{
	return context_width * context_height * 2;
}

/******************************************************************************* 
	PSGL
********************************************************************************/

PS3Graphics::PS3Graphics()
{
	psgl_device = NULL;
	psgl_context = NULL;
	gl_width = 0;
	gl_height = 0;

	m_smooth = false;
	m_pal60Hz = false;
	m_overscan = false;
	m_overscan_amount = 0.0;
}

PS3Graphics::~PS3Graphics()
{
	Deinit();
}

void PS3Graphics::Init()
{
	PSGLInitDevice(NULL, m_pal60Hz);
	int32_t ret = PSGLInit();

	if (ret == CELL_OK)
	{
		SetResolution();
	}
	GetAllAvailableResolutions();

	// TODO : Initially set to false. This will be overridden later
	SetAspectRatio(false);
}

void PS3Graphics::PSGLInitDevice(uint32_t resolutionId, uint16_t pal60Hz/*, uint16_t tripleBuffering*/)
{
	PSGLdeviceParameters params;
	PSGLinitOptions options;
	options.enable = PSGL_INIT_MAX_SPUS | PSGL_INIT_INITIALIZE_SPUS;
#if CELL_SDK_VERSION == 0x340001
	options.enable |= PSGL_INIT_TRANSIENT_MEMORY_SIZE;
#else
	options.enable |=	PSGL_INIT_HOST_MEMORY_SIZE;
#endif
	options.maxSPUs = 1;
	options.initializeSPUs = GL_FALSE;
	options.persistentMemorySize = 0;
	options.transientMemorySize = 0;
	options.errorConsole = 0;
	options.fifoSize = 0;
	options.hostMemorySize = 0;

	psglInit(&options);

	params.enable = PSGL_DEVICE_PARAMETERS_COLOR_FORMAT | \
			PSGL_DEVICE_PARAMETERS_DEPTH_FORMAT | \
			PSGL_DEVICE_PARAMETERS_MULTISAMPLING_MODE;
	params.colorFormat = GL_ARGB_SCE;
	params.depthFormat = GL_NONE;
	params.multisamplingMode = GL_MULTISAMPLING_NONE_SCE;

	/*
	if (tripleBuffering)
	{
		params.enable |= PSGL_DEVICE_PARAMETERS_BUFFERING_MODE;
		params.bufferingMode = PSGL_BUFFERING_MODE_TRIPLE;
	}
	*/

	if (pal60Hz)
	{
		params.enable |= PSGL_DEVICE_PARAMETERS_RESC_PAL_TEMPORAL_MODE;
		params.rescPalTemporalMode = RESC_PAL_TEMPORAL_MODE_60_INTERPOLATE;
		params.enable |= PSGL_DEVICE_PARAMETERS_RESC_RATIO_MODE;
		params.rescRatioMode = RESC_RATIO_MODE_FULLSCREEN;
	}

	if (resolutionId)
	{
		//Resolution setting
		CellVideoOutResolution resolution;
		cellVideoOutGetResolution(resolutionId, &resolution);

		params.enable |= PSGL_DEVICE_PARAMETERS_WIDTH_HEIGHT;
		params.width = resolution.width;
		params.height = resolution.height;
		m_currentResolutionId = resolutionId;
	}

	psgl_device = psglCreateDeviceExtended(&params);

	// Get the dimensions of the screen in question, and do stuff with it :)
	psglGetDeviceDimensions(psgl_device, &gl_width, &gl_height); 

	// Create a context and bind it to the current display.
	psgl_context = psglCreateContext();

	/*
	if(m_viewport_width == 0)
		m_viewport_width = gl_width;
	if(m_viewport_height == 0)
		m_viewport_height = gl_height;
	*/

	psglMakeCurrent(psgl_context, psgl_device);

	psglResetCurrentContext();
}

int32_t PS3Graphics::PSGLInit()
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_LIGHTING);
	glDisable(GL_FOG);
	glDisable(GL_DITHER);
	glShadeModel(GL_FLAT);
	glEnable(GL_VSYNC_SCE);
	glEnable(GL_TEXTURE_2D);

	/*
	//glEnable(GL_BLEND);
	//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
	//glBlendColor(0, 0, 0, 0);
	//glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	 */

	context_width = SCREEN_RENDER_TEXTURE_WIDTH;
	context_height = SCREEN_RENDER_TEXTURE_HEIGHT;

	InitCg();

	SetViewports();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glGenBuffers(2, vbo);

	glBindBuffer(GL_TEXTURE_REFERENCE_BUFFER_SCE, vbo[0]);
	glBufferData(GL_TEXTURE_REFERENCE_BUFFER_SCE, SCREEN_RENDER_TEXTURE_HEIGHT * SCREEN_RENDER_TEXTURE_PITCH, NULL, GL_STREAM_DRAW);
	glTextureReferenceSCE(GL_TEXTURE_2D, 1, SCREEN_RENDER_TEXTURE_WIDTH, SCREEN_RENDER_TEXTURE_HEIGHT, 0, GL_RGB5_A1, SCREEN_RENDER_TEXTURE_PITCH, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	SetSmooth(m_smooth);

	// PSGL doesn't clear the screen on startup, so let's do that here.
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glColor4f(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	psglSwap();

	// Use some initial values for the screen quad.
	GLfloat vertexes[] = {
		0, 0, 0,
		0, 1, 0,
		1, 1, 0,
		1, 0, 0,
		0, 1,
		0, 0,
		1, 0,
		1, 1
	};

	GLfloat vertex_buf[128];
	__builtin_memcpy(vertex_buf, vertexes, 12 * sizeof(GLfloat));
	__builtin_memcpy(vertex_buf + 32, vertexes + 12, 8 * sizeof(GLfloat));
	__builtin_memcpy(vertex_buf + 32 * 3, vertexes + 12, 8 * sizeof(GLfloat));

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, 256, vertex_buf, GL_STATIC_DRAW);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, 0);
	glTexCoordPointer(2, GL_FLOAT, 0, (void*)128);

	return CELL_OK;
}

void PS3Graphics::Deinit()
{
	glDeleteTextures(1, &tex);
	PSGLDeInitDevice();
}

void PS3Graphics::PSGLDeInitDevice()
{
	glFinish();
	cellDbgFontExit();

	psglDestroyContext(psgl_context);
	psglDestroyDevice(psgl_device);
#if CELL_SDK_VERSION == 0x340001
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

/******************************************************************************* 
	libdbgfont
********************************************************************************/

void PS3Graphics::InitDbgFont()
{
	CellDbgFontConfig cfg;
	memset(&cfg, 0, sizeof(cfg));
	cfg.bufSize = 512;
	cfg.screenWidth = gl_width;
	cfg.screenHeight = gl_height;
	cellDbgFontInit(&cfg);
}

#ifdef CELL_DEBUG_FPS
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
#endif

/******************************************************************************* 
	Draw functions
********************************************************************************/

void PS3Graphics::UpdateCgParams(unsigned width, unsigned height, unsigned tex_width, unsigned tex_height)
{
	cgGLSetStateMatrixParameter(_cgpModelViewProj, CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);
	cgGLSetParameter2f(_cgpVideoSize, width, height);
	cgGLSetParameter2f(_cgpTextureSize, tex_width, tex_height);
	cgGLSetParameter2f(_cgpOutputSize, _cgViewWidth, _cgViewHeight);
}

void PS3Graphics::Draw(int width, int height, uint16_t* screen)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glBufferSubData(GL_TEXTURE_REFERENCE_BUFFER_SCE, 0, width * height * 2, screen);
	glTextureReferenceSCE(GL_TEXTURE_2D, 1, width, height, 0, GL_RGB5_A1, width*2, 0);
	UpdateCgParams(width, height, width, height);
	glDrawArrays(GL_QUADS, 0, 4); 
	DrawHUD();
	last_redraw = sys_time_get_system_time();
}

void PS3Graphics::DrawHUD()
{
	static uint8_t  intensity;
	static uint32_t led_color;
	int enabled;
	float yPos=0.00f;
	float ybrk=0.03f;

	if (ui.display_drives)
	{
		resources_get_int("Drive8Type", &enabled);
		if (enabled)
		{
			intensity = ((ui.drive8.led_pwm1 * 0xff) / 1000) & 0xff;
			if (intensity > 0)
			{
				yPos+=ybrk;
				led_color = (ui.drive8.led_color == DRIVE_ACTIVE_RED) ? (0xff000000 | intensity) : (0xff000000 | (intensity << 8));
				cellDbgFontPrintf(0.85f, yPos, Emulator_GetFontSize(), led_color,  "D8  TRK %u", ui.drive8.half_track_number/2);
			}
		}

		resources_get_int("Drive9Type", &enabled);
		if (enabled)
		{
			if (intensity > 0)
			{
				yPos+=ybrk;
				led_color = (ui.drive9.led_color == DRIVE_ACTIVE_RED) ? (0xff000000 | intensity) : (0xff000000 | (intensity << 8));
				cellDbgFontPrintf(0.85f, yPos, Emulator_GetFontSize(), led_color,  "D9  TRK %u", ui.drive9.half_track_number/2);
			}
		}

		resources_get_int("Drive10Type", &enabled);
		if (enabled)
		{
			if (intensity > 0)
			{
				yPos+=ybrk;
				led_color = (ui.drive10.led_color == DRIVE_ACTIVE_RED) ? (0xff000000 | intensity) : (0xff000000 | (intensity << 8));
				cellDbgFontPrintf(0.85f, yPos, Emulator_GetFontSize(), led_color,  "D10 TRK %u", ui.drive10.half_track_number/2);
			}
		}
		resources_get_int("Drive11Type", &enabled);
		if (enabled)
		{
			if (intensity > 0) {
				yPos+=ybrk;
				led_color = (ui.drive11.led_color == DRIVE_ACTIVE_RED) ? (0xff000000 | intensity) : (0xff000000 | (intensity << 8));
				cellDbgFontPrintf(0.85f, yPos, Emulator_GetFontSize(), led_color,  "D11 TRK %u", ui.drive11.half_track_number/2);
			}
		}
	}

	if (ui.tape.status)
	{
		if (tape_get_file_name() != NULL)
		{
			yPos+=ybrk;
			cellDbgFontPrintf  (0.80f, yPos, Emulator_GetFontSize(), (ui.tape.motor) ? GREEN : PURPLE,  "TAPE %d", ui.tape.counter);

			switch (ui.tape.control)
			{
				case DATASETTE_CONTROL_STOP:
					cellDbgFontPuts(0.80f, yPos, Emulator_GetFontSize(), PURPLE,  "         STOP");
					break;
				case DATASETTE_CONTROL_START:
					cellDbgFontPuts(0.80f, yPos, Emulator_GetFontSize(), PURPLE,  "         PLAY");
					break;
				case DATASETTE_CONTROL_FORWARD:
					cellDbgFontPuts(0.80f, yPos, Emulator_GetFontSize(), PURPLE,  "         FWD");
					break;
				case DATASETTE_CONTROL_REWIND:
					cellDbgFontPuts(0.80f, yPos, Emulator_GetFontSize(), PURPLE,  "         REW");
					break;
				case DATASETTE_CONTROL_RECORD:
					cellDbgFontPuts(0.80f, yPos, Emulator_GetFontSize(), PURPLE,  "         REC");
					break;
				case DATASETTE_CONTROL_RESET:
					cellDbgFontPuts(0.80f, yPos, Emulator_GetFontSize(), PURPLE,  "         RESET");
					break;
				case DATASETTE_CONTROL_RESET_COUNTER:
					cellDbgFontPuts(0.80f, yPos, Emulator_GetFontSize(), PURPLE,  "         RESETCNT");
					break;
			}
		}
	}

	if (ui.display_speed)
	{
		cellDbgFontPrintf(0.09f, 0.92f, Emulator_GetFontSize(), PURPLE, "Speed = %f, FPS = %f, Warp = %s", ui.speed, ui.frame_rate, (ui.warp_enabled == 1) ? "On" : "Off");
	}

	cellDbgFontDraw();
}

void PS3Graphics::Refresh()
{
	// Is this sufficient for a callback redraw?

	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_QUADS, 0, 4); 
	DrawHUD();
	psglSwap();
	last_redraw = sys_time_get_system_time();
}

/******************************************************************************* 
	Resolution functions
********************************************************************************/

void PS3Graphics::ChangeResolution(uint32_t resId, uint16_t pal60Hz)
{
	cellDbgFontExit();
	PSGLDeInitDevice();
	
	PSGLInitDevice(resId, pal60Hz);
	PSGLInit();
	InitDbgFont();
	SetResolution();
}

int PS3Graphics::CheckResolution(uint32_t resId)
{
	return cellVideoOutGetResolutionAvailability(CELL_VIDEO_OUT_PRIMARY, resId, \
	CELL_VIDEO_OUT_ASPECT_AUTO,0);
}

void PS3Graphics::NextResolution()
{
	if(m_currentResolutionPos+1 < m_supportedResolutions.size())
	{
		m_currentResolutionPos++;
		m_currentResolutionId = m_supportedResolutions[m_currentResolutionPos];
	}
}

void PS3Graphics::PreviousResolution()
{
	if(m_currentResolutionPos > 0)
	{
		m_currentResolutionPos--;
		m_currentResolutionId = m_supportedResolutions[m_currentResolutionPos];
	}
}

void PS3Graphics::SwitchResolution(uint32_t resId, uint16_t pal60Hz)
{
	if(CheckResolution(resId))
	{
		ChangeResolution(resId, pal60Hz);
	}
}

/******************************************************************************* 
	Cg
********************************************************************************/

int32_t PS3Graphics::InitCg()
{
	cgRTCgcInit();

	_cgContext = cgCreateContext();
	if (_cgContext == NULL)
		return 1;
	if (strlen(_curFragmentShaderPath.c_str()) > 0)
		return LoadFragmentShader(_curFragmentShaderPath.c_str());
	else
	{
		_curFragmentShaderPath = DEFAULT_SHADER_FILE;
		return LoadFragmentShader(_curFragmentShaderPath.c_str());
	}
}

static CGprogram LoadShaderFromSource(CGcontext cgtx, CGprofile target, const char* filename, const char *entry)
{
	const char* args[] = { "-fastmath", "-unroll=all", "-ifcvt=all", 0 };
	CGprogram id = cgCreateProgramFromFile(cgtx, CG_SOURCE, filename, target, entry, NULL);

	cgGLLoadProgram(id);

	return id;
}

int32_t PS3Graphics::LoadFragmentShader(std::string shaderPath)
{
	// store the current path
	_curFragmentShaderPath = shaderPath;

	_vertexProgram = LoadShaderFromSource(_cgContext, CG_PROFILE_SCE_VP_RSX, shaderPath.c_str(), "main_vertex");
	_fragmentProgram = LoadShaderFromSource(_cgContext, CG_PROFILE_SCE_FP_RSX, shaderPath.c_str(), "main_fragment");

	// bind and enable the vertex and fragment programs
	cgGLEnableProfile(CG_PROFILE_SCE_VP_RSX);
	cgGLEnableProfile(CG_PROFILE_SCE_FP_RSX);
	cgGLBindProgram(_vertexProgram);
	cgGLBindProgram(_fragmentProgram);

	// acquire mvp param from vertex shader
	_cgpModelViewProj = cgGetNamedParameter(_vertexProgram, "modelViewProj");

	_cgpVideoSize = cgGetNamedParameter(_fragmentProgram, "IN.video_size");
	_cgpTextureSize = cgGetNamedParameter(_fragmentProgram, "IN.texture_size");
	_cgpOutputSize = cgGetNamedParameter(_fragmentProgram, "IN.output_size");

	if (active_shader)
		lib_free(active_shader);

	active_shader = util_concat(shaderPath.c_str(), NULL);

	return CELL_OK;
}

/******************************************************************************* 
	Get functions
********************************************************************************/

uint32_t PS3Graphics::GetInitialResolution()
{
	return m_initialResolution;
}

uint32_t PS3Graphics::GetPAL60Hz()
{
	return m_pal60Hz;
}

void PS3Graphics::GetAllAvailableResolutions()
{
	bool defaultresolution = true;
	uint32_t videomode[] = {
		CELL_VIDEO_OUT_RESOLUTION_480, CELL_VIDEO_OUT_RESOLUTION_576,
		CELL_VIDEO_OUT_RESOLUTION_960x1080, CELL_VIDEO_OUT_RESOLUTION_720,
		CELL_VIDEO_OUT_RESOLUTION_1280x1080, CELL_VIDEO_OUT_RESOLUTION_1440x1080,
		CELL_VIDEO_OUT_RESOLUTION_1600x1080, CELL_VIDEO_OUT_RESOLUTION_1080};

	// Provide future expandability of the videomode array
	uint16_t num_videomodes = sizeof(videomode)/sizeof(uint32_t);
	for (int i=0; i < num_videomodes; i++)
	{
		if (cellVideoOutGetResolutionAvailability(CELL_VIDEO_OUT_PRIMARY, videomode[i], CELL_VIDEO_OUT_ASPECT_AUTO,0))
		{
			m_supportedResolutions.push_back(videomode[i]);
			m_initialResolution = videomode[i];

			if (m_currentResolutionId == videomode[i])
			{
				defaultresolution = false;
				m_currentResolutionPos = m_supportedResolutions.size()-1;
			}
		}
	}

	// In case we didn't specify a resolution - make the last resolution
	// that was added to the list (the highest resolution) the default resolution
	if (m_currentResolutionPos > num_videomodes | defaultresolution)
		m_currentResolutionPos = m_supportedResolutions.size()-1;
}

uint32_t PS3Graphics::GetCurrentResolution()
{
	return m_supportedResolutions[m_currentResolutionPos];
}

CellVideoOutState PS3Graphics::GetVideoOutState()
{
	return m_stored_video_state;
}

/******************************************************************************* 
	Set functions
********************************************************************************/

void PS3Graphics::SetViewports()
{
	float device_aspect = psglGetDeviceAspectRatio(psgl_device);
	GLuint width = gl_width;
	GLuint height = gl_height;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// calculate the glOrtho matrix needed to transform the texture to the desired aspect ratio
	float desired_aspect = m_ratio;

	GLuint real_width = width, real_height = height;

	// If the aspect ratios of screen and desired aspect ratio are sufficiently equal (floating point stuff), 
	// assume they are actually equal.
	if ( (int)(device_aspect*1000) > (int)(desired_aspect*1000) )
	{
		float delta = (desired_aspect / device_aspect - 1.0) / 2.0 + 0.5;
		glViewport(width * (0.5 - delta), 0, 2.0 * width * delta, height);
		real_width = (int)(2.0 * width * delta);
	}

	else if ( (int)(device_aspect*1000) < (int)(desired_aspect*1000) )
	{
		float delta = (device_aspect / desired_aspect - 1.0) / 2.0 + 0.5;
		glViewport(0, height * (0.5 - delta), width, 2.0 * height * delta);
		real_height = (int)(2.0 * height * delta);
	}
	else
		glViewport(0, 0, width, height);

	if (m_overscan)
		glOrthof(-m_overscan_amount/2, 1 + m_overscan_amount/2, -m_overscan_amount/2, 1 + m_overscan_amount/2, -1, 1);
	else
		glOrthof(0, 1, 0, 1, -1, 1);

	_cgViewWidth = real_width;
	_cgViewHeight = real_height;

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void PS3Graphics::SetResolution()
{
	cellVideoOutGetState(CELL_VIDEO_OUT_PRIMARY, 0, &m_stored_video_state);
}

void PS3Graphics::SetOverscan(bool will_overscan, float amount)
{
	m_overscan_amount = amount;
	m_overscan = will_overscan;
	SetViewports();
}

void PS3Graphics::SetPAL60Hz(bool pal60Hz)
{
	m_pal60Hz = pal60Hz;
}

void PS3Graphics::SetSmooth(bool smooth)
{
	m_smooth = smooth;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_smooth ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_smooth ? GL_LINEAR : GL_NEAREST);
}

void PS3Graphics::SetAspectRatio(bool keep_aspect)
{
	if (keep_aspect)
		m_ratio = SCREEN_4_3_ASPECT_RATIO;
	else
		m_ratio = SCREEN_16_9_ASPECT_RATIO;
	SetViewports();
}
