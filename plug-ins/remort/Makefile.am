lib_LTLIBRARIES = libremort.la 

libremort_la_SOURCES = \
cmlt.cpp \
impl.cpp \
cremort.cpp \
cunremort.cpp \
fixremort.cpp \
remortnanny.cpp \
remortbonus.cpp \
remortbonuses_impl.cpp \
lifeprice.cpp \
victorybonus.cpp

libremort_la_MOC = \
remortnanny.h \
remortbonus.h \
remortbonuses_impl.h \
lifeprice.h \
victorybonus.h


include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

plugin_INCLUDES = \
-I$(top_srcdir)/plug-ins/services/core \
$(INCLUDES_ANATOLIA) \
$(INCLUDES_SRC) 

libremort_la_LIBADD = \
../services/core/libservices_core.la \
$(LIBADD_ANATOLIA)

AM_CPPFLAGS += $(plugin_INCLUDES)
