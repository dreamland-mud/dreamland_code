/* $Id: necromancer.cpp,v 1.1.2.2.6.3 2008/05/27 21:30:01 rufina Exp $
 *
 * ruffina, 2005
 */
/*
 *
 * sturm, 2003
 */
#include "basicmobilebehavior.h"

#include "npcharacter.h"
#include "room.h"
#include "spell.h"

#include "fight.h"
#include "gsn_plugin.h"
#include "magic.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

#ifndef AI_STUB
GSN(assist);
GSN(none);
GSN(dispel_affects);

/*-----------------------------------------------------------------------------
 *                    NECROMANCER'S AND UNDEAD'S BRAIN
 *----------------------------------------------------------------------------*/
BasicMobileBehavior::SpellChance BasicMobileBehavior::necroSnAttack [] = {
    { gsn_web,              50 },
    { gsn_hand_of_undead,   70 },
    { gsn_acid_blast,       50 },
    { gsn_disruption,       60 },
    { gsn_energy_drain,     50 },
    { gsn_spectral_furor,   50 },
    { gsn_sonic_resonance,  50 },
    { gsn_lightning_bolt,   50 },
    { gsn_burning_hands,   100 },
    { gsn_magic_missile,    -1 }
};

BasicMobileBehavior::SpellChance BasicMobileBehavior::necroSnPanicAttack [] = {
    { gsn_hand_of_undead,   40 },
    { gsn_acid_blast,      100 },
    { gsn_web,              40 },
    { gsn_disruption,      100 },
    { gsn_energy_drain,     60 },
    { gsn_spectral_furor,  100 },
    { gsn_sonic_resonance, 100 },
    { gsn_lightning_bolt,  100 },
    { gsn_burning_hands,   100 },
    { gsn_magic_missile,    -1 }
};

BasicMobileBehavior::SpellChance BasicMobileBehavior::necroSnPassiveDefence[] = {
    { gsn_dispel_affects,     30 },
    { gsn_corruption,       50 },
    { gsn_curse,            30 },
    { gsn_plague,           30 },
    { gsn_poison,           30 },
    { gsn_slow,             30 },
    { gsn_weaken,           30 },
    { gsn_chill_touch,      30 },
    { gsn_poison,           -1 }
};
    
BasicMobileBehavior::SpellChance BasicMobileBehavior::necroSnHealing [] = {
    { gsn_assist,          100 },
    { gsn_none,             -1 }
};

BasicMobileBehavior::SpellChance BasicMobileBehavior::necroSnRange [] = {
    { gsn_web,             100 },
    { gsn_hand_of_undead,  100 },
    { gsn_disruption,      100 },
    { gsn_spectral_furor,  100 },
    { gsn_sonic_resonance, 100 },
    { gsn_lightning_bolt,  100 },
    { gsn_none,             -1 }
};

bool BasicMobileBehavior::specFightNecro( )
{
    Character *victim;
    SpellChance * spellTable;
    
    if (!( victim = findCastVictim( ) ))
	return false;
    
    if (HEALTH(victim) < 25 || HEALTH(ch) < 25) 
	spellTable = necroSnPanicAttack;
    else if (HEALTH(ch) <= 70 && HEALTH(ch) >= 25) 
	spellTable = necroSnAttack;
    else if (HEALTH(ch) < 30) {
	spellTable = necroSnHealing;

	if (SpellChanceTable( spellTable, ch, ch ).findSpell( ) > 0)
	    victim = ch;
	else
	    spellTable = necroSnPassiveDefence;
    } 
    else 
	spellTable = necroSnPassiveDefence;
    
    return SpellChanceTable( spellTable, ch, victim ).castSpell( );
}

bool BasicMobileBehavior::healNecro( Character *patient )
{
    if (chance( HEALTH(patient) ))
	return false;
	
    return SpellChanceTable( necroSnHealing, ch, patient ).castSpell( ~FSPELL_OBSTACLES );
}


#endif
