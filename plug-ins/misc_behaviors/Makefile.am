lib_LTLIBRARIES = libmisc_behaviors.la 

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

plugin_INCLUDES = \
-I$(top_srcdir)/plug-ins/quest/core \
-I$(top_srcdir)/plug-ins/quest/command \
-I$(top_srcdir)/plug-ins/services/core \
$(INCLUDES_AI) \
$(INCLUDES_SRC) 

libmisc_behaviors_la_LIBADD = \
../services/core/libservices_core.la \
../quest/core/libquest_core.la \
../quest/command/libquest_command.la \
$(LIBADD_AI)

libmisc_behaviors_la_SOURCES = \
midgaardfountain.cpp \
ofcolguards.cpp \
special.cpp \
petquestor.cpp \
impl.cpp 

libmisc_behaviors_la_MOC = \
midgaardfountain.h \
ofcolguards.h \
petquestor.h 


AM_CPPFLAGS += $(plugin_INCLUDES)
