/* $Id$
 *
 * ruffina, 2004
 */


#include "logstream.h"
#include "descriptor.h"
#include "pcharacter.h"

#include "wiznet.h"
#include "act.h"
#include "merc.h"

#include "def.h"


/* wiznet table and prototype for future flag setting */
const struct wiznet_type wiznet_table [] =
{
    { "on",                      WIZ_ON,                        AVATAR },
    { "prefix",                  WIZ_PREFIX,                AVATAR },
    { "ticks",                   WIZ_TICKS,                AVATAR },
    { "logins",                  WIZ_LOGINS,                AVATAR },
    { "sites",                   WIZ_SITES,                GOD },
    { "links",                   WIZ_LINKS,                ANGEL },
    { "newbies",                 WIZ_NEWBIE,                AVATAR },
    { "spam",                    WIZ_SPAM,                IMMORTAL },
    { "deaths",                  WIZ_DEATHS,                AVATAR },
    { "resets",                  WIZ_RESETS,                GOD },
    { "mobdeaths",               WIZ_MOBDEATHS,                GOD },
    { "flags",                   WIZ_FLAGS,                AVATAR },
    { "penalties",               WIZ_PENALTIES,                IMMORTAL },
    { "saccing",                 WIZ_SACCING,                IMMORTAL },
    { "levels",                  WIZ_LEVELS,                AVATAR },
    { "load",                    WIZ_LOAD,                SUPREME },
    { "restore",                 WIZ_RESTORE,                SUPREME },
    { "snoops",                  WIZ_SNOOPS,                SUPREME },
    { "switches",                WIZ_SWITCHES,                SUPREME },
    { "secure",                  WIZ_SECURE,                CREATOR },
    { "damages",                 WIZ_DAMAGES,                AVATAR },
    { "rnames",                  WIZ_RNAMES,                AVATAR },
    { "confirm",                 WIZ_CONFIRM,                AVATAR },
    { "quest",                   WIZ_QUEST,                AVATAR },
    { "language",                WIZ_LANGUAGE,                AVATAR},
    { "skills",                  WIZ_SKILLS,                AVATAR },
    { "religion",                WIZ_RELIGION,                AVATAR },
    {        0, 0, 0 }
};

/* returns a flag for wiznet */
long wiznet_lookup (const char *name)
{
    int flag;

    for (flag = 0; wiznet_table[flag].name != 0; flag++)
    {
        if (dl_tolower(name[0]) == dl_tolower(wiznet_table[flag].name[0])
              && !str_prefix(name,wiznet_table[flag].name))
            return flag;
    }

    return -1;
}

const char * wiznet_name( int flag )
{
    for (int i = 0; wiznet_table[i].name != 0; i++)
        if (wiznet_table[i].flag == flag)
            return wiznet_table[i].name;
    
    return "";
}


void 
wiznet( long flag, long flag_skip, int min_level, const char *fmt, ... )
{
    Descriptor *d;
    PCharacter *pch;

    {
        va_list args;
        va_start( args, fmt );
        DLString msg = vfmt( NULL, fmt, args ).colourStrip( );
        va_end( args );
        LogStream::sendNotice( ) << "[wiznet:" << wiznet_name( flag ) << "] " << msg << endl;
    }

    for ( d = descriptor_list; d != 0; d = d->next ) {
        if (d->connected != CON_PLAYING)
            continue;

        if (!d->character)
            continue;

        if (d->character->is_npc( ))
            continue;
        
        pch = d->character->getPC( );

        if (!pch->is_immortal( ))
            continue;
        
        if (flag && !IS_SET(pch->wiznet, flag))
            continue;
        
        if (flag_skip && IS_SET(pch->wiznet, flag_skip))
            continue;

        if (pch->get_trust( ) < min_level)
            continue;

        if (IS_SET(pch->wiznet, WIZ_PREFIX))
            pch->send_to( "--> " );

        va_list av;

        va_start(av, fmt);
        pch->vpecho(fmt, av);
        pch->pecho("");
        va_end(av);
    }
}

