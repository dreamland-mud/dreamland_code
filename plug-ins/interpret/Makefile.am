lib_LTLIBRARIES = libinterpret.la libcommonlayers.la 

include $(top_srcdir)/src/Makefile.inc
plugin_INCLUDES = \
$(INCLUDES_SRC) \
-I$(top_srcdir)/plug-ins/output \
-I$(top_srcdir)/plug-ins/descriptor \
-I$(top_srcdir)/plug-ins/iomanager \
-I$(top_srcdir)/plug-ins/system \
-I/usr/include/jsoncpp

libinterpret_la_SOURCES = \
interpretlayer.cpp \
commandinterpreter.cpp \
interp.cpp \
translit.cpp \
impl.cpp

libinterpret_la_LIBADD = \
../output/liboutput.la \
../descriptor/libdescriptor.la

libcommonlayers_la_LIBADD = \
libinterpret.la \
../system/libsystem.la \
../output/liboutput.la \
../iomanager/libiomanager.la \
../descriptor/libdescriptor.la \
-ljsoncpp

libcommonlayers_la_SOURCES = commonlayers.cpp

include $(top_srcdir)/plug-ins/Makefile.inc
AM_CPPFLAGS += $(plugin_INCLUDES)
