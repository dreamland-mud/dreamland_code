lib_LTLIBRARIES = \
libdesire.la

libdesire_la_SOURCES = \
defaultdesire.cpp \
misc_desires.cpp \
desire_damages.cpp \
impl.cpp 

libdesire_la_MOC = \
defaultdesire.h \
misc_desires.h

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

plugin_INCLUDES = $(INCLUDES_FIGHT) $(INCLUDES_SRC) 

libdesire_la_LIBADD = \
$(LIBADD_FIGHT)

AM_CPPFLAGS += $(plugin_INCLUDES)
