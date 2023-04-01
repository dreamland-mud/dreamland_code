/* $Id$
 *
 * ruffina, 2004
 */
#include "communicationchannel.h"
#include "twitlist.h"

#include "skillreference.h"
#include "pcharacter.h"

#include "mudtags.h"
#include "mercdb.h"
#include "merc.h"
#include "act.h"
#include "def.h"

GSN(garble);

/*-----------------------------------------------------------------------
 * CommunicationChannel
 *-----------------------------------------------------------------------*/
CommunicationChannel::CommunicationChannel( ) 
            :   off( 0, &comm_flags ),
                ignore( false ), garble( false ), isolate( false ), deafen( false ), hook(false), ooc(false),
                positionOther( 0, &position_table )
{
}

CommunicationChannel::~CommunicationChannel( ) 
{
}

bool CommunicationChannel::canHear( Character *ch ) const
{
    return ch->get_trust( ) >= trustHear.getValue( );
}

bool CommunicationChannel::checkIgnore( Character *ch, Character *victim ) const
{
    XMLAttributeTwitList::Pointer twit;
    
    if (!ignore)
        return false;
        
    if (victim->is_npc( ))
        return false;

    twit = victim->getPC( )->getAttributes( ).findAttr<XMLAttributeTwitList>( "twit" );

    if (!twit)
        return false;

    return twit->isAvailable( ch->getNameC() );
}

void CommunicationChannel::applyGarble( Character *ch, DLString &msg ) const
{
    ostringstream srcStream;
    char result[MAX_INPUT_LENGTH];
    const char *src;
    char *dst;

    if (!garble)
        return;

    if (!ch->isAffected( gsn_garble ))
        return;
    
    mudtags_convert( msg.c_str( ), srcStream, TAGS_CONVERT_VIS|TAGS_CONVERT_COLOR|TAGS_ENFORCE_NOCOLOR, 0 );
    DLString srcString = srcStream.str( );
    src = srcString.c_str( );
    dst = result;

    for (; *src; src++, dst++)
    {
        char c = *src;

        if( c >= 'a' && c <= 'z' )
            *dst= 'a' + number_range( 0, 25 );
        else if( c >= 'A' && c <= 'Z' )
            *dst = 'A' + number_range( 0, 25 );
        else if( c >= 'Ю' && c <= 'Ч' )
            *dst = 'Ю' + number_range( 0, 'Ч' - 'Ю' );
        else if( c >= 'ю' && c <= 'ъ' )
            *dst = 'ю' + number_range( 0, 'ъ' - 'ю' );
        else
            *dst = c;
    }

    *dst = 0;
    msg = result;
}

bool CommunicationChannel::checkIsolator( Character *ch, Character *victim ) const
{
    if (!isolate)
        return false;
    if (ch->is_npc( ) || victim->is_npc( ))
        return false;
    if (victim->is_immortal( ))
        return false;
    if (!ch->getPC( )->getAttributes( ).isAvailable("isolator"))
        return false;
    if (victim->getPC( )->getAttributes( ).isAvailable("isolator"))
        return false;

    return true;
}


DLString CommunicationChannel::outputSelf( Character *ch, const DLString &format, const DLString &msg ) const
{
    DLString message = fmt( ch, format.c_str( ), ch, msg.c_str( ) );
    return message;
}

DLString CommunicationChannel::outputVict( Character *ch, Character *victim, 
                                const DLString &format, const DLString &msg ) const
{
    DLString message = fmt( victim, format.c_str( ), ch, msg.c_str( ), victim );
    return message;
}

DLString CommunicationChannel::outputChar( Character *ch, Character *victim, 
                                  const DLString &format, const DLString &msg ) const
{
    DLString message = fmt( ch, format.c_str( ), ch, msg.c_str( ), victim );
    return message;
}

void CommunicationChannel::postOutput( Character *outputTo, const DLString &message ) const
{
}

