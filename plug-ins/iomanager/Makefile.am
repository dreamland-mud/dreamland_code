lib_LTLIBRARIES = libiomanager.la

include $(top_srcdir)/src/Makefile.inc
plugin_INCLUDES = \
$(INCLUDES_SRC) \
-I/usr/include/jsoncpp \
-I$(top_srcdir)/plug-ins/interpret \
-I$(top_srcdir)/plug-ins/loadsave \
-I$(top_srcdir)/plug-ins/system \
-I$(top_srcdir)/plug-ins/descriptor \
-I$(top_srcdir)/plug-ins/output

libiomanager_la_SOURCES = \
iomanager.cpp \
comm.cpp \
serversocket.cpp \
serversocketcontainer.cpp \
codepage.cpp \
webprompt.cpp \
pagerhandler.cpp \
interprethandler.cpp \
commonlayers.cpp \
nannyhandler.cpp \
wrapperhandler.cpp \
defaultbufferhandler.cpp \
descriptor.cpp \
backdoorhandler.cpp \
badnames.cpp \
base64.cpp \
rpccommandmanager.cpp \
webmanip.cpp \
lasthost.cpp \
banflags.cpp \
ban.cpp \
impl.cpp

libiomanager_la_MOC = \
pagerhandler.h \
interprethandler.h \
nannyhandler.h \
wrapperhandler.h \
webprompt.h \
defaultbufferhandler.h \
serversocket.h \
serversocketcontainer.h \
backdoorhandler.h \
lasthost.h \
badnames.h \
ban.h

libiomanager_la_LIBADD = \
../system/libscheduler_task_round.la \
../system/libsystem.la \
../loadsave/libloadsave.la \
../descriptor/libdescriptor.la \
../output/liboutput.la \
../interpret/libinterpret.la \
-lcrypto \
-ljsoncpp

banflags.h: banflags.conf
banflags.cpp: banflags.conf banflags.h
libiomanager_la_moc_xml.cpp: banflags.h

CLEANFILES = banflags.cpp $(top_builddir)/src/banflags.h 

include $(top_srcdir)/plug-ins/Makefile.inc
AM_CPPFLAGS += $(plugin_INCLUDES)
