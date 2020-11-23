/* $Id$
 *
 * ruffina, 2009
 */
#include "ahenn_effects.h"
#include "language.h"
#include "languagemanager.h"

#include "skillreference.h"

#include "pcharacter.h"
#include "affect.h"

#include "magic.h"
#include "fight.h"
#include "act.h"
#include "loadsave.h"
#include "mercdb.h"
#include "merc.h"
#include "def.h"

GSN(inspiration);
GSN(curse);
GSN(poison);
GSN(plague);
GSN(blindness);
GSN(corruption);
GSN(weaken);     
GSN(slow);       

bool BadSpellWE::run( PCharacter *ch, Character *victim ) const
{
    static const int spells [] = {
        gsn_curse,
        gsn_poison,
        gsn_plague,
        gsn_blindness,
        gsn_slow,
        gsn_weaken,
        gsn_corruption,
    };
    static const int spells_size = sizeof( spells ) / sizeof( *spells );

    int i;
    
    act( "{CСила древних проклятий проникает в мир.{x", ch, 0, 0, TO_ALL );

    if (is_safe( ch, victim ))
        return true;
    
    set_violent( ch, victim, false );

    for (i = 0; i < spells_size; i++)
        spell( spells[i], 
               number_range( ch->getModifyLevel( ) + 5, 120 ),
               ch, victim, FSPELL_BANE );

    return true;
}

bool InspirationWE::run( PCharacter *ch, Character *victim ) const
{
    Affect af;

    af.type     = gsn_inspiration;
    af.level    = ch->getModifyLevel( );
    af.duration = af.level / 3;
    
    affect_join( victim, &af );

    act( "{CЖажда творить и совершенствовать переполняет тебя.{x", victim, 0, 0, TO_CHAR );
    act( "{C$c1 выглядит просветленн$gым|ым|ой.{x", victim, 0, 0, TO_ROOM );
    return true;
}


