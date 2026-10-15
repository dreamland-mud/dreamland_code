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
#include "skillreference.h"

#include "fight.h"
#include "magic.h"
#include "merc.h"

#include "def.h"


GSN(none);
GSN(assist);
GSN(disruption);
GSN(hand_of_undead);
GSN(lightning_bolt);
GSN(sonic_resonance);
GSN(spectral_furor);
GSN(web);

/*-----------------------------------------------------------------------------
 *                    NECROMANCER'S AND UNDEAD'S BRAIN
 *----------------------------------------------------------------------------*/
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

bool BasicMobileBehavior::healNecro( Character *patient )
{
    if (chance( HEALTH(patient) ))
        return false;
        
    return SpellChanceTable( necroSnHealing, ch, patient ).castSpell( ~FSPELL_OBSTACLES );
}

