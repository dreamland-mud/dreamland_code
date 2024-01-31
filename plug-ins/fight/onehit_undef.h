/* $Id: onehit_undef.h,v 1.1.2.2 2008/05/27 21:30:02 rufina Exp $
 * 
 * ruffina, 2004
 */
#ifndef __ONEHIT_UNDEF_H__
#define __ONEHIT_UNDEF_H__

#include "onehit_weapon.h"

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
};

#endif
