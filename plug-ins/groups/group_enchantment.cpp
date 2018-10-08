
/* $Id: group_enchantment.cpp,v 1.1.2.28.6.14 2010-09-01 21:20:45 rufina Exp $
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
#include "group_enchantment.h"
#include "xmlattributerestring.h"
#include "profflags.h"

#include "so.h"

#include "pcharacter.h"
#include "room.h"
#include "object.h"
#include "affect.h"
#include "magic.h"
#include "fight.h"
#include "damage_impl.h"
#include "act_move.h"
#include "gsn_plugin.h"

#include "merc.h"
#include "mercdb.h"
#include "handler.h"
#include "act.h"
#include "def.h"

GSN(inspiration);

SPELL_DECL(BlessWeapon);
VOID_SPELL(BlessWeapon)::run( Character *ch, Object *obj, int sn, int level ) 
{ 
    Affect af;

    if (obj->item_type != ITEM_WEAPON) {
        ch->send_to("Это не оружие.\n\r");
        return;
    }

    if ( IS_WEAPON_STAT(obj,WEAPON_VAMPIRIC)
        ||  IS_OBJ_STAT(obj,ITEM_DARK)
        ||  IS_OBJ_STAT(obj,ITEM_EVIL) )
    {
        ch->pecho("Дьявольская сущность %1$O2 отвергает твое благословление.", obj);
        return;
    }
    
    if (IS_WEAPON_STAT(obj,WEAPON_HOLY)) {
        ch->pecho("%1$^O1 уже благословле%1$Gно|н|на для священной битвы.", obj);
        return;
    }
        
    af.type         = sn;
    af.level         = level / 2;
    af.duration         = level / 8;

    af.where         = TO_WEAPON;
    af.location         = 0;
    af.modifier         = 0;
    af.bitvector = WEAPON_HOLY;
    affect_to_obj( obj, &af);

    af.where     = TO_OBJECT;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = ITEM_ANTI_EVIL|ITEM_ANTI_NEUTRAL;
    affect_to_obj( obj, &af );

    ch->pecho("Ты благословляешь %1$O4 для священной битвы.", obj);

}

SPELL_DECL(EnchantArmor);
VOID_SPELL(EnchantArmor)::run( Character *ch, Object *obj, int sn, int level ) 
{ 
    Affect *paf;
    Affect af;
    int result, fail;
    int ac_bonus, add_ac;
    bool inspire;

    if (obj->item_type != ITEM_ARMOR)
    {
        ch->send_to("Это не доспехи.\n\r");
        return;
    }

    if (obj->wear_loc != wear_none)
    {
        ch->send_to("Вещь должна находиться в списке инвентаря.\n\r");
        return;
    }

    if (IS_OBJ_STAT(obj, ITEM_NOENCHANT)
        || (obj->behavior && obj->behavior->isLevelAdaptive( )))
    {
        ch->send_to( "Эта вещь не подлежит улучшению.\r\n" );
        return;
    }

    inspire = ch->isAffected( gsn_inspiration );
    /* this means they have no bonus */
    ac_bonus = 0;
    fail = 25;        /* base 25% chance of failure */

    /* find the bonuses */

    if (!obj->enchanted)
        for ( paf = obj->pIndexData->affected; paf != 0; paf = paf->next )
        {
            if ( paf->location == APPLY_AC )
            {
                    ac_bonus = paf->modifier;
                    fail += 5 * (ac_bonus * ac_bonus);
             }

            else  /* things get a little harder */
                    fail += 20;
            }

    for ( paf = obj->affected; paf != 0; paf = paf->next )
    {
        if ( paf->location == APPLY_AC )
          {
            ac_bonus = paf->modifier;
            fail += 5 * (ac_bonus * ac_bonus);
        }

        else /* things get a little harder */
            fail += 20;
    }

    /* apply other modifiers */
    fail -= level;

    if (IS_OBJ_STAT(obj,ITEM_BLESS))
        fail -= 15;
    if (IS_OBJ_STAT(obj,ITEM_GLOW))
        fail -= 5;

    fail = URANGE(5,fail,85);
    result = number_percent();
    
    if (inspire) 
        result *= 2;

    /* the moment of truth */
    if (result < (fail / 5))  /* item destroyed */
    {
        act("$o1 ярко вспыхивает... и испаряется!", ch,obj,0,TO_ALL);
        extract_obj(obj);
        return;
    }

    if (result < (fail / 3)) /* item disenchanted */
    {
        Affect *paf_next;

        act("$o1 на миг ярко вспыхивает... но затем угасает.", ch,obj,0,TO_ALL);
        obj->enchanted = true;

        /* remove all affects */
        for (paf = obj->affected; paf != 0; paf = paf_next)
        {
            paf_next = paf->next;
            ddeallocate( paf );
        }

        obj->affected = 0;

        /* clear all flags */
        obj->extra_flags = obj->pIndexData->extra_flags;
        return;
    }

    if ( result <= fail )  /* failed, no bad result */
    {
        ch->send_to("Ничего не произошло.\n\r");
        return;
    }
    
    if (inspire) {
        act( "$o1 на мгновение отражает свет далеких звезд..", ch, obj, 0, TO_ALL );
        SET_BIT(obj->extra_flags,ITEM_GLOW);
        add_ac = - number_range( 3, 5 );
    }
    else if (result <= (90 - level/5))  /* success! */
    {
        act("Золотая аура окружает $o4.",ch,obj,0,TO_ALL);
        add_ac = -1;
    }
    else  /* exceptional enchant */
    {
        act("$o1 вспыхивает бриллиантово-золотым светом!", ch,obj,0,TO_ALL);
        SET_BIT(obj->extra_flags,ITEM_GLOW);
        add_ac = -2;
    }

    if (ch->getTrueProfession( )->getFlags( ch ).isSet(PROF_MAGIC))
        SET_BIT(obj->extra_flags,ITEM_MAGIC);
                
    /* now add the enchantments */

    if (obj->level < LEVEL_HERO)
        obj->level = min(LEVEL_HERO - 1,obj->level + 1);
    
    affect_enchant( obj );

    af.where     = TO_OBJECT;
    af.bitvector = 0;
    af.type      = sn;
    af.level     = level;

    af.duration  = -1;
    af.location  = APPLY_AC;
    af.modifier  = add_ac;
    affect_enhance( obj, &af );
    
    if (inspire) {
        int add_hr, add_dr;
        
        add_hr = number_range( 0, obj->level / 50 );
        add_dr = number_range( 0, obj->level / 50 );

        if (add_hr > 0) {
            af.duration  = 200;
            af.location  = APPLY_HITROLL;
            af.modifier  = add_hr;

            affect_enhance( obj, &af );
        }

        if (add_dr > 0) {
            af.duration  = 200;
            af.location  = APPLY_DAMROLL;
            af.modifier  = add_dr;

            affect_enhance( obj, &af );
        }
    }
}


