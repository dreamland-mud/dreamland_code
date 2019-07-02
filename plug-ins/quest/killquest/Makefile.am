lib_LTLIBRARIES = libquest_kill.la

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

plugin_INCLUDES = \
-I$(srcdir)/../core \
-I$(top_srcdir)/plug-ins/selfrate \
$(INCLUDES_AI) \
$(INCLUDES_SRC)

libquest_kill_la_LIBADD = \
../../selfrate/libselfrate.la \
../core/libquest_core.la \
$(LIBADD_AI)


libquest_kill_la_SOURCES = \
impl_killquest.cpp \
killquest.cpp \
victimbehavior.cpp 


libquest_kill_la_MOC = \
killquest.h \
victimbehavior.h 


AM_CPPFLAGS += $(plugin_INCLUDES)
