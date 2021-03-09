
/* $Id: objthrow.cpp,v 1.1.2.5 2008/04/14 20:12:37 rufina Exp $
 *
 * ruffina, 2004
 */
/***************************************************************************
 * Все права на этот код 'Dream Land' пренадлежат Igor {Leo} и Olga {Varda}*
 * Некоторую помощь в написании этого кода, а также своими идеями помогали:*
 *    Igor S. Petrenko     {NoFate, Demogorgon}                            *
 *    Koval Nazar          {Nazar, Redrum}                                 *
 *    Doropey Vladimir     {Reorx}                                         *
 *    Kulgeyko Denis       {Burzum}                                        *
 *    Andreyanov Aleksandr {Manwe}                                         *
 *    и все остальные, кто советовал и играл в этот MUD                    *
 ***************************************************************************/
#include "objthrow.h"

#include "skill.h"

#include "npcharacter.h"
#include "pcharacter.h"
#include "room.h"
#include "object.h"
#include "affect.h"

#include "act_move.h"
#include "gsn_plugin.h"
#include "fight.h"
#include "damage.h"
#include "handler.h"
#include "effects.h"
#include "magic.h"
#include "clanreference.h"
#include "interp.h"
#include "stats_apply.h"
#include "merc.h"
#include "mercdb.h"
#include "act.h"
#include "def.h"

CLAN(battlerager);
GSN(accuracy);
PROF(warrior);
PROF(samurai);
PROF(paladin);
PROF(thief);
PROF(ninja);

static void arrow_damage( Object *arrow, Character *ch, Character *victim, int damroll, int door );

static bool check_rock_catching( Character *victim, Object *obj )
{
    if (obj->item_type != ITEM_WEAPON)
        return false;
    if (obj->value0() != WEAPON_STONE)
        return false;
    if (victim->size < SIZE_HUGE) 
        return false;
    return true;
}

bool check_obj_dodge( Character *ch, Character *victim, Object *obj, int bonus )
{
    int chance;

    if ( !IS_AWAKE(victim) || MOUNTED(victim) )
        return false;

    if ( victim->is_npc() )
        chance  = min( static_cast<short>( 30 ), victim->getModifyLevel() );
    else
    {
        int prof = victim->getProfession( );

        chance  = gsn_dodge->getEffective( victim ) / 2;
        chance += victim->getCurrStat(STAT_DEX) - 20;

        if (prof == prof_warrior || prof == prof_samurai || prof == prof_paladin)
            chance += chance / 5;
        else if (prof == prof_thief || prof == prof_ninja)
            chance += chance / 10;
    }

    chance -= (bonus - 90);

    bool canCatchMissile = (!victim->is_npc() && victim->getClan( ) == clan_battlerager);
    bool canCatchRock = check_rock_catching(victim, obj);

    if (!canCatchMissile && !canCatchRock)
        chance /= 2;
        
    if (number_percent( ) >= chance)
        return false;

    if (canCatchRock) {
        oldact("Ты ловишь руками $o4.",ch,obj,victim,TO_VICT);
        oldact("$C1 ловит руками $o4.",ch,obj,victim,TO_CHAR);
        oldact("$c1 ловит руками $o4.",victim,obj,ch,TO_NOTVICT);
        obj_to_char(obj,victim);
    }
    else if (canCatchMissile) {
        oldact("Ты ловишь руками $o4.",ch,obj,victim,TO_VICT);
        oldact("$C1 ловит руками $o4.",ch,obj,victim,TO_CHAR);
        oldact("$c1 ловит руками $o4.",victim,obj,ch,TO_NOTVICT);
        obj_to_char(obj,victim);
    } else {
        oldact("Ты уклоняешься от $o2.",ch,obj,victim,TO_VICT);
        oldact("$C1 уклоняется от $o2.",ch,obj,victim,TO_CHAR);
        oldact("$c1 уклоняется от $o2.",victim,obj,ch,TO_NOTVICT);
        obj_to_room(obj,victim->in_room);
        gsn_dodge->improve( victim, true, ch );
    }

    return true;
}

