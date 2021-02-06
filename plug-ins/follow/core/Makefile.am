lib_LTLIBRARIES = libfollow_core.la

include $(top_srcdir)/src/Makefile.inc
plugin_INCLUDES = \
$(INCLUDES_SRC) \
-I$(top_srcdir)/plug-ins/descriptor \
-I$(top_srcdir)/plug-ins/output \
-I$(top_srcdir)/plug-ins/system \
-I$(top_srcdir)/plug-ins/iomanager \
-I$(top_srcdir)/plug-ins/interpret \
-I$(top_srcdir)/plug-ins/loadsave 

libfollow_core_la_SOURCES = \
follow_utils.cpp \
character.cpp

libfollow_core_la_LIBADD = \
../../output/liboutput.la \
../../descriptor/libdescriptor.la \
../../iomanager/libiomanager.la \
../../loadsave/libloadsave.la \
../../interpret/libinterpret.la \
../../system/libsystem.la \
../../system/libscheduler_task_round.la 

include $(top_srcdir)/plug-ins/Makefile.inc
AM_CPPFLAGS += $(plugin_INCLUDES)
