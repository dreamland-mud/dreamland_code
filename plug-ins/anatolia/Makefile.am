lib_LTLIBRARIES = libanatolia.la libcmd_wizard.la
SUBDIRS = core

libanatolia_la_SOURCES = \
act_comm.cpp \
act_hera.cpp \
act_info.cpp \
act_lock.cpp \
act_look.cpp \
act_move.cpp \
act_obj.cpp \
handler.cpp \
impl.cpp 

libcmd_wizard_la_SOURCES = \
act_wiz.cpp \
set.cpp 

include $(top_srcdir)/src/Makefile.inc
include $(top_srcdir)/plug-ins/Makefile.inc
plugin_INCLUDES = \
$(INCLUDES_ANATOLIA) \
$(INCLUDES_SRC) 

libanatolia_la_LIBADD = \
$(LIBADD_ANATOLIA_CORE) \
$(top_builddir)/plug-ins/help/libhelp.la \
$(top_builddir)/plug-ins/alignment/libalignment.la \
$(top_builddir)/plug-ins/occupations/liboccupations.la 

libcmd_wizard_la_LIBADD = $(libanatolia_la_LIBADD) libanatolia.la

AM_CPPFLAGS += $(plugin_INCLUDES)
