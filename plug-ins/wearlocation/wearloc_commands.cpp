/* $Id$
 *
 * ruffina, 2004
 */
#include <string.h>

#include "wearloc_utils.h"
#include "misc_wearlocs.h"
#include "commandtemplate.h"

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "core/behavior/behavior_utils.h"
#include "room.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "core/object.h"

#include "fight_exception.h"
#include "loadsave.h"
#include "save.h"
#include "act.h"
#include "merc.h"

#include "def.h"
#include "l10n.h"

WEARLOC(hair);
WEARLOC(tail);
// wear_personal comes as an extern from wearloc_utils.h

/* True for anything that can never come off: religious and crafted tattoos.
 * By item type, not by wearloc -- craft tattoos live in their own wearloc family
 * (tat_face, tat_arms, tat_wrist_l, tat_wrist_r), so a wearloc test misses them.
 * Same pair as plug-ins/clan/impl/ruler.cpp:918.
 *
 * Deliberately NOT canRemove(): that would also skip cursed gear, and the curse
 * refusal is something the player wants to hear. */
static bool is_tattoo( Object *obj )
{
    return obj->item_type == ITEM_TATTOO || obj->item_type == ITEM_CRAFT_TATTOO;
}

/*
 * Same search as get_obj_wear, minus tattoos.
 *
 * A tattoo answers to ordinary words -- 'знак', a god's name, or for a crafted
 * one its picture ('весы', 'порядок') -- and can never be taken off, so it used
 * to swallow 'remove <word>' and refuse while the item the player actually meant
 * sat there still worn. Player idea 16125.
 */
