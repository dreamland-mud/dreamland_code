#ifndef XMLATTRIBUTEVISITED_H
#define XMLATTRIBUTEVISITED_H

#include <set>
#include "xmlstring.h"
#include "xmlvariablecontainer.h"
#include "playerattributes.h"

class RoomIndexData;

/**
 * Every room a player has ever stood in, by vnum, kept across remorts. Feeds
 * the "every room" achievement. Stored as compact ranges ("3001-3005,3010");
 * the set is built from the string on first use.
 */
class XMLAttributeVisited : public RemortAttribute, public XMLVariableContainer {
XML_OBJECT
public:
    typedef ::Pointer<XMLAttributeVisited> Pointer;

    XMLAttributeVisited();

    /** Remember vnum. True when it is a new room. */
    bool visit(int vnum);

    /** How many of the visited rooms are ordinary ones (see isOrdinary). */
    int countOrdinary() const;

    /** {visited, total}: ordinary rooms visited and in the world. */
    virtual Scripting::Register toRegister() const;

    virtual const DLString & getType( ) const
    {
        return TYPE;
    }

    /** A room every mortal can walk into: not in a system, hidden, clan,
     *  mansion or wizlocked area, and not god-only, imp-only, newbie-only,
     *  nowhere or a mansion room. */
    static bool isOrdinary(RoomIndexData *pRoom);

    /** Ordinary rooms in the world, recounted when the room list changes. */
    static int ordinaryTotal();

    static const DLString TYPE;

    XML_VARIABLE XMLString rooms;

private:
    void parse() const;
    void store();

    mutable std::set<int> vnums;
    mutable bool parsed;
};

#endif
