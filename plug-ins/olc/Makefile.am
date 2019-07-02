lib_LTLIBRARIES = libolc.la libolc_skills.la

libolc_skills_la_SOURCES = olcstubskill.cpp
libolc_skills_la_MOC = olcstubskill.h
libolc_skills_la_LIBADD = \
../skills_impl/libskills_impl.la \
../system/libsystem.la

libolc_la_SOURCES = \
olcflags.cpp \
olc.cpp \
xmlindexdata.cpp \
redit.cpp \
sedit.cpp \
aedit.cpp \
medit.cpp \
oedit.cpp \
eeedit.cpp \
mem.cpp \
olc_act.cpp \
olc_impl.cpp \
olc_save.cpp \
olc_savecompat.cpp \
olc_dump.cpp \
olcstate.cpp \
onlinecreation.cpp \
ovalues.cpp \
security.cpp \
webedit.cpp

libolc_la_MOC = \
redit.h \
aedit.h \
medit.h \
sedit.h \
oedit.h \
eeedit.h \
security.h \
olcstate.h 


include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

plugin_INCLUDES = \
$(INCLUDES_FIGHT) \
-I$(top_srcdir)/plug-ins/skills_impl \
-I$(top_srcdir)/plug-ins/updates \
-I$(top_srcdir)/plug-ins/areas \
-I$(top_srcdir)/plug-ins/ed \
-I$(top_srcdir)/plug-ins/ed/lib \
$(INCLUDES_SRC) 


libolc_la_LIBADD = \
$(LIBADD_FIGHT) \
../ed/libcoreed.la \
../ed/lib/libed.la \
../areas/libareas.la \
../skills_impl/libskills_impl.la \
../updates/libupdates.la

olcflags.cpp: olcflags.conf olcflags.h
olcflags.h: olcflags.conf
libolc_la_moc_xml.cpp: olcflags.h

CLEANFILES = olcflags.cpp $(top_builddir)/src/olcflags.h 

AM_CPPFLAGS += $(plugin_INCLUDES)
