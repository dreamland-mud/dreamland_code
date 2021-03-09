
/* $Id: group_healing.cpp,v 1.1.2.13.6.4 2008/05/21 08:15:31 rufina Exp $
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

#include "spelltemplate.h"

#include "so.h"
#include "pcharacter.h"
#include "room.h"
#include "object.h"
#include "affect.h"
#include "magic.h"
#include "fight.h"
#include "act_move.h"
#include "gsn_plugin.h"

#include "handler.h"
#include "merc.h"
#include "mercdb.h"
#include "act.h"
#include "def.h"



SPELL_DECL(Aid);
VOID_SPELL(Aid)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    if (ch->isAffected(sn)) {
        ch->pecho("Это заклинание использовалось совсем недавно.");
        return;
    }

    victim->hit += level * 5; // Historically allowed to go above max_hit.
    update_pos( victim );
    victim->pecho("Волна тепла согревает твое тело.");
    act("%^C1 выглядит лучше.", victim, 0, 0, TO_ROOM);
    if (ch != victim) ch->pecho("Ok.");

    postaffect_to_char(ch, sn, level / 50);
}


SPELL_DECL(Assist);
VOID_SPELL(Assist)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
        if ( ch->isAffected(sn ) )
        {
                ch->pecho("Это заклинание использовалось совсем недавно.");
                return;
        }

        victim->hit += 100 + level * 5;  // Historically allowed to go above max_hit.
        update_pos( victim );
        victim->pecho("Волна тепла согревает твое тело.");
        act("%^C1 выглядит лучше.", victim, 0, 0, TO_ROOM);
        if ( ch != victim )
                ch->pecho("Ok.");

        postaffect_to_char(ch, sn, level / 50);
}

SPELL_DECL(Refresh);
VOID_SPELL(Refresh)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    victim->move = min( victim->move + level, (int)victim->max_move );
    if (victim->max_move == victim->move)
    {
        oldact("Ты пол$gно|он|на сил!", victim, 0, 0, TO_CHAR);
    }
    else
        victim->pecho("Усталость проходит.");

    if ( ch != victim )
        ch->pecho("Ok.");
}


/** Ensure that hit points value is no bigger than max HP. 
 *  If max HP is negative don't make situation even worse.
 */
static void adjust_hit_points(Character *victim)
{
    int hit_upper_limit = max(0, (int)victim->max_hit);
    victim->hit = min((int)victim->hit, hit_upper_limit);
}

SPELL_DECL(CureLight);
VOID_SPELL(CureLight)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    victim->hit += dice(1, 8) + level / 4 + 5;
    adjust_hit_points(victim);
    update_pos( victim );
    victim->pecho("Ты чувствуешь себя слегка лучше!");

    if ( ch != victim )
        ch->pecho("Ok.");
}

SPELL_DECL(CureSerious);
VOID_SPELL(CureSerious)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    victim->hit += dice(2, 8) + level / 3 + 10;
    adjust_hit_points(victim);
    victim->pecho("Ты чувствуешь себя лучше!");

    if ( ch != victim )
        ch->pecho("Ok.");
}

SPELL_DECL(CureCritical);
VOID_SPELL(CureCritical)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    victim->hit += dice(3, 8) + level / 2 + 20;
    adjust_hit_points(victim);
    update_pos( victim );
    victim->pecho("Ты чувствуешь себя намного лучше!");

    if ( ch != victim )
        ch->pecho("Ok.");
}


SPELL_DECL(Heal);
VOID_SPELL(Heal)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    victim->hit += level * 2 + number_range( 40, 45 ); 
    adjust_hit_points(victim);    
    update_pos( victim );
    victim->pecho("Волна тепла согревает твое тело.");

    if ( ch != victim )
        ch->pecho("Ok.");
}

