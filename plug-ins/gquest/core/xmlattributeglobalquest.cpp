/* $Id: xmlattributeglobalquest.cpp,v 1.1.2.1 2005/09/10 21:13:00 rufina Exp $
 *
 * ruffina, 2003
 */

#include "xmlattributeglobalquest.h"

XMLAttributeGlobalQuest::XMLAttributeGlobalQuest( ) : noexp( false )
{
}

bool XMLAttributeGlobalQuest::getNoExp( ) const
{
    return noexp.getValue( );
}

void XMLAttributeGlobalQuest::setNoExp( bool value ) 
{
    noexp.setValue( value );
}

bool XMLAttributeGlobalQuest::isJoined( ) const
{
    return false;
}    

