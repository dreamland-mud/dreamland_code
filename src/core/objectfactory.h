#ifndef OBJECTFACTORY_H
#define OBJECTFACTORY_H

#include <jsoncpp/json/json.h>
#include "fenia/register-decl.h"
#include "xmlmultistring.h"
#include "grammar_entities.h"
#include "affectlist.h"
#include "extradescription.h"
#include "xmldocument.h"
#include "globalbitvector.h"

class Object;
class AreaIndexData;

struct obj_index_data;
typedef struct obj_index_data OBJ_INDEX_DATA;

typedef list<Object *> ObjectList;

/*
 * Prototype for an object.
 */
struct        obj_index_data
{
    obj_index_data();
    virtual ~obj_index_data();

    OBJ_INDEX_DATA *        next;
    ExtraDescrList extraDescriptions;
    AffectList        affected;

    // Replace 'name' with multi-lang keywords.
    XMLMultiString keyword;
    XMLMultiString   short_descr;
    XMLMultiString   description;
    XMLMultiString smell;
    XMLMultiString sound;

    int                vnum;
    int                reset_num;
    DLString material;
    int                item_type;
    int               extra_flags;
    int               wear_flags;
    int                level;
    int                 condition;
    int                count;
    int                weight;
    int                        cost;
    int                        value[5];
    int                 limit;
    Grammar::MultiGender gram_gender;
    XMLDocument::Pointer behavior;
    Scripting::Object *wrapper;
    AreaIndexData *                area;
    ObjectList instances;

    GlobalBitvector behaviors;
    Json::Value props;

    /** Return props value for the key (props[key] or props["xxx"][key]). */
    DLString getProperty(const DLString &key) const;

    const char * getDescription( lang_t lang ) const;
    const char * getShortDescr( lang_t lang ) const;
};

// Global hash table mapping obj virtual number to the obj index struct.
extern obj_index_data * obj_index_hash[1024];


/*
 * Translates room virtual number to its obj index struct.
 * Hash table lookup.
 */
obj_index_data * get_obj_index(int vnum);

#endif
