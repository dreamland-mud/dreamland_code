lib_LTLIBRARIES = libmisc_behaviors.la 

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

plugin_INCLUDES = \
-I$(top_srcdir)/plug-ins/quest/core \
-I$(top_srcdir)/plug-ins/quest/command \
-I$(top_srcdir)/plug-ins/services/core \
-I$(top_srcdir)/plug-ins/services/petshop \
$(INCLUDES_AI) \
$(INCLUDES_SRC) 

libmisc_behaviors_la_LIBADD = \
../services/core/libservices_core.la \
../services/petshop/libservices_petshop.la \
../quest/core/libquest_core.la \
../quest/command/libquest_command.la \
$(LIBADD_AI)

libmisc_behaviors_la_SOURCES = \
anatolia_limits.cpp \
midgaardfountain.cpp \
hierophant.cpp \
pocketwatch.cpp \
masquerade.cpp \
ofcolguards.cpp \
rats.cpp \
special.cpp \
moneychanger.cpp \
petquestor.cpp \
equipsets.cpp \
impl.cpp 

libmisc_behaviors_la_MOC = \
anatolia_limits.h \
midgaardfountain.h \
pocketwatch.h \
masquerade.h \
ofcolguards.h \
rats.h \
moneychanger.h \
petquestor.h \
equipsets.h \
hierophant.h 


AM_CPPFLAGS += $(plugin_INCLUDES)
