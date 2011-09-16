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

#ifndef PS3GRAPHICS_H_
#define PS3GRAPHICS_H_

/* System includes */
#include <string>
#include <PSGL/psgl.h>
#include <PSGL/psglu.h>
#include <cell/dbgfont.h>
#ifdef CELL_DEBUG_FPS
#include <sys/sys_time.h>
#endif

#include <vector>
 
#define DEFAULT_SHADER_FILE VICE_USRDIR "shaders/stock.cg"

#define SCREEN_16_9_ASPECT_RATIO (16.0/9)
#define SCREEN_4_3_ASPECT_RATIO (4.0/3)

#define SCREEN_REAL_ASPECT_RATIO SCREEN_4_3_ASPECT_RATIO

enum
{
	ASPECT_RATIO_4_3,
	ASPECT_RATIO_5_4,
	ASPECT_RATIO_8_7,
	ASPECT_RATIO_16_9,
	ASPECT_RATIO_16_10,
	ASPECT_RATIO_16_15,
	ASPECT_RATIO_19_14,
	ASPECT_RATIO_2_1,
	ASPECT_RATIO_3_2,
	ASPECT_RATIO_3_4,
	ASPECT_RATIO_1_1,
	ASPECT_RATIO_AUTO,
	ASPECT_RATIO_CUSTOM
};

#define LAST_ASPECT_RATIO ASPECT_RATIO_CUSTOM

class PS3Graphics
{
	public:
		/* constructor/destructor */
		PS3Graphics();

		~PS3Graphics();

		/* public variables */
		enum menu_type
		{
			TEXTURE_BACKDROP,
			TEXTURE_MENU
		};
		float aspectratios[LAST_ASPECT_RATIO];
		uint32_t aspect_x, aspect_y;
		uint32_t frame_count;

		/* PSGL functions */
		void Init();
		void Deinit();

		/* draw functions */
		void Draw(int width, int height, uint16_t* screen);
		void Refresh();

		/* cg */
		int32_t InitCg();
		int32_t LoadFragmentShader(std::string shaderPath);
		void UpdateCgParams(unsigned width, unsigned height, unsigned tex_width, unsigned tex_height);

		void SetResolution();

		/* get functions */
		int CheckResolution(uint32_t resId);
		uint32_t GetCurrentResolution();
		uint32_t GetInitialResolution();
		uint32_t GetPAL60Hz();
		std::string GetFragmentShaderPath() { return _curFragmentShaderPath; }
		CellVideoOutState GetVideoOutState();

		/* set functions */
		void SetAspectRatio(bool keep_aspect);
		void SetPAL60Hz(bool pal60Hz);
		void SetOverscan(bool overscan, float amount = 0.0);
		void SetSmooth(bool smooth);

		/* libdbgfont */
		void InitDbgFont();

		/* resolution functions */
		void ChangeResolution(uint32_t resId, uint16_t pal60Hz/*, uint16_t tripleBuffering, uint32_t scaleEnabled, uint32_t scaleFactor*/);
		void SwitchResolution(uint32_t resId, uint16_t pal60Hz/*, uint16_t tripleBuffering, uint32_t scaleEnabled, uint32_t scaleFactor*/);
		void NextResolution();
		void PreviousResolution();

		/* uncategorized */
		unsigned int TimeSinceLastDraw(void);
                const char* GetCurrentShader(void);
		void ScreenDump (void);
		void DumpScreen (void);
		void DestroyDump (void);
		unsigned char* RetrieveDump (void);
                int RetrieveDumpSize (void);
		int32_t PSGLReInit(int width, int height, int depth);
	private:
		/* private variables */
		uint32_t fbo_enable;
		uint32_t m_tripleBuffering;
		uint32_t m_overscan;
		uint32_t m_pal60Hz;
		uint32_t m_smooth, m_smooth2;
		uint8_t *decode_buffer;

		uint32_t m_viewport_x, m_viewport_y, m_viewport_width, m_viewport_height;
		uint32_t m_viewport_x_temp, m_viewport_y_temp, m_viewport_width_temp, m_viewport_height_temp, m_delta_temp;
		uint32_t m_vsync;
		int m_calculate_aspect_ratio_before_game_load;
		int m_currentResolutionPos;
		int m_resolutionId;
		uint32_t fbo_scale;
		uint32_t fbo_width, fbo_height;
		uint32_t fbo_vp_width, fbo_vp_height;
		uint32_t m_currentResolutionId;
		uint32_t m_initialResolution;
		float m_overscan_amount;
		float m_ratio;
		GLuint _cgViewWidth;
		GLuint _cgViewHeight;
		GLuint fbo_tex;
		GLuint fbo;
		GLuint gl_width;
		GLuint gl_height;
		GLuint tex;
		GLuint tex_menu;
		GLuint tex_backdrop;
		GLuint vbo[2];
		GLfloat m_left, m_right, m_bottom, m_top, m_zNear, m_zFar;
		std::string _curFragmentShaderPath;
		std::vector<uint32_t> m_supportedResolutions;
		CGcontext _cgContext;
		CGprogram _vertexProgram;
		CGprogram _fragmentProgram;
		CGparameter _cgpModelViewProj;
		CGparameter _cgpVideoSize;
		CGparameter _cgpTextureSize;
		CGparameter _cgpOutputSize;
		CGparameter _cgp_vertex_VideoSize;
		CGparameter _cgp_vertex_TextureSize;
		CGparameter _cgp_vertex_OutputSize;
		CGparameter _cgp_timer;
		CGparameter _cgp_vertex_timer;
		CellDbgFontConsoleId dbg_id;
		PSGLdevice* psgl_device;
		PSGLcontext* psgl_context;
		CellVideoOutState m_stored_video_state;

		short context_width;
		short context_height;

		/* PSGL */
		int32_t PSGLInit();
		void PSGLInitDevice(uint32_t resolutionId = 0, uint16_t pal60Hz = 0/*, uint16_t tripleBuffering = 1*/);
		void PSGLDeInitDevice();

		/* resolution functions */
		void GetAllAvailableResolutions();

		/* draw functions */
		void DrawHUD();

		/* set functions */
		void SetViewports();
};

#endif /* EMULATOR_GRAPHICS_H_ */
