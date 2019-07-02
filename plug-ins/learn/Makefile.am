lib_LTLIBRARIES = liblearn.la 

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

plugin_INCLUDES = \
$(INCLUDES_AI) \
$(INCLUDES_SRC)

liblearn_la_LIBADD = \
$(LIBADD_AI)

liblearn_la_SOURCES = \
allskillslist.cpp \
cskills.cpp \
train.cpp \
glist.cpp \
forget.cpp \
practice.cpp \
showskill.cpp \
teach.cpp \
impl.cpp

liblearn_la_MOC = \
forget.h \
teach.h \
train.h


AM_CPPFLAGS += $(plugin_INCLUDES)
