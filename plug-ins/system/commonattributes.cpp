/* $Id: commonattributes.cpp,v 1.1.2.5.6.1 2007/06/26 07:21:39 rufina Exp $
 *
 * ruffina, 2004
 */
#include "commonattributes.h"


const DLString XMLEmptyAttribute::TYPE = "XMLEmptyAttribute";

const DLString XMLStringAttribute::TYPE = "XMLStringAttribute";

const DLString XMLIntegerAttribute::TYPE = "XMLIntegerAttribute";

const DLString XMLStringListAttribute::TYPE = "XMLStringListAttribute";

XMLEmptyAttribute::XMLEmptyAttribute( )
{
}
XMLEmptyAttribute::~XMLEmptyAttribute( )
{
}

void XMLEmptyAttribute::fromXML( const XMLNode::Pointer& node ) 
{
}

bool XMLEmptyAttribute::toXML( XMLNode::Pointer& node ) const
{
    return true;
}

XMLStringAttribute::XMLStringAttribute( )
{
}

XMLStringAttribute::~XMLStringAttribute( )
{
}

XMLIntegerAttribute::XMLIntegerAttribute( )
{
}

XMLIntegerAttribute::~XMLIntegerAttribute( )
{
}

XMLStringListAttribute::XMLStringListAttribute( )
{
}

XMLStringListAttribute::~XMLStringListAttribute( )
{
}

