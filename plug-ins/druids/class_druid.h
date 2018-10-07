/* $Id$
 *
 * ruffina, 2004
 */
#ifndef CLASS_DRUID_H
#define CLASS_DRUID_H

#include "objectbehavior.h"
#include "basicmobilebehavior.h"
#include "savedcreature.h"

class Character;

class DruidStaff : public ObjectBehavior {
XML_OBJECT
public:
    typedef ::Pointer<DruidStaff> Pointer;

    virtual bool death( Character * );
    virtual void fight( Character * );
    virtual bool canEquip( Character * );
};

class DruidSummonedAnimal : public SavedCreature,
                            public BasicMobileDestiny 
{
XML_OBJECT
public:
    typedef ::Pointer<DruidSummonedAnimal> Pointer;
    
    virtual ~DruidSummonedAnimal( );

    bool myHero( Character *ch ) const;

    XML_VARIABLE XMLString heroName;
    XML_VARIABLE XMLInteger biteQuality;
};

#endif
