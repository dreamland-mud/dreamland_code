/* $Id$
 *
 * ruffina, 2004
 */
#include "movetypes.h"

#include "pcharacter.h"

#include "merc.h"
#include "def.h"

const char * movedanger_names [] = {
    "moresafe", "safe", "normal", "dangerous", "death"
};

const struct movetype_t movetypes [] = {
 { MOVETYPE_SWIMMING,   MOVETYPE_NORMAL,    1, false, "swimming",  "плыть",    "приплы%1$Gло|л|ла|ли",           "уплы%1$Gло|л|ла|ли",
 },
 { MOVETYPE_WATER_WALK, MOVETYPE_NORMAL,    1, false, "waterwalk", "идти",     "пришлепа%1$Gло|ел|ла|ли по воде", "ушлепа%1$Gло|ел|ла|ли по воде",
 },
 { MOVETYPE_SLINK,      MOVETYPE_MORESAFE,  3, true,  "slink",     "ползти",   "приполз%1$Gло||ла|ли",            "уполз%1$Gло||ла|ли",
 },
 { MOVETYPE_CRAWL,      MOVETYPE_SAFE,      2, true,  "crawl",     "красться", "прокрал%1$Gось|ся|ась|ись",       "прокрал%1$Gось|ся|ась|ись",
 },
 { MOVETYPE_WALK,       MOVETYPE_NORMAL,    1, true,  "normal",    "идти",     "приш%1$Gло|ел|ла|ли",             "уш%1$Gло|ел|ла|ли",
 },
 { MOVETYPE_QUICKLY,    MOVETYPE_DANGEROUS, 1, true,  "quickly",   "быстро",   "быстро приш%1$Gло|ел|ла|ли",      "быстро уш%1$Gло|ел|ла|ли",
 },
 { MOVETYPE_RUNNING,    MOVETYPE_DEATH,     1, true,  "running",   "бежать",   "прибежа%1$Gло|л|ла|ли",           "убежа%1$Gло|л|ла|ли",
 },
 { MOVETYPE_FLEE,       MOVETYPE_DEATH,     1, true,  "flee",      "сбежать",  "сбежа%1$Gло|л|ла|ли",             "сбежа%1$Gло|л|ла|ли",
 },
 { MOVETYPE_RIDING,     MOVETYPE_DANGEROUS, 1, true,  "riding",    "скакать",   "прискака%1$Gло|л|ла|ли",          "ускака%1$Gло|л|ла|ли",
 },
 { MOVETYPE_FLYING,     MOVETYPE_NORMAL,    1, true,  "flying",    "лететь",    "прилете%1$Gло|л|ла|ли",           "улете%1$Gло|л|ла|ли",
 },
 { 0, 0, 0, 0, 0 },
};


int movetype_lookup( const char *argument )
{
    if (argument && argument[0])
        for (int i = 0; movetypes[i].name; i++)
            if (!str_prefix(argument, movetypes[i].name)
                || !str_prefix(argument, movetypes[i].rname))
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

