/* $Id$
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
/***************************************************************************
 *     ANATOLIA 2.1 is copyright 1996-1997 Serdar BULUT, Ibrahim CANPUNAR  *
 *     ANATOLIA has been brought to you by ANATOLIA consortium                   *
 *         Serdar BULUT {Chronos}                bulut@rorqual.cc.metu.edu.tr       *
 *         Ibrahim Canpunar  {Asena}        canpunar@rorqual.cc.metu.edu.tr    *        
 *         Murat BICER  {KIO}                mbicer@rorqual.cc.metu.edu.tr           *        
 *         D.Baris ACAR {Powerman}        dbacar@rorqual.cc.metu.edu.tr           *        
 *     By using this code, you have agreed to follow the terms of the      *
 *     ANATOLIA license, in the file Anatolia/anatolia.licence             *        
 ***************************************************************************/

/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/
 
/***************************************************************************
*        ROM 2.4 is copyright 1993-1995 Russ Taylor                           *
*        ROM has been brought to you by the ROM consortium                   *
*            Russ Taylor (rtaylor@pacinfo.com)                                   *
*            Gabrielle Taylor (gtaylor@pacinfo.com)                           *
*            Brian Moore (rom@rom.efn.org)                                   *
*        By using this code, you have agreed to follow the terms of the           *
*        ROM license, in the file Rom24/doc/rom.license                           *
***************************************************************************/

#include "logstream.h"
#include "affect.h"
#include "character.h"
#include "room.h"
#include "object.h"

#include "dreamland.h"
#include "save.h"
#include "merc.h"
#include "mercdb.h"
#include "desire.h"
#include "act.h"
#include "gsn_plugin.h"
#include "fight.h"
#include "handler.h"
#include "material.h"
#include "magic.h"
#include "def.h"

using std::max;

DESIRE(thirst);
DESIRE(hunger);
GSN(corrosion);

/** Display decay message for items in inventory/eq or on the floor. */
static void show_effect_message(Object *obj, const DLString &msg)
{
    if (obj->carried_by || obj->in_room)
        obj->getRoom()->echo(POS_RESTING, msg.c_str(), obj);
}

