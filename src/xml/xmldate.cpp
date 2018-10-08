/* $Id: xmldate.cpp,v 1.7.2.2.28.2 2009/10/11 18:35:38 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 */
/***************************************************************************
                          xmldate.cpp  -  description
                             -------------------
    begin                : Wed May 30 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#include "xmldate.h"
#include "xmllong.h"
#include "exceptionskipvariable.h"

void XMLDate::fromXML( const XMLNode::Pointer& parent ) throw( ExceptionBadType )
{
    XMLNode::Pointer node = parent->getFirstNode( );

    if (!node.isEmpty( )) 
        setTime( Long( node->getCData( ) ) );
}

bool XMLDate::toXML( XMLNode::Pointer& parent ) const
{
    XMLLong value( getTime( ) );

    return value.toXML( parent );
}

bool XMLDateNoEmpty::toXML( XMLNode::Pointer& parent ) const
{
    if (getTime( ) <= 0)
        return false;
    else
        return XMLDate::toXML( parent );
}

