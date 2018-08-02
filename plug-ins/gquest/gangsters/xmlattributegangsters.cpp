/* $Id: xmlattributegangsters.cpp,v 1.1.2.1 2005/09/10 21:13:02 rufina Exp $
 *
 * ruffina, 2003
 */

#include "xmlattributegangsters.h"

XMLAttributeGangsters::XMLAttributeGangsters( ) 
	    : XMLAttributeGlobalQuest( ), joined( false ) 
{
}

int XMLAttributeGangsters::getKilled( ) const 
{
    return killed.getValue( );
}

void XMLAttributeGangsters::setKilled( int i ) 
{
    killed.setValue( i );
}

bool XMLAttributeGangsters::isJoined( )  const
{
    return joined.getValue( ) || killed.getValue( ) > 0;
}

void XMLAttributeGangsters::setJoined( )
{
    joined.setValue( true );
}

