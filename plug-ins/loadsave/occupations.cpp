/* $Id$
 *
 * ruffina, 2004
 */
#include "occupations.h"

#include "room.h"
#include "object.h"
#include "behavior.h"
#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "act.h"
#include "interp.h"
#include "loadsave.h"
#include "merc.h"

#include "def.h"

static const char * occ_names [] = 
{
    "none",
    "shopper",
    "practicer",
    "repairman",
    "quest_trader",
    "quest_master",
    "healer",
    "blacksmith",
    "trainer",
    "clanguard",
    "adept",
    "battlehorse",
    0
};

static int occ_name2type( const DLString &name )
{
    for (int i = 0; occ_names[i]; i++)
        if (name == occ_names[i])
            return i;

    return -1;
}

static const char * occ_type2name(int occType)
{
    if (occType < 0 || occType >= OCC_MAX)
        return 0;

    return occ_names[occType];
}

/**
  This method checks if mob behavior class supports specified occupation.
  Alternatively, it checks for a corresponding property on mob's prototype to exist.
  This alternative check would allow to eventually move from C++ behaviors to Fenia-defined triggers.
*/ 
bool mob_has_occupation( NPCharacter *mob, const DLString &occName )
{
    int occType = occ_name2type( occName );

    if (occType <= 0)
        return false;

    return mob_has_occupation( mob, occType );
}

// Fenia behavior that grants an occupation, when its name differs from the
// occupation's own (behavior names may have spaces, occupation names don't).
static const char * occ_behavior_name( int occType )
{
    if (occType == OCC_CLANGUARD)
        return "clan guard";

    return occ_type2name( occType );
}

bool mob_has_occupation( NPCharacter *mob, int occType )
{
    const char *occName = occ_behavior_name(occType);

    if (!occName)
        return false;

    // Mob behaviors: try to find existing behavior called occName and see if it's assigned
    Behavior *bhv = behaviorManager->findExisting(occName);
    if (bhv && mob->pIndexData->behaviors.isSet(bhv))
        return true;

    // Old-style behavior
    return mob->behavior 
           && IS_SET(mob->behavior->getOccupation( ), (1 << occType) );
}

bool mob_index_is_shopper( MOB_INDEX_DATA *pMob )
{
    Behavior *bhv = behaviorManager->findExisting( "shopper" );
    return bhv && pMob->behaviors.isSet( bhv );
}

// props["shopper"]["stock"]: "3020 3021 104:3", or a bare int for one vnum.
// Tier tokens count by their vnum. Malformed tokens are skipped.
list<int> mob_index_shop_stock( MOB_INDEX_DATA *pMob )
{
    list<int> vnums;

    // isMember first: a non-const operator[] would add an empty key to the proto.
    if (!mob_index_is_shopper( pMob ) || !pMob->props.isObject( ) || !pMob->props.isMember( "shopper" ))
        return vnums;

    const Json::Value &cfg = pMob->props["shopper"];
    if (!cfg.isObject( ) || !cfg.isMember( "stock" ))
        return vnums;

    const Json::Value &stock = cfg["stock"];
    if (stock.isInt( )) {
        if (stock.asInt( ) > 0)
            vnums.push_back( stock.asInt( ) );
        return vnums;
    }
    if (!stock.isString( ))
        return vnums;

    DLString args = stock.asString( );
    DLString token;
    while (!( token = args.getOneArgument( ) ).empty( )) {
        DLString::size_type colon = token.find( ':' );
        DLString vnumStr = colon == DLString::npos ? token : token.substr( 0, colon );
        if (!vnumStr.isNumber( ))
            continue;
        int vnum = vnumStr.toInt( );
        if (vnum > 0)
            vnums.push_back( vnum );
    }
    return vnums;
}

// props["shopper"]["buys"]: item type names, "weapon armor gem".
bool mob_index_shop_buys( MOB_INDEX_DATA *pMob, int itemType )
{
    if (!mob_index_is_shopper( pMob ) || !pMob->props.isObject( ) || !pMob->props.isMember( "shopper" ))
        return false;

    const Json::Value &cfg = pMob->props["shopper"];
    if (!cfg.isObject( ) || !cfg.isMember( "buys" ) || !cfg["buys"].isString( ))
        return false;

    DLString typeName = item_table.name( itemType );
    if (typeName.empty( ))
        return false;

    // Whole words only: is_name would let "key" match "keyring".
    DLString args = cfg["buys"].asString( );
    DLString word;
    while (!( word = args.getOneArgument( ) ).empty( ))
        if (word == typeName)
            return true;
    return false;
}

bool mob_has_behavior(NPCharacter *mob, BehaviorReference& bhv)
{
    if (mob->pIndexData->behaviors.isSet(bhv))
        return true;

    return false;
}

NPCharacter * find_attracted_mob( Character *ch, int occType )
{
    NPCharacter *mob = NULL;
    
    for (Character *rch = ch->in_room->people; rch; rch = rch->next_in_room)
        if (( mob = rch->getNPC( ) )
            && mob->behavior 
            && IS_SET(mob->behavior->getOccupation( ), (1 << occType)))
        {
            return mob;
        }

    return NULL;
}

