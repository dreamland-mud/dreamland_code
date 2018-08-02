lib_LTLIBRARIES = libquest_steal.la

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

INCLUDES = \
-I$(srcdir)/../core \
-I$(top_srcdir)/plug-ins/selfrate \
$(INCLUDES_AI) \
$(INCLUDES_SRC)

libquest_steal_la_LIBADD = \
../../selfrate/libselfrate.la \
../core/libquest_core.la \
$(LIBADD_AI)

libquest_steal_la_SOURCES = \
stealquest.cpp \
objects.cpp \
mobiles.cpp \
impl_stealquest.cpp 


libquest_steal_la_MOC = \
stealquest.h \
objects.h \
mobiles.h 


