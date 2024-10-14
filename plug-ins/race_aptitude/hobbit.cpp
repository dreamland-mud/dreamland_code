/* $Id$
 *
 * ruffina, 2004
 */
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
#include "movetypes.h"
#include "directions.h"
#include "terrains.h"
#include "move_utils.h"
#include "doors.h"
#include "magic.h"
#include "fight.h"
#include "damage.h"
#include "stats_apply.h"
#include "loadsave.h"
#include "act.h"
#include "merc.h"

#include "def.h"

GSN(search_stones);

#define OBJ_VNUM_THROWING_STONE 118 

static Object * create_stone( int level )
{
    Object *stone;
    Affect af;

    stone = create_object( get_obj_index( OBJ_VNUM_THROWING_STONE ), 0 );
    stone->level = level;
    stone->timer = Date::SECOND_IN_MONTH;
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

        if (Char::getCarryWeight(ch) + stone->getWeight( ) > Char::canCarryWeight(ch)) {
            ch->pecho( "Ты не в силах удержать %1$O4 и роняешь %1$P2.", stone );
            obj_to_room( stone, ch->in_room );
            break;
        }

        obj_to_char( stone, ch );
    }
    
    gsn_search_stones->improve( ch, true );
}

