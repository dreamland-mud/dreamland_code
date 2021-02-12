lib_LTLIBRARIES = libskills_loader.la

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

plugin_INCLUDES = \
-I$(top_srcdir)/plug-ins/skills_impl \
-I$(top_srcdir)/plug-ins/groups \
-I$(top_srcdir)/plug-ins/clan/impl \
-I$(top_srcdir)/plug-ins/race_aptitude \
-I$(top_srcdir)/plug-ins/craft \
-I$(top_srcdir)/plug-ins/cards \
-I$(top_srcdir)/plug-ins/clan/skill \
-I$(top_srcdir)/plug-ins/other_skills \
$(INCLUDES_FIGHT) \
$(INCLUDES_SRC)

libskills_loader_la_LIBADD = \
../system/libsystem.la \
../skills_impl/libskills_impl.la \
../feniaroot/libfeniaroot.la \
../groups/libgroup_skills.la \
../groups/libclass_skills.la \
../groups/libgenericskill.la \
../clan/impl/libclan_impl.la \
../race_aptitude/librace_aptitude.la \
../craft/libcraft.la \
../cards/libcards.la \
../other_skills/libother_skills.la

libskills_loader_la_SOURCES = impl.cpp

AM_CPPFLAGS += $(plugin_INCLUDES)
