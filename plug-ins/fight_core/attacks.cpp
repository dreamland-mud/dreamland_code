/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
 * Все права на этот код 'Dream Land' пренадлежат Igor {Leo} и Olga {Varda}*
 * Некоторую помощь в написании этого кода, а также своими идеями помогали:*
 *    Igor S. Petrenko     {NoFate, Demogorgon}                            *
 *    Koval Nazar          {Nazar, Redrum}                                 *
 *    Doropey Vladimir     {Reorx}                                         *
 *    Kulgeyko Denis       {Burzum}                                        *
 *    Andreyanov Aleksandr {Manwe}                                         *
 *    и все остальные, кто советовал и играл в этот MUD                    *
 ***************************************************************************/
/***************************************************************************
 *     ANATOLIA 2.1 is copyright 1996-1997 Serdar BULUT, Ibrahim CANPUNAR  *        
 *     ANATOLIA has been brought to you by ANATOLIA consortium                   *
 *         Serdar BULUT {Chronos}                bulut@rorqual.cc.metu.edu.tr       *
 *         Ibrahim Canpunar  {Asena}        canpunar@rorqual.cc.metu.edu.tr    *        
 *         Murat BICER  {KIO}                mbicer@rorqual.cc.metu.edu.tr           *        
 *         D.Baris ACAR {Powerman}        dbacar@rorqual.cc.metu.edu.tr           *        
 *     By using this code, you have agreed to follow the terms of the      *
 *     ANATOLIA license, in the file Anatolia/anatolia.licence             *        
 ***************************************************************************/
/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*        ROM 2.4 is copyright 1993-1995 Russ Taylor                           *
*        ROM has been brought to you by the ROM consortium                   *
*            Russ Taylor (rtaylor@pacinfo.com)                                   *
*            Gabrielle Taylor (gtaylor@pacinfo.com)                           *
*            Brian Moore (rom@rom.efn.org)                                   *
*        By using this code, you have agreed to follow the terms of the           *
*        ROM license, in the file Rom24/doc/rom.license                           *
***************************************************************************/
#include "jsoncpp/json/json.h"
#include "configurable.h"
#include "attacks.h"
#include "damageflags.h"
#include "immunity.h"
#include "mobilefactory.h"
#include "race.h"
#include "grammar_entities_impl.h"
#include "merc.h"
#include "def.h"

using namespace Grammar;