int send_arrow( Character *ch, Character *victim, Object *arrow, int door, int chance ,int bonus)
{
    EXIT_DATA *pExit;
    Room *dest_room;
    int damroll=0,hitroll=0;
    
    for (auto &paf: arrow->affected)
    {
            if ( paf->location == APPLY_DAMROLL )
                    damroll += paf->modifier;
            if ( paf->location == APPLY_HITROLL )
                    hitroll += paf->modifier;
    }

    dest_room = ch->in_room;
    chance += (hitroll + (ch->getCurrStat(STAT_DEX) - 18)) * 2;
    damroll *= 10;
    damroll += bonus;

    while (1)
    {
        chance -= 10;

        if (victim->in_room == dest_room)
        {/* снайперским выстрелом попали в комнату с мишенью :-) */
            if (number_percent() < chance)
            {
                if ( check_obj_dodge(ch,victim,arrow,chance))
                        return 0;
                        
                oldact("$o1 поражает тебя!", victim, arrow, 0, TO_CHAR);
                oldact("$o1 поражает $C4!", ch, arrow, victim, TO_CHAR);

                if (ch->in_room == victim->in_room)
                    oldact("$o1 $c2 поражает $C4!", ch, arrow, victim, TO_NOTVICT);
                else
                {
                    oldact("$o1 $c2 поражает $C4!", ch, arrow, victim, TO_ROOM);
                    oldact("$o1 поражает $c4!", victim, arrow, 0, TO_ROOM);
                }

                if ( is_safe(ch,victim)
                        || ( victim->is_npc() && IS_SET(victim->act,ACT_NOTRACK)) )
                {
                    oldact("$o1 отскакивает от $c2, не причиняя вреда...",victim,arrow,0,TO_ALL);
                    oldact("$o1 отскакивает от $c2, не причиняя вреда...",victim,arrow,0,TO_CHAR);
                    obj_to_room(arrow,victim->in_room);
                }
                else
                {
                    arrow_damage( arrow, ch, victim, damroll, door );
                }

                return 1;
            }
            else
            {
                obj_to_room(arrow,victim->in_room);
                oldact("$o1 падает на землю у твоих ног!",victim,arrow,0, TO_ALL);
                return 0;
            }
        }

        pExit = dest_room->exit[ door ];
        
        if ( !pExit )
            break;
        else
        {
            dest_room = pExit->u1.to_room;
            oldact("$o1 прилетает $T!", dest_room->people, arrow, 
                                     dirs[dirs[door].rev].enter, TO_ALL);
        }
    }

    return 0;
}
        

static void arrow_damage( Object *arrow, Character *ch, Character *victim,
                          int damroll, int door )
{
    int dam, sn, dam_type;

    if (arrow->item_type == ITEM_WEAPON)
        dam_type = attack_table[arrow->value3()].damage;
    else
        dam_type = DAM_BASH;

    sn = get_weapon_skill( arrow )->getIndex( );
    dam = dice( arrow->value1(), arrow->value2() );

    if (ch->isAffected( gsn_accuracy ))
        dam *= 2;

    dam = number_range( dam, 2 * dam );
    dam += damroll + (get_str_app(ch).missile);

    if (IS_WEAPON_STAT(arrow,WEAPON_POISON))
    {
        short level;
        Affect *poison, af;

        poison = arrow->affected.find(gsn_poison);
        if (!poison)
            level = arrow->level;
        else
            level = poison->level;

        if (!saves_spell(level,victim,DAM_POISON))
        {
            victim->pecho("Ты чувствуешь как яд растекается по твоим венам.");
            oldact("$c1 отравле$gно|н|на ядом от $o2.", victim,arrow,0,TO_ROOM);

            af.bitvector.setTable(&affect_flags);
            af.type      = gsn_poison;
            af.level     = level * 3/4;
            af.duration  = level / 2;
            af.location = APPLY_STR;
            af.modifier  = -1;
            af.bitvector.setValue(AFF_POISON);
            af.sources.add(ch);
            af.sources.add(arrow);
            affect_join( victim, &af );
        }

    }

    if (IS_WEAPON_STAT(arrow,WEAPON_FLAMING))
    {
        oldact("$o1 обжигает $c4.",victim,arrow,0,TO_ROOM);
        oldact("$o1 обжигает тебя.",victim,arrow,0,TO_CHAR);
        fire_effect( (void *) victim,arrow->level,dam,TARGET_CHAR);
    }
    
    if (IS_WEAPON_STAT(arrow,WEAPON_FROST))
    {
        oldact("$o1 обмораживает $c4.",victim,arrow,0,TO_ROOM);
        oldact("$o1 обмораживает тебя.",victim,arrow,0,TO_CHAR);
        cold_effect(victim,arrow->level,dam,TARGET_CHAR);
    }
    
    if (IS_WEAPON_STAT(arrow,WEAPON_SHOCKING))
    {
        oldact("$o1 парализует $c4 разрядом молнии.",victim,arrow,0,TO_ROOM);
        oldact("$o1 парализует тебя разрядом молнии.",victim,arrow,0,TO_CHAR);
        shock_effect(victim,arrow->level,dam,TARGET_CHAR);
    }

    if ( dam_type == DAM_PIERCE
            && dam > victim->max_hit / 10
            && number_percent() < 50 )
    {
        Affect af;

        af.bitvector.setTable(&affect_flags);
        af.type      = sn;
        af.level     = ch->getModifyLevel();
        af.duration  = -1;
        af.location = APPLY_HITROLL;
        af.modifier  = - (dam / 20);
        af.bitvector.setValue(AFF_CORRUPTION);

        affect_join( victim, &af );

        obj_to_char(arrow,victim);
        equip_char(victim,arrow,wear_stuck_in);
    }
    else
        obj_to_room(arrow,victim->in_room);

    damage_nocatch( ch, victim, dam, sn, dam_type, true, DAMF_WEAPON );
    ch->setLastFightTime( );
    victim->setLastFightTime( );

    if (victim->is_npc( ) && victim->getNPC( )->behavior)
        victim->getNPC( )->behavior->shooted( ch, door );
}
