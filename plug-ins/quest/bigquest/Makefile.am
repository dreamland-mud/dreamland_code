lib_LTLIBRARIES = libquest_big.la

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

INCLUDES = \
-I$(srcdir)/../core \
-I$(top_srcdir)/plug-ins/selfrate \
$(INCLUDES_AI) \
$(INCLUDES_SRC)

libquest_big_la_LIBADD = \
../../selfrate/libselfrate.la \
../core/libquest_core.la \
$(LIBADD_AI)


libquest_big_la_SOURCES = \
impl_bigquest.cpp \
bigquest.cpp \
bigvictimbehavior.cpp \
bandamobile.cpp


libquest_big_la_MOC = \
bigquest.h \
bigvictimbehavior.h \
bandamobile.h


