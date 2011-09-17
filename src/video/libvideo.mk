
include common.mk

PPU_SRCS	=	video/render1x1.c video/render1x1pal.c video/render1x2.c video/render2x2.c video/render2x2pal.c video/renderyuv.c video/video-canvas.c video/video-cmdline-options.c video/video-color.c video/video-render-1x2.c video/video-render-2x2.c video/video-render-pal.c video/video-render.c video/video-resources-pal.c video/video-resources.c video/video-viewport.c video/video-render-crt.c video/render2x2ntsc.c video/render1x2crt.c video/render1x1ntsc.c  


PPU_LIB_TARGET	=	libvideo.ppu.a

include $(CELL_MK_DIR)/sdk.target.mk
