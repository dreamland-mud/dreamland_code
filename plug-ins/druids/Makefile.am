lib_LTLIBRARIES = libdruids.la 

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

plugin_INCLUDES = \
$(INCLUDES_AI) \
-I$(top_srcdir)/plug-ins/languages/core \
-I$(top_srcdir)/plug-ins/skills_impl \
$(INCLUDES_SRC)

libdruids_la_LIBADD = \
$(LIBADD_AI) \
../languages/core/liblanguages_core.la \
../skills_impl/libskills_impl.la

libdruids_la_SOURCES = \
class_druid.cpp \
druidic.cpp \
druidic_effects.cpp \
impl.cpp

libdruids_la_MOC = \
class_druid.h \
druidic.h \
druidic_effects.h


AM_CPPFLAGS += $(plugin_INCLUDES)
