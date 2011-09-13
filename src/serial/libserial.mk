
include common.mk

PPU_SRCS	=	serial/fsdrive.c serial/serial-device.c serial/serial-iec-bus.c serial/serial-iec-device.c serial/serial-iec-lib.c serial/serial-iec.c serial/serial-realdevice.c serial/serial-trap.c serial/serial.c


PPU_LIB_TARGET	=	libserial.ppu.a

include $(CELL_MK_DIR)/sdk.target.mk
