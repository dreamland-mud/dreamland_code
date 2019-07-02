#
# ruffina, DreamLand, 2008
#
noinst_LTLIBRARIES = libprocess.la

src_INCLUDES = \
-I$(top_builddir) \
-I$(srcdir) \
-I$(srcdir)/../lang \
-I$(srcdir)/../util \
-I$(srcdir)/../xml \
-I$(srcdir)/../l10n

AM_CPPFLAGS =  -Wall

libprocess_la_SOURCES = \
thread.cpp \
mutex.cpp \
process.cpp \
monitor.cpp 

AM_CPPFLAGS += $(src_INCLUDES)
