#/***************************************************************************
# * Все права на этот код 'Dream Land' пренадлежат Igor {Leo} и Olga {Varda}*
# * Некоторую помощь в написании этого кода, а также своими идеями помогали:*
# *    Igor S. Petrenko     {NoFate, Demogorgon}                            *
# *    Koval Nazar          {Nazar, Redrum}                                 *
# *    Doropey Vladimir     {Reorx}                                         *
# *    Kulgeyko Denis       {Burzum}                                        *
# *    Andreyanov Aleksandr {Manwe}                                         *
# *    Zadvinsky Alexandr   {Kiddy}                                         *
# *    и все остальные, кто советовал и играл в этот MUD                    *
# ***************************************************************************/

lib_LTLIBRARIES = libdreamland.la libdreamland_impl.la

bin_PROGRAMS = dreamland

AM_LDFLAGS = -no-undefined 
AM_CXXFLAGS = -Wall
CXXLD = $(CXX)

SUBDIRS =  \
regex      \
lang       \
alloc      \
util       \
l10n       \
xml        \
flags      \
gref       \
process    \
plugin     \
scheduler  \
io         \
fenia      \
core

include $(top_srcdir)/src/Makefile.inc
src_INCLUDES = $(INCLUDES_SRC)

#####################################################
# dreamland shared library
#
libdreamland_la_SOURCES =

libdreamland_la_LIBADD = \
fenia/libfenia.la \
xml/libxml.la \
lang/liblang.la \
l10n/libl10n.la \
io/libio.la \
plugin/libplugin.la \
util/libutil.la \
scheduler/libscheduler.la \
regex/libregex.la \
alloc/liballoc.la \
gref/libgref.la \
flags/libflags.la \
process/libprocess.la \
@BDB_LIBS@ -lz @GC_LIBS@ -lcrypto++

#####################################################
# dreamland implementation library
#
libdreamland_impl_la_SOURCES = \
autoflags.cpp \
dreamland.cpp 

libdreamland_impl_la_MOC = \
dreamland.h 

libdreamland_impl_la_LIBADD = \
libdreamland.la \
version.lo \
core/libcore.la \
core/fenia/libcorefenia.la \
core/behavior/libbehavior.la \
core/scheduler/libcorescheduler.la \
core/clan/libclan.la \
core/skills/libskills.la 

#####################################################
# dreamland binary
#
dreamland_SOURCES = main.cpp 

dreamland_LDADD = libdreamland_impl.la libdreamland.la


#####################################################
# automaticaly generated sources
#
autoflags.cpp: $(srcdir)/makebits.pl $(srcdir)/bits.conf
	$(srcdir)/makebits.pl -s $(srcdir)/bits.conf -r -f $@

autoflags.h: $(srcdir)/makebits.pl $(srcdir)/bits.conf
	$(srcdir)/makebits.pl -s $(srcdir)/bits.conf -h -r -f $@

all-recursive: autoflags.h 

version.cpp: $(dreamland_OBJECTS) $(srcdir)/newvers.sh
	sh $(srcdir)/newvers.sh

CLEANFILES = autoflags.cpp autoflags.h version.cpp
AM_CPPFLAGS = $(src_INCLUDES)
