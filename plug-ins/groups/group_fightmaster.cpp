
/* $Id: group_fightmaster.cpp,v 1.1.2.24.6.20 2010-09-01 21:20:45 rufina Exp $
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

#include "logstream.h"
#include "skill.h"
#include "skillcommandtemplate.h"
#include "skillmanager.h"

#include "movetypes.h"
#include "directions.h"
#include "terrains.h"
#include "move_utils.h"
#include "doors.h"
#include "affect.h"
#include "commonattributes.h"


#include "npcharacter.h"
#include "object.h"
#include "pcharacter.h"
#include "race.h"
#include "room.h"

#include "act.h"
#include "clanreference.h"
#include "damage.h"
#include "def.h"
#include "fight.h"
#include "loadsave.h"
#include "interp.h"
#include "magic.h"
#include "material.h"
#include "merc.h"

#include "morphology.h"
#include "onehit.h"
#include "onehit_weapon.h"

#include "skill_utils.h"
#include "roomutils.h"
#include "vnum.h"
#include "l10n.h"

GSN(area_attack);
GSN(bash);
GSN(bash_door);
GSN(blind_fighting);
GSN(cavalry);
GSN(crush);
GSN(double_kick);
GSN(forest_fighting);
GSN(kick);
GSN(protective_shield);
GSN(smash);
GSN(parry);
GSN(hand_block);
GSN(shield_block);
GSN(dodge);
GSN(trip);
GSN(slice);
CLAN(shalafi);
PROF(anti_paladin);
PROF(samurai);
PROF(warrior);
PROF(paladin);
PROF(ninja);
PROF(thief);
WEARLOC(tat_wrist_l);
WEARLOC(tat_wrist_r);

static auto needsSpam = [](Character *target) {
    return target->getPC() 
            && IS_SET(target->getPC()->config, CONFIG_FIGHTSPAM);
};


/*
 * 'area attack' skill command
 */
SKILL_DECL(areaattack);
SKILL_APPLY(areaattack)
{
    int count = 0, max_count;
    Character *vch, *vch_next;

    if (number_percent() >= gsn_area_attack->getEffective(ch))
        return false;

    gsn_area_attack->improve(ch, true, victim);

    int slevel = skill_level(*gsn_area_attack, ch);

    if (slevel < 70)
        max_count = 1;
    else if (slevel < 80)
        max_count = 2;
    else if (slevel < 90)
        max_count = 3;
    else
        max_count = 4;

    for (vch = ch->in_room->people; vch != 0; vch = vch_next) {
        vch_next = vch->next_in_room;
        if (vch != victim && vch->fighting == ch) {
            one_hit_nocatch(ch, vch);
            count++;
        }
        if (count == max_count)
            break;
    }

    return true;
}


/*
 * 'hand block' skill command
 */
SKILL_DECL(handblock);
SKILL_APPLY(handblock)
{
   int chance;

    if ( !IS_AWAKE(victim) )
        return false;
    
    if (!check_bare_hands( victim ))
        return false;

    if ( IS_AFFECTED(victim,AFF_STUN) )
        return false;
    
    if (!victim->getWearloc( ).isSet( wear_hands ))
        return false;

    if ( victim->is_npc() || !gsn_hand_block->usable( victim ))
        return false;
        
    if ((chance = gsn_hand_block->getEffective( victim )) <= 1)
        return false;

    if ( victim->getProfession( ) == prof_ninja) {
        chance /= 2;
    }
    else
        chance /= 3;

    if ( !victim->can_see( ch ) )
    {
        if (number_percent( ) < gsn_blind_fighting->getEffective( victim ))
        {
            gsn_blind_fighting->improve( victim, true, ch );
            chance = ( int )( chance / 1.5 );
        }
        else
            chance = ( int )( chance / 4 );
    }

    if ( IS_AFFECTED(victim,AFF_WEAK_STUN) )
    {
        chance = ( int )( chance * 0.5 );
    }

    if ( number_percent( ) >= chance +( skill_level(*gsn_hand_block, victim) - ch->getModifyLevel() ) ) 
        return false;
    
    victim->move -= move_dec( victim );

    if(SHADOW(victim))
    {
        echo_char(victim, needsSpam, _("Тень пинает тебя.") );
        echo_char(ch, needsSpam, _("Тень и %2$C1 играют в кунг-фу."), ch, victim );
        echo_notvict( ch, victim, needsSpam, _("Тень и %2$C1 играют в кунг-фу."), ch, victim );
        return false;
    }

    echo_char(victim, needsSpam, _("Ты отражаешь руками атаку %1$C2."), ch, victim);
    echo_char(ch, needsSpam, _("%2$^C1 отражает руками твою атаку."), ch, victim);
    echo_notvict(ch, victim, needsSpam, _("%2$^C1 отражает руками атаку %1$C2."), ch, victim);

    gsn_hand_block->improve( victim, true, ch );
    return true;}



