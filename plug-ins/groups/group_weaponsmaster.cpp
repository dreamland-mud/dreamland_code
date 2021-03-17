
/* $Id: group_weaponsmaster.cpp,v 1.1.2.21.6.12 2009/09/11 11:24:54 rufina Exp $
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

#include "objthrow.h"
#include "skill.h"
#include "skillcommandtemplate.h"
#include "skillmanager.h"

#include "affect.h"
#include "pcharacter.h"
#include "race.h"
#include "npcharacter.h"
#include "object.h"
#include "gsn_plugin.h"
#include "act_move.h"
#include "mercdb.h"

#include "magic.h"
#include "skill_utils.h"
#include "damage.h"
#include "material.h"
#include "fight.h"
#include "vnum.h"
#include "stats_apply.h"
#include "merc.h"
#include "mercdb.h"
#include "handler.h"
#include "act.h"
#include "interp.h"
#include "def.h"

RELIG(godiva);

/*
 * 'second wield' skill command
 */

SKILL_RUNP( second )
{
    Object *obj;

    if (argument[0] == '\0') {
        ch->println( "Вооружиться чем?" );
        return;
    }

    if (( obj = get_obj_carry (ch, argument) ) == 0) {
        ch->println( "У тебя нет этого." );
        return;
    }

    if (!obj->can_wear( ITEM_WIELD )) {
        ch->println( "Ты не можешь вооружиться этим как вторичным оружием." );
        return;
    }
    
    wear_second_wield->wear( obj, F_WEAR_REPLACE | F_WEAR_VERBOSE );
}

/*
 * Disarm a creature.
 * Caller must check for successful attack.
 */
void disarm( Character *ch, Character *victim ,int disarm_second)
{
        Object *obj;
        Object *obj2;
        int skill;

        if (disarm_second)
        {
                if ((obj=get_eq_char(victim,wear_second_wield)) == 0)
                {
                        bug("Disarm second with 0 wear_second_wield",0);
                        return;
                }
        }
        else
        {
                if ((obj=get_eq_char(victim,wear_wield)) == 0)
                {
                        bug("Disarm first with 0 wear_wield",0);
                        return;
                }
        }

        if ( IS_OBJ_STAT(obj,ITEM_NOREMOVE))
        {
                act("$S оружие не двигается с места!",ch,0,victim,TO_CHAR);
                act_p("$c1 пытается обезоружить тебя, но оружие не двигается с места!",
                        ch,0,victim,TO_VICT,POS_RESTING);
                act_p("$c1 пытается обезоружить $C4, но оружие не двигается с места.",
                        ch,0,victim,TO_NOTVICT,POS_RESTING);
                return;
        }

        if (( skill = gsn_grip->getEffective( victim ) ) > 1)
        {
                skill += (victim->getCurrStat(STAT_STR) - ch->getCurrStat(STAT_STR)) * 5;

                if ( number_percent() < skill )
                {
                        act_p("$C1 хватает тебя за руку и ускользает!",
                                ch,0,victim,TO_CHAR,POS_RESTING);
                        act_p("$c1 пытается обезоружить тебя, но ты хватаешь $s за руку и ускользаешь!",
                                ch,0,victim,TO_VICT,POS_RESTING);
                        act_p("$c1 пытается обезоружить $C4, но безуспешно.",
                                ch,0,victim,TO_NOTVICT,POS_RESTING);
                        gsn_grip->improve( victim, true, ch );
                        return;
                }
                else         
                        gsn_grip->improve( victim, false, ch );
        }

        act("Ты обезоруживаешь $C4!", ch,0, victim, TO_CHAR);
        act( "$c1 обезоруживает $C4!",ch, 0, victim,TO_NOTVICT);

        obj_from_char( obj );

        if ( IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_INVENTORY) ) 
        {
                   act("{R$c1 ВЫБИ$gЛО{x|Л{x|ЛА у тебя оружие!{x", ch, 0, victim, TO_VICT );
                obj_to_char( obj, victim );
        }
        else
        {
                   act("{R$c1 ВЫБИ$gЛО{x|Л{x|ЛА у тебя оружие, и оно упало на землю!{x", ch, 0, victim, TO_VICT );
                obj_to_room( obj, victim->in_room );
                if (victim->is_npc() && victim->wait == 0 && victim->can_see(obj))
                        do_get_raw(victim, obj);
        }

        if ( (obj2 = get_eq_char(victim, wear_second_wield)) != 0)
        {
                act_p( "Ты вооружаешься вторичным оружием как основным!",
                        ch, 0, victim,TO_VICT,POS_RESTING);
                act_p( "$C1 вооружается вторичным оружием как основным!",
                        ch, 0,victim,TO_CHAR ,POS_RESTING);
                act_p( "$C1 вооружается вторичным оружием как основным!",
                        ch, 0, victim,TO_NOTVICT ,POS_RESTING);
                unequip_char( victim, obj2);
                equip_char( victim, obj2 , wear_wield);
        }

        return;
}

