lib_LTLIBRARIES = libclan_behavior.la 

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

plugin_INCLUDES = \
-I$(top_srcdir)/plug-ins/skills_impl \
-I$(top_srcdir)/plug-ins/services/core \
-I$(top_srcdir)/plug-ins/services/petshop \
-I$(top_srcdir)/plug-ins/clan/core \
$(INCLUDES_AI) \
$(INCLUDES_SRC)

libclan_behavior_la_SOURCES = \
clanarea.cpp \
clanobjects.cpp \
clanmobiles.cpp \
clanrooms.cpp \
impl.cpp

libclan_behavior_la_MOC = \
clanarea.h \
clanobjects.h \
clanmobiles.h \
clanrooms.h 

libclan_behavior_la_LIBADD = \
$(LIBADD_AI) \
../../skills_impl/libskills_impl.la \
../../services/core/libservices_core.la \
../../services/petshop/libservices_petshop.la \
../core/libclan_core.la

AM_CPPFLAGS += $(plugin_INCLUDES)
