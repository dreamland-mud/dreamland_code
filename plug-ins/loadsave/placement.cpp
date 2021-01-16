/* $Id$
 *
 * ruffina, 2004
 */
#include "loadsave.h"
#include "save.h"

#include "affect.h"
#include "outofband.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "object.h"
#include "room.h"

#include "dreamland.h"
#include "act.h"
#include "merc.h"
#include "mercdb.h"
#include "vnum.h"
#include "def.h"

WEARLOC(none);
WEARLOC(light);
void limit_timestamp( Object *obj, Character *ch );

/*
 * Move a char out of a room.
 */
void char_from_room( Character *ch )
{
        if ( ch->in_room == 0 )
        {
                bug( "Char_from_room: 0.", 0 );
                return;
        }

        if (!ch->is_npc() && !ch->is_immortal())
        {
                --ch->in_room->area->nplayer;
        }

        if ( ch->death_ground_delay > 0 )
        {
                ch->death_ground_delay = 0;
                ch->trap.clear( );
        }

        ch->in_room->updateLight();

        if ( ch == ch->in_room->people )
        {
                ch->in_room->people = ch->next_in_room;
        }
        else
        {
                Character *prev;

                for ( prev = ch->in_room->people; prev; prev = prev->next_in_room )
                {
                        if ( prev->next_in_room == ch )
                        {
                                prev->next_in_room = ch->next_in_room;
                                break;
                        }
                }

                if ( prev == 0 )
                        bug( "Char_from_room: ch not found.", 0 );
        }

        if ( ch->is_npc( )
                && !IS_CHARMED(ch) )
        {
                save_mobs( ch->in_room );
        }

//      outOfBandManager->run("mobListUpdate", MobListUpdateArgs(ch->desc));

        ch->in_room      = 0;
        ch->next_in_room = 0;
        ch->on              = 0;  /* sanity check! */
}



/*
 * Move a char into a room.
 */
void char_to_room( Character *ch, Room *pRoomIndex )
{
        Character *pop;
        int cnt = 0;

        if ( pRoomIndex == 0 )
        {
                Room *room;

                bug( "Char_to_room: 0.", 0 );
        
                if ((room = get_room_instance(ROOM_VNUM_TEMPLE)) != 0)
                        char_to_room(ch,room);
        
                return;
        }

        for(pop = pRoomIndex->people;
                        pop;
                        pop = pop->next_in_room, cnt++ );
        
        if( cnt > 0 )
                cnt = number_range(0, cnt);

        ch->in_room                                        = pRoomIndex;

        if( cnt > 0 )
        {
                pop = pRoomIndex->people;

                while( --cnt )
                        pop = pop->next_in_room;

                ch->next_in_room = pop->next_in_room;
                pop->next_in_room = ch;
        }
        else
        {
                ch->next_in_room                = pRoomIndex->people;
                pRoomIndex->people        = ch;
        }

        if (!ch->is_npc() && !ch->is_immortal())
        {
                if (ch->in_room->area->empty)
                {
                        ch->in_room->area->empty = false;
                        ch->in_room->area->age = 0;
                }
                ++ch->in_room->area->nplayer;
        }

        ch->in_room->updateLight();

        if ( ch->is_npc( )
                && !IS_CHARMED(ch)  )
        {
                save_mobs( pRoomIndex );
        }

        outOfBandManager->run("charToRoom", CharToRoomArgs(ch->desc));
//      outOfBandManager->run("mobListUpdate", MobListUpdateArgs(ch->desc));
}


/*
 * Give an obj to a char.
 */
void obj_to_char( Object *obj, Character *ch )
{
        obj->next_content         = ch->carrying;
        ch->carrying         = obj;
        obj->carried_by         = ch;
        obj->in_room         = 0;
        obj->in_obj                 = 0;
        ch->carry_number        += obj->getNumber( );
        ch->carry_weight        += obj->getWeight( );

        if ( ch->is_npc( )
                && !IS_CHARMED(ch)
                && ch->in_room != 0 )
        {
                save_mobs( ch->in_room );
        }

        limit_timestamp( obj, ch );
}