/*
 * 'disarm' skill command
 */

SKILL_RUNP( disarm )
{
        Character *victim;
        Object *obj;
        int chance,hth,ch_weapon,vict_weapon,ch_vict_weapon,disarm_second=0;
        char arg[MAX_INPUT_LENGTH];

        if ( MOUNTED(ch) )
        {
                ch->send_to("Ты не сможешь обезоружить, если ты верхом!\n\r");
                return;
        }

        hth = 0;

        if ( (chance = gsn_disarm->getEffective( ch )) <= 1)
        {
                ch->send_to("Ты не знаешь как обезоружить противника.\n\r");
                return;
        }

        if ( !ch->is_npc() && !ch->move )
        {
                act("Ты слишком уста$gло|л|ла для этого.", ch, 0, 0, TO_CHAR);
                return;
        }
        else
                ch->move -= move_dec( ch );

        if (SHADOW(ch))
        {
                ch->send_to("Да... Как глупо пытаться разоружить свою тень.\n\r");
                act_p("$c1 пытается выбить у своей тени оружие.\n...как глупо это выглядит.",
                        ch, 0, 0, TO_ROOM,POS_RESTING);
                return;
        }

        if ( ( victim = ch->fighting ) == 0 )
        {
                ch->send_to("Сейчас ты не сражаешься.\n\r");
                return;
        }

        if (victim->getReligion() == god_godiva && get_eq_char(victim, wear_tattoo)) {
            ch->pecho("{DПризрачная аура{x вокруг %1$C2 мешает тебе обезоружить %1$P2.", victim);
            victim->pecho("%^C1 пытается обезоружить тебя, но не может пробиться сквозь {Dпризрачную ауру{x.", ch);
            return;
        }

        if ( ( obj = get_eq_char( victim, wear_wield ) ) == 0 )
        {
                ch->send_to("Твой противник не вооружен.\n\r");
                if(IS_CHARMED(ch) && ch->master->getPC())
                ch->master->println("Твой противник не вооружен.");
                return;
        }

        argument = one_argument(argument,arg);
        if ( !ch->is_npc() && arg[0] != '\0' )
        {
                if (is_name(arg,"second") )
                        disarm_second = 1;
                else
                        disarm_second = 0;
        }

        /* find weapon skills */
        ch_weapon = ch->getSkill(get_weapon_sn(ch, false));

        vict_weapon = victim->getSkill(get_weapon_sn(victim, disarm_second));
        ch_vict_weapon = ch->getSkill(get_weapon_sn(victim, disarm_second));

        /* modifiers */

        /* skill */
        if ( get_eq_char(ch,wear_wield) == 0)
                chance = chance * hth/150;
        else
                chance = chance * ch_weapon/100;

        chance += (ch_vict_weapon/2 - vict_weapon) / 2;

        /* dex vs. strength */
        chance += ch->getCurrStat(STAT_DEX);
        chance -= 2 * victim->getCurrStat(STAT_STR);

        /* level */
        chance += ( ch->getModifyLevel() - victim->getModifyLevel() ) * 2;
    
        /* can see */
        if (!ch->can_see( obj )) {
            if (number_percent( ) < gsn_blind_fighting->getEffective( ch ))
                chance = chance * 3 / 4;
            else {
                chance /= 3;
                ch->send_to("Выбить оружие, которое ты не видишь, оказывается гораздо сложнее...\n\r");
            }
        }

        /* and now the attack */
        if (number_percent() < chance)
        {
                ch->setWait( gsn_disarm->getBeats( )  );
                disarm( ch, victim ,disarm_second);
                gsn_disarm->improve( ch, true, victim );
        }
        else
        {
                ch->setWait( gsn_disarm->getBeats( ) );
                act_p("Тебе не удалось обезоружить $C4.",
                        ch,0,victim,TO_CHAR,POS_RESTING);
                act_p("$c1 пытается обезоружить тебя, но не может.",
                        ch,0,victim,TO_VICT,POS_RESTING);
                act_p("$c1 пытается обезоружить $C4, но не может.",
                        ch,0,victim,TO_NOTVICT,POS_RESTING);
                gsn_disarm->improve( ch, false, victim );
        }

        return;
}


