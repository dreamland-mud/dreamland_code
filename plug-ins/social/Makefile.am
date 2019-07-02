lib_LTLIBRARIES = libsocial.la 

include $(top_srcdir)/src/Makefile.inc
plugin_INCLUDES = \
$(INCLUDES_SRC) \
-I$(top_srcdir)/plug-ins/interpret \
-I$(top_srcdir)/plug-ins/command \
-I$(top_srcdir)/plug-ins/help \
-I$(top_srcdir)/plug-ins/loadsave \
-I$(top_srcdir)/plug-ins/descriptor \
-I$(top_srcdir)/plug-ins/output \
-I$(top_srcdir)/plug-ins/system \
-I$(top_srcdir)/plug-ins/iomanager 

libsocial_la_LIBADD = \
../system/libsystem.la \
../loadsave/libloadsave.la \
../interpret/libinterpret.la \
../descriptor/libdescriptor.la \
../output/liboutput.la \
../command/libcommand.la \
../help/libhelp.la \
../iomanager/libiomanager.la

libsocial_la_SOURCES = \
customsocial.cpp \
socialmanager.cpp  \
socialbase.cpp  \
social.cpp \
csocials.cpp \
mysocial.cpp \
impl.cpp


libsocial_la_MOC = \
customsocial.h \
mysocial.h \
social.h 


include $(top_srcdir)/plug-ins/Makefile.inc
AM_CPPFLAGS += $(plugin_INCLUDES)
