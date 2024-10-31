#ifndef MOBILEFACTORY_H
#define MOBILEFACTORY_H

#include <jsoncpp/json/json.h>
#include "fenia/register-decl.h"
#include "xmlmultistring.h"
#include "globalbitvector.h"
#include "grammar_entities.h"
#include "lang.h"
#include "clanreference.h"
#include "mobilespecial.h"
#include "xmldocument.h"

struct mob_index_data;
class AreaIndexData;

typedef struct mob_index_data MOB_INDEX_DATA;

/* dice */
#define DICE_NUMBER 0
#define DICE_TYPE   1
#define DICE_BONUS  2

/*
 * Prototype for a mob.
 */
struct mob_index_data
{
    mob_index_data( );
    virtual ~mob_index_data();

    MOB_INDEX_DATA *        next;
    ProgWrapper<SPEC_FUN> spec_fun;
    int                vnum;
    int                group;
    int                count;
    int                killed;

    // Replace player_name with multi-lang keywords.
    XMLMultiString keyword;

    XMLMultiString   short_descr;
    XMLMultiString   long_descr;
    XMLMultiString   description;
    XMLMultiString smell;

    int                act;
    int                affected_by;
    int                detection;
    int                alignment;
    int                level;
    int                hitroll;
    int                        hit[3];
    int                        mana[3];
    int                damage[3];
    int                ac[4];
    int                 dam_type;
    int                off_flags;
    int                imm_flags;
    int                res_flags;
    int                vuln_flags;
    int                start_pos;
    int                default_pos;
    int                sex;
    DLString           race;
    int                wealth;
    int                form;
    int                parts;
    int                size;
    DLString           material;
    GlobalBitvector     practicer;
    GlobalBitvector religion;
    GlobalBitvector affects;
    GlobalBitvector behaviors;
    Json::Value props;
    Grammar::Number     gram_number;
    XMLDocument::Pointer behavior;
    Scripting::Object *wrapper;
    AreaIndexData *                area;
    ClanReference clan;

    int getSize() const;

    /** Return props value for the key (props[key] or props["xxx"][key]). */
    DLString getProperty(const DLString &key) const;

    const char * getDescription( lang_t lang ) const;
    const char * getShortDescr( lang_t lang ) const;
    const char * getLongDescr( lang_t lang ) const;
};


// Global hash table mapping mob virtual number to the mob index struct.
extern mob_index_data  * mob_index_hash[1024];

/*
 * Translates mob virtual number to its mob index struct.
 * Hash table lookup.
 */
mob_index_data * get_mob_index(int vnum);

#endif
