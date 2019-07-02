lib_LTLIBRARIES = \
libliquid.la

include $(top_srcdir)/src/Makefile.inc
plugin_INCLUDES = \
-I$(top_srcdir)/plug-ins/skills_impl \
$(INCLUDES_FIGHT) \
$(INCLUDES_SRC) 

libliquid_la_SOURCES = \
liquidflags.cpp \
impl.cpp \
defaultliquid.cpp \
liquidmanager.cpp \
drinkcontainer.cpp \
drink_commands.cpp \
drink_utils.cpp 

libliquid_la_MOC = \
defaultliquid.h \
drinkcontainer.h

libliquid_la_LIBADD = \
../skills_impl/libskills_impl.la \
$(LIBADD_FIGHT) 

liquidflags.cpp: liquidflags.conf liquidflags.h
liquidflags.h: liquidflags.conf
libliquid_la_moc_xml.cpp: liquidflags.h

CLEANFILES = liquidflags.cpp $(top_builddir)/src/liquidflags.h 

include $(top_srcdir)/plug-ins/Makefile.inc
AM_CPPFLAGS += $(plugin_INCLUDES)
