/* $Id: fight_exp.cpp,v 1.1.2.6 2008/11/13 02:32:36 rufina Exp $
 *
 * ruffina, 2004
 */
/***************************************************************************
 * Все права на этот код 'Dream Land' пренадлежат Igor {Leo} и Olga {Varda}*
 * Некоторую помощь в написании этого кода, а также своими идеями помогали:*
 *    Igor S. Petrenko            {NoFate, Demogorgon}                           *
 *    Koval Nazar            {Nazar, Redrum}                                    *
 *    Doropey Vladimir            {Reorx}                                           *
 *    Kulgeyko Denis            {Burzum}                                           *
 *    Andreyanov Aleksandr  {Manwe}                                           *
 *    и все остальные, кто советовал и играл в этот MUD                           *
 ***************************************************************************/

#include "skill.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"
#include "object.h"
#include "bonus.h"
#include "gsn_plugin.h"
#include "wiznet.h"
#include "merc.h"
#include "mercdb.h"
#include "handler.h"
#include "bonus.h"
#include "fight.h"
#include "act.h"
#include "def.h"
#include "skill_utils.h"

PROF(samurai);
BONUS(experience);
BONUS(pet_day);

int xp_compute( PCharacter *gch, Character *victim, int npccount, int pccount, Character *leader, int base_exp_bonus );

/** Report mob's parameter gain to wiznet 'levels'. */
static void wiznet(NPCharacter *pet, Character *victim, const char *what, int oldPetParam, int petParam, int victParam)
{
    Character *master = pet->master;

    if (master)
        wiznet( WIZ_LEVELS, 0, 0, 
                  "Питомец %C2 %C1 (ур. %d) повышает %s, убив %C4 (ур. %d). Параметр убийцы %d -> %d, жертвы %d.", 
                  master, pet, pet->getLevel(), 
                  what, victim, victim->getRealLevel(), oldPetParam, petParam, victParam);
    else 
        wiznet( WIZ_LEVELS, 0, 0, 
                  "Моб %C1 (ур. %d) повышает %s, убив %C4 (ур. %d). Параметр убийцы %d -> %d, жертвы %d.", 
                  pet, pet->getLevel(), 
                  what, victim, victim->getRealLevel(), oldPetParam, petParam, victParam);
}

/**
 * Charmed mobs level up and improve their parameters when killing a stronger opponent.
 * Every parameter is capped by mob's level * N, and no improvement occurs if victim's level
 * is below mob's level by 5. During the 1st day of Summer there are more chances to improve.
 */
