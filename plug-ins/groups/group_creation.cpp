
/* $Id: group_creation.cpp,v 1.1.2.12.6.10 2010-08-24 20:23:09 rufina Exp $
 *
 * ruffina, 2004
 */
/***************************************************************************
 * Все права на этот код 'Dream Land' пренадлежат Igor {Leo} и Olga {Varda}*
 * Некоторую помощь в написании этого кода, а также своими идеями помогали:*
 *    Igor S. Petrenko     {NoFate, Demogorgon}                            *
 *    Koval Nazar          {Nazar, Redrum}                                 *
 *    Doropey Vladimir     {Reorx}                                         *
 *    Kulgeyko Denis       {Burzum}                                        *
 *    Andreyanov Aleksandr {Manwe}                                         *
 *    и все остальные, кто советовал и играл в этот MUD                    *
 ***************************************************************************/

#include "spelltemplate.h"
#include "xmlattributerestring.h"
#include "profflags.h"

#include "so.h"
#include "pcharacter.h"
#include "room.h"
#include "object.h"
#include "affect.h"
#include "magic.h"
#include "fight.h"
#include "act_move.h"
#include "gsn_plugin.h"

#include "handler.h"
#include "liquid.h"
#include "drink_utils.h"
#include "merc.h"
#include "mercdb.h"
#include "act.h"
#include "vnum.h"
#include "def.h"

LIQ(water);

#define OBJ_VNUM_MANNA               18
#define OBJ_VNUM_MUSHROOM             20
#define OBJ_VNUM_LIGHT_BALL             21
#define OBJ_VNUM_SPRING                     22
#define OBJ_VNUM_HOLY_SPRING             24
#define OBJ_VNUM_NATURAL_SPRING      36
#define OBJ_VNUM_DISC                     23
#define OBJ_VNUM_ROSE                   1001

SPELL_DECL(ContinualLight);
VOID_SPELL(ContinualLight)::run( Character *ch, char *target_name, int sn, int level ) 
{ 
    Object *light;

    if (target_name[0] != '\0'  /* do a glow on some object */
        && ( light = get_obj_carry(ch, target_name) ))
    {
        if (light->is_obj_stat(ITEM_GLOW)) {
            ch->pecho( "%1$^O1 уже свет%1$nится|ятся.", light );
            return;
        }

        SET_BIT(light->extra_flags,ITEM_GLOW);
        if (!ch->getProfession( )->getFlags( ch ).isSet(PROF_DIVINE))
            SET_BIT(light->extra_flags, ITEM_MAGIC);
        
        light->getRoom( )->echo( POS_RESTING, "%1$^O1 вспыхива%1$nет|ют белым светом.", light );
        return;
    }

    light = create_object( get_obj_index( OBJ_VNUM_LIGHT_BALL ), 0 );
    if (!ch->getProfession( )->getFlags( ch ).isSet(PROF_DIVINE))
        SET_BIT(light->extra_flags, ITEM_MAGIC);

    dress_created_item( sn, light, ch, target_name );
    obj_to_char( light, ch );

    act("$c1 взмахивает руками и создает $o4.",ch,light,0,TO_ROOM);
    act("Ты взмахиваешь руками и создаешь $o4.",ch,light,0,TO_CHAR);
}


SPELL_DECL(CreateFood);
VOID_SPELL(CreateFood)::run( Character *ch, char *target_name, int sn, int level ) 
{ 
    Object *mushroom;
    int vnum;
    
    if (ch->getProfession( )->getFlags( ch ).isSet(PROF_DIVINE))
        vnum = OBJ_VNUM_MANNA;
    else
        vnum = OBJ_VNUM_MUSHROOM;
        
    mushroom = create_object( get_obj_index( vnum ), 0 );
    mushroom->value[0] = std::min( level / 2, 35 );
    mushroom->value[1] = level;
    dress_created_item( sn, mushroom, ch, target_name );
    obj_to_char( mushroom, ch );

    if (ch->getProfession( )->getFlags( ch ).isSet(PROF_DIVINE)) {
        act( "$c1 взмахивает руками, и с неба падает $o1.", ch, mushroom, 0, TO_ROOM );
        act( "Ты взмахиваешь руками, и с неба падает $o1.", ch, mushroom, 0, TO_CHAR );
    }
    else {
        act( "$c1 взмахивает руками и создает $o4.", ch, mushroom, 0, TO_ROOM);
        act( "Ты взмахиваешь руками и создаешь $o4.", ch, mushroom, 0, TO_CHAR);
    }
}


