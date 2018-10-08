/* $Id: class_ranger.cpp,v 1.1.2.25.4.25 2010-09-01 21:20:44 rufina Exp $
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
#include "summoncreaturespell.h"
#include "class_ranger.h"
#include "objthrow.h"

#include "skill.h"
#include "spelltarget.h"
#include "spelltemplate.h"
#include "affecthandlertemplate.h"
#include "skillcommandtemplate.h"
#include "skillmanager.h"

#include "pcharactermanager.h"
#include "affect.h"
#include "pcharacter.h"
#include "room.h"
#include "npcharacter.h"
#include "room.h"
#include "object.h"

#include "dreamland.h"
#include "gsn_plugin.h"
#include "act_move.h"
#include "mercdb.h"

#include "magic.h"
#include "fight.h"
#include "stats_apply.h"
#include "onehit.h"
#include "onehit_weapon.h"
#include "damage_impl.h"
#include "vnum.h"
#include "merc.h"
#include "handler.h"
#include "save.h"
#include "act.h"
#include "interp.h"
#include "def.h"

PROF(ranger);

static Object * create_arrow( int color, int level );
static Object * find_arrow( Character *ch, Object *quiver );


#define OBJ_VNUM_RANGER_STAFF        28
#define OBJ_VNUM_RANGER_ARROW        6
#define OBJ_VNUM_RANGER_BOW          7

/*
 * 'track' skill command
 */

SKILL_RUNP( track )
{
    DLString arg( argument );
    EXIT_DATA *pexit;
    int d;

    if (gsn_track->getEffective( ch ) < 2) {
	ch->send_to("Здесь нет следов.\r\n");
	return;
    }

    if (arg.empty( )) {
	ch->println( "Кого ты хочешь выследить?" );
	return;
    }

    ch->setWait( gsn_track->getBeats( ) );
    act_p("$c1 всматривается в землю в поисках следов.",ch,0,0,TO_ROOM,POS_RESTING);

    if (number_percent() < gsn_track->getEffective( ch ))
	if (( d = ch->in_room->history.went( arg, false ) ) != -1)
	    if (( pexit = ch->in_room->exit[d] )) {
		gsn_track->improve( ch, true );
		ch->printf( "%s's tracks lead %s.\r\n", arg.c_str( ), dirs[d].name );
		
		if (IS_SET(pexit->exit_info, EX_CLOSED)) 
		    open_door_extra( ch, d, pexit );
		
		move_char(ch, d );
		return;
	    }
    
    ch->send_to("Ты не видишь здесь следов.\n\r");
    gsn_track->improve( ch, false );
}

/*
 * find an arrow in the quiver
 */
static Object * find_arrow( Character *ch, Object *quiver )
{
    Object *arrow = NULL;

    for (Object *obj = quiver->contains; obj != 0; obj = obj->next_content)
	if (obj->item_type == ITEM_WEAPON && obj->value[0] == WEAPON_ARROW) {
	    arrow = obj;
	    break; 
	}

    if (!arrow) {
	act("В $o6 закончились стрелы.", ch, quiver, 0, TO_CHAR);
	return NULL;
    }

    if (ch->getRealLevel( ) + 10 < arrow->level) {
	ch->println("Тебе не хватает опыта воспользоватся этой стрелой.");
	return NULL;
    }

    obj_from_obj( arrow );
    return arrow;
}

/*
 * 'shoot' command for skill 'bow' 
 */

SKILL_RUNP( shoot )
{
    Character *victim;
    Object *wield;
    Object *arrow;
    Object *quiver;

    char arg1[512],arg2[512];
    bool success;
    int chance,direction;
    int range, range0 = ( ch->getModifyLevel() / 10) + 1;
    int master_shoots = 2;

    if (!gsn_bow->usable( ch ))
    {
	  ch->send_to("Ты не умеешь стрелять из лука.\n\r");
	  return;
    }

    argument=one_argument( argument, arg1 );
    one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
	    ch->send_to("Выстрелить в каком направлении и в кого?\n\r");
	    return;
    }

    if ( ch->fighting )
    {
	    ch->send_to("Сражаясь, ты не можешь прицелиться.\n\r");
	    return;
    }

    direction = direction_lookup( arg1 );

    if (direction < 0)
    {
	    ch->send_to("Выстрелить в каком направлении и в кого?\n\r");
	    return;
    }
    
    range = range0;
    if ( ( victim = find_char( ch, arg2, direction, &range) ) == 0 )
    {
	    ch->send_to("Там таких нет.\n\r");
	    return;
    }

    if ( !victim->is_npc() && victim->desc == 0 )
    {
	    ch->send_to("Ты не можешь сделать этого.\n\r");
	    return;
    }

    if ( victim == ch )
    {
	    ch->send_to("Это невозможно!\n\r");
	    return;
    }

    if (is_safe(ch,victim))
    {
	    ch->pecho("Боги покровительствуют %C3.", victim);
	    return;
    }

    
    wield = get_eq_char(ch, wear_wield);
    quiver = get_eq_char(ch, wear_hold);

    if ( !wield
	    || wield->item_type != ITEM_WEAPON
	    || wield->value[0] != WEAPON_BOW )
    {
	    ch->send_to("Для того, чтобы стрелять тебе нужен лук!\n\r");
	    return;    	
    }

    if (!ch->is_npc( ) 
	&& (get_eq_char(ch,wear_second_wield)
	    || get_eq_char(ch,wear_shield)) )
    {
	    ch->send_to("Твоя вторая рука должна быть свободна!\n\r");
	    return;    	
    }

    if (!ch->is_npc( ) && !quiver)
    {
	    ch->send_to("У тебя в руках ничего нет!\n\r");
	    return;    	
    }	
    
    if (!ch->is_npc( ) && 
	(quiver->item_type != ITEM_CONTAINER || !IS_SET(quiver->value[1], CONT_FOR_ARROW)))
    {
	    ch->send_to("Возьми в руки колчан.\n\r");
	    return;
    }

    if ( ch->in_room == victim->in_room )
    {
	    ch->send_to("Ты не можешь стрелять из лука в упор.\n\r");
	    return;
    }

    if (ch->is_npc( )) {
	arrow = create_arrow( 0, ch->getModifyLevel( ) );
	arrow->timer = 1;
    }
    else 
	arrow = find_arrow( ch, quiver );
    
    if (!arrow)
	return;

    ch->setWait( gsn_bow->getBeats( )  );

    chance = (gsn_bow->getEffective( ch ) - 50) * 2;
    if ( victim->position == POS_SLEEPING )
	    chance += 40;
    if ( victim->position == POS_RESTING )
	    chance += 10;
    if ( victim->position == POS_FIGHTING )
	    chance -= 40;
    chance += ch->hitroll;
    
    ch->pecho( "%1$^O1, посланн%1$Gое|ый|ая тобой, улетела %2$s.", arrow, dirs[ direction ].leave );
    ch->recho( "%1$^O1, посланн%1$Gое|ый|ая %3$C5, улетела %2$s.", arrow, dirs[ direction ].leave, ch );

    set_violent( ch, victim, false );
    
    try {
	success = send_arrow( ch, victim, arrow,
			      direction, chance,
			      dice( wield->value[1], wield->value[2] ) );
    } catch (const VictimDeathException &e) {
	return;
    }
    
    gsn_bow->improve( ch, success, victim );
    
    yell_panic( ch, victim,
                "Помогите! Меня кто-то обстреливает!",
		"Помогите! Меня обстреливает %1$C1!",
		FYP_VICT_ANY );
    
    if (ch->is_npc( ))
	return;

    if (number_percent() >= gsn_mastering_bow->getEffective( ch )) {
	gsn_mastering_bow->improve( ch, false, victim );
	return;
    }
    
    for (int i = 0; i < master_shoots; i++) {
	range = range0;
	if (find_char( ch, arg2, direction, &range) != victim)
	    return; 
	
	if (!( arrow = find_arrow( ch, quiver ) ))
	    return;
	
	try {
	    success = send_arrow( ch, victim, arrow,
				  direction, chance,
				  dice( wield->value[1], wield->value[2] ) );
	} catch (const VictimDeathException &e) {
	    return;
	}

	gsn_mastering_bow->improve( ch, success, victim );
    }
}


