#
# ruffina, DreamLand, 2003
# NoFate, 2002
noinst_LTLIBRARIES = liblang.la

src_INCLUDES = \
-I$(top_builddir) \
-I$(srcdir) \
-I$(srcdir)/../util \
-I$(srcdir)/../l10n

AM_CPPFLAGS =  -Wall

liblang_la_SOURCES = \
boolean.cpp \
dlobject.cpp \
dlstring.cpp \
exception.cpp \
integer.cpp \
long.cpp \
longlong.cpp \
short.cpp \
stringset.cpp \
stringlist.cpp

AM_CPPFLAGS += $(src_INCLUDES)
