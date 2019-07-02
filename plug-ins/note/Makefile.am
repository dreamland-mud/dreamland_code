lib_LTLIBRARIES = libnotes.la libgeneric_notes.la 

include $(top_srcdir)/src/Makefile.inc
plugin_INCLUDES = \
$(INCLUDES_SRC) \
-I/usr/include/jsoncpp \
-I$(top_srcdir)/plug-ins/system \
-I$(top_srcdir)/plug-ins/ban \
-I$(top_srcdir)/plug-ins/descriptor \
-I$(top_srcdir)/plug-ins/output \
-I$(top_srcdir)/plug-ins/iomanager \
-I$(top_srcdir)/plug-ins/loadsave \
-I$(top_srcdir)/plug-ins/interpret \
-I$(top_srcdir)/plug-ins/command \
-I$(top_srcdir)/plug-ins/help \
-I$(top_srcdir)/plug-ins/ed \
-I$(top_srcdir)/plug-ins/ed/lib

AM_YFLAGS = -d

libnotes_la_LIBADD = \
../descriptor/libdescriptor.la \
../output/liboutput.la \
../interpret/libinterpret.la \
../command/libcommand.la \
../help/libhelp.la \
../system/libsystem.la \
../iomanager/libiomanager.la \
../system/libscheduler_task_round.la \
../loadsave/libloadsave.la \
../ban/libban.la \
../ed/lib/libed.la \
../ed/libcoreed.la \
-ljsoncpp

libnotes_la_SOURCES = \
impl.cpp \
note.cpp \
webnote.cpp \
noteattrs.cpp \
notemanager.cpp \
notethread.cpp \
notecommand.cpp \
unread.cpp \
dreams.cpp \
notehooks.cpp \
noteflags.cpp \
listfilter_parse.y++ \
listfilter_scan.l++

libgeneric_notes_la_LIBADD = \
../interpret/libinterpret.la \
../output/liboutput.la \
../command/libcommand.la \
../system/libsystem.la \
../system/libscheduler_task_round.la \
libnotes.la

libgeneric_notes_la_SOURCES =  genericnotes.cpp

libnotes_la_MOC = \
note.h \
webnote.h \
noteattrs.h \
notemanager.h \
notethread.h \
dreams.h \
unread.h

noteflags.cpp: noteflags.conf noteflags.h
noteflags.h: noteflags.conf
libnotes_la_moc_xml.cpp: noteflags.h

CLEANFILES = listfilter_scan.c++ \
listfilter_parse.c++ \
listfilter_parse.h++ \
noteflags.cpp \
$(top_builddir)/src/noteflags.h

include $(top_srcdir)/plug-ins/Makefile.inc
AM_CPPFLAGS += $(plugin_INCLUDES)
