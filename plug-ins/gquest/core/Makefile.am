lib_LTLIBRARIES = libgquest_core.la

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

INCLUDES = \
$(INCLUDES_ANATOLIA) \
$(INCLUDES_SRC)

libgquest_core_la_LIBADD = \
$(LIBADD_ANATOLIA)

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