/*
 * 'shield cleave' skill command
 */

SKILL_RUNP( shield )
{
    Character *victim;
    int wskill,vict_skill;
    float chance, skill_mod, level_mod, size_mod, stat_mod, wskill_mod;
    Object *shield, *weapon, *dual;
    
    //////////////// BASE MODIFIERS //////////////// TODO: add this to XML
    skill_mod   = 0.5;
    wskill_mod  = 0.02;
    stat_mod    = 0.02;
    level_mod   = 0.02;    
    size_mod    = 0.02;   
    
    //////////////// ELIGIBILITY CHECKS ////////////////

    ///// Standard checks: TODO: turn this into a function
    
    if ( ( victim = ch->fighting ) == 0 )
    {
        ch->send_to("Этот навык можно применять только в бою.\n\r");
        return;
    }  

    if ( !gsn_shield_cleave->usable( ch ) )
    {
        ch->send_to("Ты не знаешь как расколоть щит противника.\n\r");
        return;
    }
 
    weapon = get_eq_char(ch,wear_wield);    
    dual = get_eq_char(ch,wear_second_wield); 
    
    if (dual == 0)
    {
        if (weapon == 0) {
            ch->send_to( "Этот навык можно использовать только с оружием в руках: топором, мечом или алебардой.\n\r");
            return;
        }
        else {
            if (attack_table[weapon->value3()].damage != DAM_SLASH) {
                ch->send_to( "Для этого навыка нужно оружие с рубящей кромкой.\n\r");
                return;                
            }
        }
    } 
    else {
        if (weapon != 0) {
            if ( attack_table[weapon->value3()].damage != DAM_SLASH &&
                 attack_table[dual->value3()].damage != DAM_SLASH ) {
            
                ch->send_to( "Для этого навыка нужно оружие с рубящей кромкой.\n\r");
                return;               
            }
        }
        else {
            bug("dual wielding with no primary wield");
            return;
        }
    }

    if ( ( shield = get_eq_char( victim, wear_shield )) == 0 )
    {
        ch->send_to("Твой противник не использует щит.\n\r");
        return;
    }

    if (material_is_flagged( shield, MAT_INDESTR ) || shield->pIndexData->limit != -1 || IS_SET( shield->extra_flags, ITEM_NOPURGE ))
    {
        ch->send_to("Щит твоего противника слишком прочен, его не удастся разрубить.\n\r");
        return;
    }

    if (SHADOW(ch)) {
        act("Ты размахиваешься и разрубаешь свою тень.", ch,0,victim,TO_CHAR);
        act("%^C1 размахивается и разрубает свою тень.", ch,0,victim,TO_VICT);
        act("%^C1 размахивается и разрубает свою тень.", ch,0,victim,TO_NOTVICT);        
        return;
    }
    
    //////////////// PROBABILITY CHECKS ////////////////        
    chance = 0;
    chance = gsn_shield_cleave->getEffective( ch ) * skill_mod; // 30-50% base
    
    if ( weapon->value0() == WEAPON_AXE ||
         weapon->value0() == WEAPON_POLEARM ||
         dual->value0() == WEAPON_AXE ||      
         dual->value0() == WEAPON_POLEARM )
        
        chance = chance * 1.2; // 48-60%
    
    else if ( weapon->value0() == WEAPON_SWORD ||
              dual->value0() == WEAPON_SWORD )     
        
        chance = chance * 0.9;
    
    else
    {
        ch->send_to("Для этого ты должен вооружиться топором, мечом или алебардой.\n\r");
        return;
    }

    // +/-2% per each 1% skill diff    
    wskill = ch->getSkill(get_weapon_sn(ch, false));
    vict_skill = std::max(1,gsn_shield_block->getEffective( ch ));
    chance += (wskill - vict_skill) * wskill_mod * 100;

    /* strength vs. con/dex, resist or evade */
    // +/-2% per each 1% skill diff  
    chance += ( ch->getCurrStat(STAT_STR) - std::max(victim->getCurrStat(STAT_CON), victim->getCurrStat(STAT_DEX)) ) * stat_mod * 100;    

    /* level */
    // +/-2% per each 1% skill level     
    chance += ( skill_level(*gsn_shield_cleave, victim) - skill_level(*gsn_shield_block, victim) ) * level_mod * 100;
    // +/-1% per each 1% obj level diff    
    chance += std::max(weapon->level, dual->level) - shield->level;

    // +/-2% per each size diff 
    chance += (ch->size - victim->size) * size_mod * 100; 
    // +10% if mounted   
    if ( MOUNTED(ch) )
        chance += 10;
    
    // TO-DO: add object material and weight factors
    
    /* can see */
    if (!ch->can_see( get_eq_char(victim,wear_shield) )) {
        if (number_percent( ) < gsn_blind_fighting->getEffective( ch ))
            chance = chance * 3 / 4;
        else {
            chance /= 3;
            ch->send_to("Разрубить щит, который ты не видишь, оказывается гораздо сложнее...\n\r");
        }
    }
        
    if ( IS_AFFECTED(ch,AFF_WEAK_STUN) )
        chance = chance / 2;         
        
    ch->setWait( gsn_shield_cleave->getBeats( )  );
    if (number_percent() < URANGE( 1, (int)(chance), 95 )) // there's always a chance
    {        
        act("Ты {1{Rраскалываешь{2 щит $C2 надвое!", ch,0,victim,TO_CHAR);
        act("$c1 {1{RРАСКАЛЫВАЕТ{2 твой щит надвое!", ch,0,victim,TO_VICT);
        act("$c1 раскалывает щит $C2 надвое.", ch,0,victim,TO_NOTVICT);
        gsn_shield_cleave->improve( ch, true, victim );
        extract_obj( get_eq_char(victim,wear_shield) );
    }
    else
    {        
        act("Твое оружие со звоном отскакивает от щита $C2!", ch,0,victim,TO_CHAR);
        act("Оружие $c2 со звоном отскакивает от твоего щита!", ch,0,victim,TO_VICT);
        act("Оружие $c2 со звоном отскакивает от щита $C2.", ch,0,victim,TO_NOTVICT);
        gsn_shield_cleave->improve( ch, false, victim );
        ch->send_to("Вибрация от столкновения на мгновение ошеломляет тебя!\n\r");
        SET_BIT(ch->affected_by,AFF_WEAK_STUN);
    }
    return;
}