static Object *get_obj_wear_no_tattoo( Character *ch, const char *cargument )
{
    char argument[MAX_INPUT_LENGTH], arg[MAX_INPUT_LENGTH];
    Object *obj;
    int number, count;

    if (get_arg_id( cargument ) != 0)
        return 0; // an explicit id means the player named this exact object

    strcpy( argument, cargument );
    number = number_argument( argument, arg );
    count  = 0;

    for (obj = ch->carrying; obj != 0; obj = obj->next_content)
        if (obj->wear_loc != wear_none
            && !is_tattoo( obj )
            && obj_has_name( obj, arg, ch )
            && ++count == number)
                return obj;

    return 0;
}

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
    bool fSlot = false;

    strcpy( cArg, argument );
    argument = one_argument( argument, argObj );
    argument = one_argument( argument, argTo );
    argument = one_argument( argument, argVict );

    if (!argObj[0]) {
        ch->pecho(_("Надеть, вооружиться или взять это в руки?"));
        return;
    }
    
    if (arg_is_to( argTo ) || arg_is_in( argTo ) || arg_is_on(argTo)) {
        if (arg_is(argVict, "hair")) {
            fHair = true;
        }
        else if (arg_is(argVict, "tail")) {
            fTail = true;
        }
        else if (arg_is(argVict, "slot")) {
            fSlot = true;
        }
        else if (( victim = get_char_room( ch, argVict  ) ) == 0) {
            ch->pecho(_("На кого ты хочешь это надеть?"));
            return;
        } else if (victim != ch && !victim->is_npc( )) {
            oldact(_("$C1 в состоянии одеться са$Gмо|м|ма!"), ch, 0, victim, TO_CHAR);
            return;
        }
    }
    else 
        one_argument( cArg, argObj );
    
    if (arg_is_all( argObj )) {
        Object *obj_next;
        
        if (victim != ch) {
            echo_master(ch, _("Ты не можешь 'надеть всё' на %C2 -- только на себя."), victim);
            return;
        }
        
        try {
            for (obj = ch->carrying; obj != 0; obj = obj_next) {
                obj_next = obj->next_content;
                
                if (obj->wear_loc == wear_none && ch->can_see( obj ))
                    wear_obj( ch, obj, F_WEAR_VERBOSE );
            }
        } catch (const VictimDeathException &) {
        }

        return;
    }
    
    if (( obj = get_obj_carry( ch, argObj ) ) == 0) {
        echo_master(ch, _("У тебя нет этого."));
        return;
    }

    if (ch == victim && fHair) {
        if (obj->getWeight( ) / 10 > 3) {
            echo_master(ch, _("%1$^O1 слишком тяжел%1$Gое|ый|ая|ые, чтобы удержаться в твоих волосах."), obj);
            return;
        }

        wear_hair->wear( obj, F_WEAR_VERBOSE );
        return;
    }
        
    if (ch == victim && fTail) {
        if (obj->getWeight( ) / 10 > 4) {
            echo_master(ch, _("%1$^O1 слишком тяжел%1$Gое|ый|ая|ые, чтобы удержаться на твоем хвосте."), obj);
            return;
        }

        wear_tail->wear( obj, F_WEAR_VERBOSE );
        return;
    }

    if (ch == victim && fSlot) {
        if (obj->getWeight( ) / 10 > 5) {
            echo_master(ch, _("%1$^O1 слишком тяжел%1$Gое|ый|ая|ые, чтобы носить %1$Gего|его|ее|их просто так."), obj);
            return;
        }

        wear_personal->wear( obj, F_WEAR_VERBOSE );
        return;
    }

    if (ch == victim) {
        if (wear_obj( ch, obj, F_WEAR_VERBOSE | F_WEAR_REPLACE) == RC_WEAR_NOMATCH)
            echo_master(ch, _("Ты не можешь надеть, вооружиться или держать в руках %O4."), obj);
        return;
    }
    
    if (!oprog_can_dress(obj, ch, victim)) {
        echo_master(ch, _("Ты не сможешь надеть %O4 на %C4."), obj, victim);
        return;
    }

    obj_from_char( obj );
    obj_to_char( obj, victim );
    
    if (wear_obj( victim, obj, 0 ) != RC_WEAR_OK) {
        if (obj->carried_by == victim) {
            obj_from_char( obj );
            obj_to_char( obj, ch );
        }
        oldact(_("Ты пытаешься надеть $o4 на $C4, но безуспешно."), ch, obj, victim, TO_CHAR);
        oldact(_("$c1 пытается надеть на тебя $o4, но не может."), ch, obj, victim, TO_VICT);
        oldact(_("$c1 пытается надеть на $C4 $o4, но не может."), ch, obj, victim, TO_NOTVICT);
        return;
    }
    
    oldact(_("Ты надеваешь $o4 на $C4."), ch, obj, victim, TO_CHAR);
    oldact(_("$c1 надевает на тебя $o4."), ch, obj, victim, TO_VICT);
    oldact(_("$c1 надевает на $C4 $o4."), ch, obj, victim, TO_NOTVICT);
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
        ch->pecho(_("Снять что?"));
        return;
    }

    if (arg_is_from( argFrom )) {
        if (( victim = get_char_room( ch, argVict ) ) == 0) {
            echo_master(ch, _("С кого ты хочешь это снять?"));
            return;
        }
        
        if (victim != ch && !victim->is_npc( )) {
            echo_master(ch, _("%1$^C1 в состоянии раздеться са%1$Gмо|м|ма!"), victim);
            return;
        }
    }
    else
        one_argument( cArg, argObj );
    
    if (arg_is_all( argObj )) {
        Object *obj_next;

        if (victim != ch) {
            echo_master(ch, _("Ты не можешь 'снять всё' с %C2 -- только с себя."), victim);
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
            echo_master(ch, _("У тебя нет этого."));
            return;
        }

        // Prefer something that can actually come off. The tattoo stays the
        // answer when it is the only match, so its refusal is still reachable.
        if (is_tattoo( obj )) {
            Object *other = get_obj_wear_no_tattoo( ch, argObj );
            if (other != 0)
                obj = other;
        }

        obj->wear_loc->remove( obj, F_WEAR_VERBOSE );
        return;
    }
    
    if (( obj = get_obj_wear_victim( victim, argObj, ch ) ) == 0) {
        echo_master(ch, _("У %C2 нет этого."), victim);
        return;
    }

    if (!obj->behavior || !obj->behavior->canDress( ch, victim )) {
        echo_master(ch, _("Ты не сможешь снять %O4 с %C2."), obj, victim);
        return;
    }
    
    if (!obj->wear_loc->remove( obj, 0 )) {
        oldact(_("Ты пытаешься снять $o4 с $C2, но безуспешно."), ch, obj, victim, TO_CHAR);
        oldact(_("$c1 пытается снять с тебя $o4, но не может."), ch, obj, victim, TO_VICT);
        oldact(_("$c1 пытается снять с $C2 $o4, но не может."), ch, obj, victim, TO_NOTVICT);
        return;
    }
    
    oldact(_("Ты снимаешь $o4 с $C2."), ch, obj, victim, TO_CHAR);
    oldact(_("$c1 снимает с тебя $o4."), ch, obj, victim, TO_VICT);
    oldact(_("$c1 снимает с $C2 $o4."), ch, obj, victim, TO_NOTVICT);
    
    if (obj->carried_by == victim) {
        obj_from_char( obj );
        obj_to_char( obj, ch );
    }
}