void gain_exp_mob( NPCharacter *ch, Character *victim )
{
    int modifier;
    int mylevel = ch->getRealLevel();
    int diff = victim->getRealLevel() - mylevel;
    bool fBonus = bonus_pet_day->isActive(NULL, time_info);
   
    if (diff > 15)
        modifier = 150;
    else if ( diff > 5 )
        modifier = 125;
    else if ( diff > 0 )
        modifier = 110;
    else if ( diff > -5 )
        modifier = 105;
    else
        return;
    
    if (fBonus)
        modifier = number_range(modifier, 150);

    if ( number_percent() * modifier / 100 > 110 )
    {
        if (victim->getRealLevel() > mylevel && mylevel < 103) {
            int oldParam = ch->getRealLevel();
            ch->setLevel( ch->getRealLevel() + 1 );
            ch->pecho("Ты получаешь уровень!");
            oldact("$c1 выглядит более опытн$gым|ым|ой!\n\r",ch,0,0,TO_ROOM);
            wiznet(ch, victim, "уровень", oldParam, ch->getRealLevel(), victim->getRealLevel());

            // Restore act bits that are removed for low-level adaptive pets.
            if (ch->getRealLevel( ) > 20) {
                if (IS_SET(ch->pIndexData->act, ACT_CLERIC))
                    SET_BIT(ch->act, ACT_CLERIC);
                if (IS_SET(ch->pIndexData->act, ACT_MAGE))
                    SET_BIT(ch->act, ACT_MAGE);
            }
        }
    }
    if ( number_percent() * modifier / 100 > 100 )
    {
        if (victim->hitroll > ch->hitroll && ch->hitroll < mylevel * 3) {
            int oldParam = ch->hitroll;
            ch->hitroll++;
            ch->pecho("Теперь ты будешь попадать более метко!");
            oldact("$c1 становится более метк$gим|им|ой!\n\r",ch,0,0,TO_ROOM);
            wiznet(ch, victim, "точность", oldParam, ch->hitroll, victim->hitroll);
        }
    }
    if ( number_percent() * modifier / 100 > 110 )
    {
        if (victim->damroll > ch->damroll && ch->damroll < mylevel * 3) {
            int oldParam = ch->damroll;
            ch->damroll++;
            ch->pecho("Теперь ты будешь больнее бить!");
            oldact("$c1 теперь будет больнее бить!\n\r",ch,0,0,TO_ROOM);
            wiznet(ch, victim, "урон", oldParam, ch->damroll, victim->damroll);
        }
    }
    if ( number_percent() * modifier / 100 > 100 )
    {
        const int max_hp = mylevel * 300;
        int gain;
        
        gain = max( 0, victim->max_hit - ch->max_hit );
        gain = gain * number_percent() * 20 / 10000; // 20% max
        
        if (gain > 0 && ch->max_hit < max_hp) {
            int oldParam = ch->max_hit;
            ch->max_hit = min(ch->max_hit + gain, max_hp);
            ch->pecho("Ты здоровеешь!");
            oldact("Здоровье $c2 растет!\n\r",ch,0,0,TO_ROOM);
            wiznet(ch, victim, "здоровье", oldParam, ch->max_hit, victim->max_hit);
        }
    }
    if ( number_percent() * modifier / 100 > 100 )
    {
        int gain;
        const int max_mana = mylevel * 150;
        
        gain = max( 0, victim->max_mana - ch->max_mana );
        gain = gain * number_percent() * 20 / 10000; // 20% max

        if (gain > 0 && ch->max_mana < max_mana) {
            int oldParam = ch->max_mana;            
            ch->max_mana = min(ch->max_mana + gain, max_mana);
            ch->pecho("Ты чувствуешь прилив энергии!");
            oldact("$c1 наполняется энергией!\n\r",ch,0,0,TO_ROOM);
            wiznet(ch, victim, "ману", oldParam, ch->max_mana, victim->max_mana);
        }
    }

    if (victim->is_npc( )) {
        NPCharacter *vch = victim->getNPC( );
        const int max_dice = max(5, mylevel / 5);

        if ( number_percent() * modifier / 100 > 110 )
        {
            if (vch->damage[DICE_NUMBER] > ch->damage[DICE_NUMBER] && ch->damage[DICE_NUMBER] < max_dice) {
                int oldParam = ch->damage[DICE_NUMBER];
                ch->damage[DICE_NUMBER]++;
                ch->pecho("Теперь ты будешь наносить больше повреждений!");
                oldact("$c1 становится более опасн$gым|ым|ой!\n\r",ch,0,0,TO_ROOM);
                wiznet(ch, victim, "dice_number", oldParam, ch->damage[DICE_NUMBER], vch->damage[DICE_NUMBER]);
            }
        }
        if ( number_percent() * modifier / 100 > 110 )
        {
            if (vch->damage[DICE_TYPE] > ch->damage[DICE_TYPE] && ch->damage[DICE_TYPE] < max_dice) {
                int oldParam = ch->damage[DICE_TYPE];
                ch->damage[DICE_TYPE]++;
                ch->pecho("Теперь ты будешь наносить больше повреждений!");
                oldact("$c1 становится более опасн$gым|ым|ой!\n\r",ch,0,0,TO_ROOM);
                wiznet(ch, victim, "dice_type", oldParam, ch->damage[DICE_TYPE], vch->damage[DICE_TYPE]);
            }
        }
    }

    Object *wield;
    int dam_type = 0;
    wield = get_eq_char( victim, wear_wield );
    if ( wield && wield->item_type == ITEM_WEAPON )
        dam_type = attack_table[wield->value3()].damage;
    else
        dam_type = attack_table[victim->dam_type].damage;
    if ( dam_type == -1 )
        dam_type = DAM_BASH;
    switch (dam_type)
    {
    case DAM_BASH:
    case DAM_PIERCE:
    case DAM_SLASH:
        break;
    default:
        dam_type = 0;
        break;
    }
    if ( number_percent() * modifier / 100 > 100 )
    {
        int gain;

        gain = min( 0, victim->armor[dam_type] - ch->armor[dam_type]);
        gain = gain * number_percent() * 5 / 10000;
        
        if (gain < 0 && ch->armor[dam_type] > -10 * mylevel) {
            int oldParam = ch->armor[dam_type];
            ch->armor[dam_type] += gain;
            ch->pecho("Твоя защита улучшается!");
            oldact("Защита $c2 улучшается!\n\r",ch,0,0,TO_ROOM);
            wiznet(ch, victim, "броню", oldParam, ch->armor[dam_type], victim->armor[dam_type]);
        }
    }
    
    for (int i = 0; i < stat_table.size; i ++)
    {
        if ( number_bits(4) == 0
            && ch->perm_stat[i] < victim->perm_stat[i]
            && ch->perm_stat[i] < 25
            && number_percent() * modifier / 100 > 110 )
        {
            int oldParam = ch->perm_stat[i];
            ch->perm_stat[i] += 1;
            ch->pecho("Твои параметры улучшаются!");
            oldact("$c1 улучшает свои параметры!\n\r",ch,0,0,TO_ROOM);
            wiznet(ch, victim, stat_table.fields[i].name, oldParam, ch->perm_stat[i], victim->perm_stat[i]);
        }
    }
}

