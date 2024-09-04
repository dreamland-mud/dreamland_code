lib_LTLIBRARIES = libolc.la

libolc_la_SOURCES = \
olcflags.cpp \
olc.cpp \
resets.cpp \
xmlindexdata.cpp \
redit.cpp \
sedit.cpp \
aedit.cpp \
hedit.cpp \
reledit.cpp \
skedit.cpp \
cmdedit.cpp \
medit.cpp \
oedit.cpp \
eeedit.cpp \
mem.cpp \
olc_act.cpp \
olc_impl.cpp \
olc_save.cpp \
olc_dump.cpp \
olcstate.cpp \
onlinecreation.cpp \
ovalues.cpp \
security.cpp \
socedit.cpp \
webedit.cpp \
feniatriggers.cpp \
fedit.cpp \
raceedit.cpp \
qedit.cpp \
bedit.cpp

libolc_la_MOC = \
redit.h \
aedit.h \
hedit.h \
reledit.h \
skedit.h \
cmdedit.h \
medit.h \
sedit.h \
socedit.h \
oedit.h \
eeedit.h \
security.h \
olcstate.h \
fedit.h \
raceedit.h \
qedit.h \
bedit.h


include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

plugin_INCLUDES = \
$(INCLUDES_FIGHT) \
-I$(top_srcdir)/plug-ins/skills_impl \
-I$(top_srcdir)/plug-ins/updates \
-I$(top_srcdir)/plug-ins/areas \
-I$(top_srcdir)/plug-ins/feniaroot \
-I$(top_srcdir)/plug-ins/religion \
-I$(top_srcdir)/plug-ins/social \
-I$(top_srcdir)/plug-ins/ed \
-I$(top_srcdir)/plug-ins/ed/lib \
$(INCLUDES_SRC) 


libolc_la_LIBADD = \
$(LIBADD_FIGHT) \
../ed/libcoreed.la \
../ed/lib/libed.la \
../areas/libareas.la \
../feniaroot/libfeniaroot.la \
../religion/libreligion.la \
../skills_impl/libskills_impl.la \
../updates/libupdates.la \
../social/libsocial.la \
-ljsoncpp

olcflags.cpp: olcflags.conf olcflags.h
olcflags.h: olcflags.conf
libolc_la_moc_xml.cpp: olcflags.h

CLEANFILES = olcflags.cpp $(top_builddir)/src/olcflags.h 

AM_CPPFLAGS += $(plugin_INCLUDES)
