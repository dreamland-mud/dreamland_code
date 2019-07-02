lib_LTLIBRARIES = \
libhometown.la

include $(top_srcdir)/src/Makefile.inc
plugin_INCLUDES = \
$(INCLUDES_SRC) \
-I$(top_srcdir)/plug-ins/system

libhometown_la_SOURCES = \
impl.cpp \
defaulthometown.cpp 

libhometown_la_MOC = \
defaulthometown.h

libhometown_la_LIBADD = \
../system/libsystem.la

include $(top_srcdir)/plug-ins/Makefile.inc
AM_CPPFLAGS += $(plugin_INCLUDES)
