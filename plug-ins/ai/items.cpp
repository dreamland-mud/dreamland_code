/* $Id$
 *
 * ruffina, 2004
 */
#include "basicmobilebehavior.h"

#include "skillreference.h"
#include "skill.h"
#include "wearloc_utils.h"
#include "room.h"
#include "object.h"
#include "npcharacter.h"

#include "interp.h"
#include "loadsave.h"
#include "merc.h"
#include "def.h"
#include "chargedprops.h"

GSN(none);
GSN(scrolls);
GSN(wands);
GSN(staves);
GSN(staff);

/*
 * MagicItemUsage
 */
enum {
    ARG_OBJ = 1,
    ARG_OBJ_CHAR = 2,
    ARG_CHAR = 3
};

// Wands work from the inventory or any wear slot, no need to hold them.
static const int WEARLOC_ANY = -1;

struct MagicItemUsage {
    int item_type;
    const char *cmd;
    int argType;
    int sn;
    bool fSelfOnly;
    int minValue, maxValue;
    int chargesValue;
    int wearloc;

    bool check( Object *obj, int sn, Character *user, Character *target ) const;
    bool use( Object *obj, Character *user, Character *target ) const;
};

bool MagicItemUsage::check( Object *obj, int spellSN, Character *user, Character *target ) const
{
    if (obj->item_type != item_type)
        return false;

    if (sn != gsn_none && !skillManager->find( sn )->usable( user )) 
        return false;

    if (fSelfOnly && target != user && target != user->fighting) 
        return false;
    
    if (wearloc != WEARLOC_ANY && obj->wear_loc != wearloc)
        if (wearlocationManager->find( wearloc )->canWear( user, obj, F_WEAR_REPLACE ) != RC_WEAR_OK) 
            return false;
    
    if (chargesValue >= 0 && obj->valueByIndex(chargesValue) <= 0) 
        return false;

    for (int i = minValue; i <= maxValue; i++)
        if (obj->valueByIndex(i) == spellSN)
            return true;

    return false;
}

bool MagicItemUsage::use( Object *obj, Character *user, Character *target ) const
{
    DLString args, objName, targetName;
    
    if (!cmd)
        return false;

    if (wearloc != WEARLOC_ANY && obj->wear_loc != wearloc) 
        if (wearlocationManager->find( wearloc )->wear( obj, F_WEAR_REPLACE|F_WEAR_VERBOSE ) != RC_WEAR_OK) 
            return false;

    objName = DLString(obj->getID());
    targetName = DLString(target->getID());

    switch (argType) {
        case ARG_OBJ:
            args = objName;
            break;

        case ARG_CHAR:
            args = targetName;
            break;

        case ARG_OBJ_CHAR:
            args = objName + " " + targetName;
            break;
    }

    return interpret_cmd( user, cmd, "%s", args.c_str( ) );
}

/*
 * MagicItemTable
 */
struct MagicItemTable {
    static Object * find( Object *list, int sn, Character *user, Character *target );
    static bool checkCharged( Object *obj, int sn, Character *user );
    static int findIndex( Object *obj );
    static bool use( Object *obj, Character *user, Character *target );

    static const MagicItemUsage usages [];
};

const MagicItemUsage MagicItemTable::usages [] = {
    { ITEM_PILL,   "eat",      ARG_OBJ,     gsn_none,     true, 1, 4, -1, wear_none },
    { ITEM_POTION, "use",      ARG_OBJ,     gsn_none,     true, 1, 4, -1, wear_none },
    { ITEM_SCROLL, "use",      ARG_OBJ_CHAR,gsn_scrolls, false, 1, 4, -1, wear_none },
    { ITEM_WAND,   "use",      ARG_OBJ_CHAR,gsn_wands,   false, 3, 3,  2, WEARLOC_ANY },
    { ITEM_STAFF,  "use",      ARG_OBJ,     gsn_staves,  false, 3, 3,  2, wear_hold },
    { -1 }
};

Object * MagicItemTable::find( Object *list, int sn, Character *user, Character *target )
{
    for (Object *obj = list; obj; obj = obj->next_content) {
        int i;

        if (!user->can_see( obj ))
            continue;
            
        if (user->getRealLevel( ) < obj->level)
            continue;

        if (obj->item_type != ITEM_WAND && obj->item_type != ITEM_STAFF && charged_props( obj )) {
            if (checkCharged( obj, sn, user ))
                return obj;
            continue;
        }
        
        if (( i = findIndex( obj ) ) >= 0)
            if (usages[i].check( obj, sn, user, target ))
                return obj;
    }

    return NULL;
}

// Charged items (chargedprops.h): fired with 'use', like a wand. A staff
// weapon has to be wielded already: the AI does not swap weapons to cast.
bool MagicItemTable::checkCharged( Object *obj, int spellSN, Character *user )
{
    const Json::Value &c = *charged_props( obj );
    bool wielded = charged_wielded( c );

    if (charged_charges( c ) <= 0)
        return false;

    if (wielded && obj->wear_loc != wear_wield && obj->wear_loc != wear_second_wield)
        return false;

    if (!( wielded ? gsn_staff : gsn_wands )->usable( user ))
        return false;

    Skill *spell = skillManager->find( spellSN );
    return spell && c.isMember( "spell" ) && c["spell"].isString( )
           && spell->getName( ) == c["spell"].asString( );
}

int MagicItemTable::findIndex( Object *obj )
{
    for (int i = 0; usages[i].item_type >= 0; i++) 
        if (usages[i].item_type == obj->item_type)
            return i;

    return -1;
}

bool MagicItemTable::use( Object *obj, Character *user, Character *target )
{
    if (obj->item_type != ITEM_WAND && obj->item_type != ITEM_STAFF && charged_props( obj ))
        return interpret_cmd( user, "use", "%s %s",
                              DLString(obj->getID()).c_str( ), DLString(target->getID()).c_str( ) );

    int i = findIndex( obj );

    if (i < 0)
        return false;
        
    return usages[i].use( obj, user, target );
}


/*
 *  try to zap/brandish/... a magic item with given spell  
 */
bool BasicMobileBehavior::useItemWithSpell( int sn, Character *target )
{
    Object *obj;
    
    obj = MagicItemTable::find( ch->carrying, sn, ch, target );

    if (!obj)
        return false;
    
    return MagicItemTable::use( obj, ch, target );
}

