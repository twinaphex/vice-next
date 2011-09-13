
include common.mk


PPU_SRCS	=	drive/drive-check.c drive/drive-cmdline-options.c drive/drive-overflow.c drive/drive-resources.c drive/drive-snapshot.c drive/drive-writeprotect.c drive/drive.c drive/drivecpu.c drive/drivemem.c drive/driveimage.c drive/driverom.c drive/drivesync.c drive/rotation.c


PPU_LIB_TARGET	=	libdrive.ppu.a

include $(CELL_MK_DIR)/sdk.target.mk
