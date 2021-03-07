/* $Id: damage_impl.h,v 1.1.2.3 2008/04/28 23:31:01 rufina Exp $
 * 
 * ruffina, 2004
 */
#ifndef __DAMAGE_IMPL_H__
#define __DAMAGE_IMPL_H__

#include "damage.h"

struct SelfDamage : public Damage {
    SelfDamage( Character *ch, int dam_type, int dam );

    virtual void calcDamage( );
};

// obsolates damage( .. TYPE_HIT .. );
struct RawDamage : public Damage {
    RawDamage( Character *ch, Character *victim, int dam_type, int dam );
    
    virtual void message( );
    virtual bool canDamage( );
    virtual void calcDamage( ) { }
};

struct SkillDamage : public virtual Damage {
    SkillDamage( Character *ch, Character *victim, int sn, int dam_type, int dam, bitstring_t dam_flag );
    SkillDamage( Affect *paf, Character *victim, int sn, int dam_type, int dam, bitstring_t dam_flag );
    
    virtual void protectResistance( );
    virtual int msgNoSpamBit( );
    virtual void message( );
    
protected:
    virtual bool mprog_immune();

    int sn;
};

#endif


