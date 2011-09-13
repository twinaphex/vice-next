
include common.mk


PPU_SRCS	=	drive/iec/cia1571d.c drive/iec/cia1581d.c drive/iec/glue1571.c drive/iec/iec-cmdline-options.c drive/iec/iec-resources.c drive/iec/iec.c drive/iec/iecrom.c drive/iec/memiec.c drive/iec/via1d1541.c drive/iec/wd1770.c

PPU_LIB_TARGET	=	libiec.ppu.a

include $(CELL_MK_DIR)/sdk.target.mk
