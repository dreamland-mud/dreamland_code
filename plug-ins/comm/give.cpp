#include "pcharacter.h"
#include "npcharacter.h"
#include "core/object.h"
#include "room.h"
#include "commandtemplate.h"
#include "areaquestutils.h"
#include "item_progs.h"
#include "../loadsave/behavior_utils.h"
#include "core/behavior/behavior_utils.h"
#include "wrapperbase.h"
#include "wrappertarget.h"
#include "wearloc_utils.h"
#include "interp.h"
#include "act.h"
#include "loadsave.h"
#include "merc.h"
#include "def.h"
#include "follow_utils.h"
#include "clanreference.h"
#include "arg_utils.h"
#include "l10n.h"

/*
 * 'give' command
 */
#define GIVE_MODE_USUAL   0
#define GIVE_MODE_PRESENT 1


static bool oprog_present( Object *obj, Character *ch, Character *victim )
{
    FENIA_CALL( obj, "Present", "CC", ch, victim );
    FENIA_NDX_CALL( obj, "Present", "OCC", obj, ch, victim );
    return false;
}

// 'config notransfer': a PC refuses items/money from third-party players (not
// self, not group, not same clan). NPC gives (shops, quests) always pass.
// See Trello 709 / Nimly.
static bool give_blocked_notransfer( Character *ch, Character *victim )
{
    if (ch->is_npc( ) || victim->is_npc( ) || !IS_SET(victim->add_comm, COMM_NOTRANSFER))
        return false;
    if (ch == victim || is_same_group( ch, victim ))
        return false;
    if (!ch->getClan( )->isDispersed( ) && ch->getClan( ) == victim->getClan( ))
        return false;
    return true;
}

static void give_obj_char( Character *ch, Object *obj, Character *victim, int mode = GIVE_MODE_USUAL )
{
    if (ch == victim) {
        ch->pecho(_("%s себе?"), (mode ? "Подарить" : "Дать"));
        return;
    }

    if ( !victim->is_npc() && IS_GHOST( victim ) )
    {
        ch->pecho(_("Разве можно что-то %s призраку?"), (mode ? "подарить" : "дать"));
        return;
    }

    if ( give_blocked_notransfer( ch, victim ) )
    {
        ch->pecho( _("%1$^C1 не принима%1$nет|ют вещей от посторонних."), victim );
        return;
    }

    if ( !Item::canDrop( ch, obj ) )
    {
        ch->pecho(_("Ты не можешь избавиться от этого."));
        return;
    }

    if ( victim->carry_number + obj->getNumber( ) > Char::canCarryNumber(victim) )
    {
		ch->pecho( _("%1$^C1 не мо%1$nжет|гут нести столько вещей."), victim );
        return;
    }

    if (Char::getCarryWeight(victim) + obj->getWeight( ) > Char::canCarryWeight(victim) )
    {
		ch->pecho( _("%1$^C1 не мо%1$nжет|гут нести такую тяжесть."), victim );
        return;
    }

    if ( !victim->can_see( obj ) )
    {
		ch->pecho( _("%1$^C1 не вид%1$nит|ят этого."), victim );
        return;
    }

    if (obj->pIndexData->limit != -1)
    {
        if (obj->isAntiAligned( victim )) {
            ch->pecho(_("%1$^C1 не смо%1$nжет|гут владеть этой вещью."), victim);
            return;
        }
    }

    // A named item handed to a charmed follower used to transfer, announce
    // itself, and only then get bounced onto the floor by oprog_get -- without
    // a word, because that path stays silent for mobs. Refuse it up front
    // instead, so the item never leaves the giver's hands.
    //
    // Deliberately narrowed to followers rather than every mob: omprog_give
    // runs the whole quest and behavior chain BEFORE oprog_get ever looks at
    // ownership (item_progs.cpp), so a quest handler is allowed to consume a
    // named item today. A blanket refusal here would silently break that.
    if ( victim->is_npc( ) && IS_CHARMED( victim ) && !obj_owner_allows( obj, victim ) )
    {
        ch->pecho(_("%1$^C1 не смо%1$nжет|гут владеть этой вещью."), victim);
        return;
    }

    // Last gate before the item moves: an affect on the item can refuse the
    // hand-off and echo its own reason. See affect/incandescent/onGive.
    if ( oprog_give_blocked( obj, ch, victim ) )
        return;

    obj_from_char( obj );
    obj_to_char( obj, victim );

    switch (mode) {
    case GIVE_MODE_USUAL:
    default:
        oldact(_("$c1 дает $o4 $C3."), ch, obj, victim, TO_NOTVICT );
        oldact(_("$c1 дает тебе $o4."), ch, obj, victim, TO_VICT );
        oldact(_("Ты даешь $o4 $C3."), ch, obj, victim, TO_CHAR );
        break;

    case GIVE_MODE_PRESENT:
        oldact(_("$c1 дарит $o4 $C3."), ch, obj, victim, TO_NOTVICT );
        oldact(_("$c1 дарит тебе $o4."), ch, obj, victim, TO_VICT );
        oldact(_("Ты даришь $o4 $C3."), ch, obj, victim, TO_CHAR );

        if (oprog_present( obj, ch, victim ))
            return;

        break;
    }
    
    omprog_give( obj, ch, victim );
}

