lib_LTLIBRARIES = \
librace.la 

include $(top_srcdir)/src/Makefile.inc
plugin_INCLUDES = \
$(INCLUDES_SRC) \
-I$(top_srcdir)/plug-ins/descriptor \
-I$(top_srcdir)/plug-ins/system \
-I$(top_srcdir)/plug-ins/alignment \
-I$(top_srcdir)/plug-ins/help

librace_la_LIBADD = ../system/libsystem.la \
../descriptor/libdescriptor.la \
../help/libhelp.la

librace_la_SOURCES = \
raceflags.cpp \
defaultrace.cpp \
defaultpcrace.cpp \
impl.cpp

librace_la_MOC = \
defaultrace.h \
defaultpcrace.h

include $(top_srcdir)/plug-ins/Makefile.inc

raceflags.cpp: raceflags.conf raceflags.h
raceflags.h: raceflags.conf
librace_la_moc_xml.cpp: raceflags.h

CLEANFILES = raceflags.cpp $(top_builddir)/src/raceflags.h 
AM_CPPFLAGS += $(plugin_INCLUDES)