SPELL_DECL(EnchantWeapon);
VOID_SPELL(EnchantWeapon)::run( Character *ch, Object *obj, int sn, int level ) 
{ 
    Affect *paf;
    Affect af;
    int result, fail;
    int hit_bonus, dam_bonus, added;

    if (obj->item_type != ITEM_WEAPON)
    {
        ch->send_to("Это не оружие.\n\r");
        return;
    }

    if (obj->wear_loc != wear_none)
    {
        ch->send_to("Вещь должна находиться в списке инвентаря.\n\r");
        return;
    }

    if (IS_OBJ_STAT(obj, ITEM_NOENCHANT)
        || (obj->behavior && obj->behavior->isLevelAdaptive( )))
    {
        ch->send_to( "Это оружие не подлежит улучшению.\r\n" );
        return;
    }
    
    /* this means they have no bonus */
    hit_bonus = 0;
    dam_bonus = 0;
    fail = 25;        /* base 25% chance of failure */

    /* find the bonuses */

    if (!obj->enchanted)
            for ( paf = obj->pIndexData->affected; paf != 0; paf = paf->next )
            {
            if ( paf->location == APPLY_HITROLL )
            {
                    hit_bonus = paf->modifier;
                    fail += 2 * (hit_bonus * hit_bonus);
             }

            else if (paf->location == APPLY_DAMROLL )
            {
                    dam_bonus = paf->modifier;
                    fail += 2 * (dam_bonus * dam_bonus);
            }

            else  /* things get a little harder */
                    fail += 25;
            }

    for ( paf = obj->affected; paf != 0; paf = paf->next )
    {
        if ( paf->location == APPLY_HITROLL )
          {
            hit_bonus = paf->modifier;
            fail += 2 * (hit_bonus * hit_bonus);
        }

        else if (paf->location == APPLY_DAMROLL )
          {
            dam_bonus = paf->modifier;
            fail += 2 * (dam_bonus * dam_bonus);
        }

        else /* things get a little harder */
            fail += 25;
    }

    /* apply other modifiers */
    fail -= 3 * level/2;

    if (IS_OBJ_STAT(obj,ITEM_BLESS))
        fail -= 15;
    if (IS_OBJ_STAT(obj,ITEM_GLOW))
        fail -= 5;

    fail = URANGE(5,fail,95);
    result = number_percent();

    if (ch->isAffected( gsn_inspiration )) 
        result += result / 4;

    /* the moment of truth */
    if (result < (fail / 5))  /* item destroyed */
    {
        act("$o1 сильно вздрагивает... и взрывается!", ch,obj,0,TO_ALL);
        extract_obj(obj);
        return;
    }


   if (result < (fail / 2)) /* item disenchanted */
    {
        Affect *paf_next;

        act("$o1 на миг ярко вспыхивает... но затем угасает.", ch,obj,0,TO_ALL);
        obj->enchanted = true;

        /* remove all affects */
        for (paf = obj->affected; paf != 0; paf = paf_next)
        {
            paf_next = paf->next;
            ddeallocate( paf );
        }
        obj->affected = 0;

        /* clear all flags */
        obj->extra_flags = obj->pIndexData->extra_flags;
        return;
    }

    if ( result <= fail )  /* failed, no bad result */
    {
        ch->send_to("Ничего не произошло.\n\r");
        return;
    }
    
    if (ch->isAffected( gsn_inspiration )) {
        act( "$o1 на мгновение отражает свет далеких звезд..", ch, obj, 0, TO_ALL );
        SET_BIT(obj->extra_flags,ITEM_GLOW);
        added = number_range( 1, 3 );
    }
    else if (result <= (100 - level/5))  /* success! */
    {
        act("Голубая аура окружает $o4.",ch,obj,0,TO_ALL);
        added = 1;
    }
    else  /* exceptional enchant */
    {
        act("$o1 загорается бриллиантово-голубым светом!", ch,obj,0,TO_ALL);
        SET_BIT(obj->extra_flags,ITEM_GLOW);
        added = 2;
    }
                
    if (ch->getTrueProfession( )->getFlags( ch ).isSet(PROF_MAGIC))
        SET_BIT(obj->extra_flags,ITEM_MAGIC);

    /* now add the enchantments */

    if (obj->level < LEVEL_HERO - 1)
        obj->level = min(LEVEL_HERO - 1,obj->level + 1);
    
    affect_enchant( obj );

    af.where     = TO_OBJECT;
    af.bitvector = 0;
    af.type      = sn;
    af.level     = level;
    af.duration  = -1;
    af.modifier  = added;

    af.location  = APPLY_DAMROLL;
    affect_enhance( obj, &af );

    af.location  = APPLY_HITROLL;
    affect_enhance( obj, &af );
}

