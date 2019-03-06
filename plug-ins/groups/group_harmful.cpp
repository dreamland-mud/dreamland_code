
/* $Id: group_harmful.cpp,v 1.1.2.7.6.3 2008/04/14 20:12:36 rufina Exp $
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


#include "so.h"

#include "spelltemplate.h"

#include "pcharacter.h"
#include "room.h"
#include "magic.h"
#include "fight.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"


SPELL_DECL(CauseLight);
VOID_SPELL(CauseLight)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    damage( ch, victim, dice(4, 9) + level / 3, sn,DAM_HARM,true, DAMF_SPELL);
}

SPELL_DECL(CauseSerious);
VOID_SPELL(CauseSerious)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    damage( ch, victim, dice(5, 9) + level / 2, sn,DAM_HARM,true, DAMF_SPELL);
}

SPELL_DECL(CauseCritical);
VOID_SPELL(CauseCritical)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    damage( ch, victim, dice(6, 9) + level - 6, sn,DAM_HARM,true, DAMF_SPELL);
}

SPELL_DECL(Harm);
VOID_SPELL(Harm)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    int dam;

    dam = max(  20, victim->hit / dice( 1, 4 ) );
    if ( saves_spell( level, victim,DAM_HARM,ch, DAMF_SPELL) )
     dam = dam / 2;
    dam = min( 200, dam );
    damage( ch, victim, dam, sn, DAM_HARM ,true, DAMF_SPELL);
}


