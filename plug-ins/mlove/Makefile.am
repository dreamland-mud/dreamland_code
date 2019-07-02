lib_LTLIBRARIES = libmlove.la 

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

plugin_INCLUDES = \
-I$(top_srcdir)/plug-ins/interpret \
-I$(top_srcdir)/plug-ins/command  \
-I$(top_srcdir)/plug-ins/descriptor \
-I$(top_srcdir)/plug-ins/output \
-I$(top_srcdir)/plug-ins/loadsave \
-I$(top_srcdir)/plug-ins/help \
-I$(top_srcdir)/plug-ins/system \
$(INCLUDES_SRC)

libmlove_la_LIBADD = \
../descriptor/libdescriptor.la \
../output/liboutput.la \
../system/libsystem.la \
../iomanager/libiomanager.la \
../interpret/libinterpret.la \
../command/libcommand.la \
../help/libhelp.la \
../loadsave/libloadsave.la 

libmlove_la_SOURCES = \
xmllovers.cpp \
xmlattributelovers.cpp \
xmlattributemarriage.cpp \
lover.cpp \
mlove_impl.cpp \
mlove.cpp \
marry.cpp \
divorce.cpp \
mtalk.cpp 

libmlove_la_MOC = \
xmlattributelovers.h \
xmllovers.h \
xmlattributemarriage.h 


AM_CPPFLAGS += $(plugin_INCLUDES)