// 'give all <victim>' / 'give all.thing <victim>' (same for 'present'). The
// command never had an all-dot loop the way get/drop/put do, so 'give all.sword
// pet' just fell into get_obj_carry and answered "у тебя нет этого" -- and when
// a charmed pet was ordered to do it, that refusal went to the mob (unseen) and
// the player was left with a bare "Ok." (Zodd, Trello bugs). Each item still
// passes through give_obj_char, so every per-item gate (weight, notransfer,
// ownership, anti-align) holds; worn and unseen items are skipped.
// A Give trigger can extract the receiver mid-loop -- a quest mob that takes the
// item and vanishes (faeriering 29110). The pointer is reused through init(),
// which zeroes in_room, so a second give_obj_char to it would deref a null room
// in can_see. Stop the moment the giver or receiver leaves the room, or the
// giver dies -- the same guard get.cpp's loot loop uses (still_looting).
static bool still_giving( Character *ch, Character *victim, Room *room )
{
    // Both parties: a killed NPC keeps its in_room until the next pulse sweep,
    // so death and room-move are complementary signals for giver and receiver
    // alike. An item can kill the receiver mid-loop (a red-hot item's onGet
    // burn), and the rest would then pour into a corpse-to-be and vanish.
    return !ch->isDead( ) && !victim->isDead( )
        && ch->in_room == room && victim->in_room == room;
}

static void give_all_char( Character *ch, const DLString &argObj, Character *victim, int mode )
{
    bool fAll = arg_is_all( argObj );
    DLString objnames;
    if (!fAll) {
        DLString::size_type dot = argObj.find( '.' );
        if (dot != DLString::npos)
            objnames = argObj.substr( dot + 1 );
    }

    Room *startRoom = ch->in_room;
    bool found = false;
    Object *obj_next;
    for (Object *obj = ch->carrying; obj != 0; obj = obj_next) {
        obj_next = obj->next_content;

        // A prior item's Give trigger can extract a NEIGHBOUR carried item
        // (Object::extract zeroes carried_by/pIndexData/next_content); stop
        // before the loop dereferences it. No live trigger does this today.
        if (obj->carried_by != ch)
            break;

        if (obj->wear_loc != wear_none)
            continue;
        if (!ch->can_see( obj ))
            continue;
        if (!fAll && !obj_has_name( obj, objnames, ch ))
            continue;

        found = true;
        give_obj_char( ch, obj, victim, mode );

        // A per-item trigger may have extracted the receiver or moved either
        // party out of the room; giving the next item would then crash.
        if (!still_giving( ch, victim, startRoom ))
            break;
    }

    if (!found)
        ch->pecho( fAll ? _("У тебя ничего нет.") : _("У тебя нет этого.") );
}

static bool mprog_bribe( Character *victim, Character *giver, int gold, int silver )
{
    if (behavior_trigger(victim, "Bribe", "CCii", victim, giver, gold, silver))
        return true;
        
    FENIA_CALL( victim, "Bribe", "Cii", giver, gold, silver );
    FENIA_NDX_CALL( victim->getNPC( ), "Bribe", "CCii", victim, giver, gold, silver );
    BEHAVIOR_VOID_CALL( victim->getNPC( ), bribe, giver, gold, silver );
    return false;
}

