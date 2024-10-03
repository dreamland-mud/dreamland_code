lib_LTLIBRARIES = libquest_command.la 

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

plugin_INCLUDES = \
-I$(srcdir)/../core \
-I$(top_srcdir)/plug-ins/services/core \
-I$(top_srcdir)/plug-ins/religion \
-I$(top_srcdir)/plug-ins/clan/core \
-I$(top_srcdir)/plug-ins/languages/core \
$(INCLUDES_AI) \
$(INCLUDES_SRC)

libquest_command_la_LIBADD = \
../../clan/core/libclan_core.la \
../../services/core/libservices_core.la \
../../languages/core/liblanguages_core.la \
../../religion/libreligion.la \
../core/libquest_core.la \
$(LIBADD_AI)

libquest_command_la_SOURCES = \
impl.cpp \
questtrader.cpp \
questmaster.cpp \
questor.cpp \
cquest.cpp \
xmlattributequestreward.cpp 

libquest_command_la_MOC = \
cquest.h \
questtrader.h \
xmlattributequestreward.h \
questmaster.h \
questor.h

AM_CPPFLAGS += $(plugin_INCLUDES)
