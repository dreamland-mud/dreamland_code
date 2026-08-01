lib_LTLIBRARIES = libupdates.la

libupdates_la_SOURCES = \
update.cpp \
update_params.cpp \
update_areas.cpp \
weather.cpp \
impl.cpp 

include $(top_srcdir)/src/Makefile.inc
include $(top_srcdir)/plug-ins/Makefile.inc
plugin_INCLUDES = $(INCLUDES_FIGHT) $(INCLUDES_SRC) \
-I$(top_srcdir)/plug-ins/iomanager


libupdates_la_LIBADD = \
$(LIBADD_FIGHT) \
../iomanager/libiomanager.la \
-ljsoncpp

AM_CPPFLAGS += $(plugin_INCLUDES)
