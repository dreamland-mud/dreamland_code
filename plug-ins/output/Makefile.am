lib_LTLIBRARIES = liboutput.la

liboutput_la_LIBADD = ../descriptor/libdescriptor.la

liboutput_la_SOURCES = \
wiznet.cpp \
infonet.cpp \
msgformatter.cpp \
act.cpp \
colour.cpp \
character.cpp \
room.cpp \
mudtags.cpp \
impl.cpp

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc
INCLUDES = \
$(INCLUDES_SRC) \
-I$(top_srcdir)/plug-ins/descriptor

