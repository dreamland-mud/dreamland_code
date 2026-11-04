/* $Id: gangchef.cpp,v 1.1.2.4.6.2 2008/04/14 19:36:15 rufina Exp $
 *
 * ruffina, 2003
 */

#include "gangsters.h"
#include "gangstersinfo.h"
#include "aitrigger.h"
#include "xmlattributegangsters.h"
#include "gangchef.h"

#include "mobbody.h"
#include "skillreference.h"
#include "mobtiers.h"
#include "room.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "fenia_utils.h"
#include "fight.h"
#include "loadsave.h"
#include "act.h"
#include "def.h"
#include "l10n.h"

GSN(sanctuary);
GSN(dark_shroud);
GSN(stardust);
GSN(resistance);

static bool chef_shielded( NPCharacter *ch )
{
    return ch->isAffected( gsn_sanctuary ) || ch->isAffected( gsn_dark_shroud )
        || ch->isAffected( gsn_stardust ) || ch->isAffected( gsn_resistance );
}

// The chef is the lair boss: boss-tier numbers and bits for the quest's top
// level. The prototype has no level, so its derived numbers are level-0 ones,
// and the gang member formula alone left the chef no tougher than his men.
// The instance tier stays the prototype's on purpose: the chef's loot is the
// bounty (onGangChefLoot), a boss tier would add tier loot on top of it.
void GangChef::config( int level )
{
    GangMob::config( level );

    const MobTiers::Config &tc = MobBody::tiers( );
    int boss = tc.parse( "boss" );
    if (!boss || !apply_mob_tier( ch, level, boss ))
        return;

    const MobTiers::Tier &row = tc.get( boss );
    Body::NameSet aff( row.affAdd.begin( ), row.affAdd.end( ) );
    Body::NameSet act( row.actAdd.begin( ), row.actAdd.end( ) );
    Body::NameSet off( row.offAdd.begin( ), row.offAdd.end( ) );
    bitstring_t affBits = MobBody::bits( &::affect_flags, aff );
    bitstring_t actBits = MobBody::bits( &::act_flags, act );
    bitstring_t offBits = MobBody::bits( &::off_flags, off );

    SET_BIT( ch->act, (int)actBits );
    SET_BIT( ch->off_flags, (int)offBits );
    ch->baseBits[MOBSET_ACT] |= actBits;
    ch->baseBits[MOBSET_OFF] |= offBits;

    // Sanctuary and the like live as real affects, as create_mobile builds
    // them (dark shroud for an evil chef), so they can be dispelled. Plain
    // bits go to the base set; the affect ones never do, or affect_check
    // would bring a dispelled shield back. create_mob_affects walks every
    // affect bit, so it runs only while the chef has none of them yet.
    bitstring_t fromAffects = affBits & AFF_FROM_AFFECTS;
    ch->baseBits[MOBSET_AFF] |= affBits & ~(bitstring_t)AFF_FROM_AFFECTS;
    SET_BIT( ch->affected_by, (int)(affBits & ~(bitstring_t)AFF_FROM_AFFECTS) );

    if (fromAffects && !(ch->affected_by & AFF_FROM_AFFECTS) && !chef_shielded( ch )) {
        SET_BIT( ch->affected_by, (int)fromAffects );
        create_mob_affects( ch );
    }
}

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

