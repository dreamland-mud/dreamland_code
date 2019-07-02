lib_LTLIBRARIES = \
libbonus.la 

include $(top_srcdir)/src/Makefile.inc
plugin_INCLUDES = \
$(INCLUDES_SRC) \
-I$(top_srcdir)/plug-ins/system \
-I$(top_srcdir)/plug-ins/output \
-I$(top_srcdir)/plug-ins/help

libbonus_la_LIBADD = ../system/libsystem.la \
../help/libhelp.la \
../output/liboutput.la 

libbonus_la_SOURCES = \
defaultbonus.cpp \
impl.cpp

libbonus_la_MOC = \
defaultbonus.h 

include $(top_srcdir)/plug-ins/Makefile.inc

AM_CPPFLAGS += $(plugin_INCLUDES)
