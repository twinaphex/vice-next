
include common.mk

PPU_SRCS	=	gfxoutputdrv/gfxoutput.c gfxoutputdrv/ppmdrv.c gfxoutputdrv/doodledrv.c

PPU_LIB_TARGET	=	libgfxoutputdrv.ppu.a

include $(CELL_MK_DIR)/sdk.target.mk