static void apply_align_changes( PCharacter *ch )
{
    Object *obj;
    Object *obj_next;

    for ( obj = ch->carrying; obj != 0; obj = obj_next )
    {
        obj_next = obj->next_content;
        if ( obj->wear_loc == wear_none )
                continue;

        if (obj->isAntiAligned( ch )) {
            oldact_p("Ты пытаешься использовать $o4, но это не для тебя.",
                ch, obj, 0, TO_CHAR,POS_RESTING );
            oldact_p("$c1 пытается использовать $o4, но оно $m не подходит.",
                ch, obj, 0, TO_ROOM,POS_RESTING );
            obj_from_char( obj );
            obj_to_room( obj, ch->in_room );
        }
    }
}

static bool can_influence_exp( PCharacter *gch, Character *leader )
{
    if (IS_GHOST( gch ))
        return false;

    if (gch->getModifyLevel( ) - leader->getModifyLevel( ) > GROUP_RANGE ) {
        gch->pecho("Ты слишком высокого уровня для этой группы.");
        return false;
    }

    if (gch->getModifyLevel( ) - leader->getModifyLevel( ) < -GROUP_RANGE ) {
        gch->pecho("Ты слишком низкого уровня для этой группы.");
        return false;
    }

    return true;
}

void group_gain( Character *ch, Character *victim, Character *realKiller )
{
    Character *gch;
    Character *leader;
    int xp, mobcount, base_exp_bonus;
    std::list<PCharacter *> players;
    std::list<PCharacter *>::iterator i;
    Character *killer = realKiller ? realKiller : ch;

    if ( victim == ch
        || ( victim->is_npc() && victim->getNPC()->pIndexData->vnum < 100 ) )
        return;

    if (killer->is_npc( )) 
        gain_exp_mob( killer->getNPC( ), victim );

    if (!victim->is_npc( ))
        return;

    if ( victim->is_npc()
        && ( victim->master != 0 || victim->leader != 0 ) )
        return;

    mobcount = 0;
    leader = (ch->leader != 0) ? ch->leader : ch;

    for (gch = ch->in_room->people; gch != 0; gch = gch->next_in_room) {
        if (is_same_group( gch, ch )) {
            if (gch->is_npc( ))
                mobcount++;
            else if (can_influence_exp( gch->getPC( ), leader ))
                players.push_back( gch->getPC( ) );
        }
    }

    if (players.empty( ))
        return;
    
    base_exp_bonus = victim->getNPC( )->behavior ? victim->getNPC( )->behavior->getExpBonus( leader ) : 0;

    for (i = players.begin( ); i != players.end( ); i++) {
        PCharacter *gch = *i;
        
        xp = xp_compute( gch, victim, mobcount, players.size( ), leader, base_exp_bonus );
        gch->pecho( "Ты получаешь %1$d очк%1$Iо|а|ов опыта за убийство %2$#C2.", xp, victim );
        gch->gainExp( xp );
        
        apply_align_changes( gch );
    }
}


