/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __OLCSAVE_ROOM_H__
#define __OLCSAVE_ROOM_H__

#include <list>

#include "xmlvariablecontainer.h"
#include "xmlstring.h"
#include "xmlflags.h"
#include "xmlinteger.h"
#include "xmlmap.h"
#include "xmllist.h"
#include "xmljsonvalue.h"
#include "xmlglobalbitvector.h"
#include "xmlmisc.h"
#include "merc.h"

class XMLResetList : public std::list<reset_data>, public virtual XMLContainer {
    typedef std::list<reset_data> List;
public:
    using List::clear;
    using List::back;
    using List::end;
    using List::begin;
    using List::empty;
    
    virtual bool nodeFromXML(const XMLNode::Pointer &child);
    virtual bool toXML(XMLNode::Pointer &parent) const;
};

class RoomIndexData;

class XMLRoom : public XMLVariableContainer {
XML_OBJECT
public:
    XMLRoom();

    void init(RoomIndexData *);
    RoomIndexData *compat(int vnum);

    XML_VARIABLE XMLMultiString name;
    XML_VARIABLE XMLMultiString description;
    XML_VARIABLE XMLFlagsNoEmpty flags;
    XML_VARIABLE XMLEnumeration sector;
    XML_VARIABLE XMLStringNoEmpty clan, guilds;
    XML_VARIABLE XMLMapBase<XMLExitDir> exits;
    XML_VARIABLE XMLMapBase<XMLExtraExit> extraExits; // replace with a list after 1st convertation
    XML_VARIABLE XMLListBase<XMLExtraDescr> extraDescr; // compat
    XML_VARIABLE XMLListBase<XMLExtraDescription> extraDescriptions;
    XML_VARIABLE XMLIntegerNoDef<100> manaRate, healRate;
    XML_VARIABLE XMLStringNoEmpty liquid;
    XML_VARIABLE XMLMultiString smell;
    XML_VARIABLE XMLMultiString sound;
    XML_VARIABLE XMLResetList resets;
    XML_VARIABLE XMLStringNode behavior;
    XML_VARIABLE XMLGlobalBitvector behaviors;
    XML_VARIABLE XMLJsonValue props;
};

#endif