/*
 * 'herbs' skill command
 */

SKILL_RUNP( herbs )
{
  Character *victim;
  char arg[MAX_INPUT_LENGTH];

  one_argument(argument,arg);

  if (ch->isAffected(gsn_herbs))
    {
      ch->send_to("Ты пока не можешь искать травы.\n\r");
      return;
    }

  if (arg[0] == '\0')
    victim = ch;
  else if ( (victim = get_char_room(ch,arg)) == 0)
    {
      ch->send_to("Таких тут нет.\n\r");
      return;
    }
  ch->setWait( gsn_herbs->getBeats( )  );

  if ((ch->in_room->sector_type == SECT_FIELD
       || ch->in_room->sector_type == SECT_FOREST
       || ch->in_room->sector_type == SECT_HILLS
       || ch->in_room->sector_type == SECT_MOUNTAIN)
      && number_percent() < gsn_herbs->getEffective( ch ))
    {
      Affect af;
      af.where  = TO_AFFECTS;
      af.type 	= gsn_herbs;
      af.level 	= ch->getModifyLevel();
      af.duration = 5;
      af.location = APPLY_NONE;
      af.modifier = 0;
      af.bitvector = 0;

      affect_to_char(ch,&af);

      ch->send_to("Ты собираешь целебные травы.\n\r");
      act_p("$c1 собирает какие-то травы.",ch,0,0,TO_ROOM,POS_RESTING);

      if (ch != victim)
	{
	  act_p("$c1 дает тебе покушать травы.",ch,0,victim,TO_VICT,POS_RESTING);
	  act_p("Ты даешь травы $C3.",ch,0,victim,TO_CHAR,POS_RESTING);
	  act_p("$c1 дает травы $C3.",ch,0,victim,TO_NOTVICT,POS_RESTING);
	}
	
      if (victim->hit < victim->max_hit)
	{
	  victim->send_to("Ты чувствуешь себя лучше.\n\r");
	  act_p("$c1 выглядит лучше.",victim,0,0,TO_ROOM,POS_RESTING);
	}
      victim->hit = min((int)victim->max_hit,victim->hit + 5 * ch->getModifyLevel() );
      gsn_herbs->improve( ch, true, victim );
      
      checkDispel( ch->getModifyLevel( ), victim, gsn_plague );
      checkDispel( ch->getModifyLevel( ), victim, gsn_poison );
    }
  else
    {
      ch->send_to("Ты ищешь травы, но ничего не находишь.\n\r");
      act_p("$c1 озирается в поисках травы.",ch,0,0,TO_ROOM,POS_RESTING);
      gsn_herbs->improve( ch, false, victim );
    }
}

/*
 * 'camp' skill command
 */

