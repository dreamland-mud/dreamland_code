/* $Id: basicmobilebehavior.cpp,v 1.1.2.4.4.13 2014-09-19 11:33:42 rufina Exp $
 *
 * ruffina, 2005
 */

#include <algorithm>
#include <functional>

#include "logstream.h"
#include "basicmobilebehavior.h"

#include "skillreference.h"
#include "npcharacter.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "room.h"

#include "dreamland.h"
#include "interp.h"
#include "handler.h"
#include "merc.h"
#include "act.h"
#include "mercdb.h"

#include "roomtraverse.h"
#include "occupations.h"

#include "def.h"

GSN(charm_person);
GSN(attract_other);
GSN(control_undead);
GSN(summon);

/*
 * MobileMemory
 */
MobileMemory::MobileMemory( ) : XMLMapBase<XMLLongLong>( false )
{
}

void MobileMemory::remember( Character *wch )
{
    if (wch->is_npc( ))
        return;

    (*this)[wch->getName( )] = dreamland->getCurrentTime( );
}

bool MobileMemory::forget( Character *wch )
{
    iterator i;

    if (wch->is_npc( ))
        return false;

    if (( i = find( wch->getName( ) )) != end( )) {
        erase( i );
        return true;
    }

    return false;
}

bool MobileMemory::memorized( Character *wch )
{
    if (wch->is_npc( ))
        return false;
    else
        return find( wch->getName( ) ) != end( );
}

void MobileMemory::poll( int diff )
{
    iterator i;
    MobileMemory fresh;
    time_t now = dreamland->getCurrentTime( );

    for (i = begin( ); i != end( ); i++)
        if (now - i->second < diff)
            fresh[i->first] = i->second;
    
    clear( );

    for (i = fresh.begin( ); i != fresh.end( ); i++)
        (*this)[i->first] = i->second;
}

/*
 * BasicMobileBehavior 
 */
bool BasicMobileBehavior::isSaved( ) const
{
    return false;
}

bool BasicMobileBehavior::hasDestiny( )
{
    return false;
}

int BasicMobileBehavior::getOccupation( )
{
    int occupation = OCC_NONE;
    
    if (!ch->pIndexData->practicer.empty( ))
        SET_BIT(occupation, 1 << OCC_PRACTICER);

    if (!ch->pIndexData->religion.empty())
        SET_BIT(occupation, 1 << OCC_ADEPT);

    return occupation;
}

Character * BasicMobileBehavior::getMaster( Character *wch )
{
    if (IS_CHARMED(wch) && wch->master != wch)
        return getMaster( wch->master );
    else
        return wch;
}

/*
 * BasicMobileBehavior - lastFought 
 */
PCharacter *BasicMobileBehavior::getLastFoughtRoom( )
{
    Character *rch;
    
    if (!hasLastFought( ))
        return NULL;

    for (rch = ch->in_room->people; rch; rch = rch->next_in_room)
        if (isLastFought( rch ))
            return rch->getPC( );
    
    return NULL;
}

PCharacter *BasicMobileBehavior::getLastFoughtWorld( )
{
    return PCharacterManager::findPlayer( lastFought );
}

bool BasicMobileBehavior::hasLastFought( ) const
{
    return !lastFought.getValue( ).empty( );
}

void BasicMobileBehavior::clearLastFought( )
{
    lastFought = "";
    lostTrack = false;
    ch->last_fought = 0;
}

bool BasicMobileBehavior::isLastFought( Character *wch )
{
    return !wch->is_npc( ) && wch->getName( ) == lastFought.getValue( );
}

void BasicMobileBehavior::rememberFought(Character *victim)
{
    memoryFought.remember(victim);
}

void BasicMobileBehavior::setLastFought( Character *wch )
{
    if (!wch->is_npc( )) {
        ch->last_fought = wch;        
        lastFought = wch->getName( );
        lostTrack = false;
        
        if (!IS_SET(ch->act, ACT_NOTRACK)) {
            memoryFought.remember( wch );
            remember( ch->in_room );
        }
    }
}

bool BasicMobileBehavior::isAdrenalined( ) const
{
    return ch->getLastFightDelay( ) < AI_ADRENALINE_TIME;
}


/*
 * BasicMobileBehavior - last charm delay 
 */
bool BasicMobileBehavior::isAfterCharm( ) const
{
    if (IS_CHARMED(ch))
        return true;

    if (RIDDEN( ch ))
        return true;
        
    return lastCharmTime.getValue( ) + AI_CHARM_RECOVER_TIME 
                    >= dreamland->getCurrentTime( );
}

/*
 * BasicMobileBehavior - returning to home position 
 */
