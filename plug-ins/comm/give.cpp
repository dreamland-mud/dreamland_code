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
#include "l10n.h"

/*
 * 'give' command
 */
#define GIVE_MODE_USUAL   0
#define GIVE_MODE_PRESENT 1

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
        
    return oprog_get( obj, victim );
}

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

    if ( ( obj = get_obj_carry( ch, arg1 ) ) == 0 )
    {
        ch->pecho(_("У тебя нет этого."));
        return;
    }

    if ( ( victim = get_char_room( ch, arg2 ) ) == 0 )
    {
        ch->pecho(_("Здесь таких нет."));
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

    if (( obj = get_obj_carry( ch, arg1 ) ) == 0) {
        ch->pecho( _("У тебя нет этого.") );
        return;
    }

    if (( victim = get_char_room( ch, arg2 ) ) == 0) {
        ch->pecho( _("Они ушли, не дождавшись подарков.") );
        return;
    }
    
    give_obj_char( ch, obj, victim, GIVE_MODE_PRESENT );
}

