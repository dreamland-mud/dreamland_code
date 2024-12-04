/* $Id: class_samurai.cpp,v 1.1.2.21.6.14 2009/08/16 02:50:30 rufina Exp $
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
#include <string.h>

#include "class_samurai.h"

#include "skill.h"
#include "spelltemplate.h"
#include "skillcommandtemplate.h"
#include "skillmanager.h"

#include "affect.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"
#include "object.h"
#include "desire.h"
#include "damage.h"
#include "string_utils.h"

#include "movetypes.h"
#include "directions.h"
#include "terrains.h"
#include "move_utils.h"
#include "doors.h"

#include "magic.h"
#include "skill_utils.h"
#include "fight.h"
#include "weapongenerator.h"
#include "vnum.h"
#include "merc.h"
#include "effects.h"
#include "loadsave.h"
#include "act.h"
#include "interp.h"
#include "def.h"
#include "skill_utils.h"

GSN(none);
GSN(explode);
GSN(hara_kiri);
GSN(katana);
PROF(samurai);
DESIRE(thirst);
DESIRE(hunger);


/*
 * 'explode' skill command
 */
static void yell_explode( Character *ch, Character *victim )
{
    yell_panic( ch, victim,
                "Помогите! Кто-то пытается подорвать меня!",
                "Помогите! %1$^C1 пытается подорвать меня!" );
}


SKILL_RUNP( explode )
{
    Character *victim;
    int dam, hp_dam, dice_dam;
    int hpch, level;
    char arg[MAX_INPUT_LENGTH];

    victim = ch->fighting;
    dam = 0;
    level = skill_level(*gsn_explode, ch);

    if (victim == 0) {
        one_argument(argument, arg);

        if (arg[0] == 0) {
            ch->pecho("Подорвать кого?");
            return;
        }

        if ((victim = get_char_room(ch,arg)) == 0) {
            ch->pecho("Здесь таких нет.");
            return;
        }
    }

    oldact("$c1 поджигает что-то.",ch,0,victim,TO_NOTVICT);
    oldact("$c1 поджигает что-то взрывчатое под тобой!", ch,0,victim,TO_VICT);
    oldact("Пусть все сгорит!",ch,0,0,TO_CHAR);

    if (!ch->is_npc() && number_percent() >= gsn_explode->getEffective( ch ) + skill_level_bonus(*gsn_explode, ch)) {
        damage(ch,victim,0,gsn_explode,DAM_FIRE, true, DAMF_WEAPON);
        gsn_explode->improve( ch, false, victim );
        yell_explode( ch, victim );
        return;
    }

    gsn_explode->improve( ch, true, victim );

    hpch = max( 10, (int)ch->hit );
    hp_dam  = number_range( hpch/9+1, hpch/5 );
    dice_dam = dice(level,20);

    if (!(is_safe(ch,victim))) {
        dam = max(hp_dam + dice_dam /10, dice_dam + hp_dam / 10);
        fire_effect(victim->in_room, ch, level,dam/2,TARGET_ROOM);
    }

    for ( auto &vch : victim->in_room->getPeople()){
       
       if(!vch->isDead() && vch->in_room == victim->in_room){

        if ( vch->is_mirror() && ( number_percent() < 50 ) ) 
            continue;

        if (is_safe_spell(ch,vch,true)
            ||  (vch->is_npc() && ch->is_npc()
            &&   (ch->fighting != vch || vch->fighting != ch)))
            continue;

        if ( is_safe(ch, vch) )
            continue;

        if (vch == victim) /* full damage */ {
            try{
            fire_effect(vch, ch, level,dam,TARGET_CHAR);            
            damage_nocatch(ch,vch,dam,gsn_explode,DAM_FIRE,true, DAMF_WEAPON);
            }
            catch (const VictimDeathException &){
                continue;
            }
        }
        else /* partial damage */ {
            try{
            fire_effect(vch, ch, level/2,dam/4,TARGET_CHAR);
            damage_nocatch(ch,vch,dam/2,gsn_explode,DAM_FIRE,true, DAMF_WEAPON);
            }
            catch (const VictimDeathException &){
                continue;
            }
        }
        
        yell_explode( ch, vch);
    }
    }

    if (!ch->is_npc() && number_percent() >= gsn_explode->getEffective( ch )) {        
        fire_effect(ch, ch, level/4,dam/10,TARGET_CHAR);
        damage(ch,ch,(ch->hit / 10),gsn_explode,DAM_FIRE,true, DAMF_WEAPON);
    
    }
}



