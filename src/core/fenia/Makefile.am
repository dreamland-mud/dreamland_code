noinst_LTLIBRARIES = libcorefenia.la

include $(top_srcdir)/src/Makefile.inc
src_INCLUDES = $(INCLUDES_SRC)

AM_CPPFLAGS = -Wall

libcorefenia_la_SOURCES = \
feniamanager.cpp \
wrapperbase.cpp \
wrappermanagerbase.cpp \
wrappertarget.cpp \
schedulerwrapper.cpp \
idcontainer.cpp \
regcontainer.cpp \
reglist.cpp \
guts.cpp

libcorefenia_la_MOC = \
regcontainer.h \
reglist.h \
schedulerwrapper.h \
wrapperbase.h \
guts.h
AM_CPPFLAGS += $(src_INCLUDES)
