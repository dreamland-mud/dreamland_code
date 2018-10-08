/* $Id: xmllovers.cpp,v 1.1.2.4.6.1 2007/06/26 07:18:04 rufina Exp $
 * ruffina, 2003
 */

#include "class.h"
#include "xmllovers.h"


XMLLovers::XMLLovers( )
{
}

XMLLovers::~XMLLovers( )
{
}

void XMLLovers::put( const DLString name )
{
        (*this)[name] = XMLString( );
}

      

void XMLLovers::erase( const DLString name)
{
        XMLMapBase<XMLString>::erase( find( name ) );
}

bool XMLLovers::isPresent( const DLString name) 
{
        return find( name ) != end( );
}

                        
