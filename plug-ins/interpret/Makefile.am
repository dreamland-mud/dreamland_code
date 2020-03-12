lib_LTLIBRARIES = libinterpret.la 

include $(top_srcdir)/src/Makefile.inc
plugin_INCLUDES = \
$(INCLUDES_SRC) \
-I$(top_srcdir)/plug-ins/output \
-I$(top_srcdir)/plug-ins/descriptor 

libinterpret_la_SOURCES = \
interpretlayer.cpp \
commandinterpreter.cpp \
interp.cpp \
translit.cpp \
impl.cpp

libinterpret_la_LIBADD = \
../output/liboutput.la \
../descriptor/libdescriptor.la

include $(top_srcdir)/plug-ins/Makefile.inc
AM_CPPFLAGS += $(plugin_INCLUDES)
