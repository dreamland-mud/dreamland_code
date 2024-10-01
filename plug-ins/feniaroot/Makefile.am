lib_LTLIBRARIES = libfeniaroot.la 

include $(top_srcdir)/plug-ins/Makefile.inc
include $(top_srcdir)/src/Makefile.inc

libfeniaroot_la_SOURCES = \
ceval.cpp \
cfindref.cpp \
characterwrapper.cpp \
commandwrapper.cpp \
feniasync.cpp \
impl.cpp \
mobindexwrapper.cpp \
objectwrapper.cpp \
objindexwrapper.cpp \
roomwrapper.cpp \
root.cpp \
schedulerwrapperext.cpp \
validatetask.cpp \
wrappermanager.cpp \
wrappersplugin.cpp \
ccodesource.cpp \
xmlattributecodesource.cpp \
dlprintf.cpp \
idcontainerext.cpp \
regcontainerext.cpp \
feniastringext.cpp \
reglistext.cpp \
tableswrapper.cpp \
affectwrapper.cpp \
structwrappers.cpp \
areaindexwrapper.cpp \
spellwrapper.cpp \
affecthandlerwrapper.cpp \
feniaspellext.cpp \
skillcommandwrapper.cpp \
areaquestwrapper.cpp \
wrap_utils.cpp \
feniacroaker.cpp \
commandtableloader.cpp \
codesourcerepo.cpp \
behaviorwrapper.cpp \
behaviorloader.cpp \
playerwrapper.cpp

libfeniaroot_la_MOC = \
root.h \
affectwrapper.h \
commandwrapper.h \
structwrappers.h \
objectwrapper.h \
objindexwrapper.h \
roomwrapper.h \
wrappersplugin.h \
xmlattributecodesource.h \
tableswrapper.h \
mobindexwrapper.h \
areaindexwrapper.h \
spellwrapper.h \
affecthandlerwrapper.h \
skillcommandwrapper.h \
areaquestwrapper.h \
characterwrapper.h \
behaviorwrapper.h \
behaviorloader.h \
playerwrapper.h

plugin_INCLUDES = \
-I$(top_srcdir)/plug-ins/ed \
-I$(top_srcdir)/plug-ins/ed/lib \
-I$(top_srcdir)/plug-ins/updates \
-I$(top_srcdir)/plug-ins/traverse \
-I$(top_srcdir)/plug-ins/questreward \
-I$(top_srcdir)/plug-ins/ai \
-I$(top_srcdir)/plug-ins/craft \
-I$(top_srcdir)/plug-ins/religion \
-I$(top_srcdir)/plug-ins/liquid \
-I$(top_srcdir)/plug-ins/skills_impl  \
-I$(top_srcdir)/plug-ins/clan/core \
-I$(top_srcdir)/plug-ins/clan/behavior \
-I$(top_srcdir)/plug-ins/services/core \
-I$(top_srcdir)/plug-ins/languages/core \
-I$(top_srcdir)/plug-ins/services/petshop \
-I/usr/include/jsoncpp \
$(INCLUDES_AI) \
$(INCLUDES_SRC)

libfeniaroot_la_LIBADD = \
$(LIBADD_AI) \
../liquid/libliquid.la \
../ed/lib/libed.la \
../ed/libcoreed.la \
../updates/libupdates.la \
../craft/libcraft.la \
../religion/libreligion.la \
../questreward/libquestreward.la \
../clan/core/libclan_core.la \
../clan/behavior/libclan_behavior.la \
../skills_impl/libskills_impl.la \
../services/core/libservices_core.la \
../services/petshop/libservices_petshop.la \
../languages/core/liblanguages_core.la \
-ljsoncpp

AM_CPPFLAGS += $(plugin_INCLUDES)
