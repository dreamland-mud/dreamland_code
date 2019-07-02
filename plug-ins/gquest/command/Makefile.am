lib_LTLIBRARIES = libgquest_command.la

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

plugin_INCLUDES = \
-I$(srcdir)/../core \
$(INCLUDES_ANATOLIA) \
$(INCLUDES_SRC)

libgquest_command_la_LIBADD = \
$(LIBADD_ANATOLIA) \
../core/libgquest_core.la


libgquest_command_la_SOURCES = \
cgquest.cpp \
gquestnotifyplugin.cpp \
impl.cpp 

AM_CPPFLAGS += $(plugin_INCLUDES)
