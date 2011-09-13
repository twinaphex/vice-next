
include common.mk

PPU_CFLAGS	+= -DVERSION=\"0.16\" -Wall -O2 -funroll-loops -fomit-frame-pointer -fno-exceptions -maltivec -mabi=altivec
PPU_CXXFLAGS	+= -DVERSION=\"0.16\" -Wall -O2 -funroll-loops -fomit-frame-pointer -fno-exceptions -maltivec -mabi=altivec

PPU_SRCS	= 	resid-fp/convolve.cc resid-fp/envelope.cc resid-fp/filter.cc resid-fp/sid.cc resid-fp/voice.cc resid-fp/convolve-sse.cc resid-fp/extfilt.cc resid-fp/pot.cc resid-fp/version.cc resid-fp/wave.cc resid-fp/convolve-altivec.cc

PPU_LIB_TARGET	=	libresid-fp.ppu.a

include $(CELL_MK_DIR)/sdk.target.mk

