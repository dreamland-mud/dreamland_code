lib_LTLIBRARIES = libquest_core.la 

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

plugin_INCLUDES = \
-I$(top_srcdir)/plug-ins/selfrate \
$(INCLUDES_AI) \
$(INCLUDES_SRC)

libquest_core_la_LIBADD = \
../../selfrate/libselfrate.la \
$(LIBADD_AI)

libquest_core_la_SOURCES = \
impl.cpp \
questexceptions.cpp \
questmanager.cpp \
quest.cpp \
questmodels.cpp \
questregistrator.cpp \
questentity.cpp \
questscenario.cpp \
mobquestbehavior.cpp \
objquestbehavior.cpp \
xmlattributequestdata.cpp 

libquest_core_la_MOC = \
questmanager.h \
quest.h \
questregistrator.h \
questscenario.h \
mobquestbehavior.h \
objquestbehavior.h \
xmlattributequestdata.h 


AM_CPPFLAGS += $(plugin_INCLUDES)
