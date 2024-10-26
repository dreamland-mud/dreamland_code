/* $Id: gangchef.cpp,v 1.1.2.4.6.2 2008/04/14 19:36:15 rufina Exp $
 *
 * ruffina, 2003
 */

#include "gangsters.h"
#include "xmlattributegangsters.h"
#include "gangchef.h"

#include "room.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "weapongenerator.h"
#include "fight.h"
#include "loadsave.h"
#include "vnum.h"
#include "act.h"
#include "def.h"

bool GangChef::death( Character *killer ) 
{
    Gangsters *gquest = Gangsters::getThis( );

    if (!killer)
        return false;

    killer = gquest->getActor( killer );
    
    log("GangChef: killed by " << killer->getNameC());

    if (!gquest->isLevelOK( killer )) {
        oldact("Ну даешь! Как ты сюда вообще попал?", killer, 0, 0, TO_CHAR);
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
   
void GangChef::createBounty(Character *killer) 
{
    Object *weapon = create_object(get_obj_index(OBJ_VNUM_WEAPON_STUB), 0);
    obj_to_char(weapon, ch);

    weapon->level = killer->getModifyLevel();
    WeaponGenerator()
        .item(weapon)
        .alignment(killer->alignment)
        .player(killer->getPC())
        .randomTier(2)
        .randomizeAll();
}

void GangChef::greet( Character *mob ) 
{
    Gangsters *gquest = Gangsters::getThis( );

    if (!gquest->isLevelOK( gquest->getActor( mob ) )) {
        oldact("$c1 вопит '{RВОН ОТСЮДА!{x'", ch, 0, mob, TO_VICT);
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
        oldact_p("$c1 тушит сигару о ладонь одного из гангстеров и выхватывает кинжал.",
               ch, 0, mob, TO_ROOM, POS_RESTING);

    /* force guest to begin the fight */
    multi_hit( mob, ch , "murder" );
}

void GangChef::fight( Character *victim, string command ) 
{
    Character *mob, *ch_next;
    
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

