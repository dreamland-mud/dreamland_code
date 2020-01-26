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

const DLString XMLStringMapAttribute::TYPE = "XMLStringMapAttribute";

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

XMLStringMapAttribute::XMLStringMapAttribute( )
{
}

XMLStringMapAttribute::~XMLStringMapAttribute( )
{
}

const DLString & get_string_attribute(PCMemoryInterface *player, const DLString &attrName)
{
    XMLStringAttribute::Pointer attr = player->getAttributes().getAttr<XMLStringAttribute>(attrName);
    return attr->getValue();
}

/**
 * Find an entry in a map attribute with given name, or return empty string.
 */
const DLString & get_map_attribute_value(PCMemoryInterface *player, const DLString &attrName, const DLString &key)
{
    XMLStringMapAttribute::Pointer map = player->getAttributes().findAttr<XMLStringMapAttribute>(attrName);
    if (!map)
        return DLString::emptyString;

    XMLStringMapAttribute::const_iterator entry = map->find(key);
    if (entry == map->end())
        return DLString::emptyString;

    return entry->second;
}

/**
 * Set a key-value pair inside a map attribute with a given name.
 */
void set_map_attribute_value(PCMemoryInterface *player, const DLString &attrName, const DLString &key, const DLString &value)
{
    XMLStringMapAttribute::Pointer map = player->getAttributes().getAttr<XMLStringMapAttribute>(attrName);
    (**map)[key] = value;
}

XMLStringMapAttribute & get_map_attribute(PCMemoryInterface *player, const DLString &attrName)
{
    XMLStringMapAttribute::Pointer map = player->getAttributes().getAttr<XMLStringMapAttribute>(attrName);
    return **map;
}

/**
 * Generate Json from player attribute string.
 */
bool get_json_attribute(PCMemoryInterface *player, const DLString &attrName, Json::Value &attrValue)
{
    const DLString &attrString = get_string_attribute(player, attrName);
    if (attrString.empty())
        return false;
        
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
list<PCMemoryInterface *> find_players_by_json_attribute(const DLString &attrName, const DLString &name, const DLString &value)
{
    const PCharacterMemoryList &pcm = PCharacterManager::getPCM();
    list<PCMemoryInterface *> result;

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
            result.push_back(player);
    }

    return result;    
}