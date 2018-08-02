/* $Id: fight_exp.cpp,v 1.1.2.6 2008/11/13 02:32:36 rufina Exp $
 *
 * ruffina, 2004
 */
/***************************************************************************
 * Все права на этот код 'Dream Land' пренадлежат Igor {Leo} и Olga {Varda}*
 * Некоторую помощь в написании этого кода, а также своими идеями помогали:*
 *    Igor S. Petrenko	    {NoFate, Demogorgon}                           *
 *    Koval Nazar	    {Nazar, Redrum}                 		   *
 *    Doropey Vladimir	    {Reorx}		                           *
 *    Kulgeyko Denis	    {Burzum}		                           *
 *    Andreyanov Aleksandr  {Manwe}		                           *
 *    и все остальные, кто советовал и играл в этот MUD	                   *
 ***************************************************************************/

#include "skill.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"
#include "object.h"
#include "gsn_plugin.h"
#include "merc.h"
#include "mercdb.h"
#include "handler.h"
#include "fight.h"
#include "act.h"
#include "def.h"

PROF(samurai);

int xp_compute( Character *gch, Character *victim, int npccount, int pccount, Character *leader, int base_exp_bonus );

#ifndef FIGHT_STUB
void gain_exp_mob( NPCharacter *ch, Character *victim )
{
    int modifier = 100;
    int diff = victim->getRealLevel() - ch->getRealLevel();
    
    if ( diff > 15 )
	modifier = 150;
    else if ( diff > 5 )
	modifier = 125;
    else if ( diff > 0 )
	modifier = 110;
    else if ( diff > -5 )
	modifier = 105;
	
    if ( number_percent() * modifier / 100 > 110 )
    {
	if (victim->getRealLevel() > ch->getRealLevel()) {
	    ch->setLevel( ch->getRealLevel() + 1 );
	    ch->send_to("Ты получаешь уровень!\n\r");
	    act_p("$c1 выглядит более опытн$gым|ым|ой!\n\r",ch,0,0,TO_ROOM,POS_RESTING);

            // Restore act bits that are removed for low-level adaptive pets.
            if (ch->getRealLevel( ) > 20) {
                if (IS_SET(ch->pIndexData->act, ACT_CLERIC))
                    SET_BIT(ch->act, ACT_CLERIC);
                if (IS_SET(ch->pIndexData->act, ACT_MAGE))
                    SET_BIT(ch->act, ACT_MAGE);
            }
	}
    }
    if ( number_percent() * modifier / 100 > 90 )
    {
	if (victim->hitroll > ch->hitroll) {
	    ch->hitroll++;
	    ch->send_to("Теперь ты будешь попадать более метко!\n\r");
	    act_p("$c1 становится более метк$gим|им|ой!\n\r",ch,0,0,TO_ROOM,POS_RESTING);
	}
    }
    if ( number_percent() * modifier / 100 > 110 )
    {
	if (victim->damroll > ch->damroll) {
	    ch->damroll++;
	    ch->send_to("Теперь ты будешь больнее бить!\n\r");
	    act_p("$c1 теперь будет больнее бить!\n\r",ch,0,0,TO_ROOM,POS_RESTING);
	}
    }
    if ( number_percent() * modifier / 100 > 100 )
    {
	int gain;
	
	gain = max( 0, victim->max_hit - ch->max_hit );
	gain = gain * number_percent() * 20 / 10000;

	if (gain > 0) {
	    ch->max_hit += gain;
	    ch->send_to("Ты здоровеешь!\n\r");
	    act_p("Здоровье $c2 растет!\n\r",ch,0,0,TO_ROOM,POS_RESTING);
	}
    }
    if ( number_percent() * modifier / 100 > 100 )
    {
	int gain;
	
	gain = max( 0, victim->max_mana - ch->max_mana );
	gain = gain * number_percent() * 20 / 10000;

	if (gain > 0) {
	    ch->max_mana += gain;
	    ch->send_to("Ты чувствуешь прилив энергии!\n\r");
	    act_p("$c1 наполняется энергией!\n\r",ch,0,0,TO_ROOM,POS_RESTING);
	}
    }

    if (victim->is_npc( )) {
	NPCharacter *vch = victim->getNPC( );

	if ( number_percent() * modifier / 100 > 110 )
	{
	    if (vch->damage[DICE_NUMBER] > ch->damage[DICE_NUMBER]) {
		ch->damage[DICE_NUMBER]++;
		ch->send_to("Теперь ты будешь наносить больше повреждений!\n\r");
		act_p("$c1 становится более опасн$gым|ым|ой!\n\r",ch,0,0,TO_ROOM,POS_RESTING);
	    }
	}
	if ( number_percent() * modifier / 100 > 110 )
	{
	    if (vch->damage[DICE_TYPE] > vch->damage[DICE_TYPE]) {
		ch->damage[DICE_TYPE]++;
		ch->send_to("Теперь ты будешь наносить больше повреждений!\n\r");
		act_p("$c1 становится более опасн$gым|ым|ой!\n\r",ch,0,0,TO_ROOM,POS_RESTING);
	    }
	}
    }

    Object *wield;
    int dam_type = 0;
    wield = get_eq_char( victim, wear_wield );
    if ( wield && wield->item_type == ITEM_WEAPON )
	dam_type = attack_table[wield->value[3]].damage;
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
	
	if (gain < 0) {
	    ch->armor[dam_type] += gain;
	    ch->send_to("Твоя защита улучшается!\n\r");
	    act_p("Защита $c2 улучшается!\n\r",ch,0,0,TO_ROOM,POS_RESTING);
	}
    }
    
    for (int i = 0; i < stat_table.size; i ++)
    {
	if ( number_bits(4) == 0
	    && number_percent() * modifier / 100 > 110 )
	{
	    ch->perm_stat[i] = min(25,ch->perm_stat[i] + 1);
	    ch->send_to("Твои параметры улучшаются!\n\r");
	    act_p("$c1 улучшает свои параметры!\n\r",ch,0,0,TO_ROOM,POS_RESTING);
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
	    act_p( "Ты пытаешься использовать $o4, но это не для тебя.",
		ch, obj, 0, TO_CHAR,POS_RESTING );
	    act_p( "$c1 пытается использовать $o4, но оно $m не подходит.",
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

    if (gch->getModifyLevel( ) - leader->getModifyLevel( ) > 8 ) {
	gch->send_to("Ты слишком высокого уровня для этой группы.\n\r");
	return false;
    }

    if (gch->getModifyLevel( ) - leader->getModifyLevel( ) < -8 ) {
	gch->send_to("Ты слишком низкого уровня для этой группы.\n\r");
	return false;
    }

    return true;
}

void group_gain( Character *ch, Character *victim )
{
    Character *gch;
    Character *leader;
    int xp, mobcount, base_exp_bonus;
    std::list<PCharacter *> players;
    std::list<PCharacter *>::iterator i;

    if ( victim == ch
	|| ( victim->is_npc() && victim->getNPC()->pIndexData->vnum < 100 ) )
	return;

    if (ch->is_npc( )) 
	gain_exp_mob( ch->getNPC( ), victim );

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
	gch->printf( "Ты получаешь %d очков опыта.\n\r", xp );
	gch->gainExp( xp );
	
	apply_align_changes( gch );
    }
}



/*
 * Compute xp for a kill.
 */
int xp_compute( Character *gch, Character *victim, int npccount, int pccount, Character *leader, int base_exp_bonus )
{
  char buf[MAX_STRING_LENGTH];
  int xp;
  int base_exp;
  short level_range;
  int neg_cha=0, pos_cha=0;

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

 // calculate exp multiplier 
  if (IS_SET(victim->act,ACT_NOALIGN))
    xp = base_exp;

  // alignment 
  else 
    if ( (IS_EVIL(gch) && IS_GOOD(victim)) 
    || (IS_EVIL(victim) && IS_GOOD(gch)))
    xp = ( int )( ( base_exp * 8 ) / 5 );

  else if ( IS_GOOD(gch) && IS_GOOD(victim) )
    xp = ( int ) base_exp / 5;

  else if ( IS_NEUTRAL(gch) && IS_NEUTRAL(victim) )
    xp = ( int ) base_exp * 3 / 5;

  else if ( IS_GOOD(gch) && IS_NEUTRAL(victim) )
    xp = ( int )( base_exp * 4 / 5 );

  else xp = base_exp;

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

    if (leader && !leader->is_npc( ) 
	&& leader->in_room == gch->in_room
	&& xp > 10 
	&& pccount > 1) 
    {
	int skill = gsn_leadership->getEffective( leader );
	
	 if (number_percent( ) < skill / 2) {
	    xp += (xp * skill / 2) / 100;
	    act_p( "{cБлагодаря умелому руководству $C2 ты получаешь больше опыта.{x",
			gch, 0, leader, TO_CHAR, POS_RESTING );
	    gsn_leadership->improve( leader, true );	
	}
	else 
	    gsn_leadership->improve( leader, false );
    }
    
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
      gch->send_to("Твое обаяние (charisma) понизилось на единицу.\n\r");
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
       gch->send_to("Твое обаяние (charisma) повысилось на единицу.\n\r");
       gch->perm_stat[STAT_CHA] += 1;
      }
     }
   }

    if (gch->getProfession( ) == prof_samurai && gch == leader 
	&& gch->perm_stat[STAT_CHA] < gch->getPC( )->getMaxTrain( STAT_CHA )
	&& victim->getModifyLevel( ) - gch->getModifyLevel( ) >= 20 
	&& chance( 10 ))
    {
	act("Ты уби$gло|л|ла достойного противника, и твое обаяние (charisma) повысилось на единицу.", gch, 0, 0, TO_CHAR);
	gch->perm_stat[STAT_CHA] += 1;
    }
    
    return xp;
}

#else
void	group_gain( Character *ch, Character *victim ) { }
#endif
