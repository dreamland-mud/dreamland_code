lib_LTLIBRARIES = libquest_charm.la

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

plugin_INCLUDES = \
-I$(srcdir)/../core \
$(INCLUDES_AI) \
$(INCLUDES_SRC)

libquest_charm_la_LIBADD = \
../core/libquest_core.la \
$(LIBADD_AI)


libquest_charm_la_SOURCES = \
impl_charmquest.cpp \
charmquest.cpp


libquest_charm_la_MOC = \
charmquest.h


AM_CPPFLAGS += $(plugin_INCLUDES)
