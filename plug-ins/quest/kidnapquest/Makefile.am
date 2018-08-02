lib_LTLIBRARIES = libquest_kidnap.la

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

INCLUDES = \
-I$(srcdir)/../core \
-I$(top_srcdir)/plug-ins/selfrate \
$(INCLUDES_AI) \
$(INCLUDES_SRC)

libquest_kidnap_la_LIBADD = \
../../selfrate/libselfrate.la \
../core/libquest_core.la \
$(LIBADD_AI)

libquest_kidnap_la_SOURCES = \
bandit.cpp \
bandit_move.cpp \
king.cpp \
kidnapmobile.cpp \
prince.cpp \
kidnapquest.cpp \
kidnapquestregistrator.cpp \
objects.cpp \
scenario.cpp \
scenario_bidon.cpp \
scenario_dragon.cpp \
scenario_urchin.cpp \
scenario_urka.cpp \
scenario_cyclop.cpp \
impl_kidnapquest.cpp 


libquest_kidnap_la_MOC = \
king.h \
kidnapmobile.h \
prince.h \
kidnapquest.h \
kidnapquestregistrator.h \
objects.h \
scenario.h \
scenario_bidon.h \
scenario_dragon.h \
scenario_urchin.h \
scenario_urka.h \
scenario_cyclop.h \
bandit.h 


