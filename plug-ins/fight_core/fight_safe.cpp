/* $Id$
 *
 * ruffina, 2004
 */
#include "fight_safe.h"
#include "fight_position.h"

#include <map>


#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "dreamland.h"
#include "affect.h"
#include "pcharactermanager.h"
#include "room.h"
#include "skill.h"
#include "pcharacter.h"
#include "merc.h"

#include "act.h"
#include "interp.h"
#include "loadsave.h"
#include "clanreference.h"
#include "race.h"
#include "npcharacter.h"
#include "object.h"
#include "wiznet.h"
#include "def.h"
#include "vnum.h"
#include "l10n.h"
#include "follow_utils.h"

CLAN(none);
CLAN(flowers);

#define IS_WANTED(victim) \
    (IS_SET(victim->act, PLR_WANTED) || IS_SET(victim->getPC()->PK_flag, PK_THIEF))


bool is_safe(Character *ch, Character *victim)
{
    bool verbose = true;

    if (!is_safe_nomessage(ch, victim, verbose))
        return false;

    if (ch && victim) {
        oldact(_("$C1 находится под защитой богов."),ch,0,victim,TO_CHAR);
        oldact(_("Боги защитили $C4 от $c2."),ch,0,victim,TO_ROOM);

        if (victim->fighting == ch) 
            stop_fighting (victim, false);
    }

    return true;
}

/*
 * A player's first attack on someone in their own group is held back with a
 * warning: the same attack typed again within WARN_WINDOW seconds goes
 * through, and attacks on that groupmate stay allowed for CONFIRM_TIME.
 * Only commands the player typed are held, never damage from ticks or AI.
 */
struct GroupmateWarning {
    long long victim = 0;
    long serial = -1;
    time_t warnedAt = 0;
    time_t confirmedUntil = 0;
};

static const time_t WARN_WINDOW = 20;
static const time_t CONFIRM_TIME = 60;
static std::map<long long, GroupmateWarning> groupmateWarnings;

bool groupmate_attack_held( Character *ch, Character *victim )
{
    if (!ch || !victim || ch == victim)
        return false;
    if (ch->is_npc( ) || interpret_actor( ) != ch)
        return false;
    if (ch->fighting == victim || victim->fighting == ch)
        return false;
    if (!is_same_group( ch, victim ))
        return false;

    GroupmateWarning &w = groupmateWarnings[ch->getID( )];
    time_t now = dreamland->getCurrentTime( );
    long serial = interpret_serial( );

    if (w.victim == victim->getID( )) {
        if (w.confirmedUntil >= now)
            return false;
        // preRun and run of the same command: warned once already
        if (w.serial == serial)
            return true;
        if (now - w.warnedAt <= WARN_WINDOW) {
            w.confirmedUntil = now + CONFIRM_TIME;
            return false;
        }
    }

    w.victim = victim->getID( );
    w.serial = serial;
    w.warnedAt = now;
    w.confirmedUntil = 0;
    ch->pecho( _("{1{Y%1$^C1 в твоей группе. Повтори команду, если правда хочешь напасть.{2"), victim );
    return true;
}

bool is_safe_attack( Character *ch, Character *victim )
{
    return is_safe( ch, victim ) || groupmate_attack_held( ch, victim );
}

static bool mprog_safe( Character *ch, Character *victim, bool verbose ) 
{
    int v = verbose;	
    FENIA_CALL( ch, "Safe", "CCi", ch, victim, v );
    FENIA_NDX_CALL( ch->getNPC( ), "Safe", "CCCi", ch, ch, victim, v );
    FENIA_CALL( victim, "Safe", "CCi", ch, victim, v );
    FENIA_NDX_CALL( victim->getNPC( ), "Safe", "CCCi", victim, ch, victim, v );
    return false;
}        

static bool is_in_pk_range( int cml, int vml, int non_wanted )
{
    if (non_wanted * cml < vml + max( 4, cml / 12 + 2) 
        && cml > vml - max( 4, cml / 12 + 2 ))
        return true;

    if (vml < cml + max( 4, vml / 12 + 2 )
        && vml > cml - max( 4, vml / 12 + 2 ) * non_wanted)
        return true;

    return false;
}

