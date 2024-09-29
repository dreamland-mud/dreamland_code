#ifndef XMLCOUNTER_H
#define XMLCOUNTER_H

#include <map>
#include "xmlnode.h"

using namespace std;

/** Represets various counters. Acts as a map at runtime, and persists to XML as a string of key-value pairs separated by : */
class XMLCounter : public map<int, int> {
public:    
    DLString toString() const;
    void fromString(const DLString &value);

    bool toXML(XMLNode::Pointer& node) const;
    void fromXML(const XMLNode::Pointer& node);
};

#endif