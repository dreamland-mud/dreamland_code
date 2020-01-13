lib_LTLIBRARIES = libservlets.la 

include $(top_srcdir)/src/Makefile.inc
plugin_INCLUDES = \
$(INCLUDES_SRC) \
-I$(top_srcdir)/plug-ins/interpret \
-I$(top_srcdir)/plug-ins/command  \
-I$(top_srcdir)/plug-ins/ban \
-I$(top_srcdir)/plug-ins/system \
-I$(top_srcdir)/plug-ins/descriptor \
-I$(top_srcdir)/plug-ins/output \
-I/usr/include/jsoncpp

libservlets_la_LIBADD = \
../system/libsystem.la \
../interpret/libinterpret.la \
../descriptor/libdescriptor.la \
../output/liboutput.la \
../ban/libban.la \
../command/libcommand.la \
-ljsoncpp

libservlets_la_SOURCES = \
jsonservlet.cpp \
servlet_utils.cpp

include $(top_srcdir)/plug-ins/Makefile.inc
AM_CPPFLAGS += $(plugin_INCLUDES)
