/* $Id: xmlattribute.cpp,v 1.4.2.7.6.1 2007/06/26 07:24:21 rufina Exp $
 *
 * ruffina, 2004
 */

#include "xmlattribute.h"

template class XMLStub<XMLAttribute>;

const DLString & XMLAttribute::getName( ) const
{
    return getType( );
}

void XMLAttribute::init( )
{
}

void XMLAttribute::destroy( )
{
}