bool is_safe_nomessage(Character *ch, Character *victim, bool verbose )
{
    if (!ch || !victim)
        return true;

    if (!ch->in_room || !victim->in_room)
        return true;
        
    if ( victim->fighting == ch
        || ch == victim )
    {
        return false;
    }

    if (mprog_safe( ch, victim, verbose ))
        return true;
    /* мертві бджоли не гудуть */
    if (victim->isDead( ) || ch->isDead( ))
        return true;

    // Special protected mobs outside of safe rooms.
    if (victim->is_npc() && IS_SET(victim->act, ACT_SAFE))
        return true;

    // Transfer safety checks from victim's pet/charmed/follower to master.
    if (victim->is_npc()
        && victim->master
        && !victim->master->is_npc() )
    {
        return is_safe_nomessage(ch,victim->master);
    }

    // Transfer safety checks from attacker's pet/charmed/follower to master.
    if (ch->is_npc()
        && ch->master
        && !ch->master->is_npc() )
    {
        return is_safe_nomessage(ch->master,victim);
    }

    if ( IS_SET( ch->in_room->room_flags, ROOM_SAFE|ROOM_NO_DAMAGE )
        || IS_SET( victim->in_room->room_flags, ROOM_SAFE|ROOM_NO_DAMAGE ) )
    {
        return true;
    }

    /* Ghosts are safe */
    if (IS_GHOST( victim  ) || IS_GHOST( ch ))
    {
        return true;
    }
    
    // unconfirmed players are safe!
    if ( !victim->is_npc()
        && !ch->is_npc()
        && ( !IS_SET( victim->act, PLR_CONFIRMED )
            || !IS_SET( ch->act, PLR_CONFIRMED ) ) )
    {
        return true;
    }

    if (( IS_SLAIN( victim ) || IS_DEATH_TIME( victim ) )
        && !IS_VIOLENT( victim )
        && !IS_SET( victim->act, PLR_WANTED ) )
    {
        return true;
    }

    /* link dead players whose adrenalin is not gushing are safe */
    if (!victim->is_npc()
        && !victim->is_adrenalined()
        && victim->desc == 0
        && !IS_VIOLENT( victim )
        && !IS_SET( victim->act, PLR_WANTED ) ) 
    {
        return true;
    }
    
    if (ch->is_npc( ) || victim->is_npc( ))
        return false;

    if (!dreamland->hasOption( DL_PK ))
        return true;

    if (victim->getModifyLevel( ) < PK_MIN_LEVEL
        || ch->getModifyLevel( ) <  PK_MIN_LEVEL)
    {
        return true;
    }

    if (ch->getClan( ) == clan_flowers || victim->getClan( ) == clan_flowers)
        return true;

    if (!ch->is_immortal( ))
    {
        if (!is_in_pk_range(ch->getModifyLevel(), victim->getModifyLevel(), !IS_WANTED(victim)))
            return true;
    }

    return false;
}

static bool is_safe_rspell( Character *victim )
{
    if (victim->is_immortal( ))
        return true;

    if (victim->is_npc( ))
        return false;

    if (IS_GHOST( victim ) || IS_DEATH_TIME( victim ))
        return true;
    
    if (victim->desc == 0 && !IS_VIOLENT( victim ))
        return true;
    
    if (victim->getModifyLevel( ) < PK_MIN_LEVEL)
        return true;

    return false;
}


bool is_safe_rspell( short level, Character *victim, bool verbose )
{
    bool rc;

    if (is_safe_rspell( victim ))
        rc = true;
    
    else if (!victim->is_npc() && !is_in_pk_range(level, victim->getModifyLevel(), !IS_WANTED(victim)))
        rc = true;

    else if (victim->is_npc() && IS_CHARMED(victim) && !victim->master->is_npc())
        rc = !is_in_pk_range(level, victim->master->getModifyLevel(), !IS_WANTED(victim->master));

    else
        rc = false;

    if (rc && verbose) {
        oldact(_("Боги защищают тебя от заклинаний в этой местности."), victim, 0, 0, TO_CHAR);
        oldact(_("Боги защищают $c4 от заклинаний в этой местности."), victim, 0, 0, TO_ROOM);
    }

    return rc;
}

bool is_safe_rspell( Affect *paf, Character *victim, bool verbose )
{
    Character *pafOwner = paf->sources.getOwner();
    // If we don't know the source of this affect, check safety against its level.
    if (!pafOwner)
        return is_safe_rspell(paf->level, victim, verbose);

    // Ensure charmed creatures are safe from master's spells.
    if (IS_CHARMED(victim) && victim->master == pafOwner)
        return true;
        
    bool rc = is_safe_nomessage(pafOwner, victim);
    if (rc && verbose) {
        oldact(_("Боги защищают тебя от заклинаний в этой местности."), victim, 0, 0, TO_CHAR);
        oldact(_("Боги защищают $c4 от заклинаний в этой местности."), victim, 0, 0, TO_ROOM);
    }

    return rc;
}
