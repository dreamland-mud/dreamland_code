/* $Id: onehit_undef.h,v 1.1.2.2 2008/05/27 21:30:02 rufina Exp $
 * 
 * ruffina, 2004
 */
#ifndef __ONEHIT_UNDEF_H__
#define __ONEHIT_UNDEF_H__

#include "onehit_weapon.h"

#ifndef FIGHT_STUB
class UndefinedOneHit: public WeaponOneHit {
public:
    UndefinedOneHit( Character *ch, Character *victim, bool secondary );
    
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
    
    void damEffectMasterHand( );
    void damEffectMasterSword( );
    void damEffectDestroyEquipment( );
    void damEffectGroundStrike( );
    void damEffectCriticalStrike( );
    void damEffectSlice( );

    bool defenseParry( );
    bool defenseHandBlock( ); 
    bool defenseBatSworm( ); 
    bool defenseBlink( ); 
    bool defenseShieldBlock( ); 
    bool defenseCrossBlock( ); 
    bool defenseDodge( );

    void destroyWeapon( );
    void destroyShield( );
    int getDestroyChance( Object * );
    bool canDestroy( Object * );
};

#else
struct UndefinedOneHit: public WeaponOneHit { 
    UndefinedOneHit( ) { }
};
#endif

#endif
