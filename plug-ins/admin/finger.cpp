/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
                          finger.cpp  -  description
                             -------------------
    begin                : Fri May 18 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#include <sstream>

#include "admincommand.h"
#include "pcharactermanager.h"
#include "pcharacter.h"
#include "character.h"
#include "race.h"
#include "clanreference.h"
#include "merc.h"
#include "def.h"

CLAN(none);

CMDADM( finger )
{
	std::basic_ostringstream<char> str;

	if ( !ch->is_immortal() )
	{
		str << "Что?" << std::endl;
	}
	else if( PCMemoryInterface* pci = PCharacterManager::find( constArguments ) )
	{
		str << "Name: " << pci->getName( ) << std::endl 
		    << "Last time: " << pci->getLastAccessTime( ).getTimeAsString( ) << std::endl
		    << "Last host: " << pci->getLastAccessHost( ) << std::endl
		    << "Level: " << pci->getLevel( ) << "  "
		    << "Race: " << pci->getRace( )->getName( ) << "  "
		    << "Class: " << pci->getProfession( )->getName( ).c_str( ) << std::endl;

		if (pci->getClan( ) != clan_none)
		    str << "Clan: " << pci->getClan( )->getShortName( ) << "  "
			<< "ClanLevel: " << pci->getClanLevel( ) << endl;
	}
	else
	{
		str << "Char not found." << std::endl;
	}
	ch->send_to( str );
}
