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
    stone->value[1] = 4 + level / 15;
    stone->value[2] = 4 + level / 30;

    af.where	          = TO_OBJECT;
    af.type               = gsn_search_stones;
    af.level              = level;
    af.duration           = -1;
    af.modifier           = level / 10;

    af.location           = APPLY_HITROLL;
    affect_to_obj( stone, &af );

    af.location           = APPLY_DAMROLL;
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
	ch->println("Ты не умеешь это делать.");
	return;
    }
    
    switch (ch->in_room->sector_type) {
    default:		chance = 0; break;
    case SECT_MOUNTAIN: chance = 100; break;
    case SECT_HILLS:    chance = 80; break;
    case SECT_CITY:	chance = IS_SET(ch->in_room->room_flags, ROOM_INDOORS) ? 0 : 40; 
			break;			
    case SECT_FOREST:	chance = 30; break;
    case SECT_FIELD:	chance = 30; break;
    }
    
    if (number_percent( ) > gsn_search_stones->getEffective( ch ) * chance / 100) {
	ch->println("Тебе не удалось отыскать ни одного камня.");

	if (number_percent( ) < chance)
	    gsn_search_stones->improve( ch, false );

	return;
    }

    if (ch->mana < gsn_search_stones->getMana( )) {
	ch->println( "У тебя не хватает энергии для поиска." );
	return;
    }

    ch->mana -= gsn_search_stones->getMana( );
    ch->setWait( gsn_search_stones->getBeats( ) );
    
    mlevel = ch->getModifyLevel( );
    count = number_range( 1, mlevel / 20 );

    act( "Ты подбираешь с земли $t.", ch, (count == 1 ? "камень" : "несколько камней"), 0, TO_CHAR );
    act( "$c1 подбирает с земли $t.", ch, (count == 1 ? "камень" : "несколько камней"), 0, TO_ROOM );
    
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

SKILL_RUNP( throwstone )
{
    Character *victim;
    Object *stone;
    bool success;
    int direction, dam, scanRange, scanRange0, throwRange;
    int chance = gsn_throw_stone->getEffective( ch );
    DLString args = argument, arg1, arg2;

    if (chance <= 1) {
	ch->println("Ты не умеешь швыряться камнями.");
	return;
    }
    
    arg1 = args.getOneArgument( );
    arg2 = args.getOneArgument( );

    if (arg1.empty( ) || arg2.empty( )) {
	ch->println("Швырнуть камень куда и в кого?");
	return;
    }

    direction = direction_lookup( arg1.c_str( ) );

    if (direction < 0) {
	ch->println("Швырнуть в каком направлении?");
	return;
    }

    scanRange = scanRange0 = max( 1, ch->getModifyLevel( ) / 10 );
    throwRange = 1 + ch->getModifyLevel( ) / 30;
    victim = find_char( ch, arg2.c_str( ), direction, &scanRange);

    if (!victim)
	return;

    if (scanRange0 - scanRange > throwRange) {
	ch->println("Жертва стоит слишком далеко.");
	return;
    }

    if (victim == ch) {
	ch->println("Просто ударь себя этим камнем по лбу.");
	return;
    }

    if (is_safe_nomessage( ch, victim )) {
	act( "Боги покровительствуют $C3.", ch, 0, victim, TO_CHAR);
	return;
    }

    if (ch->in_room == victim->in_room) {
	ch->println("Ты не можешь швыряться камнями в упор.");
	return;
    }

    if (!( stone = get_eq_char( ch, wear_hold ) )
	|| stone->item_type != ITEM_WEAPON
	|| stone->value[0] != WEAPON_STONE)
    {
	ch->println("Возьми в руку камень!");
	return;
    }
/*
    if (get_eq_char(ch, wear_wield)) {
	ch->println("Твоя вторая рука дожна быть свободна!");
	return;    	
    }
*/
    ch->setWait( gsn_throw_stone->getBeats( ) );
    set_violent( ch, victim, false );
    act( "Ты швыряешь $o4 $T.", ch, stone, dirs[ direction ].leave, TO_CHAR );
    act( "$c1 швыряет $o4 $T.", ch, stone, dirs[ direction ].leave, TO_ROOM );

    if (victim->position == POS_SLEEPING)
	chance += 40;
    if (victim->position == POS_RESTING)
	chance += 10;
    if (victim->position == POS_FIGHTING)
	chance -= 40;

    chance += ch->hitroll - ch->getRealLevel( );
    dam = dice(stone->value[1], stone->value[2]);
    dam += ch->damroll + get_str_app(ch).missile;

    if (ch->size < SIZE_HUGE)
	dam /= 2;

    if (number_percent( ) <= gsn_flaming_stone->getEffective( ch )) {
	Affect saf;
	saf.where	       = TO_WEAPON;
	saf.type               = gsn_flaming_stone;
	saf.level              = level;
	saf.duration           = -1;
        saf.bitvector	= WEAPON_FLAMING;
	affect_to_obj( stone, &saf);

    } else if (number_percent( ) <= gsn_freezing_stone->getEffective( ch )) {
	Affect saf;
	saf.where	       = TO_WEAPON;
	saf.type               = gsn_freezing_stone;
	saf.level              = level;
	saf.duration           = -1;
	saf.bitvector	= WEAPON_FROST;
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


