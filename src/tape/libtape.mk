
include common.mk

PPU_SRCS	=	tape/t64.c tape/tap.c tape/tape-internal.c tape/tape-snapshot.c tape/tape.c tape/tapeimage.c



PPU_LIB_TARGET	=	libtape.ppu.a

include $(CELL_MK_DIR)/sdk.target.mk
