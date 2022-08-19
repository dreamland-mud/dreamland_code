/* $Id: class_thief.cpp,v 1.1.2.27.6.25 2010-09-01 21:20:44 rufina Exp $
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

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

#include "grammar_entities_impl.h"
#include "skill.h"
#include "skillcommandtemplate.h"
#include "skillmanager.h"
#include "affecthandlertemplate.h"

#include "objectbehavior.h"
#include "affect.h"
#include "pcharacter.h"
#include "room.h"
#include "npcharacter.h"
#include "room.h"
#include "core/object.h"
#include "bonus.h"
#include "dreamland.h"


#include "drink_utils.h"
#include "act_move.h"
#include "arg_utils.h"
#include "chance.h"
#include "exitsmovement.h"
#include "act_lock.h"
#include "mercdb.h"
#include "save.h"

#include "magic.h"
#include "fight.h"
#include "onehit.h"
#include "onehit_weapon.h"
#include "damage_impl.h"
#include "vnum.h"
#include "occupations.h"
#include "skill_utils.h"
#include "selfrate.h"
#include "../anatolia/handler.h"
#include "act.h"
#include "interp.h"
#include "merc.h"
#include "def.h"

GSN(backguard);
GSN(backstab);
GSN(blackjack);
GSN(circle);
GSN(dual_backstab);
GSN(envenom);
GSN(key_forgery);
GSN(knife);
GSN(pick_lock);
GSN(poison);
GSN(push);
GSN(rear_kick);
GSN(steal);

BONUS(thief_skills);

static bool mprog_steal_fail( Character *victim, Character *thief )
{
    FENIA_CALL( victim, "StealFail", "C", thief );
    FENIA_NDX_CALL( victim->getNPC( ), "StealFail", "CC", victim, thief );
    return false;
}

static bool mprog_steal_item( Character *victim, Character *thief, Object *obj )
{
    FENIA_CALL( victim, "StealItem", "CO", thief, obj );
    FENIA_NDX_CALL( victim->getNPC( ), "StealItem", "CCO", victim, thief, obj );
    return false;
}

static bool mprog_steal_money( Character *victim, Character *thief, int gold, int silver )
{
    FENIA_CALL( victim, "StealMoney", "Cii", thief, gold, silver );
    FENIA_NDX_CALL( victim->getNPC( ), "StealMoney", "CCii", victim, thief, gold, silver );
    return false;
}

/*----------------------------------------------------------------------------
 * Backstab
 *---------------------------------------------------------------------------*/
class BackstabOneHit: public SkillWeaponOneHit {
public:
    BackstabOneHit( Character *ch, Character *victim );

    virtual void calcTHAC0( );
    virtual void calcDamage( );
};

BackstabOneHit::BackstabOneHit( Character *ch, Character *victim )
            : Damage(ch, victim, 0, 0, DAMF_WEAPON), SkillWeaponOneHit( ch, victim, gsn_backstab )
{
}

void BackstabOneHit::calcDamage( )
{
    damBase( );
    damapply_class(ch, dam);
    damApplyPosition( );
    damApplyDamroll( );
	
    if (wield != 0) {
	int slevel = skill_level(*gsn_backstab, ch);    
        dam = ( slevel / 10 + 1 ) * dam + slevel;
    }

    WeaponOneHit::calcDamage( );
}

void BackstabOneHit::calcTHAC0( )
{
    thacBase( );
    thacApplyHitroll( );
    thacApplySkill( );
    thac0 -= 10 * (100 - gsn_backstab->getEffective( ch ));
}

/*----------------------------------------------------------------------------
 * Dual backstab
 *---------------------------------------------------------------------------*/
class DualBackstabOneHit: public SkillWeaponOneHit {
public:
    DualBackstabOneHit( Character *ch, Character *victim );

    virtual void calcTHAC0( );
    virtual void calcDamage( );
};

