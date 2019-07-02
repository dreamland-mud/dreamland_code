lib_LTLIBRARIES = \
librace_skills.la \
librace_aptitude.la

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

plugin_INCLUDES = \
-I$(top_srcdir)/plug-ins/skills_impl \
$(INCLUDES_FIGHT) \
$(INCLUDES_SRC)

librace_skills_la_LIBADD = \
$(LIBADD_FIGHT) \
../skills_impl/libskills_impl.la

librace_skills_la_SOURCES = \
hobbit.cpp \
felar.cpp \
rockseer.cpp \
centaur.cpp

librace_aptitude_la_LIBADD = \
$(librace_skills_la_LIBADD) \
librace_skills.la

librace_aptitude_la_SOURCES = \
impl_raceaptitude.cpp \
raceaptitude.cpp 

librace_aptitude_la_MOC = \
raceaptitude.h 



AM_CPPFLAGS += $(plugin_INCLUDES)
