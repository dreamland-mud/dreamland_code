/* $Id: xmlvariablecontainer.cpp,v 1.17.2.6.8.2 2009/10/11 18:35:39 rufina Exp $
 *
 * ruffina, Dream Land, 2004
 */

#include "xmlvariablecontainer.h"
#include "xmlnode.h"

const DLString & XMLVariableContainer::getType( ) const
{
    return mocGetType( );
}

bool XMLVariableContainer::nodeFromXML( const XMLNode::Pointer& parent ) 
{
    return mocNodeFromXML( parent );
}

bool XMLVariableContainer::toXML( XMLNode::Pointer& parent ) const
{
    return mocToXML( parent );
}