SPELL_DECL(Fireproof);
VOID_SPELL(Fireproof)::run( Character *ch, Object *obj, int sn, int level ) 
{ 
    Affect af;
    if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
    {
        ch->pecho("%1$^O1 уже защище%1$Gно|н|на от огня.", obj);
        return;
    }

    af.where     = TO_OBJECT;
    af.type      = sn;
    af.level     = level;
    af.duration  = number_fuzzy(level / 4);
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = ITEM_BURN_PROOF;
    affect_to_obj( obj, &af);

    act("Защитная аура окружает $o4.",ch,obj,0,TO_ALL);
}


SPELL_DECL(FlameOfGod);
VOID_SPELL(FlameOfGod)::run( Character *ch, Object *obj, int sn, int level ) 
{ 
    Affect af;
    int chance;

    if (obj->item_type != ITEM_WEAPON) {
        ch->send_to( "Это не оружие.\r\n" );
        return;
    }
    
    if (obj->isAffected(sn )) {
        act_p("Священный огонь уже пылает в $o6.", ch, obj, 0, TO_CHAR, POS_RESTING);
        return;
    }
    
    if ( IS_WEAPON_STAT(obj,WEAPON_VAMPIRIC)
        ||  IS_OBJ_STAT(obj,ITEM_DARK|ITEM_EVIL)
        ||  IS_OBJ_STAT(obj,ITEM_ANTI_GOOD) )
    {
        act_p("Ты не можешь зажечь священный огонь в $o6.",ch,obj,0,TO_CHAR,POS_RESTING);
        return;
    }
    
    chance = level - obj->level + ch->getSkill( sn );
    chance -= number_percent( );

    if (chance < -10) {
        act_p("Ты прогнева$gло|л|ла своего Бога и он уничтожил оружие.", ch, 0, 0, TO_CHAR, POS_RESTING);
        extract_obj( obj );
        return;
    }

    if (chance < 0) {
        ch->send_to( "Ничего не произошло.\r\n" );
        return;
    }

    af.level     = level;
    af.duration  = level / 4;

    af.where     = TO_WEAPON;
    af.type      = number_bits(2) ? gsn_flamestrike : gsn_fireball;
    af.location  = 0;
    af.modifier  = 10;
    af.bitvector = WEAPON_SPELL;
    affect_to_obj( obj, &af );
        
    af.where     = TO_OBJECT;
    af.type      = sn;
    af.location  = APPLY_NONE;
    af.modifier  = 0;
    af.bitvector = ITEM_ANTI_EVIL|ITEM_ANTI_NEUTRAL;
    affect_to_obj( obj, &af );

    act_p("Ты взываешь к Богам и $o1 загорается священным огнем!", ch, obj, 0, TO_CHAR, POS_RESTING);
    act_p("$c1 взывает к Богам и $o1 загорается священным огнем!", ch, obj, 0, TO_ROOM, POS_RESTING);
}


