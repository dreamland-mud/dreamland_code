/*
 * Mob body resolver (mob reform, plan docs/plans/mob-reform.md §3.3a).
 *
 * The C++ twin of scripts/body-resolve.py. It works on plain names (form,
 * part, flag and wearloc names as they appear in fight/mob_forms.json), not on
 * engine bit tables, so it has no engine dependencies and can be unit-tested
 * against the shared fixtures (src/core/body_test.cpp).
 *
 * The resolution order is the Python script's steps 0-11 and must stay in
 * lock step with it: any change here needs the same change there and the
 * fixtures regenerated.
 */
#ifndef BODY_H
#define BODY_H

#include <map>
#include <set>
#include <string>
#include <vector>
#include <jsoncpp/json/json.h>

namespace Body {

typedef std::set<std::string> NameSet;
typedef std::vector<std::string> NameList;

/** The seven bit sets a body carries besides parts. Order is fixed. */
enum BitSet { BS_ACT = 0, BS_OFF, BS_AFF, BS_DET, BS_IMM, BS_RES, BS_VULN, BS_MAX };
extern const char * const bitSetNames[BS_MAX];

/** Add/del lists of one layer (race or prototype). */
struct Mods {
    NameList formsAdd, formsDel;
    NameSet partsAdd, partsDel;
    NameSet bitsAdd[BS_MAX], bitsDel[BS_MAX];
    NameSet wearlocsAdd, wearlocsDel;

    bool empty() const;
};

struct FormDef {
    std::string group;           // functional, lower, upper, full
    NameList impliesForms;
    NameSet partsAdd, partsDel, partsOnly, partsAlways, partsDelLate;
    bool lateDelNpcOnly = false;
    NameSet bitsAdd[BS_MAX];     // <k>_add plus the bare <k> list (bare only for aff..vuln)
    NameSet bitsDel[BS_MAX];     // act_del, off_del only
    NameSet wearlocsAdd;
    bool noFloat = false;
    std::string moveverb;
    bool hasAcFactor = false;
    double acFactor = 0;
    // hoofed only: per lower body kind ("biped", "quadruped")
    std::map<std::string, std::pair<NameSet, NameSet> > hooves; // kind -> (add, del)
};

struct PartDef {
    NameSet wearlocs, sentientWearlocs, npcWearlocs, quadrupedWearlocs, forbidsWearlocs;
    std::string requirePart;
    std::map<int, NameSet> implies, absentImplies; // BitSet -> names
};

struct VerbRow {
    std::string base;
    std::string danger;
    int wait = 1;
    bool textOnly = false;
};

/** Parsed fight/mob_forms.json. */
struct Config {
    bool loaded = false;
    int version = 0;
    std::map<std::string, std::string> renames;
    std::map<std::string, std::string> deletedForms;
    std::map<std::string, FormDef> forms;
    std::map<std::string, PartDef> parts;
    NameSet wearlocsAlways;
    std::map<std::string, VerbRow> verbRows;
    std::map<std::string, std::string> verbByBase;
    NameList formVerbPriority;
    double defaultAcFactor = 3.0;
    NameList sizeOrder;

    /** Parse the JSON. Returns false (and leaves loaded=false) on a missing or broken file. */
    bool fromJson(const Json::Value &value, std::string &error);
    void clear();
};

struct Input {
    NameList forms;
    std::string size = "medium";
    bool npc = true;
    Mods race, proto;
    std::string moveverb;        // race <moveverb>, empty if none
};

struct Result {
    NameSet forms, parts, wearlocs;
    NameSet bits[BS_MAX];
    std::string movetype, moveverb, movedanger;
    double acFactor = 3.0;
    double formAc = 1.0;
    bool isEdible = true, canHoldCards = false, rideable = false, bloodless = false;
    NameList warnings;
};

/** Steps 0-11 of scripts/body-resolve.py. */
Result resolve(const Config &cfg, const Input &input);

/** Body-model sizes are compared by their index in size_order. -1 if unknown. */
int sizeIndex(const Config &cfg, const std::string &size);

}

#endif
