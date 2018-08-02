lib_LTLIBRARIES = libquest_locate.la

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

INCLUDES = \
-I$(srcdir)/../core \
-I$(top_srcdir)/plug-ins/selfrate \
$(INCLUDES_AI) \
$(INCLUDES_SRC)

libquest_locate_la_LIBADD = \
../../selfrate/libselfrate.la \
../core/libquest_core.la \
$(LIBADD_AI)

libquest_locate_la_SOURCES = \
impl_locatequest.cpp \
locatequest.cpp \
objects.cpp \
mobiles.cpp \
scenarios.cpp \
scenarios_impl.cpp \
algorithms.cpp 


libquest_locate_la_MOC = \
locatequest.h \
objects.h \
mobiles.h \
scenarios.h \
scenarios_impl.h \
algorithms.h 


