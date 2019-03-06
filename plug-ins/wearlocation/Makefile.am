lib_LTLIBRARIES = \
libwearlocation.la

include $(top_srcdir)/src/Makefile.inc
INCLUDES = \
$(INCLUDES_SRC) \
-I$(top_srcdir)/plug-ins/descriptor \
-I$(top_srcdir)/plug-ins/output \
-I$(top_srcdir)/plug-ins/system \
-I$(top_srcdir)/plug-ins/iomanager \
-I$(top_srcdir)/plug-ins/loadsave \
-I$(top_srcdir)/plug-ins/fight_core \
-I$(top_srcdir)/plug-ins/follow/core \
-I$(top_srcdir)/plug-ins/interpret \
-I$(top_srcdir)/plug-ins/help \
-I$(top_srcdir)/plug-ins/command

libwearlocation_la_LIBADD = \
../output/liboutput.la \
../descriptor/libdescriptor.la \
../iomanager/libiomanager.la \
../loadsave/libloadsave.la \
../system/libsystem.la \
../system/libscheduler_task_round.la \
../interpret/libinterpret.la \
../help/libhelp.la \
../command/libcommand.la \
../fight_core/libfight_core.la \
../follow/core/libfollow_core.la

libwearlocation_la_SOURCES = \
impl.cpp \
defaultwearlocation.cpp \
misc_wearlocs.cpp \
wearlocationmanager.cpp \
wearloc_commands.cpp

libwearlocation_la_MOC = \
defaultwearlocation.h \
misc_wearlocs.h


include $(top_srcdir)/plug-ins/Makefile.inc
