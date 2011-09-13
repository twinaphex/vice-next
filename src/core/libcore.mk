
include common.mk

PPU_SRCS	=	core/ciacore.c core/ciatimer.c core/flash040core.c core/riotcore.c core/t6721.c core/tpicore.c core/viacore.c



PPU_LIB_TARGET	=	libcore.ppu.a

include $(CELL_MK_DIR)/sdk.target.mk
