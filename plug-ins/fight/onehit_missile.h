/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __ONEHIT_MISSILE_H__
#define __ONEHIT_MISSILE_H__

#include "onehit.h"

#ifndef FIGHT_STUB
class MissileOneHit : public OneHit {
public:
    MissileOneHit( Character *ch, Character *victim, Object *missile, int range );
    
    virtual void init( );
    virtual bool canDamage( );
    virtual void calcTHAC0( );
    virtual void calcDamage( );
    virtual void postDamageEffects( );

protected:
    bool defenseParry( );
    bool defenseShieldBlock( );
    bool defenseDodge( );
        
    virtual void damBase( );
    void damApplySharp( );
    void damApplyHoly( );
    void damApplyAccuracy( );
    void damApplyDamroll( );
    void damApplyMissile( );
    void damApplyStrength( );

    void thacApplyMissile( );

    void damEffectFunkyWeapon( );
    void damEffectStucking( );

    virtual bool mprog_immune();

    int range;
    Object *missile;
    Skill *missileSkill;
    int missile_sn;

private:
    virtual void msgOutput( Character *, const char * );
};

class ThrowerOneHit : public MissileOneHit {
public:
    ThrowerOneHit( Character *ch, Character *victim, Object *thrower, Object *missile, int range );
    
    virtual void init( );

protected:
    virtual void damBase( );
    virtual bool mprog_immune();

    Object *thrower;
    Skill *throwerSkill;
    int thrower_sn;
    int skill_thrower;
};

#else
struct MissileOneHit : public OneHit { 
    MissileOneHit( ) { }
    virtual void init( ) { }
};
struct ThrowerOneHit : public MissileOneHit { 
    ThrowerOneHit( ) { }
};
#endif

#endif