void acid_effect(void *vo, short level, int dam, int target, bitstring_t dam_flag )
{
    if ( target == TARGET_ROOM ) /* nail objects on the floor */
    {
        Room *room = (Room *) vo;
        Object *obj, *obj_next;

        for ( obj = room->contents; obj != 0; obj = obj_next )
        {
            obj_next = obj->next_content;
            acid_effect(obj,level,dam,TARGET_OBJ, dam_flag);
        }
        return;
    }

    if ( target == TARGET_CHAR )  /* do the effect on a victim */
    {
        Character *victim = (Character *) vo;
        Object *obj, *obj_next;
        
        /* let's toast some gear */
        for ( obj = victim->carrying; obj != 0; obj = obj_next )
        {
            obj_next = obj->next_content;
            acid_effect(obj,level,dam,TARGET_OBJ, dam_flag);
        }
        return;
    }

    if ( target == TARGET_OBJ ) /* toast an object */
    {
        Object *obj = (Object *) vo;
        Object *t_obj,*n_obj;
        Character *victim = obj->carried_by;
        int chance;
        const char *msg;

        if ( IS_OBJ_STAT(obj,ITEM_BURN_PROOF ) ||
             IS_OBJ_STAT(obj,ITEM_NOPURGE) ||
             material_is_flagged(obj, MAT_INDESTR) )
             return;
         
        // effect level between 0.25 and 0.9 of spell level
        int dam_ratio, effect_level;
        if (victim)
            dam_ratio = (100 * dam) / max(1, (int)victim->max_hit);
        else
            dam_ratio = max(0, (level - obj->level) * 5);                  
        effect_level = level * (20 + dam_ratio) / 100; // adds 5% to effective level per each 1% damage dealt
        effect_level = URANGE(level * 25 / 100, effect_level, level * 90 / 100);
         
        chance = effect_level / 2;
        if ( IS_OBJ_STAT(obj,ITEM_BLESS) )
            chance -= 5;
        if (material_is_flagged(obj, MAT_TOUGH))
            chance /= 2;
        
        if (material_is_flagged( obj, MAT_MELTING ))  {
            chance *= 2;
            msg = "%1$^O1 та%1$nет|ют и испаря%1$nется|ются!";
        }
        else
        switch ( obj->item_type )
        {
        default:
            return;
        case ITEM_CONTAINER:
        case ITEM_CORPSE_PC:
        case ITEM_CORPSE_NPC:
            msg = "%1$^O1 дым%1$nится|ятся и медленно растворя%1$nется|ются.";
            break;
        case ITEM_ARMOR:
            msg = "%1$^O1 {gпокрыва%1$nется|ются {yокалиной.{x";
            break;
        case ITEM_CLOTHING:
            msg = "%1$^O1 рассыпа%1$nется|ются на клочки.";
            break;
        case ITEM_STAFF:
        case ITEM_WAND:
            chance -= 10;
            msg = "%1$^O1 переламыва%1$nется|ются пополам.";
            break;
        case ITEM_SCROLL:
        case ITEM_SPELLBOOK:
        case ITEM_TEXTBOOK:
        case ITEM_RECIPE:
            chance += 10;
            msg = "%1$^O1 обраща%1$nется|ются в пепел.";
            break;
        }

        chance = URANGE(1,chance,50);

        if ( number_percent() > chance )
            return;

        show_effect_message(obj, msg);

        if ( obj->item_type == ITEM_ARMOR )  /* etch it */
        {
            int i;
            Affect af;

            af.location = APPLY_AC;
            af.level    = effect_level;
            af.duration = -1;
            af.modifier = number_range(1,5);
            af.type     = gsn_corrosion;

            affect_enchant( obj );
            affect_enhance( obj, &af );

            if ( obj->carried_by != 0 && obj->wear_loc != wear_none )
                for ( i = 0; i < 4; i++ )
                    obj->carried_by->armor[i] += 1;
            return;
        }

        /* get rid of the object */
        if ( obj->contains )  /* dump contents */
        {
            dreamland->removeOption( DL_SAVE_OBJS );

            for ( t_obj = obj->contains; t_obj != 0; t_obj = n_obj )
            {
                n_obj = t_obj->next_content;
                obj_from_obj(t_obj);
                if ( obj->in_room != 0 )
                    obj_to_room(t_obj,obj->in_room);
                else if ( obj->carried_by != 0 )
                    obj_to_room(t_obj,obj->carried_by->in_room);
                else {
                    extract_obj(t_obj);
                    continue;
                }

                acid_effect(t_obj,level/2,dam/2,TARGET_OBJ, dam_flag);
                dreamland->removeOption( DL_SAVE_OBJS );
            }

            dreamland->resetOption( DL_SAVE_OBJS );

            if ( obj->in_room != 0 )
                save_items( obj->in_room );
            else if ( obj->carried_by != 0 )
                save_items( obj->carried_by->in_room );
        }

        extract_obj(obj);
        return;
    }
}

void cold_effect(void *vo, short level, int dam, int target, bitstring_t dam_flag )
{
    if (target == TARGET_ROOM) /* nail objects on the floor */
    {
        Room *room = (Room *) vo;
        Object *obj, *obj_next;

        for (obj = room->contents; obj != 0; obj = obj_next)
        {
            obj_next = obj->next_content;
            cold_effect(obj,level,dam,TARGET_OBJ, dam_flag);
        }
        return;
    }

    if (target == TARGET_CHAR) /* whack a character */
    {
        Character *victim = (Character *) vo;
        Object *obj, *obj_next;
     
        // saves against level*0.9 (worst) to level*0.25 (best)
        int dam_ratio, effect_level;
        dam_ratio = (100 * dam) / max(1, (int)victim->max_hit);
        effect_level = level * (20 + 5 * dam_ratio) / 100; // adds 5% to effective level per each 1% damage dealt
        effect_level = URANGE(level * 25 / 100, effect_level, level * 90 / 100);
        
        /* chill touch effect */
        if (!saves_spell(effect_level, victim, DAM_COLD, 0, dam_flag))
        {
            Affect af;

            act_p("Волна холода пронизывает $c4.",
                   victim,0,0,TO_ROOM,POS_RESTING);
            act_p("Холод окутывает тебя и проникает до самых костей.",
                   victim,0,0,TO_CHAR,POS_RESTING);
            af.type      = gsn_chill_touch;
            af.level     = level;
            af.duration  = 6;
            af.location = APPLY_STR;
            af.modifier = min(-1, -level / 15);
            affect_join( victim, &af );
        }

        /* hunger! (warmth sucked out */
        if (!victim->is_npc())
            desire_hunger->gain( victim->getPC( ), dam/20 );

        /* let's toast some gear */
        for (obj = victim->carrying; obj != 0; obj = obj_next)
        {
            obj_next = obj->next_content;
            cold_effect(obj,level,dam,TARGET_OBJ, dam_flag);
        }
        return;
   }

   if (target == TARGET_OBJ) /* toast an object */
   {
        Object *obj = (Object *) vo;
        Character *victim = obj->carried_by;       
        int chance;
        const char *msg;

        if ( IS_OBJ_STAT(obj,ITEM_NOPURGE) ||
             material_is_flagged(obj, MAT_INDESTR) )
             return;
         
        // effect level between 0.25 and 0.9 of spell level
        int dam_ratio, effect_level;
        if (victim)
            dam_ratio = (100 * dam) / max(1, (int)victim->max_hit);
        else
            dam_ratio = max(0, (level - obj->level) * 5);                  
        effect_level = level * (20 + dam_ratio) / 100; // adds 5% to effective level per each 1% damage dealt
        effect_level = URANGE(level * 25 / 100, effect_level, level * 90 / 100);
         
        chance = effect_level / 2;
        if ( IS_OBJ_STAT(obj,ITEM_BLESS) )
            chance -= 5;
        if (material_is_flagged(obj, MAT_TOUGH))
            chance /= 2;

        switch(obj->item_type)
        {
            default:
                return;
            case ITEM_POTION:
                msg = "%1$^O1 покрыва%1$nется|ются инеем и взрыва%1$nется|ются!";
                chance += 25;
                break;
            case ITEM_DRINK_CON:
                msg = "%1$^O1 покрыва%1$nется|ются инеем и взрыва%1$nется|ются!";
                chance += 5;
                break;
        }

        chance = URANGE(1,chance,50);

        if (number_percent() > chance)
            return;

        show_effect_message(obj, msg);

        extract_obj(obj);
        return;
    }
}



