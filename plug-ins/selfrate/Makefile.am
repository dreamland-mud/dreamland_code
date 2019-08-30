lib_LTLIBRARIES = libselfrate.la 

include $(top_srcdir)/src/Makefile.inc
plugin_INCLUDES = \
$(INCLUDES_SRC) \
-I$(top_srcdir)/plug-ins/interpret \
-I$(top_srcdir)/plug-ins/command \
-I$(top_srcdir)/plug-ins/system \
-I$(top_srcdir)/plug-ins/help \
-I$(top_srcdir)/plug-ins/iomanager \
-I$(top_srcdir)/plug-ins/loadsave \
-I$(top_srcdir)/plug-ins/descriptor \
-I$(top_srcdir)/plug-ins/output 

libselfrate_la_LIBADD = \
../output/liboutput.la \
../system/libsystem.la \
../descriptor/libdescriptor.la \
../interpret/libinterpret.la \
../loadsave/libloadsave.la \
../command/libcommand.la \
../help/libhelp.la \
../iomanager/libiomanager.la


libselfrate_la_SOURCES = selfrate.cpp 
libselfrate_la_MOC = selfrate.h

include $(top_srcdir)/plug-ins/Makefile.inc
AM_CPPFLAGS += $(plugin_INCLUDES)
