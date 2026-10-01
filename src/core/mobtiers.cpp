/*
 * Mob tiers, see mobtiers.h.
 */
#include <algorithm>
#include <cmath>
#include <cstdlib>

#include "mobtiers.h"

namespace MobTiers {

double Curve::at(int lvl) const
{
    size_t n = std::min(level.size(), value.size());
    if (n == 0)
        return 0;
    if (n == 1 || lvl <= level[0])
        return value[0];

    for (size_t i = 1; i < n; i++)
        if (lvl <= level[i]) {
            double span = level[i] - level[i-1];
            if (span <= 0)
                return value[i];
            return value[i-1] + (value[i] - value[i-1]) * (lvl - level[i-1]) / span;
        }

    // Past the last point: keep the slope of the last segment (levels above
    // 100 exist, the corpus goes to 170).
    double span = level[n-1] - level[n-2];
    if (span <= 0)
        return value[n-1];
    return value[n-1] + (value[n-1] - value[n-2]) * (lvl - level[n-1]) / span;
}

static void readCurve(const Json::Value &base, const char *key, Curve &c)
{
    const Json::Value &lv = base["level"], &vv = base[key];
    if (!lv.isArray() || !vv.isArray())
        return;
    for (const auto &e: lv)
        c.level.push_back(e.asDouble());
    for (const auto &e: vv)
        c.value.push_back(e.asDouble());
}

void Config::clear()
{
    *this = Config();
}

static void readTier(const Json::Value &d, Tier &t)
{
    t.defined = true;
    t.name = d.get("name", "").asString();
    t.hp = d.get("hp", 1.0).asDouble();
    t.dmg = d.get("dmg", 1.0).asDouble();
    t.ac = d.get("ac", 1.0).asDouble();
    t.mana = d.get("mana", 1.0).asDouble();
    t.hitroll = d.get("hitroll", 0.0).asDouble();
    t.saves = d.get("saves", 0.0).asDouble();
    t.statCap = d.get("stat_cap", 13).asDouble();
    t.offEnabled = d.get("off_enabled", -1).asInt();
    t.offParity = d.get("off_parity", false).asBool();
    t.xp = d.get("xp", 1.0).asDouble();
    t.xpMinLevel = d.get("xp_min_level", 0).asInt();
    t.gold = d.get("gold", 1.0).asDouble();
    const Json::Value &aa = d["aff_add"];
    if (aa.isArray())
        for (const auto &e: aa)
            t.affAdd.push_back(e.asString());
    const Json::Value &w = d["word"];
    if (w.isObject())
        for (const auto &lang: w.getMemberNames())
            t.word[lang] = w[lang].asString();
}

/** Linear blend of two defined tiers for an undefined number in between. */
static Tier blend(const Tier &a, const Tier &b, double f)
{
    Tier t = f < 0.5 ? a : b;  // discrete fields from the nearer neighbour
    auto mix = [f](double x, double y) { return x + (y - x) * f; };
    t.defined = false;
    t.name.clear();
    t.word.clear();
    t.hp = mix(a.hp, b.hp);
    t.dmg = mix(a.dmg, b.dmg);
    t.ac = mix(a.ac, b.ac);
    t.mana = mix(a.mana, b.mana);
    t.hitroll = mix(a.hitroll, b.hitroll);
    t.saves = mix(a.saves, b.saves);
    t.statCap = mix(a.statCap, b.statCap);
    t.xp = mix(a.xp, b.xp);
    t.gold = mix(a.gold, b.gold);
    return t;
}

bool Config::fromJson(const Json::Value &value, std::string &error)
{
    clear();

    if (!value.isObject() || !value["tiers"].isObject() || !value["base"].isObject()) {
        error = "no 'tiers'/'base' sections";
        return false;
    }

    version = value.get("version", 0).asInt();

    const Json::Value &nv = value["names"];
    if (nv.isObject())
        for (const auto &key: nv.getMemberNames())
            if (nv[key].isInt())
                names[key] = nv[key].asInt();

    const Json::Value &tv = value["tiers"];
    for (const auto &key: tv.getMemberNames()) {
        int n = atoi(key.c_str());
        if (n < TIER_BEST || n > TIER_WORST || !tv[key].isObject())
            continue;
        readTier(tv[key], tiers[n]);
        if (!tiers[n].name.empty() && names.count(tiers[n].name) == 0)
            names[tiers[n].name] = n;
    }

    int first = 0, last = 0;
    for (int n = TIER_BEST; n <= TIER_WORST; n++)
        if (tiers[n].defined) {
            if (!first)
                first = n;
            last = n;
        }
    if (!first) {
        error = "no tier defined";
        return false;
    }
    for (int n = TIER_BEST; n <= TIER_WORST; n++) {
        if (tiers[n].defined)
            continue;
        if (n < first) {
            tiers[n] = tiers[first];
            tiers[n].defined = false;
        } else if (n > last) {
            tiers[n] = tiers[last];
            tiers[n].defined = false;
        } else {
            int lo = n - 1, hi = n + 1;
            while (!tiers[lo].defined) lo--;
            while (!tiers[hi].defined) hi++;
            tiers[n] = blend(tiers[lo], tiers[hi], double(n - lo) / (hi - lo));
        }
    }

    defaultTier = TIER_NORMAL;
    if (value.isMember("default_tier")) {
        int t = parse(value["default_tier"].isString() ? value["default_tier"].asString()
                                                       : std::to_string(value["default_tier"].asInt()));
        if (t)
            defaultTier = t;
    }

    statCapPerLevel = value.get("stat_cap_per_level", 0.25).asDouble();

    const Json::Value &var = value["variance"];
    if (var.isObject())
        for (const auto &key: var.getMemberNames())
            variance[key] = var[key].asDouble();

    const Json::Value &base = value["base"];
    readCurve(base, "hp", baseHp);
    readCurve(base, "dmg", baseDmg);
    readCurve(base, "ac", baseAc);
    readCurve(base, "mana", baseMana);
    readCurve(base, "wealth", baseWealth);
    if (baseHp.empty() || baseDmg.empty()) {
        error = "base.level/base.hp/base.dmg missing";
        return false;
    }

    const Json::Value &cm = value["class_mult"];
    if (cm.isObject())
        for (const auto &stat: cm.getMemberNames())
            if (cm[stat].isObject())
                for (const auto &act: cm[stat].getMemberNames())
                    classMult[stat][act] = cm[stat][act].asDouble();

    noncasterManaPerLevel = value.get("noncaster_mana_per_level", 5).asDouble();

    const Json::Value &ca = value["caster_acts"];
    if (ca.isArray())
        for (const auto &e: ca)
            casterActs.insert(e.asString());
    else
        casterActs = { "mage", "cleric", "necromancer", "vampire" };

    const Json::Value &cv = value["caster_vnums"];
    if (cv.isArray())
        for (const auto &e: cv)
            casterVnums.insert(e.asInt());

    const Json::Value &op = value["off_priority"];
    if (op.isObject())
        for (const auto &key: op.getMemberNames())
            if (op[key].isArray())
                for (const auto &e: op[key])
                    offPriority[key].push_back(e.asString());

    loaded = true;
    return true;
}

int Config::parse(const std::string &arg) const
{
    if (arg.empty())
        return 0;
    auto i = names.find(arg);
    if (i != names.end())
        return i->second;
    if (arg.find_first_not_of("0123456789") == std::string::npos && arg.size() <= 2) {
        int n = atoi(arg.c_str());
        if (n >= TIER_BEST && n <= TIER_WORST)
            return n;
    }
    return 0;
}

std::string Config::name(int tier) const
{
    if (tier >= TIER_BEST && tier <= TIER_WORST && !tiers[tier].name.empty())
        return tiers[tier].name;
    for (auto &n: names)
        if (n.second == tier)
            return n.first;
    return std::to_string(tier);
}

const Tier &Config::get(int tier) const
{
    if (tier < TIER_BEST || tier > TIER_WORST)
        tier = defaultTier;
    return tiers[tier];
}

double Config::var(const std::string &key) const
{
    auto i = variance.find(key);
    return i == variance.end() ? 0 : i->second;
}

bool Config::isCaster(const MobInfo &mob) const
{
    if (casterVnums.count(mob.vnum))
        return true;
    for (auto &a: mob.acts)
        if (casterActs.count(a))
            return true;
    return false;
}

static double classFactor(const Config &cfg, const char *stat, const std::set<std::string> &acts)
{
    auto i = cfg.classMult.find(stat);
    if (i == cfg.classMult.end())
        return 1.0;
    double f = 1.0;
    for (auto &m: i->second)
        if (acts.count(m.first))
            f *= m.second;
    return f;
}

Centres Config::centres(const MobInfo &mob) const
{
    Centres c;
    const Tier &t = get(mob.tier);
    int lvl = std::max(mob.level, 1);

    c.hp = std::max(1, (int)lround(baseHp.at(lvl) * t.hp * classFactor(*this, "hp", mob.acts)));
    c.dmgAve = std::max(1, (int)lround(baseDmg.at(lvl) * t.dmg * classFactor(*this, "dmg", mob.acts)));
    c.hitrollBonus = (int)lround(lvl * t.hitroll);
    c.ac = (int)lround(baseAc.at(lvl) * t.ac * mob.formAc);
    c.saves = (int)lround(lvl * t.saves);
    c.statCap = (int)lround(t.statCap + lvl * statCapPerLevel);

    if (isCaster(mob) && !baseMana.empty())
        c.mana = std::max(0, (int)lround(baseMana.at(lvl) * t.mana));
    else
        c.mana = std::max(0, (int)lround(noncasterManaPerLevel * lvl));

    if (mob.sentient && !baseWealth.empty())
        c.wealth = std::max(0L, lround(baseWealth.at(lvl) * t.gold));

    return c;
}

int Config::offCount(int tier, int vnum) const
{
    const Tier &t = get(tier);
    if (t.offEnabled < 0)
        return -1;
    return t.offEnabled + (t.offParity ? std::abs(vnum) % 2 : 0);
}

}