/*
 * 'harakiri' skill command
 */

SKILL_RUNP( harakiri )
{
    int chance;

    if ( MOUNTED(ch) )
    {
        ch->pecho("Ты не можешь сделать харакири, если ты верхом!");
        return;
    }

    if (ch->isAffected(gsn_hara_kiri))
    {
        oldact("Если уж реши$gло|л|ла покончить с собой -- попробуй убить Тисахна.", ch, 0, 0, TO_CHAR);
        return;
    }

    /* fighting */
    if (ch->position == POS_FIGHTING || ch->fighting)
    {
        ch->pecho("Используй свой шанс сразиться до конца.");
        return;
    }

    if(SHADOW(ch)) {
      ch->pecho("Ты безуспешно режешь свою тень.");
      oldact_p("$c1 не может даже сделать себе харакири.\n...пора на пенсию.",
             ch, 0, 0, TO_ROOM,POS_RESTING);
      return;
    }

    chance = gsn_hara_kiri->getEffective(ch);

    if (number_percent() < chance + skill_level_bonus(*gsn_hara_kiri, ch))
    {

        ch->hit = 1;
        ch->mana = 1;
        ch->move = 1;
        
        desire_hunger->reset( ch->getPC( ) );
        desire_thirst->reset( ch->getPC( ) );

        ch->pecho("Ты отрезаешь себе палец и ждешь, когда вытечет вся кровь.");
        oldact_p("$c1 разрезает свое тело и ждет смерти.", ch,0,0,TO_ROOM,POS_FIGHTING);
        gsn_hara_kiri->improve( ch, true );
        interpret_raw( ch, "sleep" );
        SET_BIT(ch->act,PLR_HARA_KIRI);

        postaffect_to_char(ch, gsn_hara_kiri, 10);
    }

    else
    {
        postaffect_to_char(ch, gsn_hara_kiri, 0);

        ch->pecho("Ты не можешь отрезать себе палец. Ведь это не так легко!.");
        gsn_hara_kiri->improve( ch, false );
    }
}


/*
 * 'katana' skill command
 */

