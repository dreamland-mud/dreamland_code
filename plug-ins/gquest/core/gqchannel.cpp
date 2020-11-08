/* $Id: gqchannel.cpp,v 1.1.2.2 2005/09/16 13:10:09 rufina Exp $
 * 
 * ruffina, 2003
 */

#include "gqchannel.h"
#include "globalquestmanager.h"
#include "globalquest.h"
#include "globalquestinfo.h"

#include "character.h"
#include "room.h"
#include "dreamland.h"
#include "messengers.h"
#include "merc.h"
#include "descriptor.h"

GQChannel* GQChannel::thisClass = NULL;

const char * const GQChannel::BOLD = "{Y";
const char * const GQChannel::NORMAL = "{y";

GQChannel::GQChannel( ) 
{
    thisClass = this;
}

GQChannel::~GQChannel( )
{
    thisClass = NULL;
}
    
void GQChannel::zecho( GlobalQuest *gquest, AreaIndexData *area, const DLString& msg ) 
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

void GQChannel::gecho( GlobalQuest *gq, ostringstream &buf )
{
    gecho( GlobalQuestManager::getThis( )->findGlobalQuestInfo( 
                            gq->getQuestID( ) )->getQuestName( ),
            buf.str( ) );

    buf.str( "" );
}

void GQChannel::gecho( GlobalQuest *gq, const DLString& msg, PCharacter *pch ) 
{
    gecho( 
            GlobalQuestManager::getThis( )->findGlobalQuestInfo( 
                            gq->getQuestID( ) )->getQuestName( ),
            msg, pch );
}

void GQChannel::gecho( GlobalQuestInfo *gqi, const DLString& msg ) 
{
    gecho( gqi->getQuestName( ), msg );
}

void GQChannel::gecho( const DLString& name, const DLString& msg, PCharacter *pch ) 
{
    Descriptor *d;
    std::basic_ostringstream<char> buf;
    
    if (dreamland->isShutdown( ))
        return;
    
    buf << BOLD << "[" << NORMAL << "Global Quest" << BOLD << ": " 
        << NORMAL << name << BOLD << "] "
        << NORMAL << msg << "{x" << endl;
    
    for ( d = descriptor_list; d; d = d->next ) 
        if (d->connected == CON_PLAYING)
            if (d->character && (!pch || pch != d->character->getPC( ))) 
                d->character->send_to( buf );

    send_discord_gquest(name, msg);
}

void GQChannel::gecho( const DLString& msg ) 
{
    Descriptor *d;
    std::basic_ostringstream<char> buf;
    
    if (dreamland->isShutdown( ))
        return;
    
    buf << BOLD << "[" << NORMAL << "Global Quest" << BOLD << "] " 
        << NORMAL << msg << "{x" << endl;
    
    for ( d = descriptor_list; d; d = d->next ) 
        if (d->connected == CON_PLAYING)
            if (d->character)
                d->character->send_to( buf );
}

void GQChannel::gechoRaw( const DLString& msg ) 
{
    Descriptor *d;
    
    if (dreamland->isShutdown( ))
        return;
    
    for ( d = descriptor_list; d; d = d->next ) 
        if (d->connected == CON_PLAYING && d->character)
            d->character->send_to( msg + "\r\n" );
}

void GQChannel::pecho( Character *ch, ostringstream& buf ) 
{
    pecho( ch, buf.str( ) );
}

void GQChannel::pecho( Character *ch, const DLString& msg ) 
{
    if (dreamland->isShutdown( ))
        return;

    ch->printf( "%s%s{x\r\n", NORMAL, msg.c_str( ) );
}

GQChannel * GQChannel::getThis( ) 
{
    return thisClass;
}

