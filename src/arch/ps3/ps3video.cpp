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

PS3Graphics::PS3Graphics() : PSGLGraphics()
{
	m_smooth = false;
	m_pal60Hz = false;
	m_overscan = false;
	m_overscan_amount = 0.0;
}

PS3Graphics::~PS3Graphics()
{
	Deinit();
}

void PS3Graphics::Deinit()
{
	PSGLGraphics::Deinit();
}

void PS3Graphics::InitDbgFont()
{
	PSGLGraphics::InitDbgFont();
}

void PS3Graphics::SetViewports()
{
	float device_aspect = this->GetDeviceAspectRatio();
	GLuint width = this->GetResolutionWidth();
	GLuint height = this->GetResolutionHeight();

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
		//glOrthof(0.5 - delta, 0.5 + delta, 0, 1, -1, 1);
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
	{
		glViewport(0, 0, width, height);
	}

	if (m_overscan)
	{
		glOrthof(-m_overscan_amount/2, 1 + m_overscan_amount/2, -m_overscan_amount/2, 1 + m_overscan_amount/2, -1, 1);
	}
	else
	{
		glOrthof(0, 1, 0, 1, -1, 1);
	}

	_cgViewWidth = real_width;
	_cgViewHeight = real_height;

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
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

bool PS3Graphics::GetPAL60Hz()
{
	return m_pal60Hz;
}

static unsigned int last_redraw=0;

void PS3Graphics::Refresh()
{
	// Is this sufficient for a callback redraw?

	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_QUADS, 0, 4); 
	DrawHUD();
	glFlush();
	psglSwap();
	last_redraw = sys_time_get_system_time();
}

void PS3Graphics::SetAspectRatio(bool keep_aspect)
{
	if (keep_aspect)
	{
		m_ratio = SCREEN_4_3_ASPECT_RATIO;
	}
	else
	{
		m_ratio = SCREEN_16_9_ASPECT_RATIO;
	}
	SetViewports();
}

unsigned int PS3Graphics::TimeSinceLastDraw()
{
	return abs(sys_time_get_system_time() - last_redraw);
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


static int offset=0;

void PS3Graphics::Draw(int width, int height, uint16_t* screen, uint16_t* overlay)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glBufferSubData(GL_TEXTURE_REFERENCE_BUFFER_SCE, 0, width * height * 2, screen);
	glTextureReferenceSCE(GL_TEXTURE_2D, 1, width, height, 0, GL_RGB5_A1, width*2, 0);
	UpdateCgParams(width, height, width, height);
	glDrawArrays(GL_QUADS, 0, 4); 
	DrawHUD();
	glFlush();
	last_redraw = sys_time_get_system_time();
}

int32_t PS3Graphics::ChangeResolution(uint32_t resId, uint16_t pal60Hz)
{
	int32_t ret;

	PSGLGraphics::DeinitDbgFont();
	Deinit();

	PSGLGraphics::Init(resId, pal60Hz);
	PSGLInit();
	PSGLGraphics::InitDbgFont();
	PSGLGraphics::SetResolution();
}

void PS3Graphics::UpdateCgParams(unsigned width, unsigned height, unsigned tex_width, unsigned tex_height)
{
	cgGLSetStateMatrixParameter(_cgpModelViewProj, CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);
	cgGLSetParameter2f(_cgpVideoSize, width, height);
	cgGLSetParameter2f(_cgpTextureSize, tex_width, tex_height);
	cgGLSetParameter2f(_cgpOutputSize, _cgViewWidth, _cgViewHeight);
}

void PS3Graphics::SetSmooth(bool smooth)
{
	m_smooth = smooth;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_smooth ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_smooth ? GL_LINEAR : GL_NEAREST);
}

CGerror CheckCgError(int line)
{
	CGerror err = cgGetError();

	return err;
}

CGprogram LoadShaderFromFile(CGcontext cgtx, CGprofile target, const char* filename, const char *entry)
{
	CGprogram id = cgCreateProgramFromFile(cgtx, CG_BINARY, filename, target, entry, NULL);
	if(!id)
	{
		CheckCgError(__LINE__);
	}

	return id;
}

