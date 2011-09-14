#ifndef EMULATOR_H_
#define EMULATOR_H_

#define MAKE_BUTTON(pad, btn) (((pad)<<4)|(btn))

#define AUDIO_BLOCK_COUNT 16
#define AUDIO_CHANNEL_COUNT 2
#define AUDIO_INPUT_RATE (48000)
#define AUDIO_BUFFER_SAMPLES (4096)

#include "cellframework/input/cellInput.h"
#include "ps3video.hpp"
#include "types.h"

extern "C" {
#include "ui.h"
}

void Emulator_RequestLoadROM(const char* rom, bool forceReboot=false, bool warpmode=false);

const char *get_current_rom(void);
void Emulator_SwitchMode(Emulator_Modes);
extern "C" void Emulator_Shutdown(void);
extern "C" Emulator_Modes Emulator_GetMode(void);
void Emulator_StartROMRunning();

extern CellInputFacade* CellInput;
extern PS3Graphics* Graphics;
float Emulator_GetFontSize();


#ifdef __cplusplus
extern "C" {
int menu(Emulator_Modes mode);
void sysutil_callback_redraw(void);
}
#endif

#endif /* EMULATOR_H_ */
