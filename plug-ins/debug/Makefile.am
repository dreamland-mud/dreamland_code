lib_LTLIBRARIES = libdebug.la

include $(top_srcdir)/src/Makefile.inc
plugin_INCLUDES = \
$(INCLUDES_SRC) \
-I$(top_srcdir)/plug-ins/loadsave \
-I$(top_srcdir)/plug-ins/output \
-I$(top_srcdir)/plug-ins/descriptor

libdebug_la_LIBADD = \
../loadsave/libloadsave.la \
../output/liboutput.la \
../descriptor/libdescriptor.la

libdebug_la_SOURCES = \
saving.cpp 


include $(top_srcdir)/plug-ins/Makefile.inc
AM_CPPFLAGS += $(plugin_INCLUDES)