SPELL_DECL(CreateRose);
VOID_SPELL(CreateRose)::run( Character *ch, char *target_name, int sn, int level ) 
{ 
    Object *rose;

    rose = create_object(get_obj_index(OBJ_VNUM_ROSE), 0);
    dress_created_item( sn, rose, ch, target_name );
    obj_to_char(rose, ch);
    
    if (!rose->getRealShortDescr( )) {
        act("$c1 взмахивает руками и создает прекрасную $o4.", ch, rose, 0, TO_ROOM);
        act("Ты взмахиваешь руками и создаешь прекрасную $o4.", ch, rose, 0, TO_CHAR);
    }
    else {
        act("$c1 взмахивает руками и создает $o4.", ch, rose, 0, TO_ROOM);
        act("Ты взмахиваешь руками и создаешь $o4.", ch, rose, 0, TO_CHAR);
    }
}


SPELL_DECL(CreateSpring);
VOID_SPELL(CreateSpring)::run( Character *ch, char *target_name, int sn, int level ) 
{ 
    Object *spring;
    int vnum;

    if (ch->getProfession( )->getFlags( ch ).isSet(PROF_DIVINE)) 
        vnum = OBJ_VNUM_HOLY_SPRING;
    else if (ch->getProfession( )->getFlags( ch ).isSet(PROF_NATURE)) {
         if (ch->in_room->sector_type != SECT_FIELD
             && ch->in_room->sector_type != SECT_FOREST
             && ch->in_room->sector_type != SECT_HILLS
             && ch->in_room->sector_type != SECT_MOUNTAIN)
        {
            ch->println("Ты не сможешь создать родник в этой местности.");
            return;
        }
        
        vnum = OBJ_VNUM_NATURAL_SPRING;
    }
    else
        vnum = OBJ_VNUM_SPRING;

    if (get_obj_room_vnum( ch->in_room, vnum )) {
        ch->send_to("Ты хочешь сделать тут озеро?\r\n");
        return;
    }

    spring = create_object( get_obj_index( vnum ), 0 );
    spring->timer = level;
    obj_to_room( spring, ch->in_room );
    dress_created_item( sn, spring, ch, target_name );
    act( "$o1 пробивается сквозь землю.", ch, spring, 0, TO_ALL );
}

SPELL_DECL(CreateWater);
VOID_SPELL(CreateWater)::run( Character *ch, Object *obj, int sn, int level ) 
{ 
    int water;

    if ( obj->item_type != ITEM_DRINK_CON )
    {
        ch->send_to("Здесь не может содержаться вода.\n\r");
        return;
    }

    if (drink_is_closed( obj, ch ))
        return;

    if (obj->value[2] != liq_water && obj->value[1] != 0 )
    {
        ch->send_to("Контейнер содержит другую жидкость.\n\r");
        return;
    }

    water = min(
                level * (weather_info.sky >= SKY_RAINING ? 4 : 2),
                obj->value[0] - obj->value[1]
                );

    if ( water > 0 )
    {
        obj->value[2] = liq_water;
        obj->value[1] += water;
        if ( !is_name( "water", obj->getName( )) )
            obj->fmtName( "%s water", obj->getName( ) );

        ch->pecho( "%1$^O1 наполне%1$Gно|н|на|ны.", obj );
    }
    else
        ch->pecho( "%1$^O1 уже заполне%1$Gно|н|на|ны.", obj );


}


SPELL_DECL(FloatingDisc);
VOID_SPELL(FloatingDisc)::run( Character *ch, char *target_name, int sn, int level ) 
{ 
    Object *disc, *floating;

    floating = get_eq_char(ch,wear_float);
    if (floating != 0 && IS_OBJ_STAT(floating,ITEM_NOREMOVE))
    {
        act_p("Ты не можешь снять $o4.",ch,floating,0,TO_CHAR,POS_RESTING);
        return;
    }

    disc = create_object(get_obj_index(OBJ_VNUM_DISC), 0);
    disc->value[0]        = ch->getModifyLevel() * 10; /* 10 pounds per level capacity */
    disc->value[3]        = ch->getModifyLevel() * 5; /* 5 pounds per level max per item */
    disc->timer                = ch->getModifyLevel() * 2 - number_range(0,level / 2);

    if (!ch->getProfession( )->getFlags( ch ).isSet(PROF_DIVINE)) 
        SET_BIT(disc->extra_flags, ITEM_MAGIC);

    dress_created_item( sn, disc, ch, target_name );
    obj_to_char(disc,ch);
    
    if (!disc->getRealShortDescr( )) {
        act("$c1 взмахивает руками и создает черный вращающийся диск.", ch,0,0,TO_ROOM);
        act("Ты взмахиваешь руками и создаешь вращающийся диск.", ch, 0, 0, TO_CHAR);
    }
    else {
        act("$c1 взмахивает руками и создает $o4.", ch, disc, 0, TO_ROOM);
        act("Ты взмахиваешь руками и создаешь $o4.", ch, disc, 0, TO_CHAR);
    }

    wear_obj(ch,disc,F_WEAR_REPLACE|F_WEAR_VERBOSE);
}

