#
# ruffina, DreamLand, 2008
#
noinst_LTLIBRARIES = liballoc.la

src_INCLUDES = \
-I$(srcdir) \
-I$(top_builddir) \
-I$(srcdir)/../lang \
-I$(srcdir)/../util \
-I$(srcdir)/../l10n

AM_CPPFLAGS =  -Wall

liballoc_la_SOURCES = \
allocateclass.cpp \
class.cpp \
oneallocate.cpp \
exceptionclassnotfound.cpp
AM_CPPFLAGS += $(src_INCLUDES)