/* attack table */
struct attack_type        attack_table        []                =
{
    { "none",                "удар",                        -1,                MultiGender::MASCULINE },  /*  0 */
    { "slice",                "разрезающий удар",        DAM_SLASH,        MultiGender::MASCULINE },        
    { "stab",                "выпад",                DAM_PIERCE,        MultiGender::MASCULINE },
    { "slash",                "рубящий удар",                DAM_SLASH,        MultiGender::MASCULINE },
    { "whip",                "хлесткий удар",        DAM_SLASH,        MultiGender::MASCULINE },
    { "claw",                "удар когтями",                DAM_SLASH,        MultiGender::MASCULINE },  /*  5 */
    { "blast",                "залп",                DAM_BASH,        MultiGender::MASCULINE },
    { "pound",                "тяжелый удар",                DAM_BASH,        MultiGender::MASCULINE },
    { "crush",                "дробящий удар",        DAM_BASH,        MultiGender::MASCULINE },
    { "grep",                "захват",                 DAM_SLASH,        MultiGender::MASCULINE },
    { "bite",                "укус",                        DAM_PIERCE,        MultiGender::MASCULINE },  /* 10 */
    { "pierce",                "глубокий выпад",        DAM_PIERCE,        MultiGender::MASCULINE },
    { "suction",        "засасывание",    DAM_DROWNING,        MultiGender::NEUTER },
    { "beating",        "серия ударов",                DAM_BASH,        MultiGender::FEMININE },
    { "digestion",        "кислотная слизь",        DAM_ACID,        MultiGender::FEMININE },
    { "charge",                "удар с разбегу",                DAM_BASH,        MultiGender::MASCULINE},  /* 15 */
    { "slap",                "шлепок",                DAM_BASH,        MultiGender::MASCULINE },
    { "punch",                "удар кулаком",                DAM_BASH,        MultiGender::MASCULINE },
    { "wrath",                "гнев",                  DAM_ENERGY,        MultiGender::MASCULINE },
    { "magic",                "магический удар",        DAM_ENERGY,        MultiGender::MASCULINE },
    { "divine",                "божественная энергия",        DAM_HOLY,        MultiGender::FEMININE},  /* 20 */
    { "cleave",                "раскалывающий удар",        DAM_SLASH,        MultiGender::MASCULINE },
    { "scratch",        "царапающий удар",        DAM_PIERCE,        MultiGender::MASCULINE },
    { "peck",                "удар клювом",                DAM_PIERCE,        MultiGender::MASCULINE },
    { "peckb",                "клюющий удар",                DAM_PIERCE,        MultiGender::MASCULINE },
    { "chop",                "рубящий удар",                DAM_SLASH,        MultiGender::MASCULINE },  /* 25 */
    { "sting",                "жалящий удар",                DAM_PIERCE,        MultiGender::MASCULINE },
    { "smash",                "разбивающий удар",        DAM_BASH,        MultiGender::MASCULINE },
    { "shbite",                "шокирующий укус",      DAM_LIGHTNING,        MultiGender::MASCULINE },
    { "flbite",                "обжигающий укус",        DAM_FIRE,        MultiGender::MASCULINE },
    { "frbite",                "леденящий укус",       DAM_COLD,        MultiGender::MASCULINE },  /* 30 */
    { "acbite",                "окисляющий укус",      DAM_ACID,        MultiGender::MASCULINE },
    { "chomp",                "грызущий удар",                DAM_PIERCE,        MultiGender::MASCULINE },
    { "drain",                "темная энергия",DAM_NEGATIVE,        MultiGender::FEMININE },
    { "thrust",                "выпад",                DAM_PIERCE,        MultiGender::MASCULINE },
    { "slime",                "жижа",        DAM_DROWNING,        MultiGender::FEMININE },
    { "shock",                "разряд",                DAM_LIGHTNING,        MultiGender::MASCULINE },
    { "thwack",                "удар с размаху",        DAM_BASH,        MultiGender::MASCULINE },
    { "flame",                "вспышка",                DAM_LIGHT,        MultiGender::FEMININE},
    { "chill",                "холод",                DAM_COLD,        MultiGender::MASCULINE },
    { "cuff",                "подзатыльник",                DAM_BASH,        MultiGender::MASCULINE },
    { "hooves",                "удар копытами",        DAM_BASH,        MultiGender::MASCULINE },
    { "horns",                "удар рогами",                DAM_BASH,        MultiGender::MASCULINE },
    { "spines",         "удар иголками",              DAM_PIERCE,         MultiGender::MASCULINE }, 
    { "cacophony",      "какофония",              DAM_SOUND,      MultiGender::FEMININE}, 
    { "poisonbite",      "отравляющий укус",              DAM_POISON,      MultiGender::MASCULINE}, 
    { "tearbite",      "разрывающий укус",              DAM_SLASH,      MultiGender::MASCULINE},
    { "mental",      "ментальный удар",              DAM_MENTAL,      MultiGender::MASCULINE},
    { "disease",      "чумные миазмы",              DAM_DISEASE,      MultiGender::PLURAL},
    { "charm",      "неотразимость",              DAM_CHARM,      MultiGender::FEMININE},
    { "sound",      "звуковая волна",              DAM_SOUND,      MultiGender::FEMININE},
    { "scorch",      "опаляющий удар",              DAM_FIRE,      MultiGender::MASCULINE},
    { "toxin",      "ядовитый удар",              DAM_POISON,      MultiGender::MASCULINE},
    { 0,                0,                        0                }
};

/*
 * Trilingual attack nouns (Trello 2594). The en/ua display forms live in
 * config/fight/attack_nouns.json, indexed 1:1 with attack_table above; the RU
 * form stays baked into attack_table[].noun (byte-identical). Loaded here in
 * fight_core so both the combat message path (onehit_undef) and the Fenia
 * ObjectWrapper bindings (identify/lore) share one source.
 */
struct AttackNounRow {
    DLString en, ua;
    void fromJson(const Json::Value &v)
    {
        en = v["en"].asString();
        ua = v["ua"].asString();
    }
};
static json_vector<AttackNounRow> attackNouns;

CONFIGURABLE_LOADED(fight, attack_nouns)
{
    attackNouns.fromJson(value);
}

const DLString & attack_noun_en(int index)
{
    if (index >= 0 && index < (int)attackNouns.size())
        return attackNouns[index].en;
    return DLString::emptyString;
}

const DLString & attack_noun_ua(int index)
{
    if (index >= 0 && index < (int)attackNouns.size())
        return attackNouns[index].ua;
    return DLString::emptyString;
}


int attack_damage(int attack)
{
    static int size = -1;

    if (size < 0)
        for (size = 0; attack_table[size].name; size++)
            ;

    return (attack >= 0 && attack < size) ? attack_table[attack].damage : DAM_NONE;
}