/*
 * Calculate align-based coefficients applied to base exp.
 * Return 'true' if it looks like a good kill, deserving further bonuses.
 */
bool xp_align_coeff(Character *gch, Character *victim, int &align_mult, int &align_div)
{
    align_mult = 1;
    align_div = 1;

    // No change for 'no-align' mobs.
    if (IS_SET(victim->act,ACT_NOALIGN)) {
        return false;
    }
    
    // Opposite align - big bonus.
    if ((IS_EVIL(gch) && IS_GOOD(victim)) || (IS_EVIL(victim) && IS_GOOD(gch))) {
        align_mult = 8;
        align_div = 5;
        return true;
    }
    
    // Good vs good: penalize.
    if (IS_GOOD(gch) && IS_GOOD(victim)) {
        align_mult = 1;
        align_div = 5;
        return false;
    }
    
    // Neutral vs neutral: boring. 
    if (IS_NEUTRAL(gch) && IS_NEUTRAL(victim)) {
        align_mult = 3;
        align_div = 5;
        return false;
    }
    
    // Good vs neutral: penalize slightly.
    if (IS_GOOD(gch) && IS_NEUTRAL(victim)) {
        align_mult = 4;
        align_div = 5;
        return false;
    }

    return true;
}

/*
 * Compute xp for a kill.
 */
