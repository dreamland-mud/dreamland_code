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
#include "roomutils.h"
#include "act.h"
#include "merc.h"

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

        if (!IS_SET(obj->extra_flags, ITEM_NOSAVEDROP))
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

        if (!IS_SET(obj->extra_flags, ITEM_NOSAVEDROP))
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
        
    oldact_p("Ты выкапываешься из земли.", ch, 0, 0, TO_CHAR, POS_DEAD); 
    oldact("Земля шевелится, и $c1 выкапывается из своей могилы.", ch, 0, 0, TO_ROOM); 
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

    oldact_p("Землятрясение заставляет тебя выбраться из могилы.", ch, 0, 0, TO_CHAR, POS_DEAD); 
    oldact("Земля раскалывается, обнаруживая лежавш$gое|его|ую в ней $c2!", ch, 0, 0, TO_ROOM); 
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

void strip_camouflage( Character *ch )
{
    if ( IS_AFFECTED( ch, AFF_CAMOUFLAGE ) )
    {
            bool showMessages = ch->affected.findAllWithBits(&affect_flags, AFF_CAMOUFLAGE).empty();
            affect_bit_strip(ch, &affect_flags, AFF_CAMOUFLAGE, true);
            REMOVE_BIT(ch->affected_by, AFF_CAMOUFLAGE);
            ch->ambushing = &str_empty[0];

            // TODO remove messages once camouflage is added via an affect.
            if (showMessages) {
                ch->pecho("Ты выходишь из своего укрытия.");
                oldact("$c1 выходит из $s укрытия.", ch, 0, 0,TO_ROOM);
            }
    }
}

void strip_hide_and_fade(Character *ch)
{
    if (IS_AFFECTED(ch, AFF_HIDE | AFF_FADE))
    {
        bool showMessage = ch->affected.findAllWithBits(&affect_flags, AFF_HIDE|AFF_FADE).empty();
        affect_bit_strip(ch, &affect_flags, AFF_HIDE, true);
        affect_bit_strip(ch, &affect_flags, AFF_FADE, true);
        REMOVE_BIT(ch->affected_by, AFF_HIDE | AFF_FADE);

        // TODO remove messages and split this "if" block into two, once hide/fade become affects.
        if (showMessage) {
            ch->pecho("Ты выходишь из тени.");
            oldact("$c1 выходит из тени.", ch, 0, 0, TO_ROOM);
        }
    }
}

void strip_invisibility(Character *ch)
{
    if (IS_AFFECTED(ch, AFF_IMP_INVIS | AFF_INVISIBLE))
    {
        bool showMessage = ch->affected.findAllWithBits(&affect_flags, AFF_IMP_INVIS|AFF_INVISIBLE).empty();
        affect_bit_strip(ch, &affect_flags, AFF_IMP_INVIS, true);
        affect_bit_strip(ch, &affect_flags, AFF_INVISIBLE, true);
        REMOVE_BIT(ch->affected_by, AFF_INVISIBLE | AFF_IMP_INVIS);

        // For mobs, if invisibility is gained via a bit.
        if (showMessage) {
            oldact("Ты становишься видим$gо|ым|ой для окружающих.", ch, 0, 0, TO_CHAR);
            oldact("$c1 становится видим$gо|ым|ой для окружающих.", ch, 0, 0, TO_ROOM);
        }
    }
}

void strip_improved_invisibility(Character *ch)
{
   if (IS_AFFECTED(ch, AFF_IMP_INVIS)) {
        bool showMessage = ch->affected.findAllWithBits(&affect_flags, AFF_IMP_INVIS).empty();
        affect_bit_strip(ch, &affect_flags, AFF_IMP_INVIS, true);
        REMOVE_BIT(ch->affected_by,  AFF_IMP_INVIS);

        // For mobs, if invisibility is gained via a bit.
        if (showMessage) {
            oldact("Ты становишься видим$gо|ым|ой для окружающих.", ch, 0, 0, TO_CHAR);
            oldact("$c1 становится видим$gо|ым|ой для окружающих.", ch, 0, 0, TO_ROOM);
        }        
    }
}

void check_camouflage( Character *ch, Room *to_room )
{
    if ( IS_AFFECTED(ch, AFF_CAMOUFLAGE)
            && !RoomUtils::isNature(to_room))
    {
            strip_camouflage( ch );
    }        
}

// Move object content to the same level as obj used to be (e.g. inventory or floor or another container).
// Usually called before obj destruction.
void obj_dump_content(Object *obj)
{
    Object *next_obj, *t_obj;

    if (!obj->contains)
        return;

    dreamland->removeOption(DL_SAVE_OBJS);

    for (t_obj = obj->contains; t_obj != 0; t_obj = next_obj) {
        next_obj = t_obj->next_content;

        obj_from_obj(t_obj);

        if (obj->in_room != 0)
            obj_to_room(t_obj, obj->in_room);
        else if (obj->carried_by != 0)
            obj_to_char(t_obj, obj->carried_by);
        else if (obj->in_obj)
            obj_to_obj(t_obj, obj->in_obj);
        else
            extract_obj(t_obj);
    }

    dreamland->resetOption(DL_SAVE_OBJS);

    save_items(obj->getRoom());
}
