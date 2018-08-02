#
# ruffina, DreamLand, 2003
# NoFate, 2002
bin_PROGRAMS = xmlreftest xmltest

noinst_LTLIBRARIES = libxml.la

INCLUDES = \
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
xmlbyte.cpp \
xmlchar.cpp \
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
xmlvariablecontainer.cpp 

xmlreftest_SOURCES = xmlreftest.cpp
xmlreftest_LDADD = libxml.la ../lang/liblang.la ../util/libutil.la ../l10n/libl10n.la ../alloc/liballoc.la
xmlreftest_MOC = xmlreftest.h

xmltest_SOURCES = xmltest.cpp
xmltest_LDADD = libxml.la ../lang/liblang.la ../util/libutil.la ../l10n/libl10n.la ../alloc/liballoc.la

CLEANFILES = xmlmatchpattern.c++ xmlparser.c++
