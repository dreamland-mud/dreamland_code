lib_LTLIBRARIES = \
libalias.la \
libcomm.la 

libalias_la_LIBADD = \
../iomanager/libiomanager.la \
../interpret/libinterpret.la \
../descriptor/libdescriptor.la \
../output/liboutput.la \
../system/libsystem.la \
../help/libhelp.la \
../command/libcommand.la 

libalias_la_SOURCES = alias.cpp

libcomm_la_SOURCES = \
banking.cpp \
configs.cpp \
corder.cpp \
impl.cpp \
pcdeleteidle.cpp \
run.cpp \
showtable.cpp \
whois.cpp \
who.cpp \
writing.cpp \
eating.cpp \
communication.cpp \
info.cpp \
senses.cpp

libcomm_la_MOC = \
banking.h \
configs.h \
corder.h \
pcdeleteidle.h \
run.h \
whois.h \
writing.h \
eating.h

include $(top_srcdir)/src/Makefile.inc
include $(top_srcdir)/plug-ins/Makefile.inc

plugin_INCLUDES = \
-I$(top_srcdir)/plug-ins/communication \
$(INCLUDES_FIGHT) \
$(INCLUDES_SRC) \
-I$(top_srcdir)/plug-ins/ed \
-I$(top_srcdir)/plug-ins/servlets \
-I$(top_srcdir)/plug-ins/ed/lib \
-I/usr/include/jsoncpp

libcomm_la_LIBADD = \
$(LIBADD_FIGHT) \
../ed/lib/libed.la \
../ed/libcoreed.la \
../servlets/libservlets.la \
../communication/libcommunication.la \
-ljsoncpp

AM_CPPFLAGS += $(plugin_INCLUDES)
