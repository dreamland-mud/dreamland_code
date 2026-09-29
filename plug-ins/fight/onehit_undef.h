/* $Id: onehit_undef.h,v 1.1.2.2 2008/05/27 21:30:02 rufina Exp $
 * 
 * ruffina, 2004
 */
#ifndef __ONEHIT_UNDEF_H__
#define __ONEHIT_UNDEF_H__

#include "onehit_weapon.h"

/**
 * Set by kill/murder around the opening multi_hit and consumed by that attacker's
 * first landed hit: a mounted lance charge and a fade+ ambush.
 */
extern Character *charge_attacker;
extern Character *ambush_attacker;

/** Resets both opener markers when the opening round ends, however it ends. */
struct OpenerGuard {
    OpenerGuard( Character *charge, Character *ambush );
    ~OpenerGuard( );
};

class UndefinedOneHit: public WeaponOneHit {
public:
    UndefinedOneHit( Character *ch, Character *victim, bool secondary, string command = "" );
    
    virtual bool canHit( );
    bool checkHands( );
    virtual bool canDamage( );
    virtual void calcDamage( );
    virtual void protectPrayer( );
    virtual void message( );
    
    virtual void priorDamageEffects( );
    virtual void postDamageEffects( );

protected:
    void damApplyMasterHand( );
    void damApplyMasterSword( );
    void damApplyDeathblow( );
    void damApplySoulLust( );
    void damApplyMounted( );
    void damApplyFadeOpener( );
    void msgOpeners( );
    void damEffectDeathblowStun( );
    void damApplyReligion();
    
    void damEffectMasterHand( );
    void damEffectMasterSword( );
    void damEffectDestroyEquipment( );
    void damEffectCriticalStrike( );
    void damEffectVorpal();

    void destroyWeapon( );
    void destroyShield( );
    int getDestroyChance( Object * );
    bool canDestroy( Object * );

    virtual bool mprog_hit();

    bool deathblowStun;
    bool charged;
    bool ambushed;
};

#endif