void BasicMobileBehavior::remember( Room *room )
{
    if (homeVnum == 0)
        homeVnum = room->vnum;
}

bool BasicMobileBehavior::backHome( bool fAlways )
{
    Room *home;

    if (homeVnum == 0) {
        if (fAlways) {
            act( "$c1 ищет свой дом.", ch, 0, 0, TO_ROOM );
            extract_char( ch );
            return true;
        }
        else 
            return false;
    }
    
    home = get_room_instance( homeVnum );

    if (!home) {
        LogStream::sendError( ) << "Mob " << ch->pIndexData->vnum << " from " << ch->in_room->vnum << ": wrong home " << homeVnum << endl;
        return false;
    }
    
    if (home == ch->in_room)
        return false;
    
    transfer_char( ch, 0, home,
                   "%1$^C1 молит Богов о возвращении.", 
                   NULL,
                   "%1$^C1 появляется из дымки." );

    ch->position = ch->default_pos;
    homeVnum = 0;
    return false;
}


bool BasicMobileBehavior::isHomesick( ) 
{
    if (ch->position <= POS_SLEEPING)
        return false;
        
    if (ch->zone == 0 || ch->in_room->area == ch->zone)
        return false;

    if (ch->switchedFrom)
        return false;

    if (ch->fighting || hasLastFought( ))
        return false;
    
    if (IS_CHARMED(ch) || RIDDEN(ch))
        return false;

    if (ch->getWrapper( ))
        return false;
    
    return true;
}

/*-------------------------------------------------------------------------
 * BasicMobileBehavior - triggers 
 *------------------------------------------------------------------------*/
bool BasicMobileBehavior::area( )
{
    memoryFought.poll( 60 * 60 * 24 ); // one day to remember (one day to forget...)
    memoryAttacked.poll( 60 * 10 );

    if (isHomesick( ) && backHome( true ))
        return true;

    return checkLastFoughtHiding();
}

bool BasicMobileBehavior::checkLastFoughtHiding()
{
    PCharacter *pch;

    if (!hasLastFought())
        return false;

    DLString savedLastFought = lastFought;

    // Check if something else, other than last fought, is preventing them from going home.
    lastFought.clear();
    if (!isHomesick()) {
        lastFought = savedLastFought;
        return false;
    }

    lastFought = savedLastFought;
    if (!(pch = getLastFoughtWorld()))
        return false;
    
    if (!pch->in_room || pch->in_room->area != ch->in_room->area)
        return false;

    if (!IS_SET(pch->in_room->room_flags, ROOM_LAW) || !IS_SET(pch->in_room->room_flags, ROOM_SAFE))
        return false;

    // Complain and go home if victim is hiding in a safe law room.

    DLString moan = fmt(NULL, "%1$^C1, подл%1$Gое|ый|ая трус%1$Gло||иха, я еще до тебя доберусь!", pch);
    interpret_raw(ch, "yell", moan.c_str());

    backHome(true);
    clearLastFought();

    return true;
}

bool BasicMobileBehavior::spell( Character *caster, int sn, bool before ) 
{ 
    // successfull charm person
    if (sn == gsn_charm_person || sn == gsn_attract_other || sn == gsn_control_undead) {
        if (caster->is_npc( ))
            return false;

        if (before) {
            beforeSpell = !(IS_CHARMED(ch));
            return false;
        }
        
        if (!beforeSpell || !IS_CHARMED(ch))
            return false;

        if (number_percent( ) < (4 + ch->getModifyLevel( ) 
                                   - caster->getModifyLevel( )) * 10)
            memoryFought.remember( caster );
        
        lastCharmTime = dreamland->getCurrentTime( );
        remember( ch->in_room );

        return false;
    }

    // successfull summon
    if (sn == gsn_summon) {
        if (before) {
            beforeSpell = ch->in_room->vnum;
            return false;
        }
        
        if (beforeSpell != ch->in_room->vnum) 
            remember( get_room_instance( beforeSpell ) );

        return false;
    }

    return false;
}

bool BasicMobileBehavior::kill( Character *victim )
{
    if (memoryFought.memorized( victim )) 
        interpret_raw( ch, "say", "Возмездие совершено! Наконец!");

    return false;
}

bool BasicMobileBehavior::extractNotify( Character *wch, bool fTotal, bool fCount )
{
    if (ch == wch)
        return true;

    // quit
    if (fTotal && !fCount) { 
        if (isLastFought( wch )) {
            clearLastFought( );
                
            if (isHomesick( ))
                backHome( false );
        }

        return true;
    }
   
    // delete, remort or death
    if (fCount || !fTotal) { 
        bool changed = false;

        if (isLastFought( wch )) {
            clearLastFought( );
            changed = true;
        }

        if (memoryFought.forget( wch )) 
            changed = true;

        if (changed)
            if (memoryFought.empty( ) && isHomesick( ))
                backHome( false ); 

        return true;
    }
    
    return true;
}

