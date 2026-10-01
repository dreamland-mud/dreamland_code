/*
 * Unit test for the mob body resolver (src/core/body.cpp).
 *
 * Re-resolves every case of the shared fixtures that scripts/body-resolve.py
 * (dreamland_tools) generated and checks the C++ result field by field. The
 * fixtures and the mob_forms.json they were made from are copied into
 * src/core/tests/data/; refresh both together whenever the Python twin changes.
 *
 * Not part of the default build (EXTRA_PROGRAMS). Build and run by hand:
 *     make -C src/core body_test && src/core/body_test src/core/tests/data
 * or standalone, with only jsoncpp:
 *     g++ -std=c++17 -I src/core src/core/body.cpp src/core/mobtiers.cpp src/core/body_test.cpp \
 *         -ljsoncpp -o body_test && ./body_test src/core/tests/data
 */
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>

#include "body.h"
#include "mobtiers.h"

using namespace std;
using namespace Body;

static bool readJson(const string &path, Json::Value &out)
{
    ifstream ifs(path.c_str());
    if (!ifs) {
        cerr << "cannot open " << path << endl;
        return false;
    }
    Json::CharReaderBuilder b;
    string errs;
    if (!Json::parseFromStream(b, ifs, &out, &errs)) {
        cerr << path << ": " << errs << endl;
        return false;
    }
    return true;
}

static NameList listOf(const Json::Value &v)
{
    NameList l;
    if (v.isArray())
        for (const auto &e: v)
            l.push_back(e.asString());
    return l;
}

static NameSet setOf(const Json::Value &v)
{
    NameList l = listOf(v);
    return NameSet(l.begin(), l.end());
}

static void readMods(const Json::Value &v, Mods &m)
{
    if (!v.isObject())
        return;
    m.formsAdd = listOf(v["forms_add"]);
    m.formsDel = listOf(v["forms_del"]);
    m.partsAdd = setOf(v["parts_add"]);
    m.partsDel = setOf(v["parts_del"]);
    for (int k = 0; k < BS_MAX; k++) {
        string key = bitSetNames[k];
        m.bitsAdd[k] = setOf(v[key + "_add"]);
        m.bitsDel[k] = setOf(v[key + "_del"]);
    }
}

static string str(const NameSet &s)
{
    ostringstream o;
    o << "[";
    for (auto &n: s)
        o << " " << n;
    o << " ]";
    return o.str();
}

static int failures = 0;

static void expectSet(const string &name, const char *field, const NameSet &got, const Json::Value &exp)
{
    NameSet want = setOf(exp);
    if (got != want) {
        cout << "DRIFT " << name << " " << field << ": want " << str(want) << " got " << str(got) << endl;
        failures++;
    }
}

static void expectStr(const string &name, const char *field, const string &got, const Json::Value &exp)
{
    if (got != exp.asString()) {
        cout << "DRIFT " << name << " " << field << ": want " << exp.asString() << " got " << got << endl;
        failures++;
    }
}

static void expectBool(const string &name, const char *field, bool got, const Json::Value &exp)
{
    if (got != exp.asBool()) {
        cout << "DRIFT " << name << " " << field << ": want " << exp.asBool() << " got " << got << endl;
        failures++;
    }
}

static void expectNum(const string &name, const char *field, double got, const Json::Value &exp)
{
    if (fabs(got - exp.asDouble()) > 0.0051) {
        cout << "DRIFT " << name << " " << field << ": want " << exp.asDouble() << " got " << got << endl;
        failures++;
    }
}

