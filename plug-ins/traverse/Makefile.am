lib_LTLIBRARIES = libtraverse.la 

include $(top_srcdir)/src/Makefile.inc
plugin_INCLUDES = \
$(INCLUDES_SRC) \
-I$(top_srcdir)/plug-ins/system \
-I$(top_srcdir)/plug-ins/anatolia \
-I$(top_srcdir)/plug-ins/interpret \
-I$(top_srcdir)/plug-ins/command \
-I$(top_srcdir)/plug-ins/help \
-I$(top_srcdir)/plug-ins/loadsave \
-I$(top_srcdir)/plug-ins/descriptor \
-I$(top_srcdir)/plug-ins/output \
-I$(top_srcdir)/plug-ins/fight_core \
-I$(top_srcdir)/plug-ins/movement 

libtraverse_la_LIBADD = \
../descriptor/libdescriptor.la \
../system/libsystem.la \
../output/liboutput.la \
../command/libcommand.la \
../help/libhelp.la \
../interpret/libinterpret.la \
../loadsave/libloadsave.la \
../anatolia/core/libanatoliacore.la \
../anatolia/libanatolia.la \
../movement/libmovement.la \
../fight_core/libfight_core.la 

libtraverse_la_SOURCES = \
roomtraverse.cpp \
commands.cpp \
wanderer.cpp

include $(top_srcdir)/plug-ins/Makefile.inc
AM_CPPFLAGS += $(plugin_INCLUDES)
