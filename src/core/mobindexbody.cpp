/*
 * Mob reform: prototype body and numbers (plan docs/plans/mob-reform.md
 * §3.3b, §3.5, §3.6 items 3-6).
 *
 * resolveBody() builds the prototype's nine bit sets and wearlocs. A race with
 * <forms> goes through the body resolver (body.h) with the prototype's
 * authored add/del applied last. Any other race keeps the historical
 * 'add | (race & ~del)' rule.
 *
 * Dels on aff/det/imm/res/vuln were dead for thirty years: the instance
 * re-ORed the race bits. They stay dead (now in the index itself, so the
 * instance is a plain copy) until the prototype names the set in <reviewed>
 * (plan §5.3 step 7, decision 17).
 *
 * deriveNumbers() resolves the tier and, while fight/mob_tiers.json is loaded,
 * replaces the authored hit/mana/damage/hitroll/ac/wealth with the tier
 * centres, picks the enabled off bits (decision 39) and adds the tier's
 * affect bits (decision 56). Without the file the
 * authored numbers stay, as before the reform.
 */
#include <algorithm>
#include <cmath>
#include <set>
#include <sstream>

#include "logstream.h"
#include "mobilefactory.h"
#include "mobbody.h"
#include "race.h"
#include "wearlocation.h"
#include "flagtable.h"
#include "merc.h"
#include "def.h"

int &mob_index_data::bodyBits(int mobset)
{
    switch (mobset) {
    case MOBSET_ACT:   return act;
    case MOBSET_OFF:   return off_flags;
    case MOBSET_AFF:   return affected_by;
    case MOBSET_DET:   return detection;
    case MOBSET_IMM:   return imm_flags;
    case MOBSET_RES:   return res_flags;
    case MOBSET_VULN:  return vuln_flags;
    case MOBSET_FORM:  return form;
    default:           return parts;
    }
}

unsigned long long mob_index_data::bodyStamp()
{
    std::ostringstream buf;
    // Only inputs a saved body or number is built from. Not wealth (gold is
    // saved whole), not the tier's spelling (tier 3 == tier champion), and the
    // wearlocs as a sorted name set: registry index order follows the order
    // race files come off the disk.
    buf << level << '|' << tier << '|' << race << '|' << getSize() << '|';
    for (int s = 0; s < MOBSET_MAX; s++)
        buf << bodyBits(s) << ',';
    std::set<DLString> wearNames;
    if (wearloc.getRegistry())
        for (int ndx: wearloc.toArray())
            wearNames.insert(wearloc.getRegistry()->getName(ndx));
    buf << '|';
    for (auto &w: wearNames)
        buf << w << ',';
    buf << '|' << numbersDerived << '|';
    for (int i = 0; i < 3; i++)
        buf << hit[i] << ',' << mana[i] << ',' << damage[i] << ',';
    for (int i = 0; i < 4; i++)
        buf << ac[i] << ',';
    buf << hitroll << ',' << saves << ',' << statCap;

    // FNV-1a, 64 bit: stable across builds and platforms, unlike std::hash.
    unsigned long long h = 14695981039346656037ULL;
    for (unsigned char c: buf.str()) {
        h ^= c;
        h *= 1099511628211ULL;
    }
    return h;
}

void mob_index_data::bodyDiff(int mobset, bitstring_t &add, bitstring_t &del) const
{
    bitstring_t now = (unsigned int)const_cast<mob_index_data *>(this)->bodyBits(mobset);
    bitstring_t on = now & ~bodySnapshot[mobset];
    bitstring_t off = bodySnapshot[mobset] & ~now;

    add = (bodyAdd[mobset] | on) & ~off;
    del = (bodyDel[mobset] | off) & ~on;

    if (mobset == MOBSET_ACT)
        add &= ~(bitstring_t)ACT_IS_NPC;
}

/* Body::BitSet index for a MOBSET_ set, -1 for form and parts. */
static int bitset_of(int mobset)
{
    switch (mobset) {
    case MOBSET_ACT:  return Body::BS_ACT;
    case MOBSET_OFF:  return Body::BS_OFF;
    case MOBSET_AFF:  return Body::BS_AFF;
    case MOBSET_DET:  return Body::BS_DET;
    case MOBSET_IMM:  return Body::BS_IMM;
    case MOBSET_RES:  return Body::BS_RES;
    case MOBSET_VULN: return Body::BS_VULN;
    }
    return -1;
}

