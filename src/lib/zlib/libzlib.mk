
include common.mk

PPU_SRCS	+=	lib/zlib/inflate.c lib/zlib/adler32.c lib/zlib/compress.c lib/zlib/crc32.c lib/zlib/crc32.h lib/zlib/deflate.c lib/zlib/deflate.h lib/zlib/gzio.c lib/zlib/infback.c lib/zlib/inffast.c lib/zlib/inffast.h lib/zlib/inffixed.h lib/zlib/inflate.h lib/zlib/inftrees.c lib/zlib/inftrees.h lib/zlib/trees.c lib/zlib/trees.h lib/zlib/uncompr.c lib/zlib/zconf.h lib/zlib/zlib.h lib/zlib/zutil.c lib/zlib/zutil.h

PPU_LIB_TARGET	=	libzlib.ppu.a

include $(CELL_MK_DIR)/sdk.target.mk
