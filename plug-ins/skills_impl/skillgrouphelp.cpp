/* $Id$
 *
 * ruffina, 2004
 */
#include "skill.h"
#include "skillmanager.h"
#include "skillgrouphelp.h"
#include "character.h"

/*-------------------------------------------------------------------
 * SkillGroupHelp 
 *------------------------------------------------------------------*/
const DLString SkillGroupHelp::TYPE = "SkillGroupHelp";

void SkillGroupHelp::getRawText( Character *ch, ostringstream &buf ) const
{
    group->show( ch->getPC( ), buf );
}

void SkillGroupHelp::setSkillGroup( SkillGroup::Pointer group )
{
    StringSet kwd;

    this->group = group;
    
    kwd.insert( group->getName( ) );    
    kwd.insert( group->getRussianName( ) );    
    
    if (!keyword.empty( ))
	kwd.fromString( keyword );
    
    fullKeyword = kwd.toString( );
    fullKeyword.toUpper( );
    helpManager->registrate( Pointer( this ) );
}

void SkillGroupHelp::unsetSkillGroup( )
{
    helpManager->unregistrate( Pointer( this ) );
    group.clear( );
    fullKeyword = "";
}

