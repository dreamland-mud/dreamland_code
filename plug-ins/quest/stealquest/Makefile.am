lib_LTLIBRARIES = libquest_steal.la

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

plugin_INCLUDES = \
-I$(srcdir)/../core \
$(INCLUDES_AI) \
$(INCLUDES_SRC)

libquest_steal_la_LIBADD = \
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


AM_CPPFLAGS += $(plugin_INCLUDES)
