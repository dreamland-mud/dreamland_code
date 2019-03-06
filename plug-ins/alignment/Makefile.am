lib_LTLIBRARIES = \
libalignment.la

include $(top_srcdir)/src/Makefile.inc
INCLUDES = \
$(INCLUDES_SRC) \
-I$(top_srcdir)/plug-ins/output \
-I$(top_srcdir)/plug-ins/descriptor


libalignment_la_SOURCES = \
impl.cpp \
alignment.cpp


libalignment_la_LIBADD = \
../output/liboutput.la \
../descriptor/libdescriptor.la

include $(top_srcdir)/plug-ins/Makefile.inc