SPELL_DECL(HungerWeapon);
VOID_SPELL(HungerWeapon)::run( Character *ch, Object *obj, int sn, int level ) 
{ 
    Affect af;
    int chance;

    if (obj->pIndexData->item_type != ITEM_WEAPON) {
        ch->send_to("Это не оружие.\r\n");
        return;
    } 

    if (IS_WEAPON_STAT(obj, WEAPON_HOLY)
        ||  IS_OBJ_STAT(obj, ITEM_BLESS)
        ||  IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)) 
    {
            act_p("Боги в гневе!", ch, 0, 0, TO_ALL, POS_RESTING);
            rawdamage(ch, ch, DAM_HOLY, 
                    (ch->hit - 1) > 1000 ? 1000 : (ch->hit - 1), true );
            return;
    } 

    if (IS_WEAPON_STAT(obj, WEAPON_VAMPIRIC)) {
        act_p("$o1 уже жаждет чужой жизни.", ch, obj, 0, TO_CHAR, POS_RESTING );
        return;
    }

    chance = ch->getSkill( sn );        

    if (IS_WEAPON_STAT(obj, WEAPON_FLAMING))        chance /= 2;
    if (IS_WEAPON_STAT(obj, WEAPON_FROST))        chance /= 2;
    if (IS_WEAPON_STAT(obj, WEAPON_SHARP))        chance /= 2;
    if (IS_WEAPON_STAT(obj, WEAPON_VORPAL))        chance /= 2;
    if (IS_WEAPON_STAT(obj, WEAPON_SHOCKING))        chance /= 2;
    if (IS_WEAPON_STAT(obj, WEAPON_FADING))        chance /= 2;
     
    if (number_percent() < chance) {    
        af.where        = TO_WEAPON;
        af.type         = sn;
        af.level        = level / 2;
        af.duration        = level / 4;
        af.location        = 0;
        af.modifier        = 0;
        af.bitvector        = WEAPON_VAMPIRIC;
        affect_to_obj( obj, &af);
        
        af.where     = TO_OBJECT;
        af.location  = APPLY_NONE;
        af.modifier  = 0;
        af.bitvector = ITEM_ANTI_GOOD|ITEM_ANTI_NEUTRAL;
        affect_to_obj( obj, &af );
        
        act_p("Ты передаешь $o3 свою жажду чужой жизни...", ch, obj, 0, TO_CHAR, POS_RESTING);
        act_p("$c1 внимательно смотрит на $o4, е$gго|го|е глаза вспыхивают {rкрасным{x", ch, obj, 0, TO_ROOM, POS_RESTING);
    } 
    else 
        act_p("Неудача.", ch, obj, 0, TO_CHAR, POS_RESTING);

}


