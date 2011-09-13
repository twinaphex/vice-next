
include common.mk


PPU_SRCS	= 	vic20/cart/finalexpansion.c vic20/cart/generic.h vic20/cart/megacart.h vic20/cart/vic20cartmem.c vic20/cart/vic-fp.h vic20/cart/finalexpansion.h vic20/cart/vic20cart.c vic20/cart/vic20cartmem.h vic20/cart/generic.c vic20/cart/megacart.c vic20/cart/vic20cart.h vic20/cart/vic-fp.c



PPU_LIB_TARGET	=	libvic20cart.ppu.a

include $(CELL_MK_DIR)/sdk.target.mk

#PPU_LIB_TARGETDIR = 	$(OBJS_DIR)/c64/libc64.ppu
#
#$(PPU_LIB_TARGET_DIR)/PPU_LIB_TARGET:	$(PPU_LIB_TARGET)
#        $(RM) -r $(PPU_LIB_TARGET_DIR)/$(PPU_LIB_TARGET)/
#        mkdir -p $(PPU_LIB_TARGET_DIR)/
#        $(CP) $(PPU_LIB_TARGET) $(PPU_LIB_TARGET_DIR)
#        cd $(PPU_LIB_TARGET_DIR) && $(PPU_AR) -x $(PPU_LIB_TARGET)