/*
 * Take an obj from its character.
 */
void obj_from_char( Object *obj )
{
        Character *ch;

        if ( ( ch = obj->carried_by ) == 0 )
        {
                bug( "Obj_from_char: null ch.", 0 );
                return;
        }
        
        obj->wear_loc->unequip( obj );

        if ( ch->carrying == obj )
        {
                ch->carrying = obj->next_content;
        }
        else
        {
                Object *prev;

                for ( prev = ch->carrying; prev != 0; prev = prev->next_content )
                {
                        if ( prev->next_content == obj )
                        {
                                prev->next_content = obj->next_content;
                                break;
                        }
                }

                if ( prev == 0 )
                        bug( "Obj_from_char: obj not in list.", 0 );
        }

        obj->carried_by                = 0;
        obj->wear_loc                        = wear_none;
        obj->next_content        = 0;
        ch->carry_number        -= obj->getNumber( );
        ch->carry_weight        -= obj->getWeight( );

        if ( ch->is_npc( )
                && !IS_CHARMED(ch)
                && ch->in_room != 0 )
        {
                save_mobs( ch->in_room );
        }


        return;
}


/*
 * Move an obj out of a room.
 */
void obj_from_room( Object *obj )
{
        Room *in_room;
        Character *ch;

        if ( ( in_room = obj->in_room ) == 0 )
        {
                bug( "obj_from_room: 0.", 0 );
                return;
        }

        for (ch = in_room->people; ch != 0; ch = ch->next_in_room)
                if (ch->on == obj)
                        ch->on = 0;

        if ( obj == in_room->contents )
        {
                in_room->contents = obj->next_content;
        }
        else
        {
                Object *prev;

                for ( prev = in_room->contents; prev; prev = prev->next_content )
                {
                        if ( prev->next_content == obj )
                        {
                                prev->next_content = obj->next_content;
                                break;
                        }
                }

                if ( prev == 0 )
                {
                        bug( "Obj_from_room: obj not found.", 0 );
                        return;
                }
        }

        Room * pRoomIndex = obj->in_room;

        obj->in_room      = 0;
        obj->next_content = 0;

        if (obj->item_type == ITEM_LIGHT)
            pRoomIndex->updateLight();

        save_items( pRoomIndex );

        return;
}

/*
 * Move an obj into a room.
 */
void obj_to_room( Object *obj, Room *pRoomIndex )
{
        obj->next_content = pRoomIndex->contents;
        pRoomIndex->contents = obj;
        obj->in_room = pRoomIndex;
        obj->carried_by = 0;
        obj->in_obj = 0;
        obj->wear_loc = wear_none;
        obj->water_float = -2;

        if (obj->item_type == ITEM_LIGHT)
            pRoomIndex->updateLight();

        save_items( pRoomIndex );
}



/*
 * Move an object into an object.
 */
void obj_to_obj( Object *obj, Object *obj_to )
{
        obj->next_content                = obj_to->contains;
        obj_to->contains                = obj;
        obj->in_obj                        = obj_to;
        obj->in_room                = 0;
        obj->carried_by                = 0;

        if (IS_PIT(obj_to))
                obj->cost = 0;

        for ( ; obj_to != 0; obj_to = obj_to->in_obj )
        {
                if ( obj_to->carried_by != 0 )
                {
                        // obj_to->carried_by->carry_number += obj->getNumber( );
                        obj_to->carried_by->carry_weight += obj->getWeight( )
                                                * obj_to->getWeightMultiplier() / 100;
                }
        }

        save_items_at_holder( obj );

        return;
}

void obj_to_obj_random( Object *item, Object *obj_to )
{
    Object *obj, *obj_prev = 0;
    int n = number_range( 2, 13 );
    
    for (obj = obj_to->contains; obj && --n; obj = obj->next_content)
        obj_prev = obj;
    
    if (obj) {
        obj_prev->next_content = item;
        item->next_content = obj;
        item->in_obj = obj_to;
    }
    else
        obj_to_obj( item, obj_to );
}            
        
/*
 * Move an object out of an object.
 */