SKILL_RUNP( camp )
{
  Affect af,af2;

  if (ch->is_npc() || !gsn_camp->usable( ch ) )
    {
      ch->send_to( "Чего?\n\r");
      return;
    }

  if (ch->isAffected(gsn_camp))
    {
      ch->println("У тебя нет сил разбить новый лагерь.");
      return;
    }


  if ( number_percent( ) > gsn_camp->getEffective( ch ) )
  {
	ch->println("Ты пытаешься разбить лагерь, но у тебя ничего не получается.");
	gsn_camp->improve( ch, true );
	return;
  }

  if ( IS_SET(ch->in_room->room_flags, ROOM_SAFE)      ||
       IS_SET(ch->in_room->room_flags, ROOM_PRIVATE)   ||
       IS_SET(ch->in_room->room_flags, ROOM_SOLITARY)  ||
         ( ch->in_room->sector_type != SECT_FIELD &&
           ch->in_room->sector_type != SECT_FOREST &&
           ch->in_room->sector_type != SECT_MOUNTAIN &&
           ch->in_room->sector_type != SECT_HILLS ) )
  {
    ch->println("Здесь недостаточно растительности для разбивки лагеря.");
    return;
  }

  if ( ch->mana < gsn_camp->getMana( ))
  {
     ch->println("У тебя не хватает энергии для разбивки лагеря.");
     return;
  }

  gsn_camp->improve( ch, true );
  ch->mana -= gsn_camp->getMana( );
  ch->setWait( gsn_camp->getBeats( ) );

  act("Ты разбиваешь лагерь.", ch, 0, 0, TO_CHAR);
  act("$c1 разбивает лагерь.", ch, 0, 0, TO_ROOM);

  af.where		= TO_AFFECTS;
  af.type               = gsn_camp;
  af.level              = ch->getModifyLevel();
  af.duration           = 12;
  af.bitvector          = 0;
  af.modifier           = 0;
  af.location           = APPLY_NONE;
  affect_to_char(ch, &af);

  af2.where		= TO_ROOM_CONST;
  af2.type              = gsn_camp;
  af2.level              = ch->getModifyLevel();
  af2.duration           = ch->getModifyLevel() / 20;
  af2.bitvector          = 0;
  af2.modifier           = 2 * ch->getModifyLevel();
  af2.location           = APPLY_ROOM_HEAL;
  ch->in_room->affectTo( &af2);

  af2.modifier           = ch->getModifyLevel();
  af2.location           = APPLY_ROOM_MANA;
  ch->in_room->affectTo( &af2);

}

AFFECT_DECL(Camp);
VOID_AFFECT(Camp)::toStream( ostringstream &buf, Affect *paf ) 
{
    if (!paf->next || paf->next->type != gsn_camp)
	return;

    buf << fmt( 0, "Здесь разбит лагерь, который в течение {W%1$d{x ча%1$Iса|сов|сов "
                   "улучшает восстановление здоровья на {W%2$d{x и маны на {W%3$d{x.",
		   paf->duration, paf->modifier, paf->next->modifier )
	<< endl;
}


/*
 * 'bear call' skill command
 */

SKILL_RUNP( bearcall )
{
    SpellTarget::Pointer target( NEW );
    ostringstream errbuf;

  if (!gsn_bear_call->usable( ch ) )
    {
      ch->send_to( "Чего?\n\r");
      return;
    }

  if ( ch->mana < gsn_bear_call->getMana( ))
  {
     ch->send_to( "У тебя не хватает сил, чтобы крикнуть медведям.\n\r");
     return;
  }

  if (!( target = gsn_bear_call->getSpell( )->locateTargets( ch, argument, errbuf ) )) {
      ch->send_to( errbuf );
      return;
  }

  if ( number_percent( ) > gsn_bear_call->getEffective( ch ) )
  {
	ch->send_to( "Медведи не слушают тебя.\n\r");
	gsn_bear_call->improve( ch, false );
	return;
  }
    
  gsn_bear_call->getSpell( )->run( ch, target, min( 100, ch->getModifyLevel( ) - 2 ) );

  ch->mana -= gsn_bear_call->getMana( );
  ch->setWait( gsn_bear_call->getBeats( ) );
  gsn_bear_call->improve( ch, true );
}

SPELL_DECL_T(BearCall, SummonCreatureSpell);
TYPE_SPELL(NPCharacter *, BearCall)::createMobile( Character *ch, int level ) const 
{
    return createMobileAux( ch, level, 
	                 (ch->is_npc( ) ? ch->max_hit : ch->getPC( )->perm_hit), 
			 (ch->is_npc( ) ? ch->max_mana : ch->getPC( )->perm_mana),
			 number_range(level/15, level/10),
			 number_range(level/3, level/2),
			 number_range(level/8, level/6) );
}

TYPE_SPELL(bool, BearCall)::canSummonHere( Character *ch ) const 
{
  if ( ch->in_room != 0 && IS_SET(ch->in_room->room_flags, ROOM_NO_MOB) )
  {
     ch->send_to( "Здесь медведи не услышат тебя.\n\r");
     return false;
  }

  if ( IS_SET(ch->in_room->room_flags, ROOM_SAFE)      ||
       IS_SET(ch->in_room->room_flags, ROOM_PRIVATE)   ||
       IS_SET(ch->in_room->room_flags, ROOM_SOLITARY)  ||
       (ch->in_room->exit[0] == 0 &&
          ch->in_room->exit[1] == 0 &&
          ch->in_room->exit[2] == 0 &&
          ch->in_room->exit[3] == 0 &&
          ch->in_room->exit[4] == 0 &&
          ch->in_room->exit[5] == 0) ||

         ( ch->in_room->sector_type != SECT_FIELD &&
           ch->in_room->sector_type != SECT_FOREST &&
           ch->in_room->sector_type != SECT_MOUNTAIN &&
           ch->in_room->sector_type != SECT_HILLS ) )
  {
    ch->send_to( "Медведи не пришли к тебе на помощь.\n\r");
    return false;
  }

  return true;
}    

/*
 * 'lion call' skill command
 */

SKILL_RUNP( lioncall )
{
    SpellTarget::Pointer target( NEW );
    ostringstream errbuf;

  if (!gsn_lion_call->usable( ch ) )
    {
          ch->send_to( "Чего?\n\r");
          return;
        }

  if ( ch->mana < gsn_lion_call->getMana( ))
  {
       ch->send_to( "У тебя не хватает сил, чтобы позвать львов.\n\r");
       return;
    }

  if (!( target = gsn_lion_call->getSpell( )->locateTargets( ch, argument, errbuf ) )) {
        ch->send_to( errbuf );
        return;
    }

  if ( number_percent( ) > gsn_lion_call->getEffective( ch ) )
  {
    ch->send_to( "Львы не слушают тебя.\n\r");
    gsn_lion_call->improve( ch, false );
    return;
    }
    
  gsn_lion_call->getSpell( )->run( ch, target, min( 100, ch->getModifyLevel( ) - 2 ) );

  ch->mana -= gsn_lion_call->getMana( );
  ch->setWait( gsn_lion_call->getBeats( ) );
  gsn_lion_call->improve( ch, true );
}

