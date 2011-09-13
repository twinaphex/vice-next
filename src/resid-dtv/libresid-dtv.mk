
include common.mk


PPU_SRCS	= 	resid-dtv/envelope.cc resid-dtv/extfilt.cc resid-dtv/filter.cc resid-dtv/sid.cc resid-dtv/version.cc resid-dtv/voice.cc resid-dtv/wave.cc


PPU_LIB_TARGET	=	libresid-dtv.ppu.a

include $(CELL_MK_DIR)/sdk.target.mk