void fire_effect(void *vo, short level, int dam, int target, bitstring_t dam_flag )
{
    if (target == TARGET_ROOM)  /* nail objects on the floor */
    {
        Room *room = (Room *) vo;
        Object *obj, *obj_next;
        for (obj = room->contents; obj != 0; obj = obj_next)
        {
            obj_next = obj->next_content;
            fire_effect(obj,level,dam,TARGET_OBJ, dam_flag);
        }
        return;
    }

    if (target == TARGET_CHAR)   /* do the effect on a victim */
    {
        Character *victim = (Character *) vo;
        Object *obj, *obj_next;
        // saves against level*0.9 (worst) to level*0.25 (best)
        int dam_ratio, effect_level;
        dam_ratio = (100 * dam) / max(1, (int)victim->max_hit);
        effect_level = level * (20 + 5 * dam_ratio) / 100; // adds 5% to effective level per each 1% damage dealt
        effect_level = URANGE(level * 25 / 100, effect_level, level * 90 / 100);

        /* chance of blindness */
        if (!IS_AFFECTED(victim,AFF_BLIND)
        &&  !saves_spell(effect_level, victim, DAM_FIRE, 0, dam_flag))
        {
            Affect af;
            act_p("$c1 ничего не видит из-за дыма, попавшего в глаза!",
                   victim,0,0,TO_ROOM,POS_RESTING);
            act_p("Твои глаза слезятся от попавшего в них дыма... и ты ничего не видишь!",
                   victim,0,0,TO_CHAR,POS_RESTING);
        
            af.bitvector.setTable(&affect_flags);
            af.type         = gsn_fire_breath;
            af.level        = level;
            af.duration     = number_range(0,level/50);
            af.location = APPLY_HITROLL;
            af.modifier = min(-4, -level / 15);
            af.bitvector.setValue(AFF_BLIND);

            affect_to_char(victim,&af);
        }
        
        /* warmth cancels frostbite */
        if (victim->isAffected(gsn_chill_touch))
            checkDispel(level,victim,gsn_chill_touch);
         
        /* getting thirsty */
        if (!victim->is_npc())
            desire_thirst->gain( victim->getPC( ), dam/20 );

        /* let's toast some gear! */
        for (obj = victim->carrying; obj != 0; obj = obj_next)
        {
            obj_next = obj->next_content;

            fire_effect(obj,level,dam,TARGET_OBJ, dam_flag);
        }
        return;
    }

    if (target == TARGET_OBJ)  /* toast an object */
    {
        Object *obj = (Object *) vo;
        Object *t_obj,*n_obj;
        Character *victim = obj->carried_by;        
        int chance;
        const char *msg;

        if ( IS_OBJ_STAT(obj,ITEM_BURN_PROOF ) ||
             IS_OBJ_STAT(obj,ITEM_NOPURGE) ||
             material_is_flagged(obj, MAT_INDESTR) )
             return;
         
        // effect level between 0.25 and 0.9 of spell level
        int dam_ratio, effect_level;
        if (victim)
            dam_ratio = (100 * dam) / max(1, (int)victim->max_hit);
        else
            dam_ratio = max(0, (level - obj->level) * 5);                  
        effect_level = level * (20 + dam_ratio) / 100; // adds 5% to effective level per each 1% damage dealt
        effect_level = URANGE(level * 25 / 100, effect_level, level * 90 / 100);
         
        chance = effect_level / 2;
        if ( IS_OBJ_STAT(obj,ITEM_BLESS) )
            chance -= 5;
        if (material_is_flagged(obj, MAT_TOUGH))
            chance /= 2;

        if  (material_is_flagged( obj, MAT_MELTING ))  {
          chance += 30;
          msg = "%1$^O1 та%1$nет|ют и испаря%1$nется|ются!";
        }
        else
        switch ( obj->item_type )
        {
        default:
            return;
        case ITEM_CONTAINER:
            msg = "%1$^O1 вспыхива%1$nет|ют ярким пламенем и сгора%1$nет|ют!";
            break;
        case ITEM_POTION:
            chance += 25;
            msg = "%1$^O1 закипа%1$nет|ют и взрыва%1$nется|ются!";
            break;
        case ITEM_SCROLL:
        case ITEM_PARCHMENT:
        case ITEM_SPELLBOOK:
        case ITEM_RECIPE:
        case ITEM_TEXTBOOK:
            chance += 50;
            msg = "%1$^O1 треска%1$nется|ются и сгора%1$nет|ют!";
            break;
        case ITEM_STAFF:
            chance += 10;
            msg = "%1$^O1 дым%1$nится|ятся и обуглива%1$nется|ются!";
            break;
        case ITEM_WAND:
            msg = "%1$^O1 искр%1$nится|ятся и шип%1$nит|ят!";
            break;
        case ITEM_FOOD:
            msg = "%1$^O1 зажарива%1$nется|ются и черне%1$nет|ют!";
            break;
        case ITEM_PILL:
            msg = "%1$^O1 та%1$nет|ют!";
            break;
        }

        chance = URANGE(1,chance,50);

        if (number_percent() > chance)
            return;

        show_effect_message(obj, msg);

        if (obj->contains)
        {
            /* dump the contents */

            dreamland->removeOption( DL_SAVE_OBJS );

            for (t_obj = obj->contains; t_obj != 0; t_obj = n_obj)
            {
                n_obj = t_obj->next_content;
                obj_from_obj(t_obj);

                if (obj->in_room != 0)
                    obj_to_room(t_obj,obj->in_room);
                else if (obj->carried_by != 0)
                    obj_to_room(t_obj,obj->carried_by->in_room);
                else {
                    extract_obj(t_obj);
                    continue;
                }

                fire_effect(t_obj,level/2,dam/2,TARGET_OBJ, dam_flag);
                dreamland->removeOption( DL_SAVE_OBJS );
            }

            dreamland->resetOption( DL_SAVE_OBJS );

            if ( obj->in_room != 0 )
                save_items( obj->in_room );
            else if ( obj->carried_by != 0 )
                save_items( obj->carried_by->in_room );
        }

        extract_obj( obj );
        return;
    }
}

