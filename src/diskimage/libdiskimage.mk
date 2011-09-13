
include common.mk

PPU_SRCS	=	diskimage/diskimage.c diskimage/fsimage-check.c diskimage/fsimage-create.c diskimage/fsimage-gcr.c diskimage/fsimage-probe.c diskimage/fsimage.c




PPU_LIB_TARGET	=	libdiskimage.ppu.a

include $(CELL_MK_DIR)/sdk.target.mk
