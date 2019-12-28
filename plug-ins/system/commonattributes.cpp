/* $Id: commonattributes.cpp,v 1.1.2.5.6.1 2007/06/26 07:21:39 rufina Exp $
 *
 * ruffina, 2004
 */
#include "json/json.h"
#include "commonattributes.h"
#include "pcmemoryinterface.h"
#include "pcharactermemorylist.h"
#include "pcharactermanager.h"
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

/**
 * Generate Json from player attribute string.
 */
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

/**
 * Create string from Json attribute and save as a player attribute.
 */
void set_json_attribute(PCMemoryInterface *player, const DLString &attrName, Json::Value &attrValue)
{
    XMLStringAttribute::Pointer attr = player->getAttributes().getAttr<XMLStringAttribute>(attrName);
    Json::FastWriter writer;
    attr->setValue(
        writer.write(attrValue));
}

/**
 * Locate first player memory with an attribute of given value.
 */
PCMemoryInterface * find_player_by_attribute(const DLString &attrName, const DLString &attrValue)
{
    const PCharacterMemoryList &pcm = PCharacterManager::getPCM();

    for (const auto &keyValue: pcm) {
        PCMemoryInterface *player = keyValue.second;   
        XMLStringAttribute::Pointer attr = player->getAttributes().findAttr<XMLStringAttribute>(attrName);
        if (attr && attr->getValue() == attrValue)
            return player;
    }

    return 0;
}

/**
 * Locate first player memory with a JSON attribute with given name:value pair.
 */
PCMemoryInterface * find_player_by_json_attribute(const DLString &attrName, const DLString &name, const DLString &value)
{
    const PCharacterMemoryList &pcm = PCharacterManager::getPCM();

    for (const auto &keyValue: pcm) {
        PCMemoryInterface *player = keyValue.second;   
        XMLStringAttribute::Pointer attr = player->getAttributes().findAttr<XMLStringAttribute>(attrName);
        if (!attr)
            continue;

        Json::Reader reader;
        Json::Value attrValue;
        if (!reader.parse(attr->getValue(), attrValue))
            continue;

        if (attrValue[name].asString() == value)
            return player;
    }

    return 0;    
}