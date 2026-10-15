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
tattoo.cpp \
gods_impl.cpp \
religionattribute.cpp \
altar.cpp \
sacrifice.cpp

libreligion_la_MOC = \
defaultreligion.h \
tattoo.h \
gods_impl.h \
religionattribute.h

religionflags.cpp: religionflags.conf $(top_builddir)/src/religionflags.h
$(top_builddir)/src/religionflags.h: religionflags.conf
	$(MAKEBITS) -s $< -h -r -f $@
libreligion_la_moc_xml.cpp: $(top_builddir)/src/religionflags.h

CLEANFILES = religionflags.cpp $(top_builddir)/src/religionflags.h 
AM_CPPFLAGS += $(plugin_INCLUDES)
