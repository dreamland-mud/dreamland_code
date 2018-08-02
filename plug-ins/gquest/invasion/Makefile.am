lib_LTLIBRARIES = libgquest_invasion.la 

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

INCLUDES = \
-I$(top_srcdir)/plug-ins/gquest/core \
$(INCLUDES_AI) \
$(INCLUDES_SRC)

libgquest_invasion_la_LIBADD = \
$(LIBADD_AI) \
../core/libgquest_core.la

libgquest_invasion_la_SOURCES = \
invasioninfo.cpp \
invasion.cpp \
scenarios.cpp \
mobiles.cpp \
objects.cpp \
xmlattributeinvasion.cpp \
impl.cpp 

libgquest_invasion_la_MOC = \
invasion.h \
invasioninfo.h \
scenarios.h \
mobiles.h \
objects.h \
xmlattributeinvasion.h 


