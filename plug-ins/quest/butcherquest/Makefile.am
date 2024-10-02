lib_LTLIBRARIES = libquest_butcher.la

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

plugin_INCLUDES = \
-I$(srcdir)/../core \
$(INCLUDES_AI) \
$(INCLUDES_SRC)

libquest_butcher_la_LIBADD = \
../core/libquest_core.la \
$(LIBADD_AI)

libquest_butcher_la_SOURCES = \
impl_butcherquest.cpp \
butcherquest.cpp \
steakcustomer.cpp

libquest_butcher_la_MOC = \
butcherquest.h \
steakcustomer.h


AM_CPPFLAGS += $(plugin_INCLUDES)
