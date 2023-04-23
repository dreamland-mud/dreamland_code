lib_LTLIBRARIES = libcommunication.la 

include $(top_srcdir)/src/Makefile.inc
plugin_INCLUDES = \
$(INCLUDES_SRC) \
-I$(top_srcdir)/plug-ins/interpret \
-I$(top_srcdir)/plug-ins/command  \
-I$(top_srcdir)/plug-ins/system \
-I$(top_srcdir)/plug-ins/descriptor \
-I$(top_srcdir)/plug-ins/output \
-I$(top_srcdir)/plug-ins/help \
-I$(top_srcdir)/plug-ins/loadsave \
-I$(top_srcdir)/plug-ins/iomanager \
-I$(top_srcdir)/plug-ins/servlets \
-I/usr/include/jsoncpp

libcommunication_la_LIBADD = \
../system/libsystem.la \
../interpret/libinterpret.la \
../descriptor/libdescriptor.la \
../output/liboutput.la \
../iomanager/libiomanager.la \
../loadsave/libloadsave.la \
../help/libhelp.la \
../command/libcommand.la \
../servlets/libservlets.la \
-ljsoncpp

libcommunication_la_SOURCES = \
languages.cpp \
twitlist.cpp \
areachannel.cpp \
communicationchannel.cpp \
globalchannel.cpp \
replay.cpp \
impl.cpp \
personalchannel.cpp \
racechannel.cpp \
roomchannel.cpp \
worldchannel.cpp \
channels.cpp \
channelservlet.cpp

libcommunication_la_MOC = \
areachannel.h \
communicationchannel.h \
globalchannel.h \
personalchannel.h \
racechannel.h \
roomchannel.h \
worldchannel.h \
replay.h \
channels.h 


include $(top_srcdir)/plug-ins/Makefile.inc
AM_CPPFLAGS += $(plugin_INCLUDES)
