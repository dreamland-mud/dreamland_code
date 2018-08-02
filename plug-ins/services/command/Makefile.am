lib_LTLIBRARIES = libservices_command.la 

libservices_command_la_SOURCES = \
impl.cpp

include $(top_srcdir)/src/Makefile.inc
include $(top_srcdir)/plug-ins/Makefile.inc

INCLUDES = \
-I$(srcdir)/../core \
$(INCLUDES_ANATOLIA) \
$(INCLUDES_SRC) 

libservices_command_la_LIBADD = \
$(LIBADD_ANATOLIA) \
../core/libservices_core.la

