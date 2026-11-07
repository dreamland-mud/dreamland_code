/*
 * Random extra defenses of champion, boss and legend prototypes
 * (docs/systems/MOB_DEFENSES_AND_LEGENDS.md D4).
 */
#ifndef MOBEXTRAS_H
#define MOBEXTRAS_H

struct mob_index_data;

/** Rebuild a prototype from its authored data: resolveBody(), deriveNumbers(),
 *  then the tier's random extras. Every index (re)build goes through here. */
void mob_index_rebuild(mob_index_data *pMobIndex);

/** The tier's random extra res/imm/vuln on top of the resolved body (fight/mob_tiers.json
 *  random_res, random_imm_pct, random_vuln, random_pool). Fixed per vnum and calendar
 *  month. Never a bit the body already has, never the damage class of the mob's own
 *  attacks, never for a unique-race or a reviewed (imm, res or vuln) prototype. */
void mob_index_random_extras(mob_index_data *pMobIndex);

#endif
