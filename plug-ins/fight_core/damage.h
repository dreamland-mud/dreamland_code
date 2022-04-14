/* $Id: damage.h,v 1.1.2.4 2008/05/27 21:30:03 rufina Exp $
 * 
 * ruffina, 2004
 */
#ifndef __DAMAGE_H__
#define __DAMAGE_H__

#include <stdarg.h>
#include "bitstring.h"
#include "fight_exception.h"
#include "eventbus.h"

class Character;
class Object;
class DLString;
class Affect;

class Damage {
public:
    Damage( Character *ch, Character *victim, int dam_type, int dam, bitstring_t dam_flag = 0 );
    Damage( Affect *paf, Character *victim, int dam_type, int dam, bitstring_t dam_flag = 0 );
    
    bool hit( bool show );
    
    virtual bool canDamage( );
    void adjustFollowers( );
    bool adjustMasterAttack( );
    void adjustPosition( );
    void adjustFighting( );
    void adjustVisibility( );
    void adjustAdrenaline( );
    void adjustDeathTime( );
    
    virtual void calcDamage( );
    void damApplyEnhancedDamage( );
    void protectMaterial( Object * );
    void protectSanctuary( );
    void protectAlign( );
    void protectTemperature( );
    void protectImmune( );
    void protectRazer( ); 
    virtual void protectResistance( );
    virtual void protectPrayer( );

    virtual void priorDamageEffects( );
    virtual void postDamageEffects( );


    void inflictDamage( );
    void handlePosition( );
    virtual void reportState( );
    bool checkRetreat( );
    void handleDeath( );

    virtual void message( ) = 0;
protected:
    void msgVict( const char *fmt, ... );
    void msgRoom( const char *fmt, ... );
    void msgChar( const char *fmt, ... );
    virtual int msgNoSpamBit( );
    bool canSeeMessage(Character *to);
    void msgEcho(Character *to, const char *f, va_list va);
    void msgOldFormat( bool vs, char *buf );
    void msgNewFormat( bool vs, char *buf );
    char msgPunct( );

    virtual bool mprog_immune();
    virtual bool mprog_hit();

    void init(Character *ch, Character *victim, int dam_type, int dam, bitstring_t dam_flag);

protected:
    Character *ch;
    Character *victim;
    Character *killer;
    Affect *paf;
    
    int dam_type;
    int dam;
    bool immune;
    bitstring_t dam_flag;
};

struct CharDeathEvent : public Event {
    CharDeathEvent(Character *victim, Character *killer, int bodypart = -1);
    Character *victim;
    Character *killer;
    int bodypart;
};

#endif


