#include <map>

#include "item_progs.h"
#include "descriptor.h"
#include "dreamland.h"
#include "character.h"
#include "npcharacter.h"
#include "core/object.h"
#include "affect.h"
#include "affecthandler.h"
#include "spelltarget.h"
#include "wrapperbase.h"
#include "wrappertarget.h"
#include "core/behavior/behavior_utils.h"
#include "../loadsave/behavior_utils.h"
#include "loadsave.h"
#include "interp.h"
#include "areaquestutils.h"
#include "follow_utils.h"
#include "merc.h"
#include "def.h"
#include "transferlog.h"
#include "l10n.h"
#include "fenia_utils.h"

/* RT part of the corpse looting code */
static bool oprog_get_money( Character *ch, Object *obj )
{
    ch->silver += obj->value0();
    ch->gold += obj->value1();

    if (obj->pIndexData->vnum > 5 && (obj->value0() > 0 || obj->value1() > 0)) {
        DLString moneyArg = Money::describe(obj->value1( ), obj->value0( ), 4, viewerLang(ch));
        ch->pecho(_("Твой кошелек пополнился на %s."), moneyArg.c_str());
    }

    // 'autosplit' config option retired -- coins are always split with the party.
    if (!ch->is_npc( ))
        if (obj->value0() > 1 || obj->value1())
            if (party_members_room( ch ).size( ) > 1)
                interpret_raw( ch, "split", "%d %d", obj->value0(), obj->value1() );
    
    extract_obj( obj );
    return true;
}

/*
 * Handoff stamps: object ID -> the player who last let go of it. Keyed by ID,
 * not pointer, so an extracted object leaves only a stale entry that the TTL
 * sweep drops, never a dangling pointer.
 */
struct HandoffStamp {
    DLString name;
    DLString host;
    time_t when = 0;
    // A coin pile merged with coins already in the container: only part of it
    // is the putter's. Ledger only, the mule alert skips it. gold and silver
    // are what the putter added.
    bool merged = false;
    int gold = 0;
    int silver = 0;
};

static std::map<long long, HandoffStamp> handoffStamps;
static const time_t HANDOFF_TTL = 6 * 60 * 60;
static const size_t HANDOFF_SWEEP_SIZE = 2000;

static void handoff_stamp_rec( Object *obj, const HandoffStamp &stamp )
{
    handoffStamps[obj->getID( )] = stamp;

    for (Object *o = obj->contains; o; o = o->next_content)
        handoff_stamp_rec( o, stamp );
}

static void handoff_clear_rec( Object *obj )
{
    handoffStamps.erase( obj->getID( ) );

    for (Object *o = obj->contains; o; o = o->next_content)
        handoff_clear_rec( o );
}

static void handoff_stamp( Object *obj, Character *ch, const HandoffStamp &proto )
{
    if (!obj || !ch || ch->is_npc( ) || obj->extracted)
        return;

    time_t now = dreamland->getCurrentTime( );

    if (handoffStamps.size( ) > HANDOFF_SWEEP_SIZE) {
        for (auto it = handoffStamps.begin( ); it != handoffStamps.end( ); )
            if (now - it->second.when > HANDOFF_TTL)
                it = handoffStamps.erase( it );
            else
                ++it;
    }

    HandoffStamp stamp = proto;
    stamp.name = ch->getNameC( );
    stamp.host = ch->desc ? ch->desc->getRealHost( ) : DLString::emptyString;
    stamp.when = now;
    handoff_stamp_rec( obj, stamp );
}

void oprog_handoff_stamp( Object *obj, Character *ch )
{
    handoff_stamp( obj, ch, HandoffStamp( ) );
}

void oprog_handoff_clear( Object *obj )
{
    if (obj)
        handoff_clear_rec( obj );
}

void oprog_handoff_stamp_merged( Object *money, Character *ch, int gold, int silver )
{
    HandoffStamp proto;
    proto.merged = true;
    proto.gold = gold;
    proto.silver = silver;
    handoff_stamp( money, ch, proto );
}

// One transfer ledger row: ch picked up obj, which stamp's player let go of.
static void handoff_row( const HandoffStamp &stamp, Object *obj, Character *ch, time_t now )
{
    Json::Value extra;
    extra["ago"] = (int)(now - stamp.when);
    if (stamp.merged) {
        extra["merged"] = true;
        extra["added_gold"] = stamp.gold;
        extra["added_silver"] = stamp.silver;
    }
    TransferLog::record( "pickup", stamp.name, ch->getNameC( ),
                         ch->in_room ? ch->in_room->vnum : 0, obj, extra );
}

// What other players put into a container gets its own rows when the whole
// container is taken: a live stamp naming neither the taker nor `covered` (the
// player whose row already counts the container's contents).
static void handoff_ledger_contents( Object *container, Character *ch, time_t now,
                                     const DLString &covered )
{
    for (Object *o = container->contains; o; o = o->next_content) {
        auto it = handoffStamps.find( o->getID( ) );
        if (it != handoffStamps.end( )
            && it->second.name != ch->getNameC( ) && it->second.name != covered
            && now - it->second.when <= HANDOFF_TTL)
        {
            handoff_row( it->second, o, ch, now );
            handoff_ledger_contents( o, ch, now, it->second.name );
        }
        else
            handoff_ledger_contents( o, ch, now, covered );
    }
}

