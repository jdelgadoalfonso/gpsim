# gpsim gui

INCLUDES = @X_CFLAGS@ @Y_CFLAGS@

lib_LTLIBRARIES = libgpsimgui.la

libgpsimgui_la_SOURCES =  gui_break.cc gui_callbacks.cc gui_dialog.cc gui_init.cc gui_main.cc \
	gui_menu.cc gui_processor.cc gui_regwin.cc gui_src.cc gui_src_asm.cc \
	gui_src_opcode.cc gui_statusbar.cc gui.h gui_callbacks.h \
	gui_interface.h gui_symbols.cc gui_watch.cc gui_breadboard.cc \
	gui_stack.cc gui_trace.cc gui_profile.cc \
	gui_stopwatch.cc gui_object.cc gui_scope.cc settings_exdbm.cc \
	gui_breadboard.h gui_object.h gui_processor.h gui_profile.h \
	gui_register.h gui_regwin.h \
	gui_scope.h gui_src.h gui_stack.h gui_statusbar.h gui_stopwatch.h \
	gui_symbols.h gui_trace.h gui_watch.h \
	settings.h settings_exdbm.h

libgpsimgui_la_LDFLAGS = @X_LDFLAGS@ @Y_LDFLAGS@

pixmapdir = $(datadir)/gpsim

AM_CFLAGS = -D DATADIR=\"~/.gpsim/\"
AM_CXXFLAGS = -D DATADIR=\"~/.gpsim/\"
AM_CPPFLAGS = -D DATADIR=\"~/.gpsim/\"
