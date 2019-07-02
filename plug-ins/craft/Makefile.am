lib_LTLIBRARIES = libcraft.la 

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

libcraft_la_SOURCES = \
impl.cpp \
subprofession.cpp \
craftskill.cpp \
craftattribute.cpp \
craftwearloc.cpp \
craft_utils.cpp

libcraft_la_MOC = \
subprofession.h \
craftskill.h \
craftattribute.h \
craftwearloc.h


plugin_INCLUDES = \
-I$(top_srcdir)/plug-ins/ed \
-I$(top_srcdir)/plug-ins/ed/lib \
-I$(top_srcdir)/plug-ins/updates \
-I$(top_srcdir)/plug-ins/traverse \
-I$(top_srcdir)/plug-ins/wearlocation \
-I$(top_srcdir)/plug-ins/skills_impl \
-I$(top_srcdir)/plug-ins/ai \
-I$(top_srcdir)/plug-ins/clan/core \
-I$(top_srcdir)/plug-ins/quest/core \
-I$(top_srcdir)/plug-ins/services/core \
-I$(top_srcdir)/plug-ins/services/petshop \
-I$(top_srcdir)/plug-ins/services/shop \
$(INCLUDES_AI) \
$(INCLUDES_SRC)

libcraft_la_LIBADD = \
$(LIBADD_AI) \
../alignment/libalignment.la \
../ed/lib/libed.la \
../ed/libcoreed.la \
../updates/libupdates.la \
../wearlocation/libwearlocation.la \
../anatolia/libcmd_wizard.la \
../skills_impl/libskills_impl.la \
../clan/core/libclan_core.la \
../quest/core/libquest_core.la \
../services/core/libservices_core.la \
../services/shop/libservices_shop.la \
../services/petshop/libservices_petshop.la 

AM_CPPFLAGS += $(plugin_INCLUDES)
