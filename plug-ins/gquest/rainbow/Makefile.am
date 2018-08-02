lib_LTLIBRARIES = libgquest_rainbow.la 

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

INCLUDES = \
-I$(top_srcdir)/plug-ins/gquest/core \
$(INCLUDES_AI) \
$(INCLUDES_SRC)

libgquest_rainbow_la_LIBADD = \
$(LIBADD_AI) \
../core/libgquest_core.la

libgquest_rainbow_la_SOURCES = \
rainbowinfo.cpp \
rainbow.cpp \
mobiles.cpp \
objects.cpp \
scenarios.cpp \
impl.cpp 

libgquest_rainbow_la_MOC = \
rainbow.h \
rainbowinfo.h \
mobiles.h \
objects.h \
scenarios.h