SKILL_RUNP( katana )
{
        Object *katana;
        Affect af;
        Object *part;
        char arg[MAX_INPUT_LENGTH];

        one_argument( argument, arg );

        if ( ch->isAffected(gsn_katana) )
        {
                ch->pecho("Ты совсем недавно уже изготавливал%Gо||а катану.", ch);
                return;
        }
        

        if ( arg[0] == '\0' )
        {
                ch->pecho("Сделать катану? Из чего?");
                return;
        }

        if ( ( part = get_obj_carry( ch, arg ) ) == 0 )
        {
                ch->pecho("У тебя нету ни куска железа.");
                return;
        }

        if ( part->pIndexData->vnum != OBJ_VNUM_CHUNK_IRON )
        {
                ch->pecho("У тебя нет нужного материала -- поищи в Королевстве Дварфов");
                return;
        }

        if (SHADOW(ch))
        {
                ch->pecho("Твоя тень все время мелькает перед глазами.\n\rТяжеловато сделать катану в таких условиях.");
                oldact_p("$c1 вместе со своей тенью пытаются сделать катану.\n\r...глупое занятие.",
                        ch, 0, 0, TO_ROOM,POS_RESTING);
                return;
        }

        if ( number_percent( ) > ( gsn_katana->getEffective( ch ) / 3 ) * 2 )
        {
                ch->pecho("Ты понапрасну изводишь брусок хорошего железа.");
                extract_obj(part);
                return;
        }

        if ( !ch->is_npc() && number_percent() < gsn_katana->getEffective( ch ) + skill_level_bonus(*gsn_katana, ch) )
        {
                postaffect_to_char(ch, gsn_katana, ch->getModifyLevel());

                katana = create_object( get_obj_index( OBJ_VNUM_KATANA_SWORD), ch->getModifyLevel() );
                katana->cost  = 0;
                katana->level = ch->getModifyLevel();
                        
                WeaponGenerator()
                    .item(katana)
                    .skill(gsn_katana)
                    .valueTier(2)
                    .hitrollTier(1)
                    .damrollTier(1)
                    .hitrollStartPenalty(0.35)
                    .damrollStartPenalty(0.35)
                    .assignValues()
                    .assignStartingHitroll()
                    .assignStartingDamroll();

                // TODO update descriptions for all 3 languages
                DLString patternText = katana->pIndexData->extraDescriptions.front()->description.get(LANG_DEFAULT);
                DLString edText = fmt(0, patternText.c_str(), ch->getNameP('2').c_str());
                katana->addProperDescription()->description[LANG_DEFAULT] = edText;
        
                obj_to_char(katana, ch);
                gsn_katana->improve( ch, true );
        
                oldact("Ты делаешь катану из $o2!",ch,part,0,TO_CHAR);
                oldact("$c1 делает катану из $o2!",ch,part,0,TO_ROOM);
        
                extract_obj(part);
                return;
        }
        else
        {
                oldact("Ты понапрасну изводишь $o4.",ch,part,0,TO_CHAR);
                extract_obj(part);
                gsn_katana->improve( ch, false );
        }
}



/*---------------------------------------------------------------------------
 * SamuraiGuildmaster
 *--------------------------------------------------------------------------*/
void SamuraiGuildmaster::give( Character *victim, Object *obj ) 
{
    if (obj->pIndexData->vnum != OBJ_VNUM_KATANA_SWORD) {
        say_act( victim, ch, "Я не принимаю подарков, $c1." );
        giveBack( victim, obj );
        return;
    }
    
    if (victim->getProfession( ) != prof_samurai) {
        say_act( victim, ch, "Ты не принадлежишь к классу самураев, ты недосто$gйно|ин|йна даже находиться здесь!" );
        giveBack( victim, obj );
        return;
    }
    
    const DLString &edText = obj->extraDescriptions.empty() ? DLString::emptyString : obj->extraDescriptions.front()->description.get(LANG_DEFAULT);
    if (!String::contains(edText, victim->getNameC()) && !String::contains(edText, victim->getNameP('2')))    
    {
        say_act( victim, ch, "Иероглифы на этом оружии говорят о том, что оно было изготовлено кем-то другим, а не тобой, $c1." );
        giveBack( victim, obj );
        return;
    }

    if (!IS_WEAPON_STAT(obj, WEAPON_KATANA)) {
        if (checkPrice( victim, 100 ))
            doFirstEnchant( victim, obj );

        giveBack( victim, obj );
        return;
    }
    
    if (!obj->behavior || obj->behavior->getType( ) != OwnedKatana::MOC_TYPE) {
        if (checkPrice( victim, 300 ))
            doOwner( victim, obj );

        giveBack( victim, obj );
        return;
    }

    giveBack( victim, obj );
}

void SamuraiGuildmaster::tell( Character *victim, const char *speech ) 
{
    if (victim->is_npc( ))
        return;

    if (victim->getProfession( ) != prof_samurai)
        return;

    if (!is_name( "death", speech ) && !is_name( "смерть", speech ))
        return;
    
    if (victim->getPC( )->death < 1) {
        say_act( victim, ch, "Смерть еще ни разу не коснулась тебя, $c1." );
        return;
    }

    if (!checkPrice( victim, 50 ))
        return;

    victim->getPC( )->death -= 1;

    oldact("$C1 забирает смерть у $c5.", victim, 0, ch, TO_ROOM );
    oldact("$C1 забирает у тебя смерть.", victim, 0, ch, TO_CHAR );
    oldact_p("{BМолнии сверкают на небе.{x", victim, 0, ch, TO_ALL, POS_SLEEPING );
}

