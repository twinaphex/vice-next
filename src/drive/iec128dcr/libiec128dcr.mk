
include common.mk


PPU_SRCS	=	drive/iec128dcr/iec128dcr.c drive/iec128dcr/iec128dcr-cmdline-options.c drive/iec128dcr/iec128dcr-resources.c drive/iec128dcr/iec128dcrrom.c


PPU_LIB_TARGET	=	libiec128dcr.ppu.a

include $(CELL_MK_DIR)/sdk.target.mk
