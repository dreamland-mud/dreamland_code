/* $Id$
 *
 * ruffina, 2004
 */
#include "wearloc_utils.h"
#include "commandtemplate.h"

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "behavior_utils.h"
#include "room.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "core/object.h"

#include "loadsave.h"
#include "save.h"
#include "act.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

WEARLOC(hair);
WEARLOC(tail);

static bool oprog_can_dress(Object *obj, Character *ch, Character *victim)
{
    FENIA_CALL( obj, "CanDress", "CC", ch, victim )
    FENIA_NDX_CALL( obj, "CanDress", "OCC", obj, ch, victim )
    BEHAVIOR_CALL(obj, canDress, ch, victim)
    return false;
}

static void oprog_dress(Object *obj, Character *ch, Character *victim)
{
    FENIA_VOID_CALL( obj, "Dress", "CC", ch, victim )
    FENIA_NDX_VOID_CALL( obj, "Dress", "OCC", obj, ch, victim )
}


/*
 * 'wear' command
 * 'wear <obj> [to <victim>]'
 * 'wear all'
 */
CMDRUNP( wear )
{
    Character *victim = ch;
    Object *obj;
    char cArg[MAX_INPUT_LENGTH];
    char argObj[MAX_INPUT_LENGTH], argTo[MAX_INPUT_LENGTH], argVict[MAX_INPUT_LENGTH];
    bool fHair = false;
    bool fTail = false;	
    
    strcpy( cArg, argument );
    argument = one_argument( argument, argObj );
    argument = one_argument( argument, argTo );
    argument = one_argument( argument, argVict );

    if (!argObj[0]) {
        ch->println("Надеть, вооружиться или взять это в руки?");
        return;
    }
    
    if (arg_is_to( argTo ) || arg_is_in( argTo ) || arg_is_on(argTo)) {
        if (arg_oneof( argVict, "волосы", "hair" )) {
            fHair = true;
        }
        else if (arg_oneof(argVict, "хвост", "tail")) {
            fTail = true;
        }
        else if (( victim = get_char_room( ch, argVict  ) ) == 0) {
            ch->println("На кого ты хочешь это надеть?");
            return;
        } else if (victim != ch && !victim->is_npc( )) {
            act("$C1 в состоянии одеться са$Gмо|м|ма!", ch, 0, victim, TO_CHAR);
            return;
        }
    }
    else 
        one_argument( cArg, argObj );
    
    if (arg_is_all( argObj )) {
        Object *obj_next;
        
        if (victim != ch) {
            ch->println("Ты не можешь сделать этого.");
            return;
        }
        
        for (obj = ch->carrying; obj != 0; obj = obj_next) {
            obj_next = obj->next_content;
            
            if (obj->wear_loc == wear_none && ch->can_see( obj ))
                wear_obj( ch, obj, F_WEAR_VERBOSE );
        }

        return;
    }
    
    if (( obj = get_obj_carry( ch, argObj ) ) == 0) {
        ch->println("У тебя нет этого.");
        return;
    }

    if (ch == victim && fHair) {
        if (obj->getWeight( ) / 10 > 3) {
            ch->pecho( "%1$^O1 слишком тяжел%1$Gое|ый|ая|ые, чтобы удержаться в твоих волосах.", obj );
            return;
        }

        wear_hair->wear( obj, F_WEAR_VERBOSE );
        return;
    }
        
    if (ch == victim && fTail) {
        if (obj->getWeight( ) / 10 > 4) {
            ch->pecho( "%1$^O1 слишком тяжел%1$Gое|ый|ая|ые, чтобы удержаться на твоем хвосте.", obj );
            return;
        }

        wear_tail->wear( obj, F_WEAR_VERBOSE );
        return;
    }

    if (ch == victim) {
        if (wear_obj( ch, obj, F_WEAR_VERBOSE | F_WEAR_REPLACE) == RC_WEAR_NOMATCH)
            ch->println("Ты не можешь надеть, вооружиться или держать это в руках.");
        return;
    }
    
    if (!oprog_can_dress(obj, ch, victim)) {
        act("Ты не сможешь надеть $o4 на $C4.", ch, obj, victim, TO_CHAR);
        return;
    }

    obj_from_char( obj );
    obj_to_char( obj, victim );
    
    if (wear_obj( victim, obj, 0 ) != RC_WEAR_OK) {
        if (obj->carried_by == victim) {
            obj_from_char( obj );
            obj_to_char( obj, ch );
        }
        act("Ты пытаешься надеть $o4 на $C4, но безуспешно.", ch, obj, victim, TO_CHAR);
        act("$c1 пытается надеть на тебя $o4, но не может.", ch, obj, victim, TO_VICT);
        act("$c1 пытается надеть на $C4 $o4, но не может.", ch, obj, victim, TO_NOTVICT);
        return;
    }

    act("Ты надеваешь $o4 на $C4.", ch, obj, victim, TO_CHAR);
    act("$c1 надевает на тебя $o4.", ch, obj, victim, TO_VICT);
    act("$c1 надевает на $C4 $o4.", ch, obj, victim, TO_NOTVICT);
    oprog_dress(obj, ch, victim);	
}



