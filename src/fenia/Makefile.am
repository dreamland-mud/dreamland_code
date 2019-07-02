#
# ruffina, DreamLand, 2004
# unicorn, Forgotten Dungeon, 2004
noinst_LTLIBRARIES = libfenia.la

bin_PROGRAMS = fenia

src_INCLUDES = -I$(srcdir) -I$(top_builddir) \
-I$(srcdir)/../xml \
-I$(srcdir)/../lang \
-I$(srcdir)/../alloc \
-I$(srcdir)/../io \
-I$(srcdir)/../util 

AM_YFLAGS = -d
AM_LFLAGS =  -olex.yy.c 
#-S$(srcdir)/../../admin/flex.skl
AM_CPPFLAGS = -Wall

libfenia_la_SOURCES = \
context.cpp \
codesource.cpp \
codesourceref.cpp \
exp-tree.cpp \
function.cpp \
impl.cpp \
lex.cpp \
nodes.cpp \
object.cpp \
handler.cpp \
parse.y++ \
phase.cpp \
ref-tree.cpp \
reference.cpp \
scan.l++ \
scope.cpp \
stmt-tree.cpp \
xmlregister.cpp \
exceptions.cpp \
feniastring.cpp \
closure.cpp \
native.cpp

libfenia_la_MOC = xmlregister.h 

CLEANFILES = parse.c++ scan.c++ parse.h++

fenia_LDADD = libfenia.la \
    ../xml/libxml.la \
    ../io/libio.la \
    ../lang/liblang.la \
    ../util/libutil.la \
    ../alloc/liballoc.la \
    ../l10n/libl10n.la \
    @BDB_LIBS@ 

fenia_SOURCES = main.cpp test.cpp

fenia_MOC = test.h 
AM_CPPFLAGS += $(src_INCLUDES)
