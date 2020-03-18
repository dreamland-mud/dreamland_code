/* $Id$
 *
 * ruffina, 2004
 */
#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "commandtemplate.h"
#include "objectbehavior.h"
#include "object.h"
#include "affect.h"
#include "room.h"
#include "pcharacter.h"
#include "character.h"

#include "save.h"
#include "act.h"
#include "act_move.h"
#include "act_lock.h"
#include "gsn_plugin.h"
#include "loadsave.h"
#include "merc.h"
#include "mercdb.h"
#include "vnum.h"
#include "def.h"

#define OBJ_VNUM_CORK 19 

GSN(golden_eye);
Object * get_obj_list_vnum( Character *ch, int vnum, Object *list );

static Object * get_key_carry( Character *ch, int vnum )
{
    Object *key, *ring;
    
    if (( key = get_obj_carry_vnum( ch, vnum ) ))
        return key;

    for (ring = get_obj_carry_type( ch, ITEM_KEYRING );
         ring;
         ring = get_obj_list_type( ch, ITEM_KEYRING, ring->next_content ))
    {
        if (( key = get_obj_list_vnum( ch, vnum, ring->contains ) ))
            return key;
    }

    return NULL;
}

/*--------------------------------------------------------------------
 *    open 
 *-------------------------------------------------------------------*/
static bool oprog_cant_open( Object *obj, Character *ch )
{
    FENIA_CALL( obj, "CantOpen", "C", ch );
    FENIA_NDX_CALL( obj, "CantOpen", "OC", obj, ch );
    return false;
}

static bool oprog_open(Object *obj, Character *ch)
{
    FENIA_CALL( obj, "Open", "C", ch );
    FENIA_NDX_CALL( obj, "Open", "OC", obj, ch );
    return false;
}

static bool oprog_open_msg(Object *obj, Character *ch)
{
    FENIA_CALL( obj, "OpenMsg", "C", ch );
    FENIA_NDX_CALL( obj, "OpenMsg", "OC", obj, ch );
    return false;
}

void open_door_extra ( Character *ch, int door, void *pexit )
{
    EXIT_DATA *pexit_rev = 0;
    int exit_info;
    bool eexit = door == DIR_SOMEWHERE;
    Room *room = ch->in_room;

    if ( !pexit )
            return;

    exit_info = eexit?
                    ((EXTRA_EXIT_DATA *) pexit)->exit_info
            : ((EXIT_DATA *) pexit)->exit_info;

    if ( !IS_SET(exit_info, EX_CLOSED) )
    {
            ch->println( "Здесь уже открыто." );
            return;
    }

    if ( IS_SET(exit_info, EX_LOCKED) )
    {
            ch->println( "Здесь заперто." );
            return;
    }

    REMOVE_BIT( eexit?
                    ((EXTRA_EXIT_DATA *) pexit)->exit_info
            : ((EXIT_DATA *) pexit)->exit_info, EX_CLOSED);

    if ( eexit ) {
        act( "$c1 открывает $n4.", ch, ((EXTRA_EXIT_DATA *) pexit)->short_desc_from, 0, TO_ROOM );
        act( "Ты открываешь $n4.", ch, ((EXTRA_EXIT_DATA *) pexit)->short_desc_from, 0, TO_CHAR );
    }
    else {
        const char *doorname = direction_doorname((EXIT_DATA *) pexit);
        act( "$c1 открывает $N4.", ch, 0, doorname, TO_ROOM );
        act( "Ты открываешь $N4.", ch, 0, doorname, TO_CHAR );
    }


    /* open the other side */
    if (!eexit && (pexit_rev = direction_reverse(room, door)))
    {
            REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );
            direction_target(room, door)->echo(POS_RESTING, "%^N1 открывается.", direction_doorname(pexit_rev));
    }
}

void open_door ( Character *ch, int door )
{
    if ( door < 0 || door > 5 )
            return;

    open_door_extra( ch, door, ch->in_room->exit[door] );
}

bool open_portal( Character *ch, Object *obj )
{
    if ( !IS_SET(obj->value[1], EX_ISDOOR) )
    {
            ch->println( "Ты не можешь сделать этого." );
            return false;
    }

    if ( !IS_SET(obj->value[1], EX_CLOSED) )
    {
            ch->println( "Это уже открыто." );
            return false;
    }

    if ( IS_SET(obj->value[1], EX_LOCKED) )
    {
            ch->println( "Здесь заперто." );
            return false;
    }

    REMOVE_BIT(obj->value[1], EX_CLOSED);
    act_p("Ты открываешь $o4.",ch,obj,0,TO_CHAR,POS_RESTING);
    act_p("$c1 открывает $o4.",ch,obj,0,TO_ROOM,POS_RESTING);

    return true;
}

bool open_drink_container( Character *ch, Object *obj )
{
    if (!IS_SET(obj->value[3], DRINK_CLOSED)) {
        ch->pecho( "%1$^O1 и так не запер%1$Gто|т|та.", obj );
        return false;
    }
    
    if (IS_SET(obj->value[3], DRINK_LOCKED)) {
        if (IS_SET(obj->value[3], DRINK_CLOSE_CORK))
            ch->pecho( "%1$^O1 плотно закупоре%1$Gно|н|на пробкой, поищи штопор.", obj );
        else if (IS_SET(obj->value[3], DRINK_CLOSE_NAIL))
            ch->pecho( "%1$^O1 закры%1$Gто|т|та крышкой и заколоче%1$Gн|но|на.", obj );
        else if (IS_SET(obj->value[3], DRINK_CLOSE_KEY))
            ch->pecho( "%1$^O1 крепко запер%1$Gто|т|та.", obj );
        else
            ch->pecho( "%1$^O1 запер%1$Gто|т|та.", obj );

        return false;
    }
    
    REMOVE_BIT(obj->value[3], DRINK_CLOSED);

    if (IS_SET(obj->value[3], DRINK_CLOSE_CORK)) {
        Object *cork;

        cork = create_object( get_obj_index( OBJ_VNUM_CORK ), 0 );
        obj_to_char( cork, ch );

        act( "Ты вынимаешь пробку из $O2.", ch, 0, obj, TO_CHAR );
        act( "$c1 вынимает пробку из $O2.", ch, 0, obj, TO_ROOM );
    }
    else if (IS_SET(obj->value[3], DRINK_CLOSE_NAIL)) {
        act( "Ты открываешь крышку $O2.", ch, 0, obj, TO_CHAR );
        act( "$c1 открывает крышку $O2.", ch, 0, obj, TO_ROOM );
    }
    else {
        act( "Ты открываешь $O4.", ch, 0, obj, TO_CHAR );
        act( "$c1 открывает $O4.", ch, 0, obj, TO_ROOM );
    }

    return true;
}

