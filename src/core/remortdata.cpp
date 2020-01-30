/* $Id: remortdata.cpp,v 1.1.2.7.4.3 2008/05/21 08:15:31 rufina Exp $
 * 
 * ruffina, 2004
 */
#include "remortdata.h"
#include "class.h"
#include "logstream.h"
#include "pcharacter.h"
#include "merc.h"
#include "def.h"

const int Remorts::POINT_PER_LIFE = 10;
const int Remorts::MAX_BONUS_LIFES = 10;

LifeData::LifeData( ) : bonus( true )
{
}


Remorts::Remorts( ) : pretitle( false )
{
    clear( );
}

int Remorts::countBonusLifes( ) const
{
    const_iterator i;
    int cnt;
    
    for (cnt = 0, i = begin( ); i != end( ); i++)
        if (i->bonus.getValue( ))
            cnt++;
    
    return min( cnt, MAX_BONUS_LIFES );
}

void Remorts::clear( )
{
    XMLVectorBase<LifeData>::clear( );
    clearBonuses( );
    owners = 0;
}

void Remorts::clearBonuses( )
{
    stats.clear( );
    stats.resize( stat_table.size, 0 );
    
    level = 0;
    hp = mana = 0;
    points = 0;
    pretitle = false;
}

void Remorts::resetBonuses( )
{
    clearBonuses( );
    points = POINT_PER_LIFE * countBonusLifes( ); 
}

int Remorts::getHitPerLevel( int level )
{
    return bonusPerLevel( hp, level );
}
int Remorts::getManaPerLevel( int level )
{
    return bonusPerLevel( mana, level );
}

int Remorts::bonusPerLevel( int bonus, int level )
{
    int r;

    r = bonus / 100;

    if (bonus % 100)
        r += (level <= bonus % 100) ? 1 : 0;
    
    return r;
}


