/* $Id: cclantalk.cpp,v 1.1.6.4.6.10 2008/07/26 19:12:32 rufina Exp $
 *
 * ruffina, 2005
 */
/***************************************************************************
 * Все права на этот код 'Dream Land' пренадлежат Igor {Leo} и Olga {Varda}*
 * Некоторую помощь в написании этого кода, а также своими идеями помогали:*
 *    Igor S. Petrenko     {NoFate, Demogorgon}                            *
 *    Koval Nazar          {Nazar, Redrum}                                 *
 *    Doropey Vladimir     {Reorx}                                         *
 *    Kulgeyko Denis       {Burzum}                                        *
 *    Andreyanov Aleksandr {Manwe}                                         *
 *    и все остальные, кто советовал и играл в этот MUD                    *
 ***************************************************************************/

#include <string.h>

#include "cclantalk.h"
#include "commandtemplate.h"
#include "logstream.h"
#include "replay.h"

#include "pcharacter.h"
#include "clanreference.h"
#include "skillreference.h"
#include "dreamland.h"
#include "loadsave.h"

#include "descriptor.h"
#include "mudtags.h"
#include "act.h"

#include "merc.h"
#include "def.h"

CLAN(none);
GSN(deafen);
GSN(garble);

/* TODO reimplement clantalk using 'communication' interfaces */

static void garble( const char *src, char *dst )
{

    for (; *src; src++, dst++)
    {
            char ch = *src;

            if( ch >= 'a' && ch <= 'z' )
                    *dst= 'a' + number_range( 0, 25 );
            else if( ch >= 'A' && ch <= 'Z' )
                    *dst = 'A' + number_range( 0, 25 );
            else if( ch >= 'Ю' && ch <= 'Ч' )
                    *dst = 'Ю' + number_range( 0, 'Ч' - 'Ю' );
            else if( ch >= 'ю' && ch <= 'ъ' )
                    *dst = 'ю' + number_range( 0, 'ъ' - 'ю' );
            else
                    *dst = ch;
    }

    *dst = 0;
}


static bool check_soap( Character *ch )
{
    static const DLString soap( "soap" );
    
    if (IS_CHARMED(ch))
        return check_soap(ch->master);
    
    if (ch->is_npc( ))
        return false;
    
    if (!ch->getPC( )->getAttributes( ).isAvailable( soap )) 
        return false;
    
    oldact("$c1 пускает изо рта {Rр{Yа{Gз{Cн{Mо{Rц{Gв{Yе{Cт{Mн{Yы{Cе{x мыльные пузыри.", ch, 0, 0, TO_ROOM);
    oldact("Ты пускаешь изо рта {Rр{Yа{Gз{Cн{Mо{Rц{Gв{Yе{Cт{Mн{Yы{Cе{x мыльные пузыри.", ch, 0, 0, TO_CHAR);
    return true;
}


COMMAND(CClanTalk, "cb")
{
    Descriptor *d;
    DLString argument = constArguments;
    DLString act_str;

    if (ch->getClan( ) == clan_none) {
        ch->pecho("Ты не принадлежишь ни к одному Клану.");
        return;
    }

    if (!ch->getClan( )->hasChannel( )) {
        ch->pecho("До тебя никому нет дела.");
        return;
    }
    
    if (argument.empty( )) {
        TOGGLE_BIT(ch->comm, COMM_NOCB);

        if (IS_SET(ch->comm, COMM_NOCB))
            ch->pecho("С этого момента ты не слышишь клановые разговоры.");
        else
            ch->pecho("Ты снова слышишь клановые разговоры.");
        return;
    }
    
    REMOVE_BIT(ch->comm, COMM_NOCB);

    if (check_soap(ch))
        return;

    act_str = ch->getClan()->getChannelPattern();
    
    if (dreamland->hasOption( DL_LOG_COMM ))
        LogStream::sendNotice( ) 
            << "[" << ch->getClan( )->getShortName( ) << "] " 
            << ch->getName( ) << ": " << argument << endl;

    if (!ch->isAffected(gsn_deafen)) {
        ostringstream msgBuf;

        msgBuf << fmt(ch, act_str.c_str(), ch) << argument << "{x";
        ch->pecho(msgBuf.str());

        if (ch->getPC()) {
            remember_history_public(ch->getPC(), msgBuf.str());
        }
    }

    for (d = descriptor_list; d != 0; d = d->next) 
        if (d->connected == CON_PLAYING
                && d->character
                && d->character != ch
                && (d->character->getClan( ) == ch->getClan( ))
                && !IS_SET(d->character->comm, COMM_NOCB)
                && !d->character->isAffected(gsn_deafen))
        {
            char msg_str[MAX_INPUT_LENGTH];

            if (ch->isAffected(gsn_garble)) {
                ostringstream out;
                mudtags_convert( argument.c_str( ), out, TAGS_CONVERT_VIS|TAGS_CONVERT_COLOR|TAGS_ENFORCE_NOCOLOR, d->character );
                garble( out.str( ).c_str( ), msg_str );
            }
            else
                strcpy( msg_str, argument.c_str( ) );

            ostringstream msgBuf;
            msgBuf << fmt(d->character, act_str.c_str(), ch) << msg_str << "{x";
            d->character->pecho(msgBuf.str());

            if (d->character->getPC())
                remember_history_public(d->character->getPC(), msgBuf.str());
        }
}

bool CClanTalk::visible( Character *ch ) const
{
    return ch->getClan( ) != clan_none;
}

void clantalk( Clan &clan, const DLString &message )
{
    va_list ap;
    ostringstream buf;
    
    buf << "{" << clan.getColor( ) 
        << "[" << clan.getShortName( ) << "] : "
        << message << "{x" << endl;

    for (Descriptor *d = descriptor_list; d != 0; d = d->next) 
        if (d->connected == CON_PLAYING
                && d->character
                && (d->character->getClan( ) == clan)
                && !IS_SET(d->character->comm, COMM_NOCB)
                && !d->character->isAffected(gsn_deafen))
        {
            d->character->send_to( buf );
        }
}

