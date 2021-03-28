/* $Id$
 *
 * ruffina, 2004
 */
#include "commandtemplate.h"

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "commonattributes.h"
#include "skill.h"
#include "affecthandler.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"
#include "core/object.h"
#include "liquid.h"
#include "affect.h"

#include "act.h"
#include "loadsave.h"
#include "merc.h"
#include "mercdb.h"
#include "def.h"

bool oprog_smell_liquid(Liquid *liq, Character *ch);

/*---------------------------------------------------------------------------
 * listen
 *--------------------------------------------------------------------------*/
static bool oprog_listen( Object *obj, Character *ch, char *argument )
{
    FENIA_CALL( obj, "Listen", "Cs", ch, argument );
    FENIA_NDX_CALL( obj, "Listen", "OCs", obj, ch, argument );
    return false;
}

CMDRUNP( listen )
{
    char arg[MAX_INPUT_LENGTH];
    Object *obj;

    argument = one_argument( argument, arg );

    if (!arg[0]) {
        ch->pecho("Послушать что?");
        return;
    }

    if ( ( obj = get_obj_wear_carry( ch, arg ) ) 
         || ( obj = get_obj_room( ch, arg ) ))
    {
        if (obj->carried_by == ch) {
            oldact("Ты подносишь к уху $o4 и прислушиваешься.", ch, obj, 0, TO_CHAR);
            oldact("$c1 подносит к уху $o4 и прислушивается.", ch, obj, 0, TO_ROOM);
        }
        else {
            oldact("Ты прикладываешь ухо к $o3 и прислушиваешься.", ch, obj, 0, TO_CHAR);
            oldact("$c1 прикладывает ухо к $o3 и прислушивается.", ch, obj, 0, TO_ROOM);
        }

        if (oprog_listen( obj, ch, argument ))
            return;
        
        if (!obj->pIndexData->sound.empty( )) {
            ch->pecho( obj->pIndexData->sound );
            return;
        }

        if (IS_OBJ_STAT(obj, ITEM_HUM))
            ch->pecho( "Ты чувствуешь слабую вибрацию." );
        else
            ch->pecho("  ... но оттуда не доносится ни звука.");

        return;
    }
    
    /* TODO: listen to a mob */

    oldact("Ты не видишь здесь этого.", ch, 0, 0, TO_CHAR);
}        

/*---------------------------------------------------------------------------
 * smell 
 *--------------------------------------------------------------------------*/
static bool oprog_smell( Object *obj, Character *ch, char *argument )
{
    FENIA_CALL( obj, "Smell", "Cs", ch, argument );
    FENIA_NDX_CALL( obj, "Smell", "OCs", obj, ch, argument );
    return false;
}

static bool rprog_smell( Room *room, Character *ch, char *argument )
{
    FENIA_CALL( room, "Smell", "Cs", ch, argument );
    return false;
}

static bool mprog_smell( Character *victim, Character *ch, char *argument )
{
    FENIA_CALL( victim, "Smell", "Cs", ch, argument );
    FENIA_NDX_CALL( victim->getNPC( ), "Smell", "OCs", victim, ch, argument );
    return false;
}

static bool afprog_smell_affected(const AffectList &affList, SpellTarget::Pointer target, Character *sniffer, char *argument)
{
    bool rc = false;
    
    for (auto &paf: affList.findAllWithHandler()) 
        if (paf->type->getAffect( )->onSmell(target, paf, sniffer))
            rc = true;

    return rc;
}

static bool afprog_smell( Character *victim, Character *sniffer, char *argument )
{
    return afprog_smell_affected(
            victim->affected, SpellTarget::Pointer(NEW, victim), sniffer, argument);
}

static bool afprog_smell( Object *obj, Character *sniffer, char *argument )
{
    return afprog_smell_affected(
            obj->affected, SpellTarget::Pointer(NEW, obj), sniffer, argument);
}

static bool afprog_smell( Room *room, Character *sniffer, char *argument )
{
    return afprog_smell_affected(
            room->affected, SpellTarget::Pointer(NEW, room), sniffer, argument);
}

