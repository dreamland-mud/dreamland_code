/* $Id: battlerager.cpp,v 1.1.6.10.4.18 2010-09-01 21:20:44 rufina Exp $
 *
 * ruffina, 2005
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

#include "battlerager.h"
#include "xmlattributerestring.h"
#include "logstream.h"

#include "commandtemplate.h"
#include "skill.h"
#include "skillcommandtemplate.h"
#include "skillmanager.h"

#include "affect.h"
#include "pcharacter.h"
#include "room.h"
#include "npcharacter.h"
#include "object.h"


#include "movetypes.h"
#include "directions.h"
#include "terrains.h"
#include "move_utils.h"
#include "doors.h"

#include "magic.h"
#include "fight.h"
#include "vnum.h"
#include "merc.h"
#include "loadsave.h"
#include "act.h"
#include "interp.h"
#include "immunity.h"
#include "def.h"
#include "skill_utils.h"

using std::max;
using std::min;

#define OBJ_VNUM_BATTLE_PONCHO       26

CLAN(battlerager);
GSN(bandage);
GSN(bloodthirst);
GSN(cure_blindness);
GSN(cure_disease);
GSN(cure_poison);
GSN(haste);
GSN(mortal_strike);
GSN(remove_curse);
GSN(resistance);
GSN(spellbane);
GSN(trophy);
GSN(truesight);

/*
 * poncho behavior
 */
void BattleragerPoncho::wear( Character *ch ) 
{
    Affect af;
    short level = skill_level(*gsn_trophy, ch);

    if (ch->isAffected(gsn_haste )) 
        return;

    af.bitvector.setTable(&affect_flags);
    af.type = gsn_haste;
    af.duration = -2;
    af.level = level;
    af.bitvector.setValue(AFF_HASTE);
    af.location = APPLY_DEX;
    af.modifier = 1 + ( level >= 18 ) + ( level >= 30 ) + ( level >= 45 ) + skill_level_bonus(*gsn_trophy, ch);
    affect_to_char(ch, &af);
}

void BattleragerPoncho::remove( Character *ch )
{
    if (ch->isAffected(gsn_haste))
        affect_strip(ch, gsn_haste);
}

PersonalBattleragerPoncho::~PersonalBattleragerPoncho( )
{
}



/*
 * 'mortal strike' skill command
 */
SKILL_DECL( mortalstrike );
SKILL_APPLY( mortalstrike )
{
    Object *wield;
    int chance, learned;
    int weaponLevelDiff;

    if (!gsn_mortal_strike->usable( ch, false ))
        return false;

    if ((learned = gsn_mortal_strike->getEffective( ch )) <= 1)
        return false;

    // Works only for primary weapon.
    if ((wield = get_eq_char(ch, wear_wield)) == 0)
        return false;

    int slevel = skill_level(*gsn_mortal_strike, ch);
    // Low-level weapon cannot strike a powerful victim.
    // However, allow weapon level as low as 80+ for heroes.
    weaponLevelDiff = max(1, slevel / 5); 
    if (victim->getModifyLevel() - wield->level > weaponLevelDiff) {
        return false;
    }

    int dam_type = attack_table[wield->value3()].damage;
    int dam_flag = DAMF_WEAPON;
    if (immune_check(victim, dam_type, dam_flag) == RESIST_IMMUNE)
        return false;

    // Calculate real chance to strike (original Anatolia code).
    chance = 1 + learned / 30; 
    chance += (slevel - victim->getModifyLevel()) / 2;
    chance = max(1, chance);

    // Dice roll failed, learn from mistakes.
    if (number_percent() > chance) {
        gsn_mortal_strike->improve( ch, false, victim );
        return false;
    }

    // Success, inflict a lot of damage. Anatolia implementation had (victim->hit+1), but the 
    // resulting damage always got reduced by sanctuary and other protections.
    int dam;
    oldact("{RТвой молниеносный удар в одно мгновение лишает $C4 жизни!{x", ch,0,victim,TO_CHAR);
    oldact("{RМолниеносный удар $c2 в одно мгновение лишает $C4 жизни!{x", ch,0,victim,TO_NOTVICT);
    oldact_p("{RМолниеносный удар $c2 в одно мгновение лишает тебя жизни!{x", ch,0,victim,TO_VICT,POS_DEAD);
    dam = victim->hit * 2; 
    damage(ch, victim, dam, gsn_mortal_strike, dam_type, true, dam_flag);
    gsn_mortal_strike->improve( ch, true, victim );
    return true;
}

