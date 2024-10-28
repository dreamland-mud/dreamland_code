lib_LTLIBRARIES = libmovement.la

include $(top_srcdir)/src/Makefile.inc
plugin_INCLUDES = \
$(INCLUDES_SRC) \
-I$(top_srcdir)/plug-ins/descriptor \
-I$(top_srcdir)/plug-ins/output \
-I$(top_srcdir)/plug-ins/system \
-I$(top_srcdir)/plug-ins/iomanager \
-I$(top_srcdir)/plug-ins/loadsave \
-I$(top_srcdir)/plug-ins/fight_core \
-I$(top_srcdir)/plug-ins/interpret

libmovement_la_SOURCES = \
directions.cpp \
exitsmovement.cpp \
fleemovement.cpp \
move_utils.cpp \
movement.cpp \
movetypes.cpp \
portalmovement.cpp \
roomhistory.cpp \
terrains.cpp \
doors.cpp \
walkment.cpp \
transfermovement.cpp \
recallmovement.cpp \
jumpmovement.cpp \
keyhole.cpp

libmovement_la_LIBADD = \
../output/liboutput.la \
../descriptor/libdescriptor.la \
../iomanager/libiomanager.la \
../loadsave/libloadsave.la \
../system/libsystem.la \
../system/libscheduler_task_round.la \
../interpret/libinterpret.la \
../fight_core/libfight_core.la

include $(top_srcdir)/plug-ins/Makefile.inc
AM_CPPFLAGS += $(plugin_INCLUDES)
