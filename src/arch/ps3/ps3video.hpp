/******************************************************************************* 
 *  -- menu.h - Menu interface for Vice Playstation 3
 *
 *     VICE PS3 -   Commodore 64 emulator for the Playstation 3
 *                  ported from the original VICE distribution
 *                  located at http://sourceforge.net/projects/vice-emu/
 *
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


#ifndef EMULATOR_GRAPHICS_H_
#define EMULATOR_GRAPHICS_H_

#include <sysutil/sysutil_sysparam.h>
#include "cellframework/graphics/PSGLGraphics.h"
 


#define DEFAULT_SHADER_FILE VICE_USRDIR "shaders/stock.cg"

//#define EMULATOR_ASPECT_RATIO_16_9 0;
//#define EMULATOR_ASPECT_RATIO_4_3 1;

#define SCREEN_16_9_ASPECT_RATIO (16.0/9)
#define SCREEN_4_3_ASPECT_RATIO (4.0/3)


#define SCREEN_REAL_ASPECT_RATIO SCREEN_4_3_ASPECT_RATIO


typedef struct _Vertex
{
	float x;
	float y;
	float z;
} Vertex;

typedef struct _TextureCoord
{
	float u;
	float v;
} TextureCoord;

typedef struct _Quad
{
	Vertex v1;
	Vertex v2;
	Vertex v3;
	Vertex v4;
	TextureCoord t1;
	TextureCoord t2;
	TextureCoord t3;
	TextureCoord t4;
} Quad;

class PS3Graphics : public PSGLGraphics
{
	public:
		PS3Graphics();
		~PS3Graphics();

		void Draw(int width, int height, uint16_t* screen, uint16_t* overlay);
		void Init();
                int32_t PSGLReInit(int width, int height, int depth);
		void Deinit();
		void Refresh();
		unsigned int TimeSinceLastDraw(void);
		int32_t ChangeResolution(uint32_t resId, uint16_t pal60Hz);
		void SetOverscan(bool overscan, float amount = 0.0);
		int32_t InitCg();
		int32_t LoadFragmentShader(std::string shaderPath);
                const char* GetCurrentShader(void);
		void UpdateCgParams(unsigned width, unsigned height, unsigned tex_width, unsigned tex_height);

		void SetAspectRatio(bool keep_aspect);
		void InitScreenQuad(int width, int height);
		void SetSmooth(bool smooth);
		void SetPAL60Hz(bool pal60Hz);
		bool GetPAL60Hz();

		void InitDbgFont();
		std::string GetFragmentShaderPath() { return _curFragmentShaderPath; }
		void ScreenDump (void);
		void DumpScreen (void);
		void DestroyDump (void);
		unsigned char* RetrieveDump (void);
                int RetrieveDumpSize (void);
                int ContextWidth (void);
                int ContextHeight (void);

	private:
		int32_t PSGLInit();
		void DrawHUD();
		Quad screenQuad;
		GLuint vbo[2];
		bool overscan;

		bool m_overscan;
		float m_overscan_amount;

		float m_ratio;
		bool m_smooth;
		bool m_pal60Hz;

		uint8_t *gl_main_buffer;
		uint8_t *vertex_buf;

		std::string _curFragmentShaderPath;

		CGcontext _cgContext;

		CGprogram _vertexProgram;
		CGprogram _fragmentProgram;

		CGparameter _cgpModelViewProj;

		short context_width;
		short context_height;


		CGparameter _cgpVideoSize;
		CGparameter _cgpTextureSize;
		CGparameter _cgpOutputSize;

		GLuint _cgViewWidth;
		GLuint _cgViewHeight;

		void SetViewports();
};

#endif /* EMULATOR_GRAPHICS_H_ */
