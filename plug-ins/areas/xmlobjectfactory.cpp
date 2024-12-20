/* $Id$
 *
 * ruffina, 2004
 */

#include "logstream.h"
#include "xmlobjectfactory.h"
#include "affect.h"
#include "behavior.h"
#include "grammar_entities_impl.h"
#include "merc.h"
#include "json_utils_ext.h"
#include "string_utils.h"

DLString format_longdescr(const DLString &longdescr);

XMLObjectFactory::XMLObjectFactory( ) : 
    extra_flags(0, &::extra_flags), wear_flags(0, &::wear_flags), 
    behaviors(behaviorManager)
{
}

void
XMLObjectFactory::init(const obj_index_data *obj)
{
    keyword = obj->keyword;
    short_descr = obj->short_descr;
    description = obj->description;
    sound = obj->sound;
    smell = obj->smell;

    material.setValue(obj->material);
    type.type = obj->item_type;
    copy(obj->value, obj->value+5, type.v);
    level.setValue(obj->level);
    weight.setValue(obj->weight);
    cost.setValue(obj->cost);
    condition.setValue(obj->condition);
    extra_flags.setValue(obj->extra_flags);
    wear_flags.setValue(obj->wear_flags);

    for (auto &pAf: obj->affected) {
        XMLAffect aff;
        aff.init(pAf);
        affects.push_back(aff);
    }

    for (auto &ed: obj->extraDescriptions) {
        extraDescriptions.push_back(XMLExtraDescription());
        extraDescriptions.back().keyword = ed->keyword;
        extraDescriptions.back().description = ed->description;
    }

    gender.setValue(obj->gram_gender.toString());
    limit.setValue(obj->limit);


    if(!obj->behavior.isEmpty( ))
        behavior.setNode(obj->behavior->getFirstNode( ));

    behaviors.set(obj->behaviors);

    JsonUtils::copy(props, obj->props);    
}

obj_index_data *
XMLObjectFactory::compat( )
{
    OBJ_INDEX_DATA *obj = new OBJ_INDEX_DATA;
    
    compat(obj);

    return obj;
}

void
XMLObjectFactory::compat(obj_index_data *obj)
{
    if (!name.getValue().empty())
        obj->keyword.fromMixedString(name);
    else
        obj->keyword = keyword;

    obj->short_descr = short_descr;
    obj->description = description;
    obj->description[RU] = format_longdescr(description[RU]);

    obj->material = material;
    obj->item_type = type.type;
    copy(type.v, type.v + 5, obj->value);
    obj->level = level.getValue( );
    obj->weight = weight.getValue( );
    obj->cost = cost.getValue( );
    obj->condition = condition.getValue( );
    obj->extra_flags = extra_flags.getValue( );
    obj->wear_flags = wear_flags.getValue( );

    XMLListBase<XMLAffect>::reverse_iterator ait;
    for(ait = affects.rbegin( ); ait != affects.rend( ); ait++) {
        Affect *pAf = ait->compat( );
        pAf->level = obj->level;
        obj->affected.push_front(pAf);
    }

    XMLListBase<XMLExtraDescr>::reverse_iterator eit;
    for(eit = extraDescr.rbegin( ); eit != extraDescr.rend( ); eit++) {
        ExtraDescription *ed = new ExtraDescription();
        ed->keyword = eit->keyword;

        if (String::hasCyrillic(eit->getValue()))
            ed->description[RU] = eit->getValue();
        else
            // some weird not yet translated extras
            ed->description[EN] = eit->getValue();

        obj->extraDescriptions.push_back(ed);
    }

    for (auto &ed: extraDescriptions) {
        ExtraDescription *newed = new ExtraDescription();
        newed->keyword = ed.keyword;
        newed->description = ed.description;
        obj->extraDescriptions.push_back(newed);
    }

    obj->gram_gender = Grammar::MultiGender(gender.getValue( ).c_str( ));
    obj->limit = limit.getValue( );

    obj->sound = sound;
    obj->smell = smell;

    if(behavior.getNode( )) {
        obj->behavior.construct( );
        XMLNode::Pointer p = behavior.getNode( );
        obj->behavior->appendChild(p);
    }

    obj->behaviors.set(behaviors);

    JsonUtils::copy(obj->props, props);
}

