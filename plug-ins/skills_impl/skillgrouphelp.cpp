/* $Id$
 *
 * ruffina, 2004
 */
#include "skill.h"
#include "skillmanager.h"
#include "skillgrouphelp.h"
#include "character.h"
#include "xmltableelement.h"

/*-------------------------------------------------------------------
 * SkillGroupHelp 
 *------------------------------------------------------------------*/
const DLString SkillGroupHelp::TYPE = "SkillGroupHelp";

void SkillGroupHelp::save() const
{
    if (group) {
        const XMLTableElement *element = group.getDynamicPointer<XMLTableElement>();
        if (element)
            element->save();
    }
}

void SkillGroupHelp::getRawText( Character *ch, ostringstream &buf ) const
{
    buf << "%PAUSE%";
    group->show( ch->getPC( ), buf );
}

void SkillGroupHelp::setSkillGroup( SkillGroup::Pointer group )
{
    this->group = group;
    
    addAutoKeyword( group->getName( ) );    
    addAutoKeyword( group->getRussianName( ) );    
    helpManager->registrate( Pointer( this ) );
}

void SkillGroupHelp::unsetSkillGroup( )
{
    helpManager->unregistrate( Pointer( this ) );
    group.clear( );
    keywordsAuto.clear();
    refreshKeywords();
    labels.transient.clear();
    labels.refresh();
}