bool open_container( Character *ch, Object *obj )
{
    if ( !IS_SET(obj->value[1], CONT_CLOSED) )
    {
            ch->println( "Это уже открыто." );
            return false;
    }

    if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) )
    {
            ch->println( "Ты не можешь сделать этого." );
            return false;
    }

    if ( IS_SET(obj->value[1], CONT_LOCKED) )
    {
            ch->println( "Здесь заперто." );
            return false;
    }
    
    if (oprog_cant_open( obj, ch ))
        return false;

    REMOVE_BIT(obj->value[1], CONT_CLOSED);

    if (!oprog_open_msg( obj, ch )) {
        act_p("Ты открываешь $o4.",ch,obj,0,TO_CHAR,POS_RESTING);
        act_p("$c1 открывает $o4.", ch, obj, 0, TO_ROOM,POS_RESTING );
    }

    oprog_open( obj, ch );
    return true;
}



CMDRUNP( open )
{
    char arg[MAX_INPUT_LENGTH];
    Object *obj;
    EXTRA_EXIT_DATA *peexit;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        ch->println( "Открыть что?" );
        return;
    }

    if (( door = find_exit( ch, arg, FEX_NO_INVIS|FEX_DOOR|FEX_NO_EMPTY ) ) >= 0)
    {
        open_door( ch, door );
        return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != 0 )
    {
        bool changed = false;
        
        switch (obj->item_type) {
        case ITEM_PORTAL:
            changed = open_portal( ch, obj );
            break;
            
        case ITEM_DRINK_CON:
            changed = open_drink_container( ch, obj );
            break;
            
        case ITEM_CONTAINER:
            changed = open_container( ch, obj );
            break;

        default:
            ch->pecho( "%^O4 невозможно открыть.", obj );
            return;
        }
        
        if ( obj->in_room != 0 && changed )
            save_items( obj->in_room );
            
        return;
    }

    if ( ( ( peexit = get_extra_exit( arg, ch->in_room->extra_exit ) ) != 0 )
            && ch->can_see( peexit ) )
    {
        open_door_extra( ch, DIR_SOMEWHERE, (void *)peexit );
        return;
    }        

    find_exit( ch, arg, FEX_NO_INVIS|FEX_DOOR|FEX_NO_EMPTY|FEX_VERBOSE );
}


/*--------------------------------------------------------------------
 *    close 
 *-------------------------------------------------------------------*/
static bool oprog_close( Object *obj, Character *ch )
{
    FENIA_CALL( obj, "Close", "C", ch );
    FENIA_NDX_CALL( obj, "Close", "OC", obj, ch );
    return false;
}

static void close_door( Character *ch, int door )
{
    // 'close door'
    EXIT_DATA *pexit;
    EXIT_DATA *pexit_rev = 0;
    Room *room = ch->in_room;

    pexit        = ch->in_room->exit[door];
    if ( IS_SET(pexit->exit_info, EX_CLOSED) )
    {
            ch->println( "Здесь уже закрыто." );
            return;
    }

    SET_BIT(pexit->exit_info, EX_CLOSED);
    
    const char *doorname = direction_doorname(pexit);
    act( "$c1 закрывает $N4.", ch, 0, doorname, TO_ROOM );
    act( "Ты закрываешь $N4.", ch, 0, doorname, TO_CHAR );

    // close the other side
    if ((pexit_rev = direction_reverse(room, door)))
    {
            SET_BIT( pexit_rev->exit_info, EX_CLOSED );
            direction_target(room, door)->echo(POS_RESTING, "%^N1 закрывается.", direction_doorname(pexit_rev));
    }
}

