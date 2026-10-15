lib_LTLIBRARIES = libloadsave.la

include $(top_srcdir)/src/Makefile.inc
include $(top_srcdir)/plug-ins/Makefile.inc
plugin_INCLUDES = \
$(INCLUDES_SRC) \
-I/usr/include/jsoncpp \
-I$(top_srcdir)/plug-ins/descriptor \
-I$(top_srcdir)/plug-ins/output \
-I$(top_srcdir)/plug-ins/interpret \
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
impl.cpp \
placement.cpp \
limited.cpp \
save.cpp \
save_drop.cpp \
save_bank.cpp \
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
roomutils.cpp \
itemutils.cpp \
char_weight.cpp \
char_body.cpp \
religionutils.cpp \
areaquestutils.cpp \
xmlattributeareaquest.cpp \
behavior_utils.cpp \
stats_apply.cpp \
player_exp.cpp \
player_account.cpp \
money_utils.cpp \
player_menu.cpp \
occupations.cpp \
accountmanager.cpp \
linkingcode.cpp \
emailcode.cpp \
accountaudit.cpp

libloadsave_la_MOC = \
objectbehaviormanager.h \
xmlattributeareaquest.h

libloadsave_la_LIBADD = \
../descriptor/libdescriptor.la \
../system/libsystem.la \
../system/libscheduler_task_round.la \
../output/liboutput.la \
../interpret/libinterpret.la \
-ljsoncpp

affectflags.cpp: affectflags.conf $(top_builddir)/src/affectflags.h
$(top_builddir)/src/affectflags.h: affectflags.conf
	$(MAKEBITS) -s $< -h -r -f $@

itemflags.cpp: itemflags.conf $(top_builddir)/src/itemflags.h
$(top_builddir)/src/itemflags.h: itemflags.conf
	$(MAKEBITS) -s $< -h -r -f $@

compatflags.cpp: compatflags.conf $(top_builddir)/src/compatflags.h
$(top_builddir)/src/compatflags.h: compatflags.conf
	$(MAKEBITS) -s $< -h -r -f $@

recipeflags.cpp: recipeflags.conf $(top_builddir)/src/recipeflags.h
$(top_builddir)/src/recipeflags.h: recipeflags.conf
	$(MAKEBITS) -s $< -h -r -f $@

libloadsave_la_moc_xml.cpp : $(top_builddir)/src/compatflags.h $(top_builddir)/src/itemflags.h $(top_builddir)/src/affectflags.h $(top_builddir)/src/recipeflags.h

CLEANFILES = \
affectflags.cpp $(top_builddir)/src/affectflags.h \
compatflags.cpp $(top_builddir)/src/compatflags.h \
recipeflags.cpp $(top_builddir)/src/recipeflags.h \
itemflags.cpp $(top_builddir)/src/itemflags.h 

AM_CPPFLAGS += $(plugin_INCLUDES)
