
#bin_PROGRAMS = ed
lib_LTLIBRARIES = libed.la 

plugin_INCLUDES = -I$(srcdir) \
-I$(top_builddir)/src \
-I$(top_builddir) \
-I$(top_srcdir) \
-I$(top_srcdir)/src/lang \
-I$(top_srcdir)/src/alloc \
-I$(top_srcdir)/src/util \
-I$(top_srcdir)/src/xml \
-I$(top_srcdir)/src/plugin

AM_YFLAGS = -d
AM_LFLAGS =  -olex.yy.c 
#-S$(srcdir)/../../../admin/flex.skl

EXTRA_libed_la_SOURCES = \
edscan.l++

libed_la_SOURCES = \
edparse.y++ \
ed.cpp \
impl.cpp \
xmled.cpp

CLEANFILES = edparse.c++ edscan.c++ edparse.h

edparse.y++: edscan.c++


#ed_LDADD = libed.la

#ed_SOURCES = test.cpp

include $(top_srcdir)/plug-ins/Makefile.inc
AM_CPPFLAGS += $(plugin_INCLUDES)