CMDRUNP( close )
{
    char arg[MAX_INPUT_LENGTH];
    Object *obj;
    EXTRA_EXIT_DATA *peexit;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        ch->println( "Закрыть что?" );
        return;
    }

    if (( door = find_exit( ch, arg, FEX_NO_INVIS|FEX_DOOR|FEX_NO_EMPTY ) ) >= 0)
    {
        close_door( ch, door );
        return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != 0 )
    {
        if ( obj->item_type == ITEM_PORTAL )
        {
            // portal stuff
            if ( !IS_SET(obj->value[1],EX_ISDOOR)
                    || IS_SET(obj->value[1],EX_NOCLOSE) )
            {
                ch->println( "Ты не можешь сделать этого." );
                return;
            }

            if ( IS_SET(obj->value[1],EX_CLOSED) )
            {
                ch->println( "Здесь уже закрыто." );
                return;
            }

            SET_BIT(obj->value[1],EX_CLOSED);
            act_p("Ты закрываешь $o4.",ch,obj,0,TO_CHAR,POS_RESTING);
            act_p("$c1 закрывает $o4.",ch,obj,0,TO_ROOM,POS_RESTING);
        }
        else if ( obj->item_type == ITEM_CONTAINER )
        {
            // 'close object'
            if ( IS_SET(obj->value[1], CONT_CLOSED) )
            {
                ch->println( "Здесь уже закрыто." );
                return;
            }

            if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) )
            {
                ch->println( "Ты не можешь сделать этого." );
                return;
            }

            SET_BIT(obj->value[1], CONT_CLOSED);
            act_p("Ты закрываешь $o4.",ch,obj,0,TO_CHAR,POS_RESTING);
            act_p( "$c1 закрывает $o4.", ch, obj, 0, TO_ROOM,POS_RESTING );
            oprog_close( obj, ch );
        }
        else if (obj->item_type == ITEM_DRINK_CON) {
            // cork a bottle 
            
            if (!IS_SET(obj->value[3], DRINK_CLOSE_CORK|DRINK_CLOSE_NAIL|DRINK_CLOSE_KEY)) {
                act( "$O4 невозможно закрыть или закупорить.", ch, 0, obj, TO_CHAR );
                return;
            }

            if (IS_SET(obj->value[3], DRINK_CLOSED)) {
                act( "$O4 уже закрыли.", ch, 0, obj, TO_CHAR );
                return;
            }
            
            if (IS_SET(obj->value[3], DRINK_CLOSE_CORK)) {
                Object *cork = get_obj_carry_vnum( ch, OBJ_VNUM_CORK );

                if (!cork) {
                    act( "У тебя нет пробки от $O2.", ch, 0, obj, TO_CHAR );
                    act( "$c1 шарит по карманам в поисках пробки.", ch, 0, obj, TO_ROOM );
                    return;
                }

                extract_obj( cork );
                act( "Ты закупориваешь $O4 пробкой.", ch, 0, obj, TO_CHAR );
                act( "$c1 закупоривает $O4 пробкой.", ch, 0, obj, TO_ROOM );
            }
            else if (IS_SET(obj->value[3], DRINK_CLOSE_NAIL)) {
                act( "Ты закрываешь $O4 крышкой.", ch, 0, obj, TO_CHAR );
                act( "$c1 закрывает $O4 крышкой.", ch, 0, obj, TO_ROOM );
            }
            else {
                act( "Ты закрываешь $O4.", ch, 0, obj, TO_CHAR );
                act( "$c1 закрывает $O4.", ch, 0, obj, TO_ROOM );
            }
            
            SET_BIT(obj->value[3], DRINK_CLOSED);
        }
        else {
            ch->println( "Это не контейнер." );
            return;
        }

        if ( obj->in_room != 0 )
                save_items( obj->in_room );

        return;
    }

    if ( ( ( peexit = get_extra_exit( arg, ch->in_room->extra_exit ) ) != 0 )
            && ch->can_see( peexit ) )
    {
        if ( !IS_SET(peexit->exit_info, EX_ISDOOR) )
        {
                ch->println( "Это не дверь!" );
                return;
        }

        if ( IS_SET(peexit->exit_info, EX_CLOSED) )
        {
                ch->println( "Здесь уже закрыто." );
                return;
        }

        SET_BIT(peexit->exit_info, EX_CLOSED);
        act( "$c1 закрывает $N4.", ch, 0, peexit->short_desc_from, TO_ROOM);
        act( "Ты закрываешь $N4.", ch, 0, peexit->short_desc_from, TO_CHAR);

        return;
    }        

    find_exit( ch, arg, FEX_NO_INVIS|FEX_DOOR|FEX_NO_EMPTY|FEX_VERBOSE );
}


/*--------------------------------------------------------------------
 *   lock 
 *-------------------------------------------------------------------*/
static void lock_door( Character *ch, int door )
{
    // 'lock door'
    EXIT_DATA *pexit;
    EXIT_DATA *pexit_rev = 0;
    Room *room = ch->in_room;

    pexit        = ch->in_room->exit[door];
    if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
    {
            ch->println( "Здесь не закрыто." );
            return;
    }

    if ( IS_SET(pexit->exit_info, EX_NOLOCK) ) 
    {
        ch->println( "Это невозможно запереть." );
        return;
    }

    if ( pexit->key <= 0 )
    {
            ch->println( "Здесь нет замочной скважины -- просто закрой дверь." );
            return;
    }

    if (!get_key_carry( ch, pexit->key))
    {
            ch->println( "У тебя нет ключа." );
            return;
    }

    if ( IS_SET(pexit->exit_info, EX_LOCKED) )
    {
            ch->println( "Здесь уже заперто." );
            return;
    }

    SET_BIT(pexit->exit_info, EX_LOCKED);
    ch->println( "*Щелк*" );
    act( "$c1 запирает $N4 на ключ.", ch, 0, direction_doorname(pexit), TO_ROOM);

    /* lock the other side */
    if ((pexit_rev = direction_reverse(room, door)))
    {
            SET_BIT( pexit_rev->exit_info, EX_LOCKED );
            direction_target(room, door)->echo(POS_RESTING, "%^N1 защелкивается.", direction_doorname(pexit_rev));
    }
}

