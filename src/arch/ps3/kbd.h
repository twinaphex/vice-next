/******************************************************************************* 
 *  -- kbd.h - Keyboard driver for Playstation 3
 *
 *     VICE PS3 -   Commodore 64 emulator for the Playstation 3
 *                  ported from the original VICE distribution
 *                  located at http://sourceforge.net/projects/vice-emu/
 *
 *
 *  Copyright (C) 2010
 *       TimRex
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

#ifndef VICE_KBD_PS3_H
#define VICE_KBD_PS3_H

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

extern void kbd_arch_init(void);

extern signed long kbd_arch_keyname_to_keynum(char *keyname);
extern const char *kbd_arch_keynum_to_keyname(signed long keynum);
extern void kbd_initialize_numpad_joykeys(int *joykeys);

#define KBD_C64_SYM_US  "ps3_sym.vkm"
#define KBD_C64_SYM_DE  "ps3_sym_de.vkm"
#define KBD_C64_POS     "ps3_pos.vkm"
#define KBD_C128_SYM    "ps3_sym.vkm"
#define KBD_C128_POS    "ps3_pos.vkm"
#define KBD_VIC20_SYM   "ps3_sym.vkm"
#define KBD_VIC20_POS   "ps3_pos.vkm"
#define KBD_PET_SYM_UK  "ps3_buks.vkm"
#define KBD_PET_POS_UK  "ps3_bukp.vkm"
#define KBD_PET_SYM_DE  "ps3_bdes.vkm"
#define KBD_PET_POS_DE  "ps3_bdep.vkm"
#define KBD_PET_SYM_GR  "ps3_bgrs.vkm"
#define KBD_PET_POS_GR  "ps3_bgrp.vkm"
#define KBD_PLUS4_SYM   "ps3_sym.vkm"
#define KBD_PLUS4_POS   "ps3_pos.vkm"
#define KBD_CBM2_SYM_UK "ps3_buks.vkm"
#define KBD_CBM2_POS_UK "ps3_bukp.vkm"
#define KBD_CBM2_SYM_DE "ps3_bdes.vkm"
#define KBD_CBM2_POS_DE "ps3_bdep.vkm"
#define KBD_CBM2_SYM_GR "ps3_bgrs.vkm"
#define KBD_CBM2_POS_GR "ps3_bgrp.vkm"

#define KBD_INDEX_C64_DEFAULT   KBD_INDEX_C64_POS
#define KBD_INDEX_C128_DEFAULT  KBD_INDEX_C128_POS
#define KBD_INDEX_VIC20_DEFAULT KBD_INDEX_VIC20_POS
#define KBD_INDEX_PET_DEFAULT   KBD_INDEX_PET_BUKP
#define KBD_INDEX_PLUS4_DEFAULT KBD_INDEX_PLUS4_POS
#define KBD_INDEX_CBM2_DEFAULT  KBD_INDEX_CBM2_BUKP

/* Keymap definition structure.  */
typedef struct {
    BYTE row;
    BYTE column;
    int vshift;
} keyconv;

extern int osk_active_bufferlen;
extern void osk_kbd_type_key(void);
extern void osk_kbd_append_buffer (char *keystring);
extern void osk_kbd_append_buffer_char (int keycode);
void kbd_process(void);

#ifdef __cplusplus
}
#endif

#endif
