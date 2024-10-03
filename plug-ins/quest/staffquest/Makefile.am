lib_LTLIBRARIES = libquest_staff.la 

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

plugin_INCLUDES = \
-I$(srcdir)/../core \
$(INCLUDES_AI) \
$(INCLUDES_SRC)

libquest_staff_la_LIBADD = \
../core/libquest_core.la \
$(LIBADD_AI)

libquest_staff_la_SOURCES = \
impl_staffquest.cpp \
staffquest.cpp \
staffbehavior.cpp

libquest_staff_la_MOC = \
staffquest.h \
staffbehavior.h


AM_CPPFLAGS += $(plugin_INCLUDES)