/*
 * 'weapon cleave' skill command
 */

SKILL_RUNP( weapon )
{
    Character *victim;
    int wskill,vict_skill;
    float chance, skill_mod, level_mod, size_mod, stat_mod, wskill_mod;
    Object *vict_weapon, *weapon, *dual;
    
    //////////////// BASE MODIFIERS //////////////// TODO: add this to XML
    skill_mod   = 0.5;
    wskill_mod  = 0.02;
    stat_mod    = 0.02;
    level_mod   = 0.02;    
    size_mod    = 0.02;   
    
    //////////////// ELIGIBILITY CHECKS ////////////////

    ///// Standard checks: TODO: turn this into a function
    
    if ( ( victim = ch->fighting ) == 0 )
    {
        ch->send_to("Этот навык можно применять только в бою.\n\r");
        return;
    }  

    if ( !gsn_weapon_cleave->usable( ch ) )
    {
        ch->send_to("Ты не знаешь как расколоть оружие противника.\n\r");
        return;
    }
 
    weapon = get_eq_char(ch,wear_wield);    
    dual = get_eq_char(ch,wear_second_wield); 
    
    if (dual == 0)
    {
        if (weapon == 0) {
            ch->send_to( "Этот навык можно использовать только с оружием в руках: топором, мечом или алебардой.\n\r");
            return;
        }
        else {
            if (attack_table[weapon->value3()].damage != DAM_SLASH) {
                ch->send_to( "Для этого навыка нужно оружие с рубящей кромкой.\n\r");
                return;                
            }
        }
    } 
    else {
        if (weapon != 0) {
            if ( attack_table[weapon->value3()].damage != DAM_SLASH &&
                 attack_table[dual->value3()].damage != DAM_SLASH ) {
            
                ch->send_to( "Для этого навыка нужно оружие с рубящей кромкой.\n\r");
                return;               
            }
        }
        else {
            bug("dual wielding with no primary wield");
            return;
        }
    }
    
    if ( ( vict_weapon = get_eq_char( victim, wear_wield )) == 0 )
    {
        ch->send_to("Твой противник не вооружен.\n\r");
        return;
    }

    if (material_is_flagged( vict_weapon, MAT_INDESTR ) || vict_weapon->pIndexData->limit != -1 || IS_SET( vict_weapon->extra_flags, ITEM_NOPURGE ))
    {
        ch->send_to("Оружие твоего противника слишком прочно, его не удастся разрубить.\n\r");
        return;
    }

    if (SHADOW(ch)) {
        act("Ты размахиваешься и разрубаешь свою тень.", ch,0,victim,TO_CHAR);
        act("%^C1 размахивается и разрубает свою тень.", ch,0,victim,TO_VICT);
        act("%^C1 размахивается и разрубает свою тень.", ch,0,victim,TO_NOTVICT);        
        return;
    }
    
    //////////////// PROBABILITY CHECKS ////////////////        
    chance = 0;
    chance = gsn_weapon_cleave->getEffective( ch ) * skill_mod; // 30-50% base
    
    if ( weapon->value0() == WEAPON_AXE ||
         weapon->value0() == WEAPON_POLEARM ||
         dual->value0() == WEAPON_AXE ||      
         dual->value0() == WEAPON_POLEARM )
        
        chance = chance * 1.2; // 48-60%
    
    else if ( weapon->value0() == WEAPON_SWORD ||
              dual->value0() == WEAPON_SWORD )     
        
        chance = chance * 0.9;
    
    else
    {
        ch->send_to("Для этого ты должен вооружиться топором, мечом или алебардой.\n\r");
        return;
    }

    // +/-2% per each 1% skill diff    
    wskill = ch->getSkill(get_weapon_sn(ch, false));
    vict_skill = ch->getSkill(get_weapon_sn(victim, false));    
    vict_skill = std::max(vict_skill, gsn_grip->getEffective( victim ));   
    chance += (wskill - vict_skill) * wskill_mod * 100;

    /* strength vs. con/dex, resist or evade */
    // +/-2% per each 1% skill diff  
    chance += ( ch->getCurrStat(STAT_STR) - std::max(victim->getCurrStat(STAT_CON), victim->getCurrStat(STAT_DEX)) ) * stat_mod * 100;    

    /* level */
    // +/-2% per each 1% skill level
    // TO-DO: add victim skill_level instead of getModifyLevel
    chance += ( skill_level(*gsn_weapon_cleave, victim) - victim->getModifyLevel() ) * level_mod * 100;
    // +/-1% per each 1% obj level diff    
    chance += std::max(weapon->level, dual->level) - vict_weapon->level;

    // +/-2% per each size diff 
    chance += (ch->size - victim->size) * size_mod * 100; 
    // +10% if mounted   
    if ( MOUNTED(ch) )
        chance += 10;
    
    // TO-DO: add object material and weight factors

    /* can see */
    if (!ch->can_see( get_eq_char(victim,wear_wield) )) {
        if (number_percent( ) < gsn_blind_fighting->getEffective( ch ))
            chance = chance * 3 / 4;
        else {
            chance /= 3;
            ch->send_to("Разрубить оружие, которое ты не видишь, оказывается гораздо сложнее...\n\r");
        }
    }
        
    if ( IS_AFFECTED(ch,AFF_WEAK_STUN) )
        chance = chance / 2;         
        
    ch->setWait( gsn_weapon_cleave->getBeats( )  );
    if (number_percent() < URANGE( 1, (int)(chance), 95 )) // there's always a chance
    {        
        act("Ты {1{Rраскалываешь{2 оружие $C2 надвое!", ch,0,victim,TO_CHAR);
        act("$c1 {1{RРАСКАЛЫВАЕТ{2 твое оружие надвое!", ch,0,victim,TO_VICT);
        act("$c1 раскалывает оружие $C2 надвое.", ch,0,victim,TO_NOTVICT);
        gsn_weapon_cleave->improve( ch, true, victim );
        extract_obj( get_eq_char(victim,wear_wield) );
    }
    else
    {        
        act("Твое оружие со звоном отскакивает от оружия $C2!", ch,0,victim,TO_CHAR);
        act("Оружие $c2 со звоном отскакивает от твоего оружия!", ch,0,victim,TO_VICT);
        act("Оружие $c2 со звоном отскакивает от оружия $C2.", ch,0,victim,TO_NOTVICT);
        gsn_weapon_cleave->improve( ch, false, victim );
        ch->send_to("Вибрация от столкновения на мгновение ошеломляет тебя!\n\r");
        SET_BIT(ch->affected_by,AFF_WEAK_STUN);
    }
    return;
}



