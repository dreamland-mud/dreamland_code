/* $Id$
 *
 * ruffina, 2004
 */
#include "scenarios.h"

#include "clanreference.h"
#include "skill_utils.h"
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

bool HealScenario::applicable( PCharacter *pch ) const
{
    if (pch->getClan( ) == clan_battlerager)
        return false;

    if (!obvious && gsn_observation->getEffective( pch ) < 50)
        return false;
    
    for (XMLStringVector::const_iterator s = remedies.begin( ); s != remedies.end( ); s++)
        if (skillManager->find( *s )->getEffective( pch ) >= 50) 
            return true;

    return false;
}


bool HealScenario::healedBy( int sn ) const
{
    if (remedies.hasElement( skillManager->find( sn )->getName( ) ))
        return true;
   
    Skill *skill = skillref_to_pointer(malady);
    if (!skill->getAffect( ))
        return false;

    if (sn == gsn_cancellation && skill->getAffect( )->isCancelled( ))
        return true;

    if (sn == gsn_dispel_affects && skill->getAffect( )->isDispelled( ))
        return true;

    return false;
}

bool HealScenario::isInfected( NPCharacter *mob ) const
{
    Skill *skill = skillref_to_pointer(malady);
    return mob->isAffected( skill->getIndex() );
}

bool HealScenario::applicable( PCharacter *pch, NPCharacter *mob ) const
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


void HealScenario::infect( NPCharacter *patient, int time, int level )  const
{
    Affect af;
    
    if (bit.getTable( ) == &detect_flags)
        af.bitvector.setTable(&detect_flags);
    else
        af.bitvector.setTable(&affect_flags);

    af.type      = malady;
    af.level     = level;
    af.duration  = time;
    af.bitvector.setValue(bit);

    affect_to_char( patient, &af );
}

