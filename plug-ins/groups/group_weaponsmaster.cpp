
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

#include "skill.h"
#include "skillcommandtemplate.h"
#include "skillmanager.h"

#include "affect.h"
#include "pcharacter.h"
#include "race.h"
#include "npcharacter.h"
#include "object.h"

#include "movetypes.h"
#include "directions.h"
#include "terrains.h"
#include "move_utils.h"
#include "doors.h"


#include "magic.h"
#include "skill_utils.h"
#include "debug_utils.h"
#include "damage.h"
#include "material.h"
#include "fight.h"
#include "vnum.h"

#include "stats_apply.h"
#include "merc.h"

#include "loadsave.h"
#include "act.h"
#include "interp.h"
#include "def.h"
#include "l10n.h"

RELIG(godiva);
GSN(lash);
GSN(whip);

/*
 * 'second wield' skill command
 */

SKILL_RUNP( second )
{
    Object *obj;

    if (argument[0] == '\0') {
        ch->pecho( _("Вооружиться чем?") );
        return;
    }

    if (( obj = get_obj_carry (ch, argument) ) == 0) {
        ch->pecho( _("У тебя нет этого.") );
        return;
    }

    if (!obj->can_wear( ITEM_WIELD )) {
        ch->pecho( _("Ты не можешь вооружиться этим как вторичным оружием.") );
        return;
    }
    
    wear_second_wield->wear( obj, F_WEAR_REPLACE | F_WEAR_VERBOSE );
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
    
    one_argument(argument, arg);
    
    whip = get_eq_char(ch, wear_wield);
    if (!whip || whip->item_type != ITEM_WEAPON || get_weapon_class(whip) != WEAPON_WHIP)
        whip = get_eq_char(ch, wear_second_wield);
    if (!whip || whip->item_type != ITEM_WEAPON || get_weapon_class(whip) != WEAPON_WHIP) 
    {
        ch->pecho( _("Возьми в руки хлыст.") );
        if(IS_CHARMED(ch) && ch->master->getPC() && Char::canCarryNumber(ch) > 0)
        ch->master->pecho(_("Для этого умения твоему последователю потребуется вооружиться хлыстом."));
        return;
    }

    if (arg[0] == '\0') {
        victim = ch->fighting;
        if (victim == NULL) {
            ch->pecho( _("Но ты ни с кем не сражаешься!") );
            return;
        }
    }
    else if ((victim = get_char_room(ch, arg)) == NULL) {
        ch->pecho(_("Тут таких нет."));
        return;
    }
    
    chance = gsn_lash->getEffective( ch );

    if (victim == ch || chance < 50) {
        ch->pecho(_("Ты запутываешься в хлысте и падаешь!"));
        oldact(_("$c1 старательно опутывает свои ноги хлыстом и падает на землю."), ch, 0, 0, TO_ROOM);
        return;
    }

    if (is_safe_attack(ch, victim))
        return;

    if (IS_CHARMED(ch) && ch->master == victim) {
        oldact(_("Но $C1 твой друг!"), ch, NULL, victim, TO_CHAR);
        return;
    }

    if (SHADOW(ch)) {
        ch->pecho(_("Ты пытаешься огреть хлыстом собственную тень."));
        oldact(_("$c1 бьет свою тень хлыстом."),ch,0,0,TO_ROOM);
        return;
    }

    if (!Char::hasLegs(victim)) {
        ch->pecho(_("Но у %C2 нету ног!"), victim);
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
        victim->setWait( gsn_lash->getBeats(victim) );

        dam = ch->damroll;
        dam += number_range(4, 4 + 4 * ch->getCurrStat(STAT_STR) + chance / 10);
        chance /= 2;

        if (ch->getSize( ) < victim->getSize( ))
            chance += (ch->getSize( ) - victim->getSize( )) * 10; 
        
        chance += (ch->getCurrStat(STAT_STR) - victim->getCurrStat(STAT_STR)) * 4;
        
        if (is_flying( victim )) 
            chance = 0;
        
        try {
            if (damage_nocatch(ch,victim,dam,gsn_lash, DAM_BASH, true, DAMF_WEAPON)
                && number_percent( ) < chance) 
            {
                oldact(_("$c1 подсекает тебя своим хлыстом!!"), ch, NULL, victim, TO_VICT);
                oldact(_("Ты подсекаешь $C4 своим хлыстом!"), ch, NULL, victim, TO_CHAR);
                oldact(_("$c1 подсекает $C4 своим хлыстом."), ch, NULL, victim, TO_NOTVICT);
                
                victim->setWaitViolence( number_range( 0, 2 ) );
                victim->position = POS_RESTING;
            }
        } catch (const VictimDeathException &) {
        }
    }
    else {
        damage(ch,victim,2,gsn_lash,DAM_BASH, false, DAMF_WEAPON);
        oldact(_("Ты лишь оцарапа$gло|л|ла $C4."), ch, NULL, victim, TO_CHAR);
        oldact(_("$c1 взмахом хлыста поцарапал $C4!"), ch, NULL, victim, TO_NOTVICT);
        oldact(_("Ты уклоняешься от хлыста $c2."), ch, NULL, victim, TO_VICT);
        gsn_lash->improve( ch, false, victim );
    }
    
    if (!wasFighting)
        yell_panic( ch, victim,
                    "Помогите! Кто-то пытается огреть меня хлыстом!",
                    "Помогите! %1$^C1 бичует меня!" );
}

