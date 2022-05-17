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
#include "mercdb.h"
#include "def.h"

GSN(none);
GSN(blade_barrier);
GSN(blindness);
GSN(bluefire);
GSN(cure_blindness);
GSN(cure_disease);
GSN(cure_poison);
GSN(curse);
GSN(demonfire);
GSN(dispel_affects);
GSN(dispel_evil);
GSN(dispel_good);
GSN(earthquake);
GSN(energy_drain);
GSN(faerie_fire);
GSN(flamestrike);
GSN(harm);
GSN(heal);
GSN(heat_metal);
GSN(holy_word);
GSN(master_healing);
GSN(plague);
GSN(poison);
GSN(ray_of_truth);
GSN(remove_curse);
GSN(restoring_light);
GSN(severity_force);
GSN(slow);
GSN(superior_heal);
GSN(teleport);
GSN(weaken);

/*----------------------------------------------------------------------------
 *                         CLERICS BRAIN
 *----------------------------------------------------------------------------*/
BasicMobileBehavior::SpellChance BasicMobileBehavior::clericSnHealing [] = {
    { gsn_restoring_light,  50 },
    { gsn_master_healing,  100 },
    { gsn_superior_heal,   100 },
    { gsn_heal,             -1 }
};

BasicMobileBehavior::SpellChance BasicMobileBehavior::clericSnAttack [] = {
    { gsn_ray_of_truth,    70 },
    { gsn_holy_word,       70 },
    { gsn_blade_barrier,   70 },
    { gsn_severity_force,  70 },
    { gsn_flamestrike,     30 },
    { gsn_energy_drain,    30 },
    { gsn_demonfire,       30 },
    { gsn_bluefire,        30 },
    { gsn_dispel_evil,     30 },
    { gsn_dispel_good,     30 },
    { gsn_heat_metal,      50 },
    { gsn_earthquake,      50 },
    { gsn_harm,           100 },
    { gsn_harm,            -1 }
};    

BasicMobileBehavior::SpellChance BasicMobileBehavior::clericSnPanicAttack [] = {
    { gsn_ray_of_truth,    50 },
    { gsn_blade_barrier,   50 },
    { gsn_severity_force, 100 },
    { gsn_demonfire,      100 },
    { gsn_bluefire,       100 },
    { gsn_dispel_evil,    100 },
    { gsn_dispel_good,    100 },
    { gsn_flamestrike,    100 },
    { gsn_harm,           100 },
    { gsn_harm,            -1 }
};
        
BasicMobileBehavior::SpellChance BasicMobileBehavior::clericSnPassiveDefence [] = {
    { gsn_dispel_affects,  90 },
    { gsn_faerie_fire,   50 },
    { gsn_blindness,     65 },
    { gsn_teleport,      40 },
    { gsn_curse,         40 },
    { gsn_plague,        40 },
    { gsn_poison,        40 },
    { gsn_slow,          50 },
    { gsn_weaken,        50 },
    { gsn_poison,        -1 },
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
    
bool BasicMobileBehavior::specFightCleric( )
{
    Character *victim;
    SpellChance * spellTable;
    
    if (!( victim = findCastVictim( ) ))
        return false;
    
    if (HEALTH(victim) < 25 || HEALTH(ch) < 25) 
        spellTable = clericSnPanicAttack;
    else if (HEALTH(ch) <= 80 && HEALTH(ch) > 45) 
        spellTable = clericSnAttack;
    else if (HEALTH(ch) <= 45) {
        spellTable = clericSnHealing;
        victim = ch;
    }
    else 
        spellTable = clericSnPassiveDefence;
    
    return SpellChanceTable( spellTable, ch, victim ).castSpell( );
}

bool BasicMobileBehavior::healCleric( Character *patient )
{
    SpellChance * spellTable;
    
    if (!chance( HEALTH(patient) ))
        spellTable = clericSnHealing;
    else
        spellTable = clericSnCurative;
    
    return SpellChanceTable( spellTable, ch, patient ).castSpell( ~FSPELL_OBSTACLES);
}