SPELL_DECL_T(LionCall, SummonCreatureSpell);
TYPE_SPELL(NPCharacter *, LionCall)::createMobile( Character *ch, int level ) const 
{
    return createMobileAux( ch, level, 
	                 (ch->is_npc( ) ? ch->max_hit : ch->getPC( )->perm_hit), 
			 (ch->is_npc( ) ? ch->max_mana : ch->getPC( )->perm_mana),
			 number_range(level/15, level/10),
			 number_range(level/3, level/2),
			 number_range(level/8, level/6) );
}

TYPE_SPELL(bool, LionCall)::canSummonHere( Character *ch ) const 
{
  if ( ch->in_room != 0 && IS_SET(ch->in_room->room_flags, ROOM_NO_MOB) )
  {
     ch->send_to( "Здесь львы не услышат тебя.\n\r");
     return false;
  }

  if ( IS_SET(ch->in_room->room_flags, ROOM_SAFE)      ||
       IS_SET(ch->in_room->room_flags, ROOM_PRIVATE)   ||
       IS_SET(ch->in_room->room_flags, ROOM_SOLITARY)  ||
       (ch->in_room->exit[0] == 0 &&
          ch->in_room->exit[1] == 0 &&
          ch->in_room->exit[2] == 0 &&
          ch->in_room->exit[3] == 0 &&
          ch->in_room->exit[4] == 0 &&
          ch->in_room->exit[5] == 0) ||

         ( ch->in_room->sector_type != SECT_FIELD &&
           ch->in_room->sector_type != SECT_FOREST &&
           ch->in_room->sector_type != SECT_MOUNTAIN &&
           ch->in_room->sector_type != SECT_HILLS ) )
  {
    ch->send_to( "Львы не пришли к тебе на помощь.\n\r");
    return false;
  }

  return true;
}    

/*
 * 'make arrow' skill 
 */
static Object * create_arrow( int color, int level )
{
    Object *arrow;
    Affect tohit, todam;
    const char *str_long, *str_short, *str_name;

    arrow = create_object(get_obj_index(OBJ_VNUM_RANGER_ARROW), 0 );
    arrow->level = level;

    tohit.where		     = TO_OBJECT;
    tohit.type               = gsn_make_arrow;
    tohit.level              = level;
    tohit.duration           = -1;
    tohit.location           = APPLY_HITROLL;
    tohit.modifier           = level / 10;
    tohit.bitvector          = 0;
    affect_to_obj( arrow, &tohit);

    todam.where		     = TO_OBJECT;
    todam.type               = gsn_make_arrow;
    todam.level              = level;
    todam.duration           = -1;
    todam.location           = APPLY_DAMROLL;
    todam.modifier           = level / 10;
    todam.bitvector          = 0;
    affect_to_obj( arrow, &todam);

    if (color != 0 && color != gsn_make_arrow)
    {
	Affect saf;

	saf.where	       = TO_WEAPON;
	saf.type               = color;
	saf.level              = level;
	saf.duration           = -1;
	saf.location           = 0;
	saf.modifier           = 0;

	if ( color == gsn_green_arrow )
	{
	    saf.bitvector	= WEAPON_POISON;
	    str_name = "green зеленая";
	    str_long = "{GЗеленая";
	    str_short = "{Gзелен|ая|ой|ой|ую|ой|ой";
	    arrow->value[1] = 4 + level / 12;
	    arrow->value[2] = 4 + level / 10;
	}
	else if (color == gsn_red_arrow)
	{
	    saf.bitvector	= WEAPON_FLAMING;
	    str_name = "red красная";
	    str_long = "{RКрасная";
	    str_short = "{Rкрасн|ая|ой|ой|ую|ой|ой";
	    arrow->value[1] = 4 + level / 15;
	    arrow->value[2] = 4 + level / 30;
	}
	else if (color == gsn_white_arrow)
	{
	    saf.bitvector	= WEAPON_FROST;
	    str_name = "white белая";
	    str_long = "{WБелая";
	    str_short = "{Wбел|ая|ой|ой|ую|ой|ой";
	    arrow->value[1] = 4 + level / 15;
	    arrow->value[2] = 4 + level / 30;
	}
	else
	{
	    saf.bitvector	= WEAPON_SHOCKING;
	    str_name = "blue голубая";
	    str_long = "{CГолубая";
	    str_short = "{Cголуб|ая|ой|ой|ую|ой|ой";
	    arrow->value[1] = 4 + level / 15;
	    arrow->value[2] = 4 + level / 30;
	}

	affect_to_obj( arrow, &saf);
    }
    else
    {
	str_name = "wooden деревянная";
	str_long = "{yДеревянная";
	str_short = "{yдеревянн|ая|ой|ой|ую|ой|ой";
	arrow->value[1] = 4 + level / 12;
	arrow->value[2] = 4 + level / 10;
    }

    arrow->fmtName( arrow->getName( ), str_name );
    arrow->fmtShortDescr( arrow->getShortDescr( ), str_short );	
    arrow->fmtDescription( arrow->getDescription( ), str_long );	
    
    return arrow;
}

/*
 * 'make arrow' skill command
 */

