
include common.mk

PPU_SRCS	=	vdrive/vdrive-bam.c vdrive/vdrive-command.c vdrive/vdrive-dir.c vdrive/vdrive-iec.c vdrive/vdrive-internal.c vdrive/vdrive-rel.c vdrive/vdrive-snapshot.c vdrive/vdrive.c


PPU_LIB_TARGET	=	libvdrive.ppu.a

include $(CELL_MK_DIR)/sdk.target.mk
