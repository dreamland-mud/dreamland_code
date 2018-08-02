lib_LTLIBRARIES = libinterpret.la libcommonlayers.la 

include $(top_srcdir)/src/Makefile.inc
INCLUDES = \
$(INCLUDES_SRC) \
-I$(top_srcdir)/plug-ins/output \
-I$(top_srcdir)/plug-ins/descriptor 

libinterpret_la_SOURCES = \
interpretlayer.cpp \
commandinterpreter.cpp \
interp.cpp \
impl.cpp

libinterpret_la_LIBADD = \
../output/liboutput.la \
../descriptor/libdescriptor.la

libcommonlayers_la_LIBADD = \
libinterpret.la \
../output/liboutput.la \
../descriptor/libdescriptor.la

libcommonlayers_la_SOURCES = commonlayers.cpp

include $(top_srcdir)/plug-ins/Makefile.inc
