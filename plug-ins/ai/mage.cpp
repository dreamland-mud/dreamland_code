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
#include "skillreference.h"

#include "fight.h"
#include "magic.h"
#include "merc.h"

#include "def.h"


GSN(none);
GSN(acid_arrow);
GSN(acid_blast);
GSN(dispel_affects);
GSN(dragons_breath);
GSN(fireball);
GSN(magnetic_thrust);
GSN(sulfuric_spray);
GSN(web);

/*----------------------------------------------------------------------------
 *                         MAGE'S BRAIN
 *----------------------------------------------------------------------------*/
BasicMobileBehavior::SpellChance BasicMobileBehavior::mageSnRange [] = {
    { gsn_web,             95 },
    { gsn_dispel_affects,  50 },
    { gsn_acid_blast,      95 },
    { gsn_dragons_breath, 100 },
    { gsn_fireball,       100 },
    { gsn_acid_arrow,     100 },
    { gsn_magnetic_thrust, 100 },
    { gsn_sulfuric_spray, 100 },
    { gsn_none,            -1 }
};

