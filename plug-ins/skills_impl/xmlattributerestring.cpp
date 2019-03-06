/* $Id$
 *
 * ruffina, 2004
 */
#include "xmlattributerestring.h"
#include "skill.h"
#include "skillmanager.h"
#include "object.h"
#include "pcharacter.h"
#include "act.h"

const DLString XMLAttributeRestring::DEFAULT_KEYWORD = "default";
const DLString XMLAttributeRestring::TYPE = "XMLAttributeRestring";

void XMLItemRestring::dress( Object *obj, PCharacter *ch ) const
{
    if (!name.empty( ))
        obj->setName( name.c_str( ) );

    if (!shortDescr.empty( ))
        obj->setShortDescr( fmt( 0, shortDescr.c_str( ), ch ).c_str( ) );

    if (!longDescr.empty( ))
        obj->setDescription( fmt( 0, longDescr.c_str( ), ch ).c_str( ) );

    if (!description.empty( ))
        obj->addExtraDescr( obj->getName( ), 
                            fmt( 0, description.c_str( ), ch ) );
}

void XMLAttributeRestring::dress( Object *obj, PCharacter *ch, const DLString &keyword ) const
{
    const_iterator i = find( keyword.empty( ) ? DEFAULT_KEYWORD : keyword );

    if (i == end( ))
        return;

    i->second.dress( obj, ch );
}

void dress_created_item( int sn, Object *obj, Character *ch, const DLString &keyword )
{
    if (ch->is_npc( ))
        return;
        
    const DLString &attrName = skillManager->find( sn )->getName( );
    XMLAttributeRestring::Pointer attr = 
            ch->getPC( )->getAttributes( ).findAttr<XMLAttributeRestring>( attrName );

    if (!attr)
        return;

    attr->dress( obj, ch->getPC( ), keyword );
}

