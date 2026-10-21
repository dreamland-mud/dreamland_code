/* $Id: mobskilldata.cpp,v 1.1.2.4.18.2 2008/02/27 01:08:02 rufina Exp $
 *
 * ruffina, 2004
 */
#include "logstream.h"
#include "mobskilldata.h"

#include "npcharacter.h"
#include "pcharacter.h"
#include "skill.h"
#include "spell.h"
#include "damageflags.h"
#include "loadsave.h"
#include "merc.h"
#include "def.h"

/*
 * Charmed mobs list and take orders for beneficial spells only, so 'report'
 * never offers an attack. A player's own pet in a fight is the exception:
 * the tier AI casts its attacks at the foe it already fights.
 */
static bool charm_hides_spell( NPCharacter *mob, const Skill *skill )
{
    if (!IS_CHARMED(mob) || !skill->getSpell() || skill->getSpell()->getSpellType() != SPELL_OFFENSIVE)
        return false;

    bool ownPet = !mob->master->is_npc( ) && mob->master->getPC( )->pet == mob;
    return !(ownPet && mob->fighting);
}

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

int MobSkillData::visible( NPCharacter *mob, const Skill *skill ) const
{
    if (!available)
        return MPROF_NONE;

    if (charm_hides_spell( mob, skill ))
        return MPROF_NONE;

    if (offense.isSet( mob->off_flags ))
        return MPROF_ANY;

    return MPROF_REQUIRED;
}

int MobSkillData::getLearned( NPCharacter *mob, const Skill *skill ) const
{
    int level = mob->getRealLevel( );

    // Spells historically were 2*level+40 by default, but can be overridden
    if (skill->getSpell( ) && dice == 0 && bonus == 0)
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

    if (charm_hides_spell( mob, skill ))
        return MPROF_NONE;

    if (offense.isSet( mob->off_flags ))
        return MPROF_ANY;
    
    if (professional.getValue( ))
        return MPROF_REQUIRED;

    return MPROF_ANY;
}

