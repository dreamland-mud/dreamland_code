lib_LTLIBRARIES = libclan_impl.la 

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

plugin_INCLUDES = \
-I$(top_srcdir)/plug-ins/skills_impl \
-I$(srcdir)/../core \
-I$(srcdir)/../command \
-I$(srcdir)/../behavior \
-I$(top_srcdir)/plug-ins/services/core \
-I$(top_srcdir)/plug-ins/services/petshop \
-I$(top_srcdir)/plug-ins/questreward \
$(INCLUDES_AI) \
$(INCLUDES_SRC) 

libclan_impl_la_SOURCES = \
defaultclan.cpp \
battlerager.cpp \
chaos.cpp \
ghost.cpp \
hunter.cpp \
invader.cpp \
knight.cpp \
lion.cpp \
ruler.cpp \
shalafi.cpp \
flowers.cpp \
common.cpp \
impl.cpp

libclan_impl_la_MOC = \
defaultclan.h \
battlerager.h \
chaos.h \
ghost.h \
hunter.h \
invader.h \
knight.h \
lion.h \
ruler.h \
shalafi.h \
flowers.h 

libclan_impl_la_LIBADD = \
$(LIBADD_AI) \
../../skills_impl/libskills_impl.la \
../../services/core/libservices_core.la \
../../services/petshop/libservices_petshop.la \
../../questreward/libquestreward.la \
../core/libclan_core.la \
../command/libclan_command.la \
../behavior/libclan_behavior.la

AM_CPPFLAGS += $(plugin_INCLUDES)
