/* $Id: globalquest.cpp,v 1.1.2.2.6.5 2009/09/24 14:09:12 rufina Exp $
 * 
 * ruffina, 2003
 */

#include "globalquest.h"
#include "globalquestmanager.h"
#include "gqchannel.h"
#include "xmlattributeglobalquest.h"

#include "class.h"

#include "summoncreaturespell.h"


#include "dlscheduler.h"
#include "object.h"
#include "room.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "clan.h"
#include "npcharacter.h"

#include "act.h"
#include "move_utils.h"
#include "mercdb.h"
#include "handler.h"
#include "merc.h"
#include "vnum.h"
#include "def.h"


GlobalQuest::GlobalQuest( )
{
}

GlobalQuest::GlobalQuest( const DLString& id ) : questID( id )
{
}

void GlobalQuest::suspend( )
{
    GlobalQuestManager::getThis( )->deactivate( this );
    DLScheduler::getThis( )->slayInstance( GlobalQuest::Pointer( this ) );
}

void GlobalQuest::resume( )
{
    int time = getTaskTime( );

    if (time < 0) 
        scheduleDestroy( );
    else {
        DLScheduler::getThis( )->putTaskInSecond( time * 60, GlobalQuest::Pointer( this ) );
        GlobalQuestManager::getThis( )->activate( this );
    }
}

class GQuestShutdownTask: public SchedulerTask {
public:
    typedef ::Pointer<GQuestShutdownTask> Pointer;
    
    GQuestShutdownTask( GlobalQuest::Pointer gq ) : gquest( gq ) 
    {
    }
    virtual void run( )
    {
        gquest->destroy( );
        gquest->clearAttributes( );
        GlobalQuestManager::getThis( )->removeRT( *gquest );
    }
    virtual int getPriority( ) const
    {
        int prio = DLScheduler::getThis( )->getPriority( );

        if (prio >= SCDP_IOREAD && prio < SCDP_AUTO) 
            prio = SCDP_AUTO + 100;
        else 
            prio = max( prio, (int)SCDP_INITIAL ) + 100;
        
        return prio;
    }

private:
    GlobalQuest::Pointer gquest;
};

void GlobalQuest::scheduleDestroy( )
{
    GlobalQuest::Pointer thiz( this );
    thiz->suspend( );
    DLScheduler::getThis( )->putTaskNOW( GQuestShutdownTask::Pointer( NEW, thiz ) );
}

void GlobalQuest::after( )
{
    int time = getTaskTime( );

    if (time < 0) 
        scheduleDestroy( );
    else {
        DLScheduler::getThis( )->putTaskInSecond( time * 60, GlobalQuest::Pointer( this ) );
        GlobalQuestManager::getThis( )->saveRT( this );
    }
}

int GlobalQuest::getPriority( ) const
{
    return SCDP_ROUND + 101;
}


void GlobalQuest::clearAttributes( ) const
{
    PCharacterMemoryList::const_iterator i;
    const PCharacterMemoryList &pcm = PCharacterManager::getPCM( );
    
    for (i = pcm.begin( ); i != pcm.end( ); i++) {
        XMLAttributes * attributes = &i->second->getAttributes( );
        XMLAttributes::iterator ipos = attributes->find( getQuestID( ) );

        if (ipos != attributes->end( )) {
            attributes->erase( ipos );
            PCharacterManager::saveMemory( i->second );
        }
    }
}

void GlobalQuest::printRemainedTime( ostringstream &buf ) const
{
    int t = getRemainedTime( );

    if (t > 0)
        buf << GQChannel::BOLD << t << GQChannel::NORMAL
            << " минут" << GET_COUNT( t, "а", "ы", "");
    else
        buf << "меньше минуты";
}

bool GlobalQuest::isLevelOK( Character *ch ) const
{
    int level = ch->getRealLevel( ); // if you see your level is in the range, you're in, no matter the remort bonuses
    
    if (ch->is_npc( ))
        return false;
   
    if (!hasLevels( ))
        return true;

    if (level >= getMinLevel( ) && level <= getMaxLevel( ))
        return true;
    
    /* проверка ситуации, когда игрок набрал уровень за время глобала */
    if (level > maxLevel.getValue( ) && level <= maxLevel.getValue( ) + 2) {
        XMLAttributeGlobalQuest::Pointer attr;

        attr = ch->getPC( )->getAttributes( ).findAttr<XMLAttributeGlobalQuest>(
                                                getQuestID( ) );

        if (attr && attr->isJoined( ))
            return true;
    }
        
    return false;
}

