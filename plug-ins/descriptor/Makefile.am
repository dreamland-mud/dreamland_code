lib_LTLIBRARIES = libdescriptor.la


libdescriptor_la_LIBADD = -lz -ljsoncpp
if WITH_MINGW
libdescriptor_la_LIBADD += -lwsock32
endif

libdescriptor_la_SOURCES = \
descriptor.cpp \
bufferhandler.cpp \
inputhandler.cpp \
outofband.cpp \
websocketrpc.cpp \
impl.cpp

libdescriptor_la_MOC = \
descriptor.h \
bufferhandler.h \
inputhandler.h

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc
plugin_INCLUDES = $(INCLUDES_SRC) -I/usr/include/jsoncpp

AM_CPPFLAGS += $(plugin_INCLUDES)
