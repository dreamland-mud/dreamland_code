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
#include "object.h"
#include "bonus.h"
#include "dreamland.h"

#include "gsn_plugin.h"
#include "drink_utils.h"
#include "act_move.h"
#include "arg_utils.h"
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
#include "merc.h"
#include "handler.h"
#include "act.h"
#include "interp.h"
#include "def.h"

GSN(key_forgery);
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
class BackstabOneHit: public WeaponOneHit, public SkillDamage {
public:
    BackstabOneHit( Character *ch, Character *victim );

    virtual void calcTHAC0( );
    virtual void calcDamage( );
};

BackstabOneHit::BackstabOneHit( Character *ch, Character *victim )
            : Damage( ch, victim, 0, 0 ), WeaponOneHit( ch, victim, false ),
              SkillDamage( ch, victim, gsn_backstab, 0, 0, DAMF_WEAPON )
{
}
void BackstabOneHit::calcDamage( )
{
    damBase( );
    gsn_enhanced_damage->getCommand( )->run( ch, victim, dam );;
    damApplyPosition( );

    if (wield != 0)
        dam = ( ch->getModifyLevel( ) < 50)
            ? ( ch->getModifyLevel( ) / 10 + 1) * dam + ch->getModifyLevel( )
            : ( ch->getModifyLevel( ) / 10 ) * dam + ch->getModifyLevel( );

    damApplyDamroll( );

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
class DualBackstabOneHit: public WeaponOneHit, public SkillDamage {
public:
    DualBackstabOneHit( Character *ch, Character *victim );

    virtual void calcTHAC0( );
    virtual void calcDamage( );
};

DualBackstabOneHit::DualBackstabOneHit( Character *ch, Character *victim )
            : Damage( ch, victim, 0, 0 ), WeaponOneHit( ch, victim, false ),
              SkillDamage( ch, victim, gsn_dual_backstab, 0, 0, DAMF_WEAPON )
{
}
void DualBackstabOneHit::calcDamage( )
{
    damBase( );
    gsn_enhanced_damage->getCommand( )->run( ch, victim, dam );;
    damApplyPosition( );

    if (wield != 0)
        dam = ( ch->getModifyLevel( ) < 56)
            ? ( ch->getModifyLevel( ) / 14 + 1) * dam + ch->getModifyLevel( )
            : ( ch->getModifyLevel( ) / 14 ) * dam + ch->getModifyLevel( );

    damApplyDamroll( );

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
class CircleOneHit: public WeaponOneHit, public SkillDamage {
public:
    CircleOneHit( Character *ch, Character *victim );

    virtual void calcDamage( );
};

CircleOneHit::CircleOneHit( Character *ch, Character *victim )
            : Damage( ch, victim, 0, 0 ), WeaponOneHit( ch, victim, false ),
              SkillDamage( ch, victim, gsn_circle, 0, 0, DAMF_WEAPON )
{
}
void CircleOneHit::calcDamage( )
{
    damBase( );
    gsn_enhanced_damage->getCommand( )->run( ch, victim, dam );;
    damApplyPosition( );
    dam = ( ch->getModifyLevel( ) / 40 + 1) * dam + ch->getModifyLevel( );
    damApplyDamroll( );
    damApplyCounter( );

    WeaponOneHit::calcDamage( );
}

/*----------------------------------------------------------------------------
 * Knife
 *---------------------------------------------------------------------------*/
class KnifeOneHit: public WeaponOneHit, public SkillDamage {
public:
    KnifeOneHit( Character *ch, Character *victim );

    virtual void calcDamage( );
};

KnifeOneHit::KnifeOneHit( Character *ch, Character *victim )
            : Damage( ch, victim, 0, 0 ), WeaponOneHit( ch, victim, false ),
              SkillDamage( ch, victim, gsn_knife, 0, 0, DAMF_WEAPON )
{
}
void KnifeOneHit::calcDamage( )
{
    damBase( );
    gsn_enhanced_damage->getCommand( )->run( ch, victim, dam );;
    damApplyPosition( );
    dam = (ch->getModifyLevel( ) / 30 + 1) * dam + ch->getModifyLevel( );
    damApplyDamroll( );
    damApplyCounter( );

    WeaponOneHit::calcDamage( );
}

/*
 * 'settraps' skill command
 */

SKILL_RUNP( settraps )
{
        if (!gsn_settraps->usable(ch))
        {
                ch->send_to("Ты абсолютно не в курсе, как это делается.\n\r");
                return;
        }

        if (!ch->in_room)
                return;

        if ( IS_SET(ch->in_room->room_flags, ROOM_LAW) )
        {
                ch->send_to("Мистическая сила защищает комнату.\n\r");
                return;
        }

        ch->setWait( gsn_settraps->getBeats( )  );

        if ( ch->is_npc()
                || number_percent( ) <  ( gsn_settraps->getEffective( ch ) * 0.7 ) )
        {
                Affect af,af2;

                gsn_settraps->improve( ch, true );

                if (  ch->in_room->isAffected(gsn_settraps ))
                {
                        ch->send_to("В этой комнате уже есть ловушка.\n\r");
                        return;
                }

                if ( ch->isAffected(gsn_settraps))
                {
                        ch->send_to("Ты слишком сильно заботишься еще о старой ловушке.\n\r");
                        return;
                }

                af.where     = TO_ROOM_AFFECTS;
                af.type      = gsn_settraps;
                af.level     = ch->getModifyLevel();
                af.duration  = ch->getModifyLevel() / 40;
                af.location  = APPLY_NONE;
                af.modifier  = 0;
                af.bitvector = AFF_ROOM_THIEF_TRAP;
                ch->in_room->affectTo( &af );

                af2.where     = TO_AFFECTS;
                af2.type      = gsn_settraps;
                af2.level            = ch->getModifyLevel();

                if ( ch->is_adrenalined() )
                        af2.duration  = 1;
                else
                        af2.duration = ch->getModifyLevel() / 10;

                af2.modifier  = 0;
                af2.location  = APPLY_NONE;
                af2.bitvector = 0;
                affect_to_char( ch, &af2 );

                ch->send_to( "Ты устраиваешь ловушку в комнате.\n\r");

                act_p("$c1 устраивает ловушку в комнате.",ch,0,0,TO_ROOM,POS_RESTING);
                return;
        }
        else {
                ch->println( "Твоя попытка устроить ловушку провалилась." );
                gsn_settraps->improve( ch, false );
        }

        return;
}

struct SettrapsDamage : public SelfDamage {
    SettrapsDamage( Character *ch, int dam ) : SelfDamage( ch, DAM_PIERCE, dam )
    {
    }
    virtual void message( ) {
        msgRoom( "Ловушка \6%C4.", ch );
        msgChar( "Ловушка \6тебя!", ch );
    }
};

AFFECT_DECL(Settraps);
VOID_AFFECT(Settraps)::entry( Room *room, Character *ch, Affect *paf )
{
    if (!is_safe_rspell(paf->level,ch)) {
        ch->send_to("Установленная кем-то ловушка препятствует тебе.\n\r");

        try {
            SettrapsDamage( ch, dice(paf->level,5)+12 ).hit( true );
        }
        catch (const VictimDeathException &) {
        }

        room->affectRemove( paf);
    }
}

VOID_AFFECT(Settraps)::toStream( ostringstream &buf, Affect *paf )
{
    buf << fmt( 0, "Здесь на {W%1$d{x ча%1$Iс|са|сов установлена воровская ловушка.",
                   paf->duration )
        << endl;
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
        ch->send_to("Отравить ядом что?\n\r");
        return;
    }

    obj =  get_obj_list(ch,argument,ch->carrying);

    if (obj== 0)
    {
        ch->send_to("У тебя нет этого.\n\r");
        return;
    }

    if ((skill = gsn_envenom->getEffective( ch )) < 1)
    {
        ch->send_to("О чем ты думаешь? Отравить себя!..\n\r");
        return;
    }

    if (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON)
    {
        if (drink_is_closed( obj, ch ))
            return;

        if (IS_OBJ_STAT(obj,ITEM_BLESS) || IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
        {
            act_p("Твоя попытка отравить $o4 закончилась неудачей.",
            ch,obj,0,TO_CHAR,POS_RESTING);
            return;
        }

        if (number_percent() < skill)  /* success! */
        {
            act_p("$c1 отравляет $o4 смертельным ядом.",ch,obj,0,TO_ROOM,POS_RESTING);
            act_p("Ты отравляешь $o4 смертельным ядом.",ch,obj,0,TO_CHAR,POS_RESTING);
            if (!IS_SET(obj->value[3], DRINK_POISONED))
            {
                SET_BIT(obj->value[3], DRINK_POISONED);
                gsn_envenom->improve( ch, true );
            }
            ch->setWait( gsn_envenom->getBeats( ) );
            return;
        }

        act_p("Твоя попытка отравить $o4 закончилась неудачей.",ch,obj,0,TO_CHAR,POS_RESTING);
        if (!IS_SET(obj->value[3], DRINK_POISONED))
            gsn_envenom->improve( ch, false );
        ch->setWait( gsn_envenom->getBeats( ) );
        return;
     }

    if (obj->item_type == ITEM_WEAPON)
    {
        if (IS_WEAPON_STAT(obj,WEAPON_FLAMING)
        ||  IS_WEAPON_STAT(obj,WEAPON_FROST)
        ||  IS_WEAPON_STAT(obj,WEAPON_VAMPIRIC)
        ||  IS_WEAPON_STAT(obj,WEAPON_SHARP)
        ||  IS_WEAPON_STAT(obj,WEAPON_VORPAL)
        ||  IS_WEAPON_STAT(obj,WEAPON_SHOCKING)
        ||  IS_WEAPON_STAT(obj,WEAPON_HOLY)
        ||  IS_OBJ_STAT(obj,ITEM_BLESS) || IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
        {
            act_p("Ты не можешь отравить ядом $o4.",ch,obj,0,TO_CHAR,POS_RESTING);
            return;
        }

        if (obj->value[3] < 0
        ||  attack_table[obj->value[3]].damage == DAM_BASH)
        {
            ch->send_to("Ты можешь отравить только оружие, имеющее острое лезвие.\n\r");
            return;
        }

        if (IS_WEAPON_STAT(obj,WEAPON_POISON))
        {
            ch->pecho( "%1$^O1 уже отравле%1$Gно|н|на ядом.", obj );
            return;
        }

        percent = number_percent();
        if (percent < skill)
        {

            af.where     = TO_WEAPON;
            af.type      = gsn_poison;
            af.level     = ch->getModifyLevel() * percent / 100;
            af.duration  = ch->getModifyLevel() * percent / 100;
            af.location  = 0;
            af.modifier  = 0;
            af.bitvector = WEAPON_POISON;
            affect_to_obj( obj, &af);

            if ( !IS_AFFECTED( ch, AFF_SNEAK ) )
              act_p("$c1 покрывает $o4 смертельным ядом.",ch,obj,0,TO_ROOM,POS_RESTING);
            act_p("Ты покрываешь $o4 смертельным ядом.",ch,obj,0,TO_CHAR,POS_RESTING);
            gsn_envenom->improve( ch, true );
            ch->setWait( gsn_envenom->getBeats( ) );
            return;
        }
        else
        {
            act_p("Твоя попытка отравить ядом $o4 закончилась неудачей.",ch,obj,0,TO_CHAR,POS_RESTING);
            gsn_envenom->improve( ch, false );
            ch->setWait( gsn_envenom->getBeats( ) );
            return;
        }
    }

    act_p("Ты не можешь отравить $o4.",ch,obj,0,TO_CHAR,POS_RESTING);
    return;
}

/*
 * 'steal' skill command
 */

SKILL_RUNP( steal )
{
        char buf  [MAX_STRING_LENGTH];
        char arg1 [MAX_INPUT_LENGTH];
        char arg2 [MAX_INPUT_LENGTH];
        Character *victim;
        Character *tmp_ch;
        Object *obj;
        Object *obj_inve;
        int percent, skill;

        argument = one_argument( argument, arg1 );
        argument = one_argument( argument, arg2 );
        skill = gsn_steal->getEffective( ch );

        if ( ch->is_npc() || skill <= 1)
        {
                ch->send_to("Кажется, ты не умеешь красть?\n\r");
                return;
        }

        if ( arg1[0] == '\0' || arg2[0] == '\0' )
        {
                ch->send_to("Украсть что и у кого?\n\r");
                return;
        }

        if ( ( victim = get_char_room( ch, arg2 ) ) == 0 )
        {
                ch->send_to("Нет этого тут.\n\r");
                return;
        }

        if( !victim->is_npc() && IS_GHOST( victim ) )
        {
                ch->send_to("Эта загадочная субстанция не имеет ничего.");
                return;
        }

        if( !ch->is_npc() && IS_DEATH_TIME( ch ) )
        {
                ch->send_to("Боги снимают с тебя свою защиту.\n\r");
                UNSET_DEATH_TIME(ch);
        }

        if (!victim->is_npc() && victim->desc == 0)
        {
                ch->send_to("Ты не можешь сделать этого.\n\r");
                return;
        }

        if ( victim == ch || victim->is_immortal() )
        {
                ch->send_to("Это не разумно.\n\r");
                return;
        }

        if ( victim->is_immortal() )
        {
                ch->send_to("Подумай, стоит ли это делать.\n\r");
                return;
        }

        if (is_safe(ch,victim))
                return;

        if ( victim->position == POS_FIGHTING )
        {
                ch->send_to("Не стоит - еще ударят больно в пылу битвы.\n\r");
                return;
        }

        tmp_ch = ch->getDoppel( );

        ch->getPC( )->check_hit_newbie( victim );
        ch->setWait( gsn_steal->getBeats( )  );

/*        percent  = number_percent( ) + ( IS_AWAKE(victim) ? 10 : -50 );
        percent += victim->can_see( ch ) ? -10 : 0;*/
        percent  = number_percent( ) + ( IS_AWAKE(victim) ? 10 : -30 );
        percent += victim->can_see( ch ) ? 20 : 0;

        if (ch->isCoder())
            percent = 1;
        if (!ch->is_npc() && bonus_thief_skills->isActive(ch->getPC(), time_info)) {
            ostringstream ostr;
            bonus_thief_skills->reportAction(ch->getPC(), ostr);
            ch->send_to(ostr);
            percent = 1;
        }

        if ( is_safe( ch, victim )
                || (!ch->is_npc() && percent > skill)
                || (victim->is_npc() && IS_SET(victim->act, ACT_NOSTEAL)) )
        {
        /*
         * Failure.
         */

                ch->send_to("Упс..\n\r");
                if ( !IS_AFFECTED( victim, AFF_SLEEP ) )
                {
                        victim->position= victim->position==POS_SLEEPING? POS_STANDING:
                        victim->position;
                        act_p( "$c1 пытается обокрасть тебя.\n\r", ch, 0, victim,TO_VICT,POS_DEAD);
                }
                act_p( "$c1 пытается обокрасть $C4.\n\r",  ch, 0, victim,TO_NOTVICT,POS_RESTING);

                if( !victim->is_npc() )
                        set_thief( ch );

                if (mprog_steal_fail( victim, ch ))
                    return;

                switch(number_range(0,3))
                {
                case 0 :
                        sprintf( buf, "%s - подлый воришка!", tmp_ch->getNameP( ) );
                        break;
                case 1 :
                        sprintf( buf, "Да %s и конфетку у ребенка украсть не сможет, неумеха!",
                                 tmp_ch->getNameP( ));
                        break;
                case 2 :
                        sprintf( buf,"%s пытается обокрасть меня!",tmp_ch->getNameP( ) );
                        break;
                case 3 :
                        sprintf(buf,"Держи свои руки подальше, %s!",tmp_ch->getNameP( ));
                        break;
                }
                if ( IS_AWAKE( victim ) )
                        do_yell( victim, buf );
                if ( !ch->is_npc() )
                {
                        if ( victim->is_npc() )
                        {
                                gsn_steal->improve( ch, false, victim );
                                multi_hit( victim, ch );
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
                        ch->send_to("Тебе не удалось нащупать ни одной монетки.\n\r");
                        return;
                }

                ch->gold     += amount_g;
                victim->gold -= amount_g;
                ch->silver     += amount_s;
                victim->silver -= amount_s;

                ch->pecho("Bingo! Тебе удалось стащить %s.", describe_money( amount_g, amount_s, 4 ).c_str( ));
                gsn_steal->improve( ch, true, victim );

                if (mprog_steal_money( victim, ch, amount_g, amount_s ))
                    return;

                return;
        }

        if ( ( obj = see_obj_carry( ch, victim, arg1 ) ) == 0 )
        {
                ch->send_to("Ты не находишь этого.\n\r");
                return;
        }

        if ( !can_drop_obj( ch, obj )
   /* ||   IS_SET(obj->extra_flags, ITEM_INVENTORY)*/
   /* ||  obj->level > ch->getModifyLevel() */ )
        {
                ch->send_to("Это тебе не удастся стянуть.\n\r");
                return;
        }

        if (obj->behavior && !obj->behavior->canSteal( ch ))
        {
                ch->send_to("Это бесполезно для тебя.\n\r");
                return;
        }

        if ( ch->carry_number + obj->getNumber( ) > ch->canCarryNumber( ) )
        {
                ch->send_to("Твои руки полны.\n\r");
                return;
        }

        if ( ch->carry_weight + obj->getWeight( ) > ch->canCarryWeight( ) )
        {
                ch->send_to("Ты не можешь нести такую тяжесть.\n\r");
                return;
        }

        DLString now(dreamland->getCurrentTime());
        Object *target;

        if ( !IS_SET( obj->extra_flags, ITEM_INVENTORY ) )
        {
                obj_from_char( obj );
                ch->send_to("Есть!\n\r");

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

    if (!gsn_pick_lock->usable( ch )) {
        ch->println( "Ты пытаешься засунуть отмычку в скважину, но промахиваешься!" );
        return;
    }

    if (MOUNTED(ch)) {
        ch->println( "Ты не можешь взломать что-либо, пока ты в седле." );
        return;
    }

    arg1 = args.getOneArgument( );
    arg2 = args.getOneArgument( );

    if (arg1.empty( )) {
        ch->println( "Взломать что?" );
        return;
    }

    if (arg2.empty( )) {
        ch->println( "Взломать чем?" );
        return;
    }

    if (!( keyhole = Keyhole::create( ch, arg1 ) )) {
        ch->println( "Ты не видишь здесь такого замка." );
        return;
    }

    keyhole->doPick( arg2 );
}

/*
 * PushMovement
 */
class PushMovement : public ExitsMovement {
public:
    PushMovement( Character *ch, Character *actor, const char *arg )
               : ExitsMovement( ch, MOVETYPE_SLINK ), actor( actor ), arg( arg )
    {
    }

protected:
    virtual bool checkPosition( Character *wch )
    {
        return true;
    }

    virtual bool findTargetRoom( )
    {
        door = find_exit( ch, arg, FEX_NO_EMPTY|FEX_NO_INVIS );

        if (door < 0) {
            actor->pecho( "Ты не видишь выхода в этом направлении." );
            return false;
        }

        pexit = from_room->exit[door];
        exit_info = pexit->exit_info;
        to_room = pexit->u1.to_room;
        peexit = NULL;

        if (IS_SET(pexit->exit_info, EX_ISDOOR)) {
            if (IS_SET(pexit->exit_info, EX_CLOSED)) {
                actor->pecho( "Там закрыто." );
                return false;
            }
        }

        return true;
    }

    virtual bool canMove( Character *wch )
    {
        if (wch == ch->mount)
            return true;

        if (!(checkActor( )
               && checkVictim( )
               && checkWeb( )))
            return false;

        if (!ExitsMovement::canMove( wch )) {
            actor->pecho( "Ты не сможешь протолкнуть %1$C4 в этом направлении.", ch );
            return false;
        }

        return true;
    }

    virtual bool tryMove( Character *wch )
    {
        if (wch != ch)
            return true;

        if (!ExitsMovement::tryMove( wch )) {
            actor->pecho( "Ты не сможешь протолкнуть %1$C4 в этом направлении.", ch );
            return false;
        }

        UNSET_DEATH_TIME(actor);

        actor->setWait( gsn_push->getBeats( ) );

        return applySkill( );
    }

    bool applySkill( )
    {
        bool fSuccess;
        int percent;

        percent  = number_percent( ) + (IS_AWAKE(ch) ? 30 : -30);
        percent += ch->can_see( actor ) ? 20 : 0;

        if (percent > gsn_push->getEffective( actor )) {
            fSuccess = false;
            actor->pecho( "Упс.." );

            if (!IS_AFFECTED( ch, AFF_SLEEP )) {
                ch->position = ch->position == POS_SLEEPING ? POS_STANDING: ch->position;
                ch->pecho( "%1$^C1 пытается вытолкнуть тебя.", actor );
            }

            actor->recho( ch, "%1$^C1 пытается вытолкнуть %2$C4.", actor, ch );

            if (IS_AWAKE( ch )) {
                interpret_raw( ch, "yell",
                               fmt( ch, "Держи свои руки подальше, %^C1!", actor ).c_str( ) );

                if (ch->is_npc( ))
                    multi_hit( ch, actor );
            }
        }
        else {
            fSuccess = true;
        }

        gsn_push->improve( actor, fSuccess, ch );
        return fSuccess;
    }

    virtual void setWaitstate( )
    {
    }

    virtual void msgOnMove( Character *wch, bool fLeaving )
    {
        if (fLeaving && wch == ch) {
            actor->pecho( "{YТы выталкиваешь %2$C4 %3$s.{x", actor, ch, dirs[door].leave );
            ch->pecho( "{Y%1$^C1 выталкивает тебя %3$s.{x", actor,  ch, dirs[door].leave );
            actor->recho( ch, "{Y%1$^C1 выталкивает %2$C4 %3$s.{x", actor, ch, dirs[door].leave );
        }

        ExitsMovement::msgOnMove( wch, fLeaving );
    }

    bool checkWeb( )
    {
        if (CAN_DETECT(actor, ADET_WEB)) {
            actor->pecho( "Ты в паутине!" );
            actor->recho( ch, "Путаясь в паутине, %1$^C1 глупо пытается вытолкнуть %2$C4.", actor, ch );
            ch->pecho( "Путаясь в паутине, %1$^C1 глупо пытается вытолкнуть тебя.", actor );
            return false;
        }

        if (CAN_DETECT(ch, ADET_WEB)) {
            actor->pecho( "Ты пытаешься вытолкнуть %2$C4, но паутина удерживает %2$P2 на месте.", actor, ch );
            actor->recho( ch, "%1$^C1 пытается вытолкнуть %2$C4, но паутина удерживает %2$P2 на месте.", actor, ch );
            ch->pecho( "%1$^C1 пытается вытолкнуть тебя, но паутина удерживает тебя на месте.", actor, ch );
            return false;
        }

        return true;
    }

    bool checkActor( )
    {
        if (MOUNTED(actor)) {
            actor->pecho( "Ты не можешь толкнуть кого-то, сидя в седле." );
            return false;
        }

        if (RIDDEN(actor)) {
            actor->pecho( "Ты не можешь толкнуть кого-то, пока ты оседла%1$Gно|н|на.", actor );
            return false;
        }

        return true;
    }

    bool checkVictim( )
    {
        if (!ch->is_npc( ) && ch->desc == 0) {
            actor->pecho( "Ты не можешь сделать этого." );
            return false;
        }

        if (actor == ch) {
            actor->pecho( "Это бесполезно." );
            return false;
        }

        if (is_safe(actor, ch))
            return false;

        if (ch->position == POS_FIGHTING) {
            actor->pecho( "Подожди, пока закончится сражение." );
            return false;
        }

        return true;
    }

    Character *actor;
    const char *arg;
};

/*
 * 'push' skill command
 */
SKILL_RUNP( push )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    Character *victim;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if (ch->is_npc() || !gsn_push->usable( ch )) {
        ch->send_to("Кажется, ты не умеешь толкать?\n\r");
        return;
    }

    if (arg1[0] == '\0' || arg2[0] == '\0') {
        ch->send_to("Вытолкнуть кого и куда?\n\r");
        return;
    }

    if ( ( victim = get_char_room( ch, arg1 ) ) == 0 ) {
        ch->send_to("Здесь таких нет.\n\r");
        return;
    }

    PushMovement( victim, ch, arg2 ).move( );
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
            ch->send_to("Ты не можешь ударить сзади, если ты верхом!\n\r");
            return;
    }

    if (gsn_backstab->getEffective( ch ) <= 1)
    {
            ch->send_to("Ты не знаешь, КАК ударить сзади.\n\r");
            return;
    }

    if ( arg[0] == '\0' )
    {
            ch->send_to("Ударить сзади? Кого?\n\r");
            return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == 0 )
    {
            ch->send_to("Этого нет здесь.\n\r");
            return;
    }


    if ( victim == ch )
    {
            ch->send_to("Как ты можешь ударить сзади себя?\n\r");
            return;
    }

    if ( is_safe( ch, victim ) )
    {
            return;
    }

    if ( ( obj = get_eq_char( ch, wear_wield ) ) == 0 )
    {
            act_p("Ты долж$gно|ен|на быть вооруже$gно|н|на, чтоб ударить сзади.",
                    ch,0,0,TO_CHAR,POS_RESTING);
            return;
    }

    if ( attack_table[obj->value[3]].damage != DAM_PIERCE )
    {
            ch->send_to("Чтоб ударить сзади, нужно вооружится колющим оружием.\n\r");
            return;
    }

    if ( victim->fighting != 0 )
    {
            ch->send_to("Ты не можешь ударить сзади того, кто уже сражается.\n\r");
            return;
    }

    if ( ch->fighting != 0 )
    {
            ch->send_to("Тебе некогда подкрадываться к противнику - ты сражаешься!\n\r");
            return;
    }

    ch->setWait( gsn_backstab->getBeats( )  );

    if ( victim->hit < (0.7 * victim->max_hit)
            && (IS_AWAKE(victim) ) )
    {
            act_p( "$C1 бол$Gьно|ен|ьна и подозрител$Gьно|ен|ьна ... ты не можешь незаметно подкрасться к не$Gму|му|й.",
                    ch, 0, victim, TO_CHAR,POS_RESTING);
            return;
    }

    if (victim->getLastFightDelay( ) < 300 && IS_AWAKE(victim) )
    {
            act_p( "$C1 беспокойно озирается по сторонам... ты не сможешь незаметно подкрасться.",
                    ch, 0, victim, TO_CHAR,POS_RESTING);
            return;
    }

    if (gsn_rear_kick->getCommand( )->run( ch, victim ))
        return;

    BackstabOneHit bs( ch, victim );
    bool fBonus = false;

    if (!ch->is_npc() && bonus_thief_skills->isActive(ch->getPC(), time_info)) {
        ostringstream ostr;
        bonus_thief_skills->reportAction(ch->getPC(), ostr);
        ch->send_to(ostr);
        fBonus = true;
    }

    try {
        if (!IS_AWAKE(victim)
                || fBonus
                || number_percent( ) < gsn_backstab->getEffective( ch ))
        {
            gsn_backstab->improve( ch, true, victim );
            bs.hit( );

            int dual_chance, dual_percent = gsn_dual_backstab->getEffective(ch);
            if (ch->is_npc())
                dual_chance = 0;
            else if (fBonus && dual_percent > 50)
                dual_chance = 100;
            else
                dual_chance =  dual_percent * 8 / 10;

            if (number_percent( ) < dual_chance) {
                gsn_dual_backstab->improve( ch, true, victim );

                if (ch->fighting == victim)
                    DualBackstabOneHit( ch, victim ).hit( );
            }
            else {
                gsn_dual_backstab->improve( ch, false, victim );

                if (IS_AFFECTED(ch, AFF_HASTE)) {
                    int haste_chance;
                    if (fBonus)
                        haste_chance = 100;
                    else
                        haste_chance = gsn_backstab->getEffective( ch ) * 4 / 10;

                    if (number_percent() < haste_chance) {
                        if (ch->fighting == victim)
                            BackstabOneHit( ch, victim ).hit( );
                    }
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

    if ( MOUNTED(ch) )
    {
            ch->send_to("Только не верхом!\n\r");
            return;
    }

    if ( ch->is_npc() || !gsn_circle->usable( ch ) )
    {
            ch->send_to("Ты не владеешь круговым ударом.\n\r");
            return;
    }

    if ( ( victim = ch->fighting ) == 0 )
    {
            ch->send_to("Сейчас ты не сражаешься.\n\r");
            return;
    }

    if ( get_eq_char(ch,wear_wield) == 0
            || attack_table[get_eq_char(ch,wear_wield)->value[3]].damage != DAM_PIERCE)
    {
            ch->send_to("Вооружись для этого колющим оружием.\n\r");
            return;
    }

    if (is_safe(ch,victim))
            return;

    ch->setWait( gsn_circle->getBeats( )  );

    for ( person = ch->in_room->people; person != 0; person = person->next_in_room )
    {
            if (person->fighting == ch)
            {
                    ch->send_to("Ты не можешь сделать это, защищаясь от ударов.\n\r");
                    return;
            }
    }

    CircleOneHit circ( ch, victim );
    bool fBonus = false;

    if (!ch->is_npc() && bonus_thief_skills->isActive(ch->getPC(), time_info)) {
        ostringstream ostr;
        bonus_thief_skills->reportAction(ch->getPC(), ostr);
        ch->send_to(ostr);
        fBonus = true;
    }


    try {
        if (ch->is_npc() || fBonus || number_percent( ) < gsn_circle->getEffective( ch ))
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
 * 'blackjack' skill command
 */

SKILL_RUNP( blackjack )
{
        Character *victim;
        Affect af;
        int chance;

        if ( MOUNTED(ch) )
        {
                ch->send_to("Только не верхом!\n\r");
                return;
        }

        if (!gsn_blackjack->usable( ch ) )
        {
                ch->send_to("У тебя нет мешочка.\n\r");
                return;
        }

        if ( (victim = get_char_room(ch,argument)) == 0 )
        {
                ch->send_to("Здесь таких нет.\n\r");
                return;
        }

        if ( ch == victim )
        {
                ch->send_to("Не надо.. можешь потерять сознание.\n\r");
                return;
        }

        if ( victim->fighting )
        {
                ch->send_to("Подожди пока закончится сражение.\n\r");
                return;
        }

        if ( IS_AFFECTED( ch, AFF_CHARM ) )
        {
                ch->send_to( "Ты же не хочешь ударить по голове своего любимого хозяина?\n\r");
                return;
        }

        if ( IS_AFFECTED(victim,AFF_SLEEP) )
        {
                act_p("$E уже спит.",ch,0,victim,TO_CHAR,POS_RESTING);
                return;
        }

        if ( is_safe(ch,victim) )
        {
                return;
        }

        if (gsn_rear_kick->getCommand( )->run( ch, victim ))
            return;

        int k = victim->getLastFightDelay( );

        if ( k >= 0 && k < FIGHT_DELAY_TIME )
                k = k * 100 /        FIGHT_DELAY_TIME;
        else
                k = 100;

        victim->setLastFightTime( );
        ch->setLastFightTime( );

        ch->setWait( gsn_blackjack->getBeats( ) );

        chance = ( int ) ( 0.5 * gsn_blackjack->getEffective( ch ) );
        chance += URANGE( 0, ( ch->getCurrStat(STAT_DEX) - 20) * 2, 10);
        chance += victim->can_see(ch) ? 0 : 5;
        if (victim->is_npc( )
            && victim->getNPC( )->behavior
            && IS_SET(victim->getNPC( )->behavior->getOccupation( ), (1 << OCC_SHOPPER)))
                chance -= 40;

        if (victim->isAffected(gsn_backguard))
            chance /= 2;

        bool fBonus = false;

        if (!ch->is_npc() && bonus_thief_skills->isActive(ch->getPC(), time_info)) {
            ostringstream ostr;
            bonus_thief_skills->reportAction(ch->getPC(), ostr);
            ch->send_to(ostr);
            fBonus = true;
        }

        if (fBonus || number_percent() < chance * k / 100)
        {
                act_p("Ты бьешь $C4 по голове мешочком со свинцом.",
                        ch,0,victim,TO_CHAR,POS_RESTING);
                act_p("Ты чувствуешь внезапную боль в черепе!",
               ch,0,victim,TO_VICT,POS_RESTING);
                act_p("$c1 бьет $C4 сзади по голове тяжелым мешочком! *OUCH*",
               ch,0,victim,TO_NOTVICT,POS_RESTING);
                gsn_blackjack->improve( ch, true, victim );

                af.type = gsn_blackjack;
                af.where = TO_AFFECTS;
                af.level = ch->getModifyLevel();
                af.duration = ch->getModifyLevel() / 15 + 1;
                af.location = APPLY_NONE;
                af.modifier = 0;
                af.bitvector = AFF_SLEEP;
                affect_join ( victim,&af );

                set_backguard( victim );
                set_violent( ch, victim, true );
                if ( IS_AWAKE(victim) )
                        victim->position = POS_SLEEPING;
        }
        else
        {
                damage(ch,victim, ch->getModifyLevel() / 2,gsn_blackjack,DAM_NONE,true);
                gsn_blackjack->improve( ch, false, victim );

                yell_panic( ch, victim,
                            "Помогите! Меня кто-то ударил по голове!",
                            "Помогите! %1$^C1 удари%1$Gло|л|ла меня по голове!" );
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

    if (!gsn_knife->usable( ch )) {
        ch->send_to("Аккуратней, смотри не порежься.\r\n");
        return;
    }

    one_argument(argument, arg);

    if (arg[0] == '\0') {
        ch->send_to("Пырнуть ножом кого?\r\n");
        return;
    }

    if ((knife = get_eq_char(ch, wear_wield)) == NULL) {
        ch->send_to("Вооружись для начала.\r\n");
        return;
    }

    if (knife->value[0] != WEAPON_DAGGER) {
        ch->send_to("Для этого тебе нужен кинжал.\r\n");
        return;
    }

    if ((victim = get_char_room(ch, arg)) == NULL) {
        ch->send_to("Нет таких здесь.\r\n");
        return;
    }

    if (ch == victim) {
        ch->send_to("У тебя боязнь себя?\n\r");
        return;
    }

    if (victim->fighting != NULL) {
        ch->send_to("Подожди, пока закончится сражение.\r\n");
        return;
    }

    if (is_safe(ch, victim))
        return;

    chance = gsn_knife->getEffective( ch );
    ch->setWait( gsn_knife->getBeats( ) );

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

    if (!gsn_key_forgery->usable( ch )) {
        ch->println( "Эти трюки не для тебя." );
        return;
    }

    if (( arg = args.getOneArgument( ) ).empty( )) {
        ch->println( "Подделать что?" );
        return;
    }

    for (blank = get_obj_carry_type( ch, ITEM_LOCKPICK );
         blank && blank->value[0] != Keyhole::LOCK_VALUE_BLANK;
         blank = get_obj_list_type( ch, ITEM_LOCKPICK, blank->next_content ))
        ;

    if (!blank) {
        ch->println( "Тебе понадобится заготовка, чтобы создать дубликат или отмычку." );
        return;
    }

    if (ch->mana < gsn_key_forgery->getMana( )) {
        ch->println( "У тебя не хватает сил для такой тонкой работы." );
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
            ch->println( "Непонятно, что же открывает этот ключ." );
            return;
        }

        if (!keyhole->isLockable( )) {
            ch->println( "Это ключ от сломанного замка." );
            return;
        }

        if (keyhole->isPickProof( )) {
            ch->println( "Это ключ от замка, который невозможно взломать. Увы.." );
            return;
        }

        ch->setWait( gsn_key_forgery->getBeats( ) );
        ch->mana -= gsn_key_forgery->getMana( );

        if (number_percent( ) >= gsn_key_forgery->getEffective( ch )) {
            act( "Тебе не удалось точно передать рисунок бороздок $o2.", ch, key, 0, TO_CHAR );
            gsn_key_forgery->improve( ch, false );
            return;
        }

        dup = create_object( key->pIndexData, 0 );
        dup->fmtName( DUP_NAMES, key->getName( ) );
        dup->fmtShortDescr( DUP_SHORT, key->getShortDescr( '2' ).c_str( ) );
        dup->fmtDescription( DUP_LONG, key->getShortDescr( '2' ).c_str( ) );
        dup->setMaterial( blank->getMaterial( ) );
        dup->wear_flags  = blank->wear_flags;
        dup->extra_flags = blank->extra_flags;
        dup->condition   = blank->condition;
        dup->weight      = blank->weight;
        dup->value[0] = 1;
        dup->value[1] = 1;
        obj_to_char( dup, ch );

        act( "Ты изготавливаешь $o4 из $O2.", ch, dup, blank, TO_CHAR );
        act( "$c1 изготавливает $o4.", ch, key, 0, TO_ROOM );

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
            ch->println( "Здесь нет замочной скважины." );
            return;
        }

        if (keyhole->isPickProof( )) {
            ch->println( "Этот замок защищен от взлома." );
            return;
        }

        ch->setWait( gsn_key_forgery->getBeats( ) );
        ch->mana -= gsn_key_forgery->getMana( );

        if (number_percent( ) >= gsn_key_forgery->getEffective( ch )) {
            act( "Твои попытки превратить $o4 в отмычку к этому замку ни к чему не привели.", ch, blank, 0, TO_CHAR );
            gsn_key_forgery->improve( ch, false );
            return;
        }

        act( "$o1 в твоих умелых руках постепенно превращается в отмычку для $N2.", ch, blank, keyhole->getDescription( ).c_str( ), TO_CHAR );
        act( "$c1 проделывает манипуляции с $o5.", ch, blank, 0, TO_ROOM );

        blank->setOwner( ch->getName( ).c_str( ) );
        blank->setName( LOCK_NAMES );
        blank->setShortDescr( fmt( 0, LOCK_SHORT, ch ).c_str( ) );
        blank->setDescription( fmt( 0, LOCK_LONG, ch ).c_str( ) );
        blank->addExtraDescr( blank->getName( ), fmt( 0, LOCK_EXTRA, ch ) );
        keyhole->record( blank );

        blank->value[0] = keyhole->getLockType( );
        blank->value[1] = 50 + gsn_key_forgery->getEffective( ch ) / 2;

        gsn_key_forgery->improve( ch, true );
        return;
    }

    ch->println( "У тебя нет такого ключа, и здесь нет такой замочной скважины." );
}

