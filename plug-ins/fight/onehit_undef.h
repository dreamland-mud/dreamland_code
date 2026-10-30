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

/** A player's melee round (multi_hit): OneHit::hit counts the blows that
 *  land for achievement eleven_strikes. The destructor clears it on any exit,
 *  a killing blow's exception included. */
struct RoundGuard {
    RoundGuard( Character *striker );
    ~RoundGuard( );
};
void round_blow_landed( Character *ch );

class UndefinedOneHit: public WeaponOneHit {
public:
    UndefinedOneHit( Character *ch, Character *victim, bool secondary, string command = "", bool offEnd = false );
    
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
    void damApplyMasterStaff( );
    void damApplyFudoshin( );
    void damApplyDeathblow( );
    void damApplySoulLust( );
    void damApplyMounted( );
    void damApplyFadeOpener( );
    void msgOpeners( );
    void damEffectDeathblowStun( );
    void damApplyReligion();
    
    void damEffectCriticalWound( );
    void damEffectMasterHand( );
    void damEffectMasterSword( );
    void damEffectMasterStaff( );
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
    bool offEnd;          // the staff's other end, struck from the grip (fight.cpp staff_offend_hit)
};

#endif