void poison_effect(void *vo,short level, int dam, int target, bitstring_t dam_flag )
{
    if (target == TARGET_ROOM)  /* nail objects on the floor */
    {
        Room *room = (Room *) vo;
        Object *obj, *obj_next;

        for (obj = room->contents; obj != 0; obj = obj_next)
        {
            obj_next = obj->next_content;
            poison_effect(obj,level,dam,TARGET_OBJ, dam_flag);
        }
        return;
    }

    if (target == TARGET_CHAR)   /* do the effect on a victim */
    {
        Character *victim = (Character *) vo;
        Object *obj, *obj_next;
        // saves against level*0.9 (worst) to level*0.25 (best)
        int dam_ratio, effect_level;
        dam_ratio = (100 * dam) / max(1, (int)victim->max_hit);
        effect_level = level * (20 + 5 * dam_ratio) / 100; // adds 5% to effective level per each 1% damage dealt
        effect_level = URANGE(level * 25 / 100, effect_level, level * 90 / 100);
        
        /* chance of poisoning */
        if (!saves_spell(effect_level, victim, DAM_POISON, 0, dam_flag))
        {
            Affect af;

            victim->pecho("{1{gТебя начинает тошнить, когда яд проникает в твое тело.{2");
            victim->recho("%1$^C2 начинает тошнить, когда яд проникает в %1$Gего|его|ее тело.", victim);

            af.bitvector.setTable(&affect_flags);
            af.type      = gsn_poison;
            af.level     = level;
            af.duration  = level / 2;
            af.location = APPLY_STR;
            af.modifier  = -1;
            af.bitvector.setValue(AFF_POISON);
            affect_join( victim, &af );
        }

        /* equipment */
        for (obj = victim->carrying; obj != 0; obj = obj_next)
        {
            obj_next = obj->next_content;
            poison_effect(obj,level,dam,TARGET_OBJ, dam_flag);
        }
        return;
    }

    if (target == TARGET_OBJ)  /* do some poisoning */
    {
        Object *obj = (Object *) vo;
        Character *victim = obj->carried_by;        
        int chance;
        const char *msg;

        if ( IS_OBJ_STAT(obj,ITEM_BURN_PROOF ) ||
             IS_OBJ_STAT(obj,ITEM_NOPURGE) ||
             material_is_flagged(obj, MAT_INDESTR) )
             return;
         
        // effect level between 0.25 and 0.9 of spell level
        int dam_ratio, effect_level;
        if (victim)
            dam_ratio = (100 * dam) / max(1, (int)victim->max_hit);
        else
            dam_ratio = max(0, (level - obj->level) * 5);                  
        effect_level = level * (20 + dam_ratio) / 100; // adds 5% to effective level per each 1% damage dealt
        effect_level = URANGE(level * 25 / 100, effect_level, level * 90 / 100);
         
        chance = effect_level / 2;
        if ( IS_OBJ_STAT(obj,ITEM_BLESS) )
            chance -= 5;
        if (material_is_flagged(obj, MAT_TOUGH))
            chance /= 2;

        switch (obj->item_type)
        {
            default:
                return;
            case ITEM_FOOD:
                break;
            case ITEM_DRINK_CON:
                if (obj->value0() == obj->value1())
                    return;
                break;
        }

        chance = URANGE(1,chance,50);

        if (number_percent() > chance)
            return;

        obj->value3(obj->value3() | DRINK_POISONED);
        msg = "Пары смертельного яда проникают в %O4. Кому-то не поздоровится...";        
        show_effect_message(obj, msg);        
        return;
    }
}


