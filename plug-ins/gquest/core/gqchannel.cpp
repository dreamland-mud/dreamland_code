/* $Id: gqchannel.cpp,v 1.1.2.2 2005/09/16 13:10:09 rufina Exp $
 * 
 * ruffina, 2003
 */

#include "gqchannel.h"
#include "globalquestmanager.h"
#include "globalquest.h"
#include "globalquestinfo.h"

#include "character.h"
#include "pcharacter.h"
#include "room.h"
#include "dreamland.h"
#include "messengers.h"
#include "merc.h"
#include "descriptor.h"
#include "multimessage.h"
#include "l10n.h"

/* Messenger fan-out convention for every gecho below: the Discord stream is
 * English-only (both the quest label and the body), Telegram stays Russian.
 * Same split as send_discord_level / send_telegram_level. */

const char * const GQChannel::BOLD = "{Y";
const char * const GQChannel::NORMAL = "{y";

void GQChannel::zecho( GlobalQuest *gquest, Area *area, const DLString& msg ) 
{
    Descriptor *d;
    Character *ch;
    
    if (dreamland->isShutdown( ))
        return;

    for ( d = descriptor_list; d; d = d->next ) {
        if (d->connected != CON_PLAYING)
            continue;

        if (!(ch = d->character) || ch->in_room->area != area)
            continue;
        
        if (gquest->isLevelOK( ch ))
            ch->send_to( msg );
    }
}

void GQChannel::gecho( GlobalQuestInfo *gqi, const DLString& msg )
{
    gecho( gqi->getQuestName( ), msg );
}

void GQChannel::gecho( const DLString& name, const DLString& msg, PCharacter *pch )
{
    Descriptor *d;

    if (dreamland->isShutdown( ))
        return;

    for ( d = descriptor_list; d; d = d->next ) {
        if (d->connected != CON_PLAYING)
            continue;

        Character *ch = d->character;
        if (!ch || (pch && pch == ch->getPC( )))
            continue;

        std::basic_ostringstream<char> buf;
        buf << BOLD << "[" << NORMAL << l( ch, "Глобал" ) << BOLD << ": "
            << NORMAL << l( ch, name.c_str( ) ) << BOLD << "] "
            << NORMAL << msg << "{x" << endl;
        ch->send_to( buf );
    }

    // The body reaching this overload is free-form immortal text ('gquest talk'),
    // so only the quest label can be localized for Discord.
    DLString nameEn = _( name ).getMessage( LANG_EN );
    send_discord_gquest( nameEn, msg );
    send_telegram_gquest( name, msg );
}

void GQChannel::gecho( GlobalQuest *gq, const MultiMessage &msg, PCharacter *pch )
{
    Descriptor *d;

    if (dreamland->isShutdown( ))
        return;

    GlobalQuestInfo::Pointer gqi = GlobalQuestManager::getThis( )->findGlobalQuestInfo( gq->getQuestID( ) );

    for (d = descriptor_list; d; d = d->next) {
        if (d->connected != CON_PLAYING)
            continue;

        Character *ch = d->character;
        if (!ch || (pch && pch == ch->getPC( )))
            continue;

        std::basic_ostringstream<char> buf;
        buf << BOLD << "[" << NORMAL << l( ch, "Глобал" ) << BOLD << ": "
            << NORMAL << gqi->getQuestNameFor( ch ) << BOLD << "] "
            << NORMAL << msg.getMessage( ch ) << "{x" << endl;
        ch->send_to( buf );
    }

    send_discord_gquest( gqi->getQuestNameFor( LANG_EN ), msg.getMessage( LANG_EN ) );
    send_telegram_gquest( gqi->getQuestName( ), msg.getRu( ) );
}

void GQChannel::gecho( const DLString &name, const MultiMessage &msg, PCharacter *pch )
{
    Descriptor *d;

    if (dreamland->isShutdown( ))
        return;

    for (d = descriptor_list; d; d = d->next) {
        if (d->connected != CON_PLAYING)
            continue;

        Character *ch = d->character;
        if (!ch || (pch && pch == ch->getPC( )))
            continue;

        std::basic_ostringstream<char> buf;
        buf << BOLD << "[" << NORMAL << l( ch, "Глобал" ) << BOLD << ": "
            << NORMAL << l( ch, name.c_str( ) ) << BOLD << "] "
            << NORMAL << msg.getMessage( ch ) << "{x" << endl;
        ch->send_to( buf );
    }

    DLString nameEn = _( name ).getMessage( LANG_EN );
    send_discord_gquest( nameEn, msg.getMessage( LANG_EN ) );
    send_telegram_gquest( name, msg.getRu( ) );
}

void GQChannel::gecho( const DLString& msg )
{
    Descriptor *d;
    std::basic_ostringstream<char> buf;

    if (dreamland->isShutdown( ))
        return;

    buf << BOLD << "[" << NORMAL << "Глобал" << BOLD << "] "
        << NORMAL << msg << "{x" << endl;
    
    for ( d = descriptor_list; d; d = d->next ) 
        if (d->connected == CON_PLAYING)
            if (d->character)
                d->character->send_to( buf );
}

void GQChannel::pecho( Character *ch, ostringstream& buf ) 
{
    pecho( ch, buf.str( ) );
}

void GQChannel::pecho( Character *ch, const DLString& msg ) 
{
    if (dreamland->isShutdown( ))
        return;

    ch->pecho( "%s%s{x", NORMAL, msg.c_str( ) );
}