/*
 * 'bloodthirst' skill command
 */

SKILL_RUNP( bloodthirst )
{
    int chance, hp_percent;

    chance = gsn_bloodthirst->getEffective( ch );

    if (IS_AFFECTED(ch,AFF_BLOODTHIRST) || ch->isAffected(gsn_bloodthirst) )
    {
        ch->pecho( "Ты уже жаждешь крови." );
        return;
    }

    if (IS_AFFECTED(ch,AFF_CALM))
    {
        ch->pecho( "Ты слишком миролюбив{Sfа{Sx, чтоб жаждать крови." );
        return;
    }

    if (ch->fighting == 0)
      {
        ch->pecho( "Это умение сработает только в бою." );
        return;
      }

    /* modifiers */

    hp_percent = ( HEALTH(ch) );
    chance += ( 25 - hp_percent / 2 );

    if (number_percent() < chance)
    {
        Affect af;

        int slevel = skill_level(*gsn_bloodthirst, ch);
        
        ch->pecho( "Ты жаждешь {rкрови!{x" );
        oldact_p("Глаза $c2 загораются кровожадным огнем.",
               ch,0,0,TO_ROOM,POS_RESTING);
        gsn_bloodthirst->improve( ch, true );

        af.bitvector.setTable(&affect_flags);
        af.type                = gsn_bloodthirst;
        af.level        = ch->getModifyLevel();
        af.duration        = 2 + slevel / 18;
        af.modifier        = slevel / 7 + 1;
        af.bitvector.setValue(AFF_BLOODTHIRST);

        af.location = APPLY_HITROLL;
        affect_to_char(ch,&af);

        af.location = APPLY_DAMROLL;
        affect_to_char(ch,&af);

        af.modifier        = slevel;
        af.location = APPLY_AC;
        affect_to_char(ch,&af);
    }

    else
    {
        ch->pecho( "На миг ты чувствуешь себя кровожадно, но это быстро проходит." );
        gsn_bloodthirst->improve( ch, false );
    }
}


/*
 * 'spellbane' skill command
 */

SKILL_RUNP( spellbane )
{
        Affect af;
        
        if (ch->isAffected(gsn_spellbane))
        {
                ch->pecho( "Ты уже отражаешь заклинания." );
                return;
        }

        int slevel = skill_level(*gsn_spellbane, ch);    

        af.type                = gsn_spellbane;
        af.level        = slevel;
        af.duration        = slevel / 3;
        af.location = APPLY_SAVING_SPELL;
        af.modifier        = ( -slevel / 12 + 1 );

        af.bitvector.setTable(&detect_flags);
        af.bitvector.setValue(DETECT_MAGIC);

        affect_to_char(ch,&af);

        oldact("Ненависть к магии окружает тебя.",ch,0,0,TO_CHAR);
        oldact("$c1 распространяет вокруг себя ненависть к магии.", ch,0,0,TO_ROOM);
}

/*
 * 'resistance' skill command
 */

SKILL_RUNP( resistance )
{
        if (ch->isAffected(gsn_resistance))
        {
                ch->pecho( "Ты уже сопротивляешься физическим атакам." );
                return;
        }

        if ((!ch->is_npc() && number_percent() < gsn_resistance->getEffective( ch ))
          || ch->is_npc() )
    {
        postaffect_to_char(ch, gsn_resistance, skill_level(*gsn_resistance, ch) / 6);

      oldact("Ты чувствуешь себя крепче!",ch,0,0,TO_CHAR);
      oldact("$c1 выглядит покрепче.",ch,0,0,TO_ROOM);
      gsn_resistance->improve( ch, true );
    }
  else
    {

     ch->pecho( "Ты напрягаешь свои мускулы, но это все впустую." );
      oldact_p("$c1 играет мускулами, пытаясь выглядеть крепче.",
             ch,0,0,TO_ROOM,POS_RESTING);
      gsn_resistance->improve( ch, false );
    }

}