/*
 * 'slot' command: the personal flavor wearlocation.
 * 'slot'             show the slot: label, contents, how to buy if not owned
 * 'slot name <text>' set the label shown in the equipment list
 * 'slot name'        reset the label back to the default
 */

/** Longest allowed label, in bytes. The equipment list renders labels inside a
 *  21-column field ("<%-21s>"), so anything longer would break the layout. The
 *  internal charset is single-byte (KOI8), so bytes == characters. */
#define SLOT_LABEL_MAX 20

/**
 * The label a player sets here is rendered verbatim inside OTHER players'
 * equipment lists (look, equipment), which makes it an injection surface:
 * a '{' starts a color code that would bleed into the rest of the viewer's
 * screen, control characters reach the viewer's terminal raw, '%' and '$'
 * are format triggers in various output paths, '|' is the Flexer pad
 * separator. Strip all of it, collapse runs of spaces, trim, cap the length.
 */
static DLString slot_label_sanitize( const DLString &input )
{
    DLString result;
    bool prevSpace = true;

    for (DLString::size_type i = 0; i < input.size( ); i++) {
        unsigned char c = input.at( i );

        // Color code: drop the brace and whatever single character follows it.
        if (c == '{') {
            i++;
            continue;
        }

        if (c < ' ')
            continue;

        if (c == '%' || c == '$' || c == '|' || c == '<' || c == '>'
            || c == '}' || c == '\\')
            continue;

        if (c == ' ') {
            if (prevSpace)
                continue;
            prevSpace = true;
        }
        else
            prevSpace = false;

        if (result.size( ) >= SLOT_LABEL_MAX)
            break;

        result += (char)c;
    }

    while (!result.empty( ) && result.at( result.size( ) - 1 ) == ' ')
        result.erase( result.size( ) - 1 );

    return result;
}

CMDRUNP( slot )
{
    DLString args = argument;
    DLString arg = args.getOneArgument( );

    if (ch->is_npc( ))
        return;

    PCharacter *pch = ch->getPC( );

    if (!pch->getWearloc( ).isSet( wear_personal )) {
        ch->pecho(_("У тебя нет личного слота. Его можно купить за квесто-очки у торговца квестовыми вещами ({yквест купить слот{x)."));
        return;
    }

    if (arg.empty( )) {
        XMLAttributeWearslot::Pointer attr =
            pch->getAttributes( ).findAttr<XMLAttributeWearslot>( XMLAttributeWearslot::ATTR_NAME );
        Object *obj = wear_personal->find( ch );

        if (obj != NULL)
            ch->pecho(_("В твоем личном слоте: %1$O1."), obj);
        else
            ch->pecho(_("Твой личный слот пуст."));

        if (attr && !attr->getLabel( ).empty( ))
            ch->pecho(_("В снаряжении он отображается как '{W%1$s{x'."), attr->getLabel( ).c_str( ));

        ch->pecho(_("Задай отображение командой {yслот имя {Dназвание{x, сбрось командой {yслот имя{x."));
        return;
    }

    if (arg_is( arg, "name" )) {
        XMLAttributeWearslot::Pointer attr;

        if (args.empty( )) {
            attr = pch->getAttributes( ).findAttr<XMLAttributeWearslot>( XMLAttributeWearslot::ATTR_NAME );

            if (attr)
                attr->setLabel( DLString::emptyString );

            ch->pecho(_("Отображение личного слота сброшено."));
            return;
        }

        DLString label = slot_label_sanitize( args );

        if (label.empty( )) {
            ch->pecho(_("Из такого названия ничего не вышло, попробуй другое."));
            return;
        }

        attr = pch->getAttributes( ).getAttr<XMLAttributeWearslot>( XMLAttributeWearslot::ATTR_NAME );
        attr->setLabel( label );
        ch->pecho(_("Теперь этот слот отображается в снаряжении как '{W%1$s{x'."), label.c_str( ));
        return;
    }

    ch->pecho(_("Задай отображение командой {yслот имя {Dназвание{x, сбрось командой {yслот имя{x."));
}
