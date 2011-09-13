
include common.mk

#PPU_CXXFLAGS	=	-nostdinc
PPU_SRCS	=	sid/fastsid.c sid/sid-cmdline-options.c sid/sid-resources.c sid/sid-snapshot.c sid/sid.c sid/resid.cc sid/resid-fp.cc



PPU_LIB_TARGET	=	libsid.ppu.a

include $(CELL_MK_DIR)/sdk.target.mk
