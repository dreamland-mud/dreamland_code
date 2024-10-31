#include "core/object.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"
#include "roomutils.h"
#include "material.h"
#include "act.h"
#include "loadsave.h"
#include "commandtemplate.h"
#include "damageflags.h"
#include "wrappertarget.h"
#include "wrapperbase.h"
#include "item_progs.h"
#include "../loadsave/behavior_utils.h"
#include "core/behavior/behavior_utils.h"
#include "save.h"
#include "dreamland.h"
#include "dl_strings.h"
#include "merc.h"
#include "def.h"


/*
 *   GET OBJECT [[FROM] CONTAINER[:POCKET]]
 *   GET MOBILE [BY] OBJECT
 */

/*
 * find ':pocket' part of container name
 */
DLString get_pocket_argument( char *arg )
{
    DLString p;
    
    while (*arg++) {
        if (*arg == ':') {
            p = arg + 1;
            *arg = '\0';
            p.colourstrip( );
            return p;
        }
    }

    return "";
}

DLString get_pocket_argument( DLString &arg )
{
    DLString::size_type pos = arg.find(':');
    if (pos != DLString::npos) {
        DLString pocket = arg.size() > pos+1 ? arg.substr(pos+1) : "";
        arg = arg.substr(0, pos);
        return pocket;
    }

    return "";
}

static void get_obj_on_victim( Character *ch, Character *victim, const char *arg )
{
    Object *obj;

    if (( obj = get_obj_wear_victim( victim, arg, ch ) ) == 0) {
        oldact("У $C2 нет ничего похожего на $t.", ch, is_number(arg) ? "это" : arg, victim, TO_CHAR);
        return;
    }
    
    oldact("Ты берешь $C4 за $o4.", ch, obj, victim, TO_CHAR);
    oldact("$c1 берет тебя за $o4.", ch, obj, victim, TO_VICT);
    oldact("$c1 берет $C4 за $o4.", ch, obj, victim, TO_NOTVICT);
    
    FENIA_VOID_CALL( obj, "Seize", "CC", ch, victim );
    FENIA_VOID_CALL( ch, "Seize", "CCO", ch, victim, obj );
    FENIA_VOID_CALL( victim, "Seize", "CCO", ch, victim, obj );

    FENIA_NDX_VOID_CALL( obj, "Seize", "OCC", obj, ch, victim );
    FENIA_NDX_VOID_CALL( ch->getNPC( ), "Seize", "CCCO", ch, ch, victim, obj );
    FENIA_NDX_VOID_CALL( victim->getNPC( ), "Seize", "CCCO", victim, ch, victim, obj );
}


static bool oprog_fetch( Character *ch, Object *obj, Object *container )
{
    if (behavior_trigger(container, "Fetch", "OCO", container, ch, obj))
        return true;

    FENIA_CALL( container, "Fetch", "CO", ch, obj );
    FENIA_NDX_CALL( container, "Fetch", "OCO", container, ch, obj );
    BEHAVIOR_CALL( container, fetch, ch, obj );

    return false;
}

static bool oprog_can_get_corpse_pc( Character *ch, Object *obj )
{
    if (!ch->is_immortal( ) && !obj->hasOwner( ch ))
    {
        oldact("Похоже, $o4 от земли не оторвать.",ch,obj,0,TO_CHAR);
        return false;
    }
    
    return true;
}

static bool oprog_can_get_furniture( Character *ch, Object *obj )
{
    if (Item::countUsers(obj) > 0) {
        oldact("Кто-то использует $o4.",ch,obj,0,TO_CHAR);
        return false;
    }

    return true;
}

static bool oprog_cant_get( Character *ch, Object *obj )
{
    FENIA_CALL( obj, "CantGet", "C", ch );
    FENIA_NDX_CALL( obj, "CantGet", "OC", obj, ch );
    return false;
}

static bool oprog_can_get( Character *ch, Object *obj )
{
    if (oprog_cant_get( ch, obj ))
        return false;

    switch (obj->item_type) {
    case ITEM_CORPSE_PC:
        return oprog_can_get_corpse_pc( ch, obj );
    case ITEM_FURNITURE:
        return oprog_can_get_furniture( ch, obj );
    }

    return true;
}


