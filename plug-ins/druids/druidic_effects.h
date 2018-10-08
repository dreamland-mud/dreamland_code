/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __DRUIDIC_EFFECTS_H__
#define __DRUIDIC_EFFECTS_H__

#include "objectbehavior.h"
#include "defaultaffecthandler.h"
#include "wordeffect.h"

class NPCharacter;
class Affect;

class DruidSpiritAffectHandler : public DefaultAffectHandler {
XML_OBJECT
public:
    typedef ::Pointer<DruidSpiritAffectHandler> Pointer;
    
    virtual void update( Character *, Affect * ); 
    virtual void stopfol( Character *, Affect * );

    XML_VARIABLE XMLString msgSelf, msgOther, msgVict;
};

// MOC_SKIP_BEGIN
#if 0
class AnimalSpiritWE : public WordEffect {
XML_OBJECT    
public:
    typedef ::Pointer<AnimalSpiritWE> Pointer;

    virtual bool run( PCharacter *, Object * ) const;
};

class AnimalSpiritComponent : public ObjectBehavior {
XML_OBJECT    
public:
    typedef ::Pointer<AnimalSpiritComponent> Pointer;

    virtual void runEffect( PCharacter * ) = 0;
protected:
    virtual int getChance( PCharacter * ) const;
    bool checkComponent( PCharacter *, Object * ) const;
    bool applyPositiveAffect( PCharacter *, Affect * ) const;
    bool applyNegativeAffect( PCharacter *, Affect * ) const;
    DruidSpiritAffectHandler::Pointer getAffectHandler( Affect * ) const;
                              
    XML_VARIABLE XMLBoolean good;
};

class SnakeSpiritComponent : public AnimalSpiritComponent {
XML_OBJECT    
public:
    typedef ::Pointer<SnakeSpiritComponent> Pointer;
    virtual void runEffect( PCharacter * );
};

class FoxSpiritComponent : public AnimalSpiritComponent {
XML_OBJECT    
public:
    typedef ::Pointer<FoxSpiritComponent> Pointer;
    virtual void runEffect( PCharacter * );
};

class BoarSpiritComponent : public AnimalSpiritComponent {
XML_OBJECT    
public:
    typedef ::Pointer<BoarSpiritComponent> Pointer;
    virtual void runEffect( PCharacter * );
};

class WolverineSpiritComponent : public AnimalSpiritComponent {
XML_OBJECT    
public:
    typedef ::Pointer<WolverineSpiritComponent> Pointer;
    virtual void runEffect( PCharacter * );
};

class ForestFaerySpiritComponent : public AnimalSpiritComponent {
XML_OBJECT    
public:
    typedef ::Pointer<ForestFaerySpiritComponent> Pointer;
    virtual void runEffect( PCharacter * );
};

class ForestTrollSpiritComponent : public AnimalSpiritComponent {
XML_OBJECT    
public:
    typedef ::Pointer<ForestTrollSpiritComponent> Pointer;
    virtual void runEffect( PCharacter * );
};

class DryadSpiritComponent : public AnimalSpiritComponent {
XML_OBJECT    
public:
    typedef ::Pointer<DryadSpiritComponent> Pointer;
    virtual void runEffect( PCharacter * );
};

#endif 
// MOC_SKIP_END
#endif
