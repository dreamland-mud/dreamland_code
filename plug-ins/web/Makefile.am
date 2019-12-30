lib_LTLIBRARIES = libweb.la 

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

libweb_la_SOURCES = \
impl.cpp \
maniplist.cpp \
webitemmanip.cpp \
webcharmanip.cpp \
webtextmanip.cpp

libweb_la_MOC = 


plugin_INCLUDES = \
-I$(top_srcdir)/plug-ins/ed \
-I$(top_srcdir)/plug-ins/ed/lib \
-I$(top_srcdir)/plug-ins/updates \
-I$(top_srcdir)/plug-ins/traverse \
-I$(top_srcdir)/plug-ins/ai \
-I$(top_srcdir)/plug-ins/mlove \
-I$(top_srcdir)/plug-ins/servlets \
-I$(top_srcdir)/plug-ins/clan/core \
-I$(top_srcdir)/plug-ins/quest/core \
-I$(top_srcdir)/plug-ins/services/core \
-I$(top_srcdir)/plug-ins/services/petshop \
-I$(top_srcdir)/plug-ins/services/shop \
-I/usr/include/jsoncpp \
$(INCLUDES_AI) \
$(INCLUDES_SRC)

libweb_la_LIBADD = \
$(LIBADD_AI) \
../alignment/libalignment.la \
../ed/lib/libed.la \
../ed/libcoreed.la \
../updates/libupdates.la \
../mlove/libmlove.la \
../servlets/libservlets.la \
../clan/core/libclan_core.la \
../quest/core/libquest_core.la \
../services/core/libservices_core.la \
../services/shop/libservices_shop.la \
../services/petshop/libservices_petshop.la \
-ljsoncpp

AM_CPPFLAGS += $(plugin_INCLUDES)
