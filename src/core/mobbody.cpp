/*
 * Mob reform: engine side of the body model, see mobbody.h.
 */
#include <set>
#include "logstream.h"
#include "flagtable.h"
#include "globalbitvector.h"
#include "wearlocation.h"
#include "mobbody.h"
#include "merc.h"

namespace MobBody {

static Body::Config formsConfig;
static MobTiers::Config tiersConfig;
static unsigned long configGeneration = 1;

Body::Config &forms()
{
    return formsConfig;
}

MobTiers::Config &tiers()
{
    return tiersConfig;
}

unsigned long generation()
{
    return configGeneration;
}

void touch()
{
    configGeneration++;
}

static void warnOnce(const std::string &what)
{
    static std::set<std::string> warned;
    if (warned.insert(what).second)
        LogStream::sendWarning() << "mob body: " << what << endl;
}

const FlagTable *bitSetTable(int bitSet)
{
    switch (bitSet) {
    case Body::BS_ACT:  return &act_flags;
    case Body::BS_OFF:  return &off_flags;
    case Body::BS_AFF:  return &affect_flags;
    case Body::BS_DET:  return &detect_flags;
    case Body::BS_IMM:  return &imm_flags;
    case Body::BS_RES:  return &res_flags;
    case Body::BS_VULN: return &vuln_flags;
    }
    return 0;
}

Body::NameSet names(const FlagTable *table, bitstring_t b)
{
    Body::NameSet result;
    if (!table || b == 0 || b == NO_FLAG)
        return result;

    for (int i = 0; i < table->size; i++)
        if (b & (1LL << table->fields[i].value))
            result.insert(table->fields[i].name);

    return result;
}

bitstring_t bits(const FlagTable *table, const Body::NameSet &n)
{
    bitstring_t result = 0;
    if (!table)
        return result;

    for (auto &name: n) {
        int i = table->index(name, true);
        if (i == NO_FLAG)
            warnOnce("no flag '" + name + "' in its table, skipped");
        else
            result |= 1LL << table->fields[i].value;
    }
    return result;
}

void wearlocs(const Body::NameSet &n, GlobalBitvector &out)
{
    out.setRegistry(wearlocationManager);

    for (auto &name: n) {
        Wearlocation *loc = wearlocationManager->findExisting(name);
        if (!loc)
            warnOnce("no wearlocation '" + name + "', skipped");
        else
            out.set(loc->getIndex());
    }
}

void Engine::fromResult(const Body::Result &r)
{
    form = MobBody::bits(&form_flags, r.forms);
    parts = MobBody::bits(&part_flags, r.parts);
    for (int k = 0; k < Body::BS_MAX; k++)
        bits[k] = MobBody::bits(bitSetTable(k), r.bits[k]);
    wearlocNames = r.wearlocs;
    movetype = r.movetype;
    moveverb = r.moveverb;
    movedanger = r.movedanger;
    acFactor = r.acFactor;
    formAc = r.formAc;
    edible = r.isEdible;
    bloodless = r.bloodless;
    canHoldCards = r.canHoldCards;
    rideable = r.rideable;
}

}
