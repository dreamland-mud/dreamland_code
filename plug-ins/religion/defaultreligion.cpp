/* $Id$
 *
 * ruffina, 2004
 */
#include "defaultreligion.h"

#include "character.h"

#include "merc.h"
#include "def.h"

/*----------------------------------------------------------------------
 * DefaultReligion 
 *---------------------------------------------------------------------*/
DefaultReligion::DefaultReligion( )
                : align( 0, &align_table ),
                  ethos( 0, &ethos_table )
{
}


const DLString & DefaultReligion::getName( ) const
{
    return Religion::getName( );
}

void DefaultReligion::setName( const DLString &name ) 
{
    this->name = name;
}

bool DefaultReligion::isValid( ) const
{
    return true;
}

bool DefaultReligion::isAllowed( Character *ch ) const
{
    if (!ethos.isSetBitNumber( ch->ethos ))
        return false;

    if (!align.isSetBitNumber( ALIGNMENT(ch) ))
        return false;

    return true;
}

const DLString &DefaultReligion::getRussianName( ) const
{
    return nameRus;
}

const DLString& DefaultReligion::getNameFor( Character *looker ) const
{
    if (!looker || !looker->getConfig( )->ruskills) 
        return shortDescr;

    return nameRus;
}

void DefaultReligion::loaded( )
{
    religionManager->registrate( Pointer( this ) );
}

void DefaultReligion::unloaded( )
{
    religionManager->unregistrate( Pointer( this ) );
}

const DLString & DefaultReligion::getShortDescr( ) const
{
    return shortDescr.getValue( );
}

const DLString & DefaultReligion::getDescription( ) const
{
    return description.getValue( );
}

