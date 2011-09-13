
include common.mk


PPU_SRCS	=	drive/iecieee/iecieee.c drive/iecieee/via2d.c

PPU_LIB_TARGET	=	libiecieee.ppu.a

include $(CELL_MK_DIR)/sdk.target.mk