int main(int argc, char **argv)
{
    string dir = argc > 1 ? argv[1] : "src/core/tests/data";
    Json::Value forms;
    if (!readJson(dir + "/mob_forms.json", forms))
        return 2;

    Config cfg;
    string error;
    if (!cfg.fromJson(forms, error)) {
        cerr << "mob_forms.json: " << error << endl;
        return 2;
    }

    // An empty or broken config must load as 'not loaded', never crash.
    Config empty;
    if (empty.fromJson(Json::Value(), error) || empty.loaded) {
        cout << "FAIL empty config reported as loaded" << endl;
        failures++;
    }

    // A typo in implies_forms or a rename target refuses the whole file.
    Json::Value bad = forms;
    bad["forms"]["dragon"]["implies_forms"].append("wingedd");
    if (empty.fromJson(bad, error)) {
        cout << "FAIL unknown implies_forms target accepted" << endl;
        failures++;
    }
    bad = forms;
    bad["renames"]["centaur"] = "quadrupedd";
    if (empty.fromJson(bad, error)) {
        cout << "FAIL unknown rename target accepted" << endl;
        failures++;
    }

    int cases = 0;
    for (const char *file: { "archetypes.json", "races.json" }) {
        Json::Value fx;
        if (!readJson(dir + "/" + file, fx))
            return 2;

        for (const auto &c: fx["cases"]) {
            cases++;
            string name = string(file) + ":" + c["name"].asString();
            const Json::Value &i = c["input"];
            const Json::Value &e = c["expected"];

            Input in;
            in.forms = listOf(i["forms"]);
            in.size = i.get("size", "medium").asString();
            in.npc = i.get("npc", true).asBool();
            in.moveverb = i.get("moveverb", "").asString();
            readMods(i["race"], in.race);
            readMods(i["proto"], in.proto);

            Result r = resolve(cfg, in);

            expectSet(name, "forms", r.forms, e["forms"]);
            expectSet(name, "parts", r.parts, e["parts"]);
            expectSet(name, "wearlocs", r.wearlocs, e["wearlocs"]);
            for (int k = 0; k < BS_MAX; k++)
                expectSet(name, bitSetNames[k], r.bits[k], e[bitSetNames[k]]);
            expectStr(name, "movetype", r.movetype, e["movetype"]);
            expectStr(name, "moveverb", r.moveverb, e["moveverb"]);
            expectStr(name, "movedanger", r.movedanger, e["movedanger"]);
            expectNum(name, "ac_factor", r.acFactor, e["ac_factor"]);
            expectNum(name, "form_ac", r.formAc, e["form_ac"]);
            expectBool(name, "isEdible", r.isEdible, e["isEdible"]);
            expectBool(name, "canHoldCards", r.canHoldCards, e["canHoldCards"]);
            expectBool(name, "rideable", r.rideable, e["rideable"]);
            expectBool(name, "bloodless", r.bloodless, e["bloodless"]);

            // Warnings: same multiset (Python iterates a set for unknown parts).
            NameList want = listOf(e["warnings"]), got = r.warnings;
            sort(want.begin(), want.end());
            sort(got.begin(), got.end());
            if (want != got) {
                cout << "DRIFT " << name << " warnings: want " << want.size() << " got " << got.size() << endl;
                for (auto &w: got)
                    cout << "    got: " << w << endl;
                failures++;
            }
        }
    }

    // Tier curve: the sample file documents the schema, check the arithmetic.
    Json::Value tj;
    if (readJson(dir + "/mob_tiers.sample.json", tj)) {
        MobTiers::Config tc;
        if (!tc.fromJson(tj, error)) {
            cout << "FAIL mob_tiers.sample.json: " << error << endl;
            failures++;
        } else {
            MobTiers::MobInfo m;
            m.level = 30; m.tier = tc.parse("normal"); m.sentient = true;
            MobTiers::Centres c = tc.centres(m);
            // level 30 sits halfway between the 25 and 35 points: (550+1225)/2
            if (c.hp != 888 || c.mana != 150 || c.hitrollBonus != 0 || c.wealth != 700) {
                cout << "FAIL tier centres L30 normal: hp " << c.hp << " mana " << c.mana
                     << " hitroll " << c.hitrollBonus << " wealth " << c.wealth << endl;
                failures++;
            }
            m.tier = tc.parse("boss"); m.acts.insert("mage"); m.sentient = false;
            c = tc.centres(m);
            if (c.hp != (int)lround(887.5 * 7.0 * 0.85) || c.wealth != 0 || c.hitrollBonus != 10) {
                cout << "FAIL tier centres L30 boss mage: hp " << c.hp << " wealth " << c.wealth
                     << " hitroll " << c.hitrollBonus << endl;
                failures++;
            }
            if (tc.parse("6") != 6 || fabs(tc.get(6).hp - 1.4) > 0.001 || tc.parse("nonsense") != 0
                    || tc.offCount(7, 3) != 2 || tc.offCount(7, 4) != 1 || tc.offCount(1, 4) != -1) {
                cout << "FAIL tier parse/interpolation/offCount" << endl;
                failures++;
            }
        }
    }

    cout << cases << " fixtures, " << failures << " drift" << endl;
    return failures == 0 && cases > 0 ? 0 : 1;
}
