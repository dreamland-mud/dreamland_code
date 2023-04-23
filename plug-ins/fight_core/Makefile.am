lib_LTLIBRARIES = libfight_core.la

include $(top_srcdir)/src/Makefile.inc
plugin_INCLUDES = \
$(INCLUDES_SRC) \
-I$(top_srcdir)/plug-ins/descriptor \
-I$(top_srcdir)/plug-ins/output \
-I$(top_srcdir)/plug-ins/system \
-I$(top_srcdir)/plug-ins/alignment \
-I$(top_srcdir)/plug-ins/iomanager \
-I$(top_srcdir)/plug-ins/loadsave \
-I$(top_srcdir)/plug-ins/occupations \
-I$(top_srcdir)/plug-ins/interpret \
-I/usr/include/jsoncpp

libfight_core_la_SOURCES = \
damageflags.cpp \
attacks.cpp \
impl.cpp \
fight_position.cpp \
fight_exception.cpp \
damage.cpp \
damage_impl.cpp \
character.cpp \
stats_apply.cpp \
weapons.cpp \
weapontier.cpp \
weaponaffixes.cpp \
weaponcalculator.cpp \
weapongenerator.cpp \
weaponrandomizer.cpp \
material.cpp \
fight_safe.cpp \
immunity.cpp \
follow_utils.cpp \
skill_utils.cpp 

libfight_core_la_LIBADD = \
../output/liboutput.la \
../descriptor/libdescriptor.la \
../iomanager/libiomanager.la \
../loadsave/libloadsave.la \
../system/libsystem.la \
../alignment/libalignment.la \
../occupations/liboccupations.la \
../system/libscheduler_task_round.la \
../interpret/libinterpret.la \
-ljsoncpp

damageflags.cpp: damageflags.conf damageflags.h
damageflags.h: damageflags.conf
damageflags.cpp: damageflags.h

CLEANFILES = damageflags.cpp $(top_builddir)/src/damageflags.h 

include $(top_srcdir)/plug-ins/Makefile.inc



AM_CPPFLAGS += $(plugin_INCLUDES)