/*
 * 'remove' command
 * 'remove <obj> [from <victim>]'
 * 'remove all'
 */
CMDRUNP( remove )
{
    Character *victim = ch;
    Object *obj;
    char cArg[MAX_INPUT_LENGTH];
    char argObj[MAX_INPUT_LENGTH], argFrom[MAX_INPUT_LENGTH], argVict[MAX_INPUT_LENGTH];
    
    strcpy( cArg, argument );
    argument = one_argument( argument, argObj );
    argument = one_argument( argument, argFrom );
    argument = one_argument( argument, argVict );

    if (!argObj[0]) {
        ch->println("Снять что?");
        return;
    }

    if (arg_is_from( argFrom )) {
        if (( victim = get_char_room( ch, argVict ) ) == 0) {
            ch->println("С кого ты хочешь это снять?");
            return;
        }
        
        if (victim != ch && !victim->is_npc( )) {
            act("$C1 в состоянии раздеться са$Gмо|м|ма!", ch, 0, victim, TO_CHAR);
            return;
        }
    }
    else
        one_argument( cArg, argObj );
    
    if (arg_is_all( argObj )) {
        Object *obj_next;

        if (victim != ch) {
            ch->println("Ты не можешь сделать этого.");
            return;
        }

        for (obj = ch->carrying; obj != 0; obj = obj_next) {
            obj_next = obj->next_content;

            if (ch->can_see( obj ))
                obj->wear_loc->remove( obj, F_WEAR_VERBOSE );
        }

        return;
    }
    
    if (ch == victim) {
        if (( obj = get_obj_wear( ch, argObj ) ) == 0) {
            ch->println("У тебя нет этого.");
            return;
        }

        obj->wear_loc->remove( obj, F_WEAR_VERBOSE );
        return;
    }
    
    if (( obj = get_obj_wear_victim( victim, argObj, ch ) ) == 0) {
        act("У $C2 нет этого.", ch, 0, victim, TO_CHAR);
        return;
    }

    if (!obj->behavior || !obj->behavior->canDress( ch, victim )) {
        act("Ты не сможешь снять $o4 с $C2.", ch, obj, victim, TO_CHAR);
        return;
    }
    
    if (!obj->wear_loc->remove( obj, 0 )) {
        act("Ты пытаешься снять $o4 с $C2, но безуспешно.", ch, obj, victim, TO_CHAR);
        act("$c1 пытается снять с тебя $o4, но не может.", ch, obj, victim, TO_VICT);
        act("$c1 пытается снять с $C2 $o4, но не может.", ch, obj, victim, TO_NOTVICT);
        return;
    }
    
    act("Ты снимаешь $o4 с $C2.", ch, obj, victim, TO_CHAR);
    act("$c1 снимает с тебя $o4.", ch, obj, victim, TO_VICT);
    act("$c1 снимает с $C2 $o4.", ch, obj, victim, TO_NOTVICT);
    
    if (obj->carried_by == victim) {
        obj_from_char( obj );
        obj_to_char( obj, ch );
    }
}


