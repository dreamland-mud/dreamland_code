#include "xmlmultistring.h"
#include "exception.h"
#include "string_utils.h"
#include "stringlist.h"
#include "dl_strings.h"

const DLString ATTR_LANG = "l";

// TODO move to separate utils file
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
    // Quick check to skip totally empty values
    bool empty = true;
    for (auto &lv: *this) {
        if (!lv.second.empty())
            empty = false;
    }

    if (empty)
        return false;
        
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



bool XMLMultiString::matchesStrict( const DLString &str ) const 
{
    if (str.empty())
        return false;

    DLString lstr = str.toLower();

    for (int i = LANG_MIN; i < LANG_MAX; i++) {
        lang_t lang = (lang_t)i;
        if (get(lang).toLower() == lstr)
            return true;
    }

    return false;
}

bool XMLMultiString::matchesUnstrict( const DLString &str ) const 
{
    if (str.empty())
        return false;

    DLString lstr = str.toLower();

    for (int i = LANG_MIN; i < LANG_MAX; i++) {
        lang_t lang = (lang_t)i;
        DLString lname = get(lang).toLower();

        if (lname.find('|') != DLString::npos)
            lname = russian_case_all_forms(lname);

        if (is_name(lstr.c_str(), lname.c_str()))
            return true;
    }
    
    return false;
}

bool XMLMultiString::matchesSubstring( const DLString &str ) const 
{
    if (str.empty())
        return false;

    DLString lstr = str.toLower();

    for (int i = LANG_MIN; i < LANG_MAX; i++) {
        lang_t lang = (lang_t)i;
        if (lstr.strPrefix(get(lang).toLower()))
            return true;
    }

    return false;
}

void XMLMultiString::fromMixedString(const DLString &str)
{
    StringList words(str);
    StringList en, ru;

    for (auto &word: words) 
        if (String::hasCyrillic(word))
            ru.push_back(word);
        else
            en.push_back(word);

    (*this)[EN] = en.toString();
    (*this)[RU] = ru.toString();
}