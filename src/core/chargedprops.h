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
#include <stdlib.h>
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
    return c.isMember( "where" ) && c["where"].isString( ) && c["where"].asString( ) == "wielded";
}

// Props set by hand (OLC, setProperty) arrive as strings; asInt() on a
// non-numeric string throws, so read numbers and flags leniently.
inline int charged_int( const Json::Value &c, const char *key, int def )
{
    if (!c.isMember( key ))
        return def;

    const Json::Value &v = c[key];
    if (v.isNumeric( ) || v.isBool( ))
        return v.asInt( );
    if (v.isString( ))
        return atoi( v.asCString( ) );
    return def;
}

inline bool charged_flag( const Json::Value &c, const char *key )
{
    if (!c.isMember( key ))
        return false;

    const Json::Value &v = c[key];
    if (v.isString( ))
        return v.asString( ) == "true" || atoi( v.asCString( ) ) != 0;
    return (v.isNumeric( ) || v.isBool( )) && v.asInt( ) != 0;
}

inline int charged_charges( const Json::Value &c )
{
    return charged_int( c, "charges", 0 );
}

#endif
