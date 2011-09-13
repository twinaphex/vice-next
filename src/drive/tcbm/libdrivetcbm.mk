
include common.mk


PPU_SRCS	=	drive/tcbm/glue1551.c drive/tcbm/mem1551.c drive/tcbm/tcbm-resources.c drive/tcbm/tpid.c drive/tcbm/tcbm.c drive/tcbm/tcbm-cmdline-options.c drive/tcbm/tcbmrom.c

PPU_LIB_TARGET	=	libdrivetcbm.ppu.a

include $(CELL_MK_DIR)/sdk.target.mk