/*
 * 'lash' skill command
 */

SKILL_RUNP( lash )
{
    char arg[MAX_INPUT_LENGTH];
    Object *whip;
    Character *victim;
    bool wasFighting;
    int chance;
    
    if (!gsn_lash->usable( ch )) {
        act("$c1 угрощающе щелкает хлыстом.", ch, 0, 0, TO_ROOM);
        ch->send_to( "Что?\r\n" );
        return;
    }
    
    one_argument(argument, arg);
    
    whip = get_eq_char(ch, wear_wield);
    if (!whip || whip->item_type != ITEM_WEAPON || whip->value0() != WEAPON_WHIP)
        whip = get_eq_char(ch, wear_second_wield);
    if (!whip || whip->item_type != ITEM_WEAPON || whip->value0() != WEAPON_WHIP) 
    {
        ch->send_to( "Возьми в руки хлыст.\n\r" );
        if(IS_CHARMED(ch) && ch->master->getPC() && ch->canCarryNumber( ) > 0)
        ch->master->println("Для этого умения твоему последователю потребуется вооружиться хлыстом.");
        return;
    }

    if (arg[0] == '\0') {
        victim = ch->fighting;
        if (victim == NULL) {
            ch->send_to( "Но ты ни с кем не сражаешься!\n\r" );
            return;
        }
    }
    else if ((victim = get_char_room(ch, arg)) == NULL) {
        ch->send_to("Tут таких нет.\n\r");
        return;
    }
    
    chance = gsn_lash->getEffective( ch );

    if (victim == ch || chance < 50) {
        ch->send_to("Ты запутываешься в хлысте и падаешь!\n\r");
        ch->setWaitViolence( 5 );
        act("$c1 старательно опутывает свои ноги хлыстом и падает на землю.", ch, 0, 0, TO_ROOM);
        return;
    }

    if (is_safe(ch, victim))
        return;

    if (IS_CHARMED(ch) && ch->master == victim) {
        act("Но $C1 твой друг!", ch, NULL, victim, TO_CHAR);
        return;
    }

    if (SHADOW(ch)) {
        ch->send_to("Ты пытаешься огреть хлыстом собственную тень.\n\r");
        act("$c1 бьет свою тень хлыстом.",ch,0,0,TO_ROOM);
        return;
    }

    wasFighting = (ch->fighting != NULL);
    
    chance += ch->getCurrStat(STAT_DEX) * 2;
    chance -= victim->getCurrStat(STAT_DEX) * 2;
    chance += ch->getCurrStat(STAT_STR);
    chance -= victim->getCurrStat(STAT_STR);

    if (IS_QUICK(ch))
        chance += 20;
    if (IS_QUICK(victim))
        chance -= 20;
    if (IS_SLOW(ch))
        chance -= 10;
    if (IS_SLOW(victim))
        chance += 10;

    chance += (ch->getModifyLevel( ) - victim->getModifyLevel( )) * 2;
    chance += gsn_whip->getEffective( ch ) - 100;
    chance = chance * 4 / 5;

    if (number_percent( ) < chance) {
        int dam;

        gsn_lash->improve( ch, true, victim );
        ch->setWaitViolence( 1 );
        victim->setWait( gsn_lash->getBeats( ) );

        dam = ch->damroll;
        dam += number_range(4, 4 + 4 * ch->getCurrStat(STAT_STR) + chance / 10);
        chance /= 2;

        if (ch->size < victim->size)
            chance += (ch->size - victim->size) * 10; 
        
        chance += (ch->getCurrStat(STAT_STR) - victim->getCurrStat(STAT_STR)) * 4;
        
        if (is_flying( victim )) 
            chance = 0;
        
        try {
            if (damage_nocatch(ch,victim,dam,gsn_lash, DAM_BASH, true, DAMF_WEAPON)
                && number_percent( ) < chance) 
            {
                act("$c1 подсекает тебя своим хлыстом!!", ch, NULL, victim, TO_VICT);
                act("Ты подсекаешь $C4 своим хлыстом!", ch, NULL, victim, TO_CHAR);
                act("$c1 подсекает $C4 своим хлыстом.", ch, NULL, victim, TO_NOTVICT);
                
                victim->setWaitViolence( number_range( 0, 2 ) );
                victim->position = POS_RESTING;
            }
        } catch (const VictimDeathException &) {
        }
    }
    else {
        damage(ch,victim,2,gsn_lash,DAM_BASH, false, DAMF_WEAPON);
        act("Ты лишь оцарапа$gло|л|ла $C4.", ch, NULL, victim, TO_CHAR);
        act("$c1 взмахом хлыста поцарапал $C4!", ch, NULL, victim, TO_NOTVICT);
        act("Ты уклоняешься от хлыста $c2.", ch, NULL, victim, TO_VICT);
        gsn_lash->improve( ch, false, victim );
        ch->setWaitViolence( 1 );
    }
    
    if (!wasFighting)
        yell_panic( ch, victim,
                    "Помогите! Кто-то пытается огреть меня хлыстом!",
                    "Помогите! %1$^C1 бичует меня!" );
}