CMDRUNP( lock )
{
    char arg[MAX_INPUT_LENGTH];
    Object *obj;
    EXTRA_EXIT_DATA *peexit;
    int door;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
            ch->println( "Запереть что?" );
            return;
    }

    if (( door = find_exit( ch, arg, FEX_NO_INVIS|FEX_DOOR|FEX_NO_EMPTY ) ) >= 0)
    {
        lock_door( ch, door );
        return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != 0 )
    {
        // portal stuff
        if (obj->item_type == ITEM_PORTAL)
        {
                if ( !IS_SET(obj->value[1],EX_ISDOOR)
                        || IS_SET(obj->value[1],EX_NOCLOSE) )
                {
                        ch->println( "Ты не можешь сделать этого." );
                        return;
                }

                if (!IS_SET(obj->value[1],EX_CLOSED))
                {
                        ch->println( "Здесь не закрыто." );
                        return;
                }

                    if (IS_SET(obj->value[1],EX_NOLOCK))
                {
                        ch->println( "Это невозможно запереть." );
                        return;
                }

                if (obj->value[4] <= 0) 
                {
                    ch->println( "Здесь нет замочной скважины -- просто закрой." );
                    return;
                }

                if (!get_key_carry(ch,obj->value[4]))
                {
                        ch->println( "У тебя нет ключа." );
                        return;
                }

                if (IS_SET(obj->value[1],EX_LOCKED))
                {
                        ch->println( "Здесь уже заперто." );
                        return;
                }

                SET_BIT(obj->value[1],EX_LOCKED);
                act_p("Ты закрываешь $o4 на ключ.",ch,obj,0,TO_CHAR,POS_RESTING);
                act_p("$c1 закрывает $o4 на ключ.",ch,obj,0,TO_ROOM,POS_RESTING);
        }
        else if ( obj->item_type == ITEM_CONTAINER )
        {
            // 'lock object'
            if ( !IS_SET(obj->value[1], CONT_CLOSED) )
            {
                    ch->println( "Это не закрыто." );
                    return;
            }

            if ( obj->value[2] < 0 )
            {
                    ch->println( "Здесь нет замочной скважины -- просто закрой." );
                    return;
            }
            
            if ( IS_SET(obj->value[1], CONT_LOCKED) )
            {
                    ch->println( "Это уже заперто." );
                    return;
            }

            if ((obj->behavior && obj->behavior->canLock( ch ))
                || get_key_carry( ch, obj->value[2])) 
            {
                SET_BIT(obj->value[1], CONT_LOCKED);
                act("Ты закрываешь $o4 на ключ.",ch,obj,0,TO_CHAR);
                act("$c1 закрывает $o4 на ключ.", ch, obj, 0, TO_ROOM);
                
            } else {
                ch->println( "У тебя нет ключа." );
                return;
            }
        }
        else if (obj->item_type == ITEM_DRINK_CON) {
            // lock drink containers

            if (IS_SET(obj->value[3], DRINK_LOCKED)) {
                if (IS_SET(obj->value[3], DRINK_CLOSE_CORK))
                    ch->pecho( "%1$^O1 и так плотно закупоре%1$Gно|н|но пробкой.", obj );
                else if (IS_SET(obj->value[3], DRINK_CLOSE_NAIL))
                    ch->pecho( "%1$^O1 и так закры%1$Gто|т|то крышкой и заколоче%1$Gно|н|но.", obj );
                else
                    ch->pecho( "%1$^O1 и так крепко запер%1$Gто|т|то.", obj );
            }
            else {
                ch->pecho("%1$^O1 уже невозможно закупорить или заколотить намертво.", obj );
            }
            
            return;
        }
        else {
            ch->println( "Это не контейнер." );
            return;
        }

        if ( obj->in_room != 0 )
                save_items( obj->in_room );

        return;
    }

    if ( ( ( peexit = get_extra_exit( arg, ch->in_room->extra_exit ) ) != 0 )
            && ch->can_see( peexit ) )
    {
        if ( !IS_SET(peexit->exit_info, EX_ISDOOR) )
        {
                ch->println( "Это не дверь!" );
                return;
        }

        if ( !IS_SET(peexit->exit_info, EX_CLOSED) )
        {
                ch->println( "Здесь не закрыто." );
                return;
        }

        if ( IS_SET(peexit->exit_info, EX_NOLOCK) ) 
        {
            ch->println( "Это невозможно запереть." );
            return;
        }

        if ( peexit->key <= 0 )
        {
                ch->println( "Здесь нет замочной скважины -- просто закрой." );
                return;
        }

        if (!get_key_carry( ch, peexit->key))
        {
                ch->println( "У тебя нет ключа." );
                return;
        }

        if ( IS_SET(peexit->exit_info, EX_LOCKED) )
        {
                ch->println( "Здесь уже заперто." );
                return;
        }

        SET_BIT(peexit->exit_info, EX_LOCKED);
        ch->println( "*Щелк*" );
        act_p( "$c1 запирает $N4 на ключ.", ch, 0, peexit->short_desc_from, TO_ROOM,POS_RESTING );

        return;
    }


    find_exit( ch, arg, FEX_NO_INVIS|FEX_DOOR|FEX_NO_EMPTY|FEX_VERBOSE );
}



/*--------------------------------------------------------------------
 *    unlock 
 *-------------------------------------------------------------------*/
static void unlock_door( Character *ch, int door )
{
    // 'unlock door'
    EXIT_DATA *pexit;
    EXIT_DATA *pexit_rev = 0;
    Room *room = ch->in_room;

    pexit = ch->in_room->exit[door];
    if ( !IS_SET(pexit->exit_info, EX_CLOSED) )
    {
            ch->println( "Здесь не закрыто." );
            return;
    }

    if ( pexit->key <= 0 )
    {
            ch->println( "Здесь нет замочной скважины -- просто открой." );
            return;
    }

    if (!get_key_carry( ch, pexit->key))
    {
            ch->println( "У тебя нет ключа." );
            return;
    }

    if ( !IS_SET(pexit->exit_info, EX_LOCKED) )
    {
            ch->println( "Здесь уже не заперто." );
            return;
    }

    REMOVE_BIT(pexit->exit_info, EX_LOCKED);
    ch->println( "*Щелк*" );
    act( "$c1 открывает ключом $N4.", ch, 0, direction_doorname(pexit), TO_ROOM);

    // unlock the other side
    if ((pexit_rev = direction_reverse(room, door)))
    {
            REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
            direction_target(room, door)->echo(POS_RESTING, "%^N1 щелкает.", direction_doorname(pexit_rev));
    }
}