/* Dels on these sets are honoured only for reviewed prototypes. */
static bool del_gated(int bitSet)
{
    return bitSet == Body::BS_AFF || bitSet == Body::BS_DET || bitSet == Body::BS_IMM
           || bitSet == Body::BS_RES || bitSet == Body::BS_VULN;
}

static const Flags &race_set(Race *race, int mobset)
{
    switch (mobset) {
    case MOBSET_ACT:   return race->getAct();
    case MOBSET_OFF:   return race->getOff();
    case MOBSET_AFF:   return race->getAff();
    case MOBSET_DET:   return race->getDet();
    case MOBSET_IMM:   return race->getImm();
    case MOBSET_RES:   return race->getRes();
    case MOBSET_VULN:  return race->getVuln();
    case MOBSET_FORM:  return race->getForm();
    default:           return race->getParts();
    }
}

/* Off bits a body may use by its parts and forms (plan §3.3b). */
static Body::NameSet body_allowed_off(const Body::Result &r, int size)
{
    Body::NameSet allowed;
    auto has = [&r](const char *part) { return r.parts.count(part) > 0; };
    bool hooves = has("two_hooves") || has("four_hooves");
    bool sentient = r.forms.count("sentient") > 0;

    if (has("hands")) {
        allowed.insert("disarm");
        allowed.insert("parry");
        allowed.insert("backstab");
    }
    if (has("legs") || hooves) {
        allowed.insert("kick");
        allowed.insert("trip");
        allowed.insert("kick_dirt");
    }
    if (has("legs"))
        allowed.insert("bash");
    if (has("tail"))
        allowed.insert("tail");
    if (has("tentacles") || has("spines") || r.forms.count("dragon") || r.forms.count("blob"))
        allowed.insert("area_attack");
    if (size >= SIZE_LARGE || hooves || r.forms.count("mammal"))
        allowed.insert("crush");
    allowed.insert("dodge");
    allowed.insert("fast");
    allowed.insert("berserk");
    if (sentient) {
        allowed.insert("rescue");
        allowed.insert("fade");
    }
    return allowed;
}

