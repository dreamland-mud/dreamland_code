lib_LTLIBRARIES = libclan_command.la

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

plugin_INCLUDES = \
-I$(srcdir)/../core \
-I$(top_srcdir)/plug-ins/communication \
$(INCLUDES_ANATOLIA) \
$(INCLUDES_SRC)

libclan_command_la_SOURCES = \
cclan.cpp \
xmlattributeinduct.cpp \
cclantalk.cpp \
impl.cpp 

libclan_command_la_MOC = \
cclan.h \
xmlattributeinduct.h 

libclan_command_la_LIBADD = \
$(LIBADD_ANATOLIA) \
../../communication/libcommunication.la \
../core/libclan_core.la


AM_CPPFLAGS += $(plugin_INCLUDES)