/* 'give NNNN coins victim' */
static void give_money_char( Character *ch, int gold, int silver, Character *victim, int mode = GIVE_MODE_USUAL )
{
    if (ch == victim)
    {
            ch->pecho(_("Дать себе?"));
            return;
    }

    if ( !victim->is_npc() && IS_GHOST( victim ) )
    {
            ch->pecho(_("Разве можно что-то дать призраку?"));
            return;
    }

    if ( give_blocked_notransfer( ch, victim ) )
    {
            ch->pecho( _("%1$^C1 не принима%1$nет|ют денег от посторонних."), victim );
            return;
    }

    victim->silver  += silver;
    victim->gold    += gold;

    if (Char::getCarryWeight(victim)  > Char::canCarryWeight(victim))
    {
            victim->silver  -= silver;
            victim->gold    -= gold;
            ch->pecho( _("%1$^C1 не мо%1$nжет|гут нести такую тяжесть."), victim );
            return;
    }

    ch->silver      -= silver;
    ch->gold        -= gold;
    
    if (silver > 0) {
        DLString slv( silver );
        if (mode == GIVE_MODE_PRESENT) {
            oldact(_("$c1 дарит тебе $t серебра."), ch, slv.c_str( ), victim, TO_VICT);
            oldact(_("Ты даришь $C3 $t серебра."),ch, slv.c_str( ), victim, TO_CHAR);
        } else {
            oldact(_("$c1 дает тебе $t серебра."), ch, slv.c_str( ), victim, TO_VICT);
            oldact(_("Ты даешь $C3 $t серебра."),ch, slv.c_str( ), victim, TO_CHAR);
        }
    }
    
    if (gold > 0) {
        DLString gld( gold );
        if (mode == GIVE_MODE_PRESENT) {
            oldact(_("$c1 дарит тебе $t золота."), ch, gld.c_str( ), victim, TO_VICT);
            oldact(_("Ты даришь $C3 $t золота."),ch, gld.c_str( ), victim, TO_CHAR);
        } else {
            oldact(_("$c1 дает тебе $t золота."), ch, gld.c_str( ), victim, TO_VICT);
            oldact(_("Ты даешь $C3 $t золота."),ch, gld.c_str( ), victim, TO_CHAR);
        }
    }

    if (mode == GIVE_MODE_PRESENT) {
        oldact(_("$c1 дарит $C3 несколько монет."),  ch, 0, victim, TO_NOTVICT);
    } else {
        oldact(_("$c1 дает $C3 несколько монет."),  ch, 0, victim, TO_NOTVICT);
    }
    
    mprog_bribe( victim, ch, gold, silver );
}

static void give_money( Character *ch, char *arg1, char *arg2, char *argument, int mode = GIVE_MODE_USUAL )
{
    Character *victim;
    int amount;
    int gold = 0, silver = 0;

    amount   = atoi(arg1);
    if (!Money::parse( ch, arg2, amount, gold, silver ))
        return;

    argument = one_argument( argument, arg2 );
    if ( arg2[0] == '\0' )
    {
            ch->pecho(_("Дать что и кому?"));
            return;
    }

    if ( ( victim = get_char_room( ch, arg2 ) ) == 0 )
    {
            ch->pecho(_("Здесь таких нет."));
            return;
    }

    give_money_char( ch, gold, silver, victim, mode );
}

CMDRUNP( give )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    Character *victim;
    Object  *obj;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
            ch->pecho(_("Дать что и кому?"));
            return;
    }

    if (is_number( arg1 ) && get_arg_id( arg1 ) == 0) {
        give_money( ch, arg1, arg2, argument );
        return;
    }

    if ( ( victim = get_char_room( ch, arg2 ) ) == 0 )
    {
        ch->pecho(_("Здесь таких нет."));
        return;
    }

    if ( arg_is_alldot( arg1 ) ) {
        give_all_char( ch, arg1, victim, GIVE_MODE_USUAL );
        return;
    }

    if ( ( obj = get_obj_carry( ch, arg1 ) ) == 0 )
    {
        ch->pecho(_("У тебя нет этого."));
        return;
    }

    give_obj_char( ch, obj, victim );
}

CMDRUNP( present )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    Character *victim;
    Object *obj;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (arg1[0] == '\0' || arg2[0] == '\0') {
        ch->pecho( _("Что и кому ты хочешь подарить?") );
        return;
    }

    if (is_number( arg1 ) && get_arg_id( arg1 ) == 0) {
        give_money( ch, arg1, arg2, argument, GIVE_MODE_PRESENT );
        return;
    }

    if (( victim = get_char_room( ch, arg2 ) ) == 0) {
        ch->pecho( _("Они ушли, не дождавшись подарков.") );
        return;
    }

    if ( arg_is_alldot( arg1 ) ) {
        give_all_char( ch, arg1, victim, GIVE_MODE_PRESENT );
        return;
    }

    if (( obj = get_obj_carry( ch, arg1 ) ) == 0) {
        ch->pecho( _("У тебя нет этого.") );
        return;
    }

    give_obj_char( ch, obj, victim, GIVE_MODE_PRESENT );
}

