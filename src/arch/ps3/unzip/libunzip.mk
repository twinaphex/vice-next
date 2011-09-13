
include common.mk

PPU_SRCS	=	arch/ps3/unzip/ioapi.c  arch/ps3/unzip/mztools.c  arch/ps3/unzip/unzip.c  arch/ps3/unzip/zip.c

PPU_LIB_TARGET	=	libunzip.ppu.a


include $(CELL_MK_DIR)/sdk.target.mk
