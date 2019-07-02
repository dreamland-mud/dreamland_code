lib_LTLIBRARIES = libai.la 

libai_la_SOURCES = \
basicmobilebehavior.cpp \
aggression.cpp \
cleric.cpp \
fight.cpp \
mage.cpp \
necromancer.cpp \
specials.cpp \
tracking.cpp \
vampire.cpp \
ranger.cpp \
caster.cpp \
assist.cpp \
items.cpp \
impl.cpp

libai_la_MOC = \
basicmobilebehavior.h 

include $(top_srcdir)/src/Makefile.inc
include $(top_srcdir)/plug-ins/Makefile.inc

plugin_INCLUDES = \
-I$(top_srcdir)/plug-ins/traverse \
$(INCLUDES_FIGHT) \
$(INCLUDES_SRC) 

libai_la_LIBADD = \
$(LIBADD_FIGHT) \
../traverse/libtraverse.la 

AM_CPPFLAGS += $(plugin_INCLUDES)
