/* $Id$
 *
 * ruffina, 2009
 */
#include "quenia_effects.h"
#include "language.h"
#include "languagemanager.h"

#include "skillreference.h"

#include "pcharacter.h"
#include "affect.h"

#include "magic.h"
#include "act.h"
#include "loadsave.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

GSN(accuracy);
GSN(sanctuary);         
GSN(haste);             
GSN(giant_strength);    
GSN(shield);            
GSN(bless);             
GSN(stone_skin);        
GSN(frenzy);            


bool GoodSpellWE::run( PCharacter *ch, Character *victim ) const
{
    static const int spells [] = {
	gsn_sanctuary,
	gsn_haste,
	gsn_giant_strength,
	gsn_shield,
	gsn_bless,
	gsn_stone_skin,
	gsn_frenzy,
    };
    static const int spells_size = sizeof( spells ) / sizeof( *spells );

    int i;

    act( "{CСила древнего благословления проникает в мир.{x", ch, 0, 0, TO_ALL );

    for (i = 0; i < spells_size; i++)
	spell( spells[i], 
	       number_range( ch->getModifyLevel( ) + 5, 120 ),
	       ch, victim, FSPELL_BANE );

    return true;
}


bool AccuracyWE::run( PCharacter *ch, Character *victim ) const
{
    Affect af;

    af.where    = TO_AFFECTS;
    af.type     = gsn_accuracy;
    af.level    = ch->getModifyLevel( );
    af.duration = af.level / 3;
    
    affect_join( victim, &af );

    act( "{CТеперь твой взгляд способен различить каждое перышко жаворонка в небе.{x", victim, 0, 0, TO_CHAR );
    act( "{CВ глазах $c2 стальным блеском вспыхивает наконечник стрелы.{x", victim, 0, 0, TO_ROOM );
    return true;
}

