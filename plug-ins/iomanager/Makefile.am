lib_LTLIBRARIES = libiomanager.la

include $(top_srcdir)/src/Makefile.inc
INCLUDES = \
$(INCLUDES_SRC) \
-I/usr/include/jsoncpp \
-I$(top_srcdir)/plug-ins/interpret \
-I$(top_srcdir)/plug-ins/loadsave \
-I$(top_srcdir)/plug-ins/system \
-I$(top_srcdir)/plug-ins/descriptor \
-I$(top_srcdir)/plug-ins/output \
-I$(top_srcdir)/plug-ins/alignment \
-I$(top_srcdir)/plug-ins/ban

libiomanager_la_SOURCES = \
iomanager.cpp \
comm.cpp \
serversocket.cpp \
serversocketcontainer.cpp \
codepage.cpp \
webprompt.cpp \
descriptorstatelistener.cpp \
descriptorstatemanager.cpp \
pagerhandler.cpp \
interprethandler.cpp \
nannyhandler.cpp \
wrapperhandler.cpp \
defaultbufferhandler.cpp \
descriptor.cpp \
backdoorhandler.cpp \
badnames.cpp \
base64.cpp \
rpccommandmanager.cpp \
webmanip.cpp \
gmcp.cpp \
impl.cpp

libiomanager_la_MOC = \
pagerhandler.h \
interprethandler.h \
nannyhandler.h \
wrapperhandler.h \
defaultbufferhandler.h \
serversocket.h \
serversocketcontainer.h \
backdoorhandler.h \
badnames.h

if WITH_MINGW
WSOCK = -lwsock32
endif

libiomanager_la_LIBADD = \
../system/libscheduler_task_round.la \
../system/libsystem.la \
../loadsave/libloadsave.la \
../descriptor/libdescriptor.la \
../output/liboutput.la \
../interpret/libinterpret.la \
../alignment/libalignment.la \
../ban/libban.la \
-lcrypto \
$(WSOCK) \
-ljsoncpp


include $(top_srcdir)/plug-ins/Makefile.inc
