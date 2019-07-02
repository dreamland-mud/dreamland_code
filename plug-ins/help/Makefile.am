lib_LTLIBRARIES = \
libhelp.la

include $(top_srcdir)/src/Makefile.inc
plugin_INCLUDES = \
$(INCLUDES_SRC) \
-I$(top_srcdir)/plug-ins/system 


libhelp_la_LIBADD = \
../system/libsystem.la 

libhelp_la_SOURCES = \
helpformatter.cpp \
markuphelparticle.cpp \
helpcontainer.cpp \
bugtracker.cpp \
impl.cpp 

libhelp_la_MOC = \
bugtracker.h \
helpcontainer.h


include $(top_srcdir)/plug-ins/Makefile.inc
AM_CPPFLAGS += $(plugin_INCLUDES)