SPELL_DECL(Mend);
VOID_SPELL(Mend)::run( Character *ch, Object *obj, int sn, int level ) 
{ 
    int result,skill;

    if ( obj->condition > 99 )
    {
    ch->send_to("Эта вещь не нуждается в ремонте.\n\r");
    return;
    }

    if (obj->wear_loc != wear_none)
    {
        ch->send_to("Для ремонта вещь должна находиться в списке инвентаря.\n\r");
        return;
    }

    skill = gsn_mend->getEffective( ch ) / 2;
    result = number_percent ( ) + skill;

    if (IS_OBJ_STAT(obj,ITEM_GLOW))
          result -= 5;
    if (IS_OBJ_STAT(obj,ITEM_MAGIC))
          result += 5;

    if (ch->isAffected( gsn_inspiration )) {
        ch->pecho( "%1$^O1 по%1$nет|ют под твоими руками, обретая первозданный вид.", obj );
        ch->recho( "%1$^O1 по%1$nет|ют под руками %2$C2, обретая первозданный вид.", obj, ch );
        obj->condition = 100;
    }
    else if (result >= 50)
    {
        ch->pecho( "%1$^O1 загора%1$nется|ются ярким светом, обретая первозданный вид.", obj );
        ch->recho( "%1$^O1 загора%1$nется|ются ярким светом, обретая первозданный вид.", obj );
        obj->condition += result;
        obj->condition = min( obj->condition , 100 );
    }
    else if ( result >=10)
    {
        ch->send_to("Ничего не произошло.\n\r");
    }
    else
    {
        ch->pecho( "%1$^O1 ярко вспыхива%1$nет|ют... и испаря%1$nется|ются!", obj );
        ch->recho( "%1$^O1 ярко вспыхива%1$nет|ют... и испаря%1$nется|ются!", obj );
        extract_obj(obj);
    }
}


SPELL_DECL(Recharge);
VOID_SPELL(Recharge)::run( Character *ch, Object *obj, int sn, int level ) 
{ 
    int chance, percent;

    if (obj->item_type != ITEM_WAND && obj->item_type != ITEM_STAFF)
    {
        ch->println("Ты можешь восстановить заклинания только у волшебных палочек и посохов.");
        return;
    }

    if (obj->value[0] >= 3 * level / 2)
    {
        ch->send_to("Тебе не хватает могущества для восстановления этих заклинаний.\n\r");
        return;
    }

    if (obj->value[1] == 0)
    {
        ch->send_to("Эти заклинания больше не могут быть восстановлены.\n\r");
        return;
    }

    chance = 40 + 2 * level;

    chance -= obj->value[0]; /* harder to do high-level spells */
    chance -= (obj->value[1] - obj->value[2]) *
              (obj->value[1] - obj->value[2]);

    chance = max(level/2,chance);

    percent = number_percent();

    if (percent < chance / 2)
    {
        act_p("$o1 слабо вспыхивает.",ch,obj,0,TO_CHAR,POS_RESTING);
        act_p("$o1 слабо вспыхивает.",ch,obj,0,TO_ROOM,POS_RESTING);
        obj->value[2] = max(obj->value[1],obj->value[2]);
        obj->value[1] = 0;
        return;
    }

    else if (percent <= chance)
    {
        int chargeback,chargemax;

        act_p("$o1 слабо вспыхивает.",ch,obj,0,TO_CHAR,POS_RESTING);
        act_p("$o1 слабо вспыхивает.",ch,obj,0,TO_CHAR,POS_RESTING);

        chargemax = obj->value[1] - obj->value[2];

        if (chargemax > 0)
            chargeback = max(1,chargemax * percent / 100);
        else
            chargeback = 0;

        obj->value[2] += chargeback;
        obj->value[1] = 0;
        return;
    }

    else if (percent <= min(95, 3 * chance / 2))
    {
        ch->send_to("Ничего не произошло.\n\r");
        if (obj->value[1] > 1)
            obj->value[1]--;
        return;
    }

    else /* whoops! */
    {
        act_p("$o1 ярко вспыхивает и взрывается!",ch,obj,0,TO_CHAR,POS_RESTING);
        act_p("$o1 ярко вспыхивает и взрывается!",ch,obj,0,TO_ROOM,POS_RESTING);
        extract_obj(obj);
    }

}


