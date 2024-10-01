lib_LTLIBRARIES = libquest_big.la

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

plugin_INCLUDES = \
-I$(srcdir)/../core \
$(INCLUDES_AI) \
$(INCLUDES_SRC)

libquest_big_la_LIBADD = \
../core/libquest_core.la \
$(LIBADD_AI)


libquest_big_la_SOURCES = \
impl_bigquest.cpp \
bigquest.cpp \
bandamobile.cpp


libquest_big_la_MOC = \
bigquest.h \
bandamobile.h


AM_CPPFLAGS += $(plugin_INCLUDES)
