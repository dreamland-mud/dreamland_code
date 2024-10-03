lib_LTLIBRARIES = libquest_heal.la 

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

plugin_INCLUDES = \
-I$(srcdir)/../core \
-I$(top_srcdir)/plug-ins/skills_impl \
$(INCLUDES_AI) \
$(INCLUDES_SRC)

libquest_heal_la_LIBADD = \
../../skills_impl/libskills_impl.la \
../core/libquest_core.la \
$(LIBADD_AI)

libquest_heal_la_SOURCES = \
impl_healquest.cpp \
healquest.cpp \
scenarios.cpp \
patientbehavior.cpp

libquest_heal_la_MOC = \
healquest.h \
scenarios.h \
patientbehavior.h


AM_CPPFLAGS += $(plugin_INCLUDES)