/*
 * 'throw spear' skill command
 */

SKILL_RUNP( throwspear )
{
        Character *victim;
        Object *spear;
        bool success;
        int chance,direction;
        int range = ( ch->getModifyLevel() / 10) + 1;
        DLString argDoor, argVict;
        ostringstream errbuf;

        if ( ch->is_npc() )
                return; /* Mobs can't shoot spears */

        if ( !gsn_spear->usable( ch ) )
        {
                ch->send_to("Ты не знаешь как метать копье.\n\r");
                return;
        }

        if ( ch->fighting )
        {
                ch->send_to("Ты не можешь сконцентрироваться для метания копья.\n\r");
                return;
        }

        if (!direction_range_argument(argument, argDoor, argVict, direction)) {
                ch->send_to("Метнуть копье куда и в кого?\n\r");
                return;
        }

        if ( ( victim = find_char( ch, argVict.c_str(), direction, &range, errbuf ) ) == 0 ) {
            ch->send_to(errbuf);
            return;
        }

        if ( !victim->is_npc() && victim->desc == 0 )
        {
                ch->send_to("Ты не можешь сделать этого.\n\r");
                return;
        }

        if ( victim == ch )
        {
                ch->send_to("Это бессмысленно.\n\r");
                return;
        }

        if ( is_safe_nomessage(ch,victim) )
        {
                ch->pecho("Боги покровительствуют %C3.", victim);
                return;
        }

        if ( ch->in_room == victim->in_room )
        {
                ch->send_to("Ты не можешь метнуть копье в упор.\n\r");
                return;
        }

        spear = get_eq_char(ch, wear_wield);

        if ( !spear
                || spear->item_type != ITEM_WEAPON
                || spear->value0() != WEAPON_SPEAR )
        {
                ch->send_to("Для метания тебе необходимо копье!\n\r");
                return;            
        }

    // This limitation seems pretty arbitrary, spears can be thrown with shield/dual. Commenting for now.
    /*
        if ( get_eq_char(ch,wear_second_wield) || get_eq_char(ch,wear_shield) )
        {
                ch->send_to("Твоя вторая рука дожна быть свободна!\n\r");
                return;            
        } */

        ch->setWait(gsn_spear->getBeats( ) );

        chance = gsn_spear->getEffective( ch );

        if ( victim->position == POS_SLEEPING )
                chance += 40;
        if ( victim->position == POS_RESTING )
                chance += 10;
        if ( victim->position == POS_FIGHTING )
                chance -= 40;

        chance += ch->hitroll - ch->getRealLevel();

        act( "Ты метаешь $o4 $T.", ch, spear, dirs[ direction ].leave, TO_CHAR  );
        act( "$c1 метает $o4 $T.", ch, spear, dirs[ direction ].leave, TO_ROOM );

        set_violent( ch, victim, false );

        obj_from_char(spear);
        int dam;
        
        dam = dice(spear->value1(),spear->value2());
        dam += ch->damroll + get_str_app(ch).missile;
        dam /= 2;

        try {
            success = send_arrow(ch,victim,spear,direction,chance,dam);
        } catch (const VictimDeathException &) {
            victim = NULL;
            success = true;
        }

        gsn_spear->improve( ch, success, victim );
}

