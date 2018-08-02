/* $Id$
 *
 * ruffina, 2004
 */
#include "defaulthometown.h"

#include "pcharacter.h"

void DefaultHometown::loaded( )
{
    hometownManager->registrate( Pointer( this ) );
}

void DefaultHometown::unloaded( )
{
    hometownManager->unregistrate( Pointer( this ) );
}

int DefaultHometown::getAltar( ) const
{
    return altar.getValue( );
}

int DefaultHometown::getRecall( ) const
{
    return recall.getValue( );
}

int DefaultHometown::getPit( ) const
{
    return pit.getValue( );
}

int DefaultHometown::getLanding( ) const
{
    return landing.getValue( );
}
    
bool DefaultHometown::isAllowed( PCharacter *pch ) const
{
    return limits.allow( pch );
}