/*
 * 'truesight' skill command
 */

SKILL_RUNP( truesight )
{

  if (ch->isAffected(gsn_truesight))
    {
      ch->pecho( "Твои глаза настолько зорки, насколько это возможно." );
      return;
    }


  if (!ch->is_npc() && number_percent() < gsn_truesight->getEffective( ch ))
    {
      Affect af;
      int slevel = skill_level(*gsn_truesight, ch);
      
      af.bitvector.setTable(&detect_flags);
      af.type         = gsn_truesight;
      af.level         = slevel;
      af.duration = slevel / 2 + 5;
      
      af.modifier = 0;
      af.bitvector.setValue(DETECT_HIDDEN);
      affect_to_char(ch, &af);

      af.bitvector.setValue(DETECT_INVIS);
      affect_to_char(ch, &af);

      af.bitvector.setValue(DETECT_IMP_INVIS);
      affect_to_char(ch,&af);


      oldact("Ты зорко смотришь вокруг!",ch,0,0,TO_CHAR);
      oldact("$c1 смотрит более зорко.",ch,0,0,TO_ROOM);
      gsn_truesight->improve( ch, true );
    }
  else
    {

     ch->pecho( "Ты зорко смотришь вокруг, но не видишь ничего нового." );
      oldact_p("$c1 зорко смотрит вокруг, но ничего нового не замечает.",
             ch,0,0,TO_ROOM,POS_RESTING);
      gsn_truesight->improve( ch, false );
    }

}


/*
 * 'bandage' skill command
 */

SKILL_RUNP( bandage )
{
        int heal;

        if (ch->isAffected(gsn_bandage))
        {
                oldact("Ты уже перевяза$gло|л|ла свои раны!",ch,0,0,TO_CHAR);
                return;
        }

        if (SHADOW(ch))
        {
                ch->pecho( "Как это наверное интересно смотрится со стороны -- бинтовать собственную тень." );
                oldact_p("$c1 пытается забинтовать свою собственную тень\n\r...похоже кому-то нужен доктор.",
                        ch, 0, 0, TO_ROOM,POS_RESTING);
                return;
        }

        int skill = ch->is_npc() ? 100 : gsn_bandage->getEffective( ch );
        if ( number_percent() < skill )
        {
                Affect af;

                int slevel = skill_level(*gsn_bandage, ch);

                ch->pecho( "Ты накладываешь повязку на свою рану!" );
                oldact("$c1 перевязывает свои раны.",ch,0,0,TO_ROOM);
                gsn_bandage->improve( ch, true );

                heal = ( dice(4, 8 ) + slevel / 2 );
                ch->hit = min( ch->hit + heal, (int)ch->max_hit );
                update_pos( ch );
                ch->pecho( "Тебе становится лучше!" );

                af.bitvector.setTable(&affect_flags);
                af.type                = gsn_bandage;
                af.level        = slevel;
                af.duration        = slevel / 10;
                af.modifier        = ( min( 15, slevel / 2 ) );
                af.bitvector.setValue(AFF_REGENERATION);
                
                affect_to_char(ch,&af);
        }
        else
        {

                ch->pecho( "Ты пытаешься перевязать свои раны, но пальцы не слушаются тебя." );
                gsn_bandage->improve( ch, false );
        }
}


/*--------------------------------------------------------------------------
 * Wiseman 
 *-------------------------------------------------------------------------*/
ClanHealerBattlerager::ClanHealerBattlerager( ) : healPets( false )
{
}

static bool has_curse(Character *wch)
{
    if (IS_AFFECTED(wch,AFF_CURSE))
        return true;

    for (Object *obj = wch->carrying; obj; obj = obj->next_content)
        if ((IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_NOREMOVE))
            &&  !IS_OBJ_STAT(obj,ITEM_NOUNCURSE))
            return true;

    return false;
}

