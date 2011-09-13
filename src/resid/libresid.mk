
include common.mk

PPU_CFLAGS	+= -DVERSION=\"0.16\" -Wall -O2 -funroll-loops -fomit-frame-pointer -fno-exceptions -maltivec -mabi=altivec
PPU_CXXFLAGS	+= -DVERSION=\"0.16\" -Wall -O2 -funroll-loops -fomit-frame-pointer -fno-exceptions -maltivec -mabi=altivec

PPU_SRCS	= 	resid/envelope.cc resid/pot.cc resid/voice.cc resid/wave6581_P_T.cc resid/wave8580_PST.cc resid/wave.cc resid/extfilt.cc resid/sid.cc resid/wave6581_PS_.cc resid/wave6581__ST.cc resid/wave8580_P_T.cc resid/filter.cc resid/version.cc resid/wave6581_PST.cc resid/wave8580_PS_.cc resid/wave8580__ST.cc


PPU_LIB_TARGET	=	libresid.ppu.a

include $(CELL_MK_DIR)/sdk.target.mk

