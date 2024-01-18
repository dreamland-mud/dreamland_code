lib_LTLIBRARIES = libfight.la

libfight_la_SOURCES = \
effects.cpp \
fight.cpp \
fight_cmds.cpp \
fight_death.cpp \
fight_exp.cpp \
fight_subr.cpp \
magic.cpp \
onehit.cpp \
onehit_undef.cpp \
onehit_weapon.cpp \
impl.cpp


include $(top_srcdir)/src/Makefile.inc
include $(top_srcdir)/plug-ins/Makefile.inc

plugin_INCLUDES = \
-I$(top_srcdir)/plug-ins/profession \
$(INCLUDES_ANATOLIA) \
$(INCLUDES_SRC) 

libfight_la_LIBADD = \
$(LIBADD_ANATOLIA) \
../profession/libprofession.la \
-ljsoncpp

AM_CPPFLAGS += $(plugin_INCLUDES)
