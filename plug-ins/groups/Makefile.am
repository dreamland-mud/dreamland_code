lib_LTLIBRARIES = \
libgenericskill.la \
libgroup_skills.la \
libclass_skills.la 

include $(top_srcdir)/src/Makefile.inc
include $(top_srcdir)/plug-ins/Makefile.inc

plugin_INCLUDES = \
-I$(top_srcdir)/plug-ins/skills_impl \
-I$(top_srcdir)/plug-ins/liquid \
-I$(top_srcdir)/plug-ins/selfrate \
-I$(top_srcdir)/plug-ins/services/core \
-I$(top_srcdir)/plug-ins/services/shop \
$(INCLUDES_AI) \
$(INCLUDES_SRC) 

libgenericskill_la_SOURCES = \
impl_genericskill.cpp \
chance.cpp \
dreamskill.cpp \
genericskill.cpp 

libgenericskill_la_MOC = \
genericskill.h 

libgenericskill_la_LIBADD = \
../descriptor/libdescriptor.la \
../output/liboutput.la \
../system/libsystem.la \
../system/libscheduler_task_round.la \
../loadsave/libloadsave.la \
../fight_core/libfight_core.la \
../fight/libfight.la \
../wearlocation/libwearlocation.la \
../skills_impl/libskills_impl.la 

libgroup_skills_la_SOURCES = \
impl_groups.cpp \
exoticskill.cpp \
group_beguiling.cpp \
group_detection.cpp \
group_fightmaster.cpp \
group_transportation.cpp \
group_weaponsmaster.cpp \
group_movement.cpp \
group_arcane.cpp

libgroup_skills_la_MOC = \
group_vampiric.h \
group_beguiling.h \
exoticskill.h

libgroup_skills_la_LIBADD = \
$(LIBADD_AI) \
../skills_impl/libskills_impl.la \
../selfrate/libselfrate.la \
../liquid/libliquid.la \
libgenericskill.la

libclass_skills_la_SOURCES = \
impl_classes.cpp \
class_antipaladin.cpp \
class_ninja.cpp \
class_ranger.cpp \
class_samurai.cpp \
class_thief.cpp \
class_vampire.cpp \
class_warlock.cpp \
class_warrior.cpp 

libclass_skills_la_MOC = \
class_samurai.h \
class_vampire.h 

libclass_skills_la_LIBADD = \
$(libgroup_skills_la_LIBADD) \
../services/core/libservices_core.la \
../selfrate/libselfrate.la \
../services/shop/libservices_shop.la 



AM_CPPFLAGS += $(plugin_INCLUDES)
