
include common.mk


PPU_SRCS	=	drive/iec/plus4exp/iec-plus4exp.c  drive/iec/plus4exp/plus4exp-cmdline-options.c  drive/iec/plus4exp/plus4exp-resources.c


PPU_LIB_TARGET	=	libiecplus4.ppu.a

include $(CELL_MK_DIR)/sdk.target.mk
