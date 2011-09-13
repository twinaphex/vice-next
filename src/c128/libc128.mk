
include common.mk


PPU_SRCS	= 	c128/c128.c c128/c128fastiec.c c128/c128mmu.c c128/daa.c c128/c128cia1.c c128/c128mem.c c128/c128-resources.c c128/functionrom.c c128/c128-cmdline-options.c c128/c128meminit.c c128/c128rom.c c128/z80.c c128/c128cpu.c c128/c128memlimit.c c128/c128romset.c c128/z80mem.c c128/c128drive.c c128/c128memrom.c c128/c128-snapshot.c c128/z80vms.c c128/c128memsnapshot.c c128/c128video.c
#c128/c128embedded.c 



PPU_LIB_TARGET	=	libc128.ppu.a

include $(CELL_MK_DIR)/sdk.target.mk

#PPU_LIB_TARGETDIR = 	$(OBJS_DIR)/c64/libc64.ppu
#
#$(PPU_LIB_TARGET_DIR)/PPU_LIB_TARGET:	$(PPU_LIB_TARGET)
#        $(RM) -r $(PPU_LIB_TARGET_DIR)/$(PPU_LIB_TARGET)/
#        mkdir -p $(PPU_LIB_TARGET_DIR)/
#        $(CP) $(PPU_LIB_TARGET) $(PPU_LIB_TARGET_DIR)
#        cd $(PPU_LIB_TARGET_DIR) && $(PPU_AR) -x $(PPU_LIB_TARGET)


