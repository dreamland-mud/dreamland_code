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
    WeaponOneHit( Character *ch, Character *victim, bool secondary, string command = "" );
    
    virtual void init( );

    /* Swing as this attack (weapon_flags value) instead of the weapon's or
     * the bare hands': only the attack type, its damage class and noun change. */
    void setAttack( int attack );
    
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
    string command;
    int weapon_sn;
    Skill *weaponSkill;
    int attack;
    int attackOverride;

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
