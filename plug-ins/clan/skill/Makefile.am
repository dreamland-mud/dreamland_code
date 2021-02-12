lib_LTLIBRARIES = libclan_skill.la 

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

plugin_INCLUDES = \
-I$(top_srcdir)/plug-ins/skills_impl \
-I$(srcdir)/../core \
-I$(srcdir)/../behavior \
$(INCLUDES_FIGHT) \
$(INCLUDES_SRC)

libclan_skill_la_SOURCES = \
clanskill.cpp \
clanorgskill.cpp \
impl.cpp

libclan_skill_la_MOC = \
clanskill.h \
clanorgskill.h 

libclan_skill_la_LIBADD = \
$(LIBADD_FIGHT) \
../../skills_impl/libskills_impl.la \
../core/libclan_core.la

AM_CPPFLAGS += $(plugin_INCLUDES)