void mob_index_data::resolveBody()
{
    Race *mobrace = raceManager->find(race);

    if (!mobrace->hasBodyForms()) {
        // Legacy body: add | (race & ~del), dead dels kept dead unless reviewed.
        for (int s = 0; s < MOBSET_MAX; s++) {
            bitstring_t base = race_set(mobrace, s).getValue();
            int bs = bitset_of(s);
            bitstring_t del = bodyDel[s];
            if (bs >= 0 && del_gated(bs) && !(reviewed & (1 << bs)))
                del = 0;
            bodyBits(s) = (int)(bodyAdd[s] | (base & ~del));
        }
        act |= ACT_IS_NPC;
        wearloc.setRegistry(wearlocationManager);
        wearloc.set(mobrace->getWearloc());
        bodyResolved = false;
        offAllowed = off_flags;
        movetype.clear();
        moveverb.clear();
        formAcPct = 100;
        // Same rule as IS_BLOODLESS, gated the same way.
        bloodless = IS_SET(form, FORM_SKELETAL|FORM_CONSTRUCT|FORM_MIST)
                    || (mob_body_model_active && !IS_SET(parts, PART_HEART)
                        && !IS_SET(parts, PART_COLD_BLOOD) && !IS_SET(form, FORM_COLD_BLOOD));
        edible = IS_EDIBLE_FORM(form);
        canHoldCards = CAN_HOLD_CARDS(this);
    } else {
        Body::Input in;
        mobrace->getBodyInput(in, true);
        in.size = size_table.name(getSize());

        Body::Mods &p = in.proto;
        Body::NameSet fa = MobBody::names(&form_flags, bodyAdd[MOBSET_FORM]);
        Body::NameSet fd = MobBody::names(&form_flags, bodyDel[MOBSET_FORM]);
        p.formsAdd.assign(fa.begin(), fa.end());
        p.formsDel.assign(fd.begin(), fd.end());
        p.partsAdd = MobBody::names(&part_flags, bodyAdd[MOBSET_PARTS]);
        p.partsDel = MobBody::names(&part_flags, bodyDel[MOBSET_PARTS]);
        for (int s = 0; s < MOBSET_MAX; s++) {
            int bs = bitset_of(s);
            if (bs < 0)
                continue;
            p.bitsAdd[bs] = MobBody::names(MobBody::bitSetTable(bs), bodyAdd[s]);
            if (!del_gated(bs) || (reviewed & (1 << bs)))
                p.bitsDel[bs] = MobBody::names(MobBody::bitSetTable(bs), bodyDel[s]);
        }

        Body::Result r = Body::resolve(MobBody::forms(), in);
        for (auto &w: r.warnings)
            LogStream::sendWarning() << "mob " << vnum << " (" << race << "): " << w << endl;

        MobBody::Engine e;
        e.fromResult(r);
        form = (int)e.form;
        parts = (int)e.parts;
        act = (int)e.bits[Body::BS_ACT] | ACT_IS_NPC;
        off_flags = (int)e.bits[Body::BS_OFF];
        affected_by = (int)e.bits[Body::BS_AFF];
        detection = (int)e.bits[Body::BS_DET];
        imm_flags = (int)e.bits[Body::BS_IMM];
        res_flags = (int)e.bits[Body::BS_RES];
        vuln_flags = (int)e.bits[Body::BS_VULN];
        MobBody::wearlocs(e.wearlocNames, wearloc);
        bodyResolved = true;
        movetype = e.movetype;
        moveverb = e.moveverb;
        formAcPct = (int)lround(e.formAc * 100);
        bloodless = e.bloodless;
        edible = e.edible;
        canHoldCards = e.canHoldCards;

        // Allowed off bits (decision 39): what the body can do, plus what the
        // forms and the race add, minus what a form or the race forbids. The
        // prototype's own off add/del are applied on top in deriveNumbers().
        Body::NameSet allowed = body_allowed_off(r, getSize());
        Body::Input raceOnly = in;
        raceOnly.proto.bitsAdd[Body::BS_OFF].clear();
        raceOnly.proto.bitsDel[Body::BS_OFF].clear();
        Body::Result rr = Body::resolve(MobBody::forms(), raceOnly);
        allowed.insert(rr.bits[Body::BS_OFF].begin(), rr.bits[Body::BS_OFF].end());
        for (auto &f: rr.forms) {
            auto fi = MobBody::forms().forms.find(f);
            if (fi != MobBody::forms().forms.end())
                for (auto &d: fi->second.bitsDel[Body::BS_OFF])
                    allowed.erase(d);
        }
        for (auto &d: in.race.bitsDel[Body::BS_OFF])
            allowed.erase(d);
        offAllowed = (int)MobBody::bits(&::off_flags, allowed);
    }

    for (int s = 0; s < MOBSET_MAX; s++)
        bodySnapshot[s] = (unsigned int)bodyBits(s);
}

/* Off bits enabled for a prototype: tier count over a per-class priority list (§3.3b). */
static bitstring_t enabled_off(mob_index_data *mob, int count)
{
    const MobTiers::Config &tc = MobBody::tiers();
    bitstring_t allowed = (unsigned int)mob->offAllowed;
    bitstring_t enabled = 0;

    // assist_* are behaviour, not body: whatever allows them enables them.
    enabled |= allowed & (ASSIST_ALL|ASSIST_ALIGN|ASSIST_RACE|ASSIST_PLAYERS|ASSIST_GUARD|ASSIST_VNUM);

    if (count < 0)
        return allowed;

    static const char * const classes[] = { "warrior", "thief", "mage", "cleric", "necromancer", "ranger", "vampire" };
    const std::vector<std::string> *list = 0;
    for (const char *c: classes) {
        int bit = act_flags.value(c);
        if (bit != NO_FLAG && IS_SET(mob->act, 1LL << bit) && tc.offPriority.count(c)) {
            list = &tc.offPriority.at(c);
            break;
        }
    }
    if (!list) {
        const char *key = IS_SET(mob->form, FORM_SENTIENT) ? "sentient" : "animal";
        auto i = tc.offPriority.find(key);
        if (i != tc.offPriority.end())
            list = &i->second;
    }
    if (!list)
        return enabled;

    for (auto &name: *list) {
        if (count <= 0)
            break;
        int bit = off_flags.value(name);
        if (bit == NO_FLAG)
            continue;
        bitstring_t b = 1LL << bit;
        if ((allowed & b) && !(enabled & b)) {
            enabled |= b;
            count--;
        }
    }
    return enabled;
}

