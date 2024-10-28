lib_LTLIBRARIES = libservices_shop.la 


libservices_shop_la_SOURCES = \
oldshop.cpp \
shoptrader.cpp \
impl.cpp

libservices_shop_la_MOC = \
shoptrader.h


include $(top_srcdir)/src/Makefile.inc
include $(top_srcdir)/plug-ins/Makefile.inc

plugin_INCLUDES = \
-I$(srcdir)/../core \
-I$(srcdir)/../command \
$(INCLUDES_AI) \
$(INCLUDES_SRC) 

libservices_shop_la_LIBADD = \
$(LIBADD_AI) \
../core/libservices_core.la 


AM_CPPFLAGS += $(plugin_INCLUDES)
