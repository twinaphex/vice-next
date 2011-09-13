
include common.mk


PPU_SRCS	=	drive/ieee/fdc.c drive/ieee/ieee-cmdline-options.c drive/ieee/ieee-resources.c drive/ieee/ieee.c drive/ieee/ieeerom.c drive/ieee/memieee.c drive/ieee/riot1d.c drive/ieee/riot2d.c drive/ieee/via1d2031.c


PPU_LIB_TARGET	=	libieee.ppu.a

include $(CELL_MK_DIR)/sdk.target.mk