/* Element words a builder puts in a coloured monster's name, by damage class. */
static int attack_element_word(const DLString &word)
{
    static const struct { const char *word; int damage; } words[] = {
        { "red", DAM_FIRE }, { "brass", DAM_FIRE }, { "gold", DAM_FIRE }, { "golden", DAM_FIRE },
        { "fire", DAM_FIRE }, { "flame", DAM_FIRE },
        { "white", DAM_COLD }, { "silver", DAM_COLD }, { "ice", DAM_COLD }, { "frost", DAM_COLD },
        { "cold", DAM_COLD },
        { "blue", DAM_LIGHTNING }, { "bronze", DAM_LIGHTNING }, { "storm", DAM_LIGHTNING },
        { "cloud", DAM_LIGHTNING }, { "lightning", DAM_LIGHTNING },
        { "black", DAM_ACID }, { "copper", DAM_ACID }, { "shadow", DAM_ACID }, { "acid", DAM_ACID },
        { "green", DAM_POISON }, { "poison", DAM_POISON },
        { 0, DAM_NONE }
    };

    for (int i = 0; words[i].word; i++)
        if (word == words[i].word)
            return words[i].damage;
    return DAM_NONE;
}

/* Damage class of a breath spec, DAM_NONE for any other spec. */
static int attack_breath_spec(const DLString &spec)
{
    if (spec == "spec_breath_fire")      return DAM_FIRE;
    if (spec == "spec_breath_frost")     return DAM_COLD;
    if (spec == "spec_breath_lightning") return DAM_LIGHTNING;
    if (spec == "spec_breath_acid")      return DAM_ACID;
    if (spec == "spec_breath_gas")       return DAM_POISON;
    return DAM_NONE;
}

static bool attack_all_word(const DLString &word)
{
    return word == "prismatic" || word == "rainbow" || word == "chromatic" || word == "tiamat";
}

/* Lowercase Latin words of a text, colour codes stripped. */
static std::vector<DLString> attack_name_words(const DLString &text)
{
    std::vector<DLString> result;
    DLString plain = text.colourStrip();
    DLString word;

    for (size_t i = 0; i <= plain.size(); i++) {
        char c = i < plain.size() ? plain[i] : ' ';
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
            word += (char)(c >= 'A' && c <= 'Z' ? c - 'A' + 'a' : c);
        } else if (!word.empty()) {
            result.push_back(word);
            word.clear();
        }
    }
    return result;
}

static int attack_member_of_class(const std::vector<int> &group, int damage)
{
    if (damage == DAM_NONE)
        return -1;
    for (size_t i = 0; i < group.size(); i++)
        if (attack_damage(group[i]) == damage)
            return (int)i;
    return -1;
}

std::vector<int> mob_index_attacks(const mob_index_data *pMobIndex)
{
    std::vector<int> result;
    Race *race = raceManager->find(pMobIndex->race);
    std::vector<DLString> words = attack_name_words(pMobIndex->short_descr.get(LANG_EN));
    std::vector<DLString> kw = attack_name_words(pMobIndex->keyword.get(LANG_EN));
    words.insert(words.end(), kw.begin(), kw.end());

    // The builder's breath spec says the element outright. spec_breath_any
    // defers to a colour word in the name and keeps every member without one.
    int breath = attack_breath_spec(pMobIndex->spec_fun.name);
    bool breathAny = pMobIndex->spec_fun.name == "spec_breath_any";
    bool all = false;
    for (auto &w: words)
        if (attack_all_word(w))
            all = true;

    // Own immunity only: the race default (dragons: fire) says nothing about this one.
    bitstring_t ownImm = pMobIndex->bodyAdd[MOBSET_IMM] & ~race->getImm().getValue();

    for (auto &group: race->getAttacks()) {
        if (group.size() == 1) {
            result.push_back(group.front());
            continue;
        }

        int pick = attack_member_of_class(group, breath);

        for (size_t i = 0; pick < 0 && !all && i < words.size(); i++)
            pick = attack_member_of_class(group, attack_element_word(words[i]));

        if (pick < 0 && (all || breathAny)) {
            result.insert(result.end(), group.begin(), group.end());
            continue;
        }

        for (size_t i = 0; pick < 0 && i < group.size(); i++) {
            bitstring_t bit = immune_bit(attack_damage(group[i]));
            if (bit && (ownImm & bit))
                pick = (int)i;
        }

        for (size_t i = 0; pick < 0 && i < group.size(); i++)
            if (group[i] == pMobIndex->dam_type)
                pick = (int)i;

        if (pick < 0)
            pick = attack_member_of_class(group, attack_damage(pMobIndex->dam_type));

        if (pick < 0) {
            // Knuth multiplicative hash: neighbouring vnums don't step through the group in order.
            unsigned int h = (unsigned int)pMobIndex->vnum * 2654435761u;
            pick = (int)((h >> 16) % group.size());
        }

        result.push_back(group[pick]);
    }

    return result;
}
