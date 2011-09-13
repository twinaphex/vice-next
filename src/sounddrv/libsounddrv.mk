
include common.mk

PPU_SRCS	=	sounddrv/soundaiff.c sounddrv/sounddummy.c sounddrv/sounddump.c sounddrv/soundfs.c sounddrv/soundiff.c sounddrv/soundvoc.c sounddrv/soundwav.c sounddrv/soundps3.cpp sounddrv/soundmovie.c


PPU_LIB_TARGET	=	libsounddrv.ppu.a

include $(CELL_MK_DIR)/sdk.target.mk