Character * GlobalQuest::getActor( Character *ch ) const
{
    if (ch && ch->is_npc() && ch->master && !ch->master->is_npc())
        return ch->master;

    return ch;
}

void GlobalQuest::exorcism(Character *ch) const
{
    int recall_vnum;
    Room *recall;
    Character *actor = getActor(ch);

    oldact("$c1 улетучивается.", ch, 0, 0, TO_ROOM);
    oldact("Ты исчезаешь отсюда.", ch, 0, 0, TO_CHAR);

    // lonely npc with no current master
    if (ch->is_npc() && (ch == actor || actor->is_npc()))
    {

        // check if it's a summoned creature like a golem or something
        if (ch->getNPC()->behavior && ch->getNPC()->behavior.getDynamicPointer<SummonedCreature>())
        {

            // it's summoned creature - try to find the player it belongs to
            PCMemoryInterface *player = PCharacterManager::find(ch->getNPC()->behavior.getDynamicPointer<SummonedCreature>()->creatorName);

            // player found - check player's clan or hometown recall spot and transfer the creature there
            if (player)
            {

                recall_vnum = player->getClan()->getRecallVnum();

                if (recall_vnum <= 0)
                    recall = get_room_instance(player->getHometown()->getRecall());
                else
                    recall = get_room_instance(recall_vnum);

                if (!recall)
                    recall = get_room_instance(ROOM_VNUM_TEMPLE);

                transfer_char(ch, ch, recall,
                              NULL, NULL, "%1$^C1 появил%1$Gось|ся|ась в комнате.");
                return;
            }

            // player not found - extract summoned creature
            else
            {
                extract_char(ch, true);
                return;
            }
        }

        // not a summoned creature - extract it
        else
        {
            extract_char(ch, true);
            return;
        }
    }

    recall_vnum = actor->getClan()->getRecallVnum();

    if (recall_vnum <= 0)
        recall = get_room_instance(actor->getPC()->getHometown()->getRecall());
    else
        recall = get_room_instance(recall_vnum);

    if (!recall)
        recall = get_room_instance(ROOM_VNUM_TEMPLE);

    transfer_char(ch, ch, recall,
                  NULL, NULL, "%1$^C1 появил%1$Gось|ся|ась в комнате.");
}

void GlobalQuest::wipeRoom( Room *room ) const
{
    Object *obj_next, *pit;
    Room *office;
    
    if (!room)
        return;

    // Move every person in the room to an altar, extract unclaimed mobs.
    // Can't do the usual next_in_room cycle, because mounted chars and mounts
    // are transferred together, breaking the loop.
    list<Character *> people = room->getPeople();
    for (auto &rch: people) {
        exorcism( rch );
    }
    
    // Move every item on the floor to the owner's pit or to Lost Property office.
    pit = find_pit_in_room(ROOM_VNUM_ALTAR);
    office = get_room_instance( ROOM_VNUM_BUREAU_2 );
    
    for (Object *obj = room->contents; obj; obj = obj_next) {
        int v = obj->pIndexData->vnum;
        obj_next = obj->next_content;
        
        obj_from_room( obj );

        if (   v == OBJ_VNUM_GUTS       || v == OBJ_VNUM_SEVERED_HEAD
            || v == OBJ_VNUM_TORN_HEART || v == OBJ_VNUM_SLICED_ARM
            || v == OBJ_VNUM_SLICED_LEG || v == OBJ_VNUM_BRAINS
            || v == OBJ_VNUM_CORPSE_NPC || v == OBJ_VNUM_POTION_VIAL)
        {
            extract_obj( obj );
            continue;
        }

        if (v == OBJ_VNUM_CORPSE_PC) {
            Room *pitRoom = get_room_instance( obj->value3() );

            if (pitRoom) 
                obj_to_room( obj, pitRoom );
            else
                obj_to_room( obj, get_room_instance( ROOM_VNUM_ALTAR ) );

            continue;
        }
            
        if (office)
            obj_to_room( obj, office );
        else 
            obj_to_obj( obj, pit );
    }
}
