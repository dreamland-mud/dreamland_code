#
# ruffina, DreamLand, 2003
#
noinst_LTLIBRARIES = libcore.la 

SUBDIRS = fenia scheduler behavior skills clan

include $(top_srcdir)/src/Makefile.inc
src_INCLUDES = $(INCLUDES_SRC)

AM_CPPFLAGS = -Wall

libcore_la_SOURCES = \
affect.cpp \
character.cpp \
npcharacter.cpp \
npcharactermanager.cpp \
object.cpp \
objectmanager.cpp \
pcharacter.cpp \
pcharactermanager.cpp \
pcharactermemory.cpp \
pcharactermemorylist.cpp \
pcrace.cpp \
pcskilldata.cpp \
profession.cpp \
race.cpp \
racelanguage.cpp \
remortdata.cpp \
room.cpp \
xmlattribute.cpp \
xmlattributes.cpp \
dlxmlloader.cpp \
hometown.cpp \
wearlocation.cpp \
liquid.cpp \
religion.cpp \
desire.cpp \
helpmanager.cpp \
mobilespecial.cpp \
dlpluginmanager.cpp \
bonus.cpp

libcore_la_MOC = \
affect.h \
character.h \
npcharacter.h \
object.h \
pcharacter.h \
pcharactermemory.h \
pcskilldata.h \
remortdata.h \
xmlattributes.h \
bonus.h

AM_CPPFLAGS += $(src_INCLUDES)
