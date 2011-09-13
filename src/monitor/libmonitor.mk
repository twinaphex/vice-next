
include common.mk

PPU_SRCS	=	monitor/asm6502.c monitor/asm6502dtv.c monitor/asmz80.c monitor/mon_assemble6502.c monitor/mon_assemblez80.c monitor/mon_breakpoint.c monitor/mon_command.c monitor/mon_disassemble.c monitor/mon_drive.c monitor/mon_file.c monitor/mon_memory.c monitor/mon_register6502.c monitor/mon_register6502dtv.c monitor/mon_registerz80.c monitor/mon_ui.c monitor/mon_util.c monitor/mon_lex.c monitor/mon_parse.c monitor/monitor.c monitor/monitor_network.c


PPU_LIB_TARGET	=	libmonitor.ppu.a

include $(CELL_MK_DIR)/sdk.target.mk
