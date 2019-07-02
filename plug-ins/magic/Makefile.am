lib_LTLIBRARIES = libmagic.la

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

plugin_INCLUDES = \
-I$(top_srcdir)/plug-ins/skills_impl \
$(INCLUDES_FIGHT) \
$(INCLUDES_SRC)

libmagic_la_LIBADD = \
../skills_impl/libskills_impl.la \
$(LIBADD_FIGHT) 

libmagic_la_SOURCES = \
ccast.cpp \
objcast.cpp \
scribing.cpp \
impl.cpp

libmagic_la_MOC = \
scribing.h 


AM_CPPFLAGS += $(plugin_INCLUDES)
