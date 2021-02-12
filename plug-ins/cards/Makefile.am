lib_LTLIBRARIES = libcards.la 

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

plugin_INCLUDES = \
-I$(top_srcdir)/plug-ins/skills_impl \
$(INCLUDES_AI) \
$(INCLUDES_SRC)

libcards_la_LIBADD = \
$(LIBADD_AI) \
../skills_impl/libskills_impl.la 


libcards_la_SOURCES = \
ccard.cpp \
cardskill.cpp \
mobiles.cpp \
objects.cpp \
skills.cpp \
xmlattributecards.cpp \
impl.cpp 

libcards_la_MOC = \
ccard.h \
cardskill.h \
mobiles.h \
objects.h \
xmlattributecards.h


AM_CPPFLAGS += $(plugin_INCLUDES)
