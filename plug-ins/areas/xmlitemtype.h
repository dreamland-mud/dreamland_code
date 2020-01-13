/* $Id$
 *
 * ruffina, 2004
 */

#ifndef __OLCSAVE_ITEMTYPE_H__
#define __OLCSAVE_ITEMTYPE_H__

#include "xmlvariablecontainer.h"
#include "xmlinteger.h"
#include "xmlboolean.h"
#include "xmlstring.h"
#include "xmlflags.h"
#include "xmlenumeration.h"
#include "skillreference.h"
#include "liquid.h"

#include "xmlmisc.h"

struct XMLItemType {
    bool toXML( XMLNode::Pointer& ) const;
    void fromXML( const XMLNode::Pointer& );

    bitstring_t type;
    int v[5];
};

class XMLItemTypeValuesDefault : public XMLVariableContainer {
XML_OBJECT
public:
    XML_VARIABLE XMLIntegerNoEmpty v0, v1, v2, v3, v4;
};

class XMLItemTypeValuesMoney : public XMLVariableContainer {
XML_OBJECT
public:
    XML_VARIABLE XMLInteger silver, gold;
    XML_VARIABLE XMLIntegerNoEmpty reserved0, reserved1, reserved2;
};

class XMLItemTypeValuesDrink : public XMLVariableContainer {
XML_OBJECT
public:
    XMLItemTypeValuesDrink( );

    XML_VARIABLE XMLIntegerNoEmpty total, left;
    XML_VARIABLE XMLLiquidReference liquid;
    XML_VARIABLE XMLFlagsNoEmpty flags;
    XML_VARIABLE XMLIntegerNoEmpty corkscrew;
};

class XMLItemTypeValuesContainer : public XMLVariableContainer {
XML_OBJECT
public:
    XMLItemTypeValuesContainer( );

    XML_VARIABLE XMLInteger capacity;
    XML_VARIABLE XMLFlagsNoEmpty flags;
    XML_VARIABLE XMLIntegerNoEmpty key;
    XML_VARIABLE XMLIntegerNoEmpty maxWeight;
    XML_VARIABLE XMLIntegerNoDef<100> weightMult;
};

class XMLItemTypeValuesBoat : public XMLVariableContainer {
XML_OBJECT
public:
    XMLItemTypeValuesBoat( );

    XML_VARIABLE XMLIntegerNoEmpty move_type;
    XML_VARIABLE XMLEnumeration position;
    XML_VARIABLE XMLFlagsNoEmpty flags;
    XML_VARIABLE XMLIntegerNoEmpty reserved0, reserved1;
};

class XMLItemTypeValuesFood : public XMLVariableContainer {
XML_OBJECT
public:
    XMLItemTypeValuesFood( );

    XML_VARIABLE XMLIntegerNoEmpty foodHours, fullHours;
    XML_VARIABLE XMLIntegerNoEmpty reserved0;
    XML_VARIABLE XMLBooleanNoFalse poisoned;
    XML_VARIABLE XMLIntegerNoEmpty reserved1;
};

class XMLItemTypeValuesPortal : public XMLVariableContainer {
XML_OBJECT
public:
    XMLItemTypeValuesPortal( );
    
    XML_VARIABLE XMLInteger charges;
    XML_VARIABLE XMLFlagsNoEmpty exitFlags, portalFlags;
    XML_VARIABLE XMLIntegerNoEmpty target, key;
};

class XMLItemTypeValuesFurniture : public XMLVariableContainer {
XML_OBJECT
public:
    XMLItemTypeValuesFurniture( );

    XML_VARIABLE XMLInteger maxPeople, maxWeight;
    XML_VARIABLE XMLFlagsNoEmpty flags;
    XML_VARIABLE XMLIntegerNoDef<100> heal, mana;
};

class XMLItemTypeValuesWeapon : public XMLVariableContainer {
XML_OBJECT
public:
    XMLItemTypeValuesWeapon( );

    XML_VARIABLE XMLEnumeration weapClass;
    XML_VARIABLE XMLInteger diceNumber, diceType;
    XML_VARIABLE XMLEnumeration weapType;
    XML_VARIABLE XMLFlagsNoEmpty specType;
};

class XMLItemTypeValuesRecipe : public XMLVariableContainer {
XML_OBJECT
public:
    XMLItemTypeValuesRecipe();
    XML_VARIABLE XMLFlagsNoEmpty flags;
    XML_VARIABLE XMLInteger complexity;
};    

class XMLItemTypeValuesArmor : public XMLVariableContainer {
XML_OBJECT
public:
    XML_VARIABLE XMLArmor ac;
    XML_VARIABLE XMLIntegerNoEmpty reserved0;
};

class XMLItemTypeValuesScrollPotionPill : public XMLVariableContainer {
XML_OBJECT
public:
    XML_VARIABLE XMLInteger level;
    XML_VARIABLE XMLSkillReference spell0, spell1, spell2, spell3;
};

class XMLItemTypeValuesStaffWand : public XMLVariableContainer {
XML_OBJECT
public:
    XML_VARIABLE XMLInteger level, total, left;
    XML_VARIABLE XMLSkillReference spell;
    XML_VARIABLE XMLIntegerNoEmpty reserved0;
};

class XMLItemTypeValuesSpellBook : public XMLVariableContainer {
XML_OBJECT
public:
    XML_VARIABLE XMLInteger total, used, quality;
    XML_VARIABLE XMLSkillReference skill0;
    XML_VARIABLE XMLSkillReference skill1;
};

class XMLItemTypeValuesKey : public XMLVariableContainer {
XML_OBJECT
public:
    XML_VARIABLE XMLInteger rotInv;
    XML_VARIABLE XMLInteger rotGround;
    XML_VARIABLE XMLIntegerNoEmpty reserved0, reserved1, reserved2;
};

class XMLItemTypeValuesKeyring : public XMLVariableContainer {
XML_OBJECT
public:
    XML_VARIABLE XMLInteger maxKeys;
    XML_VARIABLE XMLIntegerNoEmpty reserved0, reserved1, reserved2, reserved3;
};

#endif
