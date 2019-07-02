#
# ruffina, DreamLand, 2003
# NoFate, 2002
noinst_LTLIBRARIES = libutil.la

AM_CPPFLAGS = -Wall

src_INCLUDES = \
-I$(srcdir)/../lang \
-I$(srcdir)/../regex \
-I$(srcdir)/../l10n \
-I$(top_srcdir)

AM_LFLAGS =  -olex.yy.c
#-S$(srcdir)/../../admin/flex.skl

libutil_la_SOURCES = \
crypto.cpp \
date.cpp \
logstream.cpp \
lastlogstream.cpp \
parsedate.l++ \
regexp.cpp \
iconvmap.cpp \
dl_match.cpp \
dl_math.cpp \
dl_strings.cpp \
dl_ctype.cpp \
profiler.cpp \
timer.cpp


CLEANFILES = parsedate.c++
AM_CPPFLAGS += $(src_INCLUDES)
