/* $Id$
 *
 * ruffina, 2004
 */
#ifndef __ONEHIT_WEAPON_H__
#define __ONEHIT_WEAPON_H__

#include "onehit.h"
#include "damage_impl.h"

class WeaponOneHit : public OneHit {
public:
    WeaponOneHit( Character *ch, Character *victim, bool secondary );
    
    virtual void init( );
    
    virtual void calcDamage( );
    virtual void postDamageEffects( );
    virtual void priorDamageEffects( );

protected:
    void damBase( );
    void damApplyShield( );
    void damApplySharp( );
    void damApplyHoly( );
    void damApplyCounter( );

    void damEffectFeeble( );
    void damEffectFunkyWeapon( );

    virtual bool mprog_immune();

    Object *wield;
    bool secondary;
    int weapon_sn;
    Skill *weaponSkill;
    int attack;

private:
    virtual void msgOutput( Character *, const char * );
};

class SkillWeaponOneHit: public WeaponOneHit, public SkillDamage {
public:
    SkillWeaponOneHit(Character *ch, Character *victim, int sn);

protected:
    virtual bool mprog_immune();
};

#endif
