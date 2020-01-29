/* $Id$
 *
 * ruffina, 2004
 */
#include "remortbonuses_impl.h"
#include "pcharacter.h"
#include "merc.h"
#include "def.h"

/*
 * StatRemortBonus
 */
StatRemortBonus::StatRemortBonus( )
                  : stat( 0, &stat_table )
{
}

DLString StatRemortBonus::getShortDescr( ) const
{
    if (shortDescr.empty( ))
        return stat_table.message( stat );
    else
        return RemortBonus::getShortDescr( );
}

bool StatRemortBonus::matches( const DLString &arg ) const
{
    if (arg == stat.name( )
        || arg == stat.message( '1' )
        || arg == stat.message( '4' ))
        return true;

    if (!shortDescr.empty( )) {
        if (arg == shortDescr.ruscase( '1' ) 
            || arg == shortDescr.ruscase( '4' ))
            return true;

        DLString s = shortDescr;
        s = s.getOneArgument( );

        if (arg == s.ruscase( '1' )
            || arg == s.ruscase( '4' ))
            return true;
    }

    return false;
}

int StatRemortBonus::bonusMaximum( PCharacter *ch ) const 
{
    return MAX_STAT - ch->getMaxStat( stat );
}

int & StatRemortBonus::bonusField( PCharacter *ch ) const
{
    return ch->getRemorts( ).stats[stat];
}

/*
 * LevelRemortBonus
 */
int LevelRemortBonus::bonusMaximum( PCharacter *ch ) const 
{
    return 3 - bonusField( ch );
}

int & LevelRemortBonus::bonusField( PCharacter *ch ) const
{
    return ch->getRemorts( ).level;
}

/*
 * HealthRemortBonus 
 */
void HealthRemortBonus::bonusApply( PCharacter *ch ) const
{
    for (int i = 1; i <= ch->getRealLevel( ); i++) {
        ch->max_hit += ch->getRemorts( ).getHitPerLevel( i );
        ch->perm_hit += ch->getRemorts( ).getHitPerLevel( i );
    }
}

void HealthRemortBonus::bonusRemove( PCharacter *ch ) const
{
    for (int i = 1; i <= ch->getRealLevel( ); i++) {
        ch->max_hit -= ch->getRemorts( ).getHitPerLevel( i );
        ch->perm_hit -= ch->getRemorts( ).getHitPerLevel( i );
    }
}

int HealthRemortBonus::bonusMaximum( PCharacter *ch ) const 
{
    return 0xffff;
}

int & HealthRemortBonus::bonusField( PCharacter *ch ) const
{
    return ch->getRemorts( ).hp;
}

/*
 * ManaRemortBonus 
 */
void ManaRemortBonus::bonusApply( PCharacter *ch ) const
{
    for (int i = 1; i <= ch->getRealLevel( ); i++) {
        ch->max_mana += ch->getRemorts( ).getManaPerLevel( i );
        ch->perm_mana += ch->getRemorts( ).getManaPerLevel( i );
    }
}

void ManaRemortBonus::bonusRemove( PCharacter *ch ) const
{
    for (int i = 1; i <= ch->getRealLevel( ); i++) {
        ch->max_mana -= ch->getRemorts( ).getManaPerLevel( i );
        ch->perm_mana -= ch->getRemorts( ).getManaPerLevel( i );
    }
}

int ManaRemortBonus::bonusMaximum( PCharacter *ch ) const 
{
    return 0xffff;
}

int & ManaRemortBonus::bonusField( PCharacter *ch ) const
{
    return ch->getRemorts( ).mana;
}

/*
 * PretitleRemortBonus
 */
bool & PretitleRemortBonus::bonusField( PCharacter *ch ) const
{
    return ch->getRemorts( ).pretitle;
}