CMDRUNP( unlock )
{
    char arg[MAX_INPUT_LENGTH];
    Object *obj;
    int door;
    EXTRA_EXIT_DATA *peexit;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
            ch->println( "Отпереть что?" );
            return;
    }

    if (( door = find_exit( ch, arg, FEX_NO_INVIS|FEX_DOOR|FEX_NO_EMPTY ) ) >= 0)
    {
        unlock_door( ch, door );
        return;
    }

    if ( ( obj = get_obj_here( ch, arg ) ) != 0 )
    {
        // portal stuff
        if ( obj->item_type == ITEM_PORTAL )
        {
            if (!IS_SET(obj->value[1],EX_ISDOOR))
            {
                    ch->println( "Ты не можешь этого сделать." );
                    return;
            }

            if (!IS_SET(obj->value[1],EX_CLOSED))
            {
                    ch->println( "Здесь не закрыто." );
                    return;
            }

            if (obj->value[4] <= 0)
            {
                ch->println( "Здесь нет замочной скважины -- просто открой." );
                return;
            }

            if (!get_key_carry(ch,obj->value[4]))
            {
                    ch->println( "У тебя нет ключа." );
                    return;
            }

            if (!IS_SET(obj->value[1],EX_LOCKED))
            {
                    ch->println( "Здесь уже не заперто." );
                    return;
            }

            REMOVE_BIT(obj->value[1],EX_LOCKED);
            act_p("Ты открываешь ключом $o4.",ch,obj,0,TO_CHAR,POS_RESTING);
            act_p("$c1 открывает ключом $o4.",ch,obj,0,TO_ROOM,POS_RESTING);
        }
        else if ( obj->item_type == ITEM_CONTAINER )
        {
            // 'unlock object'

            if ( !IS_SET(obj->value[1], CONT_CLOSED) )
            {
                    ch->println( "Здесь не закрыто." );
                    return;
            }

            if ( obj->value[2] < 0 )
            {
                    ch->println( "Здесь нет замочной скважины -- просто открой." );
                    return;
            }

            if ( !IS_SET(obj->value[1], CONT_LOCKED) )
            {
                    ch->println( "Здесь уже не заперто." );
                    return;
            }

            if ((obj->behavior && obj->behavior->canLock( ch ))
                || get_key_carry( ch, obj->value[2])) 
            {
                REMOVE_BIT(obj->value[1], CONT_LOCKED);
                act_p("Ты открываешь ключом $o4.",ch,obj,0,TO_CHAR,POS_RESTING);
                act_p("$c1 открывает ключом $o4.", ch, obj, 0, TO_ROOM,POS_RESTING );
                
            } else {
                ch->println( "У тебя нет ключа." );
                return;
            }
        }
        else if ( obj->item_type == ITEM_DRINK_CON ) {
            Object *key;
            
            // uncork a bottle
            if (!IS_SET(obj->value[3], DRINK_LOCKED)) {
                ch->println( "Тут не заперто и не закупорено." );
                return;
            }

            key = get_key_carry( ch, obj->value[4] );

            if (!key) {
                if (IS_SET(obj->value[3], DRINK_CLOSE_CORK)) 
                    ch->println( "У тебя нечем вытащить пробку." );
                else if (IS_SET(obj->value[3], DRINK_CLOSE_NAIL))
                    ch->println( "У тебя нечем оторвать крышку." );
                else
                    ch->println( "У тебя нечем открыть эту емкость." );
                
                return;
            }

            if (IS_SET(obj->value[3], DRINK_CLOSE_CORK)) {
                act( "Ты расшатываешь пробку в $O6 с помощью $o4.", ch, key, obj, TO_CHAR );
                act( "$c1 расшатывает пробку в $O6 с помощью $o4.", ch, key, obj, TO_ROOM );
            }
            else if (IS_SET(obj->value[3], DRINK_CLOSE_NAIL)) {
                act( "Ты выдергиваешь гвозди из крышки $O2 с помощью $o4.", ch, key, obj, TO_CHAR );
                act( "$c1 выдергивает гвозди из крышки $O2 с помощью $o4.", ch, key, obj, TO_ROOM );
            }
            else {
                act( "Ты открываешь $o5 $O2.", ch, key, obj, TO_CHAR );
                act( "$c1 открывает $o5 $O2.", ch, key, obj, TO_ROOM );
            }

            REMOVE_BIT(obj->value[3], DRINK_LOCKED);
                
        }
        else
        {
            ch->println( "Это не контейнер." );
            return;
        }

        if ( obj->in_room != 0 )
                save_items( obj->in_room );

        return;
    }

    if ( ( ( peexit = get_extra_exit( arg, ch->in_room->extra_exit ) ) != 0 )
            && ch->can_see( peexit ) )
    {
        if ( !IS_SET(peexit->exit_info, EX_ISDOOR) )
        {
                ch->println( "Это не дверь!" );
                return;
        }

        if ( !IS_SET(peexit->exit_info, EX_CLOSED) )
        {
                ch->println( "Здесь не закрыто." );
                return;
        }

        if ( peexit->key <= 0 )
        {
                ch->println( "Здесь нет замочной скважины -- просто открой." );
                return;
        }

        if (!get_key_carry( ch, peexit->key))
        {
                ch->println( "У тебя нет ключа." );
                return;
        }

        if ( !IS_SET(peexit->exit_info, EX_LOCKED) )
        {
                ch->println( "Здесь уже не заперто." );
                return;
        }

        REMOVE_BIT(peexit->exit_info, EX_LOCKED);
        ch->println( "*Щелк*" );
        act_p( "$c1 открывает ключом $N4.", ch, 0, peexit->short_desc_from, TO_ROOM,POS_RESTING );

        return;
    }

    find_exit( ch, arg, FEX_NO_INVIS|FEX_DOOR|FEX_NO_EMPTY|FEX_VERBOSE );
}




