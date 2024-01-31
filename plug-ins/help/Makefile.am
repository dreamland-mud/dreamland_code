lib_LTLIBRARIES = \
libhelp.la

include $(top_srcdir)/src/Makefile.inc
plugin_INCLUDES = \
$(INCLUDES_SRC) \
-I$(top_srcdir)/plug-ins/system \
-I$(top_srcdir)/plug-ins/descriptor \
-I$(top_srcdir)/plug-ins/output \
-I/usr/include/jsoncpp 

libhelp_la_LIBADD = \
../descriptor/libdescriptor.la \
../system/libsystem.la \
../output/liboutput.la \
-ljsoncpp

libhelp_la_SOURCES = \
helpformatter.cpp \
markuphelparticle.cpp \
helpcontainer.cpp \
bugtracker.cpp \
areahelp.cpp \
impl.cpp 

libhelp_la_MOC = \
bugtracker.h \
areahelp.h \
helpcontainer.h


include $(top_srcdir)/plug-ins/Makefile.inc
AM_CPPFLAGS += $(plugin_INCLUDES)
