/* $Id: damage.h,v 1.1.2.4 2008/05/27 21:30:03 rufina Exp $
 * 
 * ruffina, 2004
 */
#ifndef __DAMAGE_H__
#define __DAMAGE_H__

#include <stdarg.h>
#include "dlstring.h"
#include "bitstring.h"
#include "lang.h"
#include "multimessage.h"
#include "fight_exception.h"
#include "eventbus.h"

class Character;
class Object;
class Affect;

/* Nominative damage-class noun in the viewer's language (Trello AXqNSTVz).
 * RU falls back to the inflected damage_table noun; en/ua come from
 * config/fight/damage_nouns.json. Defined in damage_impl.cpp. */
DLString damage_noun(int dam_type, lang_t lang);

/* Gear advisor combat-proc scoring (COMBAT_PROC_SCORING.md). Relative worth of a
 * spell cast in combat (config/fight/spell_combat_value.json); 0 for an unlisted
 * spell. _global scales procs vs stats, _level_ref is the item level that scores
 * at 1.0x. Defined in damage_impl.cpp. */
double spell_combat_value(const DLString &spell);
double spell_combat_global();
double spell_combat_level_ref();

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

    /** Whether this damage rouses a sleeping victim where it lands. True for spells,
     *  traps and affects; a melee round defers it to the end (stand_up_after_round),
     *  so that every strike of it still gets the sleeping victim's damage bonus.
     */
    virtual bool wakesSleepingVictim( ) const { return true; }

    virtual void reportState( );
    bool checkRetreat( );
    void handleDeath( );

    virtual void message( ) = 0;

    inline int getDamage() const { return dam; }
    
protected:
    void msgVict( const char *fmt, ... );
    void msgRoom( const char *fmt, ... );
    void msgChar( const char *fmt, ... );
    // Trilinguality (Trello 2594): MultiMessage overloads. The frame resolves
    // per recipient in viewerLang(to), and the spliced damage verb resolves in
    // the same language (RU stays byte-identical -- the RU verb tables live in
    // C++, EN/UA layer from config/fight/damage_verbs.json with RU fallback).
    void msgVict( const MultiMessage &fmt, ... );
    void msgRoom( const MultiMessage &fmt, ... );
    void msgChar( const MultiMessage &fmt, ... );
    virtual int msgNoSpamBit( );
    bool canSeeMessage(Character *to);
    void msgEcho(Character *to, const char *f, va_list va);
    void msgEcho(Character *to, const MultiMessage &f, va_list va);
    void msgEchoImpl(Character *to, const DLString &fmt, lang_t lang, va_list va);
    void msgOldFormat( bool vs, char *buf, lang_t lang );
    void msgNewFormat( bool vs, char *buf, lang_t lang );
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
    
    // What to pass to the death handler as source of death.
    DLString deathReason;
};

struct CharDeathEvent : public Event {
    CharDeathEvent(Character *victim, Character *killer, bitstring_t flags, const DLString &label, int damtype);
    Character *victim;
    Character *killer;
    bitstring_t flags;
    DLString label;
    int damtype;
};

#endif


