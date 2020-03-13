/* $Id$
 *
 * ruffina, 2004
 */
#ifndef ATTACKS_H
#define ATTACKS_H

#include "grammar_entities.h"

struct attack_type
{
    const char *        name;                        /* name */
    const char *        noun;                        /* message */
    int           damage;                        /* damage class */
    Grammar::MultiGender gender;        /* grammatical gender of russian noun */
};
extern struct attack_type        attack_table        [];

#endif