/*------------------------------------------------------------------------
 * Keyhole base class
 *-----------------------------------------------------------------------*/
const int Keyhole::MAX_KEY_TYPES     = 8;
const int Keyhole::LOCK_VALUE_MULTI  = -1;
const int Keyhole::LOCK_VALUE_BLANK  = -2;
const int Keyhole::ERROR_KEY_TYPE    = -3;

Keyhole::Keyhole( )
          : ch( NULL ), lockpick( NULL ), keyring( NULL ), key( NULL )
{
}

Keyhole::Keyhole( Character *ach ) 
             : ch( ach )
{
}

Keyhole::Keyhole( Character *ach, Object *akey ) 
             : ch( ach ), key( akey )
{
}

Keyhole::~Keyhole( )
{
}

Keyhole::Pointer Keyhole::locate( Character *ch, Object *key )
{
    Keyhole::Pointer null;
    int keyVnum = key->pIndexData->vnum;
    
    for (Room *room = room_list; room; room = room->rnext) {
        if (!ch->can_see( room ))
            continue;

        for (int d = 0; d < DIR_SOMEWHERE; d++)
            if (room->exit[d] && room->exit[d]->key == keyVnum)
                if (!room->exit[d]->u1.to_room || ch->can_see( room->exit[d] ))
                    return DoorKeyhole::Pointer( NEW, ch, room, d, key );

        for (EXTRA_EXIT_DATA *ex = room->extra_exit; ex; ex = ex->next)
            if (ex->key == keyVnum)
                if (ch->can_see( ex ))
                    return ExtraExitKeyhole::Pointer( NEW, ch, room, ex, key );
    }

    for (Object *obj = object_list; obj; obj = obj->next) {
        if (!ch->can_see( obj ) 
            || !ch->can_see( obj->getRoom( ) )
            || (obj->getCarrier( ) && !ch->can_see( obj->getCarrier( ) )))
            continue;

        if (obj->item_type == ITEM_PORTAL && obj->value[4] == keyVnum)
            return PortalKeyhole::Pointer( NEW, ch, obj, key );

        if (obj->item_type == ITEM_CONTAINER && obj->value[2] == keyVnum)
            return ContainerKeyhole::Pointer( NEW, ch, obj, key );
    }

    return null;
}

Keyhole::Pointer Keyhole::create( Character *ch, const DLString &arg )
{
    Object *obj;
    EXTRA_EXIT_DATA *peexit;
    int door;
    Keyhole::Pointer null;

    if (( peexit = get_extra_exit( arg.c_str( ), ch->in_room->extra_exit ) )
                && ch->can_see( peexit ))
    {
        return ExtraExitKeyhole::Pointer( NEW, ch, ch->in_room, peexit );
    }

    if (( door = find_exit( ch, arg.c_str( ), 
                            FEX_NO_INVIS|FEX_DOOR|FEX_NO_EMPTY) ) >= 0)
    {
        return DoorKeyhole::Pointer( NEW, ch, ch->in_room, door );
    }

    if (( obj = get_obj_here( ch, arg.c_str( ) ) )) {
        if (obj->item_type == ITEM_PORTAL)
            return PortalKeyhole::Pointer( NEW, ch, obj );

        if (obj->item_type == ITEM_CONTAINER)
            return ContainerKeyhole::Pointer( NEW, ch, obj );

        act( "В $o6 нет замочной скважины.", ch, obj, 0, TO_CHAR );
        return null;
    }

    return null;
}

void Keyhole::argsPickLock( const DLString &arg )
{
    char buf[MAX_INPUT_LENGTH];
    char *pbuf = buf;

    strcpy( buf, arg.c_str( ) );
    
    while (*pbuf++) {
        if (*pbuf == ':') {
            argLockpick = pbuf + 1;
            *pbuf = 0;
            argKeyring = buf;
            return;
        }
    }

    argLockpick = arg;
}

bool Keyhole::isPickProof( )
{
    return IS_SET(getLockFlags( ), bitPickProof( ));
}

bool Keyhole::isCloseable( )
{
    return IS_SET(getLockFlags( ), bitCloseable( ));
}

bool Keyhole::hasKey( )
{
    return getKey( ) > 0;
}

bool Keyhole::isLockable( )
{
    if (!hasKey( ))
        return false;
    
    if (bitUnlockable( ) == 0)
        return true;
        
    return !IS_SET(getLockFlags( ), bitUnlockable( ));
}

int Keyhole::getLockType( )
{
    return (hasKey( ) ? getKey( ) % MAX_KEY_TYPES : ERROR_KEY_TYPE);
}

bool Keyhole::doPick( const DLString &arg )
{
    bitstring_t flags = getLockFlags( );

    if (!isLockable( )) {
        ch->println( "Здесь нет замочной скважины." );
        return false;
    }

    if (!IS_SET(flags, bitLocked( ))) {
        ch->println( "Здесь уже не заперто." );
        return false;
    }
    
    if (!checkGuards( ))
        return false;
    
    if (isPickProof( )) {
        ch->println( "Этот замок защищен от взлома." );
        return false;
    }
    
    argsPickLock( arg );

    if (!findLockpick( ))
        return false;
    
    msgTryPickOther( );

    if (!checkLockPick( lockpick )) {
        act( "Ты не смо$gгло|г|гла пропихнуть $o4 в эту замочную скважину.", ch, lockpick, 0, TO_CHAR );
        ch->setWait( gsn_pick_lock->getBeats( ) / 2 );
        return false;
    }
    
    msgTryPickSelf( );

    if (number_percent( ) >= gsn_pick_lock->getEffective( ch )) {
        if (number_percent( ) >= gsn_pick_lock->getEffective( ch )
            && number_percent( ) > lockpick->value[1]) 
        {
            ch->pecho( "  ... но, слишком резко надавив, ломаешь %1$P2!", lockpick );
            extract_obj( lockpick );
        }
        else
            ch->println( "  ... но твои манипуляции ни к чему не приводят." );

        gsn_pick_lock->improve( ch, false );
        ch->setWait( gsn_pick_lock->getBeats( ) );
        return false;
    }
    
    unlock( );

    gsn_pick_lock->improve( ch, true );
    ch->setWait( gsn_pick_lock->getBeats( ) / 2 );
    record( lockpick );
    return true;
}

