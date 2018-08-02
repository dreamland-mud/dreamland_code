/* $Id: xmllimits.cpp,v 1.1.2.4.24.3 2009/10/11 18:35:38 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */

#include "xmllimits.h"

const DLString XMLLimits::TYPE = "XMLLimits";

XMLLimits::XMLLimits( )
	     : XMLClausesList( false )
{
}

XMLLimits::~XMLLimits( )
{
}

bool XMLLimits::allow( DLObject * arg ) const 
{
    for (const_iterator i = begin( ); i != end( ); i++) 
	if (i->match( arg )) 
	    return i->allow;
    
    return true;
}
