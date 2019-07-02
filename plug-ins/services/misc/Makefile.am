lib_LTLIBRARIES = libservices_misc.la 

libservices_misc_la_SOURCES = \
healer.cpp \
smithman.cpp \
impl.cpp

libservices_misc_la_MOC = \
healer.h \
smithman.h

include $(top_srcdir)/src/Makefile.inc
include $(top_srcdir)/plug-ins/Makefile.inc

plugin_INCLUDES = \
-I$(srcdir)/../core \
$(INCLUDES_AI) \
$(INCLUDES_SRC) 

libservices_misc_la_LIBADD = \
$(LIBADD_AI) \
../core/libservices_core.la 


AM_CPPFLAGS += $(plugin_INCLUDES)
