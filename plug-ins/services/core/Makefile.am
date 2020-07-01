lib_LTLIBRARIES = libservices_core.la 

include $(top_srcdir)/src/Makefile.inc
plugin_INCLUDES = \
$(INCLUDES_SRC) \
-I$(top_srcdir)/plug-ins/descriptor \
-I$(top_srcdir)/plug-ins/output \
-I$(top_srcdir)/plug-ins/system

libservices_core_la_LIBADD = \
../../output/liboutput.la \
../../loadsave/libloadsave.la \
../../descriptor/libdescriptor.la

libservices_core_la_SOURCES = \
article.cpp \
price.cpp \
service.cpp \
trader.cpp \
servicetrader.cpp \
impl.cpp

libservices_core_la_MOC = \
price.h \
servicetrader.h


include $(top_srcdir)/plug-ins/Makefile.inc
AM_CPPFLAGS += $(plugin_INCLUDES)
