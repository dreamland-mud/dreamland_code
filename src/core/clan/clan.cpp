/* $Id$
 *
 * ruffina, 2004
 */
#include "clan.h"

Clan::Clan( )
{
}

Clan::Clan( const DLString &n ) : name( n )
{
}

Clan::~Clan( )
{
}

bool Clan::isValid( ) const
{
    return false;
}
const DLString &Clan::getName( ) const
{
    return name;
}
const DLString &Clan::getShortName( ) const
{
    return DLString::emptyString;
}
const DLString &Clan::getLongName( ) const
{
    return DLString::emptyString;
}
const DLString &Clan::getColor( ) const
{
    static const DLString color( "x" );
    return color;
}
const DLString &Clan::getPaddedName( ) const
{
    return DLString::emptyString;
}

const DLString& Clan::getChannelPattern() const
{
    return DLString::emptyString;
}
const DLString & Clan::getTitle( PCMemoryInterface * ) const
{
    return DLString::emptyString;
}
bool Clan::isLeader( PCMemoryInterface * ) const
{
    return false;
}
bool Clan::isRecruiter( PCMemoryInterface * ) const
{
    return false;
}
bool Clan::canInduct( PCharacter * ) const
{
    return false;
}

void Clan::onInduct(PCharacter *) const
{
}

void Clan::makeMonument( Character *, Character * ) const
{
}
void Clan::handleVictory( PCharacter *, PCharacter * )
{
}
void Clan::handleDefeat( PCharacter *, PCharacter * )
{
}

bool Clan::isEnemy( const Clan & )
{
    return false;
}
bool Clan::isDispersed( ) const
{
    return true;
}
int Clan::getRecallVnum( ) const
{
    return 0;
}
bool Clan::hasChannel( ) const
{
    return false;
}
bool Clan::hasDiplomacy( ) const
{
    return false;
}
ClanData * Clan::getData( ) 
{
    return NULL;
}
const ClanMembership * Clan::getMembership( ) const 
{
    return NULL;
}
ClanMembership * Clan::getMembership( ) 
{
    return NULL;
}
const ClanTitles * Clan::getTitles( ) const
{
    return NULL;
}
const ClanOrgs * Clan::getOrgs( ) const
{
    return NULL;
}
bool Clan::isHidden( ) const
{
    return true;
}