void obj_from_obj( Object *obj )
{
        Object *obj_from;

        if ( ( obj_from = obj->in_obj ) == 0 )
        {
                bug( "Obj_from_obj: null obj_from.", 0 );
                return;
        }

        Object *u_obj = obj_from;

        if ( obj == obj_from->contains )
        {
                obj_from->contains = obj->next_content;
        }
        else
        {
                Object *prev;

                for ( prev = obj_from->contains; prev; prev = prev->next_content )
                {
                        if ( prev->next_content == obj )
                        {
                                prev->next_content = obj->next_content;
                                break;
                        }
                }

                if ( prev == 0 )
                {
                        bug( "Obj_from_obj: obj not found.", 0 );
                        return;
                }
        }

        obj->next_content = 0;
        obj->in_obj       = 0;
        obj->pocket = "";

        for ( ; obj_from != 0; obj_from = obj_from->in_obj )
        {
                if ( obj_from->carried_by != 0 )
                {
                        // obj_from->carried_by->carry_number -= obj->getNumber( );
                        obj_from->carried_by->carry_weight -= obj->getWeight( )
                                        * obj_from->getWeightMultiplier() / 100;
                }
        }

        save_items_at_holder( u_obj );

        return;
}


/*
 * Vampiric graves
 */
void extract_grave( Character *ch ) 
{
    Object *grave = get_obj_world_unique( OBJ_VNUM_GRAVE, ch );

    if (grave)
        extract_obj( grave );
}

void undig( Character *ch ) 
{
    if (!DIGGED(ch)) 
        return;
    
    REMOVE_BIT(ch->act, PLR_DIGGED);
    char_from_room( ch ); 
    char_to_room( ch, ch->was_in_room ); 
    ch->was_in_room = 0; 

    if (ch->ambushing && ch->ambushing[0]) {
        free_string( ch->ambushing );
        ch->ambushing = &str_empty[0]; 
    }

    if (ch->position < POS_RESTING)
        ch->position = POS_RESTING;
        
    act_p("Ты выкапываешься из земли.", ch, 0, 0, TO_CHAR, POS_DEAD); 
    act_p("Земля шевелится, и $c1 выкапывается из своей могилы.", ch, 0, 0, TO_ROOM, POS_RESTING ); 
    extract_grave( ch );
}

void undig_earthquake( Character *ch ) 
{
    if (!DIGGED(ch)) 
        return;
    
    REMOVE_BIT(ch->act, PLR_DIGGED);
    char_from_room( ch ); 
    char_to_room( ch, ch->was_in_room ); 
    ch->was_in_room = 0; 

    if (ch->ambushing && ch->ambushing[0]) {
        free_string( ch->ambushing );
        ch->ambushing = &str_empty[0]; 
    }

    act_p("Землятрясение заставляет тебя выбраться из могилы.", ch, 0, 0, TO_CHAR, POS_DEAD); 
    act_p("Земля раскалывается, обнаруживая лежавш$gое|его|ую в ней $c2!", ch, 0, 0, TO_ROOM, POS_RESTING); 
    extract_grave( ch );
}

/*
 * global list of objects
 */
void obj_from_list( Object *obj )
{
    if (obj->prev) {
        obj->prev->next = obj->next;
    } else {
        object_list = obj->next;
    }
    
    if (obj->next) {
        obj->next->prev = obj->prev;
    }

    obj->next = 0;
    obj->prev = 0;
}

void obj_to_list( Object *obj )
{
    if (object_list) {
        object_list->prev = obj;
    }

    obj->next                = object_list;
    object_list                = obj;
    obj->prev                = 0;
}

/*
 * global list of characters
 */
void char_to_list( Character *ch, Character **list )
{
    if (*list) {
        (*list)->prev = ch;
    }

    ch->next = *list;
    *list    = ch;
    ch->prev = 0;
}

void char_from_list( Character *ch, Character **list )
{
    if (ch->prev) {
        ch->prev->next = ch->next;
    } else {
        *list = ch->next;
    }
    
    if (ch->next) {
        ch->next->prev = ch->prev;
    }

    ch->next = 0;
    ch->prev = 0;
}