SKILL_RUNP( makearrow )
{
    Skill *arrowSkill;
    int count,mana,wait;
    char arg[MAX_INPUT_LENGTH];

    if (ch->is_npc())
	return;

    if (!gsn_make_arrow->usable( ch )) {
	ch->println("Ты не умеешь изготавливать стрелы.");
	return;
    }

    if ( ch->in_room->sector_type != SECT_FIELD
	    && ch->in_room->sector_type != SECT_FOREST
	    && ch->in_room->sector_type != SECT_HILLS )
    {
	ch->send_to( "Здесь нет ни кусочка дерева (кроме тебя)! Попробуй сделать это в лесу!\n\r");
	return;
    }

    mana = gsn_make_arrow->getMana( );
    wait = gsn_make_arrow->getBeats( );

    argument = one_argument(argument, arg);
    
    if (arg[0] == '\0')	{
	arrowSkill = &*gsn_make_arrow;
    }
    else { 
        if (arg_oneof(arg, "green", "зеленая", "зеленые"))
	    arrowSkill = &*gsn_green_arrow;
        else if (arg_oneof(arg, "red", "красная", "красные"))
	    arrowSkill = &*gsn_red_arrow;
        else if (arg_oneof(arg, "white", "белая", "белые"))
	    arrowSkill = &*gsn_white_arrow;
        else if (arg_oneof(arg, "blue", "голубая", "голубые"))
	    arrowSkill = &*gsn_blue_arrow;
	else {
	    ch->send_to("Можно изготовить только зеленые, красные, белые или голубые стрелы.\n\r");
	    return;
	}

	if (!arrowSkill->usable( ch )) {
	    ch->send_to("Ты не умеешь изготавливать такие стрелы.\n\r");
	    return;
	}

	mana += arrowSkill->getMana( );
	wait += arrowSkill->getBeats( );
    }

    if (ch->mana < mana) {
	ch->send_to( "У тебя не хватает энергии для изготовления стрел.\n\r");
	return;
    }

    ch->mana -= mana;
    ch->setWait( wait );

    ch->send_to("Ты сосредотачиваешься на изготовлении стрел!\n\r");
    act_p("$c1 сосредотачивается на изготовлении стрел!",ch,0,0,TO_ROOM,POS_RESTING);

    if (number_percent() > arrowSkill->getEffective( ch )) {
	ch->send_to("..но у тебя ничего не выходит.\n\r");
	arrowSkill->improve( ch, false );
	return;
    }
    
    count = ch->getModifyLevel( ) / 5;

    for (int i = 0; i < count; i++) {
	if (number_percent( ) > gsn_make_arrow->getEffective( ch )) {
	    ch->send_to( "Ты пытаешься изготовить стрелу... но она ломается.\n\r");
	    gsn_make_arrow->improve( ch, false );
	    continue;
	}

	ch->send_to( "Ты изготавливаешь стрелу.\n\r");
	obj_to_char( create_arrow( arrowSkill->getIndex( ), ch->getModifyLevel( ) ), ch );
    }

    arrowSkill->improve( ch, true );
}



/*
 * 'make bow' skill command
 */

SKILL_RUNP( makebow )
{
  Object *bow;
  Affect tohit,todam;
  int mana,wait;

  if (ch->is_npc()) 
      return;

  if (!gsn_make_bow->usable( ch ))
    {
      ch->send_to("Ты не знаешь как изготовить лук.\n\r");
      return;
    }

  if ( ch->in_room->sector_type != SECT_FIELD &&
       ch->in_room->sector_type != SECT_FOREST &&
       ch->in_room->sector_type != SECT_HILLS )
  {
    ch->send_to( "Здесь нет ни кусочка дерева (кроме тебя)! Попробуй сделать это в лесу!\n\r");
    return;
  }

  mana = gsn_make_bow->getMana( );
  wait = gsn_make_bow->getBeats( );

  if ( ch->mana < mana )
  {
     ch->send_to( "У тебя не хватает энергии для изготовления лука.\n\r");
     return;
  }
  ch->mana -= mana;
  ch->setWait( wait );

  if ( number_percent( ) > gsn_make_bow->getEffective( ch ) )
   {
	ch->send_to( "Ты пытаешься изготовить лук... но он ломается.\n\r");
	gsn_make_bow->improve( ch, false );
	return;
   }
  ch->send_to( "Ты изготавливаешь лук.\n\r");
  gsn_make_bow->improve( ch, true );

  bow = create_object(get_obj_index(OBJ_VNUM_RANGER_BOW), ch->getModifyLevel() );
  bow->level = ch->getRealLevel( );
  bow->value[1] = 4 + ch->getModifyLevel() / 15;
  bow->value[2] = 4 + ch->getModifyLevel() / 15;

  tohit.where		    = TO_OBJECT;
  tohit.type               = gsn_make_arrow;
  tohit.level              = ch->getModifyLevel();
  tohit.duration           = -1;
  tohit.location           = APPLY_HITROLL;
  tohit.modifier           = ch->getModifyLevel() / 10;
  tohit.bitvector          = 0;
  affect_to_obj( bow, &tohit);

  todam.where		   = TO_OBJECT;
  todam.type               = gsn_make_arrow;
  todam.level              = ch->getModifyLevel();
  todam.duration           = -1;
  todam.location           = APPLY_DAMROLL;
  todam.modifier           = ch->getModifyLevel() / 10;
  todam.bitvector          = 0;
  affect_to_obj( bow, &todam);

  obj_to_char(bow,ch);
}



/*
 *  From SoG
 */
/*
 * 'forest fighting' skill command
 */

