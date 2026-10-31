/* $Id$
 *
 * Hero item helpers shared by the quest trader and the Fenia API.
 */
#ifndef HEROITEMS_H
#define HEROITEMS_H

#include "lang.h"

class Character;
class PCharacter;

/** The adjectives the quest master engraves on a hero item, indexed
 *  [alignment: good, neutral, evil][gender: neuter, male, female]. Russian and
 *  Ukrainian carry the six-case pad, English has neither case nor gender. The
 *  Ukrainian nouns in limbo have to keep the same gender as the Russian ones,
 *  or the adjective will not agree with them. PersonalNameRepair reads them back
 *  off old objects, so keep the Russian strings exactly as they have always been
 *  written into objects. */
extern const LangText HERO_ADJECTIVES[3][3];

/** The adjective for this character's alignment and the item's gender (SEX_*). */
const LangText & hero_adjective( Character *ch, int gender );

/** Copies of vnum owned by pch in a bank vault they can open: their own cell,
 *  their account's shared cell and their clan's cell. Exact vnum only. */
int hero_vault_count( PCharacter *pch, int vnum );

#endif