void Keyhole::unlock( )
{
    REMOVE_BIT(getLockFlags( ), bitLocked( ));
    ch->in_room->echo( POS_RESTING, "*Щелк*" );
}

bool Keyhole::checkLockPick( Object *o )
{
    if (o->item_type != ITEM_LOCKPICK)
        return false;
        
    if (!ch->can_see( o ) && !ch->can_hear( o ))
        return false;
        
    if (o->value[0] == LOCK_VALUE_MULTI)
        return true;
        
    return o->value[0] == getLockType( );
}

bool Keyhole::checkGuards( )
{
    for (Character *rch = ch->in_room->people; rch; rch = rch->next_in_room)
        if (rch->is_npc( )
                && IS_AWAKE(rch)
                && ch->getModifyLevel( ) + 5 < rch->getModifyLevel( ))
        {
            act( "$C1 маячит перед тобой, загораживая вожделенный замок.", ch, 0, rch, TO_CHAR );
            return false;
        }

    return true;
}

bool Keyhole::findLockpick( )
{
    if (!argKeyring.empty( )) {
        if (!( keyring = get_obj_list_type( ch, argKeyring, ITEM_KEYRING, ch->carrying ) )) {
            ch->println( "У тебя нет такого кольца для ключей." );
            return false;
        }

        if (!( lockpick = get_obj_list_type( ch, argLockpick, ITEM_LOCKPICK, keyring->contains ) )) {
            act( "На $o6 не нанизано ничего похожего.", ch, keyring, 0, TO_CHAR );
            return false;
        }
    }
    else if (!( lockpick = get_obj_list_type( ch, argLockpick, ITEM_LOCKPICK, ch->carrying )) ) {
        ch->println( "У тебя нет такой отмычки." );
        return false;
    }

    return true;
}

void Keyhole::record( Object *obj )
{
    char *ed_text;
    DLString edText, edEntry;
    
    if (!obj->getOwner( ) || ch->getName( ) != obj->getOwner( ))
        return;

    if (!( ed_text = get_extra_descr( obj->getName( ), obj->extra_descr ) ))
        return;
    
    edText  = ed_text;
    edEntry = getDescription( ).ruscase( '2' );
   
    if (edText.find( edEntry ) != DLString::npos)
        return;

    obj->addExtraDescr( obj->getName( ), 
                        edText + "       " + edEntry + "\n" );
}

bool Keyhole::doLore( ostringstream &buf )
{
    if (number_percent( ) >= gsn_golden_eye->getEffective( ch ))
        return false;

    if (!isLockable( )) 
        buf << "Это ключ от поломанного замка." << endl;
    else if (isPickProof( )) 
        buf << "Открывает защищенный от взлома замок на "
            << getDescription( ).ruscase( '6' ) << "." << endl;
    else
        buf << "Открывает замок на "
            << getDescription( ).ruscase( '6' ) << "." << endl;
    
    if (key->value[0] == 0)
        buf << "Рассыпается, лежа в кармане." << endl;

    if (key->value[1] > 0)
        buf << "Рассыпается, лежа на земле." << endl;

    gsn_golden_eye->improve( ch, true );
    return true;
}

bool Keyhole::doExamine( )
{
    if (!isLockable( ))
        return false;

    if (number_percent( ) >= gsn_golden_eye->getEffective( ch ))
        return false;
        
    if (isPickProof( )) 
        act( "Замок защищен от взлома.", ch, 0, 0, TO_CHAR );
    else {
        act( "Замок не устоит перед хорошим взломщиком.", ch, 0, 0, TO_CHAR );

        for (Object *o = ch->carrying; o; o = o->next_content) {
            if (checkLockPick( o )) {
                ch->pecho( "%1$^O1 тихонько звяка%1$nет|ют.", o );
                continue;
            }

            if (!ch->can_see( o ) && !ch->can_hear( o ))
                continue;

            if (o->item_type == ITEM_KEYRING) 
                for (Object *l = o->contains; l; l = l->next_content)
                    if (checkLockPick( l )) 
                        ch->pecho( "%1$^O1 на %2$O6 тихонько звяка%1$nет|ют.", o, l );
        }
    }
    
    gsn_golden_eye->improve( ch, true );
    return true;
}


/*------------------------------------------------------------------------
 * ItemKeyhole 
 *-----------------------------------------------------------------------*/
