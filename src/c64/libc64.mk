
include common.mk


PPU_INCDIRS 	+= 	-I./c64 

PPU_SRCS	= 	c64/c64-cmdline-options.c \
	c64/c64-resources.c \
	c64/c64-snapshot.c \
	c64/c64.c \
	c64/c64_256k.c \
	c64/c64bus.c \
	c64/c64cia1.c \
	c64/c64cia2.c \
	c64/c64datasette.c \
	c64/c64drive.c \
	c64/c64export.c \
	c64/c64fastiec.c \
	c64/c64gluelogic.c \
	c64/c64iec.c \
	c64/c64io.c \
	c64/c64keyboard.c \
	c64/c64mem.c \
	c64/c64meminit.c \
	c64/c64memlimit.c \
	c64/c64memrom.c \
	c64/c64memsnapshot.c \
	c64/c64model.c \
	c64/c64parallel.c \
	c64/c64pla.c \
	c64/c64printer.c \
	c64/c64rom.c \
	c64/c64romset.c \
	c64/c64rsuser.c \
	c64/c64sound.c \
	c64/c64video.c \
	c64/patchrom.c \
	c64/plus256k.c \
	c64/plus60k.c \
	c64/psid.c \
	c64/psiddrv.a65 \
	c64/reloc65.c \
	c64/c64embedded.c 

PPU_LIB_TARGET	=	libc64.ppu.a

include $(CELL_MK_DIR)/sdk.target.mk

#PPU_LIB_TARGETDIR = 	$(OBJS_DIR)/c64/libc64.ppu
#
#$(PPU_LIB_TARGET_DIR)/PPU_LIB_TARGET:	$(PPU_LIB_TARGET)
#        $(RM) -r $(PPU_LIB_TARGET_DIR)/$(PPU_LIB_TARGET)/
#        mkdir -p $(PPU_LIB_TARGET_DIR)/
#        $(CP) $(PPU_LIB_TARGET) $(PPU_LIB_TARGET_DIR)
#        cd $(PPU_LIB_TARGET_DIR) && $(PPU_AR) -x $(PPU_LIB_TARGET)