void mob_index_data::deriveNumbers()
{
    const MobTiers::Config &tc = MobBody::tiers();

    tier = tc.loaded ? tc.parse(tierName) : 0;
    if (!tier)
        tier = tc.loaded ? tc.defaultTier : MobTiers::TIER_NORMAL;
    if (!tierName.empty() && tc.loaded && !tc.parse(tierName))
        LogStream::sendWarning() << "mob " << vnum << ": unknown tier '" << tierName << "', using default" << endl;

    if (!tc.loaded) {
        numbersDerived = false;
        return;
    }

    Race *mobrace = raceManager->find(race);
    MobTiers::MobInfo m;
    m.vnum = vnum;
    m.level = level;
    m.tier = tier;
    m.formAc = formAcPct / 100.0;
    m.style = tierStyle;
    if (!tierStyle.empty() && !tc.styles.count(tierStyle))
        LogStream::sendWarning() << "mob " << vnum << ": unknown tier style '" << tierStyle << "'" << endl;
    m.sentient = IS_SET(form, FORM_SENTIENT);
    m.acts = MobBody::names(&act_flags, (unsigned int)act);

    MobTiers::Centres c = tc.centres(m);
    int hp = std::max(1, (int)lround(c.hp * mobrace->getHpMult()));
    int dam = std::max(1, (int)lround(c.dmgAve * mobrace->getDmgMult()));

    // hit = NdT+B: N = level/4+1, T so the dice average sits on the centre.
    int n = std::max(level, 0) / 4 + 1;
    int t = std::max(2, (int)lround(2.0 * hp / n - 1));
    hit[DICE_NUMBER] = n;
    hit[DICE_TYPE] = t;
    hit[DICE_BONUS] = hp - n * (t + 1) / 2;

    // damage = NdT+B: N = level/10+1, B (damroll) carries the rest.
    n = std::max(level, 0) / 10 + 1;
    t = std::max(2, (int)lround(2.0 * dam / n - 1));
    damage[DICE_NUMBER] = n;
    damage[DICE_TYPE] = t;
    damage[DICE_BONUS] = std::max(0, dam - n * (t + 1) / 2);

    mana[DICE_NUMBER] = 0;
    mana[DICE_TYPE] = 0;
    mana[DICE_BONUS] = c.mana;

    hitroll = c.hitrollBonus;
    for (int i = 0; i < 4; i++)
        ac[i] = c.ac * 10;
    wealth = (int)c.wealth;
    saves = c.saves;
    statCap = c.statCap;
    numbersDerived = true;

    // Affect bits granted by the tier (item 56: sanctuary from elite up), on top
    // of the body. resolveBody() always runs first, so a tier change never stacks.
    const MobTiers::Tier &tierRow = tc.get(tier);
    if (!tierRow.affAdd.empty()) {
        Body::NameSet tierAff(tierRow.affAdd.begin(), tierRow.affAdd.end());
        affected_by |= (int)MobBody::bits(&::affect_flags, tierAff);
        bodySnapshot[MOBSET_AFF] = (unsigned int)affected_by;
    }

    // Act bits granted by the tier (decision 72: warrior from champion up), not
    // for casters: their kit is spells, and the class bit picks their off list.
    if (!tierRow.actAdd.empty() && !tc.isCaster(m)) {
        Body::NameSet tierAct(tierRow.actAdd.begin(), tierRow.actAdd.end());
        act |= (int)MobBody::bits(&::act_flags, tierAct);
        bodySnapshot[MOBSET_ACT] = (unsigned int)act;
    }

    if (bodyResolved) {
        bitstring_t enabled = enabled_off(this, tc.offCount(tier, vnum));
        if (!tierRow.offAdd.empty()) {
            Body::NameSet tierOff(tierRow.offAdd.begin(), tierRow.offAdd.end());
            bitstring_t add = MobBody::bits(&::off_flags, tierOff) & (unsigned int)offAllowed;
            // A slow body (worm, slug, zombie) never turns fast.
            if (IS_SET(affected_by, AFF_SLOW))
                add &= ~(bitstring_t)OFF_FAST;
            enabled |= add;
        }
        enabled |= bodyAdd[MOBSET_OFF];
        enabled &= ~bodyDel[MOBSET_OFF];
        off_flags = (int)enabled;
        bodySnapshot[MOBSET_OFF] = (unsigned int)off_flags;
    }
}
