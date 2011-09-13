
include common.mk


PPU_SRCS	=	vdc/vdc.c vdc/vdc-color.c vdc/vdc-mem.c vdc/vdc-snapshot.c vdc/vdc-cmdline-options.c vdc/vdc-draw.c vdc/vdc-resources.c




PPU_LIB_TARGET	=	libvdc.ppu.a

include $(CELL_MK_DIR)/sdk.target.mk
