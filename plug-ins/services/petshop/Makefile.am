lib_LTLIBRARIES = libservices_petshop.la 

libservices_petshop_la_SOURCES = \
pet.cpp \
impl.cpp

libservices_petshop_la_MOC = \
pet.h


include $(top_srcdir)/src/Makefile.inc
include $(top_srcdir)/plug-ins/Makefile.inc

plugin_INCLUDES = \
-I$(srcdir)/../core \
$(INCLUDES_AI) \
$(INCLUDES_SRC) 

libservices_petshop_la_LIBADD = \
$(LIBADD_AI) \
../core/libservices_core.la


AM_CPPFLAGS += $(plugin_INCLUDES)
