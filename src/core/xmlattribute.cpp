/* $Id: xmlattribute.cpp,v 1.4.2.7.6.1 2007/06/26 07:24:21 rufina Exp $
 *
 * ruffina, 2004
 */

#include "xmlattribute.h"
#include "register-impl.h"

template class XMLStub<XMLAttribute>;

const DLString & XMLAttribute::getName( ) const
{
    return getType( );
}

Scripting::Register XMLAttribute::toRegister() const
{
    return Scripting::Register();
}

void XMLAttribute::init( )
{
}

void XMLAttribute::destroy( )
{
}


