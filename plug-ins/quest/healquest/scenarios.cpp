/* $Id$
 *
 * ruffina, 2004
 */
#include "scenarios.h"

#include "clanreference.h"
#include "affecthandler.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "affect.h"

#include "loadsave.h"
#include "merc.h"
#include "def.h"

GSN(observation);
GSN(cancellation);
GSN(dispel_affects);
CLAN(battlerager);

/*--------------------------------------------------------------------
 * HealScenario
 *--------------------------------------------------------------------*/
HealScenario::HealScenario( )
                 : obvious( false ),
                   immune( 0, &imm_flags ),
                   profession( 0, &act_flags )
{
}

bool HealScenario::applicable( PCharacter *pch )
{
    if (pch->getClan( ) == clan_battlerager)
        return false;

    if (!obvious && gsn_observation->getEffective( pch ) < 50)
        return false;
    
    for (XMLStringVector::iterator s = remedies.begin( ); s != remedies.end( ); s++)
        if (skillManager->find( *s )->getEffective( pch ) >= 50) 
            return true;

    return false;
}


bool HealScenario::healedBy( int sn )
{
    if (remedies.hasElement( skillManager->find( sn )->getName( ) ))
        return true;
    
    if (!malady->getAffect( ))
        return false;

    if (sn == gsn_cancellation && malady->getAffect( )->isCancelled( ))
        return true;

    if (sn == gsn_dispel_affects && malady->getAffect( )->isDispelled( ))
        return true;

    return false;
}

bool HealScenario::isInfected( NPCharacter *mob )
{
    return mob->isAffected( malady );
}

bool HealScenario::applicable( PCharacter *pch, NPCharacter *mob )
{
    if (IS_SET(mob->act, profession)) 
        return false;

    if (bit.getTable( ) == &affect_flags && IS_SET(mob->affected_by, bit)) 
        return false;

    if (bit.getTable( ) == &detect_flags && IS_SET(mob->detection, bit)) 
        return false;
    
    if (IS_SET(mob->imm_flags, immune)) 
        return false;
    
    return true; 
}


void HealScenario::infect( NPCharacter *patient, int time, int level ) 
{
    Affect af;
    
    if (bit.getTable( ) == &detect_flags)
        af.where = TO_DETECTS;
    else
        af.where = TO_AFFECTS;

    af.type      = malady;
    af.level     = level;
    af.duration  = time;
    af.bitvector = bit;

    affect_to_char( patient, &af );
}

