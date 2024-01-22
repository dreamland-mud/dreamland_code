/* $Id$
 *
 * ruffina, 2004
 */
#include "follow_utils.h"

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "skillreference.h"
#include "affecthandler.h"
#include "affect.h"
#include "behavior_utils.h"
#include "npcharacter.h"
#include "pcharacter.h"
#include "room.h"

#include "dreamland.h"
#include "act.h"
#include "save.h"
#include "loadsave.h"
#include "interp.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

GSN(charm_person);

void follower_die( Character *ch )
{
    Character *fch;
    Character *fch_next;

    if (ch->master) 
        follower_stop( ch );

    ch->leader = 0;

    for (fch = char_list; fch != 0; fch = fch_next) {
        fch_next = fch->next;

        if (fch->master == ch)
            follower_stop( fch );

        if (fch->leader == ch)
            fch->leader = fch;
    }
}

void follower_add( Character *ch, Character *mch )
{
    if (ch->master != NULL)
        return;

    ch->master        = mch;
    ch->leader        = NULL;

    if (ch->master->can_see( ch ) || (ch->master->getPC() && ch->master->getPC()->pet && ch->master->getPC()->pet == ch))
       mch->pecho("%1$^C1 теперь следу%1$nет|ют за тобой.", ch);
    
    ch->pecho("Ты теперь следуешь за %1$C5.", mch);
}

static void afprog_stopfol( Character *ch )
{
    for (auto &paf: ch->affected.findAllWithHandler()) {
        if (paf->type->getAffect())
            paf->type->getAffect()->onStopfol(SpellTarget::Pointer(NEW, ch), paf);
    }
}

static bool mprog_stopfol( Character *ch, Character *master )
{
    FENIA_CALL( ch, "Stopfol", "C", master );
    FENIA_NDX_CALL( ch->getNPC( ), "Stopfol", "CC", ch, master );
    BEHAVIOR_VOID_CALL( ch->getNPC( ), stopfol, master );
    return false;
}

void affect_add_charm(Character *victim)
{
    Affect af;
    af.type      = gsn_charm_person;
    af.level     = victim->getModifyLevel();
    af.duration  = -2;
    af.bitvector.setTable(&affect_flags);
    af.bitvector.setValue(AFF_CHARM);
    affect_to_char( victim, &af );        
}



void follower_stop( Character *ch, bool verbose )
{
    Character *master = ch->master;

    if (master == NULL)
        return;

    if (IS_CHARMED(ch)) {
        REMOVE_BIT( ch->affected_by, AFF_CHARM );
        affect_bit_strip(ch, &affect_flags, AFF_CHARM);
    }

    follower_clear(ch, verbose);
}

void follower_clear( Character * ch, bool verbose )
{
    Character *master = ch->master;

    if (master == NULL)
        return;
   
    if (verbose && master->can_see( ch ))
        master->pecho("%1$^C1 больше не следу%1$nет|ют за тобой.", ch);

    if (verbose)
        ch->pecho("Ты больше не следуешь за %1$C5.", master);

    afprog_stopfol( ch );

    ch->master = NULL;
    ch->leader = NULL;
    
    mprog_stopfol( ch, master );

    if (!ch->is_npc( )) 
        return;
    
    if (ch->position == POS_SLEEPING && !IS_AFFECTED( ch, AFF_SLEEP ))
        ch->position = ch->getNPC( )->default_pos;

    if (!master->is_npc( ) && master->getPC( )->pet == ch)
        master->getPC( )->pet = NULL;

    save_mobs( ch->in_room );
}

/*
 * New is_same_group by chronos
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 */
bool is_same_group( Character *ach, Character *bch )
{
    set<Character *> leaders_a, leaders_b;

    // Create a set of: 'A', 'leader of A', 'leader of leader of A' and so on.
    for (Character *ch = ach; ch != 0; ch = ch->leader) {
        if (leaders_a.count(ch) > 0)
            break;

        leaders_a.insert(ch);
    }

    // Same for B.
    for (Character *ch = bch; ch != 0; ch = ch->leader) {
        if (leaders_b.count(ch) > 0)
            break;

        leaders_b.insert(ch);
    }

    // Check if anyone who leads A also leads B, and viceversa.
    for (auto &ch: leaders_a)
        if (leaders_b.count(ch) > 0)
            return true;

    for (auto &ch: leaders_b)
        if (leaders_a.count(ch) > 0)
            return true;

    return false;
}

/*
 * guarding utils
 */
void guarding_stop( PCharacter *guard, PCharacter *victim )
{
    oldact("Ты прекращаешь охранять $C4.", guard, 0, victim, TO_CHAR);
    oldact("$c1 прекращает охранять тебя.", guard, 0, victim, TO_VICT);
    oldact("$c1 прекращает охранять $C4.", guard, 0, victim, TO_NOTVICT);
    guard->guarding  = 0;
    victim->guarded_by = 0;
}

void guarding_nuke( Character *ch, Character *victim )
{
    if (ch->is_npc( ) || victim->is_npc( ))
        return;

    if (ch->getPC( )->guarding == victim || victim->getPC( )->guarded_by == ch)
        guarding_stop( ch->getPC( ), victim->getPC( ) );
}

void guarding_assert( Character *victim )
{
    PCharacter *gch;
    
    if (victim->is_npc( ))
        return;
        
    gch = victim->getPC( )->guarded_by;

    if (gch && !is_same_group( victim, gch )) 
        guarding_stop( gch, victim->getPC( ) );
}

void guarding_clear( Character *ch )
{
    if (!ch->is_npc( )) {
        PCharacter *pch = ch->getPC( );
        
        if (pch->guarded_by)
            guarding_stop( pch->guarded_by, pch );
        
        if (pch->guarding)
            guarding_stop( pch, pch->guarding );
    }
}

GroupMembers party_members_room( Character *ch, Room *room )
{
    GroupMembers members;

    if (!room)
        room = ch->in_room;

    for (Character *gch = room->people; gch != NULL; gch = gch->next_in_room) 
        if (!gch->is_npc( )
            && !IS_CHARMED(gch) 
            && is_same_group( gch, ch ))
            members.insert( gch );

    return members;
}

GroupMembers party_members_world( Character *ch )
{
    GroupMembers members;

    for (Character *gch = char_list; gch != NULL; gch = gch->next) 
        if (!gch->is_npc( )
            && !IS_CHARMED(gch) 
            && is_same_group( gch, ch ))
            members.insert( gch );

    return members;
}

GroupMembers group_members_room( Character *ch, Room *room )
{
    GroupMembers members;

    if (!room)
        room = ch->in_room;

    for (Character *gch = room->people; gch != NULL; gch = gch->next_in_room) 
        if (is_same_group( gch, ch ))
            members.insert( gch );

    return members;
}

GroupMembers group_members_world( Character *ch )
{
    GroupMembers members;

    for (Character *gch = char_list; gch != NULL; gch = gch->next) 
        if (is_same_group( gch, ch ))
            members.insert( gch );

    return members;
}