void SamuraiGuildmaster::giveBack( Character *victim, Object *obj )
{
    oldact("$c1 возвращает $o4 $C3.", ch, obj, victim, TO_NOTVICT );
    oldact("$c1 возвращает тебе $o4.", ch, obj, victim, TO_VICT );

    obj_from_char( obj );
    obj_to_char( obj, victim );
}

bool SamuraiGuildmaster::checkPrice( Character *victim, int qp )
{
    if (victim->is_npc( ))
        return false;
    
    if (victim->getPC( )->getQuestPoints() < qp) {
        say_act( victim, ch, "У тебя недостаточно квестовых очков." );
        return false;
    }

    victim->getPC( )->addQuestPoints(-qp);
    return true;
}

void SamuraiGuildmaster::doFirstEnchant( Character *victim, Object *katana )
{
    Affect af;

    af.bitvector.setTable(&weapon_type2);
    af.type        = gsn_none;
    af.level        = 100;
    af.duration        = -1;
    af.modifier        = 0;
    af.bitvector.setValue(WEAPON_KATANA);
    
    affect_to_obj( katana, &af );
    
    say_act( victim, ch, "Как только ты вооружишься этим, ты почувствуешь, что сила ее постоянно увеличивается." );
}

void SamuraiGuildmaster::doOwner( Character *victim, Object *katana )
{
    if (katana->behavior)
        katana->behavior->unsetObj( );
        
    katana->behavior.setPointer( new OwnedKatana );
    katana->behavior->setObj( katana );
    katana->setOwner( victim->getNameC() );
    SET_BIT(katana->extra_flags, ITEM_NOSAC|ITEM_NOPURGE);

    say_act( victim, ch, "Катана стала твоей личной собственностью. " 
                         "Отныне никто не посмеет прикоснуться к ней." );
}

/*
 * Katana
 */
bool Katana::canEquip( Character *ch )
{
    if (!BasicObjectBehavior::canEquip(ch))
        return false;

    if (!ch->is_immortal() && ch->getProfession( ) != prof_samurai) {
        oldact( "$o1 выпадает из твоих рук.", ch, obj, 0, TO_CHAR );
        oldact( "$o1 выпадает из рук $c2.", ch, obj, 0, TO_ROOM );
        ch->pecho("Катаны -- только для самураев.");
        obj_from_char( obj );
        obj_to_room( obj, ch->in_room );
        return false;
    }

    return true;    
}

void Katana::get( Character *ch )
{
    BasicObjectBehavior::get(ch);

    if (obj->carried_by != ch)
        return;

    if (obj->hasOwner(ch))
        oldact_p("{BМерцающая аура окружает лезвие $o2.{x", ch, obj, 0, TO_CHAR, POS_SLEEPING);
}

void Katana::wear( Character *ch )
{
    const DLString &edText = obj->extraDescriptions.empty() ? DLString::emptyString : obj->extraDescriptions.front()->description.get(LANG_DEFAULT);

  if (IS_WEAPON_STAT(obj,WEAPON_KATANA)
        && (String::contains(edText, ch->getNameC()) || String::contains(edText, ch->getNameP('2'))))
  {
    if (!obj->getRealShortDescr().emptyValues())
        ch->pecho("Ты ощущаешь %O4 как часть себя!", obj);
    else
        ch->pecho("Ты ощущаешь СВОЮ катану, как часть себя!");
  }
}

bool Katana::mayFloat( ) 
{
    return true;
}

/*
 * OwnedKatana: legacy behavior
 */
void OwnedKatana::get( Character *ch )
{
    BasicObjectBehavior::get(ch);

    if (obj->carried_by != ch)
        return;

    if (obj->hasOwner(ch))
        oldact_p("{BМерцающая аура окружает лезвие $o2.{x", ch, obj, 0, TO_CHAR, POS_SLEEPING);
}

