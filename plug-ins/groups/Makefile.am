lib_LTLIBRARIES = \
libgenericskill.la \
libgroup_skills.la \
libclass_skills.la \
libgenericskill_loader.la 

include $(top_srcdir)/src/Makefile.inc
include $(top_srcdir)/plug-ins/Makefile.inc

INCLUDES = \
-I$(top_srcdir)/plug-ins/skills_impl \
-I$(top_srcdir)/plug-ins/liquid \
-I$(top_srcdir)/plug-ins/selfrate \
-I$(top_srcdir)/plug-ins/services/core \
-I$(top_srcdir)/plug-ins/services/shop \
$(INCLUDES_AI) \
$(INCLUDES_SRC) 

libgenericskill_la_SOURCES = \
impl_genericskill.cpp \
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
../skills_impl/libskills_impl.la 

libgroup_skills_la_SOURCES = \
impl_groups.cpp \
group_attack.cpp \
group_beguiling.cpp \
group_benedictions.cpp \
group_combat.cpp \
group_creation.cpp \
group_curative.cpp \
group_defensive.cpp \
group_detection.cpp \
group_draconian.cpp \
group_enchantment.cpp \
group_enhancement.cpp \
group_fightmaster.cpp \
group_harmful.cpp \
group_healing.cpp \
group_illusion.cpp \
group_maladictions.cpp \
group_meditation.cpp \
group_necromancy.cpp \
group_protective.cpp \
group_transportation.cpp \
group_weaponsmaster.cpp \
group_weather.cpp \
group_immortal.cpp \
group_other.cpp \
group_movement.cpp \
group_nature.cpp

libgroup_skills_la_MOC = \
group_vampiric.h \
group_beguiling.h \
group_necromancy.h \
group_other.h

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
class_paladin.cpp \
class_ranger.cpp \
class_samurai.cpp \
class_thief.cpp \
class_vampire.cpp \
class_warlock.cpp \
class_warrior.cpp \
class_necromancer.cpp \
class_universal.cpp \
class_cleric.cpp 

libclass_skills_la_MOC = \
class_antipaladin.h \
class_samurai.h \
class_vampire.h \
class_ranger.h \
class_universal.h \
class_warlock.h 

libclass_skills_la_LIBADD = \
$(libgroup_skills_la_LIBADD) \
../services/core/libservices_core.la \
../services/shop/libservices_shop.la 

libgenericskill_loader_la_SOURCES = \
genericskillloader.cpp

libgenericskill_loader_la_LIBADD = \
$(libgroup_skills_la_LIBADD) \
libgroup_skills.la \
libclass_skills.la \
../druids/libdruids.la


