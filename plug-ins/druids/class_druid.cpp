/* $Id$
 *
 * ruffina, 2004
 */
#include "class_druid.h"

#include "skill.h"
#include "spelltarget.h"
#include "spelltemplate.h"
#include "affecthandlertemplate.h"
#include "skillcommandtemplate.h"
#include "skillmanager.h"

#include "affect.h"
#include "object.h"
#include "character.h"

#include "gsn_plugin.h"
#include "handler.h"
#include "magic.h"
#include "merc.h"
#include "mercdb.h"
#include "act.h"
#include "def.h"

#define OBJ_VNUM_DRUID_STAFF 37

PROF(druid);

SPELL_DECL(DruidStaff);
VOID_SPELL(DruidStaff)::run( Character *ch, char *, int sn, int level ) 
{ 
    Object *staff;
    Affect tohit;
    Affect todam;

    staff = create_object( get_obj_index(OBJ_VNUM_DRUID_STAFF),level);
    ch->println("Ты создаешь посох друида!");
    act("$c1 создает посох друида!",ch,0,0,TO_ROOM);

    staff->value1(4 + level / 15);
    staff->value2(4 + level / 15);

    tohit.type               = sn;
    tohit.level              = ch->getModifyLevel();
    tohit.duration           = -1;
    tohit.location = APPLY_HITROLL;
    tohit.modifier           = 2 + level/5;
    affect_to_obj( staff, &tohit);

    todam.type               = sn;
    todam.level              = ch->getModifyLevel();
    todam.duration           = -1;
    todam.location = APPLY_DAMROLL;
    todam.modifier           = 2 + level/5;
    affect_to_obj( staff, &todam);

    staff->timer = level;
    staff->level = ch->getModifyLevel();

    obj_to_char(staff,ch);
}

/*
 * druid staff behavior
 */
void DruidStaff::fight( Character *ch )
{
    if (obj->wear_loc != wear_wield && obj->wear_loc != wear_second_wield)
        return;

    if (chance( 90 ))
        return;

    act_p( "{BСиневатое свечение окутывает твой посох друида.{x", ch, 0, 0, TO_CHAR, POS_DEAD );
    act( "{BСиневатое свечение окутывает посох друида $c2.{x", ch, 0, 0, TO_ROOM );

    spell( gsn_cure_critical, ch->getModifyLevel( ), ch, ch, FSPELL_BANE );
}

bool DruidStaff::death( Character *ch )
{
    act_p( "Твой посох друида исчезает.", ch, 0, 0, TO_CHAR, POS_DEAD );
    act( "Посох друида $c2 исчезает.", ch, 0, 0, TO_ROOM );
    extract_obj( obj );
    return false;
}

bool DruidStaff::canEquip( Character *ch )
{
  if (ch->getProfession( ) != prof_druid) {
        ch->println("Ты не знаешь как использовать эту вещь.");
        act( "$o1 выскальзывает из твоих рук.", ch, obj, 0, TO_CHAR );
        act( "$o1 выскальзывает из рук $c2.", ch, obj, 0, TO_ROOM );
        obj_from_char( obj );
        obj_to_room( obj, ch->in_room );
        return false;
    }

    return true;
}

/*
 * DruidSummonedAnimal behavior
 */
DruidSummonedAnimal::~DruidSummonedAnimal( )
{
}

bool DruidSummonedAnimal::myHero( Character *ch ) const
{
    return !ch->is_npc( ) && ch->getName( ) == heroName.getValue( );
}

