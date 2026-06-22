lib_LTLIBRARIES = libremort.la

libremort_la_SOURCES = \
cmlt.cpp \
impl.cpp \
cremort.cpp \
remortnanny.cpp


include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

plugin_INCLUDES = \
$(INCLUDES_ANATOLIA) \
$(INCLUDES_SRC)

libremort_la_LIBADD = \
$(LIBADD_ANATOLIA)

AM_CPPFLAGS += $(plugin_INCLUDES)
