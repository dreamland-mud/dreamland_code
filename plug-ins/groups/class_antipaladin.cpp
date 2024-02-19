/* $Id: class_antipaladin.cpp,v 1.1.2.21.4.14 2009/09/11 11:24:54 rufina Exp $
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
#include "logstream.h"

#include "spelltemplate.h"

#include "skill.h"
#include "skillcommandtemplate.h"
#include "skillmanager.h"

#include "affect.h"
#include "pcharacter.h"
#include "room.h"
#include "object.h"
#include "behavior_utils.h"

#include "act_move.h"


#include "magic.h"
#include "fight.h"
#include "onehit.h"
#include "onehit_weapon.h"
#include "damage_impl.h"
#include "weapongenerator.h"
#include "vnum.h"
#include "merc.h"
#include "act.h"
#include "interp.h"
#include "handler.h"
#include "def.h"
#include "skill_utils.h"

PROF(anti_paladin);
GSN(cleave);

/*----------------------------------------------------------------------------
 * Cleave 
 *---------------------------------------------------------------------------*/
class CleaveOneHit: public SkillWeaponOneHit {
public:
    CleaveOneHit( Character *ch, Character *victim );
    
    virtual void calcTHAC0( );
    virtual void calcDamage( );
};

CleaveOneHit::CleaveOneHit( Character *ch, Character *victim )
            : Damage(ch, victim, 0, 0, DAMF_WEAPON), SkillWeaponOneHit( ch, victim, gsn_cleave )
{
}

void CleaveOneHit::calcDamage( )
{
    int chance;
    
    damBase( );
    damapply_class(ch, dam);
    damApplyPosition( );

    if (victim->is_immortal( ))
        chance = 0;
    else if (wield == 0)
        chance = 0;
    else {
        chance = 5 + (skill_level(*gsn_cleave, ch) - victim->getModifyLevel( ));
        chance = URANGE( 4, chance, 20 );
    }

    if (number_percent( ) < chance) {
        oldact("Ты рассекаешь $C4 {RПОПОЛАМ{x!",ch,0,victim,TO_CHAR);
        oldact("$c1 рассекает тебя {RПОПОЛАМ{x!",ch,0,victim,TO_VICT);
        oldact("$c1 рассекает $C4 {RПОПОЛАМ{x!",ch,0,victim,TO_NOTVICT);

        ch->setWait( 2 );

        handleDeath( );
        throw VictimDeathException( );
    }
    else {
        dam = ( dam * 2 + skill_level(*gsn_cleave, ch) );
    }

    damApplyDamroll( );
    
    damNormalize( );

    if (number_percent( ) < 50)
        protectSanctuary( );

    protectAlign( );
    protectImmune( );
    protectRazer( ); 
    protectMaterial( wield );
}

void CleaveOneHit::calcTHAC0( )
{
    thacBase( );
    thacApplyHitroll( );
    thacApplySkill( );
    thac0 -= 10 * (100 - gsn_cleave->getEffective( ch ));
}


/*
 * 'cleave' skill command
 */

SKILL_RUNP( cleave )
{
    char arg[MAX_INPUT_LENGTH];
    Character *victim;
    Object *obj;

    if ( MOUNTED(ch) ) {
        ch->pecho("Находясь в седле, трудно это сделать!");
        return;
    }

    one_argument( argument, arg );

    if (ch->master != 0 && ch->is_npc())
        return;

 
    if (arg[0] == '\0') {
        ch->pecho("Рассечь кого?");
        return;
    }

    if (( victim = get_char_room( ch, arg ) ) == 0) {
        ch->pecho("Этого нет здесь.");
        return;
    }

    if (victim == ch) {
        ch->pecho("Себя???");
        return;
    }

    if (is_safe( ch, victim ))
        return;

    if ( ( obj = get_eq_char( ch, wear_wield ) ) == 0) {
        ch->pecho("Вооружись для начала режущим или рубящим оружием.");
        return;
    }

    if (attack_table[obj->value3()].damage != DAM_SLASH) {
        ch->pecho("Чтобы рассечь кого-то, нужно вооружится режущим или рубящим оружием.");
        return;
    }

    if (victim->fighting != 0) {
        ch->pecho("Дождись окончания боя.");
        return;
    }

    if (victim->hit < 0.9 * victim->max_hit && IS_AWAKE(victim) )
    {
        oldact_p("$C1 ране$Gно|н|на и настороженно оглядывается... ты не сможешь подкрасться незаметно.",
                ch, 0, victim, TO_CHAR,POS_RESTING);
        return;
    }

    
    CleaveOneHit cleave_hit( ch, victim );
    
    try {
        if (!IS_AWAKE(victim) || ch->is_npc() || number_percent( ) < gsn_cleave->getEffective( ch ))
        {
            gsn_cleave->improve( ch, true, victim );
            cleave_hit.hit( );
        }
        else
        {
            gsn_cleave->improve( ch, false, victim );
            cleave_hit.miss( );
        }

        yell_panic( ch, victim,
                    "Помогите! Кто-то напал на меня!",
                    "Помогите! %1$^C1 хочет меня рассечь пополам!" );
    }
    catch (const VictimDeathException& e) {                                     
    }
}


