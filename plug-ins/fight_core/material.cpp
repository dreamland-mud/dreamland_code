/* $Id: material.cpp,v 1.1.2.6 2009/01/18 20:12:41 rufina Exp $
 *
 * ruffina, 2005
 */
#include "object.h"
#include "character.h"
#include "mercdb.h"
#include "merc.h"

#include "immunity.h"

#include <jsoncpp/json/json.h>
#include "configurable.h"
#include "damageflags.h"
#include "material.h"

struct material_t {
    DLString name;
    int burns;
    int floats;
    json_flag<&material_types> type;
    json_flag<&material_flags> flags;
    json_flag<&imm_flags> vuln;
    DLString rname;

    void fromJson(const Json::Value &value) 
    {
        name = value["name"].asString();
        burns = value["burns"].asInt();
        floats = value["floats"].asInt();
        type.fromJson(value["type"]);
        flags.fromJson(value["flags"]);
        vuln.fromJson(value["vuln"]);
        rname = value["rname"].asString();
    }
};

json_vector<material_t> material_table;
CONFIGURABLE_LOADED(fight, material)
{
    material_table.fromJson(value);
}


static void 
material_parse( Object *obj,  vector<material_t *> &result )
{    
    if (obj->getMaterial( )) {
        char *token, name[MAX_STRING_LENGTH];

        strcpy( name, obj->getMaterial( ) );
        
        token = strtok( name, ", " );
        
        for(token=strtok(name, ", "); token; token=strtok(NULL, ", "))
            if (*token != '\0') {
                for (auto &mat: material_table)
                    if (!str_cmp( token, mat.name.c_str() )) {
                        result.push_back(&mat);
                        break;
                    }                
            }
    }
}

bool material_is_typed( Object *obj, int type )
{
    vector<material_t *> result;

    material_parse( obj, result );
    
    for (auto &mat: result)
        if (IS_SET( mat->type, type ))
            return true;

    return false;
}

bool material_is_flagged( Object *obj, int flags )
{
    vector<material_t *> result;

    material_parse( obj, result );
    
    for (auto &mat: result)
        if (IS_SET( mat->flags, flags ))
            return true;

    return false;
}

int material_swims( Object *obj )
{
    int swim = 0;
    vector<material_t *> result;

    material_parse( obj, result );
    
    for (auto &mat: result)
        swim += mat->floats;
    
    if (swim > 0)
        return SWIM_ALWAYS;
    else if (swim < 0)
        return SWIM_NEVER;
    else
        return SWIM_UNDEF;
}

int material_burns( Object *obj )
{
    int max_burn = 0;
    vector<material_t *> result;

    material_parse( obj, result );
    
    for (auto &mat: result)
        if (mat->burns < 0)
            return mat->burns;
        else if (mat->burns > max_burn)
            max_burn = mat->burns;

    return max_burn;
}

int material_immune( Object *obj, Character *ch )
{
    bitstring_t bits = 0;
    int res = RESIST_NORMAL;    
    vector<material_t *> result;

    material_parse( obj, result );
    
    for (auto &mat: result)
        SET_BIT( bits, mat->vuln );
    
    immune_from_flags( ch, bits, res );

    return immune_resolve( res );
}