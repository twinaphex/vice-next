
include common.mk

PPU_SRCS	=	sounddrv/soundps3.cpp


PPU_LIB_TARGET	=	libsounddrv.ppu.a

include $(CELL_MK_DIR)/sdk.target.mk
