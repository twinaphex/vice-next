
include common.mk

PPU_SRCS	=	lib/lpng/png.c lib/lpng/pngerror.c lib/lpng/pngget.c lib/lpng/pngmem.c lib/lpng/pngpread.c lib/lpng/pngread.c lib/lpng/pngrio.c lib/lpng/pngrtran.c lib/lpng/pngrutil.c lib/lpng/pngset.c lib/lpng/pngtrans.c lib/lpng/pngwio.c lib/lpng/pngwrite.c lib/lpng/pngwtran.c lib/lpng/pngwutil.c

PPU_LIB_TARGET	=	libpng.ppu.a

include $(CELL_MK_DIR)/sdk.target.mk
