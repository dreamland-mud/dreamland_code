/* $Id: raceaptitude.cpp,v 1.1.2.6 2008/05/27 21:30:05 rufina Exp $
 *
 * ruffina, 2004
 */
#include "raceaptitude.h"

#include "logstream.h"

#include "skillmanager.h"
#include "pcharacter.h"
#include "room.h"
#include "race.h"
#include "npcharacter.h"

#include "merc.h"
#include "def.h"

const DLString RaceAptitude::CATEGORY = "Расовые способности";

RaceAptitude::RaceAptitude( ) 
{
}

bool RaceAptitude::visible( Character * ch ) const
{
    const SkillRaceInfo *ri;

    if (ch->is_npc( ) && mob.visible( ch->getNPC( ), this ) == MPROF_ANY)
	return true;
    
    ri = getRaceInfo( ch );
    return (ri && ri->level.getValue( ) < LEVEL_IMMORTAL);
}

bool RaceAptitude::available( Character * ch ) const
{
    return ch->getRealLevel( ) >= getLevel( ch );
}

bool RaceAptitude::usable( Character *ch, bool message = true ) const
{
    return available( ch );
}

int RaceAptitude::getLevel( Character *ch ) const
{
    if (!visible( ch ))
	return 999;
    
    if (ch->is_npc( ) && mob.visible( ch->getNPC( ), this ) == MPROF_ANY)
	return 1;

    return getRaceInfo( ch )->level.getValue( );
}

int RaceAptitude::getLearned( Character *ch ) const
{
    if (!usable( ch ))
	return 0;

    if (ch->is_npc( )) 
	return mob.getLearned( ch->getNPC( ), this );

    return ch->getPC( )->getSkillData( getIndex( ) ).learned.getValue( );
}

int RaceAptitude::getWeight( Character * ) const
{
    return 0;
}

bool RaceAptitude::canForget( PCharacter * ) const
{
    return false;
}

bool RaceAptitude::canPractice( PCharacter * ch, std::ostream & ) const
{
    return available( ch );
}

bool RaceAptitude::canTeach( NPCharacter *mob, PCharacter *ch, bool verbose )
{
    if (!mob) {
        if (verbose)
            ch->println( "Тебе не с кем практиковаться здесь." );
	return false;
    }
    
    if (mob->pIndexData->practicer.isSet(  (int)getGroup( ) ))
	return true;

    if (verbose)
        ch->println( "Ты не можешь практиковать это здесь." );
    return false;
}

void RaceAptitude::show( PCharacter *ch, std::ostream &buf ) 
{
    Races::iterator i;

    buf << (spell ? "Заклинание" : "Умение") 
	<< " '{W" << getName( ) << "{x'" 
	<< " '{W" << getRussianName( ) << "{x'"
	<< ", особенность ";

    switch (races.size( )) {
    case 0:
	buf << "неизвестной расы ";
	break;
    case 1:
	buf << "расы ";
	break;
    default:
	buf << "рас ";
	break;
    }

    for (i = races.begin( ); i != races.end( ); ) {
	buf << "{W" << i->first << "{x";

	if (++i != races.end( ))
	    buf << ", ";
    }
    
    buf << endl
	<< "Входит в группу '{hg{W" << getGroup( )->getName( ) << "{x'";
	
    if (!visible( ch )) {
	buf << endl;
	return;
    }
	
    buf << ", уровень {W" << getLevel( ch ) << "{x";

    if (available( ch ))
	buf << ", изучено на {W" 
	    << ch->getSkillData( getIndex( ) ).learned 
	    << "%{x";

    buf << endl;
}

const SkillRaceInfo *
RaceAptitude::getRaceInfo( Character *ch ) const
{
    Races::const_iterator i = races.find( ch->getRace( )->getName( ) );
    
    return (i == races.end( ) ? NULL : &i->second);
}

