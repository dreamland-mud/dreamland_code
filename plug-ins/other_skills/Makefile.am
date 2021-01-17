lib_LTLIBRARIES =  libother_skills.la 

libother_skills_la_SOURCES = \
draconian.cpp \
items.cpp \
impl.cpp 

libother_skills_la_MOC = 

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

plugin_INCLUDES = \
-I$(top_srcdir)/plug-ins/skills_impl \
-I$(top_srcdir)/plug-ins/liquid \
$(INCLUDES_FIGHT) \
 $(INCLUDES_SRC)

libother_skills_la_LIBADD = \
../skills_impl/libskills_impl.la \
../liquid/libliquid.la \
$(LIBADD_FIGHT) 

AM_CPPFLAGS += $(plugin_INCLUDES)