CMDRUNP( smell )
{
    char arg[MAX_INPUT_LENGTH];
    Object *obj;
    Character *victim;
    XMLStringAttribute::Pointer attr;

    argument = one_argument( argument, arg );

    /*
     * Smell room:
     * - room onSmell trigger, if returns true, overrides index data property
     * - smells from all affects are shown next (onSmellRoom)
     */
    if (!arg[0] || arg_oneof_strict( arg, "room", "комната" )) {
        bool rc = false;

        oldact("Ты нюхаешь воздух.", ch, 0, 0, TO_CHAR);
        oldact("$c1 принюхивается.", ch, 0, 0, TO_ROOM);

        if (rprog_smell( ch->in_room, ch, argument ))
            rc = true;

        if (!rc) {
            Properties::const_iterator p = ch->in_room->pIndexData->properties.find( "smell" );
            if (p != ch->in_room->pIndexData->properties.end( )) {
                ch->pecho(p->second);
                rc = true;
            }
        }

        rc = afprog_smell( ch->in_room, ch, argument ) || rc;

        if (!rc)
            ch->pecho("Вокруг пахнет вполне обычно.");

        return;
    }

    if (arg_oneof_strict( arg, "description", "описание")) {
        if (ch->is_npc( ))
            return;
        
        attr = ch->getPC( )->getAttributes( ).getAttr<XMLStringAttribute>( "smell" );

        if (!argument[0]) {
            if (attr && !attr->getValue( ).empty( ))
                ch->printf("Ты пахнешь:\n\r%s\n\r", attr->getValue( ).c_str( ) ); 
            else
                ch->pecho("Ты ничем особенным не пахнешь.");
            return;
        }

        if (arg_oneof_strict( argument, "clear", "очистить" )) {
            attr->setValue( "" );
            ch->pecho( "Теперь ты никак не пахнешь." );
            return;
        }

        if (arg_is_help( argument )) {
            ch->pecho( "Используй 'smell description <строка>' для установки запаха\n\r"
                         "или 'smell description clear' для его очистки." );
            return;
        }
        
        attr->setValue( argument );
        ch->printf("Теперь ты будешь пахнуть так:\n\r%s\n\r", argument );
        return;
    }

    /*
     * Smell item:
     * - item onSmell trigger, if returns true, overrides index data property
     * - liquid smell is shown next
     * - smells from all affects are shown next (onSmellObj)
     */
    if ( ( obj = get_obj_wear_carry( ch, arg ) ) 
         || ( obj = get_obj_room( ch, arg ) ))
    {
        bool rc = false;

        oldact("Ты нюхаешь $o4.", ch, obj, 0, TO_CHAR);
        oldact("$c1 нюхает $o4.", ch, obj, 0, TO_ROOM);

        if (oprog_smell( obj, ch, argument ))
            rc = true;
        
        if (!rc && !obj->pIndexData->smell.empty( )) {
            ch->pecho( obj->pIndexData->smell );
            rc = true;
        }

        if (obj->item_type == ITEM_DRINK_CON && obj->value1() > 0) {
            Liquid *liq = liquidManager->find(obj->value2());
            if (oprog_smell_liquid(liq, ch))
                rc = true;
        }

        rc = afprog_smell( obj, ch, argument ) || rc;
        
        if (!rc)
            ch->pecho("Пахнет вполне обычно.");

        return;
    }

    /*
     * Smell char:
     * - char onSmell trigger, if returns true, overrides index data property or player's config
     * - smells from all affects are shown next (onSmellChar)
     */
    if (( victim = get_char_room( ch, arg ) )) {
        bool rc = false;

        if (ch == victim) {
            oldact("Ты обнюхиваешь себя.", ch, 0, 0, TO_CHAR);
            oldact("$c1 обнюхивает себя.", ch, 0, 0, TO_ROOM);
        } else {
            oldact("Ты обнюхиваешь $C4.", ch, 0, victim, TO_CHAR);
            oldact("$c1 обнюхивает $C4.", ch, 0, victim, TO_NOTVICT);
            oldact("$c1 обнюхивает тебя.", ch, 0, victim, TO_VICT);
        }

        if (!( rc = mprog_smell( victim, ch, argument ) )) {
            if (victim->is_npc( )) {
                if (!victim->getNPC( )->pIndexData->smell.empty( )) {
                    ch->pecho( victim->getNPC( )->pIndexData->smell );
                    rc = true;
                }
            } 
            else {
                attr = victim->getPC( )->getAttributes( ).findAttr<XMLStringAttribute>( "smell" );
                if (attr && !attr->getValue( ).empty( )) {
                    ch->pecho( attr->getValue( ) );
                    rc = true;
                }
            }
        }


        rc = afprog_smell( victim, ch, argument ) || rc;
        
        if (!rc)
            ch->pecho("Пахнет вполне обычно.");

        return;
    }

    oldact("Ты не видишь здесь этого.", ch, 0, 0, TO_CHAR);
}        


