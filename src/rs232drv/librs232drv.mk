
include common.mk

PPU_SRCS	=	rs232drv/rs232drv.c rs232drv/rsuser.c


PPU_LIB_TARGET	=	librs232drv.ppu.a

include $(CELL_MK_DIR)/sdk.target.mk
