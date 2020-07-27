
lib_LTLIBRARIES = libareas.la

include $(top_srcdir)/src/Makefile.inc

plugin_INCLUDES = $(INCLUDES_SRC) \
-I$(top_srcdir)/plug-ins/interpret \
-I$(top_srcdir)/plug-ins/command  \
-I$(top_srcdir)/plug-ins/help \
-I$(top_srcdir)/plug-ins/descriptor \
-I$(top_srcdir)/plug-ins/output \
-I$(top_srcdir)/plug-ins/system \
-I$(top_srcdir)/plug-ins/iomanager \
-I$(top_srcdir)/plug-ins/loadsave \
-I$(top_srcdir)/plug-ins/movement

libareas_la_SOURCES = \
db2.cpp \
load_compat.cpp \
load_xml.cpp \
xmlareahelp.cpp \
xmlitemtype.cpp \
xmlmisc.cpp \
xmlarea.cpp \
xmlroom.cpp \
xmlobjectfactory.cpp \
xmlmobilefactory.cpp

libareas_la_LIBADD = \
../loadsave/libloadsave.la \
../output/liboutput.la \
../command/libcommand.la \
../help/libhelp.la \
../system/libscheduler_task_round.la \
../movement/libmovement.la \
../iomanager/libiomanager.la

libareas_la_MOC = \
xmlitemtype.h \
xmlmisc.h \
xmlarea.h \
xmlroom.h \
xmlobjectfactory.h \
xmlmobilefactory.h \
xmlareahelp.h

include $(top_srcdir)/plug-ins/Makefile.inc
AM_CPPFLAGS += $(plugin_INCLUDES)
