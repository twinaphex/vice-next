
include common.mk

PPU_SRCS	=	fileio/cbmfile.c fileio/fileio.c fileio/p00.c


PPU_LIB_TARGET	=	libfileio.ppu.a

include $(CELL_MK_DIR)/sdk.target.mk
