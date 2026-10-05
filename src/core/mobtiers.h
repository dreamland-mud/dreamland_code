/*
 * Mob tiers (mob reform, plan docs/plans/mob-reform.md §3.5 and §4).
 *
 * Parsed fight/mob_tiers.json plus the pure arithmetic that turns level, tier
 * and body into the centre numbers of a prototype. No engine dependencies.
 *
 * File schema (all keys optional unless noted; Phase C writes the real one):
 *
 * {
 *   "version": 1,
 *   "default_tier": "normal",
 *   "names": { "trash": 9, "normal": 7, "elite": 5, "champion": 3, "boss": 1 },
 *   "tiers": {                                   // required, keyed by number 1..10
 *     "7": { "name": "normal",
 *            "hp": 1.0, "dmg": 1.0, "ac": 1.0, "mana": 1.0,
 *            "hitroll": 0.0,                     // bonus per level (L/10 = 0.1)
 *            "saves": -0.05,                     // saving throw per level (-L/20)
 *            "stat_cap": 13,                     // + level * stat_cap_per_level
 *            "off_enabled": 1, "off_parity": true, // count of enabled off bits, -1 = all
 *            "xp": 1.0, "xp_min_level": 0,       // xp mult applies from this victim level on
 *                                                // (trash "x0.75 above level 10" = 11)
 *            "gold": 1.0,
 *            "aff_add": [ "sanctuary" ],         // affect bits the tier grants (item 56)
 *            "off_add": [ "fast" ],              // off bits on top of the count, body-allowed, never on a slow body
 *            "act_add": [ "warrior" ],           // act bits for non-casters (decision 72)
 *            "word": { "en": "...", "ru": "...", "ua": "..." } },
 *     ... missing numbers are interpolated between their neighbours
 *   },
 *   "stat_cap_per_level": 0.25,
 *   "variance": { "hp": 0.15, "dmg": 0.15, "hitroll": 0.10, "saves": 0.10, "mana": 0.20 },
 *   "base": {                                    // required: points of a piecewise-linear curve
 *     "level":  [ 1, 5, 15, ... ],
 *     "hp":     [ ... ], "dmg": [ ... ], "ac": [ ... ], "mana": [ ... ], "wealth": [ ... ]
 *   },
 *   "class_mult": { "hp": { "mage": 0.85, "cleric": 0.85 }, "dmg": { "warrior": 1.15 } },
 *   "noncaster_mana_per_level": 5,
 *   "caster_acts": [ "mage", "cleric", "necromancer", "vampire" ],
 *   "caster_vnums": [ ],
 *   "off_priority": { "warrior": [...], "thief": [...], "mage": [...], "cleric": [...],
 *                     "sentient": [...], "animal": [...] }
 * }
 */
#ifndef MOBTIERS_H
#define MOBTIERS_H

#include <map>
#include <set>
#include <string>
#include <vector>
#include <jsoncpp/json/json.h>

namespace MobTiers {

const int TIER_BEST = 1;
const int TIER_WORST = 10;
const int TIER_NORMAL = 7;

struct Tier {
    bool defined = false;
    std::string name;
    double hp = 1, dmg = 1, ac = 1, mana = 1;
    double hitroll = 0, saves = 0;
    double statCap = 13;
    int offEnabled = -1;
    bool offParity = false;
    double xp = 1;
    int xpMinLevel = 0;
    double gold = 1;
    std::vector<std::string> affAdd;  // affect bits every mob of the tier gets (sanctuary, item 56)
    // Decision 72: off bits the tier enables when the body allows them (fast,
    // a slow form forbids it), and act bits for non-casters (warrior = extra attacks).
    std::vector<std::string> offAdd, actAdd;
    std::map<std::string, std::string> word;
};

struct Curve {
    std::vector<double> level, value;
    double at(int lvl) const;
    bool empty() const { return level.empty() || value.empty(); }
};

/** Centre numbers of one prototype, as compat() writes them into the index. */
struct Centres {
    int hp = 0, mana = 0, dmgAve = 0, hitrollBonus = 0, ac = 0, saves = 0, statCap = 0;
    long wealth = 0;
};

/** What the centre computation needs to know about a prototype. */
struct MobInfo {
    int vnum = 0;
    int level = 0;
    int tier = TIER_NORMAL;
    double formAc = 1.0;
    bool sentient = false;
    std::string style;               // <tierStyle>, empty = none
    std::set<std::string> acts;      // act flag names of the prototype
};

struct Config {
    bool loaded = false;
    int version = 0;
    int defaultTier = TIER_NORMAL;
    std::map<std::string, int> names;
    Tier tiers[TIER_WORST + 1];       // index 1..10, filled after interpolation
    double statCapPerLevel = 0.25;
    std::map<std::string, double> variance;
    Curve baseHp, baseDmg, baseAc, baseMana, baseWealth;
    std::map<std::string, std::map<std::string, double> > classMult;
    double noncasterManaPerLevel = 5;
    std::set<std::string> casterActs;
    std::set<int> casterVnums;
    std::map<std::string, std::vector<std::string> > offPriority;
    // Boss styles (Kit 2026-10-02): "fortress" fat but mild, "brute" deadly
    // but thin. hp and damage multipliers on top of the tier.
    std::map<std::string, std::pair<double, double> > styles;

    bool fromJson(const Json::Value &value, std::string &error);
    void clear();

    /** Name ("elite") or number ("5") to a tier number, 0 if neither. */
    int parse(const std::string &arg) const;
    /** Tier name for a number, or the number itself if unnamed. */
    std::string name(int tier) const;
    const Tier &get(int tier) const;
    double var(const std::string &key) const;

    bool isCaster(const MobInfo &mob) const;
    Centres centres(const MobInfo &mob) const;
    /** How many allowed off bits a prototype gets, -1 = all of them. */
    int offCount(int tier, int vnum) const;
};

}

#endif
