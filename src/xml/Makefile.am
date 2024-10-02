#
# ruffina, DreamLand, 2003
# NoFate, 2002

noinst_LTLIBRARIES = libxml.la

src_INCLUDES = \
-I$(srcdir) \
-I$(top_builddir) \
-I$(srcdir)/../lang \
-I$(srcdir)/../l10n \
-I$(srcdir)/../alloc \
-I$(srcdir)/../util 

AM_CPPFLAGS = -Wall -O0

AM_LFLAGS = -olex.yy.c 
#-S../${top_srcdir}/admin/flex.skl

libxml_la_SOURCES = \
exceptions.cpp \
xmlboolean.cpp \
xmlclause.cpp \
xmlcontainer.cpp \
xmldate.cpp \
xmldocument.cpp \
xmlinteger.cpp \
xmllimits.cpp \
xmllong.cpp \
xmllonglong.cpp \
xmlmatchpattern.l++ \
xmlnode.cpp \
xmlparser.l++ \
xmlpolymorphvariable.cpp \
xmlref.cpp \
xmlpredicate.cpp \
xmlshort.cpp \
xmlstring.cpp \
xmlstringlist.cpp \
xmltimestamp.cpp \
xmlvariable.cpp \
xmlvariablecontainer.cpp \
xmljsonvalue.cpp \
xmlcounter.cpp \
xmlmultistring.cpp

CLEANFILES = xmlmatchpattern.c++ xmlparser.c++
AM_CPPFLAGS += $(src_INCLUDES)
