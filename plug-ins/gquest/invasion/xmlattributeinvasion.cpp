/* $Id: xmlattributeinvasion.cpp,v 1.1.2.1 2005/09/10 21:13:02 rufina Exp $
 *
 * ruffina, 2004
 */

#include "xmlattributeinvasion.h"
#include "class.h"
#include "so.h"                                                                 

XMLAttributeInvasion::XMLAttributeInvasion( )
                        : punished( false )
{
}

int XMLAttributeInvasion::getKilled( ) const 
{
    return killed.getValue( );
}

void XMLAttributeInvasion::setKilled( int i ) 
{
    killed.setValue( i );
}

bool XMLAttributeInvasion::isPunished( ) const
{
    return punished.getValue( );
}

void XMLAttributeInvasion::punish( ) 
{
    punished.setValue( true );
}
