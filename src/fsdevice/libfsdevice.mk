
include common.mk

PPU_SRCS	=	fsdevice/fsdevice-close.c fsdevice/fsdevice-cmdline-options.c fsdevice/fsdevice-flush.c fsdevice/fsdevice-open.c fsdevice/fsdevice-read.c fsdevice/fsdevice-resources.c fsdevice/fsdevice-write.c fsdevice/fsdevice.c


PPU_LIB_TARGET	=	libfsdevice.ppu.a

include $(CELL_MK_DIR)/sdk.target.mk
