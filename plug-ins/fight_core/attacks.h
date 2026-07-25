/* $Id$
 *
 * ruffina, 2004
 */
#ifndef ATTACKS_H
#define ATTACKS_H

#include "grammar_entities.h"

class DLString;

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

#endif
