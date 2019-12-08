lib_LTLIBRARIES = libsearcher.la 

include $(top_srcdir)/src/Makefile.inc
plugin_INCLUDES = \
$(INCLUDES_FIGHT) \
-I$(top_srcdir)/plug-ins/skills_impl \
-I$(top_srcdir)/plug-ins/updates \
-I$(top_srcdir)/plug-ins/areas \
-I$(top_srcdir)/plug-ins/ed \
-I$(top_srcdir)/plug-ins/ed/lib \
-I/usr/include/jsoncpp \
$(INCLUDES_SRC) 

AM_YFLAGS = -d

libsearcher_la_LIBADD = \
$(LIBADD_FIGHT) \
../ed/libcoreed.la \
../ed/lib/libed.la \
../areas/libareas.la \
../skills_impl/libskills_impl.la \
../updates/libupdates.la \
-ljsoncpp

libsearcher_la_SOURCES = \
searcher.cpp \
searcher_parse.y++ \
searcher_scan.l++

libsearcher_la_MOC = 

CLEANFILES = searcher_scan.c++ \
searcher_parse.c++ \
searcher_parse.h++ 

include $(top_srcdir)/plug-ins/Makefile.inc
AM_CPPFLAGS += $(plugin_INCLUDES)
