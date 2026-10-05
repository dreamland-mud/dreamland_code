/*
 * Mob reform: the nine body bit sets of a mob, as indexes into the per-set
 * arrays of mob_index_data and NPCharacter. Own header so npcharacter.h and
 * mobilefactory.h can both see it without an include cycle.
 */
#ifndef MOBSETS_H
#define MOBSETS_H

enum {
    MOBSET_ACT = 0, MOBSET_OFF, MOBSET_AFF, MOBSET_DET, MOBSET_IMM, MOBSET_RES, MOBSET_VULN,
    MOBSET_FORM, MOBSET_PARTS, MOBSET_MAX
};

#endif