int xp_compute( PCharacter *gch, Character *victim, int npccount, int pccount, Character *leader, int base_exp_bonus )
{
  char buf[MAX_STRING_LENGTH];
  int xp;
  int base_exp;
  short level_range;
  int neg_cha=0, pos_cha=0;
    int align_mult, align_div;
    bool align_bonus;

  level_range = victim->getModifyLevel() - gch->getModifyLevel();

  switch (level_range)  {
  default :     base_exp =   0;     break;
  case -9 :     base_exp =   1;     break;
  case -8 :     base_exp =   2;     break;
  case -7 :     base_exp =   5;     break;
  case -6 :     base_exp =   9;     break;
  case -5 :     base_exp =  11;     break;
  case -4 :     base_exp =  22;     break;
  case -3 :     base_exp =  33;     break;
  case -2 :     base_exp =  43;     break;
  case -1 :     base_exp =  60;     break;
  case  0 :     base_exp =  74;     break;
  case  1 :     base_exp =  84;     break;
  case  2 :     base_exp =  99;     break;
  case  3 :     base_exp = 121;     break;
  case  4 :     base_exp = 143;     break;
  }

  if (level_range > 4)
    base_exp = 140 + 20 * (level_range - 4);
    
  base_exp += base_exp_bonus;


    // calculate and apply exp multiplier 
    align_bonus = xp_align_coeff(gch, victim, align_mult, align_div);
    xp = (int)((base_exp * align_mult) / align_div);
   

    // more exp at the low levels 
    if ( gch->getModifyLevel() < 6)
        xp += 50 / gch->getModifyLevel();

    //limit 
    xp = std::min( xp, 200 );
    
    // randomize the rewards 
    xp = number_range (xp * 3/4, xp * 5/4);

    // adjust for NPC
    xp = (int)(xp - xp / 20 * npccount);

    // adjust for PC
    int surpluspc = pccount > 5 ? pccount - 5 : 0;
    xp = xp - xp / 10 * surpluspc;

    xp = std::max( xp, 0 );

    // Leadership skill.
    if (leader && !leader->is_npc( ) 
        && leader->in_room == gch->in_room
        && xp > 10 
        && pccount > 1) 
    {
        int skill = gsn_leadership->getEffective( leader );
        int bonus = skill_level_bonus(*gsn_leadership, leader);
        
         if (number_percent( ) < (skill + bonus * 2) / 2) {
            xp += (xp * skill / 2) / 100;
            xp += bonus * 10;
            if (gch != leader)
                gch->pecho("{cБлагодаря умелому руководству %C2 ты получаешь больше опыта.{x", leader);
            else
                leader->pecho("{cБлагодаря твоему умелому руководству группа получает больше опыта.{x");
             
            gsn_leadership->improve( leader, true );        
        }
        else 
            gsn_leadership->improve( leader, false );
    }

    // Calendar bonuses: for now simply increase exp on 13th of each month.
    if (align_bonus && xp > 10 && bonus_experience->isActive(gch, time_info)) {
        ostringstream ostr;
        xp = number_range(xp + xp / 2, xp * 2);
        bonus_experience->reportAction(gch, ostr);
        gch->send_to(ostr);
    }
   

    // Kill counters and charisma update. 
    if (IS_GOOD(gch))
    {
     if (IS_GOOD(victim)) { gch->getPC( )->anti_killed++; neg_cha = 1; }
     else if (IS_NEUTRAL(victim)) {gch->getPC( )->has_killed++; pos_cha =1;}
     else if (IS_EVIL(victim)) {gch->getPC( )->has_killed++; pos_cha = 1;}
    }

    if (IS_NEUTRAL(gch))
    {
     if (xp > 0)
     {
      if (IS_GOOD(victim)) {gch->getPC( )->has_killed++; pos_cha = 1;}
      else if (IS_NEUTRAL(victim)) {gch->getPC( )->anti_killed++; neg_cha = 1;}
      else if (IS_EVIL(victim)) {gch->getPC( )->has_killed++; pos_cha =1;}
     }
    }

    if (IS_EVIL(gch))
    {
     if (xp > 0)
     {
      if (IS_GOOD(victim)) {gch->getPC( )->has_killed++; pos_cha = 1;}
      else if (IS_NEUTRAL(victim)) {gch->getPC( )->has_killed++; pos_cha = 1;}
      else if (IS_EVIL(victim)) {gch->getPC( )->anti_killed++; neg_cha = 1;}
     }
    }

 if ( neg_cha )
  {
    if ( (gch->getPC( )->anti_killed % 100) == 99 )
    {
     sprintf(buf,"На твоем счету %d ТРУПОВ %s.\n\r",
            gch->getPC( )->anti_killed.getValue( ),
        IS_GOOD(gch) ? "goods" :
        IS_NEUTRAL(gch) ? "neutrals" :
        IS_EVIL(gch) ? "evils" : "nones" );
     gch->send_to(buf);
     if (gch->perm_stat[STAT_CHA] > 3 && IS_GOOD(gch) )
     {
      gch->pecho("Твое обаяние (charisma) понизилось на единицу.");
      gch->perm_stat[STAT_CHA] -= 1;
     }
    }
   }
  else if ( pos_cha )
   {
    if ( (gch->getPC( )->has_killed % 200) == 199 )
    {
     sprintf(buf,"На твоем счету %d ТРУПОВ %s.\n\r",
            gch->getPC( )->has_killed.getValue( ),
        IS_GOOD(gch) ? "anti-goods" :
        IS_NEUTRAL(gch) ? "anti-neutrals" :
        IS_EVIL(gch) ? "anti-evils" : "nones" );
      gch->send_to(buf);
      if (gch->perm_stat[STAT_CHA] < gch->getPC( )->getMaxTrain( STAT_CHA )
        && IS_GOOD(gch) )
      {
       gch->pecho("Твое обаяние{le (charisma){x повысилось на единицу.");
       gch->perm_stat[STAT_CHA] += 1;
      }
     }
   }

    if (gch->getProfession( ) == prof_samurai && gch == leader 
        && gch->perm_stat[STAT_CHA] < gch->getPC( )->getMaxTrain( STAT_CHA )
        && victim->getModifyLevel( ) - gch->getModifyLevel( ) >= 20 
        && chance( 10 ))
    {
        oldact("Ты уби$gло|л|ла достойного противника, и твое обаяние{le (charisma){x повысилось на единицу.", gch, 0, 0, TO_CHAR);
        gch->perm_stat[STAT_CHA] += 1;
    }
    
    return xp;
}

