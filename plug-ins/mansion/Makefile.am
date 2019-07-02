lib_LTLIBRARIES = libmansion.la

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

plugin_INCLUDES = \
-I$(top_srcdir)/plug-ins/services/core \
$(INCLUDES_ANATOLIA) \
$(INCLUDES_SRC) 

libmansion_la_LIBADD = \
../services/core/libservices_core.la \
$(LIBADD_ANATOLIA)

libmansion_la_SOURCES = \
homerecall.cpp \
mkey.cpp \
impl.cpp 

libmansion_la_MOC = \
homerecall.h \
mkey.h 

AM_CPPFLAGS += $(plugin_INCLUDES)
