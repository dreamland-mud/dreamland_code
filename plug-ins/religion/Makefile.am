lib_LTLIBRARIES = \
libreligion.la

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

plugin_INCLUDES = \
$(INCLUDES_AI) \
$(INCLUDES_SRC)

libreligion_la_LIBADD = \
../liquid/libliquid.la \
$(LIBADD_AI)

libreligion_la_SOURCES = \
impl.cpp \
religionflags.cpp \
defaultreligion.cpp \
templeman.cpp \
tattoo.cpp \
gods_impl.cpp \
religionattribute.cpp \
altar.cpp \
sacrifice.cpp

libreligion_la_MOC = \
defaultreligion.h \
templeman.h \
tattoo.h \
gods_impl.h \
religionattribute.h

religionflags.cpp: religionflags.conf religionflags.h
religionflags.h: religionflags.conf
libreligion_la_moc_xml.cpp: religionflags.h

CLEANFILES = religionflags.cpp $(top_builddir)/src/religionflags.h 
AM_CPPFLAGS += $(plugin_INCLUDES)
