lib_LTLIBRARIES =  libskills_impl.la 

libskills_impl_la_SOURCES = \
skillsflags.cpp \
basicskill.cpp \
defaultaffecthandler.cpp \
defaultspell.cpp \
defaultskillcommand.cpp \
defaultskillgroup.cpp \
impl.cpp \
mobskilldata.cpp \
spellmanager.cpp \
summoncreaturespell.cpp \
objthrow.cpp \
transportspell.cpp \
sleepaffecthandler.cpp \
skillhelp.cpp \
skillgrouphelp.cpp \
xmlattributerestring.cpp \
feniaskillaction.cpp \
ccast.cpp \
skill_alloc.cpp

libskills_impl_la_MOC = \
basicskill.h \
defaultaffecthandler.h \
defaultspell.h \
defaultskillcommand.h \
defaultskillgroup.h \
mobskilldata.h \
spellmanager.h \
summoncreaturespell.h \
transportspell.h \
sleepaffecthandler.h \
xmlattributerestring.h \
feniaskillaction.h


include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

plugin_INCLUDES = $(INCLUDES_FIGHT) $(INCLUDES_SRC)
libskills_impl_la_LIBADD = $(LIBADD_FIGHT) -ljsoncpp

skillsflags.cpp: skillsflags.conf skillsflags.h
skillsflags.h: skillsflags.conf
libskills_la_moc_xml.cpp: skillsflags.h
libskills_impl_la_moc_xml.cpp: skillsflags.h

CLEANFILES = skillsflags.cpp $(top_builddir)/src/skillsflags.h 

AM_CPPFLAGS += $(plugin_INCLUDES)
