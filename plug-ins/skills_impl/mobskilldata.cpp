/* $Id: mobskilldata.cpp,v 1.1.2.4.18.2 2008/02/27 01:08:02 rufina Exp $
 *
 * ruffina, 2004
 */
#include "logstream.h"
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
                : offense( 0, &off_flags ),
                  available( false )
{
}

MobSkillData::~MobSkillData( ) 
{
}

bool MobSkillData::toXML(XMLNode::Pointer &parent) const
{
    if (!available)
        return false;

    return XMLVariableContainer::toXML(parent);
}

void MobSkillData::fromXML(const XMLNode::Pointer &parent) 
{
    XMLVariableContainer::fromXML(parent);
    available = true;
}

int MobSkillData::visible( NPCharacter *mob, const Skill * ) const
{
    if (!available)
        return MPROF_NONE;

    if (offense.isSet( mob->off_flags ))
        return MPROF_ANY;
    
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
    if (!available)
        return MPROF_NONE;

    if (offense.isSet( mob->off_flags ))
        return MPROF_ANY;
    
    if (professional.getValue( ))
        return MPROF_REQUIRED;

    return MPROF_ANY;
}