DualBackstabOneHit::DualBackstabOneHit( Character *ch, Character *victim )
            : Damage(ch, victim, 0, 0, DAMF_WEAPON), SkillWeaponOneHit( ch, victim, gsn_dual_backstab )
{
}

void DualBackstabOneHit::calcDamage( )
{
    damBase( );
    damapply_class(ch, dam);
    damApplyPosition( );
    damApplyDamroll( );
	
    if (wield != 0) {
	int slevel = skill_level(*gsn_dual_backstab, ch);    
        dam = ( slevel / 10 + 1 ) * dam + slevel;
    }

    WeaponOneHit::calcDamage( );
}

void DualBackstabOneHit::calcTHAC0( )
{
    thacBase( );
    thacApplyHitroll( );
    thacApplySkill( );
    thac0 -= 10 * (100 - gsn_dual_backstab->getEffective( ch ));
}
/*----------------------------------------------------------------------------
 * Circle
 *---------------------------------------------------------------------------*/
class CircleOneHit: public SkillWeaponOneHit {
public:
    CircleOneHit( Character *ch, Character *victim );

    virtual void calcDamage( );
};

CircleOneHit::CircleOneHit( Character *ch, Character *victim )
            : Damage(ch, victim, 0, 0, DAMF_WEAPON), SkillWeaponOneHit( ch, victim, gsn_circle )
{
}

void CircleOneHit::calcDamage( )
{
    damBase( );
    damapply_class(ch, dam);
    damApplyPosition( );
    damApplyDamroll( );
	
    int slevel = skill_level(*gsn_circle, ch);    
    dam = ( slevel / 40 + 1 ) * dam + slevel;
	
    damApplyCounter( );

    WeaponOneHit::calcDamage( );
}

/*----------------------------------------------------------------------------
 * Knife
 *---------------------------------------------------------------------------*/
class KnifeOneHit: public SkillWeaponOneHit {
public:
    KnifeOneHit( Character *ch, Character *victim );

    virtual void calcDamage( );

};

KnifeOneHit::KnifeOneHit( Character *ch, Character *victim )
            : Damage(ch, victim, 0, 0, DAMF_WEAPON), SkillWeaponOneHit( ch, victim, gsn_knife )
{
}

void KnifeOneHit::calcDamage( )
{
    damBase( );
    damapply_class(ch, dam);
    damApplyPosition( );
    damApplyDamroll( );
	
    int slevel = skill_level(*gsn_knife, ch);    
    dam = ( slevel / 30 + 1 ) * dam + slevel;
	
    damApplyCounter( );

    WeaponOneHit::calcDamage( );
}



/* for poisoning weapons and food/drink */
/*
 * 'envenom' skill command
 */

