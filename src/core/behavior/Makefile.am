noinst_LTLIBRARIES = libbehavior.la

include $(top_srcdir)/src/Makefile.inc
src_INCLUDES = $(INCLUDES_SRC)

AM_CPPFLAGS = -Wall

libbehavior_la_SOURCES = \
mobilebehavior.cpp \
objectbehavior.cpp \
areabehavior.cpp \
roombehavior.cpp 


libbehavior_la_MOC = \
areabehavior.h \
mobilebehavior.h \
objectbehavior.h \
roombehavior.h

AM_CPPFLAGS += $(src_INCLUDES)
