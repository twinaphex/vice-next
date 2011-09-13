
include common.mk

PPU_SRCS	=	rtc/ds1302.c rtc/rtc.c 
#rtc/bq4830y.c rtc/ds12c887.c 


PPU_LIB_TARGET	=	librtc.ppu.a

include $(CELL_MK_DIR)/sdk.target.mk