SPELL_DECL(SuperiorHeal);
VOID_SPELL(SuperiorHeal)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    victim->hit += level * 3 + number_range( 100, 120 ); 
    adjust_hit_points(victim);    
    update_pos( victim );
    victim->pecho("Волна тепла окутывает тебя.");

    if ( ch != victim )
        ch->pecho("Ok.");
}


SPELL_DECL(MasterHealing);
VOID_SPELL(MasterHealing)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    victim->hit += level * 5 + number_range( 60, 100 ); 
    adjust_hit_points(victim);    
    update_pos( victim );
    victim->pecho("Волна тепла согревает твое тело.");

    if ( ch != victim )
        ch->pecho("Ok.");
}


SPELL_DECL(MassHealing);
VOID_SPELL(MassHealing)::run( Character *ch, Room *room, int sn, int level ) 
{ 
    Character *gch;

    for ( gch = room->people; gch != 0; gch = gch->next_in_room )
    {
        if ((ch->is_npc() && gch->is_npc()) ||
            (!ch->is_npc() && !gch->is_npc()))
        {
            if (spellbane( ch, gch ))
                continue;

            spell(gsn_heal,level,ch, gch);
            spell(gsn_refresh,level,ch, gch);
        }
    }

}

SPELL_DECL(GroupHeal);
VOID_SPELL(GroupHeal)::run( Character *ch, Room *room, int sn, int level ) 
{ 
    Character *gch;
    int heal_sn;

    if (gsn_master_healing->usable( ch ))
        heal_sn = gsn_master_healing;
    else if (gsn_superior_heal->usable( ch ))
        heal_sn = gsn_superior_heal;
    else
        heal_sn = gsn_heal;

    for ( gch = room->people; gch != 0; gch = gch->next_in_room )
    {
        if( !is_same_group( gch, ch ) )
                continue;

        if (spellbane( ch, gch ))
            continue;
        
        spell(heal_sn,level,ch, gch);
        spell(gsn_refresh,level,ch, gch);
    }

}

SPELL_DECL(EmpathicHealing);
VOID_SPELL(EmpathicHealing)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af, *paf;
    int hp;
    bool removed = false;

    if (ch == victim) {
        ch->pecho("Это заклинание ты можешь использовать только на других.");
        return;
    }

    if (ch->isAffected(sn )) {
        ch->pecho("Это заклинание использовалось совсем недавно.");
        return;
    }

    if (IS_AFFECTED(victim, AFF_PLAGUE)
        && (paf = victim->affected.find(gsn_plague)) != NULL)
    {
        affect_join( ch, paf );
        ch->pecho("Ты чувствуешь жар и лихорадку.");
        removed = true;
        affect_strip( victim, gsn_plague );
    }

    if ( IS_AFFECTED(victim, AFF_POISON)
        && (paf = victim->affected.find(gsn_poison)) != NULL)
    {
        affect_join( ch, paf );
        ch->pecho("Ты чувствуешь себя очень болезненно.");
        removed = true;
        affect_strip( victim, gsn_poison );
    }
   
    af.bitvector.setTable(&affect_flags);
    af.type             = sn;
    af.level            = level;
    
    if (!removed && victim->max_hit == victim->hit) {
        oldact("Кажется, $C1 абсолютно здоро$Gво|в|ва", ch, 0, victim, TO_CHAR);
        af.duration = 1;
    
    } else {
        act("Сосредоточившись, ты переносишь раны %2$C2 на собственное тело.", ch, 0, victim, TO_CHAR);
        act("Сосредоточившись, %C1 переносит твои раны на собственное тело.", ch, 0, victim, TO_VICT);
        oldact("Сосредоточившись, $c1 переносит раны $C2 на собственное тело.", ch, 0, victim, TO_NOTVICT);

        hp = victim->max_hit - victim->hit;
        hp = URANGE( 0, hp, ch->hit - 1 );

        victim->hit += hp;
        ch->hit     -= hp;
        update_pos( victim );

        af.duration         = hp / 100;
        af.bitvector.setValue(AFF_REGENERATION);
    }

    affect_join( ch, &af );

}

