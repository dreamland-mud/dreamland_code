lib_LTLIBRARIES = libgquest_gangsters.la 

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

plugin_INCLUDES = \
-I$(top_srcdir)/plug-ins/gquest/core \
$(INCLUDES_AI) \
$(INCLUDES_SRC)

libgquest_gangsters_la_LIBADD = \
$(LIBADD_AI) \
../core/libgquest_core.la

libgquest_gangsters_la_SOURCES = \
gangstersinfo.cpp \
gangsters.cpp \
gangmob.cpp \
gangchef.cpp \
objects.cpp \
xmlattributegangsters.cpp \
impl.cpp 

libgquest_gangsters_la_MOC = \
gangsters.h \
gangstersinfo.h \
objects.h \
xmlattributegangsters.h \
gangchef.h \
gangmob.h 


AM_CPPFLAGS += $(plugin_INCLUDES)
