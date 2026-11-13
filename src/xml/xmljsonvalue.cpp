#include "xmljsonvalue.h"

XMLJsonValue::XMLJsonValue()
{

}

bool XMLJsonValue::toXML(XMLNode::Pointer& parent) const
{
    // No node for null: a missing node reads back as null anyway, and
    // writing "null" put churn on every prototype without props.
    if (isNull())
        return false;

    XMLNode::Pointer node(NEW);
    
    node->setType(XMLNode::XML_TEXT);

    Json::FastWriter writer;
    node->setCData(writer.write(*this));
    
    parent->appendChild(node);
    return true;
}


void XMLJsonValue::fromXML(const XMLNode::Pointer& parent)
{
    XMLNode::Pointer node = parent->getFirstNode();

    if (!node.isEmpty()) {
        Json::Reader reader;
        reader.parse(node->getCData(), *this);
    }
}