void shock_effect(void *vo,short level, int dam, int target, bitstring_t dam_flag )
{
    if (target == TARGET_ROOM)
    {
        Room *room = (Room *) vo;
        Object *obj, *obj_next;

        for (obj = room->contents; obj != 0; obj = obj_next)
        {
            obj_next = obj->next_content;
            shock_effect(obj,level,dam,TARGET_OBJ, dam_flag);
        }
        return;
    }

    if (target == TARGET_CHAR)
    {
        Character *victim = (Character *) vo;
        Object *obj, *obj_next;
        // saves against level*0.9 (worst) to level*0.25 (best)
        int dam_ratio, effect_level;
        dam_ratio = (100 * dam) / max(1, (int)victim->max_hit);
        effect_level = level * (20 + 5 * dam_ratio) / 100; // adds 5% to effective level per each 1% damage dealt
        effect_level = URANGE(level * 25 / 100, effect_level, level * 90 / 100); 
         
        /* daze and confused? */
        if (!saves_spell(effect_level, victim, DAM_LIGHTNING, 0, dam_flag))
        {
            victim->send_to("Твое тело парализовано.\n\r");
            victim->setDaze( URANGE(8, dam_ratio, 20) );
        }

        /* toast some gear */
        for (obj = victim->carrying; obj != 0; obj = obj_next)
        {
            obj_next = obj->next_content;
            shock_effect(obj,level,dam,TARGET_OBJ, dam_flag);
        }
        return;
    }

    if (target == TARGET_OBJ)
    {
        Object *obj = (Object *) vo;
        Character *victim = obj->carried_by;        
        int chance;
        const char *msg;

        if ( IS_OBJ_STAT(obj,ITEM_BURN_PROOF ) ||
             IS_OBJ_STAT(obj,ITEM_NOPURGE) ||
             material_is_flagged(obj, MAT_INDESTR) )
             return;
         
        // effect level between 0.25 and 0.9 of spell level
        int dam_ratio, effect_level;
        if (victim)
            dam_ratio = (100 * dam) / max(1, (int)victim->max_hit);
        else
            dam_ratio = max(0, (level - obj->level) * 5);                  
        effect_level = level * (20 + dam_ratio) / 100; // adds 5% to effective level per each 1% damage dealt
        effect_level = URANGE(level * 25 / 100, effect_level, level * 90 / 100);
         
        chance = effect_level / 2;
        if ( IS_OBJ_STAT(obj,ITEM_BLESS) )
            chance -= 5;
        if (material_is_flagged(obj, MAT_TOUGH))
            chance /= 2;

        switch(obj->item_type)
        {
            default:
                return;
           case ITEM_WAND:
           case ITEM_STAFF:
                chance += 10;
                msg = "%1$^O1 с треском взрыва%1$nется|ются!";
                break;
           case ITEM_JEWELRY:
                chance -= 10;
                msg = "%1$^O1 плав%1$nится|ятся.";
        }
        
        chance = URANGE(1,chance,50);

        if (number_percent() > chance)
            return;

        show_effect_message(obj, msg);

        extract_obj(obj);
        return;
    }
}

