/* $Id: vampire.cpp,v 1.1.2.3.6.7 2008/05/27 21:30:01 rufina Exp $
 *
 * ruffina, 2005
 */
#include "basicmobilebehavior.h"

#include "npcharacter.h"
#include "pcharacter.h"
#include "room.h"
#include "affect.h"

#include "dreamland.h"
#include "loadsave.h"
#include "fight.h"
#include "gsn_plugin.h"
#include "magic.h"
#include "interp.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

GSN(dispel_affects);
PROF(vampire);

/*----------------------------------------------------------------------------
 *                         VAMPIRIC BRAIN
 *----------------------------------------------------------------------------*/
struct BasicMobileBehavior::VampVictims : public vector<Character *> {
    VampVictims( ) { }
    VampVictims( BasicMobileBehavior::Pointer v ) : vamp( v ) { }
    virtual ~VampVictims( ) { }

    virtual bool hit( Character * ) = 0; 
    virtual bool canAdd( Character * wch ) 
    {
        NPCharacter *ch = vamp->getChar( );

        if (wch == ch || is_safe_nomessage(ch, wch)) 
            return false;
        if (!ch->can_see( wch ))
            return false;
        
        if (vamp->memoryFought.memorized( wch ) || vamp->isLastFought( wch ))
            return true;
        
        if (!vamp->canAggressVampire( wch ))
            return false;

        // Don't get distracted on a hunt.
        if (ch->zone && ch->in_room->areaIndex() != ch->zone && vamp->hasLastFought())
            return false;

        return true;
    }
    inline void add( Character *wch )
    {
        if (canAdd( wch ))
            push_back( wch );
    }
    inline Character * getVictim( )
    {
        return empty( ) ? NULL : at( number_range( 0, size( ) - 1 ) );
    }
    inline bool attack( )
    {
        Character *wch = getVictim( );

        if (!wch)
            return false;
        
        return hit( wch );
    }
    inline void cast( int sn, Character *wch )
    {
        ::spell( sn, vamp->getChar( )->getModifyLevel( ), vamp->getChar( ), wch,
                 FSPELL_VERBOSE | FSPELL_BANE | FSPELL_WAIT | FSPELL_OBSTACLES | FSPELL_MANA );
    }
    inline bool mustSuck( Character *wch )
    {
        Affect *paf = wch->affected.find(gsn_vampiric_touch);

        if (!paf && number_percent( ) < 30)
            return true;
        if (paf && paf->duration == 0)
            return true;
        if (number_percent( ) < 10)
            return true;
        
        return false;
    }

protected:
    BasicMobileBehavior::Pointer vamp;
};

struct BasicMobileBehavior::TouchVictims : public BasicMobileBehavior::VampVictims {
    TouchVictims( BasicMobileBehavior::Pointer vamp ) : VampVictims( vamp ) { }
    
    virtual bool canAdd( Character *wch )
    {
        if (!VampVictims::canAdd( wch ))
            return false;
        if (wch->isAffected(gsn_vampiric_touch)) 
            return false;
        if (!IS_AWAKE(wch) && wch->isAffected(gsn_vampiric_bite))
            return false;

        return true;
    }
    virtual bool hit( Character *wch )
    {
        interpret_raw( vamp->getChar( ), "touch", 
                       wch->getDoppel( vamp->getChar( ) )->getNameP( ) );
        return true;
    }
};

struct BasicMobileBehavior::BiteVictims : public BasicMobileBehavior::VampVictims {
    BiteVictims( BasicMobileBehavior::Pointer vamp ) : VampVictims( vamp ) { }

    virtual bool canAdd( Character *wch )
    {
        if (!VampVictims::canAdd( wch ))
            return false;
        if (IS_AWAKE(wch))
            return false;
        if (wch->isAffected(gsn_vampiric_bite))
            return false;
        
        return true;
    }
    virtual bool hit( Character *wch )
    {
        interpret_raw( vamp->getChar( ), "bite", 
                       wch->getDoppel( vamp->getChar( ) )->getNameP( ) );
        return true;
    }
};

struct BasicMobileBehavior::SuckVictims : public BasicMobileBehavior::VampVictims {
    SuckVictims( BasicMobileBehavior::Pointer vamp ) : VampVictims( vamp ) { }
    
    virtual bool canAdd( Character *wch )
    {
        if (!VampVictims::canAdd( wch ))
            return false;
        if (IS_AWAKE(wch))
            return false;
        if (!wch->isAffected(gsn_vampiric_bite))
            return false;
        
        return true;
    }
    virtual bool hit( Character *wch )
    {
        interpret_raw( vamp->getChar( ), "suck", 
                       wch->getDoppel( vamp->getChar( ) )->getNameP( ) );
        return true;
    }
};

struct BasicMobileBehavior::DispelVictims : public BasicMobileBehavior::VampVictims {
    DispelVictims( BasicMobileBehavior::Pointer vamp ) : VampVictims( vamp ) { }
    
    virtual bool canAdd( Character *wch )
    {
        if (!VampVictims::canAdd( wch ))
            return false;
        if (IS_AWAKE(wch))
            return false;
        if (!gsn_dispel_affects->usable( vamp->getChar( ) ))
            return false;
                
        return true;
    }
    virtual bool hit( Character *wch )
    {
        if (mustSuck(wch))
            return false;

        cast( gsn_dispel_affects, wch );
        return true;
    }
};

struct BasicMobileBehavior::BlindVictims : public BasicMobileBehavior::VampVictims {
    BlindVictims( BasicMobileBehavior::Pointer vamp ) : VampVictims( vamp ) { }
    
