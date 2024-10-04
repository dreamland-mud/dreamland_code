#include "xmlmultistring.h"
#include "exception.h"
#include "string_utils.h"

const DLString ATTR_LANG = "l";

static lang_t attr2lang(const DLString langAttr)
{
    if (langAttr == "en")
        return EN;

    if (langAttr == "ua")
        return UA;

    if (langAttr == "ru")
        return RU;

    return LANG_DEFAULT;
}

static DLString lang2attr(lang_t lang)
{
    switch (lang) {
        case EN: return "en";
        case UA: return "ua";
        case RU: return "ru";
        default: return "en";
    }
}

XMLMultiString::XMLMultiString()
{
    for (int lang = LANG_MIN; lang < LANG_MAX; lang++)
        (*this)[(lang_t)lang] = DLString::emptyString;
}

const DLString &XMLMultiString::get(lang_t lang) const
{
    if (lang < LANG_MIN || lang >= LANG_MAX)
        return DLString::emptyString;
        
    return find(lang)->second;
}

// Translate:
// <names l="en">burning hands</names>
// <names l="ua">палаючі руки</names>
// <names l="ru">горящие руки</names>
// into a map of 3 strings. Expect fromXML to be invoked 3 times.
void XMLMultiString::fromXML(const XMLNode::Pointer& parent)
{
    XMLNode::Pointer node = parent->getFirstNode();
    DLString cdata = node ? node->getCData() : DLString::emptyString;
    DLString langAttr = parent->getAttribute(ATTR_LANG);
    lang_t lang; 
    
    // For legacy nodes w/o 'l' attribute, guess the lang from the node content.
    if (langAttr.empty())
        lang = String::hasCyrillic(cdata) ? RU : EN;
    else
        lang = attr2lang(langAttr);

    (*this)[lang] = cdata;
}

// Append 3 <names l="en"> etc nodes to this parent's parent.
// Parent node is <names>, its parent is e.g. <Skill>, output is:
// <Skill>
//  <names l="en"> ... </names>
//  <names l="ua"> ... </names>
// ...
// </Skill>
bool XMLMultiString::toXML(XMLNode::Pointer& parent) const
{
    XMLNode::Pointer grandma = parent->getParent();
    if (grandma.isEmpty())
        throw Exception("Nowhere to append a multi-string");

    for (int lang = LANG_MIN; lang < LANG_MAX; lang++) {
        DLString value = find((lang_t)lang)->second;

        XMLNode::Pointer target;

        // Choose where to append the node for this language entry.
        if (lang == LANG_MIN) {
            // Populate this parent with the EN value.
            target = parent;

        } else {
            // For remaining languages, add sibling nodes to the parent node.
            XMLNode::Pointer auntie(NEW);
            auntie->setName(parent->getName());
            auntie->setType(parent->getType());
            grandma->appendChild(auntie);
            target = auntie;
        }

        // Generate the CDATA node with current translation.
        XMLNode::Pointer node(NEW);        
        node->setType(XMLNode::XML_TEXT);
        node->setCData(value);

        // Stick it to the chosen parent.
        target->appendChild(node);
        target->insertAttribute(ATTR_LANG, lang2attr((lang_t)lang));
    }

    return true;
}

