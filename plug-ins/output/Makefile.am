lib_LTLIBRARIES = liboutput.la

liboutput_la_LIBADD = ../descriptor/libdescriptor.la -ljsoncpp

liboutput_la_SOURCES = \
wiznet.cpp \
infonet.cpp \
msgformatter.cpp \
act.cpp \
colour.cpp \
character.cpp \
room.cpp \
mudtags.cpp \
messengers.cpp \
debug_utils.cpp \
impl.cpp

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc
plugin_INCLUDES = \
$(INCLUDES_SRC) \
-I/usr/include/jsoncpp \
-I$(top_srcdir)/plug-ins/descriptor

AM_CPPFLAGS += $(plugin_INCLUDES)
