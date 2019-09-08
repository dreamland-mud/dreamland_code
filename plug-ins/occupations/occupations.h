/* $Id$
 *
 * ruffina, 2004
 */
#ifndef OCCUPATIONS_H
#define OCCUPATIONS_H

class NPCharacter;
class Object;
class DLString;

enum {
    OCC_NONE = 0,
    OCC_SHOPPER,
    OCC_PRACTICER,
    OCC_REPAIRMAN,
    OCC_QUEST_TRADER,
    OCC_QUEST_MASTER,
    OCC_HEALER,
    OCC_SMITHMAN,
    OCC_TRAINER,
    OCC_CLANGUARD,
    OCC_MAX,
};

bool mob_has_occupation( NPCharacter *, const char * );
bool mob_has_occupation( NPCharacter *, int );
bool obj_has_trigger( Object *, const DLString & );
bool obj_is_special(Object *obj);

#endif
