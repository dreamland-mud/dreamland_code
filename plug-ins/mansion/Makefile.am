lib_LTLIBRARIES = libmansion.la

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

plugin_INCLUDES = \
$(INCLUDES_ANATOLIA) \
$(INCLUDES_SRC) 

libmansion_la_LIBADD = \
$(LIBADD_ANATOLIA)

libmansion_la_SOURCES = \
homerecall.cpp \
mkey.cpp \
impl.cpp 

libmansion_la_MOC = \
homerecall.h \
mkey.h 

AM_CPPFLAGS += $(plugin_INCLUDES)