    virtual bool canAdd( Character *wch )
    {
        if (!VampVictims::canAdd( wch ))
            return false;
        if (IS_AWAKE(wch))
            return false;
        if (IS_AFFECTED( wch, AFF_BLIND))
            return false;
        if (!gsn_blindness->usable( vamp->getChar( ) ))
            return false;
                
        return true;
    }
    virtual bool hit( Character *wch )
    {
        if (mustSuck(wch))
            return false;

        cast( gsn_blindness, wch );
        return true;
    }
};

struct BasicMobileBehavior::KillVictims : public BasicMobileBehavior::VampVictims {
    KillVictims( BasicMobileBehavior::Pointer vamp ) : VampVictims( vamp ) { }

    virtual bool canAdd( Character *wch )
    {
        if (wch->fighting == vamp->getChar( )
            || vamp->getChar( )->fighting == wch)
            return true;

        if (!VampVictims::canAdd( wch ))
            return false;

        if (!IS_AWAKE( wch ))
            return false;
        
        return true;
    }
    // attack : chain light., burning hands, colour spray, dispel good
    // malad. : blindness, curse, dispel magic, slow, weaken
    // defense: bat swarm
    virtual bool hit( Character *wch )
    {
        NPCharacter *ch = vamp->getChar( );
        int myHit = HEALTH(ch);
        int hisHit = HEALTH(wch);
        int sn = -1, snAttack = -1, snMalad = -1;
        
        if (gsn_chain_lightning->usable( ch ) && myHit > 50)
            snAttack = gsn_chain_lightning;
        else if (gsn_vampiric_blast->usable( ch ))
            snAttack = gsn_vampiric_blast;
        else if (gsn_dispel_good->usable( ch ) && IS_GOOD(wch))
            snAttack = gsn_dispel_good;
        else if (gsn_colour_spray->usable( ch ))
            snAttack = gsn_colour_spray;
        else if (gsn_burning_hands->usable( ch ))
            snAttack = gsn_burning_hands;

        if (gsn_blindness->usable( ch ) && !IS_AFFECTED(wch, AFF_BLIND))
            snMalad = gsn_blindness;
        else if (gsn_curse->usable( ch ) && !IS_AFFECTED(wch, AFF_CURSE))
            snMalad = gsn_curse;
        else if (gsn_slow->usable( ch ) && !IS_AFFECTED(wch, AFF_SLOW))
            snMalad = gsn_slow;
        else if (gsn_weaken->usable( ch ) && !IS_AFFECTED(wch, AFF_WEAKEN))
            snMalad = gsn_weaken;
        
        if (snAttack != -1 && number_percent( ) > hisHit)
        {
            sn = snAttack;
        }
        else if (gsn_dispel_affects->usable( ch ) 
             && number_percent( ) < 50 && myHit > 30)
        {
            sn = gsn_dispel_affects;
        }
        else if (gsn_bat_swarm->usable( ch ) 
                && !ch->isAffected(gsn_bat_swarm )
                && number_percent( ) < 80 && myHit < 50)
        {
            sn = gsn_bat_swarm;
        }
        else if (snMalad != -1 && myHit > 50 && number_percent( ) < 30)
        {
            sn = snMalad;
        }
        else 
            sn = snAttack;
        
        if (sn == -1)
            return false;
        
        cast( sn, wch );
        return true;
    }
};

/*---------------------------------------------------------------------------*/

bool BasicMobileBehavior::canAggressVampire( Character *wch )
{
    if (wch->getProfession( ) == prof_vampire)
        return false;
    if (wch->is_npc( ) && IS_SET(wch->act, ACT_UNDEAD|ACT_VAMPIRE))
        return false;
    if (wch->is_immortal() && !wch->getPC()->getAttributes().isAvailable("ai_aggress"))
        return false;
    if (wch->getModifyLevel( ) > ch->getModifyLevel( ) + 8)
        return false;
    return true;
}

bool BasicMobileBehavior::specFightVampire( )
{
    Character *wch;
    KillVictims vkill( this );

    for (wch = ch->in_room->people; wch; wch = wch->next_in_room) 
        if (wch->fighting == ch) 
            vkill.add( wch );
            
    return vkill.attack( );
}

bool BasicMobileBehavior::aggressVampire( )
{
    Character *wch;
    KillVictims vkill( this );
    TouchVictims vtouch( this );
    BiteVictims vbite( this );
    SuckVictims vsuck( this );
    DispelVictims vdispel( this );
    BlindVictims vblind( this );
    
    if (isAfterCharm( ))
        return false;
    
    if (ch->wait > 0)
        return false;

    if (number_range( 1, PULSE_MOBILE ) == 1)
        return false;

    for (wch = ch->in_room->people; wch; wch = wch->next_in_room) {
        if (wch->fighting || wch->position == POS_FIGHTING) 
            continue;
        if (wch->is_npc( ) && !IS_CHARMED(wch))
            continue;

        vsuck.add( wch );
        vbite.add( wch );
        vtouch.add( wch );
        vkill.add( wch );
        vdispel.add( wch );
        vblind.add( wch );
    }
    
    if (!vtouch.attack( )) 
        if (number_bits( 1 ) || !vdispel.attack( ))
            if (number_bits( 1 ) || !vblind.attack( ))
                if (number_percent( ) < 30 || !vsuck.attack( ))
                    if (number_percent( ) < 30 || !vbite.attack( ))
                        if (!vkill.attack( ))
                            return false;
    
    return true;
}
