#
# ruffina, DreamLand, 2003
# based on: NoFate, 2002
noinst_LTLIBRARIES = libplugin.la

INCLUDES = \
-I$(top_builddir) \
-I$(srcdir) \
-I$(srcdir)/../xml \
-I$(srcdir)/../lang \
-I$(srcdir)/../alloc \
-I$(srcdir)/../io \
-I$(srcdir)/../util 

AM_CPPFLAGS =  -Wall

libplugin_la_SOURCES = \
sharedobject.cpp \
initializer.cpp \
pluginmanager.cpp

libplugin_la_MOC = \
sharedobject.h
