# gpsim cli

INCLUDES = @X_CFLAGS@ 

lib_LTLIBRARIES = libgpsimcli.la

libgpsimcli_la_SOURCES = \
	parse.yy \
	scan.ll \
	cmd_attach.cc \
	cmd_break.cc \
	cmd_bus.cc \
	cmd_clear.cc \
	cmd_disasm.cc \
	cmd_dump.cc \
	cmd_echo.cc \
	cmd_frequency.cc \
	cmd_help.cc \
	cmd_icd.cc \
	cmd_list.cc \
	cmd_load.cc \
	cmd_log.cc \
	cmd_macro.cc \
	cmd_module.cc \
	cmd_node.cc \
	cmd_processor.cc \
	cmd_quit.cc \
	cmd_reset.cc \
	cmd_run.cc \
	cmd_set.cc \
	cmd_shell.cc  \
	cmd_step.cc \
	cmd_stimulus.cc \
	cmd_symbol.cc \
	cmd_trace.cc \
	cmd_version.cc \
	cmd_x.cc \
	command.cc \
	input.cc \
	socket.cc \
	ui_gpsim.cc \
	cmd_attach.h cmd_bus.h cmd_dump.h cmd_load.h cmd_reset.h cmd_symbol.h \
	command.h cmd_break.h cmd_echo.h cmd_log.h cmd_module.h cmd_node.h cmd_run.h \
	cmd_shell.h cmd_trace.h \
	input.h cmd_clear.h cmd_help.h cmd_processor.h cmd_set.h cmd_step.h \
	cmd_version.h misc.h cmd_disasm.h  cmd_list.h  cmd_quit.h \
	cmd_stimulus.h cmd_x.h symbol_t.h cmd_frequency.h cmd_icd.h \
	cmd_macro.h ui_gpsim.h

libgpsimcli_la_LIBADD = $(LIBREADLINE)

AM_YFLAGS = -dvt

libgpsimcli_la_LDFLAGS = @X_LDFLAGS@


EXTRA_DIST = scan.h parse.h
