
include common.mk

PPU_SRCS	=	printerdrv/driver-select.c printerdrv/drv-ascii.c printerdrv/drv-mps803.c printerdrv/drv-nl10.c printerdrv/interface-serial.c printerdrv/interface-userport.c printerdrv/output-graphics.c printerdrv/output-select.c printerdrv/output-text.c printerdrv/printer-serial.c printerdrv/printer-userport.c printerdrv/printer.c printerdrv/drv-raw.c



PPU_LIB_TARGET	=	libprinterdrv.ppu.a

include $(CELL_MK_DIR)/sdk.target.mk
