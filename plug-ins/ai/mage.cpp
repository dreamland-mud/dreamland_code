/* $Id: mage.cpp,v 1.1.2.2.6.4 2009/01/17 23:29:38 rufina Exp $
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
GSN(none);
GSN(dispel_affects);
GSN(dragons_breath);

/*----------------------------------------------------------------------------
 *                         MAGE'S BRAIN
 *----------------------------------------------------------------------------*/
BasicMobileBehavior::SpellChance BasicMobileBehavior::mageSnAttack [] = {
    { gsn_acid_blast,      50 },
    { gsn_web,             50 },
    { gsn_dragons_breath,  40 },
    { gsn_fireball,        40 },
    { gsn_acid_arrow,      40 },
    { gsn_scream,          30 },
    { gsn_magnetic_trust,  50 },
    { gsn_energy_drain,    30 },
    { gsn_sulfurus_spray,  40 },
    { gsn_lightning_bolt,  30 },
    { gsn_colour_spray,    40 },
    { gsn_burning_hands,   30 },
    { gsn_shocking_grasp,  40 }, 
    { gsn_magic_missile,   -1 }
};
 
BasicMobileBehavior::SpellChance BasicMobileBehavior::mageSnPanicAttack [] = {
    { gsn_web,             40 },
    { gsn_scream,          40 },
    { gsn_acid_blast,     100 },
    { gsn_dragons_breath, 100 },
    { gsn_fireball,       100 },
    { gsn_acid_arrow,     100 },
    { gsn_magnetic_trust, 100 },
    { gsn_energy_drain,    40 },
    { gsn_sulfurus_spray, 100 },
    { gsn_lightning_bolt, 100 },
    { gsn_colour_spray,   100 },
    { gsn_burning_hands,  100 },
    { gsn_shocking_grasp, 100 }, 
    { gsn_magic_missile,   -1 }
};

BasicMobileBehavior::SpellChance BasicMobileBehavior::mageSnPassiveDefence [] = {
    { gsn_dispel_affects,  40 },
    { gsn_faerie_fire,     40 },
    { gsn_blindness,       30 },
    { gsn_teleport,        30 },
    { gsn_curse,           30 },
    { gsn_plague,          30 },
    { gsn_poison,          30 },
    { gsn_slow,            30 },
    { gsn_weaken,          30 },
    { gsn_chill_touch,     30 },
    { gsn_poison,          -1 }
};

BasicMobileBehavior::SpellChance BasicMobileBehavior::mageSnRange [] = {
    { gsn_web,             95 },
    { gsn_dispel_affects,  50 },
    { gsn_acid_blast,      95 },
    { gsn_dragons_breath, 100 },
    { gsn_fireball,       100 },
    { gsn_acid_arrow,     100 },
    { gsn_magnetic_trust, 100 },
    { gsn_sulfurus_spray, 100 },
    { gsn_none,            -1 }
};

bool BasicMobileBehavior::specFightMage( )
{
    Character *victim;
    SpellChance * spellTable;
    
    if (!( victim = findCastVictim( ) ))
	return false;
    
    if (HEALTH(victim) < 25 || HEALTH(ch) < 25) 
	spellTable = mageSnPanicAttack;
    else if (HEALTH(ch) <= 70 && HEALTH(ch) >= 25) 
	spellTable = mageSnAttack;
    else 
	spellTable = mageSnPassiveDefence;
    
    return SpellChanceTable( spellTable, ch, victim ).castSpell( );
}

#endif