SKILL_RUNP( forest )
{
    char arg[MAX_STRING_LENGTH];
    Affect af;
    bool attack;
    int mana;

    if (ch->is_npc() || !gsn_forest_fighting->getEffective( ch )) {
	ch->send_to("Что?\n");
	return;
    }
    
    argument = one_argument( argument, arg );

    if (!*arg) {
	ch->send_to("Использование: forest {{ attack|defence|normal }\n\r");
	return;
    }
    else if (!str_prefix(arg, "normal")) {
	if (!ch->isAffected(gsn_forest_fighting)) {
	    ch->send_to("Ты не используешь в бою твои знания о лесе.\n\r");
	    return;
	}
	else {
	    ch->send_to("Ты прекращаешь использовать в бою твои знания о лесе.\n\r");
	    affect_strip(ch, gsn_forest_fighting);
	    return;
	}
    }
    else if (!str_prefix(arg, "defence")) 
	attack = false;
    else if (!str_prefix(arg, "attack"))
	attack = true;
    else {
	run(ch, str_empty);
	return;
    }

    mana = gsn_forest_fighting->getMana( );

    if (ch->mana < mana) {
	ch->send_to("У тебя недостаточно энергии (mana).\n\r");
	return;
    }
    
    ch->mana -= mana;
    ch->setWait( gsn_forest_fighting->getBeats( ) );
    

    if (ch->isAffected(gsn_forest_fighting))
	affect_strip(ch, gsn_forest_fighting);
    
    af.where 	 = TO_AFFECTS;
    af.type  	 = gsn_forest_fighting;
    af.level 	 = ch->getModifyLevel();
    af.duration	 = (6 + ch->getModifyLevel() / 2);
    af.location  = APPLY_NONE;
    af.bitvector = 0;

    if (attack) {
	af.modifier  = FOREST_ATTACK; 
	act_p("Ты чувствуешь себя дик$gим|им|ой!", ch, 0, 0, TO_CHAR, POS_DEAD);
	act_p("$c1 выглядит дик$gим|им|ой.", ch, 0, 0, TO_ROOM, POS_RESTING);
    }
    else {
	af.modifier  = FOREST_DEFENCE;
	act_p("Ты чувствуешь себя защищенн$gым|ым|ой.", ch, 0, 0, TO_CHAR, POS_DEAD);
	act_p("$c1 выглядит защищенн$gым|ым|ой.", ch, 0, 0, TO_ROOM, POS_RESTING);
    }

    affect_to_char(ch, &af);
}

BOOL_SKILL( forest )::run( Character *ch, int type ) 
{
    Affect* paf;

    if (ch->in_room->sector_type != SECT_FOREST
	&& ch->in_room->sector_type != SECT_HILLS
	&& ch->in_room->sector_type != SECT_MOUNTAIN) 
	return false;
    
    if (ch->is_npc( ))
	return gsn_forest_fighting->usable( ch );

    for (paf = ch->affected; paf; paf = paf->next) 
	if (paf->type == gsn_forest_fighting
	    && paf->modifier == type)
	{
	    return true;
	}

    return false;
}

/*
 * 'butcher' skill command
 */

SKILL_RUNP( butcher )
{
	Object *obj;

	char buf[MAX_STRING_LENGTH];
	char arg[MAX_STRING_LENGTH];

	if (ch->is_npc())
		return;

	one_argument(argument,arg);
	if ( arg[0]=='\0' )
	{
		ch->send_to("Разделать что?\n\r");
		return;
	}

	if ( (obj = get_obj_here(ch,arg)) == 0 )
	{
		ch->send_to("Ты не видишь этого здесь.\n\r");
		return;
	}

	if ( obj->item_type != ITEM_CORPSE_PC && obj->item_type != ITEM_CORPSE_NPC )
	{
		ch->send_to("Ты не сможешь разделать это на мясо.\n\r");
		return;
	}

	if ( obj->carried_by != 0 )
	{
		ch->send_to("Сперва положи это на землю.\n\r");
		return;
	}

	if ( gsn_butcher->getEffective( ch ) < 1 )
	{
		ch->send_to("Для этого у тебя недостаточно опыта!\n\r");
		return;
	}

	if ( obj->value[0] <= 0 )
	{
		ch->send_to("Ты разве видишь мясо в этом трупе?!\n\r");
		return;
	}

	act_p("$c1 ковыряет кинжалом $o4, надеясь срезать немного мяса.",
		ch,obj,0,TO_ROOM,POS_RESTING);

	int numsteaks;

	numsteaks = number_bits(2) + 1;

	if ( numsteaks > obj->value[0] )
		numsteaks = obj->value[0];

	obj->value[0] -= numsteaks;

	if ( number_percent() < gsn_butcher->getEffective( ch ) )
	{
		int i;
		Object *steak;

		sprintf(buf, "Ты срезаешь с $o2 %i кус%s мяса.",
			numsteaks,
			GET_COUNT(numsteaks,"ок","ка","ков"));
		act_p(buf,ch,obj,0,TO_CHAR,POS_RESTING);

		gsn_butcher->improve( ch, true );

		dreamland->removeOption( DL_SAVE_OBJS );

		for ( i=0; i < numsteaks; i++ )
		{
			steak = create_object(get_obj_index(OBJ_VNUM_STEAK),0);
			steak->fmtShortDescr( steak->getShortDescr( ), obj->getShortDescr( '2' ).c_str( ) );
			steak->fmtDescription( steak->getDescription( ), obj->getShortDescr( '2' ).c_str( ));
			
                        /* save originating mob vnum */
                        steak->value[2] = obj->value[3];
						
			obj_to_room(steak,ch->in_room);
		}

		dreamland->resetOption( DL_SAVE_OBJS );
		save_items( ch->in_room );
	}	
	else
	{
		act_p("Неумеха! Ты испорти$gлo|л|лa столько мяса!",ch,0,0,TO_CHAR,POS_RESTING);
		gsn_butcher->improve( ch, false );
	}
}

/*
 * 'tiger power' skill command
 */

