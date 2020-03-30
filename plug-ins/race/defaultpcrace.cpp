/* $Id$
 *
 * ruffina, 2004
 */
#include "defaultpcrace.h"
#include "profession.h"
#include "character.h"
#include "merc.h"
#include "def.h"

/*-------------------------------------------------------------------
 * DefaultPCRace
 *------------------------------------------------------------------*/
DefaultPCRace::DefaultPCRace( ) :
          classes( professionManager ),
          stats( &stat_table ),
          align( 0, &align_table )
{
}

const EnumerationArray & DefaultPCRace::getStats( ) const 
{
    return stats;
}

GlobalArray & DefaultPCRace::getClasses( ) 
{
    return classes;
}
int DefaultPCRace::getPoints( ) const
{
    return points.getValue( );
}
int DefaultPCRace::getHpBonus( ) const
{
    return hpBonus.getValue( );
}
int DefaultPCRace::getManaBonus( ) const
{
    return manaBonus.getValue( );
}
int DefaultPCRace::getPracBonus( ) const
{
    return pracBonus.getValue( );
}
const Flags & DefaultPCRace::getAlign( ) const
{
    return align;
}
int DefaultPCRace::getMinAlign( ) const
{
    return minAlign.getValue( );
}
int DefaultPCRace::getMaxAlign( ) const
{
    return maxAlign.getValue( );
}


DLString DefaultPCRace::getWhoNameFor( Character *looker, Character *owner ) const
{
    if (!looker || !looker->getConfig( )->rucommands) 
        return nameWho;

    if (!owner || owner->getSex( ) == SEX_MALE)
        return nameWhoRus;

    return nameWhoFemale;
}

DLString DefaultPCRace::getScoreNameFor( Character *looker, Character *owner ) const
{
    if (!looker || !looker->getConfig( )->rucommands) 
        return name;

    if (!nameScore.empty( ))
        return nameScore;

    if (!owner || owner->getSex( ) == SEX_MALE)
        return nameMale.ruscase( '1' );

    return nameFemale.ruscase( '1' );
}   


