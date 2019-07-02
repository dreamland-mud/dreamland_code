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
elvish_effects.cpp \
quenia.cpp \
quenia_effects.cpp \
arcadian.cpp \
arcadian_effects.cpp \
arcadian_behaviors.cpp \
ahenn.cpp \
ahenn_effects.cpp \
khuzdul.cpp \
khuzdul_effects.cpp

liblanguages_impl_la_MOC = \
elvish_effects.h \
quenia.h \
quenia_effects.h \
arcadian.h \
arcadian_effects.h \
arcadian_behaviors.h \
ahenn.h \
ahenn_effects.h \
khuzdul.h \
khuzdul_effects.h


AM_CPPFLAGS += $(plugin_INCLUDES)
