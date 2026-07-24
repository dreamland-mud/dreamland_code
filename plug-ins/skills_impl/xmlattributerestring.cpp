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
#include "string_utils.h"
#include "act.h"

using namespace Scripting;

const DLString XMLAttributeRestring::DEFAULT_KEYWORD = "default";
const DLString XMLAttributeRestring::TYPE = "XMLAttributeRestring";

void XMLItemRestring::dress( ::Object *obj, PCharacter *ch ) const
{
    if (!name.empty( ))
        obj->setKeyword( name.c_str( ) );

    // A restring is authored in a single language; mirror it into every
    // language slot so EN/UA viewers see the restring instead of falling back
    // to the untouched prototype name. Otherwise the instance's per-language
    // short/long/desc stay empty and firstNonEmpty/toNoun surface the
    // prototype for non-RU viewers, making the restring invisible to them.
    // Author-language canonical, displayed as-is (no machine translation).
    if (!shortDescr.empty( )) {
        DLString s = fmt( 0, shortDescr.c_str( ), ch );
        for (int l = LANG_MIN; l < LANG_MAX; l++)
            obj->setShortDescr( s, (lang_t)l );
    }

    if (!longDescr.empty( )) {
        DLString s = fmt( 0, longDescr.c_str( ), ch );
        for (int l = LANG_MIN; l < LANG_MAX; l++)
            obj->setDescription( s, (lang_t)l );
    }

    if (!description.empty( )) {
        DLString s = fmt( 0, description.c_str( ), ch );
        ExtraDescription *ed = obj->addProperDescription( );
        for (int l = LANG_MIN; l < LANG_MAX; l++)
            ed->description[(lang_t)l] = s;
    }
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