SKILL_RUNP( envenom )
{
    Object *obj;
    Affect af;
    int percent,skill;

    /* find out what */
    if (argument[0] == '\0')
    {
        ch->pecho("Отравить ядом что?");
        return;
    }

    obj =  get_obj_list(ch,argument,ch->carrying);

    if (obj== 0)
    {
        ch->pecho("У тебя нет этого.");
        return;
    }

    skill = gsn_envenom->getEffective(ch);
    
    if (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON)
    {
        if (drink_is_closed( obj, ch ))
            return;

        if (IS_OBJ_STAT(obj,ITEM_BLESS) || IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
        {
            oldact_p("Твоя попытка отравить $o4 закончилась неудачей.",
            ch,obj,0,TO_CHAR,POS_RESTING);
            return;
        }

        if (number_percent() < skill)  /* success! */
        {
            oldact("$c1 отравляет $o4 смертельным ядом.",ch,obj,0,TO_ROOM);
            oldact("Ты отравляешь $o4 смертельным ядом.",ch,obj,0,TO_CHAR);
            if (!IS_SET(obj->value3(), DRINK_POISONED))
            {
                obj->value3(obj->value3() | DRINK_POISONED);
                gsn_envenom->improve( ch, true );
            }
            
            return;
        }

        oldact("Твоя попытка отравить $o4 закончилась неудачей.",ch,obj,0,TO_CHAR);
        if (!IS_SET(obj->value3(), DRINK_POISONED))
            gsn_envenom->improve( ch, false );
        
        return;
     }

    if (obj->item_type == ITEM_WEAPON)
    {
        if (IS_WEAPON_STAT(obj,WEAPON_FLAMING)
        ||  IS_WEAPON_STAT(obj,WEAPON_FROST)
        ||  IS_WEAPON_STAT(obj,WEAPON_SHOCKING)
        ||  IS_WEAPON_STAT(obj,WEAPON_HOLY)
        ||  IS_WEAPON_STAT(obj,WEAPON_FADING)	    
        ||  IS_OBJ_STAT(obj,ITEM_BLESS) )
        {
            oldact("Мощные свойства $o2 не позволяют тебе нанести яд.",ch,obj,0,TO_CHAR);
            return;
        }

        if (obj->value3() < 0
        ||  attack_table[obj->value3()].damage == DAM_BASH)
        {
            ch->pecho("Ты можешь отравить только оружие, имеющее острое лезвие.");
            return;
        }

        if (IS_WEAPON_STAT(obj,WEAPON_POISON))
        {
            ch->pecho( "%1$^O1 уже отравле%1$Gно|н|на|ны ядом.", obj );
            return;
        }

        percent = number_percent();
        if (percent < skill)
        {

	    int slevel = skill_level(*gsn_envenom, ch);
		
            af.bitvector.setTable(&weapon_type2);
            af.type      = gsn_poison;
            af.level     = slevel * percent / 100;
            af.duration  = slevel * percent / 100;
            
            af.modifier  = 0;
            af.bitvector.setValue(WEAPON_POISON);
            affect_to_obj( obj, &af);

            if ( !IS_AFFECTED( ch, AFF_SNEAK ) )
              oldact("$c1 покрывает $o4 смертельным ядом.",ch,obj,0,TO_ROOM);
            oldact("Ты покрываешь $o4 смертельным ядом.",ch,obj,0,TO_CHAR);
            gsn_envenom->improve( ch, true );
            
            return;
        }
        else
        {
            oldact("Твоя попытка отравить ядом $o4 закончилась неудачей.",ch,obj,0,TO_CHAR);
            gsn_envenom->improve( ch, false );
            
            return;
        }
    }

    oldact("Ты не можешь отравить $o4.",ch,obj,0,TO_CHAR);
    return;
}

/*
 * 'steal' skill command
 */

SKILL_RUNP( steal )
{
        char arg1 [MAX_INPUT_LENGTH];
        char arg2 [MAX_INPUT_LENGTH];
        Character *victim;
        Object *obj;
        Object *obj_inve;
        int percent, skill;

        argument = one_argument( argument, arg1 );
        argument = one_argument( argument, arg2 );
        skill = gsn_steal->getEffective( ch );


        if ( arg1[0] == '\0' || arg2[0] == '\0' )
        {
                ch->pecho("Украсть что и у кого?");
                return;
        }

        if ( ( victim = get_char_room( ch, arg2 ) ) == 0 )
        {
                ch->pecho("Нет этого тут.");
                return;
        }

        if( !victim->is_npc() && IS_GHOST( victim ) )
        {
                ch->pecho("Твои руки проходят насквозь через карманы призрака.");
                return;
        }


        if (!victim->is_npc() && is_total_newbie(victim->getPC()))
        {
                ch->pecho("Не стоит воровать у новичков.");
                return;
        }

        if ( victim == ch || victim->is_immortal() )
        {
                ch->pecho("Это неразумно.");
                return;
        }


        if (is_safe(ch,victim))
                return;

        if ( victim->position == POS_FIGHTING )
        {
                ch->pecho("Не стоит -- еще ударят больно в пылу битвы.");
                return;
        }

        ch->setWait( gsn_steal->getBeats(ch)  );


        percent  = number_percent( ) + ( IS_AWAKE(victim) ? 10 : -30 );
        percent += victim->can_see( ch ) ? 20 : 0;
	percent -= skill_level_bonus(*gsn_steal, ch);

        if (ch->isCoder())
            percent = 1;
        if (!ch->is_npc() && bonus_thief_skills->isActive(ch->getPC(), time_info)) {
            ostringstream ostr;
            bonus_thief_skills->reportAction(ch->getPC(), ostr);
            ch->send_to(ostr);
            percent = max(1, percent-number_range(20,25));
        }

        if ( is_safe( ch, victim )
                || (!ch->is_npc() && percent > skill)
                || (victim->is_npc() && IS_SET(victim->act, ACT_NOSTEAL)) )
        {
        /*
         * Failure.
         */
		if (victim->is_npc() && IS_SET(victim->act, ACT_NOSTEAL))
			ch->pecho("У %C2 магическая защита от воришек.", victim);
		
                ch->pecho("Тебя застукали за попыткой воровства!");
                if ( !IS_AFFECTED( victim, AFF_SLEEP ) )
                {
                        victim->position= victim->position==POS_SLEEPING? POS_STANDING:
                        victim->position;
                        oldact_p("$c1 пытается обокрасть тебя.\n\r", ch, 0, victim,TO_VICT,POS_DEAD);
                }
                oldact("$c1 пытается обокрасть $C4.\n\r",  ch, 0, victim,TO_NOTVICT);

                if( ( !victim->is_npc() ) || ( number_percent() < 25 ) )
                        set_thief( ch );

                if (mprog_steal_fail( victim, ch ))
                    return;

                const char *msg;
                switch(number_range(0,3))
                {
                case 0 :
                        msg = "%1$^C1 -- подлая ворюга!";
                        break;
                case 1 :
                        msg = "Да %1$C1 и конфетку у ребенка украсть не сможет, неумеха!";
                        break;
                case 2 :
                        msg = "%1$^C1 пытается обокрасть меня!";
                        break;
                default:
                        msg = "Держи свои руки подальше, %1$C1!";
                        break;
                }

                yell_panic(ch, victim, "Кто-то пытается обокрасть меня!", msg, "steal");

                if ( !ch->is_npc() )
                {
                        if ( victim->is_npc() )
                        {
                                gsn_steal->improve( ch, false, victim );
                                multi_hit( victim, ch , "murder" );
                        }
                }

                return;
        }

        if (arg_is_silver( arg1 ) || arg_is_gold( arg1 ))
        {
                int amount_s = 0;
                int amount_g = 0;
                if (arg_is_silver( arg1 ))
                        amount_s = victim->silver * number_range(1, 20) / 100;
                else
                        amount_g = victim->gold * number_range(1, 7) / 100;

                if ( amount_s <= 0 && amount_g <= 0 )
                {
                        ch->pecho("Тебе не удалось нащупать ни одной монетки.");
                        return;
                }

                ch->gold     += amount_g;
                victim->gold -= amount_g;
                ch->silver     += amount_s;
                victim->silver -= amount_s;

                ch->pecho("Бинго! Тебе удалось стащить %s.", describe_money( amount_g, amount_s, 4 ).c_str( ));
                gsn_steal->improve( ch, true, victim );

                if (mprog_steal_money( victim, ch, amount_g, amount_s ))
                    return;

                return;
        }

        if ( ( obj = see_obj_carry( ch, victim, arg1 ) ) == 0 )
        {
                ch->pecho("Ты не находишь этого.");
                return;
        }

        if ( !can_drop_obj( ch, obj )
   /* ||   IS_SET(obj->extra_flags, ITEM_INVENTORY)*/
   /* ||  obj->level > ch->getModifyLevel() */ )
        {
                ch->pecho("Это тебе не удастся стянуть.");
                return;
        }

        if (obj->behavior && !obj->behavior->canSteal( ch ))
        {
                ch->pecho("У этого предмета магическая защита от воровства.");
                return;
        }

        if ( ch->carry_number + obj->getNumber( ) > ch->canCarryNumber( ) )
        {
                ch->pecho("Твои руки полны.");
                return;
        }

        if ( ch->carry_weight + obj->getWeight( ) > ch->canCarryWeight( ) )
        {
                ch->pecho("Ты не можешь нести такую тяжесть.");
                return;
        }

        DLString now(dreamland->getCurrentTime());
        Object *target;

        if ( !IS_SET( obj->extra_flags, ITEM_INVENTORY ) )
        {
                obj_from_char( obj );
                ch->pecho("Есть!");

                target = obj;
        }
        else
        {
                obj_inve = create_object( obj->pIndexData, 0 );
                clone_object( obj, obj_inve );
                REMOVE_BIT( obj_inve->extra_flags, ITEM_INVENTORY );
                ch->pecho("Ты укра%Gло|л|ла одну штуку!", ch);

                target = obj_inve;
        }

        obj_to_char( target, ch );
        target->properties["stolen"] = now;
        oprog_get( target, ch );
        gsn_steal->improve( ch, true, victim );

        if (mprog_steal_item( victim, ch, target ))
            return;
}

/*
 * 'pick lock' skill command
 */

SKILL_RUNP( pick )
{
    DLString args = argument, arg1, arg2;
    Keyhole::Pointer keyhole;


    if (MOUNTED(ch)) {
        ch->pecho( "Ты не можешь взломать что-либо, пока ты в седле." );
        return;
    }

    arg1 = args.getOneArgument( );
    arg2 = args.getOneArgument( );

    if (arg1.empty( )) {
        ch->pecho( "Взломать что?" );
        return;
    }

    if (arg2.empty( )) {
        ch->pecho( "Взломать чем?" );
        return;
    }

    if (!( keyhole = Keyhole::create( ch, arg1 ) )) {
        ch->pecho( "Ты не видишь здесь такого замка." );
        return;
    }

    keyhole->doPick( arg2 );
}



/*
 * 'backstab' skill command
 */

SKILL_RUNP( backstab )
{
    char arg[MAX_INPUT_LENGTH];
    Character *victim;
    Object *obj;

    one_argument( argument, arg );

    if ( MOUNTED(ch) )
    {
            ch->pecho("Ты не можешь ударить сзади, если ты верхом!");
            return;
    }

    if ( arg[0] == '\0' )
    {
            ch->pecho("Ударить сзади? Кого?");
            return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == 0 )
    {
            ch->pecho("Таких здесь нет.");
            return;
    }


    if ( victim == ch )
    {
            ch->pecho("Как ты можешь ударить сзади себя?");
            return;
    }

    if ( is_safe( ch, victim ) )
    {
            return;
    }

    if ( ( obj = get_eq_char( ch, wear_wield ) ) == 0 )
    {
            oldact_p("Ты долж$gно|ен|на быть вооруже$gно|н|на, чтоб ударить сзади.",
                    ch,0,0,TO_CHAR,POS_RESTING);
            return;
    }

    if ( attack_table[obj->value3()].damage != DAM_PIERCE && obj->value0() != WEAPON_DAGGER )
    {
            ch->pecho("Чтобы ударить сзади, нужно вооружиться кинжалом или другим колющим оружием.");
            return;
    }

    if ( victim->fighting != 0 )
    {
            ch->pecho("Ты не можешь ударить сзади того, кто уже сражается.");
            return;
    }

    if ( ch->fighting != 0 )
    {
            ch->pecho("Тебе некогда подкрадываться к противнику -- ты сражаешься!");
            return;
    }

    if ( victim->hit < (0.7 * victim->max_hit)
            && (IS_AWAKE(victim) ) )
    {
            oldact_p("$C1 бол$Gьно|ен|ьна и подозрител$Gьно|ен|ьна... ты не можешь незаметно подкрасться к не$Gму|му|й.",
                    ch, 0, victim, TO_CHAR,POS_RESTING);
            return;
    }

    if (victim->getLastFightDelay( ) < 300 && IS_AWAKE(victim) )
    {
            oldact_p("$C1 беспокойно озирается по сторонам... ты не сможешь незаметно подкрасться.",
                    ch, 0, victim, TO_CHAR,POS_RESTING);
            return;
    }

    if (gsn_rear_kick->getCommand( )->apply( ch, victim ))
        return;

    BackstabOneHit bs( ch, victim );
    int bsBonus = 0, hasteBonus = 0, sBonus = 0;
    sBonus += skill_level_bonus(*gsn_backstab, ch);	

    if (!ch->is_npc() && bonus_thief_skills->isActive(ch->getPC(), time_info)) {
        ostringstream ostr;
        bonus_thief_skills->reportAction(ch->getPC(), ostr);
        ch->send_to(ostr);
        bsBonus+= number_range(20,25);
        hasteBonus+= number_range(20,25);
    }

    Chance bsChance(ch, gsn_backstab->getEffective(ch)-1+bsBonus+sBonus, 100);

    try {
        if (!IS_AWAKE(victim)
                || bsChance.reroll())
        {
            gsn_backstab->improve( ch, true, victim );
            bs.hit( );
			
            if (IS_QUICK(ch)) {
                    int haste_chance = hasteBonus + gsn_backstab->getEffective( ch ) * 4 / 10;

                    if (Chance(ch, haste_chance-1, 100).reroll()) {
                        if (ch->fighting == victim)
                            BackstabOneHit( ch, victim ).hit( );
                    }
					else {

            int dual_chance, dual_percent = gsn_dual_backstab->getEffective(ch);
            if (ch->is_npc())
                dual_chance = 0;
            else if (bsBonus > 0 && dual_percent > 50)
                dual_chance = 100;
            else
                dual_chance = skill_level_bonus(*gsn_dual_backstab, ch) + dual_percent * 8 / 10;

            if (Chance(ch, dual_chance-1, 100).reroll()) {
                gsn_dual_backstab->improve( ch, true, victim );

                if (ch->fighting == victim)
                    DualBackstabOneHit( ch, victim ).hit( );
            }
            else {
                gsn_dual_backstab->improve( ch, false, victim );
				}
			}
            }
			
			else {

            int dual_chance, dual_percent = gsn_dual_backstab->getEffective(ch);
            if (ch->is_npc())
                dual_chance = 0;
            else if (bsBonus > 0 && dual_percent > 50)
                dual_chance = 100;
            else
                dual_chance = skill_level_bonus(*gsn_dual_backstab, ch) + dual_percent * 8 / 10;

            if (Chance(ch, dual_chance-1, 100).reroll()) {
                gsn_dual_backstab->improve( ch, true, victim );

                if (ch->fighting == victim)
                    DualBackstabOneHit( ch, victim ).hit( );
            }
            else {
                gsn_dual_backstab->improve( ch, false, victim );
				}
			}


		}
        else
        {
            gsn_backstab->improve( ch, false, victim );
            bs.miss( );
        }

        yell_panic( ch, victim,
                    "Помогите! Кто-то ударил меня сзади!",
                    "Помогите! %1$^C1 удари%1$Gло|л|ла меня в спину!" );
    }
    catch (const VictimDeathException& e) {
    }
}

/*
 * 'circle' skill command
 */

SKILL_RUNP( circle )
{
    Character *victim;
    Character *person;
    Object *obj;

    if ( MOUNTED(ch) )
    {
            ch->pecho("Только не верхом!");
            return;
    }

    if ( ( victim = ch->fighting ) == 0 )
    {
            ch->pecho("Сейчас ты не сражаешься.");
            return;
    }

    if ( (obj = get_eq_char(ch,wear_wield)) == 0
            || (attack_table[obj->value3()].damage != DAM_PIERCE 
            && obj->value0() != WEAPON_DAGGER))
    {
            ch->pecho("Вооружись для этого кинжалом или другим колющим оружием.");
            return;
    }

    if (is_safe(ch,victim))
            return;

    for ( person = ch->in_room->people; person != 0; person = person->next_in_room )
    {
            if (person->fighting == ch)
            {
                    ch->pecho("Ты не можешь сделать это, защищаясь от ударов.");
                    return;
            }
    }
 
    CircleOneHit circ( ch, victim );
    int circleBonus = 0;
    circleBonus += skill_level_bonus(*gsn_circle, ch);

    if (!ch->is_npc() && bonus_thief_skills->isActive(ch->getPC(), time_info)) {
        ostringstream ostr;
        bonus_thief_skills->reportAction(ch->getPC(), ostr);
        ch->send_to(ostr);
        circleBonus+= number_range(20,25);
    }


    try {
        if (ch->is_npc() || number_percent( ) < min(100,circleBonus + gsn_circle->getEffective( ch )))
        {
                circ.hit( );
                gsn_circle->improve( ch, true, victim );
        }
        else
        {
                circ.miss( );
                gsn_circle->improve( ch, false, victim );
        }
    }
    catch (const VictimDeathException& e) {
    }
}


/*
 * 'knife' skill command
 */

SKILL_RUNP( knife )
{
    char arg[MAX_INPUT_LENGTH];
    Character *victim;
    Object *knife;
    int chance;

    one_argument(argument, arg);

    if (arg[0] == '\0') {
        ch->pecho("Пырнуть ножом кого?");
        return;
    }

    if ((knife = get_eq_char(ch, wear_wield)) == NULL) {
        ch->pecho("Вооружись для начала.");
        return;
    }

    if (knife->value0() != WEAPON_DAGGER) {
        ch->pecho("Для этого тебе нужен кинжал.");
        return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL) {
        ch->pecho("Нет таких здесь.");
        return;
    }

    if (ch == victim) {
        ch->pecho("У тебя боязнь себя?");
        return;
    }

    if (victim->fighting != NULL) {
        ch->pecho("Подожди, пока закончится сражение.");
        return;
    }

    if (is_safe(ch, victim))
        return;

    chance = min(100, gsn_knife->getEffective( ch ) + skill_level_bonus(*gsn_knife, ch));

    try {
        KnifeOneHit khit( ch, victim );

        if (number_percent( ) < chance) {
            khit.hit( );
            gsn_knife->improve( ch, true, victim );
        }
        else {
            khit.miss( );
            gsn_knife->improve( ch, false, victim );
        }

        yell_panic( ch, victim,
                    "Помогите! Кто-то пырнул меня ножом!",
                    "Помогите! %1$^C1 пырну%1$Gло|л|ла меня ножом!" );
    }
    catch (const VictimDeathException &e) {
    }
}

/*
 * 'key forgery' skill command
 */

SKILL_RUNP( forge )
{
    DLString args = argument, arg;
    Keyhole::Pointer keyhole;
    Object *key, *blank;

    if (( arg = args.getOneArgument( ) ).empty( )) {
        ch->pecho( "Подделать что?" );
        return;
    }

    for (blank = get_obj_carry_type( ch, ITEM_LOCKPICK );
         blank && blank->value0() != Keyhole::LOCK_VALUE_BLANK;
         blank = get_obj_list_type( ch, ITEM_LOCKPICK, blank->next_content ))
        ;

    if (!blank) {
        ch->pecho( "Тебе понадобится заготовка, чтобы создать дубликат или отмычку." );
        return;
    }

    /*
     * create duplicate of a key
     */
    if (( key = get_obj_list_type( ch, arg, ITEM_KEY, ch->carrying ))) {
        Object *dup;
        static const char * DUP_NAMES = "дубликат duplicate %s";
        static const char * DUP_SHORT = "дубликат||а|у||ом|е %s";
        static const char * DUP_LONG  = "Дубликат %s лежит тут.";

        if (!( keyhole = Keyhole::locate( ch, key ) )) {
            ch->pecho( "Непонятно, что же открывает этот ключ." );
            return;
        }

        if (!keyhole->isLockable( )) {
            ch->pecho( "Это ключ от сломанного замка." );
            return;
        }

        if (keyhole->isPickProof( )) {
            ch->pecho( "Это ключ от замка, который невозможно взломать. Увы.." );
            return;
        }

	int chance;
	chance = min(100, gsn_key_forgery->getEffective( ch ) + skill_level_bonus(*gsn_key_forgery, ch));    
	    
        if (number_percent( ) >= chance) {
            oldact("Тебе не удалось точно передать рисунок бороздок $o2.", ch, key, 0, TO_CHAR );
            gsn_key_forgery->improve( ch, false );
            return;
        }

        dup = create_object( key->pIndexData, 0 );
        dup->gram_gender = Grammar::MultiGender::MASCULINE;
        dup->fmtName( DUP_NAMES, key->getName( ) );
        dup->fmtShortDescr( DUP_SHORT, key->getShortDescr( '2' ).c_str( ) );
        dup->fmtDescription( DUP_LONG, key->getShortDescr( '2' ).c_str( ) );
        dup->setMaterial( blank->getMaterial( ) );
        dup->wear_flags  = blank->wear_flags;
        dup->extra_flags = blank->extra_flags;
        dup->condition   = blank->condition;
        dup->weight      = blank->weight;
        dup->value0(1);
        dup->value1(1);
        obj_to_char( dup, ch );

        oldact("Ты изготавливаешь $o4 из $O2.", ch, dup, blank, TO_CHAR );
        oldact("$c1 изготавливает $o4.", ch, key, 0, TO_ROOM );

        gsn_key_forgery->improve( ch, true );
        extract_obj( blank );
        return;
    }

    /*
     * create lockpick for a keyhole
     */
    if (( keyhole = Keyhole::create( ch, arg ) )) {
        static const char * LOCK_NAMES = "отмычка lockpick";
        static const char * LOCK_SHORT = "фирменн|ая|ой|ой|ую|ой|ой отмычк|а|и|е|у|ой|е %1$#^C2";
        static const char * LOCK_LONG  = "Фирменная отмычка (lockpick) %1$#^C2 оставлена тут хозя%1$#Gином|ином|йкой.";
        static const char * LOCK_EXTRA = "Эта отмычка из 'Фирменного набора %1$#^C2' подходит для замка\n";

        if (!keyhole->isLockable( )) {
            ch->pecho( "Здесь нет замочной скважины." );
            return;
        }

        if (keyhole->isPickProof( )) {
            ch->pecho( "Этот замок защищен от взлома." );
            return;
        }

	int chance;
	chance = min(100, gsn_key_forgery->getEffective( ch ) + skill_level_bonus(*gsn_key_forgery, ch)); 
	    
        if (number_percent( ) >= chance) {
            oldact("Твои попытки превратить $o4 в отмычку к этому замку ни к чему не привели.", ch, blank, 0, TO_CHAR );
            gsn_key_forgery->improve( ch, false );
            return;
        }

        oldact("$o1 в твоих умелых руках постепенно превращается в отмычку для $N2.", ch, blank, keyhole->getDescription( ).c_str( ), TO_CHAR );
        oldact("$c1 проделывает манипуляции с $o5.", ch, blank, 0, TO_ROOM );

//        blank->setOwner( ch->getName( ).c_str( ) );
        blank->gram_gender = Grammar::MultiGender::FEMININE;
        blank->setName( LOCK_NAMES );
        blank->setShortDescr( fmt( 0, LOCK_SHORT, ch ).c_str( ) );
        blank->setDescription( fmt( 0, LOCK_LONG, ch ).c_str( ) );
        blank->addExtraDescr( blank->getName( ), fmt( 0, LOCK_EXTRA, ch ) );
        keyhole->record( blank );

        blank->value0(keyhole->getLockType( ));
        blank->value1(50 + gsn_key_forgery->getEffective( ch ) / 2);

        gsn_key_forgery->improve( ch, true );
        return;
    }

    ch->pecho( "У тебя нет такого ключа, и здесь нет такой замочной скважины." );
}

