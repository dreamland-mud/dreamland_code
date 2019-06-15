/* $Id: fight_subr.cpp,v 1.1.2.4 2008/05/27 21:30:02 rufina Exp $
 *
 * ruffina, 2004
 */
/***************************************************************************
 * Все права на этот код 'Dream Land' пренадлежат Igor {Leo} и Olga {Varda}*
 * Некоторую помощь в написании этого кода, а также своими идеями помогали:*
 *    Igor S. Petrenko            {NoFate, Demogorgon}                           *
 *    Koval Nazar            {Nazar, Redrum}                                    *
 *    Doropey Vladimir            {Reorx}                                           *
 *    Kulgeyko Denis            {Burzum}                                           *
 *    Andreyanov Aleksandr  {Manwe}                                           *
 *    и все остальные, кто советовал и играл в этот MUD                           *
 ***************************************************************************/

#include "skill.h"
#include "affect.h"
#include "room.h"
#include "pcharacter.h"
#include "dreamland.h"
#include "npcharacter.h"
#include "object.h"
#include "race.h"
#include "gsn_plugin.h"
#include "merc.h"
#include "mercdb.h"
#include "act.h"
#include "handler.h"
#include "interp.h"
#include "save.h"
#include "vnum.h"

#include "fight.h"
#include "def.h"

#ifndef FIGHT_STUB
bool check_stun( Character *ch, Character *victim ) 
{
    if ( IS_AFFECTED(ch,AFF_WEAK_STUN) )
    {
        act_p("{WТы оглуше$gно|н|на и не можешь реагировать на атаки $C2.{x",
            ch,0,victim,TO_CHAR,POS_FIGHTING);
        act_p("{W$c1 оглуше$gно|н|на и не может реагировать на твои атаки.{x",
            ch,0,victim,TO_VICT,POS_FIGHTING);
        act_p("{W$c1 оглуше$gно|н|на и не может реагировать на атаки.{x",
            ch,0,victim,TO_NOTVICT,POS_FIGHTING);

        REMOVE_BIT(ch->affected_by,AFF_WEAK_STUN);
        
        set_violent( ch, victim, false );
        return true;
    }

    if ( IS_AFFECTED(ch,AFF_STUN) )
    {
        act_p("{WТы оглуше$gно|н|на и не можешь реагировать на атаки $C2.{x",
            ch,0,victim,TO_CHAR,POS_FIGHTING);
        act_p("{W$c1 оглуше$gно|н|на и не может реагировать на твои атаки.{x",
            ch,0,victim,TO_VICT,POS_FIGHTING);
        act_p("{W$c1 оглуше$gно|н|на и не может реагировать на атаки.{x",
            ch,0,victim,TO_NOTVICT,POS_FIGHTING);

        affect_strip(ch,gsn_power_word_stun);

        SET_BIT(ch->affected_by,AFF_WEAK_STUN);

        set_violent( ch, victim, false );
        return true;
    }

    return false;
}

void check_assist(Character *ch, Character *victim)
{
    Character *rch, *rch_next;

    for (rch = ch->in_room->people; rch != 0; rch = rch_next) {
        rch_next = rch->next_in_room;
        
        if (!IS_AWAKE(rch) || rch->fighting || rch == ch || rch == victim)
            continue;
        
        /* mobile assistance */
        if (rch->is_npc( )) {
            if (rch->getNPC( )->behavior)
                rch->getNPC( )->behavior->assist( ch, victim );

            continue;
        }
        
        /* PC-master assist his charmices */
        if (ch->is_npc( )
            && IS_AFFECTED(ch, AFF_CHARM)
            && ch->master == rch)
        {
            one_hit( rch, victim );
            continue;
        }
        
        /* PC assist characters from his group */
        if (IS_SET(rch->act, PLR_AUTOASSIST)
            && is_same_group( ch, rch ))
        {
            act("Ты вступаешь в битву на стороне $C2.", rch, 0, ch, TO_CHAR);
            one_hit( rch, victim );
            continue;
        }
    }
}


bool check_bare_hands( Character *ch )
{
    Object *obj;

    for (obj = ch->carrying; obj; obj = obj->next_content)
        if (obj->wear_loc == wear_wield
            || obj->wear_loc == wear_second_wield
            || obj->wear_loc == wear_shield
            || obj->wear_loc == wear_hold)
            return false;
    
    return true;
}

void check_bloodthirst( Character *ch )
{
    Character *vch, *vch_next;

    if (dreamland->hasOption( DL_BUILDPLOT ))
        return;
    if (!IS_AFFECTED(ch, AFF_BLOODTHIRST))
        return;
    if (!IS_AWAKE(ch))
        return;
    if (ch->fighting)
        return;

    // Prevent bloodfirsty and stunned mobs from spam-attacking.
    if (ch->is_npc() && ch->wait > 0)
        return;
    
    for (vch = ch->in_room->people; vch && !ch->fighting; vch = vch_next)
    {
        vch_next = vch->next_in_room;

        if (ch != vch && ch->can_see(vch) && !is_safe_nomessage(ch, vch))
        {
            ch->println( "{RБОЛЬШЕ КРОВИ! БОЛЬШЕ КРОВИ! БОЛЬШЕ КРОВИ!!!{x" );
            REMOVE_BIT(ch->affected_by, AFF_CHARM);

            if (ch->is_npc( ) && ch->in_room) 
                save_mobs( ch->in_room );

            interpret_raw( ch, "murder",  vch->getDoppel( ch )->getNameP( ) );
        }
    }
}

#else
void        check_assist(Character *ch,Character *victim) { }
bool        check_stun( Character *ch, Character *victim ) { return false; } 
bool        check_bare_hands( Character *ch ) { return false; }
void        check_bloodthirst( Character *ch ) { }
#endif

