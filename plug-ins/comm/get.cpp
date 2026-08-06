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
#include "l10n.h"

WEARLOC(none);

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
        oldact(_("У $C2 нет ничего похожего на $t."), ch, is_number(arg) ? "это" : arg, victim, TO_CHAR);
        return;
    }
    
    oldact(_("Ты берешь $C4 за $o4."), ch, obj, victim, TO_CHAR);
    oldact(_("$c1 берет тебя за $o4."), ch, obj, victim, TO_VICT);
    oldact(_("$c1 берет $C4 за $o4."), ch, obj, victim, TO_NOTVICT);
    
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
        oldact(_("Похоже, $o4 от земли не оторвать."),ch,obj,0,TO_CHAR);
        return false;
    }
    
    return true;
}

static bool oprog_can_get_furniture( Character *ch, Object *obj )
{
    if (Item::countUsers(obj) > 0) {
        oldact(_("Кто-то использует $o4."),ch,obj,0,TO_CHAR);
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
            ch->pecho(_("Ты не умеешь обшаривать чужие трупы."));
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
            ch->pecho(_("Это не твоя добыча."));
        return false;
    }
    
    if (container->count == 0) {
        if (verbose)
            ch->pecho(_("Больше взять ничего не получится."));
        return false;
    }

    // The corpse is someone killed by 'ch', let's check the mark.
    if (obj && obj->getProperty("loot") != "true") {
        if (verbose)
            ch->pecho(_("Ты не можешь снять %O4 с трупа противника, это не добыча."), obj);
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
            oldact(_("Тебе не удалось нашарить ни одного кармана у $o2."),ch,container,0,TO_CHAR);
            return false;
        }
        
        if (IS_SET( container->value1(), CONT_CLOSED )) {
            ch->pecho(_("%1$^O4 нужно сперва открыть."), container );
            return false;
        }

        return true;

    case ITEM_KEYRING:
    case ITEM_CORPSE_NPC:
        return true;

    default:
        ch->pecho(_("%1$^O1 не контейнер, ты не можешь ничего оттуда взять."), container );
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
        ch->pecho(_("Ты не можешь взять %1$O4."), obj );
        return GET_OBJ_ERR;
    }

    if (obj->pIndexData->limit != -1)
    {
        if (obj->isAntiAligned( ch )) {
            if (ch->is_immortal()) 
                ch->pecho(_("Осторожно, ты не смог%1$Gло||ла бы владеть этой вещью, будучи смертн%1$Gым|ым|ой."), ch);
            else {
                ch->pecho(_("%2$^s не позволят тебе владеть %1$O5."),
                          obj,
                          IS_NEUTRAL(ch) ? "силы равновесия" : IS_GOOD(ch) ? "священные силы" : "твои демоны");
                
                ch->recho(_("%1$^C1 обжигается о %2$O4."), ch, obj );
                return GET_OBJ_ERR;
            }
        }
    }

    if (ch->carry_number + obj->getNumber( ) > Char::canCarryNumber(ch))
    {
        if (ch->is_immortal())
            ch->pecho(_("Осторожно, ты уже несешь слишком много вещей."));
        else {
            ch->pecho(_("Ты не можешь унести больше %d вещей и поэтому не сможешь поднять %O4."), Char::canCarryNumber(ch), obj);
            return GET_OBJ_STOP;
        }
    }

    if (Char::getCarryWeight(ch) + obj->getWeight( ) > Char::canCarryWeight(ch))
    {
        if (ch->is_immortal())
            ch->pecho(_("Осторожно, ты не смог%1$Gло||ла бы поднять такую тяжесть, будучи смертн%1$Gым|ым|ой."), ch);
        else {
            ch->pecho(_("Ты не можешь нести вес больше %d фунтов и поэтому не сможешь поднять %O4."), Char::canCarryWeight(ch), obj);
            return GET_OBJ_STOP;
        }
    }

    return GET_OBJ_OK;
}

static bool get_obj( Character *ch, Object *obj )
{
    oldact(_("Ты берешь $o4."), ch, obj, 0, TO_CHAR);
    oldact(_("$c1 берет $o4."), ch, obj, 0, TO_ROOM);
            
    obj_from_room( obj );
    obj_to_char( obj, ch );

    if (oprog_get( obj, ch ))
        return true;

    return false;
}