SPELL_DECL(WeaponMorph);
VOID_SPELL(WeaponMorph)::run( Character *ch, char *target_name, int sn, int level ) 
{ 
    Object *obj;
    int result, fail;
    DLString args( target_name ), arg1, arg2;

    arg1 = args.getOneArgument( );
    arg2 = args.getOneArgument( );
    
    if (!( obj = get_obj_carry( ch, arg1.c_str( ) ) )) {
        ch->println( "У тебя нет этого." );
        return;
    }

    if (obj->item_type != ITEM_WEAPON) {
        ch->println("Ты можешь создать булаву только из оружия.");
        return;
    }

    if (obj->value[0] == WEAPON_MACE
            || obj->value[0] == WEAPON_ARROW
            || IS_WEAPON_STAT(obj,WEAPON_TWO_HANDS))
    {
        ch->println("Ты не можешь создать булаву из этого оружия.");
        return;
    }

    if (obj->pIndexData->limit != -1) {
        ch->println("Эта вещь - уникальна. Ты ничего не можешь сделать.");
        return;
    }

    fail = 70;
    fail -=level / 3;

    if (IS_OBJ_STAT(obj,ITEM_BLESS))
            fail -= 15;

    if (IS_OBJ_STAT(obj,ITEM_GLOW))
            fail -= 5;

    fail = URANGE(5,fail,95);
    result = number_percent();

    if (result < (fail / 3))  /* item destroyed */
    {
        obj->getRoom( )->echo( POS_RESTING, 
            "{W%1$^O1 ярко вспыхива%1$nет|ют... и испаря%1$nется|ются!{x", obj );
        return;
    }

    obj->value[0] = WEAPON_MACE;
    obj->value[3] = DAMW_POUND;
    obj->getRoom( )->echo( POS_RESTING, 
        "{W%1$^O1 окружа%1$nется|ются {Rярко-красной аурой{W и приобрета%1$nет|ют новую форму.{x", obj );

    obj->setName( "mace булава" );
    obj->setShortDescr( fmt( 0, "Больш|ая|ой|ой|ую|ой|ой булав|а|ы|е|у|ой|е %C2", ch ).c_str( ) );
    obj->setDescription( fmt( 0, "Большая булава (mace) создана %C5 и забыта здесь.", ch ).c_str( ) );
    dress_created_item( sn, obj, ch, arg2 );
}


SPELL_DECL(WintersTouch);
VOID_SPELL(WintersTouch)::run( Character *ch, Object *obj, int sn, int level ) 
{ 
    Affect af;

    if (obj->item_type != ITEM_WEAPON) {
        act_p("$o1 - не оружие.", ch, obj, 0, TO_CHAR, POS_RESTING);
        return;
    }

    if (IS_WEAPON_STAT(obj,WEAPON_FLAMING) || IS_WEAPON_STAT(obj,WEAPON_SHOCKING)) {
        act_p("С $o5, кажется, ничего не происходит.", ch, obj, 0, TO_CHAR, POS_RESTING);
        return;
    }

    if (IS_WEAPON_STAT(obj,WEAPON_FROST)) {
        act_p("$o1 и без того отмороженное.", ch, obj, 0, TO_CHAR, POS_RESTING);
        return;
    }

    af.where        = TO_WEAPON;
    af.type         = sn;
    af.level        = level / 2;
    af.duration     = level / 4;
    af.location     = 0;
    af.modifier     = 0;
    af.bitvector    = WEAPON_FROST;
    affect_to_obj( obj, &af );
    
    act_p("Ты отдаешь $o4 во власть холода.", ch, obj, 0, TO_CHAR, POS_RESTING);

}