// Clears the stamps whoever takes the object (a scavenger mob too), so a stamp
// always means "let go and not picked up since". Window: HANDOFF_TTL.
static void handoff_check( Object *obj, Character *ch )
{
    if (handoffStamps.empty( ))
        return;

    time_t now = dreamland->getCurrentTime( );
    auto it = handoffStamps.find( obj->getID( ) );

    // Live and let go of by another player: the mule verdict and a ledger row.
    HandoffStamp stamp;
    bool handed = false;
    if (it != handoffStamps.end( )) {
        stamp = it->second;
        handed = !ch->is_npc( ) && stamp.name != ch->getNameC( )
                 && now - stamp.when <= HANDOFF_TTL;
    }

    if (handed)
        handoff_row( stamp, obj, ch, now );

    // Before the stamps go: a bag the taker dropped himself, or one whose own
    // stamp is old, can hold what someone else put in a minute ago.
    if (!ch->is_npc( ))
        handoff_ledger_contents( obj, ch, now, handed ? stamp.name : DLString::emptyString );

    // Clear the contents too: a bag handed over by give carries nothing stale
    // out of the previous floor stay.
    handoff_clear_rec( obj );

    if (!handed || stamp.merged)
        return;

    gprog( "onHandoff", "COssi", ch, obj, stamp.name.c_str( ), stamp.host.c_str( ),
           (int)(now - stamp.when) );
}

bool oprog_get( Object *obj, Character *ch )
{
    // Named items go back on the floor no matter who tried to take them.
    // oprog_get itself is called last in the give chain, so a quest handler
    // that consumes what it was given has already had its say by now.
    if (!obj_owner_enforce( obj, ch ))
        return true;

    handoff_check( obj, ch );
    if (obj->extracted)
        return true;

    aquest_trigger(obj, ch, "Get", "OC", obj, ch);

    if (behavior_trigger(obj, "Get", "OC", obj, ch))
        return true;

    FENIA_CALL( obj, "Get", "C", ch );
    FENIA_NDX_CALL( obj, "Get", "OC", obj, ch );
    BEHAVIOR_VOID_CALL( obj, get, ch );

    for (auto &paf: obj->affected.findAllWithHandler())
        if (paf->type->getAffect() && paf->type->getAffect()->onGet(SpellTarget::Pointer(NEW, obj), paf, ch))
            return true;

    if (obj->extracted)
        return true;

    switch (obj->item_type) {
    case ITEM_MONEY:
        return oprog_get_money( ch, obj );
    }

    return false;
}

bool oprog_give_blocked( Object *obj, Character *ch, Character *victim )
{
    for (auto &paf: obj->affected.findAllWithHandler())
        if (paf->type->getAffect() && paf->type->getAffect()->onGive(SpellTarget::Pointer(NEW, obj), paf, ch, victim))
            return true;

    return false;
}

bool oprog_drop( Object *obj, Character *ch )
{
    oprog_handoff_stamp( obj, ch );

    if (behavior_trigger(obj, "Drop", "OC", obj, ch))
        return true;

    FENIA_CALL( obj, "Drop", "C", ch )
    FENIA_NDX_CALL( obj, "Drop", "OC", obj, ch )
    BEHAVIOR_CALL( obj, drop, ch )

    return false;
}

/*
 * Something was eaten: a pill (use_pill) or food (the Fenia 'eat' command, via
 * obj.trigger("Eat")). The stomach is already filled. True keeps the object.
 */
bool oprog_eat( Object *obj, Character *ch )
{
    if (behavior_trigger(obj, "Eat", "OC", obj, ch))
        return true;

    FENIA_CALL( obj, "Eat", "C", ch );
    FENIA_NDX_CALL( obj, "Eat", "OC", obj, ch );
    return false;
}

/*
 * A whole hand-over: area quests, behaviors and both sides' Fenia triggers each
 * get a say, and any of them may take the item back, which is what every
 * `obj->carried_by != victim` bail-out is checking. Lives here rather than in
 * the 'give' command because 'request' and the Fenia obj.trigger("Give") wrapper
 * need the same chain.
 */
bool omprog_give( Object *obj, Character *ch, Character *victim )
{
    if (aquest_trigger(obj, ch, "Give", "OCC", obj, ch, victim))
        return true;
    if (obj->carried_by != victim)
        return true;

    if (aquest_trigger(victim, ch, "Give", "CCO", victim, ch, obj))
        return true;
    if (obj->carried_by != victim)
        return true;

    if (behavior_trigger(victim, "Give", "CCO", victim, ch, obj))
        return true;        
    if (obj->carried_by != victim)
        return true;

    FENIA_CALL( obj, "Give", "CC", ch, victim )
    if (obj->carried_by != victim)
        return true;

    FENIA_NDX_CALL( obj, "Give", "OCC", obj, ch, victim )
    if (obj->carried_by != victim)
        return true;

    BEHAVIOR_VOID_CALL( obj, give, ch, victim )
    if (obj->carried_by != victim)
        return true;
    
    FENIA_CALL( victim, "Give", "CO", ch, obj );
    if (obj->carried_by != victim)
        return true;

    FENIA_NDX_CALL( victim->getNPC( ), "Give", "CCO", victim, ch, obj );
    if (obj->carried_by != victim)
        return true;

    BEHAVIOR_VOID_CALL( victim->getNPC( ), give, ch, obj );        
    if (obj->carried_by != victim)
        return true;

    // Last word for any NPC, after every specific trigger above had its say:
    // e.g. a bought pet eats food its owner hands it (global/onGive).
    if (victim->is_npc( )) {
        gprog("onGive", "CCO", victim, ch, obj);
        if (obj->carried_by != victim)
            return true;
    }
        
    return oprog_get( obj, victim );
}
