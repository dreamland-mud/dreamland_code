lib_LTLIBRARIES = libloadsave.la

include $(top_srcdir)/src/Makefile.inc
include $(top_srcdir)/plug-ins/Makefile.inc
plugin_INCLUDES = \
$(INCLUDES_SRC) \
-I/usr/include/jsoncpp \
-I$(top_srcdir)/plug-ins/descriptor \
-I$(top_srcdir)/plug-ins/output \
-I$(top_srcdir)/plug-ins/system 

libloadsave_la_SOURCES = \
affectflags.cpp \
compatflags.cpp \
itemflags.cpp \
recipeflags.cpp \
affects.cpp \
creation.cpp \
extraction.cpp \
finding.cpp \
arg_utils.cpp \
impl.cpp \
placement.cpp \
limited.cpp \
save.cpp \
save_drop.cpp \
character.cpp \
pcharacter.cpp \
pcharactermanager.cpp \
room.cpp \
save_charmed.cpp \
mobilebehaviormanager.cpp \
objectbehaviormanager.cpp \
areabehaviormanager.cpp \
roombehaviormanager.cpp \
fread_utils.cpp \
itemevents.cpp \
areautils.cpp \
roomutils.cpp \
itemutils.cpp \
religionutils.cpp

libloadsave_la_MOC = \
objectbehaviormanager.h

libloadsave_la_LIBADD = \
../descriptor/libdescriptor.la \
../system/libsystem.la \
../system/libscheduler_task_round.la \
../output/liboutput.la \
-ljsoncpp

affectflags.cpp: affectflags.conf affectflags.h
affectflags.h: affectflags.conf

itemflags.cpp: itemflags.conf itemflags.h
itemflags.h: itemflags.conf

compatflags.cpp: compatflags.conf compatflags.h
compatflags.h: compatflags.conf

recipeflags.cpp: recipeflags.conf recipeflags.h
recipeflags.h: recipeflags.conf

libloadsave_la_moc_xml.cpp : compatflags.h itemflags.h affectflags.h recipeflags.h

CLEANFILES = \
affectflags.cpp $(top_builddir)/src/affectflags.h \
compatflags.cpp $(top_builddir)/src/compatflags.h \
recipeflags.cpp $(top_builddir)/src/recipeflags.h \
itemflags.cpp $(top_builddir)/src/itemflags.h 

AM_CPPFLAGS += $(plugin_INCLUDES)
