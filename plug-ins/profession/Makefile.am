lib_LTLIBRARIES = \
libprofession.la

include $(top_srcdir)/src/Makefile.inc
plugin_INCLUDES = \
$(INCLUDES_SRC) \
-I$(top_srcdir)/plug-ins/system \
-I$(top_srcdir)/plug-ins/alignment \
-I$(top_srcdir)/plug-ins/help


libprofession_la_LIBADD = ../system/libsystem.la \
../help/libhelp.la \
../alignment/libalignment.la

libprofession_la_SOURCES = \
profflags.cpp \
defaultprofession.cpp \
mobileprofession.cpp \
impl.cpp 

libprofession_la_MOC = \
defaultprofession.h \
mobileprofession.h 


include $(top_srcdir)/plug-ins/Makefile.inc

profflags.cpp: profflags.conf profflags.h
profflags.h: profflags.conf
libprofession_la_moc_xml.cpp: profflags.h

CLEANFILES = profflags.cpp $(top_builddir)/src/profflags.h 
AM_CPPFLAGS += $(plugin_INCLUDES)