static bool oprog_cant_fetch( Object *container, Character *ch, Object *obj, const DLString &pocket )
{
    if (behavior_trigger(container, "CantFetch", "OCOs", container, ch, obj, pocket.c_str()))
        return true;

    FENIA_CALL( container, "CantFetch", "COs", ch, obj, pocket.c_str( ) );
    FENIA_NDX_CALL( container, "CantFetch", "OCOs", container, ch, obj, pocket.c_str( ) );
    return false;
}

bool oprog_can_fetch_corpse_pc( Character *ch, Object *container, Object *obj, bool verbose )
{
    if (ch->is_npc( )) {
        if (verbose)
            ch->pecho("Ты не умеешь обшаривать чужие трупы.");
        return false;
    }
    
    if (ch->is_immortal( ))
        return true;
        
    if (container->hasOwner( ch ))
        return true;
        
    if (container->killer.empty())
        return true;

    if (container->killer != ch->getNameC() && container->killer != "!anybody!")
    {
        if (verbose)
            ch->pecho("Это не твоя добыча.");
        return false;
    }
    
    if (container->count == 0) {
        if (verbose)
            ch->pecho("Больше взять ничего не получится.");
        return false;
    }

    // The corpse is someone killed by 'ch', let's check the mark.
    if (obj && obj->getProperty("loot") != "true") {
        if (verbose)
            ch->pecho("Ты не можешь снять %O4 с трупа противника, это не добыча.", obj);
        return false;
    }

    return true;
}


static bool oprog_can_fetch( Character *ch, Object *container, Object *obj, const DLString &pocket )
{
    if (oprog_cant_fetch( container, ch, obj, pocket ))
        return false;

    switch (container->item_type) {
    case ITEM_CORPSE_PC:
        return oprog_can_fetch_corpse_pc( ch, container, obj, true );
        
    case ITEM_CONTAINER:
        if (!pocket.empty( ) && !IS_SET(container->value1(), CONT_WITH_POCKETS)) {
            oldact("Тебе не удалось нашарить ни одного кармана у $o2.",ch,container,0,TO_CHAR);
            return false;
        }
        
        if (IS_SET( container->value1(), CONT_CLOSED )) {
            ch->pecho("%1$^O4 нужно сперва открыть.", container );
            return false;
        }

        return true;

    case ITEM_KEYRING:
    case ITEM_CORPSE_NPC:
        return true;

    default:
        ch->pecho("%1$^O1 не контейнер, ты не можешь ничего оттуда взять.", container );
        return false;
    }
}

#define GET_OBJ_STOP     -1
#define GET_OBJ_ERR       0
#define GET_OBJ_OK        1

static int can_get_obj( Character *ch, Object *obj )
{
    if (!oprog_can_get( ch, obj ))
        return GET_OBJ_ERR;

    if ( (!obj->can_wear( ITEM_TAKE )) && (!ch->is_immortal()) )
    {
        ch->pecho("Ты не можешь взять %1$O4.", obj );
        return GET_OBJ_ERR;
    }

    if (obj->pIndexData->limit != -1)
    {
        if (obj->isAntiAligned( ch )) {
            if (ch->is_immortal()) 
                ch->pecho("Осторожно, ты не смог%1$Gло||ла бы владеть этой вещью, будучи смертн%1$Gым|ым|ой.", ch);
            else {
                ch->pecho("%2$^s не позволят тебе владеть %1$O5.",
                          obj,
                          IS_NEUTRAL(ch) ? "силы равновесия" : IS_GOOD(ch) ? "священные силы" : "твои демоны");
                
                ch->recho("%1$^C1 обжигается о %2$O4.", ch, obj );
                return GET_OBJ_ERR;
            }
        }
    }

    if (ch->carry_number + obj->getNumber( ) > Char::canCarryNumber(ch))
    {
        if (ch->is_immortal())
            ch->pecho("Осторожно, ты уже несешь слишком много вещей.");
        else {
            ch->pecho("Ты не можешь унести больше %d вещей и поэтому не сможешь поднять %O4.", Char::canCarryNumber(ch), obj);
            return GET_OBJ_STOP;
        }
    }

    if (Char::getCarryWeight(ch) + obj->getWeight( ) > Char::canCarryWeight(ch))
    {
        if (ch->is_immortal())
            ch->pecho("Осторожно, ты не смог%1$Gло||ла бы поднять такую тяжесть, будучи смертн%1$Gым|ым|ой.", ch);
        else {
            ch->pecho("Ты не можешь нести вес больше %d фунтов и поэтому не сможешь поднять %O4.", Char::canCarryWeight(ch), obj);
            return GET_OBJ_STOP;
        }
    }

    return GET_OBJ_OK;
}

