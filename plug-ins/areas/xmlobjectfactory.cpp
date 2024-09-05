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
#include "json_utils.h"


XMLObjectFactory::XMLObjectFactory( ) : 
    extra_flags(0, &::extra_flags), wear_flags(0, &::wear_flags), 
    behaviors(behaviorManager),
    properties( false )
{
}

void
XMLObjectFactory::init(const obj_index_data *obj)
{
    name.setValue(obj->name);
    short_descr.setValue(obj->short_descr);
    description.setValue(obj->description);
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

    EXTRA_DESCR_DATA *pEd;
    for (pEd = obj->extra_descr; pEd; pEd = pEd->next) {
        extraDescr.push_back(XMLExtraDescr( ));
        extraDescr.back( ).keyword = pEd->keyword;
        extraDescr.back( ).setValue(pEd->description);
    }

    gender.setValue(obj->gram_gender.toString());
    limit.setValue(obj->limit);
    sound.setValue(obj->sound);    
    smell.setValue(obj->smell);

    if(!obj->behavior.isEmpty( ))
        behavior.setNode(obj->behavior->getFirstNode( ));

    behaviors.set(obj->behaviors);

    for (Properties::const_iterator p = obj->properties.begin( ); p != obj->properties.end( ); p++)
        properties.insert( *p );

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
    obj->name = str_dup(name.getValue( ).c_str( ));
    obj->short_descr = str_dup(short_descr.getValue( ).c_str( ));
    obj->description = str_dup(description.getValue( ).c_str( ));
    obj->material = str_dup(material.getValue( ).c_str( ));
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
        EXTRA_DESCR_DATA *ed = new_extra_descr();
        ed->keyword = str_dup(eit->keyword.c_str( ));
        ed->description = str_dup(eit->getValue( ).c_str( ));
        ed->next = obj->extra_descr;
        obj->extra_descr = ed;
    }

    obj->gram_gender = Grammar::MultiGender(gender.getValue( ).c_str( ));
    obj->limit = limit.getValue( );

    obj->sound = sound.getValue( );
    obj->smell = smell.getValue( );

    if(behavior.getNode( )) {
        obj->behavior.construct( );
        XMLNode::Pointer p = behavior.getNode( );
        obj->behavior->appendChild(p);
    }

    obj->behaviors.set(behaviors);

    for (XMLMapBase<XMLString>::iterator p = properties.begin( ); p != properties.end( ); p++)
        obj->properties.insert( *p );

    JsonUtils::copy(obj->props, props);
}

