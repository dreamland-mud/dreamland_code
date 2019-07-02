lib_LTLIBRARIES = \
libplugin_command.la \
libadmin.la

include $(top_srcdir)/src/Makefile.inc
plugin_INCLUDES = \
$(INCLUDES_SRC) \
-I$(top_srcdir)/plug-ins/interpret \
-I$(top_srcdir)/plug-ins/command  \
-I$(top_srcdir)/plug-ins/descriptor \
-I$(top_srcdir)/plug-ins/loadsave \
-I$(top_srcdir)/plug-ins/output \
-I$(top_srcdir)/plug-ins/ban \
-I$(top_srcdir)/plug-ins/system \
-I$(top_srcdir)/plug-ins/help \
-I$(top_srcdir)/plug-ins/iomanager

if WITH_MINGW
WSOCK = -lwsock32
endif

libadmin_la_LIBADD = \
../descriptor/libdescriptor.la \
../output/liboutput.la \
../interpret/libinterpret.la \
../command/libcommand.la \
../help/libhelp.la \
../loadsave/libloadsave.la \
../iomanager/libiomanager.la \
../system/libsystem.la \
../anatolia/core/libanatoliacore.la \
../ban/libban.la \
$(WSOCK)

libadmin_la_SOURCES = \
impl.cpp \
cbackup.cpp \
crecover.cpp \
cundelete.cpp \
deny.cpp \
cban.cpp \
finger.cpp \
idelete.cpp \
nochannel.cpp \
nopost.cpp \
socket.cpp \
reward.cpp \
lasthost.cpp \
confirm.cpp 

libplugin_command_la_LIBADD = \
../descriptor/libdescriptor.la \
../interpret/libinterpret.la \
../output/liboutput.la \
../help/libhelp.la \
../command/libcommand.la \
../iomanager/libiomanager.la

libplugin_command_la_SOURCES = cplugin.cpp

libadmin_la_MOC = \
deny.h \
cban.h \
nochannel.h \
nopost.h \
reward.h \
lasthost.h \
confirm.h 

libplugin_command_la_MOC = cplugin.h

include $(top_srcdir)/plug-ins/Makefile.inc
AM_CPPFLAGS += $(plugin_INCLUDES)
