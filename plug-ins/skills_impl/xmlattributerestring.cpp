/* $Id$
 *
 * ruffina, 2004
 */
#include "xmlattributerestring.h"
#include "register-impl.h"
#include "idcontainer.h"
#include "regcontainer.h"
#include "skill.h"
#include "skillmanager.h"
#include "core/object.h"
#include "pcharacter.h"
#include "act.h"

using namespace Scripting;

const DLString XMLAttributeRestring::DEFAULT_KEYWORD = "default";
const DLString XMLAttributeRestring::TYPE = "XMLAttributeRestring";

void XMLItemRestring::dress( ::Object *obj, PCharacter *ch ) const
{
    if (!name.empty( ))
        obj->setKeyword( name.c_str( ) );

    if (!shortDescr.empty( ))
        obj->setShortDescr( fmt( 0, shortDescr.c_str( ), ch ), LANG_DEFAULT);

    if (!longDescr.empty( ))
        obj->setDescription( fmt( 0, longDescr.c_str( ), ch ), LANG_DEFAULT );

    if (!description.empty( ))
        obj->addExtraDescr( obj->getKeyword( ).toString(), 
                            fmt( 0, description.c_str( ), ch ), LANG_DEFAULT );
}

Scripting::Register XMLAttributeRestring::toRegister() const
{
    Register result = Register::handler<RegContainer>();
    RegContainer *map = result.toHandler().getDynamicPointer<RegContainer>();
    for (auto &r: *this) {
        map->setField(r.first, r.second.toRegister());
    }

    return result;
}

void XMLAttributeRestring::dress( ::Object *obj, PCharacter *ch, const DLString &keyword ) const
{
    const_iterator i = find( keyword.empty( ) ? DEFAULT_KEYWORD : keyword );

    if (i == end( ))
        return;

    i->second.dress( obj, ch );
}

void dress_created_item( int sn, ::Object *obj, Character *ch, const DLString &keyword )
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


Scripting::Register XMLItemRestring::toRegister() const
{
    Register itemReg = Register::handler<IdContainer>();
    IdContainer *item = itemReg.toHandler().getDynamicPointer<IdContainer>();
    item->setField(IdRef("name"), name);
    item->setField(IdRef("shortDescr"), shortDescr);
    item->setField(IdRef("longDescr"), longDescr);
    item->setField(IdRef("description"), description);
    return itemReg;    
}
