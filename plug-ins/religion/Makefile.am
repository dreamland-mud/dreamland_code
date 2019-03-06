lib_LTLIBRARIES = \
libreligion.la

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

INCLUDES = \
$(INCLUDES_AI) \
$(INCLUDES_SRC)

libreligion_la_LIBADD = \
../liquid/libliquid.la \
$(LIBADD_AI)

libreligion_la_SOURCES = \
impl.cpp \
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

