
include common.mk


PPU_SRCS	= 	vic20/vic20bus.c vic20/vic20embedded.c vic20/vic20memsnapshot.c vic20/vic20rsuser.c vic20/vic.c vic20/vic-resources.c vic20/vic20.c vic20/vic20iec.c vic20/vic20-midi.c vic20/vic20-snapshot.c vic20/vic-cmdline-options.c vic20/vic-snapshot.c vic20/vic20-cmdline-options.c vic20/vic20ieeevia1.c vic20/vic20printer.c vic20/vic20sound.c vic20/vic-color.c vic20/vic20cpu.c vic20/vic20ieeevia2.c vic20/vic20-resources.c vic20/vic20via1.c vic20/vic-cycle.c vic20/vic20datasette.c vic20/vic20mem.c vic20/vic20rom.c vic20/vic20via2.c vic20/vic-draw.c vic20/vic20drive.c vic20/vic20memrom.c vic20/vic20romset.c vic20/vic20video.c vic20/vic-mem.c



PPU_LIB_TARGET	=	libvic20.ppu.a

include $(CELL_MK_DIR)/sdk.target.mk

#PPU_LIB_TARGETDIR = 	$(OBJS_DIR)/c64/libc64.ppu
#
#$(PPU_LIB_TARGET_DIR)/PPU_LIB_TARGET:	$(PPU_LIB_TARGET)
#        $(RM) -r $(PPU_LIB_TARGET_DIR)/$(PPU_LIB_TARGET)/
#        mkdir -p $(PPU_LIB_TARGET_DIR)/
#        $(CP) $(PPU_LIB_TARGET) $(PPU_LIB_TARGET_DIR)
#        cd $(PPU_LIB_TARGET_DIR) && $(PPU_AR) -x $(PPU_LIB_TARGET)


