#
# ruffina, DreamLand, 2003
# based on: NoFate, 2002
noinst_LTLIBRARIES = libio.la

src_INCLUDES = \
-I$(srcdir) \
-I$(top_builddir) \
-I$(srcdir)/../xml \
-I$(srcdir)/../lang \
-I$(srcdir)/../alloc \
-I$(srcdir)/../util 

AM_CPPFLAGS = -Wall

libio_la_SOURCES = \
txncontext.cpp \
dbio.cpp \
dldirectory.cpp \
dlfile.cpp \
dlfileop.cpp \
dlfilestream.cpp \
exceptiondbio.cpp \
socketmanager.cpp \
xmlloader.cpp \
xmltableelement.cpp \
xmltableloader.cpp \
xmlfile.cpp \
xmlconfigurable.cpp 

AM_CPPFLAGS += $(src_INCLUDES)
