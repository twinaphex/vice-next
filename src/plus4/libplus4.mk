
include common.mk


PPU_SRCS	= 	plus4/digiblaster.c plus4/plus4embedded.c plus4/plus4pio1.c plus4/plus4video.c plus4/ted-mem.c plus4/plus4acia.c plus4/plus4iec.c plus4/plus4pio2.c plus4/sidcartjoy.c plus4/ted-resources.c plus4/plus4bus.c plus4/plus4mem.c plus4/plus4printer.c plus4/ted-badline.c plus4/ted-snapshot.c plus4/plus4.c plus4/plus4memcsory256k.c plus4/plus4-resources.c plus4/ted.c plus4/ted-sound.c plus4/plus4cart.c plus4/plus4memhannes256k.c plus4/plus4rom.c plus4/ted-cmdline-options.c plus4/ted-timer.c plus4/plus4-cmdline-options.c plus4/plus4memlimit.c plus4/plus4romset.c plus4/ted-color.c plus4/ted-timing.c plus4/plus4cpu.c plus4/plus4memrom.c plus4/plus4-snapshot.c plus4/ted-draw.c plus4/plus4datasette.c plus4/plus4memsnapshot.c plus4/plus4speech.c plus4/ted-fetch.c plus4/plus4drive.c plus4/plus4parallel.c plus4/plus4tcbm.c plus4/ted-irq.c


PPU_LIB_TARGET	=	libplus4.ppu.a

include $(CELL_MK_DIR)/sdk.target.mk

#PPU_LIB_TARGETDIR = 	$(OBJS_DIR)/c64/libc64.ppu
#
#$(PPU_LIB_TARGET_DIR)/PPU_LIB_TARGET:	$(PPU_LIB_TARGET)
#        $(RM) -r $(PPU_LIB_TARGET_DIR)/$(PPU_LIB_TARGET)/
#        mkdir -p $(PPU_LIB_TARGET_DIR)/
#        $(CP) $(PPU_LIB_TARGET) $(PPU_LIB_TARGET_DIR)
#        cd $(PPU_LIB_TARGET_DIR) && $(PPU_AR) -x $(PPU_LIB_TARGET)


