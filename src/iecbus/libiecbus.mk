
include common.mk


PPU_SRCS	=	iecbus/iecbus.c

PPU_LIB_TARGET	=	libiecbus.ppu.a

include $(CELL_MK_DIR)/sdk.target.mk
