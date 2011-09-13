VICEVERSION="2.2"

CELL_SDK ?= /usr/local/cell
CELL_MK_DIR ?= $(CELL_SDK)/samples/mk

include $(CELL_MK_DIR)/sdk.makedef.mk
CELL_HOST_PATH ?= $(CELL_SDK)/host-win32

STRIP = $(CELL_HOST_PATH)/ppu/bin/ppu-lv2-strip

include common_flags.mk

# Required for SDK_3_41
PPU_CFLAGS	+=	-DPS3_SDK_3_41 -DPS3
PPU_CXXFLAGS	+=	-DPS3_SDK_3_41 -DPS3

# Required for SDK_1_92
#PPU_CFLAGS	+=	-DPS3_SDK_1_92 -DPS3
#PPU_CXXFLAGS	+=	-DPS3_SDK_1_92 -DPS3


PPU_CFLAGS	+=	-DPSGL -funroll-loops -DWORDS_BIGENDIAN 
PPU_CXXFLAGS	+=	-DPSGL -funroll-loops -DWORDS_BIGENDIAN 

PPU_INCDIRS 	+= 	-I./arch/ -I./arch/ps3/ -I./arch/ps3/cellframework/ -I./drive/ -I./vdrive/ -I./imagecontents/ -I./lib/zlib/ -I./c64/cart/ -I./sid/ -I./vicii/ -I./tape/ -I./raster/ -I./c64/ -I./rtc/ -I./drive/iecieee/ -I./drive/iec/c64exp/ -I./drive/iec/ -I./parallel -I./monitor -I./sounddrv/ -I./lib/lpng/ -I./c64dtv/ -I./core/ -I./vdc/ -I./vic20/ -I./vic20/cart/ -I./plus4 -I./drive/iec/plus4exp/ -I./
