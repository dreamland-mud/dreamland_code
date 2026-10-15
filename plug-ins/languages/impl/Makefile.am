lib_LTLIBRARIES = liblanguages_impl.la 

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

plugin_INCLUDES = \
-I$(srcdir)/../core \
-I$(top_srcdir)/plug-ins/liquid \
$(INCLUDES_AI) \
$(INCLUDES_SRC)


liblanguages_impl_la_LIBADD = \
../../liquid/libliquid.la \
../core/liblanguages_core.la \
$(LIBADD_AI)

liblanguages_impl_la_SOURCES = \
impl.cpp \
quenia.cpp \
arcadian.cpp \
arcadian_effects.cpp \
arcadian_behaviors.cpp \
ahenn.cpp \
khuzdul.cpp

liblanguages_impl_la_MOC = \
quenia.h \
arcadian.h \
arcadian_effects.h \
arcadian_behaviors.h \
ahenn.h \
khuzdul.h


AM_CPPFLAGS += $(plugin_INCLUDES)
