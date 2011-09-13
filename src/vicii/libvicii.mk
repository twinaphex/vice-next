
include common.mk

PPU_SRCS	=	vicii/vicii-badline.c vicii/vicii-cmdline-options.c vicii/vicii-fetch.c vicii/vicii-irq.c vicii/vicii-mem.c vicii/vicii-phi1.c vicii/vicii-resources.c vicii/vicii-sprites.c vicii/vicii-stubs.c vicii/vicii-timing.c vicii/vicii.c vicii/vicii-clock-stretch.c 

PPU_SRCS	+=	vicii/vicii-color.c vicii/vicii-draw.c vicii/vicii-snapshot.c 
#PPU_SRCS	+=	vicii/viciidtv-color.c vicii/viciidtv-draw.c vicii/viciidtv-snapshot.c 



PPU_LIB_TARGET	=	libvicii.ppu.a

include $(CELL_MK_DIR)/sdk.target.mk
