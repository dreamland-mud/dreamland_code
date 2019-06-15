/* $Id$
 *
 * ruffina, 2004
 */
#include "racechannel.h"
#include "replay.h"

#include "pcharacter.h"
#include "pcrace.h"
#include "act.h"

/*-----------------------------------------------------------------------
 * RaceChannel
 *-----------------------------------------------------------------------*/
RaceChannel::RaceChannel( )
{
}

bool RaceChannel::isGlobalListener( Character *ch, Character *victim ) const
{
    if (victim->is_npc( ))
        return false;

    if (!victim->is_immortal( ))
        if (victim->getRace( ) != ch->getRace( ))
            return false;

    return WorldChannel::isGlobalListener( ch, victim );
}

DLString RaceChannel::outputSelf( Character *ch, const DLString &format, const DLString &msg ) const
{
    DLString race = ch->getRace( )->getPC( )->getMltName( );
    DLString message = fmt( ch, format.c_str( ), ch, race.c_str( ), msg.c_str( ) );
    return message;
//    ch->println( message );
//    postOutput( ch, message );
}

DLString RaceChannel::outputVict( Character *ch, Character *victim, 
                              const DLString &format, const DLString &msg ) const
{
    DLString race = ch->getRace( )->getPC( )->getMltName( );
    DLString message = fmt( victim, format.c_str( ), ch, race.c_str( ), msg.c_str( ) );
    return message;
//    victim->println( message );
//    postOutput( victim, message );
}

bool RaceChannel::canTalkGlobally( Character *ch ) const
{
    if (!GlobalChannel::canTalkGlobally( ch ))
        return false;

    if (ch->is_npc( ))
        return false;

    return true;
}

