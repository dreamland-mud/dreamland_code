lib_LTLIBRARIES = libban.la

if WITH_MINGW
WSOCK = -lwsock32
endif


include $(top_srcdir)/src/Makefile.inc
include $(top_srcdir)/plug-ins/Makefile.inc
INCLUDES = \
$(INCLUDES_SRC) \
-I$(top_srcdir)/plug-ins/descriptor \
-I$(top_srcdir)/plug-ins/output \
-I$(top_srcdir)/plug-ins/system

libban_la_SOURCES = \
banflags.cpp \
ban.cpp \
impl.cpp

libban_la_MOC = \
ban.h

libban_la_LIBADD = \
../system/libscheduler_task_round.la \
../system/libsystem.la \
../output/liboutput.la \
../descriptor/libdescriptor.la \
$(WSOCK)

banflags.h: banflags.conf
banflags.cpp: banflags.conf banflags.h
libban_la_moc_xml.cpp: banflags.h

CLEANFILES = banflags.cpp $(top_builddir)/src/banflags.h 

