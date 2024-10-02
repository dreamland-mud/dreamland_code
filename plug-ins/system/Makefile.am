lib_LTLIBRARIES = libscheduler_task_round.la libsystem.la

include $(top_srcdir)/src/Makefile.inc
plugin_INCLUDES = \
$(INCLUDES_SRC) \
-I$(top_srcdir)/plug-ins/descriptor \
-I/usr/include/jsoncpp

libscheduler_task_round_la_SOURCES = \
schedulertaskroundcharacter.cpp \
schedulertaskroundobject.cpp \
schedulertaskroundpcharacter.cpp \
schedulertaskroundplugin.cpp \
schedulertaskroundroom.cpp 

libscheduler_task_round_la_LIBADD = \
../descriptor/libdescriptor.la

libsystem_la_LIBADD = \
libscheduler_task_round.la \
../descriptor/libdescriptor.la \
-ljsoncpp

libsystem_la_SOURCES = \
xmlattributeplugin.cpp \
scheduledxmlattribute.cpp \
schedulertaskattributemanager.cpp \
system_impl.cpp \
xmlattributestatistic.cpp \
commonattributes.cpp \
playerattributes.cpp \
xmlpcpredicates.cpp \
xmlattributeticker.cpp \
xmlattributecoder.cpp \
xmltableloaderplugin.cpp \
xmlattributetrust.cpp \
xmlconfigurableplugin.cpp \
mobilebehaviorplugin.cpp \
objectbehaviorplugin.cpp \
areabehaviorplugin.cpp \
roombehaviorplugin.cpp \
descriptorstatelistener.cpp \
descriptorstatemanager.cpp \
fenia_utils.cpp \
math_utils.cpp \
calendar_utils.cpp \
morphology.cpp \
configurable.cpp \
json_utils_ext.cpp \
xmlkillingattribute.cpp \
alignment.cpp \
player_utils.cpp

libsystem_la_MOC = \
schedulertaskattributemanager.h \
xmlattributestatistic.h \
xmlattributeticker.h \
xmlattributecoder.h \
xmlattributetrust.h \
xmlkillingattribute.h


include $(top_srcdir)/plug-ins/Makefile.inc
AM_CPPFLAGS += $(plugin_INCLUDES)
