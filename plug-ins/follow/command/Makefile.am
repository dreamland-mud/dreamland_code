lib_LTLIBRARIES = libfollow_command.la 

include $(top_srcdir)/src/Makefile.inc
INCLUDES = \
$(INCLUDES_SRC) \
-I$(top_srcdir)/plug-ins/interpret \
-I$(top_srcdir)/plug-ins/command  \
-I$(top_srcdir)/plug-ins/communication \
-I$(top_srcdir)/plug-ins/help \
-I$(top_srcdir)/plug-ins/ban \
-I$(top_srcdir)/plug-ins/system \
-I$(top_srcdir)/plug-ins/descriptor \
-I$(top_srcdir)/plug-ins/output \
-I$(top_srcdir)/plug-ins/loadsave \
-I$(top_srcdir)/plug-ins/follow/core

libfollow_command_la_LIBADD = \
../../system/libsystem.la \
../../interpret/libinterpret.la \
../../descriptor/libdescriptor.la \
../../output/liboutput.la \
../../loadsave/libloadsave.la \
../../ban/libban.la \
../../help/libhelp.la \
../../command/libcommand.la \
../../communication/libcommunication.la \
../core/libfollow_core.la

libfollow_command_la_SOURCES = \
groupchannel.cpp \
commands.cpp \
impl.cpp 

libfollow_command_la_MOC = \
groupchannel.h 


include $(top_srcdir)/plug-ins/Makefile.inc
