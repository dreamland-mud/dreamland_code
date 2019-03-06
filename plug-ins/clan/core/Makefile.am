lib_LTLIBRARIES = libclan_core.la

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

INCLUDES = \
-I$(top_srcdir)/plug-ins/descriptor \
-I$(top_srcdir)/plug-ins/output \
-I$(top_srcdir)/plug-ins/system \
-I$(top_srcdir)/plug-ins/loadsave \
$(INCLUDES_SRC)

libclan_core_la_SOURCES = \
clanflags.cpp \
clantypes.cpp \
clantitles.cpp \
clanorg.cpp \
impl.cpp

libclan_core_la_MOC = \
clantitles.h \
clantypes.h \
clanorg.h

libclan_core_la_LIBADD = \
../../system/libsystem.la \
../../output/liboutput.la \
../../loadsave/libloadsave.la \
../../descriptor/libdescriptor.la 


clanflags.cpp: clanflags.conf clanflags.h
clanflags.h: clanflags.conf
libclan_core_la_moc_xml.cpp: clanflags.h

CLEANFILES = clanflags.cpp $(top_builddir)/src/clanflags.h 

