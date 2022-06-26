/* $Id$
 *
 * ruffina, 2004
 */
#include "objthrow.h"
#include "skillreference.h"
#include "skillcommandtemplate.h"
#include "skillmanager.h"
#include "logstream.h"

#include "affect.h"
#include "pcharacter.h"
#include "room.h"
#include "npcharacter.h"
#include "object.h"
#include "race.h"

#include "dreamland.h"
#include "act_move.h"
#include "magic.h"
#include "fight.h"
#include "damage.h"
#include "stats_apply.h"
#include "handler.h"
#include "act.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

GSN(search_stones);
GSN(throw_stone);
GSN(flaming_stone);
GSN(freezing_stone);

#define OBJ_VNUM_THROWING_STONE 118 

static Object * create_stone( int level )
{
    Object *stone;
    Affect af;

    stone = create_object( get_obj_index( OBJ_VNUM_THROWING_STONE ), 0 );
    stone->level = level;
    stone->value1(4 + level / 15);
    stone->value2(4 + level / 30);

    af.type               = gsn_search_stones;
    af.level              = level;
    af.duration           = -1;
    af.modifier           = level / 10;

    af.location = APPLY_HITROLL;
    affect_to_obj( stone, &af );

    af.location = APPLY_DAMROLL;
    affect_to_obj( stone, &af );

    return stone;
}

/*
 * 'search stones' skill command
 */

SKILL_RUNP( searchstones )
{
    Object *stone;
    int chance, count, mlevel;
    
    if (!gsn_search_stones->usable( ch )) {
        ch->pecho("Ты не умеешь это делать.");
        return;
    }
    
    switch (ch->in_room->getSectorType()) {
    default:                chance = 0; break;
    case SECT_MOUNTAIN: chance = 100; break;
    case SECT_HILLS:    chance = 80; break;
    case SECT_CITY:        chance = IS_SET(ch->in_room->room_flags, ROOM_INDOORS) ? 0 : 40; 
                        break;                        
    case SECT_FOREST:        chance = 60; break;
    case SECT_FIELD:        chance = 60; break;
    }

    if (chance == 0) {
        ch->pecho("В этой местности бесполезно искать камни.");
        return;
    }
    
    if (number_percent( ) > gsn_search_stones->getEffective( ch ) * chance / 100) {
        ch->pecho("Тебе не удалось отыскать ни одного камня.");

        if (number_percent( ) < chance)
            gsn_search_stones->improve( ch, false );

        return;
    }
    
    mlevel = ch->getModifyLevel( );
    count = number_range( 5, 5 + mlevel / 30 );

    oldact("Ты подбираешь с земли $t.", ch, (count == 1 ? "камень" : "несколько камней"), 0, TO_CHAR );
    oldact("$c1 подбирает с земли $t.", ch, (count == 1 ? "камень" : "несколько камней"), 0, TO_ROOM );
    
    for (int i = 0; i < count; i++) {
        stone = create_stone( mlevel );

        if (ch->getCarryWeight( ) + stone->getWeight( ) > ch->canCarryWeight( )) {
            ch->pecho( "Ты не в силах удержать %1$O4 и роняешь %1$P2.", stone );
            obj_to_room( stone, ch->in_room );
            break;
        }

        obj_to_char( stone, ch );
    }
    
    gsn_search_stones->improve( ch, true );
}

/*
 * 'throw stone' skill command
 */
static Object * find_stone(Character *ch)
{
    for (Object *obj = ch->carrying; obj; obj = obj->next_content )
        if (obj->item_type == ITEM_WEAPON
            && obj->value0() == WEAPON_STONE
            && (ch->can_see( obj ) || ch->can_hear( obj )))
        {
            return obj;
        }

    return 0;
}

SKILL_RUNP( throwstone )
{
    Character *victim;
    Object *stone;
    bool success;
    int direction, dam;
    int chance = gsn_throw_stone->getEffective( ch );
    int range = ( ch->getModifyLevel() / 10) + 1;
    DLString args = argument;
    DLString argDoor, argVict;
    ostringstream errbuf;

    if (!direction_range_argument(argument, argDoor, argVict, direction)) {
        ch->pecho("Швырнуть камень куда и в кого?");
        return;
    }

    if ( ( victim = find_char( ch, argVict.c_str(), direction, &range, errbuf ) ) == 0 ) {
        ch->send_to(errbuf);
        return;
    }

    if (victim == ch) {
        ch->pecho("Просто ударь себя этим камнем по лбу.");
        return;
    }

    if (is_safe_nomessage( ch, victim )) {
        oldact("Боги покровительствуют $C3.", ch, 0, victim, TO_CHAR);
        return;
    }

    if (ch->in_room == victim->in_room) {
        ch->pecho("Ты не можешь швыряться камнями в упор.");
        return;
    }

    stone = find_stone(ch);
    if (!stone) {
        ch->pecho("У тебя в инвентаре нет ни одного камня.");
        return;
    }

    set_violent( ch, victim, false );
    oldact("Ты швыряешь $o4 $T.", ch, stone, dirs[ direction ].leave, TO_CHAR );
    oldact("$c1 швыряет $o4 $T.", ch, stone, dirs[ direction ].leave, TO_ROOM );

    if (victim->position == POS_SLEEPING)
        chance += 40;
    if (victim->position == POS_RESTING)
        chance += 10;
    if (victim->position == POS_FIGHTING)
        chance -= 40;

    chance += ch->hitroll - ch->getRealLevel( );
    dam = dice(stone->value1(), stone->value2());
    dam += ch->damroll + get_str_app(ch).missile;

    if (ch->size < SIZE_HUGE)
        dam /= 2;

    if (number_percent( ) <= gsn_flaming_stone->getEffective( ch )) {
        Affect saf;
        saf.bitvector.setTable(&weapon_type2);
        saf.type               = gsn_flaming_stone;
        saf.level              = ch->getModifyLevel();
        saf.duration           = -1;
        saf.bitvector.setValue(WEAPON_FLAMING);
        affect_to_obj( stone, &saf);

    } else if (number_percent( ) <= gsn_freezing_stone->getEffective( ch )) {
        Affect saf;
        saf.bitvector.setTable(&weapon_type2);
        saf.type               = gsn_freezing_stone;
        saf.level              = ch->getModifyLevel();
        saf.duration           = -1;
        saf.bitvector.setValue(WEAPON_FROST);
        affect_to_obj( stone, &saf);
    }

    try {
        obj_from_char( stone );
        success = send_arrow(ch, victim, stone, direction, chance, dam);
    } catch (const VictimDeathException &) {
        victim = NULL;
        success = true;
    }
    
    gsn_throw_stone->improve( ch, success, victim );
}


