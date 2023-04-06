/* $Id: felar.cpp,v 1.1.2.7 2010-09-01 21:20:46 rufina Exp $
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
#include "skillreference.h"

#include "pcharacter.h"
#include "npcharacter.h"
#include "dreamland.h"
#include "fight.h"
#include "move_utils.h"
#include "skill_utils.h"
#include "merc.h"
#include "handler.h"
#include "mercdb.h"
#include "act.h"
#include "interp.h"
#include "def.h"

GSN(tail);


SKILL_RUNP( tail )
{
    Character *victim;
    int chance;
    bool fightingCheck;
    int damage_tail;
    char arg[MAX_STRING_LENGTH];
    
    one_argument( argument, arg );
    
    if (MOUNTED( ch )) {
        ch->pecho("Только не верхом!");
        return;
    }

    fightingCheck = ch->fighting;

    chance = gsn_tail->getEffective( ch );
    
    if (arg[0] == '\0') {
        victim = ch->fighting;
        if (victim == 0) {
            ch->pecho("Сейчас ты не сражаешься!");
            return;
        }
    }
    else if ((victim = get_char_room(ch, arg)) == 0) {
        ch->pecho("Этого нет здесь.");
        return;
    }

    if (victim->position < POS_FIGHTING) {
        oldact_p("Тебе нужно подождать, пока $E повернется к тебе.",
                ch,0,victim,TO_CHAR,POS_RESTING);
        return;
    }

    if (victim == ch) {
        ch->pecho("Ты пытаешься огреть себя хвостом, но ничего не выходит.");
        return;
    }

    if (is_safe(ch,victim))
        return;

    if (IS_CHARMED(ch) && ch->master == victim) {
        oldact("Но $C1 твой друг!",ch,0,victim,TO_CHAR);
        return;
    }

    /* modifiers */

    /* size  and weight */
    chance += min(ch->canCarryWeight( ), ch->carry_weight) / 200;
    chance -= min(victim->canCarryWeight( ), victim->carry_weight) / 250;


    if (ch->size < victim->size)
        chance += (ch->size - victim->size) * 25;
    else
        chance += (ch->size - victim->size) * 10;


    /* stats */
    chance += ch->getCurrStat(STAT_STR) +  ch->getCurrStat(STAT_DEX);
    chance -= victim->getCurrStat(STAT_DEX) * 2;

    if (is_flying( ch ))
        chance -= 10;


    /* speed */
    if (IS_QUICK(ch))
        chance += 20;
    if (IS_QUICK(victim))
        chance -= 30;

    /* level */

    chance += (skill_level(*gsn_tail, ch) - victim->getModifyLevel()) * 2;

    /* now the attack */
    if (number_percent() < chance / 2)
    {
        oldact("$c1 наносит тебе удар хвостом!",ch,0,victim,TO_VICT);
        oldact("Ты наносишь $C3 удар хвостом!",ch,0,victim,TO_CHAR);
        oldact("$c1 наносит $C3 удар хвостом.",ch,0,victim,TO_NOTVICT);
        gsn_tail->improve( ch, true, victim );
    
        victim->setWaitViolence( number_bits( 2 ) + 1 );

        if (IS_SET(victim->parts, PART_LEGS))
            victim->position = POS_RESTING;
            
        damage_tail = ch->damroll +
                ( 2 * number_range(4, 4 + 10 * ch->size + chance/10) );

        damage(ch,victim,damage_tail,gsn_tail, DAM_BASH, true, DAMF_WEAPON);
    }
    else
    {
        damage(ch,victim,0,gsn_tail,DAM_BASH, true, DAMF_WEAPON);
        
        oldact("Ты теряешь равновесие и падаешь!",ch,0,victim,TO_CHAR);
        oldact("$c1 теряет равновесие и падает!",ch,0,victim,TO_NOTVICT);
        oldact("Ты уклоняешься от хвоста $c2, и $e падает.",ch,0,victim,TO_VICT);
        
        gsn_tail->improve( ch, false, victim );
        ch->position = POS_RESTING;
    }
    
    if (!fightingCheck)
        yell_panic( ch, victim,
                    "Помогите! Кто-то бьет меня хвостом!",
                    "Помогите! %1$^C1 пытается ударить меня хвостом!" );
}


