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


#ifndef PS3VIDEO_H_
#define PS3VIDEO_H_

#include <sysutil/sysutil_sysparam.h>
#include "cellframework/graphics/PSGLGraphics.h"
 
#define DEFAULT_SHADER_FILE VICE_USRDIR "shaders/stock.cg"

#define SCREEN_16_9_ASPECT_RATIO (16.0/9)
#define SCREEN_4_3_ASPECT_RATIO (4.0/3)

#define SCREEN_REAL_ASPECT_RATIO SCREEN_4_3_ASPECT_RATIO

class PS3Graphics : public PSGLGraphics
{
	public:
		PS3Graphics();
		~PS3Graphics();

		void Draw(int width, int height, uint16_t* screen, uint16_t* overlay);
		void Init();
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
		int32_t PSGLReInit(int width, int height, int depth);
	private:
		int32_t PSGLInit();
		void DrawHUD();
		GLuint vbo[2];
		bool overscan;

		bool m_overscan;
		float m_overscan_amount;

		float m_ratio;
		bool m_smooth;
		bool m_pal60Hz;

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
