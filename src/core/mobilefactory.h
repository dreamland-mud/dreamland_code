#ifndef MOBILEFACTORY_H
#define MOBILEFACTORY_H

#include <jsoncpp/json/json.h>
#include "fenia/register-decl.h"
#include "xmlmultistring.h"
#include "bitstring.h"
#include "globalbitvector.h"
#include "grammar_entities.h"
#include "lang.h"
#include "clanreference.h"
#include "mobilespecial.h"
#include "xmldocument.h"

struct mob_index_data;
class AreaIndexData;

typedef struct mob_index_data MOB_INDEX_DATA;

/* Mob reform: the nine body bit sets of a prototype, as authored add/del pairs. */
enum {
    MOBSET_ACT = 0, MOBSET_OFF, MOBSET_AFF, MOBSET_DET, MOBSET_IMM, MOBSET_RES, MOBSET_VULN,
    MOBSET_FORM, MOBSET_PARTS, MOBSET_MAX
};

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

    // Mob reform (plan docs/plans/mob-reform.md §3.5, §3.6 items 3-6).
    // Authored add/del per bit set, kept so asave writes back what the builder
    // wrote even where the instance ignores it (unreviewed dels, item 17).
    bitstring_t        bodyAdd[MOBSET_MAX];
    bitstring_t        bodyDel[MOBSET_MAX];
    // Body bits as resolveBody/deriveNumbers left them, to spot later direct edits.
    bitstring_t        bodySnapshot[MOBSET_MAX];
    // Bit sets (Body::BitSet order act..vuln) whose authored dels are honoured: <reviewed>.
    int                reviewed;
    // Authored <tier> (name or 1-10, empty = default) and the resolved number.
    DLString           tierName;
    int                tier;
    // Body: true when it came from the resolver (race with <forms>), else legacy.
    bool               bodyResolved;
    GlobalBitvector    wearloc;
    int                offAllowed;     // off bits the body may use (item 39)
    DLString           movetype, moveverb;
    int                formAcPct;      // form AC factor x100
    bool               bloodless, edible, canHoldCards;
    // Numbers: true when hit/mana/damage/hitroll/ac/wealth are tier centres.
    bool               numbersDerived;
    int                saves;
    int                statCap;

    int getSize() const;

    /** Mob reform: build body bits and wearlocs from race + authored diffs. */
    void resolveBody();
    /** Mob reform: tier, centre numbers and enabled off bits (needs vnum and body). */
    void deriveNumbers();
    /** The index field holding one bit set (act, off_flags, ..., parts). */
    int &bodyBits(int mobset);
    /** Authored diff for asave: the loaded add/del plus any direct edit made since load. */
    void bodyDiff(int mobset, bitstring_t &add, bitstring_t &del);

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