static bool get_obj( Character *ch, Object *obj )
{
    oldact("Ты берешь $o4.", ch, obj, 0, TO_CHAR);
    oldact("$c1 берет $o4.", ch, obj, 0, TO_ROOM);
            
    obj_from_room( obj );
    obj_to_char( obj, ch );

    if (oprog_get( obj, ch ))
        return true;

    return false;
}

static bool get_obj_container( Character *ch, Object *obj, Object *container )
{
    ostringstream toChar, toRoom;
    DLString prep;
    
    switch (container->item_type) {
    case ITEM_KEYRING:
        toChar << "Ты снимаешь $o4 с $O2.";
        toRoom << "$c1 снимает $o4 с $O2.";
        break;

    case ITEM_CONTAINER:
        if (IS_SET(container->value1(), CONT_PUT_ON)) 
            prep = "со";
        else if (IS_SET(container->value1(), CONT_PUT_ON2)) 
            prep = "с";
        else
            prep = "из";
        
        toChar << "Ты берешь $o4 " << prep << " $O2.";
        toRoom << "$c1 берет $o4 " << prep << " $O2.";
        break;

    case ITEM_CORPSE_NPC:
    case ITEM_CORPSE_PC:
        toChar << "Ты снимаешь $o4 с $O2.";
        toRoom << "$c1 снимает $o4 с $O2.";
        break;

    default:
        toChar << "Ты берешь $o4 из $O2.";
        toRoom << "$c1 берет $o4 из $O2.";
        break;
    }

    oldact( toChar.str( ).c_str( ), ch, obj, container, TO_CHAR );
    oldact( toRoom.str( ).c_str( ), ch, obj, container, TO_ROOM );

    obj_from_obj( obj );
    obj_to_char( obj, ch );

    // Decrease looting counter before any other 'get' progs have a chance to run.
    if (container->item_type == ITEM_CORPSE_PC && !ch->is_immortal( ) && !container->hasOwner( ch ))
        container->count--;

    if (oprog_get( obj, ch ))
        return true;
        
    if (oprog_fetch( ch, obj, container ))
        return true;
    
    return false;
}

/*
 *
 *  get all
 *  get <name>
 *  get all.<name>
 *  get all.'<names list>'
 *
 *  get <name> [from] <container>[:<pocket>]
 *  get all [from] <container>[:<pocket>]
 *  get all.<name> [from] <container>[:<pocket>]
 *  get all.'<names list>' [from] <container>[:<pocket>]
 *
 *  get <victim> by <name>
 *
 */
