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

#include "OSKUtil.h"

#include "sysutil/sysutil_oskdialog.h"
#include "sys/memory.h"

//Just for testing purposes, can be removed later
#define MESSAGE		L"OSK Dialog"
#define INIT_TEXT	L""

// TODO : Modified from 7MB to 3MB. - TimRex - 15-11-10
#define OSK_DIALOG_MEMORY_CONTAINER_SIZE 1024*1024*5

/* mFlags */
#define OSK_IN_USE	(0x00000001)

OSKUtil::OSKUtil()
{
	//m_msg = msg;
	//m_init = init;
	mFlags = 0;
	memset(result_text_buffer, 0, sizeof(*result_text_buffer));
	memset(result_text_buffer_char, 0, 256);
}

OSKUtil::OSKUtil(std::string& msg, std::string& init)
{
	m_msg = msg;
	m_init = init;
	mFlags = 0;
}

OSKUtil::~OSKUtil()
{
}

void OSKUtil::str_to_utf16(uint16_t*& buf, const std::string& out) {}


bool OSKUtil::Init(void)
{
	int ret;
	ret = sys_memory_container_create(&containerid, OSK_DIALOG_MEMORY_CONTAINER_SIZE);

	if(ret != 0)
	{
		return (false);
	}

	return true;
}

bool OSKUtil::Start(const wchar_t* msg, const wchar_t* init)
{
	int ret;
	if (mFlags & OSK_IN_USE)
	{
		return (true);
	}

	inputFieldInfo.message = (uint16_t*)msg;			//Text to be displayed, as guide message, at upper left on the OSK
	inputFieldInfo.init_text = (uint16_t*)init;			//Initial text
	inputFieldInfo.limit_length = CELL_OSKDIALOG_STRING_SIZE;	//Length limitation for input text
	CreateActivationParameters();
	if(!EnableKeyLayout())
	{
		return (false);
	}

	// TODO : Play with this
	//ret = cellOskDialogExtSetBaseColor( float red, float green, float blue, float alpha );

	ret = cellOskDialogLoadAsync(containerid, &dialogParam, &inputFieldInfo);
	if(ret < 0)
	{
		return (false);
	}
	mFlags |= OSK_IN_USE;
	return (true);
}

bool OSKUtil::Abort()
{
	int ret;

	if ((mFlags & OSK_IN_USE) == 0)
	{
		return (false);
	}

	ret = cellOskDialogAbort();
	if (ret < 0)
	{
		return (false);
	}
	return (true);
}

void OSKUtil::Stop()
{
	int ret;
	//osk_callback_data_t data;

	outputInfo.result = CELL_OSKDIALOG_INPUT_FIELD_RESULT_OK;	// Result onscreen keyboard dialog termination
	outputInfo.numCharsResultString = 16;				// Specify number of characters for returned text
	outputInfo.pResultString = (uint16_t *)result_text_buffer;	// Buffer storing returned text

	ret = cellOskDialogUnloadAsync(&outputInfo);

	switch (outputInfo.result)
	{
		int num;

 		case CELL_OSKDIALOG_INPUT_FIELD_RESULT_OK:
			num=wcstombs(result_text_buffer_char, result_text_buffer, 256);
			result_text_buffer_char[num]=0;
			break;

        	case CELL_OSKDIALOG_INPUT_FIELD_RESULT_CANCELED:
        	case CELL_OSKDIALOG_INPUT_FIELD_RESULT_ABORT:
        	case CELL_OSKDIALOG_INPUT_FIELD_RESULT_NO_INPUT_TEXT:

		default:
			// TODO : This was added by TimRex - 15-11-10
			result_text_buffer_char[0]=0;
			break;
	}

	mFlags &= ~OSK_IN_USE;
}

void OSKUtil::Close()
{
	int ret;

	ret = sys_memory_container_destroy(containerid);

}

const char * OSKUtil::OutputString()
{
	return result_text_buffer_char;
}

void OSKUtil::CreateActivationParameters()
{
	// Initial display psition of the OSK (On-Screen Keyboard) dialog [x, y]
	pos.x = 0.0;
	pos.y = 0.0;

	// Set standard position
	int32_t LayoutMode = CELL_OSKDIALOG_LAYOUTMODE_X_ALIGN_CENTER | CELL_OSKDIALOG_LAYOUTMODE_Y_ALIGN_TOP;
	cellOskDialogSetLayoutMode(LayoutMode);

	//Select panels to be used using flags
	// NOTE: We don't need CELL_OSKDIALOG_PANELMODE_JAPANESE_KATAKANA and CELL_OSKDIALOG_PANELMODE_JAPANESE obviously (and Korean), so I'm going to
	// leave that all out	
	dialogParam.allowOskPanelFlg =
						CELL_OSKDIALOG_PANELMODE_ALPHABET |
						CELL_OSKDIALOG_PANELMODE_NUMERAL |
						CELL_OSKDIALOG_PANELMODE_NUMERAL_FULL_WIDTH |
						CELL_OSKDIALOG_PANELMODE_ENGLISH;
	// Panel to display first
	dialogParam.firstViewPanel = CELL_OSKDIALOG_PANELMODE_ALPHABET;
	// Initial display position of the onscreen keyboard dialog
	dialogParam.controlPoint = pos;
	// Prohibited operation flag(s) (ex. CELL_OSKDIALOG_NO_SPACE)
	// dialogParam.prohibitFlgs = 0;

	// TODO  Removed by TimRex - 15-11-10
	//       We need RETURN for the C64 emulator.
	//	 Any other keys we can add would be good also.  Custom keys? Custom strings/commands?
	//dialogParam.prohibitFlgs = CELL_OSKDIALOG_NO_RETURN;
}
bool OSKUtil::EnableKeyLayout()
{
	int ret;
	ret = cellOskDialogSetKeyLayoutOption(CELL_OSKDIALOG_10KEY_PANEL | CELL_OSKDIALOG_FULLKEY_PANEL);
	if (ret < 0)
	{
		return (false);
	}
	return (true);
}

uint64_t OSKUtil::getString(std::string& out) {}
