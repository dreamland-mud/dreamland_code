/* $Id: clanskill.cpp,v 1.1.6.5.6.13 2009/03/16 20:06:59 rufina Exp $
 *
 * ruffina, 2004
 */
#include "clanskill.h"
#include "clantypes.h"

#include "skillmanager.h"
#include "pcharacter.h"
#include "room.h"
#include "object.h"
#include "npcharacter.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

const DLString ClanSkill::CATEGORY = "Клановые умения";
GROUP(clan);

ClanSkill::ClanSkill( )
{
}

SkillGroupReference & ClanSkill::getGroup( ) 
{
    return group_clan;
}

bool ClanSkill::visible( Character * ch ) const
{
    const SkillClanInfo *ci;
    
    if (ch->is_npc( ) && mob.visible( ch->getNPC( ), this ) == MPROF_ANY)
	return true;
    
    if (!( ci = getClanInfo( ch ) ))
	return false;

    if (ci->level.getValue( ) >= LEVEL_IMMORTAL)
	return false;

    if (!ch->is_npc( ) && ci->clanLevel.getValue( ) > ch->getPC( )->getClanLevel( ))
	return false;

    if (ci->maxLevel.getValue( ) < ch->getRealLevel( ) && ci->maxLevel.getValue( ) < LEVEL_MORTAL)
	return false;

    return true;
}

bool ClanSkill::available( Character * ch ) const
{
    return ch->getRealLevel( ) >= getLevel( ch );
}

bool ClanSkill::usable( Character * ch, bool message = true ) const 
{
    const SkillClanInfo *ci;
    
    if (!available( ch ))
	return false;
    
    ci = getClanInfo( ch );

    if (ci && !ci->needItem.getValue( ))
	return true;

    if (ch->getClan( )->getData( )
        && ch->getClan( )->getData( )->hasItem( ))
	return true;

    if (message)
	ch->println( "Клан не может сейчас придать тебе сил." );

    return false;
}

int ClanSkill::getLevel( Character *ch ) const
{
    if (!visible( ch ))
	return 999;
    
    if (ch->is_npc( ) && mob.visible( ch->getNPC( ), this ) == MPROF_ANY)
	return 1;

    return getClanInfo( ch )->level.getValue( );
}

int ClanSkill::getLearned( Character *ch ) const
{
    const SkillClanInfo *ci;

    if (!usable( ch, false ))
	return 0;

    if (ch->is_npc( )) 
	return mob.getLearned( ch->getNPC( ), this );
    
    if (( ci = getClanInfo( ch ) ) && !ci->needPractice)
	return getMaximum( ch );

    return ch->getPC( )->getSkillData( getIndex( ) ).learned;
}

int ClanSkill::getWeight( Character * ) const
{
    return 0;
}

int ClanSkill::getMaximum( Character *ch ) const
{
    const SkillClanInfo *ci;

    if (( ci = getClanInfo( ch ) ))
	return ci->maximum;

    return BasicSkill::getMaximum( ch );
}

bool ClanSkill::canForget( PCharacter * ) const
{
    return false;
}

bool ClanSkill::canPractice( PCharacter * ch, std::ostream & ) const
{
    return available( ch );
}

bool ClanSkill::canTeach( NPCharacter *mob, PCharacter * ch, bool verbose ) 
{
    if (mob && ch->getClan( ) == mob->getClan( ))
	return true;
   
    if (verbose) { 
        if (mob)
            ch->pecho( "%^C1 не служит твоему клану.", mob );
        else
            ch->println( "Клановые умения практикуют у служителей клана, "
                         "например, у лекаря или охранника." );
    }

    return false;
}

void ClanSkill::show( PCharacter *ch, std::ostream & buf ) 
{
    list<DLString> clanNames;
    Clans::iterator i;
    PCSkillData &data = ch->getSkillData( getIndex( ) );
    
    buf << "'{W" << getName( ) << "{x' " 
	<< "'{W" << getRussianName( ) << "{x', "
        << (spell && spell->isCasted( ) ? "заклинание" : "умение") << " ";

    for (i = clans.begin( ); i != clans.end( ); i++) {
	Clan *clan = ClanManager::getThis( )->find( i->first );
	
	if (clan->isValid( )) {
	    ostringstream ostr;
	    
	    ostr << "{" << clan->getColor( ) << clan->getShortName( ) << "{x";
	    clanNames.push_back( ostr.str( ) );
	}
    }

    switch (clanNames.size( )) {
    case 0: 
	buf << "неизвестного клана ";
	break;
    case 1:
	buf << "клана " << clanNames.front( );
	break;
    default:
	buf << "кланов ";
    
	for (list<DLString>::iterator i = clanNames.begin( ); i != clanNames.end( ); )	{
	    buf << *i;
	    
	    if (++i != clanNames.end( ))
		buf << ", ";
	}

	break;
    }
    
    if (!visible( ch )) {
	buf << endl;
	return;
    }
	
    buf << ", уровень {W" << getLevel( ch ) << "{x";

    if (available( ch ))
	buf << ", изучено на {W" << data.learned << "%{x";
    
    buf << endl;
}

const SkillClanInfo * 
ClanSkill::getClanInfo( Character *ch ) const
{
    Clans::const_iterator i;
    
    i = clans.find( ch->getClan( )->getName( ) );

    return (i == clans.end( ) ? NULL : &i->second);
}

SkillClanInfo::SkillClanInfo( )
                 : level( 1 ), maximum( 100 ), rating( 1 ),
		   clanLevel( 0 ), 
		   needItem( true ), needPractice( true ),
		   maxLevel( LEVEL_MORTAL )
{
}