SKILL_RUNP( tiger )
{
    int chance, hp_percent, mana;

    if ((chance = gsn_tiger_power->getEffective( ch )) == 0)
    {
	ch->send_to("Что?\n\r");
	return;
    }
    act_p("$c1 призывает силу 10 тигров!.",ch,0,0,TO_ROOM,POS_RESTING);

    if (IS_AFFECTED(ch,AFF_BERSERK) || ch->isAffected(gsn_berserk) ||
    ch->isAffected(gsn_tiger_power) || ch->isAffected(gsn_frenzy))
    {
	ch->send_to("Ты немного злишься.\n\r");
	return;
    }

    if (IS_AFFECTED(ch,AFF_CALM))
    {
	ch->send_to("Ты слишком миролюбив для этого.\n\r");
	return;
    }
    if (ch->in_room->sector_type != SECT_FIELD &&
           ch->in_room->sector_type != SECT_FOREST &&
           ch->in_room->sector_type != SECT_MOUNTAIN &&
           ch->in_room->sector_type != SECT_HILLS )
  {
    ch->send_to("Нет никого, кто услышал бы твой призыв.\n\r");
    return;
  }

    mana = gsn_tiger_power->getMana( );
    
    if (ch->mana < mana)
    {
	ch->send_to("У тебя не хватает энергии для этого.\n\r");
	return;
    }

    /* modifiers */

    /* fighting */
    if (ch->position == POS_FIGHTING)
	chance += 10;

    hp_percent = HEALTH(ch);
    chance += 25 - hp_percent/2;

    if (number_percent() < chance)
    {
	Affect af;

	ch->setWaitViolence( 1 );
	ch->mana -= mana;

	/* heal a little damage */
	ch->hit += ch->getModifyLevel() * 2;
	ch->hit = min(ch->hit,ch->max_hit);

	ch->send_to("10 тигров приходят на твой призыв, когда ты зовешь их!\n\r");
	act_p("10 тигров приходят на призыв $c2, и присоединяются к не$gму|му|й.",
               ch,0,0,TO_ROOM,POS_RESTING);
	gsn_tiger_power->improve( ch, true );

	af.where	= TO_AFFECTS;
	af.type		= gsn_tiger_power;
	af.level	= ch->getModifyLevel();
	af.duration	= number_fuzzy( ch->getModifyLevel() / 8);

	af.modifier	= max( 1, ch->getModifyLevel() / 5 );
	af.location	= APPLY_HITROLL;
	affect_to_char(ch,&af);

	af.location	= APPLY_DAMROLL;
	af.bitvector 	= AFF_BERSERK;
	affect_to_char(ch,&af);

	af.modifier	= max( 10, 10 * ( ch->getModifyLevel() / 5 ) );
	af.location	= APPLY_AC;
	af.bitvector    = 0;
	affect_to_char(ch,&af);
    }

    else
    {
	ch->setWaitViolence( 2 );
	ch->mana -= mana / 2;

	ch->send_to("Твоя сила повышается, но ничего не выходит.\n\r");
	gsn_tiger_power->improve( ch, false );
    }
}

/*----------------------------------------------------------------------------
 * Ambush 
 *---------------------------------------------------------------------------*/
class AmbushOneHit: public WeaponOneHit, public SkillDamage {
public:
    AmbushOneHit( Character *ch, Character *victim );

    virtual void calcTHAC0( );
    virtual void calcDamage( );
};

AmbushOneHit::AmbushOneHit( Character *ch, Character *victim )
	    : Damage( ch, victim, 0, 0 ), WeaponOneHit( ch, victim, false ),
	      SkillDamage( ch, victim, gsn_ambush, 0, 0, DAMF_WEAPON )
{
}

void AmbushOneHit::calcDamage( ) 
{
    damBase( );
    gsn_enhanced_damage->getCommand( )->run( ch, victim, dam );;
    damApplyPosition( );
    damApplyDamroll( );
    dam *= 3;

    WeaponOneHit::calcDamage( );
}    

void AmbushOneHit::calcTHAC0( )
{
    thacBase( );
    thacApplyHitroll( );
    thacApplySkill( );
    thac0 -= 10 * (100 - gsn_ambush->getEffective( ch ));
}

/*
 * 'ambush' skill command
 */
SKILL_RUNP( ambush )
{
    char arg[MAX_INPUT_LENGTH];
    Character *victim;
    char buf[MAX_STRING_LENGTH];

    if ( MOUNTED(ch) )
    {
	    ch->send_to("Ты не можешь быть в засаде верхом!\n\r");
	    return;
    }

    one_argument( argument, arg );

    if ( ch->is_npc() || !gsn_ambush->usable( ch ) )
    {
	    ch->send_to("Ты не знаешь, как устроить засаду.\n\r");
	    return;
    }

    if ( arg[0] == '\0' )
    {
	    if ( ch->ambushing[0] == '\0' )
	    {
		    ch->send_to("Засаду кому?\n\r");
		    return;
	    }
	    else
	    {
		    sprintf(buf, "Ты сидишь в засаде на %s.\n\r", ch->ambushing);
		    ch->send_to(buf);
		    return;
	    }
    }

    if ( ( victim = get_char_room( ch, arg ) ) == 0 )
    {
	    if ( !IS_AFFECTED(ch,AFF_CAMOUFLAGE) )
	    {
		    ch->send_to("Сначала тебе стоит замаскироваться.\n\r");
		    return;
	    }
	    ch->send_to("Ты готовишься к засаде.\n\r");
	    ch->ambushing = str_dup(arg);
	    return;
    }

    if ( victim == ch )
    {
	    ch->send_to("Засаду себе? Это еще как?!\n\r");
	    return;
    }

    if ( !IS_AFFECTED(ch,AFF_CAMOUFLAGE) || victim->can_see(ch) )
    {
	    ch->send_to("Твоя жертва тебя видит.\n\r");
	    return;
    }

    if ( is_safe( ch, victim ) )
	    return;

    ch->setWait( gsn_ambush->getBeats( )  );
    AmbushOneHit amb( ch, victim );
    
    try {
	if ( !IS_AWAKE(victim)
		|| ch->is_npc()
		|| number_percent( ) < gsn_ambush->getEffective( ch ) )
	{
		gsn_ambush->improve( ch, true, victim );
		amb.hit( );
	}
	else
	{
		gsn_ambush->improve( ch, false, victim );
		amb.miss( );
	}

	yell_panic( ch, victim,
		    "Помогите! На меня кто-то напал из засады!",
		    "Помогите! На меня из засады напа%1$Gло|л|ла %1$C1!" );
    }
    catch (const VictimDeathException& e) {                                     
    }
}

