/* $Id$
 *
 * ruffina, 2004
 */
/***************************************************************************
                          affect.h  -  description
                             -------------------
    begin                : Thu Apr 26 2001
    copyright            : (C) 2001 by Igor S. Petrenko
    email                : nofate@europe.com
 ***************************************************************************/

#ifndef AFFECT_H
#define AFFECT_H

#include "xmlvariablecontainer.h"
#include "xmlinteger.h"
#include "xmlstring.h"
#include "xmllonglong.h"
#include "xmllist.h"
#include "xmlflags.h"
#include "xmlenumeration.h"
#include "xmlglobalbitvector.h"
#include "skillreference.h"

class Character;
class Object;
class Room;

class AffectSource : public XMLVariableContainer {
XML_OBJECT;
public:
    AffectSource();
    AffectSource(Character *ch);
    AffectSource(Object *obj);
    AffectSource(Room *room);
    virtual ~AffectSource();

    XML_VARIABLE XMLEnumerationNoEmpty type;
    XML_VARIABLE XMLIntegerNoEmpty ownerVnum;
    XML_VARIABLE XMLStringNoEmpty ownerName;
    XML_VARIABLE XMLLongLong ownerID;
};

/** Keeps information about where the affect is coming from: mob, player, item, room, in any combination. */
class AffectSourceList : public XMLListBase<AffectSource> {
public:
    void add(Character *ch);
    void add(Object *obj);
    void add(Room *room);
    bool contains(const AffectSource &src) const;

    Character *getOwner() const;
};

/**
 * Store information about affect(s) applied to a character, object or room.
 */
class Affect : public XMLVariableContainer {
friend class AffectManager;    
XML_OBJECT;
public:
    typedef ::Pointer<Affect> Pointer;

    Affect();
    virtual ~Affect();

    /** Allocate memory and create an exact copy of this affect. */
    Affect *clone() const;

    /** Populate target affect with all the fields from this one. */
    void copyTo(Affect &target) const;

    /** Describes skil that added this affect, or none. */
    XML_VARIABLE XMLSkillReference type;

    /** Describes strenght of the affect, how easy to dispel. */
    XML_VARIABLE XMLInteger level;

    /** How long the affect applies for, in minutes. -1 for permanent, -2 for permanent that cannot be dispelled. */
    XML_VARIABLE XMLInteger duration;

    /** Which bit the affect modifies. */    
    XML_VARIABLE XMLFlagsWithTable bitvector;

    /** Which numerical field (location) the affect modifies. */
    XML_VARIABLE XMLEnumerationNoEmpty location;

    /** How much the affect would modify a location. */
    XML_VARIABLE XMLIntegerNoEmpty modifier;

    /** How this affect changes fields specified by GlobalRegistry (wearlocation, liquid, skill). */
    XML_VARIABLE XMLGlobalBitvector global;

    /** Where this affect is coming from. */
    XML_VARIABLE AffectSourceList sources;

    /** True if this affect is no longer valid and sits in the extract list. */
    inline bool isExtracted() const { return extracted; }

protected:
    /** Clear and mark this affect as extracted before adding it to extract list. Used by AffectManager. */
    void extract();

    bool extracted;
};

#endif
