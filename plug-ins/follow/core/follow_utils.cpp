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

    if (ch->master->can_see( ch ))
       act( "$c1 теперь следует за тобой.", ch, 0, mch, TO_VICT );
       
    act( "Ты теперь следуешь за $C5.",  ch, 0, mch, TO_CHAR );
}

static void afprog_stopfol( Character *ch )
{
    Affect *paf, *paf_next;

    for (paf = ch->affected; paf; paf = paf_next) {
	paf_next = paf->next;

	if (paf->type->getAffect( ))
	    paf->type->getAffect( )->stopfol( ch, paf );
    }
}

static bool mprog_stopfol( Character *ch, Character *master )
{
    FENIA_CALL( ch, "Stopfol", "C", master );
    FENIA_NDX_CALL( ch->getNPC( ), "Stopfol", "CC", ch, master );
    BEHAVIOR_VOID_CALL( ch->getNPC( ), stopfol, master );
    return false;
}

void follower_stop( Character *ch )
{
    Character *master = ch->master;

    if (master == NULL)
	return;

    if (IS_AFFECTED( ch, AFF_CHARM )) { /* XXX causes double affect_strip */
	REMOVE_BIT( ch->affected_by, AFF_CHARM );
	affect_strip( ch, gsn_charm_person );
    }

    if (master->can_see( ch )) 
       act( "$c1 теперь не следует за тобой.", ch, 0, master, TO_VICT );

    act( "Ты теперь не следуешь за $C5.", ch, 0, master, TO_CHAR );

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
 Character *ch,*vch,*ch_next,*vch_next;

 for (ch = ach; ch != 0; ch = ch_next) {
    if (!ch->in_room)
	return false;
    
    ch_next = ch->leader;
    
    for(vch = bch; vch != 0; vch = vch_next) {
	if (!vch->in_room)
	    return false;
	
	vch_next = vch->leader;
	
	if (ch == vch) 
	    return true;
	
	if (vch == vch_next)
	    break;
    }
   
    if (ch == ch_next)
	break;
 }
 return false;
}

/*
 * finds a visible person following this character
 */
Character * follower_find( Character *ch, const char *cargument )
{
    Character *wch;
    int number, count;
    char argument[MAX_STRING_LENGTH], arg[MAX_STRING_LENGTH];

    strcpy( argument, cargument );
    number = number_argument( argument, arg );
    count  = 0;

    if (!str_cmp( arg, "self" ) || !str_cmp( arg, "я" ))
	return ch;
    
    for (wch = char_list; wch; wch = wch->next) {
	if (wch->in_room == 0)
	    continue;
	if (wch->master != ch && wch->leader != ch)
	    continue;
	if (!ch->can_see( wch ))
	    continue;
	if (!is_name( arg, wch->getNameP( '7' ).c_str( ) ))
	    continue;
	if (++count >= number)
	    return wch;
    }

    return 0;
}

Character * follower_find_nosee( Character *ch, const char *cargument )
{
    Character *wch;
    int number, count;
    char argument[MAX_STRING_LENGTH], arg[MAX_STRING_LENGTH];

    strcpy( argument, cargument );
    number = number_argument( argument, arg );
    count  = 0;

    if (!str_cmp( arg, "self" ) || !str_cmp( arg, "я" ))
	return ch;
    
    for (wch = char_list; wch; wch = wch->next) {
	if (wch->in_room == 0)
	    continue;
	if (wch->master != ch && wch->leader != ch)
	    continue;
	if (!is_name( arg, wch->getNameP( '7' ).c_str( ) ))
	    continue;
	if (++count >= number)
	    return wch;
    }

    return 0;
}

/*
 * guarding utils
 */
void guarding_stop( PCharacter *guard, PCharacter *victim )
{
    act("Ты прекращаешь охранять $C4.", guard, 0, victim, TO_CHAR);
    act("$c1 прекращает охранять тебя.", guard, 0, victim, TO_VICT);
    act("$c1 прекращает охранять $C4.", guard, 0, victim, TO_NOTVICT);
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
	    && !IS_AFFECTED(gch, AFF_CHARM) 
	    && is_same_group( gch, ch ))
	    members.insert( gch );

    return members;
}

GroupMembers party_members_world( Character *ch )
{
    GroupMembers members;

    for (Character *gch = char_list; gch != NULL; gch = gch->next) 
	if (!gch->is_npc( )
	    && !IS_AFFECTED(gch, AFF_CHARM) 
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

