lib_LTLIBRARIES = libanatoliacore.la 

include $(top_srcdir)/src/Makefile.inc
INCLUDES = \
$(INCLUDES_SRC) \
-I$(top_srcdir)/plug-ins/descriptor \
-I$(top_srcdir)/plug-ins/output \
-I$(top_srcdir)/plug-ins/system \
-I$(top_srcdir)/plug-ins/fight_core \
-I$(top_srcdir)/plug-ins/follow/core \
-I$(top_srcdir)/plug-ins/interpret \
-I$(top_srcdir)/plug-ins/loadsave 

libanatoliacore_la_SOURCES = \
character.cpp \
pcharacter.cpp \
impl.cpp

libanatoliacore_la_LIBADD = \
../../descriptor/libdescriptor.la \
../../output/liboutput.la \
../../loadsave/libloadsave.la \
../../interpret/libinterpret.la \
../../system/libscheduler_task_round.la \
../../iomanager/libiomanager.la \
../../fight_core/libfight_core.la \
../../follow/core/libfollow_core.la

include $(top_srcdir)/plug-ins/Makefile.inc
