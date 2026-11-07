/* $Id$
 *
 * ruffina, 2004
 */
#ifndef ATTACKS_H
#define ATTACKS_H

#include <vector>
#include "grammar_entities.h"

class DLString;
struct mob_index_data;

struct attack_type
{
    const char *        name;                        /* name */
    const char *        noun;                        /* message */
    int           damage;                        /* damage class */
    Grammar::MultiGender gender;        /* grammatical gender of russian noun */
};
extern struct attack_type        attack_table        [];

/* Trilingual attack nouns: the RU form stays in attack_table[].noun; the en/ua
 * display forms come from config/fight/attack_nouns.json, indexed 1:1 with
 * attack_table. Returns an empty string when the index is out of range or the
 * cell is unset (caller falls back to the RU noun). */
const DLString & attack_noun_en(int index);
const DLString & attack_noun_ua(int index);

/* Damage class (DAM_*) of an attack (weapon_flags value), DAM_NONE when out of range. */
int attack_damage(int attack);

/* A prototype's concrete attacks: its race attack slots with every 'a|b|c'
 * group resolved to one member, deterministically. In order: the breath spec
 * (spec_breath_fire/frost/lightning/acid/gas), an element word in the EN
 * short description or keywords (prismatic, rainbow, chromatic and tiamat
 * keep every member; with spec_breath_any and no word, every member too),
 * the prototype's own element immunity (race
 * defaults excluded), its authored dam_type, a vnum hash. */
std::vector<int> mob_index_attacks(const mob_index_data *pMobIndex);

#endif
