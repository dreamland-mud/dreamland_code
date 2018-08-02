lib_LTLIBRARIES = libriding.la

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
-I$(top_srcdir)/plug-ins/movement \
-I$(top_srcdir)/plug-ins/help \
-I$(top_srcdir)/plug-ins/command

libriding_la_SOURCES = \
commands.cpp 

libriding_la_LIBADD = \
../output/liboutput.la \
../descriptor/libdescriptor.la \
../iomanager/libiomanager.la \
../loadsave/libloadsave.la \
../system/libsystem.la \
../system/libscheduler_task_round.la \
../interpret/libinterpret.la \
../fight_core/libfight_core.la \
../follow/core/libfollow_core.la \
../movement/libmovement.la \
../help/libhelp.la \
../command/libcommand.la

include $(top_srcdir)/plug-ins/Makefile.inc
