
/* $Id: group_illusion.cpp,v 1.1.2.12.6.7 2008/07/27 07:42:19 rufina Exp $
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

#include "char.h"
#include "so.h"
#include "pcharacter.h"
#include "room.h"
#include "object.h"
#include "affect.h"
#include "magic.h"
#include "fight.h"
#include "act_move.h"
#include "interp.h"
#include "gsn_plugin.h"

#include "merc.h"
#include "mercdb.h"
#include "handler.h"
#include "act.h"
#include "def.h"

PROF(samurai);


SPELL_DECL(Fear);
VOID_SPELL(Fear)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;

    if ((victim->getProfession( ) == prof_samurai) && ( victim->getModifyLevel() >=10) ) {
	 act("Это заклинание не может причинить вреда твоему противнику.", ch, 0, 0, TO_CHAR);
	 return;
    }

    if (victim->isAffected(gsn_fear)) {
	if (ch == victim)
	    act("Ты и так дрожишь от страха.", ch, 0, 0, TO_CHAR);
	else
	    act("$C1 уже дрожит от страха.", ch, 0, victim, TO_CHAR);
	return;
    }

    if (saves_spell( level, victim,DAM_OTHER, ch, DAMF_SPELL)) {
	act("Тебе не удалось запугать $C4...", ch, 0, victim, TO_CHAR); 
	return;
    }

    af.where     = TO_DETECTS;
    af.type      = gsn_fear;
    af.level     = level;
    af.duration  = level / 10;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = ADET_FEAR;
    affect_to_char( victim, &af );
    act("Ты дрожишь от страха.", victim, 0, 0, TO_CHAR);
    act("$c1 дрожит от страха.", victim, 0, 0, TO_ROOM);
}

SPELL_DECL(ImprovedInvis);
VOID_SPELL(ImprovedInvis)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Affect af;

    if ( IS_AFFECTED(ch, AFF_IMP_INVIS) ) {
	ch->send_to("Тебя уже и так совсем не видно.\r\n");
	return;
    }

    if (IS_AFFECTED(ch, AFF_FAERIE_FIRE)) {
	ch->send_to("Ты не можешь стать совсем невидимым, когда светишься.\r\n");
	return;
    }

    act_p("$c1 становится совсем невидим$gым|ым|ой.",
           ch, 0, 0, TO_ROOM,POS_RESTING);

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 10 ;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_IMP_INVIS;
    affect_to_char( ch, &af );

    act("Ты становишься совсем невидим$gым|ым|ой.", ch, 0, 0, TO_CHAR);
}


SPELL_DECL(Invisibility);
VOID_SPELL(Invisibility)::run( Character *ch, Object *obj, int sn, int level ) 
{
    Affect af;

    if (IS_OBJ_STAT(obj,ITEM_INVIS))
    {
	ch->pecho( "%1$^O1 уже невиди%1$Gмо|м|ма|мы.", obj );
	return;
    }

    ch->in_room->echo( POS_RESTING, "%1$^O1 станов%1$nится|ятся невидим%1$Gым|ым|ой|ыми.", obj );

    af.where	= TO_OBJECT;
    af.type		= sn;
    af.level	= level;
    af.duration	= level / 4 + 12;
    af.location	= APPLY_NONE;
    af.modifier	= 0;
    af.bitvector	= ITEM_INVIS;
    affect_to_obj( obj, &af);
}

VOID_SPELL(Invisibility)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Affect af;

    if ( IS_AFFECTED(victim, AFF_INVISIBLE) ) {
	if (victim == ch)
	    ch->send_to("Тебя уже и так не видно.\r\n");
	else
	    act("$C1 уже и так невиди$Gмо|м|ма.", ch, 0, victim, TO_CHAR);
	return;
    }

    act_p("$c1 становится невидим$gым|ым|ой.",
           victim, 0, 0, TO_ROOM,POS_RESTING);

    af.where     = TO_AFFECTS;
    af.type      = sn;
    af.level     = level;
    af.duration  = (level / 8 + 10);
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = AFF_INVISIBLE;
    affect_to_char( victim, &af );

    act("Ты становишься невидим$gым|ым|ой.", victim, 0, 0, TO_CHAR);
}


SPELL_DECL(MassInvis);
VOID_SPELL(MassInvis)::run( Character *ch, Room *room, int sn, int level ) 
{ 
    Affect af;
    Character *gch;

    for ( gch = room->people; gch != 0; gch = gch->next_in_room )
    {
	if ( !is_same_group( gch, ch ) || IS_AFFECTED(gch, AFF_INVISIBLE) )
	    continue;

	if (spellbane( ch, gch ))
	    continue;

        act("$c1 становится невидим$gым|ым|ой.", gch, 0, 0, TO_ROOM);
	act("Ты становишься невидим$gым|ым|ой.", gch, 0, 0, TO_CHAR);

	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level     = level/2;
	af.duration  = 24;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_INVISIBLE;
	affect_to_char( gch, &af );
    }
    ch->send_to("Ok.\n\r");
}


SPELL_DECL(Ventriloquate);
VOID_SPELL(Ventriloquate)::run( Character *ch, char *target_name, int sn, int level ) 
{ 
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char speaker[MAX_INPUT_LENGTH];
    Character *vch;

    target_name = one_argument( target_name, speaker );

    sprintf( buf1, "%s произносит '{g%s{x'.\n\r", speaker, target_name );
    sprintf( buf2, "Кто-то заставляет %s произнести '{g%s{x'.\n\r", speaker, target_name );
    buf1[0] = Char::upper(buf1[0]);

    for ( vch = ch->in_room->people; vch != 0; vch = vch->next_in_room )
    {
	if ( !is_name( speaker, vch->getNameP( ) ) )
	    vch->send_to(saves_spell(level,vch,DAM_OTHER, ch, DAMF_SPELL) ? buf2 : buf1);
    }

}

VOID_SPELL(Ventriloquate)::utter( Character * ) 
{
}

