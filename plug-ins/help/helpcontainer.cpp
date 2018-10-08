/* $Id$
 *
 * ruffina, 2004
 */
#include "helpcontainer.h"

HelpContainer::~HelpContainer( )
{
}

void HelpContainer::loaded( )
{
    for (iterator i = begin( ); i != end( ); i++) {
        helpManager->registrate( *i );
    }
}

void HelpContainer::unloaded( )
{
    for (iterator i = begin( ); i != end( ); i++) {
        helpManager->unregistrate( *i );
    }
}


