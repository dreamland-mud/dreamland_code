/* $Id$
 *
 * ruffina, 2004
 */
#include "communicationchannel.h"

#include "commandmanager.h"
#include "skillreference.h"
#include "pcharacter.h"
#include "affect.h"

#include "mudtags.h"

#include "multimessage.h"
#include "merc.h"
#include "act.h"
#include "def.h"

/* Catalog file-key for every channel message (msgSelf/msgOther/...). These come
 * from channels.xml, not a cpp __FILE__, so they can't use the _() macro; wrap
 * them in a MultiMessage under this shared key instead. Entries live in the
 * translation catalog keyed (this file, ru); untranslated messages fall back to
 * RU byte-for-byte, so wrapping is zero-regression. See Trello 2594. */
static const DLString CHANNELS_L10N_FILE = "commands/channels.xml";

GSN(garble);
GSN(slurred_speech);

/*-----------------------------------------------------------------------
 * Drunk speech
 *-----------------------------------------------------------------------*/
static const int SLUR_MAX_PCT = 40;

int slur_percent( Character *ch )
{
    Affect *paf = ch->affected.find( gsn_slurred_speech );

    if (!paf)
        return 0;

    return URANGE( 0, (int)paf->modifier, SLUR_MAX_PCT );
}

// A KOI8 letter: Latin, Russian (0xC0 and up) or the Ukrainian ones below it.
static bool slur_letter( char c )
{
    unsigned char u = (unsigned char)c;

    if ((u >= 'a' && u <= 'z') || (u >= 'A' && u <= 'Z') || u >= 0xC0)
        return true;

    switch (u) {
    case 0xA4: case 0xA6: case 0xA7: case 0xAD:
    case 0xB4: case 0xB6: case 0xB7: case 0xBD:
        return true;
    }

    return false;
}

DLString slur_text( const DLString &src, int pct )
{
    DLString out;

    for (DLString::size_type i = 0; i < src.size( ); i++) {
        char c = src[i];

        if (!slur_letter( c ) || number_percent( ) > pct) {
            out += c;
            continue;
        }

        if (i + 1 < src.size( ) && slur_letter( src[i + 1] ) && chance( 50 )) {
            out += src[i + 1];
            out += c;
            i++;
        }
        else {
            out += c;
            out += c;
        }
    }

    return out;
}

/*-----------------------------------------------------------------------
 * CommunicationChannel
 *-----------------------------------------------------------------------*/
CommunicationChannel::CommunicationChannel( ) 
            :   off( 0, &comm_flags ),
                garble( false ), isolate( false ), deafen( false ), hook(false), ooc(false),
                positionOther( 0, &position_table )
{
}

CommunicationChannel::~CommunicationChannel( ) 
{
}

bool CommunicationChannel::saveCommand() const
{
    // TODO: make a separate XML profile for each channel, rather than keeping all in channels.xml
    Command::Pointer channelsCommand = commandManager->findExact("channels");
    if (channelsCommand)
        return channelsCommand->saveCommand();

    return false;
}


bool CommunicationChannel::canHear( Character *ch ) const
{
    return ch->get_trust( ) >= trustHear.getValue( );
}

void CommunicationChannel::applyGarble( Character *ch, DLString &msg ) const
{
    ostringstream srcStream;
    char result[MAX_INPUT_LENGTH];
    const char *src;
    char *dst;

    if (!garble)
        return;

    bool fGarble = ch->isAffected( gsn_garble );
    int slur = fGarble ? 0 : slur_percent( ch );

    if (!fGarble && slur <= 0)
        return;
    
    mudtags_convert( msg.c_str( ), srcStream, TAGS_CONVERT_VIS|TAGS_CONVERT_COLOR|TAGS_ENFORCE_NOCOLOR, 0 );
    DLString srcString = srcStream.str( );

    if (!fGarble) {
        msg = slur_text( srcString, slur );
        return;
    }

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

// Strip player-injected web/command tags from channel text before it reaches any
// recipient. Only the message is sanitized, never the server format. Shared with the
// non-framework channels (clan `cb`, spouse `mtalk`) via the free helper.
void CommunicationChannel::stripWeb( Character *ch, DLString &msg ) const
{
    mudtags_strip_web( msg, ch );
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


DLString CommunicationChannel::localized( const DLString &format, Character *reader ) const
{
    return MultiMessage( format, CHANNELS_L10N_FILE ).getMessage( reader );
}

DLString CommunicationChannel::localizedEn( const DLString &format ) const
{
    return MultiMessage( format, CHANNELS_L10N_FILE ).getMessage( LANG_EN );
}

DLString CommunicationChannel::outputSelf( Character *ch, const DLString &format, const DLString &msg ) const
{
    DLString localFormat = localized( format, ch );
    DLString message = fmt( ch, localFormat.c_str( ), ch, msg.c_str( ) );
    return message;
}

DLString CommunicationChannel::outputVict( Character *ch, Character *victim,
                                const DLString &format, const DLString &msg ) const
{
    DLString localFormat = localized( format, victim );
    DLString message = fmt( victim, localFormat.c_str( ), ch, msg.c_str( ), victim );
    return message;
}

DLString CommunicationChannel::outputChar( Character *ch, Character *victim,
                                  const DLString &format, const DLString &msg ) const
{
    DLString localFormat = localized( format, ch );
    DLString message = fmt( ch, localFormat.c_str( ), ch, msg.c_str( ), victim );
    return message;
}

DLString CommunicationChannel::webKind( ) const
{
    return "world";
}

DLString CommunicationChannel::webArea( Character *to, Character *from ) const
{
    return DLString::emptyString;
}

void CommunicationChannel::postOutput( Character *outputTo, const DLString &message ) const
{
}

