#
# ruffina, DreamLand, 2008
#
noinst_LTLIBRARIES = libflags.la

src_INCLUDES = \
-I$(srcdir) \
-I$(top_builddir) \
-I$(srcdir)/../lang \
-I$(srcdir)/../alloc \
-I$(srcdir)/../util \
-I$(srcdir)/../xml \
-I$(srcdir)/../l10n

AM_CPPFLAGS =  -Wall

libflags_la_SOURCES = \
flagtable.cpp \
flagtableregistry.cpp \
flags.cpp \
enumeration.cpp \
xmlflags.cpp \
xmlenumeration.cpp 

AM_CPPFLAGS += $(src_INCLUDES)
