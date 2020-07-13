lib_LTLIBRARIES = libgquest_core.la

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

plugin_INCLUDES = \
-I$(top_srcdir)/plug-ins/skills_impl \
$(INCLUDES_ANATOLIA) \
$(INCLUDES_SRC)

libgquest_core_la_LIBADD = \
$(LIBADD_ANATOLIA) \
../../skills_impl/libskills_impl.la


libgquest_core_la_SOURCES = \
globalquestmanager.cpp \
globalquestinfo.cpp \
globalquest.cpp \
gqchannel.cpp \
gqexceptions.cpp \
xmlattributereward.cpp \
xmlattributeglobalquest.cpp \
impl.cpp 

libgquest_core_la_MOC = \
globalquest.h \
globalquestmanager.h \
globalquestinfo.h \
xmlattributereward.h \
xmlattributeglobalquest.h 


AM_CPPFLAGS += $(plugin_INCLUDES)
