#ifndef ITEMVALUE_H
#define ITEMVALUE_H

/*
 * One item value model (config/fight/item_value.json), shared by the gear advisor
 * (characterwrapper.cpp ga_*), the random item generator and Fenia's item measure
 * (.config("fight/item_value")). Sections: "melee"/"caster" (per-profile stat
 * weights), "shared" (profile-independent weights), "flags" ([melee, caster] per
 * affect flag), "res" ([res, imm, vuln] per damage class), "measure" (Fenia-side).
 *
 * item_value() returns the configured number, or `def` when the file, section or
 * key is missing -- callers pass today's constant as `def`, so a missing or partial
 * file keeps the old behaviour. idx >= 0 picks an element of an array value.
 */
double item_value(const char *section, const char *key, double def, int idx = -1);

#endif
