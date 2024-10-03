#include <sstream>
#include "xmlcounter.h"
#include "stringlist.h"
#include "xmlstring.h"
#include "exceptionbadtype.h"
#include "integer.h"

DLString XMLCounter::toString() const
{
    ostringstream buf;

    for (auto pair: *this) {
        buf << pair.first << ":" << pair.second << " ";
    }

    return buf.str();
}

void XMLCounter::fromString(const DLString &value)
{
    this->clear();

    for (auto pair: value.split(" ")) {
        auto keyValue = pair.split(":");

        if (keyValue.size() != 2)
            throw ExceptionBadType("XMLCounter bad pair " + pair);

        Integer key, value;
        if (!Integer::tryParse(key, keyValue.front()))
            throw ExceptionBadType("XMLCounter bad key " + keyValue.front() + " for  pair " + pair);
        
        if (!Integer::tryParse(value, keyValue.back()))
            throw ExceptionBadType("XMLCounter bad value " + keyValue.back() + " for  pair " + pair);

        (*this)[key] = value;
    }
}

bool XMLCounter::toXML(XMLNode::Pointer& node) const
{
    return XMLString(toString()).toXML(node);
}

void XMLCounter::fromXML(const XMLNode::Pointer& node)
{
    XMLString xmlString;
    xmlString.fromXML(node);
    fromString(xmlString.getValue());
}