CGprogram LoadShaderFromSource(CGcontext cgtx, CGprofile target, const char* filename, const char *entry)
{
	CGprogram id = cgCreateProgramFromFile(cgtx, CG_SOURCE, filename, target, entry, NULL);
	if(!id)
	{
		CheckCgError(__LINE__);
	}

	return id;
}

static const char* active_shader=NULL;

const char* PS3Graphics::GetCurrentShader()
{
	return active_shader;
}


int32_t PS3Graphics::InitCg()
{
	cgRTCgcInit();

	_cgContext = cgCreateContext();
	if (_cgContext == NULL)
	{
		return 1;
	}
	if (strlen(_curFragmentShaderPath.c_str()) > 0)
	{
		return LoadFragmentShader(_curFragmentShaderPath.c_str());
	}
	else
	{
		_curFragmentShaderPath = DEFAULT_SHADER_FILE;
		return LoadFragmentShader(_curFragmentShaderPath.c_str());
	}
}

int32_t PS3Graphics::LoadFragmentShader(std::string shaderPath)
{

	// store the cur path
	_curFragmentShaderPath = shaderPath;

	_vertexProgram = LoadShaderFromSource(_cgContext, CG_PROFILE_SCE_VP_RSX, shaderPath.c_str(), "main_vertex");
	if (_vertexProgram <= 0)
	{
		return 1;
	}

	_fragmentProgram = LoadShaderFromSource(_cgContext, CG_PROFILE_SCE_FP_RSX, shaderPath.c_str(), "main_fragment");
	if (_fragmentProgram <= 0)
	{
		return 1;
	}

	// bind and enable the vertex and fragment programs
	cgGLEnableProfile(CG_PROFILE_SCE_VP_RSX);
	cgGLEnableProfile(CG_PROFILE_SCE_FP_RSX);
	cgGLBindProgram(_vertexProgram);
	cgGLBindProgram(_fragmentProgram);

	// acquire mvp param from v shader
	_cgpModelViewProj = cgGetNamedParameter(_vertexProgram, "modelViewProj");
	if (CheckCgError (__LINE__) != CG_NO_ERROR)
	{
		// FIXME: WHY DOES THIS GIVE ERROR ON OTHER LOADS
		// return 1;
	}

	_cgpVideoSize = cgGetNamedParameter(_fragmentProgram, "IN.video_size");
	_cgpTextureSize = cgGetNamedParameter(_fragmentProgram, "IN.texture_size");
	_cgpOutputSize = cgGetNamedParameter(_fragmentProgram, "IN.output_size");

	if (active_shader)
		lib_free(active_shader);

	active_shader = util_concat(shaderPath.c_str(), NULL);
	return CELL_OK;
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
	context_height = SCREEN_RENDER_TEXTURE_HEIGHT;;

	uint32_t ret = InitCg();
	#ifdef CELL_DEBUG
	if (ret != CELL_OK)
	{
		printf("Failed to InitCg: %d", __LINE__);
	}
	#endif

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

void PS3Graphics::Init()
{
	PSGLGraphics::Init(NULL, m_pal60Hz);
	int32_t ret = PSGLInit();

	if (ret == CELL_OK)
	{
		PSGLGraphics::SetResolution();
	}
	PSGLGraphics::GetAllAvailableResolutions();

	// TODO : Initially set to false. This will be overridden later
	SetAspectRatio(false);
}

static unsigned char *pPixels;

void PS3Graphics::ScreenDump (void)
{
	pPixels = new unsigned char[context_width * context_height * 2];

	unsigned char *buffer = (unsigned char *) glMapBuffer(GL_TEXTURE_REFERENCE_BUFFER_SCE, GL_READ_ONLY);
	if (buffer != NULL)
	{
		memcpy (pPixels, buffer, context_width * context_height * 2);
	}
	glUnmapBuffer(GL_TEXTURE_REFERENCE_BUFFER_SCE);
}


void PS3Graphics::DumpScreen (void)
{
	Draw (context_width, context_height, (std::uint16_t*) pPixels, NULL);
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

int PS3Graphics::ContextHeight (void)
{
	return context_height;
}

int PS3Graphics::ContextWidth (void)
{
	return context_width;
}
