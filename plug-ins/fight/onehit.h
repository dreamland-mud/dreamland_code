/* $Id: onehit.h,v 1.1.2.4 2010/01/01 15:48:15 rufina Exp $
 * 
 * ruffina, 2004
 */
#ifndef __ONEHIT_H__
#define __ONEHIT_H__

#include "damage.h"

class Skill;

#ifndef FIGHT_STUB
class OneHit: public virtual Damage {
public:
    OneHit( Character *ch, Character *victim );
    
    void hit( );

    virtual void init( ) = 0;
    virtual bool canHit( );
    bool checkShadow();

    bool diceroll( );
    void miss( );

    virtual void calcTHAC0( );
    void thacBase( );
    void thacApplyHitroll( );
    void thacApplySkill( );

    virtual void calcArmorClass( );
    void acBase( );
    void acApplyArmorUse( );
    void acApplyBlindFighting( );
    void acApplyPosition( );

    virtual void calcDamage( );
    void damNormalize( );
    void damApplyPosition( );
    void damApplyDamroll( );
    void damApplyAttitude( );
    
    void protectShadowShroud( );
    virtual void priorDamageEffects( );
    virtual void postDamageEffects( );

private:
    virtual void msgOutput( Character *, const char * );
    void msgEchoNoSpam( Character *, const char *, int, bool );
protected:
    void msgWeaponChar( const char * );
    void msgWeaponVict( const char * );
    void msgWeaponRoom( const char * );

    void msgFightChar( const char * );
    void msgFightVict( const char * );
    void msgFightRoom( const char * );

protected:
    int skill;
    int thac0;
    int victim_ac;
    int orig_dam;
};

#else
struct OneHit: public virtual Damage {
    OneHit( ) { }
    virtual void message( ) { }
};
#endif

#endif