void sand_effect(void *vo, short level, int dam, int target, bitstring_t dam_flag )
{
    if ( target == TARGET_ROOM ) /* nail objects on the floor */
    {
        Room *room = (Room *) vo;
        Object *obj, *obj_next;

        for (obj = room->contents; obj != 0; obj = obj_next) {
            obj_next = obj->next_content;
            sand_effect(obj,level,dam,TARGET_OBJ, dam_flag);
        }
        return;
    }

    if ( target == TARGET_CHAR )  /* do the effect on a victim */
    {
        Character *victim = (Character *) vo;
        Object *obj, *obj_next;
        // saves against level*0.9 (worst) to level*0.25 (best)
        int dam_ratio, effect_level;
        dam_ratio = (100 * dam) / max(1, (int)victim->max_hit);
        effect_level = level * (20 + 5 * dam_ratio) / 100; // adds 5% to effective level per each 1% damage dealt
        effect_level = URANGE(level * 25 / 100, effect_level, level * 90 / 100); 

        /* chance of blindness */
        if (!IS_AFFECTED(victim,AFF_BLIND) && !saves_spell(effect_level, victim, DAM_SLASH, 0, dam_flag))
        {
            Affect af;
            victim->recho("%^C1 ничего не видит из-за песка, попавшего в глаза!", victim);
            victim->pecho("Твои глаза слезятся от попавшего в них песка... ты ничего не видишь!");
        
            af.bitvector.setTable(&affect_flags);
            af.type         = gsn_sand_storm;
            af.level        = level;
            af.duration     = number_range(0,level/50);
            af.location = APPLY_HITROLL;
            af.modifier = min(-4, -level / 15);
            af.bitvector.setValue(AFF_BLIND);
            affect_to_char(victim,&af);
        }

        /* let's toast some gear */
        for ( obj = victim->carrying; obj; obj = obj_next )
        {
            obj_next = obj->next_content;
            sand_effect(obj,level,dam,TARGET_OBJ, dam_flag);
        }
        return;
    }

    if ( target == TARGET_OBJ ) /* toast an object */
    {
        Object *obj = (Object *) vo;
        Object *t_obj,*n_obj;
        Character *victim = obj->carried_by;        
        int chance;
        const char *msg;

        if ( IS_OBJ_STAT(obj,ITEM_BURN_PROOF ) ||
             IS_OBJ_STAT(obj,ITEM_NOPURGE) ||
             material_is_flagged(obj, MAT_INDESTR) )
             return;
         
        // effect level between 0.25 and 0.9 of spell level
        int dam_ratio, effect_level;
        if (victim)
            dam_ratio = (100 * dam) / max(1, (int)victim->max_hit);
        else
            dam_ratio = max(0, (level - obj->level) * 5);                  
        effect_level = level * (20 + dam_ratio) / 100; // adds 5% to effective level per each 1% damage dealt
        effect_level = URANGE(level * 25 / 100, effect_level, level * 90 / 100);
         
        chance = effect_level / 2;
        if ( IS_OBJ_STAT(obj,ITEM_BLESS) )
            chance -= 5;
        if (material_is_flagged(obj, MAT_TOUGH))
            chance /= 2;

        switch (obj->item_type)
        {
        default:
            return;
        case ITEM_CONTAINER:
        case ITEM_CORPSE_PC:
        case ITEM_CORPSE_NPC:
            chance += 50;
            msg = "%1$^O1 наполня%1$nется|ются песком и исчеза%1$nет|ют.";
            break;
        case ITEM_ARMOR:
            chance -=10;
            msg = "%1$^O1 царапа%1$nется|ются песком.";
            break;
        case ITEM_CLOTHING:
            msg = "%1$^O1 разрыва%1$nется|ются песком.";
            break;
        case ITEM_WAND:
            chance = 50;
            msg = "%1$^O1 рассыпа%1$nется|ются в пыль.";
            break;
        case ITEM_SCROLL:
        case ITEM_SPELLBOOK:
        case ITEM_RECIPE:
        case ITEM_TEXTBOOK:
            chance += 20;
            msg = "%1$^O1 покрыва%1$nется|ются слоем песка.";
            break;
        case ITEM_POTION:
            chance +=10;
            msg = "%1$^O1 разбива%1$nется|ются на кусочки под ударом песка.";
            break;
        }

        chance = URANGE(1,chance,50);

        if ( number_percent() > chance )
            return;

        show_effect_message(obj, msg);

        if ( obj->item_type == ITEM_ARMOR )  /* etch it */
        {
            int i;
            Affect af;

            af.location = APPLY_AC;
            af.level    = level;
            af.duration = -1;
            af.modifier = 1;
            af.type     = -1;

            affect_enchant( obj );
            affect_enhance( obj, &af );

            if ( obj->carried_by != 0 && obj->wear_loc != wear_none )
                for ( i = 0; i < 4; i++ )
                    obj->carried_by->armor[i] += 1;

            return;
        }

        /* get rid of the object */
        if ( obj->contains )  /* dump contents */
        {
            dreamland->removeOption( DL_SAVE_OBJS );

            for ( t_obj = obj->contains; t_obj != 0; t_obj = n_obj ) 
            {
                n_obj = t_obj->next_content;
                obj_from_obj(t_obj);
                if ( obj->in_room != 0 )
                    obj_to_room(t_obj,obj->in_room);
                else if (obj->carried_by != 0)
                    obj_to_room(t_obj,obj->carried_by->in_room);
                else {
                    extract_obj(t_obj);
                    continue;
                }

                sand_effect(t_obj,level/2,dam/2,TARGET_OBJ, dam_flag);
                dreamland->removeOption( DL_SAVE_OBJS );
            }

            dreamland->resetOption( DL_SAVE_OBJS );

            if ( obj->in_room != 0 )
                save_items( obj->in_room );
            else if ( obj->carried_by != 0 )
                save_items( obj->carried_by->in_room );
        }

        extract_obj(obj);
        return;
    }
}

