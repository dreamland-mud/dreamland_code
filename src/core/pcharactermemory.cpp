/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
                          pcharactermemory.cpp  -  description
                             -------------------
    begin                : Tue May 29 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#include "pcharactermemory.h"
#include "class.h"

PCharacterMemory::PCharacterMemory( )
{
}

PCharacterMemory::~PCharacterMemory( )
{
}

const DLString& PCharacterMemory::getName( ) const throw( )
{
	return name.getValue( );
}

void PCharacterMemory::setName( const DLString& name ) throw( )
{
	this->name.setValue( name );
}

const DLString& PCharacterMemory::getPassword( ) const throw( )
{
	return password.getValue( );
}
void PCharacterMemory::setPassword( const DLString &password ) throw( )
{
	this->password = password;
}

const Date& PCharacterMemory::getLastAccessTime( ) const throw( )
{
	return lastAccessTime;
}

void PCharacterMemory::setLastAccessTime( const Date& lastAccessTime ) throw( )
{
	this->lastAccessTime = lastAccessTime;
}

const DLString& PCharacterMemory::getLastAccessHost( ) const throw( )
{
	return lastAccessHost;
}

void PCharacterMemory::setLastAccessHost( const DLString& lastAccessHost ) throw( )
{
	this->lastAccessHost.setValue( lastAccessHost );
}

short PCharacterMemory::getLevel( ) const throw( )
{
	return level.getValue( );
}

void PCharacterMemory::setLevel( short level ) throw( )
{
	this->level = level;
}

int PCharacterMemory::getTrust( ) const throw( )
{
	return trust.getValue( );
}

void PCharacterMemory::setTrust( int trust ) throw( )
{
	this->trust = trust;;
}
int PCharacterMemory::getSecurity( ) const throw( )
{
	return security.getValue( );
}

void PCharacterMemory::setSecurity( int security ) throw( )
{
	this->security = security;;
}

ClanReference &PCharacterMemory::getPetition( ) throw( )
{
	return petition;
}

void PCharacterMemory::setPetition( const ClanReference & petition ) throw( )
{
	this->petition.assign( petition );
}

ClanReference &PCharacterMemory::getClan( ) throw( )
{
	return clan;
}

void PCharacterMemory::setClan( const ClanReference & clan ) throw( )
{
	this->clan.assign( clan );
}

HometownReference &PCharacterMemory::getHometown( ) throw( )
{
	return hometown;
}

void PCharacterMemory::setHometown( const HometownReference & hometown ) throw( )
{
	this->hometown.assign( hometown );
}

ProfessionReference &PCharacterMemory::getProfession( ) throw( )
{
        return profession;
}

void PCharacterMemory::setProfession( const ProfessionReference & profession ) throw( )
{
        this->profession.assign( profession );
}

RaceReference &PCharacterMemory::getRace( ) throw( )
{
	return race;
}

void PCharacterMemory::setRace( const RaceReference & race ) throw( )
{
	this->race.assign( race );
}

short PCharacterMemory::getClanLevel( ) const throw( )
{
	return clanLevel.getValue( );
}

void PCharacterMemory::setClanLevel( short clanLevel ) throw( )
{
	this->clanLevel.setValue( clanLevel );
}

short PCharacterMemory::getSex( ) const throw( )
{
	return sex.getValue( );
}

void PCharacterMemory::setSex( short sex ) throw( )
{
	this->sex.setValue( sex );
}

XMLAttributes& PCharacterMemory::getAttributes( ) throw( )
{
	return attributes;
}

const XMLAttributes& PCharacterMemory::getAttributes( ) const throw( )
{
	return attributes;
}

void PCharacterMemory::setAttributes( const XMLAttributes& attributes ) throw( )
{
	this->attributes = attributes;
}

Remorts & PCharacterMemory::getRemorts( ) throw( )
{
    return remorts;
}

void PCharacterMemory::setRemorts( const Remorts& remorts ) throw( ) 
{
    this->remorts = remorts;
}

const RussianString& PCharacterMemory::getRussianName( ) const throw( )
{
    return russianName;
}

void PCharacterMemory::setRussianName( const DLString& name ) throw( )
{
    russianName.setFullForm( name );
}

int PCharacterMemory::get_trust( ) const
{
    if (getAttributes( ).isAvailable( "coder" ))
	return 0xFFFF;
    
    if (getTrust( ) != 0)
	return getTrust( );

    return getLevel( );
}

ReligionReference & PCharacterMemory::getReligion( ) throw( )
{
    return religion;
}

void PCharacterMemory::setReligion( const ReligionReference &religion )
{
    this->religion.assign( religion );
}

bool PCharacterMemory::isOnline( ) const
{
    return false;
}

PCharacter * PCharacterMemory::getPlayer( ) 
{
    return NULL;
}
