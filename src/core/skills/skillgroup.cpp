/* $Id$
 *
 * ruffina, 2004
 */
#include "flags.h"
#include "globalarray.h"
#include "skillgroup.h"
#include "character.h"

/*-------------------------------------------------------------------
 * SkillGroup
 *------------------------------------------------------------------*/
SkillGroup::SkillGroup( )
{
}

SkillGroup::SkillGroup( const DLString &name ) : elementName( name )
{
}

SkillGroup::~SkillGroup( )
{
}

const DLString &SkillGroup::getName( ) const
{
    return elementName;
}

bool SkillGroup::isValid( ) const
{
    return false;
}

const DLString &SkillGroup::getRussianName( ) const
{
    return DLString::emptyString;
}

bool SkillGroup::visible( Character * ) const
{
    return true;
}

bool SkillGroup::available( Character * ) const
{
    return true;
}

void SkillGroup::show( PCharacter *, ostringstream & ) const
{
}

int SkillGroup::getPracticer( ) const
{
    return 0;
}

const DLString& SkillGroup::getNameFor( Character *ch ) const
{
    return getName( );
}

/*-------------------------------------------------------------------
 * SkillGroupManager
 *------------------------------------------------------------------*/
SkillGroupManager* skillGroupManager = 0;

SkillGroupManager::SkillGroupManager( ) 
{
    checkDuplicate( skillGroupManager );
    skillGroupManager = this;
    setRegistryName("skillGroup");
    saveRegistryName();
}

SkillGroupManager::~SkillGroupManager( )
{
    eraseRegistryName();
    skillGroupManager = 0;
}

GlobalRegistryElement::Pointer SkillGroupManager::getDumbElement( const DLString &name ) const
{
    return SkillGroup::Pointer( NEW, name );
}

/*-------------------------------------------------------------------
 * SkillGroupAction
 *------------------------------------------------------------------*/
SkillGroupAction::~SkillGroupAction( )
{
}



GLOBALREF_IMPL(SkillGroup, ' ')
XMLGLOBALREF_IMPL(SkillGroup)

