/* $Id$
 *
 * ruffina, 2004
 */
#include <ctype.h>
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

void Clan::getCastMessages( StringList &, StringList &, StringList & ) const
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
const DLString &Clan::getEnglishName( ) const
{
    return DLString::emptyString;
}
const DLString &Clan::getNameFor( lang_t lang ) const
{
    if (lang == LANG_EN) {
        const DLString &en = getEnglishName( );
        if (!en.empty( ))
            return en;
    }
    if (lang == LANG_UA) {
        const DLString &ua = getUkrainianName( );
        if (!ua.empty( ))
            return ua;
    }

    return getRussianName( );
}
const DLString &Clan::getShortFor( lang_t ) const
{
    return DLString::emptyString;
}
const DLString &Clan::getLongNameFor( lang_t ) const
{
    return getLongName( );
}

const DLString& Clan::getChannelPattern() const
{
    return DLString::emptyString;
}
const DLString & Clan::getTitle( PCMemoryInterface *, lang_t ) const
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

void Clan::makeMonument( Character *, Character *, int ) const
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
StringList Clan::getRankTitles( lang_t, int ) const
{
    return StringList( );
}

int Clan::getHelpId( ) const
{
    const DLString &longName = getLongNameFor( LANG_RU );
    DLString::size_type p = longName.find( "{hh" );
    int id = 0;

    if (p == DLString::npos)
        return 0;

    for (p += 3; p < longName.size( ) && isdigit( (unsigned char) longName[p] ); p++)
        id = id * 10 + (longName[p] - '0');

    return id;
}

const ClanOrgs * Clan::getOrgs( ) const
{
    return NULL;
}
bool Clan::isHidden( ) const
{
    return true;
}
