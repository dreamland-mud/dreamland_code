#ifndef CHARGEDPROPS_H
#define CHARGEDPROPS_H

/*
 * Charged items: any item can carry props["charged"] =
 *   {spell, level, charges, max, where: "carried"|"wielded", crumble: bool}
 * and cast it through 'use' the way a wand does. The instance's copy overrides
 * the prototype's. A handmade item starts from its prototype's copy, which is
 * copied onto the instance the first time charges change, so charges are per
 * instance and saved with the instance props.
 */
#include <jsoncpp/json/json.h>
#include "object.h"
#include "objectfactory.h"

// isMember before operator[]: a non-const read would insert a null member.
inline const Json::Value * charged_props( Object *obj )
{
    if (obj->props.isMember( "charged" )) {
        const Json::Value &c = obj->props["charged"];
        return c.isObject( ) ? &c : 0;
    }

    if (obj->pIndexData && obj->pIndexData->props.isMember( "charged" )) {
        const Json::Value &c = obj->pIndexData->props["charged"];
        return c.isObject( ) ? &c : 0;
    }

    return 0;
}

// The instance's own copy, for writing. Only call when charged_props() is not null.
inline Json::Value & charged_own( Object *obj )
{
    if (!obj->props.isMember( "charged" ))
        obj->props["charged"] = obj->pIndexData->props["charged"];

    return obj->props["charged"];
}

inline bool charged_wielded( const Json::Value &c )
{
    return c.isMember( "where" ) && c["where"].asString( ) == "wielded";
}

inline int charged_charges( const Json::Value &c )
{
    return c.isMember( "charges" ) ? c["charges"].asInt( ) : 0;
}

#endif