static bool get_obj_container( Character *ch, Object *obj, Object *container )
{
    // Whole per-preposition sentences (not a spliced RU prep) so each renders
    // per viewer through oldact(MultiMessage).
    MultiMessage mChar, mRoom;

    switch (container->item_type) {
    case ITEM_KEYRING:
        mChar = _("Ты снимаешь $o4 с $O2.");
        mRoom = _("$c1 снимает $o4 с $O2.");
        break;

    case ITEM_CONTAINER:
        if (IS_SET(container->value1(), CONT_PUT_ON)) {
            mChar = _("Ты берешь $o4 со $O2.");
            mRoom = _("$c1 берет $o4 со $O2.");
        }
        else if (IS_SET(container->value1(), CONT_PUT_ON2)) {
            mChar = _("Ты берешь $o4 с $O2.");
            mRoom = _("$c1 берет $o4 с $O2.");
        }
        else {
            mChar = _("Ты берешь $o4 из $O2.");
            mRoom = _("$c1 берет $o4 из $O2.");
        }
        break;

    case ITEM_CORPSE_NPC:
    case ITEM_CORPSE_PC:
        mChar = _("Ты снимаешь $o4 с $O2.");
        mRoom = _("$c1 снимает $o4 с $O2.");
        break;

    default:
        mChar = _("Ты берешь $o4 из $O2.");
        mRoom = _("$c1 берет $o4 из $O2.");
        break;
    }

    oldact( mChar, ch, obj, container, TO_CHAR );
    oldact( mRoom, ch, obj, container, TO_ROOM );

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
 * Taking something can kill the taker: an incandescent item burns whoever picks
 * it up, and death moves them to their altar. Once that happens the bulk loops
 * must stop -- otherwise the character carries on emptying a room or a corpse
 * they are no longer standing next to.
 */
static bool still_looting( Character *ch, Room *room )
{
    return !ch->isDead( ) && ch->in_room == room;
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
static bool is_container_like( Object *obj )
{
    return obj->item_type == ITEM_CONTAINER
           || obj->item_type == ITEM_CORPSE_NPC
           || obj->item_type == ITEM_CORPSE_PC;
}

/*
 * Find a container by name, searching inventory, then equipment, then the room
 * -- the same order and visibility rules as get_obj_here, but skipping
 * everything that cannot hold anything.
 *
 * 'взя все сундук' has to mean the chest, not the "ключ от сундучка" lying in
 * the backpack: get_obj_here returns the first name match in any of those
 * scopes, and a key whose own name mentions the chest wins over the chest.
 * Making get_obj_here itself container-preferring would move targeting for
 * every spell and skill that shares it, so the preference lives here.
 *
 * Deliberately limited to a plain name. '2.сундук' and id-targeting must keep
 * counting exactly what the old scan counted, otherwise they would silently
 * resolve to a different object than before.
 */
static Object * get_container_here( Character *ch, const DLString &argument )
{
    Object *obj;

    if (argument.find( '.' ) != DLString::npos)
        return 0;

    if (get_arg_id( argument ) != 0)
        return 0;

    for (obj = ch->carrying; obj != 0; obj = obj->next_content)
        if (obj->wear_loc == wear_none
            && (ch->can_see( obj ) || ch->can_hear( obj ))
            && is_container_like( obj )
            && obj_has_name( obj, argument, ch ))
            return obj;

    for (obj = ch->carrying; obj != 0; obj = obj->next_content)
        if (obj->wear_loc != wear_none
            && is_container_like( obj )
            && obj_has_name( obj, argument, ch ))
            return obj;

    for (obj = ch->in_room->contents; obj != 0; obj = obj->next_content)
        if ((ch->can_see( obj ) || ch->can_hear( obj ))
            && is_container_like( obj )
            && obj_has_name( obj, argument, ch ))
            return obj;

    return 0;
}

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
        ch->pecho(_("Взять что?"));
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
                oldact(_("Ты не видишь здесь $T."), ch, 0, that.c_str( ), TO_CHAR);

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
            Room *startRoom = ch->in_room;

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

                    if (!still_looting( ch, startRoom ))
                        break;
                }
            }

            dreamland->resetOption( DL_SAVE_OBJS );

            if ( !found )
            {
                if (all)
                    ch->pecho(_("Ты ничего не видишь здесь."));
                else if (allDot)
                    ch->pecho(_("Ты не видишь ничего подобного здесь."));
                else
                    oldact(_("Ты не видишь здесь $T."), ch, 0, that.c_str( ), TO_CHAR);
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
            ch->pecho(_("Ты не можешь сделать этого."));
            return;
        }

        // Split out potential pocket argument from <container>:<pocket>.
        pocket = get_pocket_argument( argContainer );

        // Prefer a real container over anything merely named after one.
        if ( !( container = get_container_here( ch, argContainer ) ) )
            container = get_obj_here( ch, argContainer );

        // Container not found, assume 'get <victim> by <name>' syntax.
        if ( !container )
        {
            Character *victim = get_char_room( ch, argTarget );
            
            if (victim)
                get_obj_on_victim( ch, victim, argContainer.c_str( ) );
            else
                oldact(_("Ты не видишь здесь $T."), ch, 0, that.c_str( ), TO_CHAR);
            return;
        }

        if (!oprog_can_fetch( ch, container, NULL, pocket ))
            return;

        if (!all && !allDot)
        {
            /*  get <name> [from] <container>[:<pocket>] */
            obj = get_obj_list( ch, argTarget.c_str( ), container->contains, pocket );

            if(!obj) {
                oldact(_("Ты не видишь ничего подобного в $o6."), ch, container, 0, TO_CHAR);
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
                ch->pecho(_("Не жадничай, пожертвования могут понадобиться кому-то еще."));
                ch->pecho(_("И, кстати, не забудь, что продать вещи из ямы для пожертвований все равно не получится."));             
                return;
            }
                
            found = false;
            Room *startRoom = ch->in_room;

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

                if (!still_looting( ch, startRoom ))
                    break;
            }

            if (!found) {
                if (!all)
                    oldact(_("Ты не видишь ничего в $o6."), ch, container, 0, TO_CHAR);
                else
                    oldact(_("Ты не видишь ничего подобного в $o6."), ch, container, 0, TO_CHAR);
            }
        }
    }
}

