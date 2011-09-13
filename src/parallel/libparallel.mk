
include common.mk


PPU_SRCS	=	parallel/parallel-trap.c parallel/parallel.c


PPU_LIB_TARGET	=	libparallel.ppu.a

include $(CELL_MK_DIR)/sdk.target.mk
