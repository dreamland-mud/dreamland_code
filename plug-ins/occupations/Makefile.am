lib_LTLIBRARIES = liboccupations.la 

liboccupations_la_SOURCES = \
occupations.cpp \
attract.cpp \
impl.cpp

liboccupations_la_MOC = \
attract.h 

include $(top_srcdir)/src/Makefile.inc
include $(top_srcdir)/plug-ins/Makefile.inc

plugin_INCLUDES = \
-I$(top_srcdir)/plug-ins/system \
-I$(top_srcdir)/plug-ins/interpret \
-I$(top_srcdir)/plug-ins/descriptor \
-I$(top_srcdir)/plug-ins/output \
-I$(top_srcdir)/plug-ins/loadsave \
-I$(top_srcdir)/plug-ins/command \
-I$(top_srcdir)/plug-ins/help \
$(INCLUDES_SRC) 

liboccupations_la_LIBADD = \
../system/libsystem.la \
../interpret/libinterpret.la \
../output/liboutput.la \
../descriptor/libdescriptor.la \
../loadsave/libloadsave.la \
../help/libhelp.la \
../command/libcommand.la
AM_CPPFLAGS += $(plugin_INCLUDES)
