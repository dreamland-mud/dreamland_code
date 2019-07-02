lib_LTLIBRARIES = libquestreward.la

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

plugin_INCLUDES = \
-I$(top_srcdir)/plug-ins/descriptor \
-I$(top_srcdir)/plug-ins/output \
-I$(top_srcdir)/plug-ins/loadsave \
-I$(top_srcdir)/plug-ins/system \
$(INCLUDES_SRC)

libquestreward_la_LIBADD = \
../descriptor/libdescriptor.la \
../output/liboutput.la \
../loadsave/libloadsave.la \
../profession/libprofession.la \
../system/libsystem.la

libquestreward_la_SOURCES = \
impl.cpp \
questreward.cpp \
personalquestreward.cpp \
questgirth.cpp \
questring.cpp \
questweapon.cpp \
questbag.cpp \
valentineprise.cpp \
ownercoupon.cpp

libquestreward_la_MOC = \
questreward.h \
personalquestreward.h \
questgirth.h \
questring.h \
questweapon.h \
questbag.h \
valentineprise.h \
ownercoupon.h


AM_CPPFLAGS += $(plugin_INCLUDES)