ItemKeyhole::ItemKeyhole( Character *ch, Object *obj )
{
    this->ch = ch;
    this->obj = obj;
}
ItemKeyhole::ItemKeyhole( Character *ch, Object *obj, Object *key )
{
    this->ch = ch;
    this->obj = obj;
    this->key = key;
}
int & ItemKeyhole::getLockFlags( )
{
    return obj->value[1];
}
bool ItemKeyhole::checkGuards( )
{
    return !obj->in_room || Keyhole::checkGuards( );
}
void ItemKeyhole::unlock( )
{
    Keyhole::unlock( );

    if (obj->in_room)
        save_items( obj->in_room );
}
void ItemKeyhole::msgTryPickSelf( )
{
    act( "Ты осторожно поворачиваешь $o4 в замочной скважине $O2.", ch, lockpick, obj, TO_CHAR );
}
void ItemKeyhole::msgTryPickOther( )
{
    act( "$c1 ковыряется в замке $O2.", ch, lockpick, obj, TO_ROOM );
}
DLString ItemKeyhole::getDescription( )
{
    DLString buf;

    buf << obj->getShortDescr( );
    if (obj->getCarrier( ) == 0)
        buf << " из '" << obj->getRoom( )->name << "'";

    return buf;
}
/*------------------------------------------------------------------------
 * ContainerKeyhole 
 *-----------------------------------------------------------------------*/
ContainerKeyhole::ContainerKeyhole( Character *ch, Object *obj )
          : ItemKeyhole( ch, obj )
{
}
ContainerKeyhole::ContainerKeyhole( Character *ch, Object *obj, Object *key )
          : ItemKeyhole( ch, obj, key )
{
}
bitstring_t ContainerKeyhole::bitPickProof( )
{
    return CONT_PICKPROOF;
}
bitstring_t ContainerKeyhole::bitLocked( )
{
    return CONT_LOCKED;
}
bitstring_t ContainerKeyhole::bitCloseable( ) 
{
    return CONT_CLOSEABLE;
}
bitstring_t ContainerKeyhole::bitUnlockable( ) 
{
    return 0;
}
int ContainerKeyhole::getKey( )
{
    return obj->value[2];
}
/*------------------------------------------------------------------------
 * ExitKeyhole 
 *-----------------------------------------------------------------------*/
bitstring_t ExitKeyhole::bitPickProof( )
{
    return EX_PICKPROOF;
}
bitstring_t ExitKeyhole::bitLocked( )
{
    return EX_LOCKED;
}
bitstring_t ExitKeyhole::bitCloseable( ) 
{
    return EX_ISDOOR;
}
bitstring_t ExitKeyhole::bitUnlockable( ) 
{
    return EX_NOLOCK;
}
/*------------------------------------------------------------------------
 * PortalKeyhole 
 *-----------------------------------------------------------------------*/
PortalKeyhole::PortalKeyhole( Character *ch, Object *obj )
          : ItemKeyhole( ch, obj )
{
}
PortalKeyhole::PortalKeyhole( Character *ch, Object *obj, Object *key )
          : ItemKeyhole( ch, obj, key )
{
}
int PortalKeyhole::getKey( )
{
    return obj->value[4];
}
/*------------------------------------------------------------------------
 * DoorKeyhole 
 *-----------------------------------------------------------------------*/
DoorKeyhole::DoorKeyhole( Character *ch, Room *room, int door )
{
    this->ch = ch;
    this->room = room;
    this->door = door;
    pexit = room->exit[door];
    to_room = pexit->u1.to_room;
    pexit_rev = (to_room ? to_room->exit[dirs[door].rev] : 0);
}

DoorKeyhole::DoorKeyhole( Character *ch, Room *room, int door, Object *key )
{
    this->ch = ch;
    this->room = room;
    this->door = door;
    this->key = key;
    pexit = room->exit[door];
    to_room = pexit->u1.to_room;
    pexit_rev = (to_room ? to_room->exit[dirs[door].rev] : 0);
}

int & DoorKeyhole::getLockFlags( )
{
    return pexit->exit_info;
}
void DoorKeyhole::unlock( )
{
    ExitKeyhole::unlock( );
    
    if (pexit_rev && pexit_rev->u1.to_room == room) {
        REMOVE_BIT(pexit_rev->exit_info, bitLocked( ));
        to_room->echo( POS_RESTING, "Дверной замок щелкает." );
    }
}
void DoorKeyhole::msgTryPickSelf( )
{
    act( "Ты осторожно поворачиваешь $o4 в замочной скважине.", ch, lockpick, 0, TO_CHAR );
}
void DoorKeyhole::msgTryPickOther( )
{
    act( "$c1 ковыряется в замке двери $t отсюда.", ch, dirs[door].leave, 0, TO_ROOM );
}
DLString DoorKeyhole::getDescription( )
{
    DLString buf;
    
    buf << "двер|ь|и|и|ь|ью|и из '" << room->name << "'";
    if (to_room)
        buf <<  " в '" << to_room->name << "'";

    return buf;
}
int DoorKeyhole::getKey( )
{
    return pexit->key;
}
/*------------------------------------------------------------------------
 * ExtraExitKeyhole 
 *-----------------------------------------------------------------------*/
ExtraExitKeyhole::ExtraExitKeyhole( Character *ch, Room *room, EXTRA_EXIT_DATA *peexit )
{
    this->ch = ch;
    this->room = room;
    this->peexit = peexit;
}

ExtraExitKeyhole::ExtraExitKeyhole( Character *ch, Room *room, EXTRA_EXIT_DATA *peexit, Object *key )
{
    this->ch = ch;
    this->room = room;
    this->peexit = peexit;
    this->key = key;
}

int & ExtraExitKeyhole::getLockFlags( )
{
    return peexit->exit_info;
}

void ExtraExitKeyhole::msgTryPickSelf( )
{
    act( "Ты осторожно поворачиваешь $o4 в замочной скважине $N2.", ch, lockpick, peexit->short_desc_from, TO_CHAR );
}
void ExtraExitKeyhole::msgTryPickOther( )
{
    act( "$c1 ковыряется в замке $N2.", ch, 0, peexit->short_desc_from, TO_ROOM );
}

DLString ExtraExitKeyhole::getDescription( )
{
    return peexit->short_desc_from;
}
int ExtraExitKeyhole::getKey( )
{
    return peexit->key;
}
