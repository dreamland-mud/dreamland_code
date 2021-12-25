/* $Id: anatolia_limits.h,v 1.1.2.1.6.1 2007/09/11 00:32:10 rufina Exp $
 *
 * ruffina, 2005
 */

#ifndef ANATOLIA_LIMITS_H 
#define ANATOLIA_LIMITS_H 

#include "objectbehaviormanager.h"

class DemonfireShield : public BasicObjectBehavior {
XML_OBJECT
public:
        typedef ::Pointer<DemonfireShield> Pointer;
    
        virtual void wear( Character *victim );                           
        virtual void remove( Character *victim );
        virtual void fight( Character *ch );
};

class Excalibur : public BasicObjectBehavior {
XML_OBJECT
public:
        typedef ::Pointer<Excalibur> Pointer;
    
        virtual void wear( Character *victim );                           
        virtual void equip( Character *victim );                           
        virtual void remove( Character *victim );
        virtual bool sac( Character *victim );
        virtual bool death( Character *victim );
        virtual void speech( Character *victim, const char *speech );
};

class FireGauntlets : public BasicObjectBehavior {
XML_OBJECT
public:
        typedef ::Pointer<FireGauntlets> Pointer;
    
        virtual void wear( Character *victim );                           
        virtual void remove( Character *victim );
        virtual void fight( Character *ch );
};

class FlyingBoots : public BasicObjectBehavior {
XML_OBJECT
public:
        typedef ::Pointer<FlyingBoots> Pointer;
    
        virtual void wear( Character *victim );                           
        virtual void equip( Character *victim );                           
        virtual void remove( Character *victim );
};

class GiantStrengthArmor : public BasicObjectBehavior {
XML_OBJECT
public:
        typedef ::Pointer<GiantStrengthArmor> Pointer;
    
        virtual void wear( Character *victim );                           
        virtual void equip( Character *victim );                           
        virtual void remove( Character *victim );
};


class HasteBracers : public BasicObjectBehavior {
XML_OBJECT
public:
        typedef ::Pointer<HasteBracers> Pointer;

        virtual void wear( Character *victim );                           
        virtual void equip( Character *victim );                           
        virtual void remove( Character *victim );
};

class LionClaw : public BasicObjectBehavior {
XML_OBJECT
public:
        typedef ::Pointer<LionClaw> Pointer;
    
        virtual void fight( Character *ch );
};

class RingOfRa : public BasicObjectBehavior {
XML_OBJECT
public:
        typedef ::Pointer<RingOfRa> Pointer;
    
        virtual void speech( Character *victim, const char *speech );
};

class RoseShield : public BasicObjectBehavior {
XML_OBJECT
public:
        typedef ::Pointer<RoseShield> Pointer;
    
        virtual void fight( Character *ch );
};


class SubissueWeapon : public BasicObjectBehavior {
XML_OBJECT
public:
        typedef ::Pointer<SubissueWeapon> Pointer;
    
        virtual void fight( Character *ch );
};


class Thunderbolt : public BasicObjectBehavior {
XML_OBJECT
public:
        typedef ::Pointer<Thunderbolt> Pointer;
    
        virtual void fight( Character *ch );
};


class TwoSnakeWhip : public BasicObjectBehavior {
XML_OBJECT
public:
        typedef ::Pointer<TwoSnakeWhip> Pointer;
    
        virtual void wear( Character *victim );                           
        virtual void equip( Character *victim );                           
        virtual void remove( Character *victim );
        virtual void get( Character *victim );
        virtual void fight( Character *ch );
};

class VolcanoeArmbands : public BasicObjectBehavior {
XML_OBJECT
public:
        typedef ::Pointer<VolcanoeArmbands> Pointer;
    
        virtual void wear( Character *victim );                           
        virtual void remove( Character *victim );
        virtual void fight( Character *ch );
};

#endif

