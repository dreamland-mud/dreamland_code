lib_LTLIBRARIES = libgmcp.la 

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

libgmcp_la_SOURCES = \
impl.cpp 

libgmcp_la_MOC = 


plugin_INCLUDES = \
-I$(top_srcdir)/plug-ins/ed \
-I$(top_srcdir)/plug-ins/ed/lib \
-I$(top_srcdir)/plug-ins/traverse \
-I/usr/include/jsoncpp \
$(INCLUDES_AI) \
$(INCLUDES_SRC)

libgmcp_la_LIBADD = \
$(LIBADD_AI) \
../ed/lib/libed.la \
../ed/libcoreed.la \
-ljsoncpp

AM_CPPFLAGS += $(plugin_INCLUDES)