void ClanHealerBattlerager::speech( Character *wch, const char *cspeech )
{
    if (!cspeech[0])
        return;

    DLString speech = DLString(cspeech).toLower();
    
    if (speech != "aid me wiseman" && speech != "помоги мне") {
        if (is_name("wiseman", cspeech) || is_name("aid", cspeech) || is_name("help", cspeech)
            || is_name("помоги", cspeech) || is_name("лекарь", cspeech))
        {
            do_say(ch, "Скажи {1{yпомоги мне{2, если тебе нужна помощь.");
        }
        return;
    }
    
    if ((wch->is_npc( ) && (!wch->master 
                            || wch->master->getClan( ) != clan
                            || !healPets))
        || (!wch->is_npc( ) && wch->getClan( ) != clan)) 
    {
        do_say(ch, "Я не хочу помогать тебе.");
        return;
    }

    bool cursed = has_curse(wch);

    if (!cursed
        && !IS_AFFECTED(wch,AFF_BLIND) 
        && !IS_AFFECTED(wch,AFF_PLAGUE)
        && !IS_AFFECTED(wch,AFF_POISON))
    {
        do_say(ch, "Ты не нуждаешься в моей помощи.");
        return;
    }

    oldact_p("$c1 дает тебе лечебное зелье, предлагая выпить его.",
           ch,0,wch,TO_VICT,POS_RESTING);
    oldact("Ты выпиваешь лечебное зелье.",ch,0,wch,TO_VICT);
    oldact("Ты передаешь лечебное зелье $C3.",ch,0,wch,TO_CHAR);
    oldact("$C1 выпивает лечебное зелье, данное тобой.",ch,0,wch,TO_CHAR);
    oldact("$c1 дает лечебное зелье $C3.",ch,0,wch,TO_NOTVICT);
    oldact("$C1 выпивает лечебное зелье, которое $m да$gло|л|ла $c1.",ch,0,wch,TO_NOTVICT);

    wch->is_npc( ) ? wch->master->setWaitViolence( 1 ) : wch->setWaitViolence( 1 );

    if (IS_AFFECTED(wch,AFF_BLIND))
        ::spell( gsn_cure_blindness, ch->getModifyLevel( ), ch, wch, FSPELL_NOTRIGGER );

    if (IS_AFFECTED(wch,AFF_PLAGUE))
        ::spell( gsn_cure_disease, ch->getModifyLevel( ), ch, wch, FSPELL_NOTRIGGER );

    if (IS_AFFECTED(wch,AFF_POISON))
        ::spell( gsn_cure_poison, ch->getModifyLevel( ), ch, wch, FSPELL_NOTRIGGER );

    if (cursed)
        ::spell( gsn_remove_curse, ch->getModifyLevel( ), ch, wch, FSPELL_NOTRIGGER );
}

/*--------------------------------------------------------------------------
 * Powerman 
 *-------------------------------------------------------------------------*/
bool ClanGuardBattlerager::specFight( )
{
    Character *victim;

    if ( !ch->isAffected(gsn_spellbane) )
            interpret( ch, "spellbane" );

    if (!( victim = getVictim( ) ))
        return true;

    if ( number_percent() < 33 )
    {
            oldact("Ты наносишь тройной удар смертоносной силы!",ch,0,0,TO_CHAR);
            oldact("$c1 наносит тройной удар смертоносной силы!",ch,0,0,TO_ROOM);
            one_hit( ch, victim );
            one_hit( ch, victim );
            one_hit( ch, victim );
    }

    if ( !ch->isAffected(gsn_resistance) )
            interpret( ch, "resistance" );

    if ( ch->hit < (ch->max_hit /3) && !IS_AFFECTED(ch, AFF_REGENERATION) )
            interpret( ch, "bandage" );

    return true;
}

void ClanGuardBattlerager::actGreet( PCharacter *wch )
{
    do_say(ch, "Добро пожаловать. Да прибудет с тобой {1{RЯрость!{2");
}

void ClanGuardBattlerager::actPush( PCharacter *wch )
{
    oldact("$C1 отвешивает тебе нехилый подзатыльник...", wch, 0, ch, TO_CHAR );
    oldact("$C1 отвешивает $c3 подзатыльник...\n\r$c1 -- как ветром сдуло.", wch, 0, ch, TO_ROOM );
}


SKILL_DECL(trophy);
