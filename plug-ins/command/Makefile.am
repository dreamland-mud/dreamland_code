lib_LTLIBRARIES = libcommand.la 

include $(top_srcdir)/src/Makefile.inc
include $(top_srcdir)/plug-ins/Makefile.inc
plugin_INCLUDES = \
-I/usr/include/jsoncpp \
$(INCLUDES_SRC) \
-I$(top_srcdir)/plug-ins/interpret \
-I$(top_srcdir)/plug-ins/descriptor \
-I$(top_srcdir)/plug-ins/output \
-I$(top_srcdir)/plug-ins/loadsave \
-I$(top_srcdir)/plug-ins/help \
-I$(top_srcdir)/plug-ins/system 

libcommand_la_LIBADD = \
../system/libsystem.la \
../interpret/libinterpret.la \
../descriptor/libdescriptor.la \
../output/liboutput.la \
../help/libhelp.la \
../loadsave/libloadsave.la \
-ljsoncpp


libcommand_la_SOURCES = \
commandflags.cpp \
commandloader.cpp \
commandmanager.cpp \
command.cpp \
wrappedcommand.cpp \
commandelement.cpp \
commandplugin.cpp \
commandpluginloader.cpp \
commandhelp.cpp \
admincommand.cpp \
levenshtein.cpp \
impl.cpp 

libcommand_la_MOC = \
command.h \
commandelement.h

commandflags.cpp: commandflags.conf commandflags.h
commandflags.h: commandflags.conf
libcommand_la_moc_xml.cpp: commandflags.h

CLEANFILES = commandflags.cpp $(top_builddir)/src/commandflags.h 

AM_CPPFLAGS += $(plugin_INCLUDES)