BOOL_SKILL( ambush )::run( Character *ch ) 
{
    Character *vch, *vch_next;

    if (ch->ambushing[0] == '\0')
	return false;
    if (!IS_AWAKE(ch))
	return false;
    if (ch->fighting)
	return false;
    if (!IS_AFFECTED(ch, AFF_CAMOUFLAGE))
	return false;

    for (vch = ch->in_room->people; vch; vch = vch_next) {
	vch_next = vch->next_in_room;

	if (ch != vch
		&& ch->can_see(vch)
		&& !vch->can_see(ch)
		&& !is_safe_nomessage(ch,vch)
		&& is_name(ch->ambushing, vch->getNameP()))
	{
	    ch->println( "{YТЫ ВЫСКАКИВАЕШЬ ИЗ ЗАСАДЫ!{x" );
	    run( ch, ch->ambushing );
	    return true;
	}
    }

    return false;
}   


/*
 * 'camouflage' skill command
 */

SKILL_RUNP( camouflage )
{
	if ( ch->is_npc() || !gsn_camouflage->usable( ch ) )
	{
		ch->send_to("Ты не знаешь, как замаскировать себя.\n\r");
		return;
	}

	if ( MOUNTED(ch) )
	{
		ch->send_to("Ты не можешь замаскироваться, когда ты в седле.\n\r");
		return;
	}

	if ( RIDDEN(ch) )
	{
		ch->send_to("Ты не можешь замаскироваться, когда ты оседлан.\n\r");
		return;
	}

	if ( IS_AFFECTED( ch, AFF_FAERIE_FIRE ) )
	{
		ch->send_to("Ты не можешь замаскироваться, когда светишься.\n\r");
		return;
	}

	if ( ch->in_room->sector_type != SECT_FOREST
		&& ch->in_room->sector_type != SECT_HILLS
		&& ch->in_room->sector_type != SECT_MOUNTAIN )
	{
		ch->send_to("Здесь негде укрыться.\n\r");
		act_p("$c1 пытается замаскироваться, но не может найти укрытия.",ch,0,0,TO_ROOM,POS_RESTING);
		return;
	}
	ch->send_to("Ты пытаешься замаскироваться.\n\r");

	int k = ch->getLastFightDelay( );

	if ( k >= 0 && k < FIGHT_DELAY_TIME )
		k = k * 100 /	FIGHT_DELAY_TIME;
	else
		k = 100;

	ch->setWait( gsn_camouflage->getBeats( )  );

	if ( IS_AFFECTED(ch, AFF_CAMOUFLAGE) )
	{
		REMOVE_BIT(ch->affected_by, AFF_CAMOUFLAGE);
		ch->ambushing = &str_empty[0];
	}

	if ( ch->is_npc()
		|| number_percent( ) < gsn_camouflage->getEffective( ch ) * k / 100 )
	{
		SET_BIT(ch->affected_by, AFF_CAMOUFLAGE);
		gsn_camouflage->improve( ch, true );
	}
	else
		gsn_camouflage->improve( ch, false );

	return;
}






SPELL_DECL(RangerStaff);
VOID_SPELL(RangerStaff)::run( Character *ch, char *, int sn, int level ) 
{ 
  Object *staff;
  Affect tohit;
  Affect todam;

  staff = create_object( get_obj_index(OBJ_VNUM_RANGER_STAFF),level);
  ch->send_to("Ты создаешь посох рейнджера!\n\r");
  act_p("$c1 создает посох рейнджера!",ch,0,0,TO_ROOM,POS_RESTING);

  staff->value[1] = 4 + level / 15;
  staff->value[2] = 4 + level / 15;

  tohit.where		   = TO_OBJECT;
  tohit.type               = sn;
  tohit.level              = ch->getModifyLevel();
  tohit.duration           = -1;
  tohit.location           = APPLY_HITROLL;
  tohit.modifier           = 2 + level/5;
  tohit.bitvector          = 0;
  affect_to_obj( staff, &tohit);

  todam.where		   = TO_OBJECT;
  todam.type               = sn;
  todam.level              = ch->getModifyLevel();
  todam.duration           = -1;
  todam.location           = APPLY_DAMROLL;
  todam.modifier           = 2 + level/5;
  todam.bitvector          = 0;
  affect_to_obj( staff, &todam);


  staff->timer = level;
  staff->level = ch->getModifyLevel();

  obj_to_char(staff,ch);

}

/*
 * ranger staff behavior
 */
void RangerStaff::fight( Character *ch )
{
    if (obj->wear_loc != wear_wield && obj->wear_loc != wear_second_wield)
	return;

    if (chance( 90 ))
	return;

    act_p( "{BТвой посох рейнджера вспыхивает голубым светом!{x", ch, 0, 0, TO_CHAR, POS_DEAD );
    act( "{BПосох рейнджера $c2 вспыхивает голубым светом!{x", ch, 0, 0, TO_ROOM );

    spell( gsn_cure_critical, ch->getModifyLevel( ), ch, ch, FSPELL_BANE );
}

bool RangerStaff::death( Character *ch )
{
    act_p( "Твой посох рейнджера исчезает.", ch, 0, 0, TO_CHAR, POS_DEAD );
    act( "Посох рейнджера $c2 исчезает.", ch, 0, 0, TO_ROOM );
    extract_obj( obj );
    return false;
}

bool RangerStaff::canEquip( Character *ch )
{
  if (ch->getTrueProfession( ) != prof_ranger) {
	ch->println("Ты не знаешь как использовать эту вещь.");
	act( "Посох рейнджера выскальзывает из твоих рук.", ch, 0, 0, TO_CHAR );
	act( "Посох рейнджера выскальзывает из рук $c2.", ch, 0, 0, TO_ROOM );
	obj_from_char( obj );
	obj_to_room( obj, ch->in_room );
	return false;
    }

    return true;
}

RangerCreature::~RangerCreature( )
{
}

