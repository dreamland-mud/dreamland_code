lib_LTLIBRARIES = liblanguages_core.la 

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

plugin_INCLUDES = \
$(INCLUDES_AI) \
$(INCLUDES_SRC) \
-I$(top_srcdir)/plug-ins/skills_impl 

liblanguages_core_la_LIBADD = \
$(LIBADD_AI) \
../../skills_impl/libskills_impl.la

liblanguages_core_la_SOURCES = \
impl.cpp \
language.cpp \
languageskill.cpp \
languagecommand.cpp \
languagemanager.cpp \
word.cpp \
wordeffect.cpp \
xmlattributelanguage.cpp \
poliglot.cpp 

liblanguages_core_la_MOC = \
language.h \
word.h \
wordeffect.h \
xmlattributelanguage.h \
poliglot.h 


AM_CPPFLAGS += $(plugin_INCLUDES)
