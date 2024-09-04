#ifndef XMLJSONVALUE_H
#define XMLJSONVALUE_H

#include <jsoncpp/json/json.h>
#include "dlstring.h"
#include "xmlnode.h"
#include "xmlvariable.h"

class XMLJsonValue : public Json::Value {
public:
    XMLJsonValue();

    bool toXML(XMLNode::Pointer& node) const;
    void fromXML(const XMLNode::Pointer& node);
};

#endif