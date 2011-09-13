
include common.mk


PPU_INCDIRS 	+= 	-I./c64 

PPU_SRCS	= 	raster/raster-cache.c raster/raster-canvas.c raster/raster-changes.c raster/raster-cmdline-options.c raster/raster-line-changes-sprite.c raster/raster-line-changes.c raster/raster-line.c raster/raster-modes.c raster/raster-resources.c raster/raster-sprite.c raster/raster-sprite-status.c raster/raster-sprite-cache.c raster/raster.c


PPU_LIB_TARGET	=	libraster.ppu.a

include $(CELL_MK_DIR)/sdk.target.mk
