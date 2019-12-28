/* $Id: commonattributes.cpp,v 1.1.2.5.6.1 2007/06/26 07:21:39 rufina Exp $
 *
 * ruffina, 2004
 */
#include "json/json.h"
#include "commonattributes.h"
#include "pcmemoryinterface.h"
#include "xmlattributes.h"
#include "logstream.h"

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

const DLString & get_string_attribute(PCMemoryInterface *player, const DLString &attrName)
{
    XMLStringAttribute::Pointer attr = player->getAttributes().getAttr<XMLStringAttribute>(attrName);
    return attr->getValue();
}

bool get_json_attribute(PCMemoryInterface *player, const DLString &attrName, Json::Value &attrValue)
{
    const DLString &attrString = get_string_attribute(player, attrName);
    Json::Reader reader;
    if (!reader.parse(attrString, attrValue)) {
        LogStream::sendNotice() << "Error parsing JSON attribute " << attrString << " for " << player->getName() << endl;
        return false;
    }

    return true;
}

void set_json_attribute(PCMemoryInterface *player, const DLString &attrName, Json::Value &attrValue)
{
    XMLStringAttribute::Pointer attr = player->getAttributes().getAttr<XMLStringAttribute>(attrName);
    Json::FastWriter writer;
    attr->setValue(
        writer.write(attrValue));
}