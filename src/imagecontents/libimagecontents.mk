
include common.mk

PPU_SRCS	=	imagecontents/diskcontents-block.c imagecontents/diskcontents-iec.c imagecontents/diskcontents.c imagecontents/imagecontents.c imagecontents/tapecontents.c



PPU_LIB_TARGET	=	libimagecontents.ppu.a

include $(CELL_MK_DIR)/sdk.target.mk
