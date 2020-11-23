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
#include "xmlflags.h"
#include "xmlenumeration.h"
#include "xmlglobalbitvector.h"
#include "skillreference.h"

class Character;

/**
 * Store information about affect(s) applied to a character, object or room.
 */
class Affect : public XMLVariableContainer {
XML_OBJECT;
public:
    typedef ::Pointer<Affect> Pointer;

    Affect();
    virtual ~Affect();

    /** Allocate memory and create an exact copy of this affect. */
    Affect *clone() const;

    /** Populate target affect with all the fields from this one. */
    void copyTo(Affect &target) const;

    /** (unused) find owner of this affect in the world. */
    Character *getOwner() const;

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

    /** (unused) Name of the person who created the affect.*/
    XML_VARIABLE XMLStringNoEmpty ownerName;
};

#endif