CMDRUNP( get )
{
    Object *obj;
    Object *obj_next;
    Object *container;
    bool found;
    DLString origArguments = argument;
    DLString arguments = argument;
    DLString argAllObj, argTarget, argContainer;
    bool all, allDot;

    argAllObj = arguments.getOneArgument();
    if (arg_is_all( argAllObj )) {
        argTarget = "";
        all = true;
        allDot = false;
    } else if (arg_is_alldot( argAllObj )) {
        arguments = origArguments.substr(4);
        argTarget = arguments.getOneArgument();
        all = false;
        allDot = true;
    } else {
        argTarget = argAllObj;
        all = false;
        allDot = false;
    }

    argContainer = arguments.getOneArgument();
    if (arg_is_from( argContainer ) || arg_is_strict(argContainer, "by"))
        argContainer = arguments.getOneArgument( );


    if (argAllObj.empty( ))
    {
        ch->pecho("Взять что?");
        return;
    }

    if(argContainer.empty( ))
    {
        DLString that = is_number(argTarget.c_str( )) ? "этого" : argTarget;

        if (!all && !allDot)
        {
            /*  get <name> */
            obj = get_obj_list( ch, argTarget.c_str( ), ch->in_room->contents );
            
            if (!obj) {
                oldact("Ты не видишь здесь $T.", ch, 0, that.c_str( ), TO_CHAR);

            } else {
                if (can_get_obj( ch, obj ) == GET_OBJ_OK)
                    get_obj( ch, obj );
            }
        }
        else
        {
            /*
             *  get all
             *  get all.<name>
             *  get all.'<names list>'
             */
            found = false;

            dreamland->removeOption( DL_SAVE_OBJS );

            for ( obj = ch->in_room->contents; obj; obj = obj_next )
            {
                obj_next = obj->next_content;
                if ( (all || obj_has_name( obj, argTarget, ch ))
                        && ch->can_see( obj ) )
                {
                    found = true;
                    
                    int rc = can_get_obj( ch, obj );
                    if (rc == GET_OBJ_ERR)
                        continue;
                    if (rc == GET_OBJ_STOP)
                        break;

                    get_obj( ch, obj );
                }
            }

            dreamland->resetOption( DL_SAVE_OBJS );

            if ( !found )
            {
                if (all)
                    ch->pecho("Ты ничего не видишь здесь.");
                else if (allDot)
                    ch->pecho("Ты не видишь ничего подобного здесь.");
                else
                    oldact("Ты не видишь здесь $T.", ch, 0, that.c_str( ), TO_CHAR);
            }
            else
                save_items( ch->in_room );
        }
    }
    else
    {
        DLString pocket;
        DLString that = is_number(argContainer.c_str( )) ? "этого" : argContainer;

        /*
         *  get <name> [from] <container>[:<pocket>]
         *  get all [from] <container>[:<pocket>]
         *  get all.<name> [from] <container>[:<pocket>]
         *  get all.'<names list>' [from] <container>[:<pocket>]
         *  or if container not found:
         *  get <victim> by <name>
         */

        // Disallow 'get <name> all.<container>' syntax.
        if (arg_is_alldot( argContainer ))
        {
            ch->pecho("Ты не можешь сделать этого.");
            return;
        }

        // Split out potential pocket argument from <container>:<pocket>.
        pocket = get_pocket_argument( argContainer );

        // Container not found, assume 'get <victim> by <name>' syntax.
        if ( !( container = get_obj_here( ch, argContainer ) ) )
        {
            Character *victim = get_char_room( ch, argTarget );
            
            if (victim)
                get_obj_on_victim( ch, victim, argContainer.c_str( ) );
            else
                oldact("Ты не видишь здесь $T.", ch, 0, that.c_str( ), TO_CHAR);
            return;
        }

        if (!oprog_can_fetch( ch, container, NULL, pocket ))
            return;

        if (!all && !allDot)
        {
            /*  get <name> [from] <container>[:<pocket>] */
            obj = get_obj_list( ch, argTarget.c_str( ), container->contains, pocket );

            if(!obj) {
                oldact("Ты не видишь ничего подобного в $o6.", ch, container, 0, TO_CHAR);
                return;
            }
            
            if (!oprog_can_fetch( ch, container, obj, pocket ))
                return;

            if (can_get_obj( ch, obj ) == GET_OBJ_OK)
                get_obj_container( ch, obj, container );
        }
        else {
            /*
             *  get all [from] <container>[:<pocket>]
             *  get all.<name> [from] <container>[:<pocket>]
             *  get all.'<names list>' [from] <container>[:<pocket>]
             */

            if (IS_PIT(container) && !ch->is_immortal() )
            {
                ch->pecho("Не жадничай, пожертвования могут понадобиться кому-то еще.");
                ch->pecho("И, кстати, не забудь, что продать вещи из ямы для пожертвований все равно не получится.");             
                return;
            }
                
            found = false;

            for ( obj = container->contains; obj; obj = obj_next )
            {
                obj_next = obj->next_content;

                if (!all && !obj_has_name( obj, argTarget, ch ))
                    continue;

                if (!ch->can_see( obj ))
                    continue;

                if (!pocket.empty( ) && obj->pocket != pocket)
                    continue;

                if (pocket.empty( ) && !obj->pocket.empty( ))
                    continue;
                    
                found = true;

                if (!oprog_can_fetch( ch, container, obj, pocket ))
                    continue;
                
                int rc = can_get_obj( ch, obj );
                if (rc == GET_OBJ_STOP)
                    return;
                if (rc == GET_OBJ_ERR)
                    continue;

                get_obj_container( ch, obj, container );
            }

            if (!found) {
                if (!all)
                    oldact("Ты не видишь ничего в $o6.", ch, container, 0, TO_CHAR);
                else
                    oldact("Ты не видишь ничего подобного в $o6.", ch, container, 0, TO_CHAR);
            }
        }
    }
}

