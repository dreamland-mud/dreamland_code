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

// enter/leave columns: RU, EN (no gender), UA (gender-inflected %1$G, order
// neuter|masculine|feminine|plural). The direction word is appended separately
// by ExitsMovement::msgOnMove (dirs[] per language).
const struct movetype_t movetypes [] = {
 { MOVETYPE_SWIMMING,   MOVETYPE_NORMAL,    1, false, "swimming",  "плыть",    "",         "приплы%1$Gло|л|ла|ли",           "уплы%1$Gло|л|ла|ли",
   "swims in",                  "swims off",                   "приплив%1$Gло||ла|ли",           "поплив%1$Gло||ла|ли",
 },
 { MOVETYPE_WATER_WALK, MOVETYPE_NORMAL,    1, false, "waterwalk", "идти",     "",         "пришлепа%1$Gло|л|ла|ли по воде", "ушлепа%1$Gло|л|ла|ли по воде",
   "strides in over the water", "strides off over the water",  "прочалапа%1$Gло|в|ла|ли по воді","почалапа%1$Gло|в|ла|ли по воді",
 },
 { MOVETYPE_SLINK,      MOVETYPE_MORESAFE,  3, true,  "slink",     "ползти",   "повзти",   "приполз%1$Gло||ла|ли",            "уполз%1$Gло||ла|ли",
   "slinks in",                 "slinks off",                  "приповз%1$Gло||ла|ли",           "поповз%1$Gло||ла|ли",
 },
 { MOVETYPE_CRAWL,      MOVETYPE_SAFE,      2, true,  "crawl",     "красться", "крастися", "прокрал%1$Gось|ся|ась|ись",       "прокрал%1$Gось|ся|ась|ись",
   "creeps in",                 "creeps off",                  "прокра%1$Gлося|вся|лася|лися",   "прокра%1$Gлося|вся|лася|лися",
 },
 { MOVETYPE_WALK,       MOVETYPE_NORMAL,    1, true,  "normal",    "идти",     "",         "приш%1$Gло|ел|ла|ли",             "уш%1$Gло|ел|ла|ли",
   "arrives",                   "leaves",                      "прийш%1$Gло|ов|ла|ли",           "піш%1$Gло|ов|ла|ли",
 },
 { MOVETYPE_QUICKLY,    MOVETYPE_DANGEROUS, 1, true,  "quickly",   "быстро",   "",         "быстро приш%1$Gло|ел|ла|ли",      "быстро уш%1$Gло|ел|ла|ли",
   "quickly arrives",           "quickly leaves",              "швидко прийш%1$Gло|ов|ла|ли",    "швидко піш%1$Gло|ов|ла|ли",
 },
 { MOVETYPE_RUNNING,    MOVETYPE_DEATH,     1, true,  "running",   "бежать",   "",         "прибежа%1$Gло|л|ла|ли",           "убежа%1$Gло|л|ла|ли",
   "runs in",                   "runs off",                    "прибіг%1$Gло||ла|ли",            "побіг%1$Gло||ла|ли",
 },
 { MOVETYPE_FLEE,       MOVETYPE_DEATH,     1, true,  "flee",      "сбежать",  "",         "сбежа%1$Gло|л|ла|ли",             "сбежа%1$Gло|л|ла|ли",
   "flees in",                  "flees",                       "%1$Gвтекло|втік|втекла|втекли",  "%1$Gвтекло|втік|втекла|втекли",
 },
 { MOVETYPE_RIDING,     MOVETYPE_DANGEROUS, 1, true,  "riding",    "скакать",   "",        "прискака%1$Gло|л|ла|ли",          "ускака%1$Gло|л|ла|ли",
   "rides in",                  "rides off",                   "прискака%1$Gло|в|ла|ли",         "поскака%1$Gло|в|ла|ли",
 },
 { MOVETYPE_FLYING,     MOVETYPE_NORMAL,    1, true,  "flying",    "лететь",    "",        "прилете%1$Gло|л|ла|ли",           "улете%1$Gло|л|ла|ли",
   "flies in",                  "flies off",                   "прилеті%1$Gло|в|ла|ли",          "полеті%1$Gло|в|ла|ли",
 },
 // Text-only rows, mob reform §3.6a. danger/wait/sneak copy the base row
 // (galloping, clattering: riding; trotting, crawling, hopping: walk;
 // slithering, oozing: slink). RU/UA wording reviewed by Kit 2026-10-05.
 { MOVETYPE_GALLOPING,  MOVETYPE_DANGEROUS, 1, true,  "galloping",  "",        "",         "прискака%1$Gло|л|ла|ли",          "ускака%1$Gло|л|ла|ли",
   "gallops in",                "gallops off",                 "прискака%1$Gло|в|ла|ли",         "поскака%1$Gло|в|ла|ли",
 },
 { MOVETYPE_CLATTERING, MOVETYPE_DANGEROUS, 1, true,  "clattering", "",        "",         "прицока%1$Gло|л|ла|ли",           "уцока%1$Gло|л|ла|ли",
   "clatters in",               "clatters off",                "прицока%1$Gло|в|ла|ли",          "поцока%1$Gло|в|ла|ли",
 },
 { MOVETYPE_TROTTING,   MOVETYPE_NORMAL,    1, true,  "trotting",   "",        "",         "притруси%1$Gло|л|ла|ли",          "утруси%1$Gло|л|ла|ли",
   "trots in",                  "trots off",                   "притруси%1$Gло|в|ла|ли",         "потруси%1$Gло|в|ла|ли",
 },
 { MOVETYPE_CRAWLING,   MOVETYPE_NORMAL,    1, true,  "crawling",   "",        "",         "приполз%1$Gло||ла|ли",            "уполз%1$Gло||ла|ли",
   "crawls in",                 "crawls off",                  "приповз%1$Gло||ла|ли",           "поповз%1$Gло||ла|ли",
 },
 { MOVETYPE_SLITHERING, MOVETYPE_MORESAFE,  3, true,  "slithering", "",        "",         "приполз%1$Gло||ла|ли",            "уполз%1$Gло||ла|ли",
   "slithers in",               "slithers off",                "приповз%1$Gло||ла|ли",           "поповз%1$Gло||ла|ли",
 },
 { MOVETYPE_OOZING,     MOVETYPE_MORESAFE,  3, true,  "oozing",     "",        "",         "притек%1$Gло||ла|ли",             "утек%1$Gло||ла|ли",
   "oozes in",                  "oozes off",                   "при%1$Gтекло|тік|текла|текли",   "по%1$Gтекло|тік|текла|текли",
 },
 { MOVETYPE_HOPPING,    MOVETYPE_NORMAL,    1, true,  "hopping",    "",        "",         "прискака%1$Gло|л|ла|ли",          "ускака%1$Gло|л|ла|ли",
   "hops in",                   "hops off",                    "прискака%1$Gло|в|ла|ли",         "поскака%1$Gло|в|ла|ли",
 },
 { 0, 0, 0, 0, 0 },
};


int movetype_lookup( const char *argument )
{
    if (argument && argument[0])
        for (int i = 0; movetypes[i].name && i < MOVETYPE_TEXT_FIRST; i++)
            if (!str_prefix(argument, movetypes[i].name)
                || !str_prefix(argument, movetypes[i].rname)
                || !str_prefix(argument, movetypes[i].uaname))
                return i;
    
    return MOVETYPE_WALK;
}


int movetype_text_lookup( const char *name )
{
    if (name && name[0])
        for (int i = MOVETYPE_TEXT_FIRST; movetypes[i].name; i++)
            if (!strcmp(name, movetypes[i].name))
                return i;

    return -1;
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

