/* $Id: gangchef.cpp,v 1.1.2.4.6.2 2008/04/14 19:36:15 rufina Exp $
 *
 * ruffina, 2003
 */

#include "gangsters.h"
#include "gangstersinfo.h"
#include "aitrigger.h"
#include "xmlattributegangsters.h"
#include "gangchef.h"

#include "room.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "fenia_utils.h"
#include "fight.h"
#include "loadsave.h"
#include "act.h"
#include "def.h"
#include "l10n.h"

bool GangChef::death( Character *killer ) 
{
    Gangsters *gquest = Gangsters::getThis( );

    if (!killer)
        return false;

    killer = gquest->getActor( killer );
    
    log("GangChef: killed by " << killer->getNameC());

    if (!gquest->isLevelOK( killer )) {
        oldact(_("Ну даешь! Как ты сюда вообще попал?"), killer, 0, 0, TO_CHAR);
        LogStream::sendNotice( ) << "BAD guy in the Lair: " << killer->getNameC() 
                                 << " lvl " << killer->getModifyLevel( ) << endl;

        gquest->state = Gangsters::ST_BROKEN;                
    } else {
        gquest->state = Gangsters::ST_CHEF_KILLED;                
        gquest->chefKiller = killer->getNameC( );
        createBounty(killer);
    }
    
    gquest->scheduleDestroy( );
    return false;
}
   
// The bounty is the boss loot set, rolled in Fenia (global/onGangChefLoot):
// the gang rolls rarely, so no daily cap and no level gate on it.
void GangChef::createBounty(Character *killer) 
{
    gprog("onGangChefLoot", "CC", ch, killer);
}

void GangChef::greet( Character *mob ) 
{
    Gangsters *gquest = Gangsters::getThis( );

    if (!gquest->isLevelOK( gquest->getActor( mob ) )) {
        oldact(_("$c1 вопит '{RВОН ОТСЮДА!{x'"), ch, 0, mob, TO_VICT);
        gquest->exorcism( mob );
        return;
    }
        
    if (mob->is_npc( ))
        return;

    mob->getPC( )->getAttributes( ).getAttr<XMLAttributeGangsters>( 
            gquest->getQuestID( ) )->setJoined( );

    if (is_safe_nomessage( ch, mob ))
        return;
    
    if (!ch->fighting)
        oldact_p(_("$c1 тушит сигару о ладонь одного из гангстеров и выхватывает кинжал."),
               ch, 0, mob, TO_ROOM, POS_RESTING);

    /* force guest to begin the fight */
    multi_hit( mob, ch , "murder" );
}

void GangChef::fight( Character *victim, string command ) 
{
    Character *mob, *ch_next;

    // Fenia (global/onGangChefFightAI) picks which gang members can actually
    // join in; the loop below is the fallback while no handler is registered.
    // If the handler extracted the chef, the behavior loses ch and would be
    // freed with it: hold it, and fight on only while ch is still in a room.
    GangChef::Pointer keep = ch->getNPC( )->behavior.getDynamicPointer<GangChef>( );

    if (ai_trigger(true, ch, "onGangChefFightAI", "CCi", ch, victim,
                   GangstersInfo::getThis( )->getMobVnum( )) != AI_UNHANDLED) {
        if (ch && ch->in_room && !ch->isDead( ))
            GangMob::fight( victim, command );
        return;
    }
    
    for (mob = ch->in_room->people; mob; mob = ch_next) {
        ch_next = mob->next_in_room;

        if (!mob->is_npc( ))
            continue;
        
        if (mob == ch)
            continue;

        if (!mob->getNPC( )->behavior 
            || !mob->getNPC( )->behavior.getDynamicPointer<GangMob>( )) 
            continue;        
        
        if (!mob->can_see( victim ))              
            continue;
        
        if (mob->fighting)
            continue;

        multi_hit( mob, victim, command );
    }
    
    GangMob::fight( victim, command );
}

bool GangChef::spec( ) 
{
    return true;
}

