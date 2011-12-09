include common.mk

PPU_SRCS	=	arch/ps3/archdep.c arch/ps3/joy.cpp arch/ps3/ui.c arch/ps3/vsidui.c arch/ps3/vsyncarch.c  arch/ps3/kbd.c arch/ps3/mousedrv.c arch/ps3/video.cpp arch/ps3/console.c


PPU_SRCS	+=	arch/ps3/cellframework/audio/buffer.c arch/ps3/cellframework/audio/librsound.c arch/ps3/cellframework/audio/quadratic_resampler.cpp arch/ps3/cellframework/audio/resampler.cpp 
PPU_SRCS	+=	arch/ps3/cellframework/threads/thread.cpp arch/ps3/cellframework/threads/mutex.cpp arch/ps3/cellframework/threads/cond.cpp arch/ps3/cellframework/threads/scoped_lock.cpp 
PPU_SRCS	+=	arch/ps3/cellframework/fileio/FileBrowser.cpp
PPU_SRCS	+=	arch/ps3/cellframework/input/cellInput.cpp arch/ps3/cellframework/utility/OSKUtil.cpp
PPU_SRCS	+=	arch/ps3/emu-ps3.cpp arch/ps3/menu.cpp arch/ps3/ps3video.cpp
PPU_SRCS	+=	arch/ps3/in_game_menu.cpp arch/ps3/ui_snapshot.c

PPU_LIB_TARGET	=	libarch.ppu.a

#Enable debug warnings
#PPU_CFLAGS	+=	-DCELL_DEBUG
#PPU_CXXFLAGS	+=	-DCELL_DEBUG

include $(CELL_MK_DIR)/sdk.target.mk
