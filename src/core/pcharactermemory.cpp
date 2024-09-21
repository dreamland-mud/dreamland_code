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
#include "grammar_entities_impl.h"

PCharacterMemory::PCharacterMemory( )
{
}

PCharacterMemory::~PCharacterMemory( )
{
}

const DLString& PCharacterMemory::getName( ) const 
{
        return name.getValue( );
}

DLString PCharacterMemory::getNameP(char gram_case) const
{
    if (russianName.getFullForm().empty())
        return name;

    return russianName.decline(gram_case);
}

const char * PCharacterMemory::getNameC() const
{
    return name.c_str();
}

void PCharacterMemory::setName( const DLString& name ) 
{
        this->name.setValue( name );
}

const DLString& PCharacterMemory::getPassword( ) const 
{
        return password.getValue( );
}
void PCharacterMemory::setPassword( const DLString &password ) 
{
        this->password = password;
}

const Date& PCharacterMemory::getLastAccessTime( ) const 
{
        return lastAccessTime;
}

void PCharacterMemory::setLastAccessTime( const Date& lastAccessTime ) 
{
        this->lastAccessTime = lastAccessTime;
}

const DLString& PCharacterMemory::getLastAccessHost( ) const 
{
        return lastAccessHost;
}

void PCharacterMemory::setLastAccessHost( const DLString& lastAccessHost ) 
{
        this->lastAccessHost.setValue( lastAccessHost );
}

const char * PCharacterMemory::getDescription( ) const
{
    return description.c_str();
}

void PCharacterMemory::setDescription( const DLString &description )
{
    this->description = description; 
}

short PCharacterMemory::getLevel( ) const 
{
        return level.getValue( );
}

void PCharacterMemory::setLevel( short level ) 
{
        this->level = level;
}

int PCharacterMemory::getTrust( ) const 
{
        return trust.getValue( );
}

void PCharacterMemory::setTrust( int trust ) 
{
        this->trust = trust;;
}
int PCharacterMemory::getSecurity( ) const 
{
        return security.getValue( );
}

void PCharacterMemory::setSecurity( int security ) 
{
        this->security = security;
}

int PCharacterMemory::getQuestPoints( ) const 
{
        return questpoints;
}

void PCharacterMemory::setQuestPoints( int questpoints ) 
{
        this->questpoints = questpoints;
}

ClanReference &PCharacterMemory::getPetition( ) 
{
        return petition;
}

void PCharacterMemory::setPetition( const ClanReference & petition ) 
{
        this->petition.assign( petition );
}

ClanReference &PCharacterMemory::getClan( ) 
{
        return clan;
}

void PCharacterMemory::setClan( const ClanReference & clan ) 
{
        this->clan.assign( clan );
}

HometownReference &PCharacterMemory::getHometown( ) 
{
        return hometown;
}

void PCharacterMemory::setHometown( const HometownReference & hometown ) 
{
        this->hometown.assign( hometown );
}

ProfessionReference &PCharacterMemory::getProfession( ) 
{
        return profession;
}

void PCharacterMemory::setProfession( const ProfessionReference & profession ) 
{
        this->profession.assign( profession );
}

RaceReference &PCharacterMemory::getRace( ) 
{
        return race;
}

void PCharacterMemory::setRace( const RaceReference & race ) 
{
        this->race.assign( race );
}

short PCharacterMemory::getClanLevel( ) const 
{
        return clanLevel.getValue( );
}

void PCharacterMemory::setClanLevel( short clanLevel ) 
{
        this->clanLevel.setValue( clanLevel );
}

short PCharacterMemory::getSex( ) const 
{
        return sex.getValue( );
}

void PCharacterMemory::setSex( short sex ) 
{
        this->sex.setValue( sex );
}

XMLAttributes& PCharacterMemory::getAttributes( ) 
{
        return attributes;
}

const XMLAttributes& PCharacterMemory::getAttributes( ) const 
{
        return attributes;
}

void PCharacterMemory::setAttributes( const XMLAttributes& attributes ) 
{
        this->attributes = attributes;
}

Remorts & PCharacterMemory::getRemorts( ) 
{
    return remorts;
}

void PCharacterMemory::setRemorts( const Remorts& remorts )  
{
    this->remorts = remorts;
}

const RussianString& PCharacterMemory::getRussianName( ) const 
{
    return russianName;
}

void PCharacterMemory::setRussianName( const DLString& name ) 
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

ReligionReference & PCharacterMemory::getReligion( ) 
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

NPCharacter * PCharacterMemory::getMobile( ) 
{
    return NULL;
}

PCSkills & PCharacterMemory::getSkills()
{
    return skills;        
}

void PCharacterMemory::setSkills(const PCSkills &skills)
{
    this->skills = skills;        
}

PCBonuses & PCharacterMemory::getBonuses()
{
    return bonuses;        
}

void PCharacterMemory::setBonuses(const PCBonuses &bonuses)
{
    this->bonuses = bonuses;        
}

int PCharacterMemory::getStartRoom() const
{
    return start_room;
}

void PCharacterMemory::setStartRoom(int vnum)
{
    this->start_room = vnum;        
}
