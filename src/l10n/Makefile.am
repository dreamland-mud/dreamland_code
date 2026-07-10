#
# ruffina, DreamLand, 2007
#
noinst_LTLIBRARIES = libl10n.la

src_INCLUDES = \
-I$(top_builddir) \
-I$(srcdir) \
-I$(srcdir)/../xml \
-I$(srcdir)/../lang   \
-I$(srcdir)/../util \
-I$(srcdir)/../core

AM_CPPFLAGS = -Wall

libl10n_la_SOURCES = \
grammar_entities.cpp \
noun.cpp \
nounholder.cpp \
verb.cpp \
pronouns.cpp \
ru_pronouns.cpp \
flexer.cpp \
inflectedstring.cpp \
xmlinflectedstring.cpp \
lang.cpp \
translation.cpp \
l10n.cpp

AM_CPPFLAGS += $(src_INCLUDES)