bool BasicMobileBehavior::canCancel( Character *caster )
{
    return !caster->is_npc( )
            && IS_CHARMED(ch)
            && ch->master == caster;
}


/*-------------------------------------------------------------------------
 * BasicMobileBehavior - experience bonuses  
 *------------------------------------------------------------------------*/
/*
 * count all mobiles around us that took part in the bloody mess
 */
void BasicMobileBehavior::findEnemiesRoom( Room *room, int klevel, list<NPCharacter *> &enemies, list<PCharacter *> &group )
{
    for (Character *rch = room->people; rch; rch = rch->next_in_room) {
        BasicMobileBehavior::Pointer bhv;

        if (!rch->is_npc( ) || !rch->getNPC( )->behavior)
            continue;
        if (!( bhv = rch->getNPC( )->behavior.getDynamicPointer<BasicMobileBehavior>( ) ))
            continue;
        if (klevel - rch->getModifyLevel( ) > 10 * max( 1 , klevel / 30 ))
            continue;

        for (list<PCharacter *>::iterator g = group.begin( ); g != group.end( ); g++)
            if (bhv->memoryAttacked.memorized( *g )) {
                enemies.push_back( rch->getNPC( ) );
                break;
            }
    }
}

struct BasicMobileBehavior::FindAllEnemiesComplete {
    typedef NodesEntry<RoomTraverseTraits> MyNodesEntry;
    FindAllEnemiesComplete( Room *s, int d, int k, list<NPCharacter *> &e, list<PCharacter *> &g )
                             : startRoom( s ), depth( d ), klevel( k ), enemies( e ), group( g )
    { 
    }
    inline bool operator () ( const MyNodesEntry *const head, bool last ) 
    {
        if (head->node != startRoom)
            BasicMobileBehavior::findEnemiesRoom( head->node, klevel, enemies, group );

        if (head->generation < depth && !last)
            return false;

        return true;
    }
    
    Room *startRoom;
    int depth;
    int klevel;
    list<NPCharacter *> &enemies;
    list<PCharacter *> &group;
};

struct DoorFunc {
    DoorFunc( ) { }
    bool operator () ( Room *const room, EXIT_DATA *exit ) const { return true; }
};

struct ExtraExitFunc {
    ExtraExitFunc( ) { }
    bool operator () ( Room *const room, EXTRA_EXIT_DATA *eexit ) const { return false; }
};

struct PortalFunc {
    PortalFunc( ) { }
    bool operator () ( Room *const room, Object *portal ) const { return false; }
};

typedef RoomRoadsIterator<DoorFunc, ExtraExitFunc, PortalFunc> MyHookIterator;

int BasicMobileBehavior::getExpBonus( Character *killer )
{
    DoorFunc df; ExtraExitFunc eef; PortalFunc pf;
    MyHookIterator iter( df, eef, pf );
    list<NPCharacter *> enemies;
    list<PCharacter *> group;
    int bonus = 0;
    int klevel = 0;

    for (Character *rch = killer->in_room->people; rch; rch = rch->next_in_room)
        if (!rch->is_npc( ) && is_same_group( rch, killer )) {
            group.push_back( rch->getPC( ) );
            klevel += rch->getModifyLevel( );
        }

    klevel /= group.size( );
    FindAllEnemiesComplete complete( killer->in_room, 4, klevel, enemies, group );

    room_traverse<MyHookIterator, FindAllEnemiesComplete>( 
            killer->in_room, iter, complete, 10000 );
    
    findEnemiesRoom( killer->in_room, klevel, enemies, group );
    
    for (list<NPCharacter *>::iterator e = enemies.begin( ); e != enemies.end( ); e++) 
        bonus += number_range( 1, (*e)->getModifyLevel( ) - klevel + 30 );
   
    if (group.size( ) > 2)
        bonus -= bonus / 4 * group.size( );

    return max( 0, bonus );
}

BasicMobileBehavior::BasicMobileBehavior( ) 
                        : lostTrack( false )
{
}


BasicMobileBehavior::~BasicMobileBehavior( ) 
{
}

/*
 * BasicMobileDestiny
 */
bool BasicMobileDestiny::isSaved( ) const
{
    return true;
}

bool BasicMobileDestiny::hasDestiny( )
{
    return true;
}

