/*
 * Mob body resolver, see body.h. Mirrors scripts/body-resolve.py step for step.
 */
#include <algorithm>
#include <cmath>
#include <sstream>

#include "body.h"

namespace Body {

const char * const bitSetNames[BS_MAX] = { "act", "off", "aff", "det", "imm", "res", "vuln" };

bool Mods::empty() const
{
    if (!formsAdd.empty() || !formsDel.empty() || !partsAdd.empty() || !partsDel.empty())
        return false;
    for (int k = 0; k < BS_MAX; k++)
        if (!bitsAdd[k].empty() || !bitsDel[k].empty())
            return false;
    return true;
}

static int bitSetIndex(const std::string &name)
{
    for (int k = 0; k < BS_MAX; k++)
        if (name == bitSetNames[k])
            return k;
    return -1;
}

static void readList(const Json::Value &v, NameList &out)
{
    if (!v.isArray())
        return;
    for (const auto &e: v)
        if (e.isString())
            out.push_back(e.asString());
}

static void readSet(const Json::Value &v, NameSet &out)
{
    if (!v.isArray())
        return;
    for (const auto &e: v)
        if (e.isString())
            out.insert(e.asString());
}

static void readImplies(const Json::Value &v, std::map<int, NameSet> &out)
{
    if (!v.isObject())
        return;
    for (const auto &key: v.getMemberNames()) {
        int k = bitSetIndex(key);
        if (k >= 0)
            readSet(v[key], out[k]);
    }
}

void Config::clear()
{
    *this = Config();
}

bool Config::fromJson(const Json::Value &value, std::string &error)
{
    clear();

    if (!value.isObject() || !value["forms"].isObject() || !value["parts"].isObject()) {
        error = "no 'forms'/'parts' sections";
        return false;
    }

    version = value.get("version", 0).asInt();

    const Json::Value &rn = value["renames"];
    if (rn.isObject())
        for (const auto &key: rn.getMemberNames())
            if (rn[key].isString() && !key.empty() && key[0] != '_')
                renames[key] = rn[key].asString();

    const Json::Value &del = value["deleted_forms"];
    if (del.isObject())
        for (const auto &key: del.getMemberNames())
            if (!key.empty() && key[0] != '_')
                deletedForms[key] = del[key].isString() ? del[key].asString() : std::string();

    const Json::Value &fv = value["forms"];
    for (const auto &name: fv.getMemberNames()) {
        const Json::Value &d = fv[name];
        if (!d.isObject())
            continue;
        FormDef f;
        f.group = d.get("group", "functional").asString();
        readList(d["implies_forms"], f.impliesForms);
        readSet(d["parts_add"], f.partsAdd);
        readSet(d["parts_del"], f.partsDel);
        readSet(d["parts_only"], f.partsOnly);
        readSet(d["parts_always"], f.partsAlways);
        readSet(d["parts_del_late"], f.partsDelLate);
        f.lateDelNpcOnly = d.get("late_del_npc_only", false).asBool();
        for (int k = 0; k < BS_MAX; k++) {
            std::string key = bitSetNames[k];
            readSet(d[key + "_add"], f.bitsAdd[k]);
            if (k != BS_ACT && k != BS_OFF)
                readSet(d[key], f.bitsAdd[k]);
            else
                readSet(d[key + "_del"], f.bitsDel[k]);
        }
        readSet(d["wearlocs_add"], f.wearlocsAdd);
        f.noFloat = d.get("no_float", false).asBool();
        f.moveverb = d.get("moveverb", "").asString();
        if (d.isMember("ac_factor") && d["ac_factor"].isNumeric()) {
            f.hasAcFactor = true;
            f.acFactor = d["ac_factor"].asDouble();
        }
        const Json::Value &hv = d["hooves"];
        if (hv.isObject())
            for (const auto &kind: hv.getMemberNames()) {
                auto &p = f.hooves[kind];
                readSet(hv[kind]["parts_add"], p.first);
                readSet(hv[kind]["parts_del"], p.second);
            }
        forms[name] = f;
    }

    const Json::Value &pv = value["parts"];
    for (const auto &name: pv.getMemberNames()) {
        const Json::Value &d = pv[name];
        PartDef p;
        if (d.isObject()) {
            readSet(d["wearlocs"], p.wearlocs);
            readSet(d["sentient_wearlocs"], p.sentientWearlocs);
            readSet(d["npc_wearlocs"], p.npcWearlocs);
            readSet(d["quadruped_wearlocs"], p.quadrupedWearlocs);
            readSet(d["forbids_wearlocs"], p.forbidsWearlocs);
            p.requirePart = d.get("wearlocs_require_part", "").asString();
            readImplies(d["implies"], p.implies);
            readImplies(d["absent_implies"], p.absentImplies);
        }
        parts[name] = p;
    }

    if (value["wearlocs"].isObject())
        readSet(value["wearlocs"]["always"], wearlocsAlways);

    const Json::Value &mv = value["movetype"];
    if (mv.isObject()) {
        const Json::Value &rows = mv["verb_rows"];
        if (rows.isObject())
            for (const auto &name: rows.getMemberNames()) {
                VerbRow r;
                r.base = rows[name].get("base", name).asString();
                r.danger = rows[name].get("danger", "MOVETYPE_NORMAL").asString();
                r.wait = rows[name].get("wait", 1).asInt();
                r.textOnly = rows[name].get("text_only", false).asBool();
                verbRows[name] = r;
            }
        const Json::Value &vb = mv["verb_by_base"];
        if (vb.isObject())
            for (const auto &key: vb.getMemberNames())
                if (vb[key].isString())
                    verbByBase[key] = vb[key].asString();
        readList(mv["form_verb_priority"], formVerbPriority);
    }

    if (value["ac"].isObject() && value["ac"]["default_factor"].isNumeric())
        defaultAcFactor = value["ac"]["default_factor"].asDouble();

    readList(value["size_order"], sizeOrder);

    // A typo here would only surface at area load, as an out_of_range
    // thrown past every catch. Refuse the file instead: the loader keeps the
    // previous config (or none).
    for (auto &f: forms)
        for (auto &g: f.second.impliesForms)
            if (!forms.count(g)) {
                error = "form " + f.first + " implies unknown form " + g;
                return false;
            }
    for (auto &r: renames)
        if (!forms.count(r.second)) {
            error = "rename " + r.first + " -> unknown form " + r.second;
            return false;
        }

    loaded = true;
    return true;
}

int sizeIndex(const Config &cfg, const std::string &size)
{
    auto i = std::find(cfg.sizeOrder.begin(), cfg.sizeOrder.end(), size);
    return i == cfg.sizeOrder.end() ? -1 : (int)(i - cfg.sizeOrder.begin());
}

static std::string join(const NameList &l)
{
    std::string s;
    for (auto &n: l) {
        if (!s.empty())
            s += " ";
        s += n;
    }
    return s;
}

static void addAll(NameSet &s, const NameSet &a)
{
    s.insert(a.begin(), a.end());
}

static void delAll(NameSet &s, const NameSet &d)
{
    for (auto &n: d)
        s.erase(n);
}

static bool contains(const NameList &l, const std::string &n)
{
    return std::find(l.begin(), l.end(), n) != l.end();
}

/** Step 0: rename, drop deleted, expand implies_forms, exclusivity warnings. */
static NameList expandForms(const Config &cfg, const NameList &forms, NameList &warn)
{
    NameList out;

    for (std::string f: forms) {
        auto r = cfg.renames.find(f);
        if (r != cfg.renames.end() && cfg.forms.count(f) == 0) {
            warn.push_back("form " + f + " renamed to " + r->second);
            f = r->second;
        }
        auto d = cfg.deletedForms.find(f);
        if (d != cfg.deletedForms.end()) {
            warn.push_back("form " + f + " deleted: " + d->second);
            continue;
        }
        if (cfg.forms.count(f) == 0) {
            warn.push_back("unknown form " + f + " ignored");
            continue;
        }
        if (!contains(out, f))
            out.push_back(f);
    }

    for (size_t i = 0; i < out.size(); i++) {
        const FormDef &fd = cfg.forms.at(out[i]);
        for (auto &g: fd.impliesForms)
            if (!contains(out, g))
                out.push_back(g);
    }

    NameList lower, upper, full;
    for (auto &f: out) {
        const std::string &g = cfg.forms.at(f).group;
        if (g == "lower") lower.push_back(f);
        else if (g == "upper") upper.push_back(f);
        else if (g == "full") full.push_back(f);
    }

    if (lower.size() > 1)
        warn.push_back("several lower bodies: " + join(lower));
    if (full.size() > 1)
        warn.push_back("several full bodies: " + join(full));
    if (!full.empty()) {
        NameSet own;
        for (auto &f: full)
            for (auto &g: cfg.forms.at(f).impliesForms)
                own.insert(g);
        NameList extra;
        for (auto &f: lower)
            if (!own.count(f)) extra.push_back(f);
        for (auto &f: upper)
            if (!own.count(f)) extra.push_back(f);
        if (!extra.empty())
            warn.push_back("full form " + full[0] + " excludes " + join(extra));
    }

    return out;
}

Result resolve(const Config &cfg, const Input &in)
{
    Result r;
    NameList &warn = r.warnings;

    NameList flist = in.forms;
    for (auto &f: in.proto.formsAdd)
        if (!contains(in.forms, f))
            flist.push_back(f);
    NameSet protoDel(in.proto.formsDel.begin(), in.proto.formsDel.end());
    NameList filtered;
    for (auto &f: flist)
        if (!protoDel.count(f))
            filtered.push_back(f);
    flist = expandForms(cfg, filtered, warn);

    NameSet fs(flist.begin(), flist.end());
    NameList lower, upper, full, func;
    for (auto &f: flist) {
        const std::string &g = cfg.forms.at(f).group;
        if (g == "lower") lower.push_back(f);
        else if (g == "upper") upper.push_back(f);
        else if (g == "full") full.push_back(f);
        else if (g == "functional") func.push_back(f);
    }
    auto F = [&cfg](const std::string &f) -> const FormDef & { return cfg.forms.at(f); };

    // 1. base body
    NameSet &parts = r.parts;
    for (auto &f: full)
        addAll(parts, F(f).partsOnly);
    for (auto &f: lower) {
        addAll(parts, F(f).partsAdd);
        delAll(parts, F(f).partsDel);
    }

    // 2. hooves on the lower body
    if (fs.count("hoofed")) {
        const FormDef &hd = F("hoofed");
        std::string kind = fs.count("quadruped") ? "quadruped" : fs.count("biped") ? "biped" : "";
        if (!kind.empty()) {
            auto h = hd.hooves.find(kind);
            if (h != hd.hooves.end()) {
                addAll(parts, h->second.first);
                delAll(parts, h->second.second);
            }
        } else
            warn.push_back("hoofed without a biped/quadruped lower body grants no hooves");
    }

    // 3. upper body
    for (auto &f: upper)
        addAll(parts, F(f).partsAdd);
    for (auto &f: upper)
        delAll(parts, F(f).partsDel);
    for (auto &f: upper)
        addAll(parts, F(f).partsAlways);

    // 4. functional
    for (auto &f: func)
        addAll(parts, F(f).partsAdd);
    for (auto &f: func)
        delAll(parts, F(f).partsDel);
    for (auto &f: func) {
        if (F(f).lateDelNpcOnly && !in.npc)
            continue; // decision 48: mist keeps a PC body's organs
        delAll(parts, F(f).partsDelLate);
    }

    // 5-6. race, prototype
    addAll(parts, in.race.partsAdd);
    delAll(parts, in.race.partsDel);
    addAll(parts, in.proto.partsAdd);
    delAll(parts, in.proto.partsDel);

    // 7. bits
    NameSet *bits = r.bits;
    if (in.npc)
        bits[BS_ACT].insert("npc");

    auto stage = [&](const NameList &fl) {
        for (auto &f: fl)
            for (int k = 0; k < BS_MAX; k++)
                addAll(bits[k], F(f).bitsAdd[k]);
        for (auto &f: fl) {
            delAll(bits[BS_ACT], F(f).bitsDel[BS_ACT]);
            delAll(bits[BS_OFF], F(f).bitsDel[BS_OFF]);
        }
    };
    NameList fullLower = full;
    fullLower.insert(fullLower.end(), lower.begin(), lower.end());
    stage(fullLower);
    stage(upper);
    stage(func);

    for (auto &pd: cfg.parts) {
        bool has = parts.count(pd.first) > 0;
        const std::map<int, NameSet> &imp = has ? pd.second.implies : pd.second.absentImplies;
        for (auto &e: imp)
            addAll(bits[e.first], e.second);
    }

    int si = sizeIndex(cfg, in.size), li = sizeIndex(cfg, "large");
    bool big = si >= 0 && li >= 0 && si >= li;
    r.rideable = fs.count("quadruped") && (fs.count("hoofed") || big);
    if (r.rideable)
        bits[BS_ACT].insert("rideable");

    for (int k = 0; k < BS_MAX; k++) {
        addAll(bits[k], in.race.bitsAdd[k]);
        delAll(bits[k], in.race.bitsDel[k]);
    }
    for (int k = 0; k < BS_MAX; k++) {
        addAll(bits[k], in.proto.bitsAdd[k]);
        delAll(bits[k], in.proto.bitsDel[k]);
    }

    // 8. wearlocs
    bool sentient = fs.count("sentient") > 0;
    NameSet &W = r.wearlocs;
    for (auto &p: parts) {
        auto pi = cfg.parts.find(p);
        if (pi == cfg.parts.end()) {
            warn.push_back("unknown part " + p);
            continue;
        }
        const PartDef &d = pi->second;
        if (p == "legs" && fs.count("quadruped"))
            addAll(W, d.quadrupedWearlocs);
        else if (!d.requirePart.empty() && !parts.count(d.requirePart))
            ; // decision 41: ring slots need hands
        else
            addAll(W, d.wearlocs);
        if (sentient)
            addAll(W, d.sentientWearlocs);
        if (in.npc)
            addAll(W, d.npcWearlocs);
    }
    for (auto &f: flist)
        addAll(W, F(f).wearlocsAdd);
    addAll(W, cfg.wearlocsAlways);
    for (auto &f: flist)
        if (F(f).noFloat) {
            W.erase("float");
            break;
        }
    for (auto &p: parts) {
        auto pi = cfg.parts.find(p);
        if (pi != cfg.parts.end())
            delAll(W, pi->second.forbidsWearlocs);
    }

    // 9. movetype
    bool hooves = parts.count("two_hooves") || parts.count("four_hooves");
    std::string base;
    if (bits[BS_AFF].count("flying"))
        base = "flying";
    else if (hooves)
        base = "riding";
    else if (!parts.count("legs"))
        base = "slink";
    else
        base = "walk";

    std::string verb;
    auto byBase = [&cfg](const std::string &key, const std::string &def) {
        auto i = cfg.verbByBase.find(key);
        return i == cfg.verbByBase.end() ? def : i->second;
    };
    if (base == "flying")
        verb = "flying";
    else if (base == "riding")
        verb = byBase(fs.count("quadruped") ? "riding_quadruped" : "riding_biped", "riding");
    else {
        verb = base == "slink" ? "slink" : "walk";
        for (auto &f: cfg.formVerbPriority)
            if (fs.count(f) && !F(f).moveverb.empty()) {
                verb = F(f).moveverb;
                break;
            }
        if (!in.moveverb.empty()) // race <moveverb>; never overrides flight, galloping or clattering
            verb = in.moveverb;
    }
    r.movetype = base;
    r.moveverb = verb;
    auto vr = cfg.verbRows.find(verb);
    if (vr != cfg.verbRows.end())
        r.movedanger = vr->second.danger;
    else {
        auto br = cfg.verbRows.find(base);
        r.movedanger = br != cfg.verbRows.end() ? br->second.danger : "MOVETYPE_NORMAL";
        warn.push_back("unknown move verb " + verb);
    }

    // 10. AC
    bool anyFactor = false;
    double factor = 0;
    for (auto &f: flist)
        if (F(f).hasAcFactor) {
            factor = anyFactor ? std::max(factor, F(f).acFactor) : F(f).acFactor;
            anyFactor = true;
        }
    if (!anyFactor)
        factor = cfg.defaultAcFactor;
    r.acFactor = factor;
    r.formAc = std::round((1 + 0.07 * (factor - 3)) * 100.0) / 100.0;

    // 11. derived
    r.forms = fs;
    r.isEdible = !(fs.count("construct") || fs.count("skeletal") || fs.count("mist")
                   || fs.count("instant_decay") || fs.count("magical"));
    r.bloodless = (!parts.count("heart") && !parts.count("cold_blood"))
                  || fs.count("skeletal") || fs.count("construct") || fs.count("mist");
    r.canHoldCards = sentient && parts.count("hands");
    return r;
}

}
