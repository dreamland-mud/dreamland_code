/* $Id: cleric.cpp,v 1.1.2.4.6.3 2008/05/27 21:30:01 rufina Exp $
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
GSN(blade_barrier);
GSN(bluefire);
GSN(cure_blindness);
GSN(cure_disease);
GSN(cure_poison);
GSN(demonfire);
GSN(dispel_evil);
GSN(dispel_good);
GSN(flamestrike);
GSN(heal);
GSN(master_healing);
GSN(ray_of_truth);
GSN(remove_curse);
GSN(restoring_light);
GSN(superior_heal);

/*----------------------------------------------------------------------------
 *                         CLERICS BRAIN
 *----------------------------------------------------------------------------*/
BasicMobileBehavior::SpellChance BasicMobileBehavior::clericSnHealing [] = {
    { gsn_restoring_light,  50 },
    { gsn_master_healing,  100 },
    { gsn_superior_heal,   100 },
    { gsn_heal,             -1 }
};

BasicMobileBehavior::SpellChance BasicMobileBehavior::clericSnCurative [] = {
    { gsn_cure_blindness, 100 },
    { gsn_cure_poison,     50 },
    { gsn_cure_disease,    25 },
    { gsn_remove_curse,    25 },
    { gsn_none,            -1 }
};

BasicMobileBehavior::SpellChance BasicMobileBehavior::clericSnRange [] = {
    { gsn_ray_of_truth,   50 },
    { gsn_blade_barrier, 100 },
    { gsn_demonfire,     100 },
    { gsn_bluefire,      100 },
    { gsn_dispel_evil,   100 },
    { gsn_dispel_good,   100 },
    { gsn_flamestrike,   100 },
    { gsn_none,           -1 }
};

bool BasicMobileBehavior::healCleric( Character *patient )
{
    SpellChance * spellTable;
    
    if (!chance( HEALTH(patient) ))
        spellTable = clericSnHealing;
    else
        spellTable = clericSnCurative;
    
    return SpellChanceTable( spellTable, ch, patient ).castSpell( ~FSPELL_OBSTACLES);
}
