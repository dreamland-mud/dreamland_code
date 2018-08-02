SUBDIRS = lib

lib_LTLIBRARIES = libcoreed.la 

include $(top_srcdir)/src/Makefile.inc
INCLUDES = \
$(INCLUDES_SRC) \
-I$(top_srcdir)/plug-ins/ed/lib \
-I$(top_srcdir)/plug-ins/descriptor \
-I$(top_srcdir)/plug-ins/output \
-I$(top_srcdir)/plug-ins/system \
-I$(top_srcdir)/plug-ins/iomanager \
-I$(top_srcdir)/plug-ins/interpret

libcoreed_la_LIBADD = \
../descriptor/libdescriptor.la \
../output/liboutput.la \
../system/libsystem.la \
../iomanager/libiomanager.la \
../interpret/libinterpret.la \
lib/libed.la

libcoreed_la_SOURCES = \
xmlpcstringeditor.cpp \
xmleditorinputhandler.cpp \
format.cpp \
impl.cpp

libcoreed_la_MOC = \
xmlpcstringeditor.h \
xmleditorinputhandler.h

include $(top_srcdir)/plug-ins/Makefile.inc
