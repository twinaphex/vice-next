
include common.mk


PPU_INCDIRS 	+= 	-I./c64 

PPU_SRCS	= 	c64/c64bus.c \
	c64/c64cia2.c \
	c64/c64datasette.c \
	c64/c64export.c \
	c64/c64gluelogic.c \
	c64/c64iec.c \
	c64/c64io.c \
	c64/c64keyboard.c \
	c64/c64meminit.c \
	c64/c64memrom.c \
	c64/c64printer.c \
	c64/c64pla.c \
	c64/c64parallel.c \
	c64/c64rsuser.c \
	c64/c64sound.c \
	c64/patchrom.c \




PPU_LIB_TARGET	=	libc64c128.ppu.a

include $(CELL_MK_DIR)/sdk.target.mk

#PPU_LIB_TARGETDIR = 	$(OBJS_DIR)/c64/libc64.ppu
#
#$(PPU_LIB_TARGET_DIR)/PPU_LIB_TARGET:	$(PPU_LIB_TARGET)
#        $(RM) -r $(PPU_LIB_TARGET_DIR)/$(PPU_LIB_TARGET)/
#        mkdir -p $(PPU_LIB_TARGET_DIR)/
#        $(CP) $(PPU_LIB_TARGET) $(PPU_LIB_TARGET_DIR)
#        cd $(PPU_LIB_TARGET_DIR) && $(PPU_AR) -x $(PPU_LIB_TARGET)


