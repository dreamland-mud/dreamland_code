/* $Id: mobskilldata.cpp,v 1.1.2.4.18.2 2008/02/27 01:08:02 rufina Exp $
 *
 * ruffina, 2004
 */
#include "mobskilldata.h"

#include "npcharacter.h"
#include "skill.h"
#include "spell.h"
#include "loadsave.h"
#include "merc.h"
#include "def.h"

/*------------------------------------------------------------------------
 * MobSkillData
 *-----------------------------------------------------------------------*/
MobSkillData::MobSkillData( ) 
                : ordered( true ), 
                  forbidden( false ),
                  offense( 0, &off_flags )
{
}

MobSkillData::~MobSkillData( ) 
{
}

int MobSkillData::visible( NPCharacter *mob, const Skill * ) const
{
    if (IS_CHARMED(mob) && !ordered.getValue( )) 
        return MPROF_NONE;
    
    if (offense.isSet( mob->off_flags ))
        return MPROF_ANY;
    
    if (forbidden.getValue( ))
        return MPROF_NONE;

    return MPROF_REQUIRED;
}

int MobSkillData::getLearned( NPCharacter *mob, const Skill *skill ) const
{
    int level = mob->getRealLevel( );

    if (skill->getSpell( ))
        return 2 * level + 40;
    else
        return dice.getValue( ) * level + bonus.getValue( );
}

/*------------------------------------------------------------------------
 * MobProfSkillData
 *-----------------------------------------------------------------------*/
MobProfSkillData::MobProfSkillData( ) 
                : MobSkillData( ), 
                  professional( true )
{
}

int MobProfSkillData::visible( NPCharacter *mob, const Skill *skill ) const
{
    if (IS_CHARMED(mob) && !ordered.getValue( )) 
        return MPROF_NONE;
    
    if (offense.isSet( mob->off_flags ))
        return MPROF_ANY;
    
    if (forbidden.getValue( ))
        return MPROF_NONE;

    if (professional.getValue( ))
        return MPROF_REQUIRED;

    return MPROF_ANY;
}