void scream_effect(void *vo, short level, int dam, int target, bitstring_t dam_flag )
{
    if (target == TARGET_ROOM)  /* nail objects on the floor */
    {
        Room *room = (Room *) vo;
        Object *obj, *obj_next;
        for (obj = room->contents; obj != 0; obj = obj_next) {
            obj_next = obj->next_content;
            scream_effect(obj,level,dam,TARGET_OBJ, dam_flag);
        }
        return;
    }

    if (target == TARGET_CHAR)   /* do the effect on a victim */
    {
        Character *victim = (Character *) vo;
        Object *obj, *obj_next;

        if (victim->fighting) { 
            if (!saves_spell(level, victim, DAM_SOUND, 0, dam_flag )) {
                victim->pecho("Дикий вопль заставляет тебя отвлечься от сражения!");
                victim->recho("Дикий вопль заставляет %C4 отвлечься от сражения!", victim);                     
                stop_fighting( victim , true );
            }
            else {
                victim->pecho("Вопль на мгновение отвлекает тебя, но ты вновь бросаешься в битву.");
                victim->recho("Вопль на мгновение отвлекает %1$C4 от сражения, но %1$P1 вновь бросается в битву.", victim); 
            }
        }
         
        // saves against level*0.9 (worst) to level*0.25 (best)
        int dam_ratio, effect_level;
        dam_ratio = (100 * dam) / max(1, (int)victim->max_hit);
        effect_level = level * (20 + 5 * dam_ratio) / 100; // adds 5% to effective level per each 1% damage dealt
        effect_level = URANGE(level * 25 / 100, effect_level, level * 90 / 100);
        
        if (!saves_spell(effect_level, victim, DAM_SOUND, 0, dam_flag))
        {
            Affect af;
            victim->pecho("{1{MТы ничего не слышишь!{2\n\r");
            victim->recho("%^C1 теперь ничего не слышит.", victim);  
        
            af.bitvector.setTable(&affect_flags);
            af.type         = gsn_scream;
            af.level        = level;
            af.duration     = number_range(0,1);
            af.modifier     = 0;
            af.bitvector.setValue(AFF_SCREAM);

            affect_to_char(victim,&af);
                        
            /* daze and confused? */
            if (number_percent() < 50) {
                victim->pecho("{1{WТы ошеломленно трясешь головой!{2");
                victim->recho("%^C1 теперь ошеломленно трясет головой.", victim);                
                victim->setDaze( URANGE(8, dam_ratio, 20) );
            }
        }

        /* getting thirsty */
        if (!victim->is_npc())
            desire_thirst->gain( victim->getPC( ), dam/20 );

        /* let's toast some gear! */
        for (obj = victim->carrying; obj != 0; obj = obj_next)
        {
            obj_next = obj->next_content;
            scream_effect(obj,level,dam,TARGET_OBJ, dam_flag);
        }
        return;
    }

    if (target == TARGET_OBJ)  /* toast an object */
    {
        Object *obj = (Object *) vo;
        Object *t_obj,*n_obj;
        Character *victim = obj->carried_by;        
        int chance;
        const char *msg;

        if ( IS_OBJ_STAT(obj,ITEM_BURN_PROOF ) ||
             IS_OBJ_STAT(obj,ITEM_NOPURGE) ||
             material_is_flagged(obj, MAT_INDESTR) )
             return;
         
        // effect level between 0.25 and 0.9 of spell level
        int dam_ratio, effect_level;
        if (victim)
            dam_ratio = (100 * dam) / max(1, (int)victim->max_hit);
        else
            dam_ratio = max(0, (level - obj->level) * 5);                  
        effect_level = level * (20 + dam_ratio) / 100; // adds 5% to effective level per each 1% damage dealt
        effect_level = URANGE(level * 25 / 100, effect_level, level * 90 / 100);
         
        chance = effect_level / 2;
        if ( IS_OBJ_STAT(obj,ITEM_BLESS) )
            chance -= 5;
        if (material_is_flagged(obj, MAT_TOUGH))
            chance /= 2;

        if (material_is_flagged( obj, MAT_MELTING )) {
            chance *= 2;
            msg = "%1$^O1 разбива%1$nется|ются и испаря%1$nется|ются!";
        }
        else if (material_is_flagged( obj, MAT_FRAGILE )) {
            chance *= 2;
            msg = "%1$^O1 разлета%1$nется|ются на множество мелких осколков";
        }
        else
        switch ( obj->item_type )
        {
        default:
            return;
        case ITEM_POTION:
            chance += 25;
            msg = "%1$^O1 разбива%1$nется|ются и содержимое выливается на землю!";
            break;
        case ITEM_SCROLL:
        case ITEM_SPELLBOOK:
        case ITEM_RECIPE:
        case ITEM_TEXTBOOK:
            chance += 50;
            msg = "%1$^O1 разрыва%1$nется|ются на клочки!";
            break;
        case ITEM_DRINK_CON:
            msg = "%1$^O1 разбива%1$nется|ются и содержимое выливается на землю!";
            chance += 5;
            break;
        case ITEM_PILL:
            msg = "%1$^O1 разлета%1$nется|ются на мелкие кусочки!";
            break;
        }

        chance = URANGE(1,chance,50);

        if (number_percent() > chance)
            return;

        show_effect_message(obj, msg);

        if (obj->contains)
        {
            /* dump the contents */
            dreamland->removeOption( DL_SAVE_OBJS );

            for (t_obj = obj->contains; t_obj != 0; t_obj = n_obj)
            {
                n_obj = t_obj->next_content;
                obj_from_obj(t_obj);
                if (obj->in_room != 0)
                    obj_to_room(t_obj,obj->in_room);
                else if (obj->carried_by != 0)
                    obj_to_room(t_obj,obj->carried_by->in_room);
                else {
                    extract_obj(t_obj);
                    continue;
                }

                scream_effect(t_obj,level/2,dam/2,TARGET_OBJ, dam_flag);
                dreamland->removeOption( DL_SAVE_OBJS );
            }

            dreamland->resetOption( DL_SAVE_OBJS );

            if ( obj->in_room != 0 )
                save_items( obj->in_room );
            else if ( obj->carried_by != 0 )
                save_items( obj->carried_by->in_room );
            }
 
        extract_obj( obj );
        return;
    }
}
