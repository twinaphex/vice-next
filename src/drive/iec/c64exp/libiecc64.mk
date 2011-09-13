
include common.mk


PPU_SRCS	=	drive/iec/c64exp/c64exp-cmdline-options.c drive/iec/c64exp/c64exp-resources.c drive/iec/c64exp/iec-c64exp.c drive/iec/c64exp/mc6821.c drive/iec/c64exp/profdos.c


PPU_LIB_TARGET	=	libiecc64.ppu.a

include $(CELL_MK_DIR)/sdk.target.mk
