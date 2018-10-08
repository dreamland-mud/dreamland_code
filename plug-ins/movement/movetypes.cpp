/* $Id$
 *
 * ruffina, 2004
 */
#include "movetypes.h"

#include "pcharacter.h"

#include "mercdb.h"
#include "def.h"

const char * movedanger_names [] = {
    "moresafe", "safe", "normal", "dangerous", "death"
};

const struct movetype_t movetypes [] = {
 { MOVETYPE_SWIMMING,   MOVETYPE_NORMAL,    1, false, "swimming",  "приплы%1$Gло|л|ла",        "уплы%1$Gло|л|ла",
 },
 { MOVETYPE_WATER_WALK, MOVETYPE_NORMAL,    1, false, "waterwalk", "приш%1$Gло|ел|ла по воде", "уш%1$Gло|ел|ла по воде",
 },
 { MOVETYPE_SLINK,      MOVETYPE_MORESAFE,  3, true,  "slink",     "припол%1$Gзло|з|зла",      "упол%1$Gзло|з|зла",
 },
 { MOVETYPE_CRAWL,      MOVETYPE_SAFE,      2, true,  "crawl",     "прокрал%1$Gось|ся|ась",    "прокрал%1$Gось|ся|ась",
 },
 { MOVETYPE_WALK,       MOVETYPE_NORMAL,    1, true,  "normal",    "приш%1$Gло|ел|ла",         "уш%1$Gло|ел|ла",
 },
 { MOVETYPE_QUICKLY,    MOVETYPE_DANGEROUS, 1, true,  "quickly",   "еще нету",                 "еще нету",
 },
 { MOVETYPE_RUNNING,    MOVETYPE_DEATH,     1, true,  "running",   "прибежа%1$Gло|л|ла",       "убежа%1$Gло|л|ла",
 },
 { MOVETYPE_FLEE,       MOVETYPE_DEATH,     1, true,  "flee",      "прибежа%1$Gло|л|ла",       "сбежа%1$Gло|л|ла",
 },
 { MOVETYPE_RIDING,     MOVETYPE_DANGEROUS, 1, true,  "riding",    "прискака%1$Gло|л|ла",      "ускака%1$Gло|л|ла",
 },
 { MOVETYPE_FLYING,     MOVETYPE_NORMAL,    1, true,  "flying",    "прилете%1$Gло|л|ла",       "улете%1$Gло|л|ла",
 },
 { 0, 0, 0, 0 },
};


int movetype_lookup( const char *argument )
{
    if (argument && argument[0])
        for (int i = 0; movetypes[i].name; i++)
            if (!str_cmp( argument, movetypes[i].name ))
                return i;
    
    return MOVETYPE_WALK;
}


int movetype_resolve( Character *ch, const char *argument )
{
    int movetype;
    
    if (argument == NULL || argument[0] == 0)
        movetype = MOVETYPE_WALK;
    else if (!ch->is_npc( ) && ch->getPC( )->getAttributes( ).isAvailable( "speedwalk" ))
        movetype = MOVETYPE_RUNNING;
    else
        movetype = movetype_lookup( argument );
    
    return movetype;
}

