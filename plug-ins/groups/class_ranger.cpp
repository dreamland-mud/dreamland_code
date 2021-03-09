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

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"

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
#include "roomutils.h"
#include "npcharacter.h"
#include "room.h"
#include "core/object.h"

#include "dreamland.h"
#include "gsn_plugin.h"
#include "act_move.h"
#include "mercdb.h"
#include "magic.h"
#include "fight.h"
#include "weapongenerator.h"
#include "stats_apply.h"
#include "directions.h"
#include "onehit.h"
#include "onehit_weapon.h"
#include "damage_impl.h"
#include "vnum.h"
#include "merc.h"
#include "../anatolia/handler.h"
#include "save.h"
#include "act.h"
#include "interp.h"
#include "def.h"
#include "skill_utils.h"

PROF(ranger);

static Object * create_arrow( int color, int level );
static Object * find_arrow( Character *ch, Object *quiver );


#define OBJ_VNUM_RANGER_STAFF        28
#define OBJ_VNUM_RANGER_ARROW        6
#define OBJ_VNUM_RANGER_BOW          7

/*
 * 'track' skill command
 */
static bool oprog_track( Object *obj, Character *ch, const char *target, int door )
{
    FENIA_CALL( obj, "Track", "Csi", ch, target, door );
    FENIA_NDX_CALL( obj, "Track", "OCsi", obj, ch, target, door );
    return false;
}

SKILL_RUNP( track )
{
    DLString arg( argument );
    EXIT_DATA *pexit;
    int d;

    if (gsn_track->getEffective( ch ) < 2) {
        ch->pecho("Ты не умеешь читать следы на земле.");
        return;
    }

    if (arg.empty( )) {
        ch->pecho( "Кого ты хочешь выследить?" );
        return;
    }

    ch->setWait( gsn_track->getBeats( ) );
    act("%^C1 всматривается в землю в поисках следов.",ch,0,0,TO_ROOM);

    int slevel;
    slevel = gsn_track->getEffective( ch ) + skill_level_bonus(*gsn_track, ch);  
    
    if (number_percent() < slevel)
        if (( d = ch->in_room->history.went( arg, false ) ) != -1) {
            if (( pexit = ch->in_room->exit[d] )) {

                gsn_track->improve( ch, true );

                for (Object *obj = ch->carrying; obj; obj = obj->next_content)
                    if (oprog_track(obj, ch, arg.c_str(), d))
                        return;

                ch->pecho("Следы %N2 ведут %s.", arg.c_str(), dirs[d].leave);
                
                if (IS_SET(pexit->exit_info, EX_CLOSED)) 
                    open_door_extra( ch, d, pexit );
                
                move_char(ch, d );
                return;
            }
        }
    
    ch->pecho("Ты не видишь здесь следов.");
    gsn_track->improve( ch, false );
}

/*
 * find an arrow in the quiver
 */
static Object * find_arrow( Character *ch, Object *quiver )
{
    Object *arrow = NULL;

    for (Object *obj = quiver->contains; obj != 0; obj = obj->next_content)
        if (obj->item_type == ITEM_WEAPON && obj->value0() == WEAPON_ARROW) {
            arrow = obj;
            break; 
        }

    if (!arrow) {
        oldact("В $o6 закончились стрелы.", ch, quiver, 0, TO_CHAR);
        return NULL;
    }

    if (ch->getModifyLevel( ) + 10 < arrow->level) {
        ch->pecho("Тебе не хватает опыта воспользоватся этой стрелой.");
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
    bool success;
    int chance,direction;
    int range, range0 = ( ch->getModifyLevel() / 10) + 1;
    int master_shoots = 2;
    DLString argDoor, argVict;
    ostringstream errbuf;

    if (!gsn_bow->usable( ch ))
    {
          ch->pecho("Ты не умеешь стрелять из лука.");
          return;
    }

    if ( ch->fighting )
    {
            ch->pecho("Сражаясь, ты не можешь прицелиться.");
            return;
    }

    if (!direction_range_argument(argument, argDoor, argVict, direction)) {
        ch->pecho("Выстрелить в каком направлении и в кого?");
        return;
    }

    range = range0;
    if ( ( victim = find_char( ch, argVict.c_str(), direction, &range, errbuf ) ) == 0 )
    {
            ch->send_to(errbuf);
            return;
    }

    if ( !victim->is_npc() && victim->desc == 0 )
    {
            ch->pecho("Ты не можешь сделать этого.");
            return;
    }

    if ( victim == ch )
    {
            ch->pecho("Это невозможно!");
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
            || wield->value0() != WEAPON_BOW )
    {
            ch->pecho("Для того, чтобы стрелять тебе нужен лук!");
            return;            
    }

    if (!ch->is_npc( ) 
        && (get_eq_char(ch,wear_second_wield)
            || get_eq_char(ch,wear_shield)) )
    {
            ch->pecho("Твоя вторая рука должна быть свободна!");
            return;            
    }

    if (!ch->is_npc( ) && !quiver)
    {
            ch->pecho("У тебя в руках ничего нет!");
            return;            
    }        
    
    if (!ch->is_npc( ) && 
        (quiver->item_type != ITEM_CONTAINER || !IS_SET(quiver->value1(), CONT_FOR_ARROW)))
    {
            ch->pecho("Возьми в руки колчан.");
            return;
    }

    if ( ch->in_room == victim->in_room )
    {
            ch->pecho("Ты не можешь стрелять из лука в упор.");
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

    chance = skill_level_bonus(*gsn_bow, ch) + (gsn_bow->getEffective( ch ) - 50) * 2;
    if ( victim->position == POS_SLEEPING )
            chance += 40;
    if ( victim->position == POS_RESTING )
            chance += 10;
    if ( victim->position == POS_FIGHTING )
            chance -= 40;
    chance += ch->hitroll;
    
    ch->pecho( "%1$^O1, посланн%1$Gое|ый|ая|ые тобой, улете%1$nла|ли %2$s.", arrow, dirs[ direction ].leave );
    ch->recho( "%1$^O1, посланн%1$Gое|ый|ая|ые %3$C5, улете%1$nла|ли %2$s.", arrow, dirs[ direction ].leave, ch );

    set_violent( ch, victim, false );
    
    try {
        success = send_arrow( ch, victim, arrow,
                              direction, chance,
                              dice( wield->value1(), wield->value2() ) );
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
        if (find_char( ch, argVict.c_str(), direction, &range, errbuf) != victim) {
            ch->send_to(errbuf);
            return; 
        }
        
        if (!( arrow = find_arrow( ch, quiver ) ))
            return;
        
        try {
            success = send_arrow( ch, victim, arrow,
                                  direction, chance,
                                  dice( wield->value1(), wield->value2() ) );
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
      ch->pecho("Ты пока не можешь применять целебные травы, подожди немного.");
      return;
    }

  if (arg[0] == '\0')
    victim = ch;
  else if ( (victim = get_char_room(ch,arg)) == 0)
    {
      ch->pecho("Таких тут нет.");
      return;
    }
  ch->setWait( gsn_herbs->getBeats( )  );

  if (RoomUtils::isNature(ch->in_room) && number_percent() < gsn_herbs->getEffective( ch ))
    {
      postaffect_to_char(ch, gsn_herbs, 5);

      ch->pecho("Ты собираешь целебные травы по окрестностям.");
      act("%^C1 собирает какие-то травы по окрестностям.",ch,0,0,TO_ROOM);

      if (ch != victim)
        {
          act("%^C1 накладывает целебные травы на твои раны.",ch,0,victim,TO_VICT);
          act("Ты накладываешь целебные травы на раны %2$C2.",ch,0,victim,TO_CHAR);
          oldact("$c1 накладывает целебные травы на раны $C2.",ch,0,victim,TO_NOTVICT);
        }
        
      if (victim->hit < victim->max_hit)
        {
          victim->pecho("Ты чувствуешь себя здоровее.");
          act("%^C1 выглядит здоровее.",victim,0,0,TO_ROOM);
        }
      int slevel = skill_level(*gsn_herbs, ch);
      victim->hit = min((int)victim->max_hit,victim->hit + 5 * slevel );
      gsn_herbs->improve( ch, true, victim );
      
      checkDispel( slevel, victim, gsn_plague );
      checkDispel( slevel, victim, gsn_poison );
    }
  else
    {
      ch->pecho("Ты ищешь целебные травы, но ничего не находишь.");
      act("%^C1 рыщет в поисках целебных трав, но ничего не находит.",ch,0,0,TO_ROOM);
      gsn_herbs->improve( ch, false, victim );
    }
}

/*
 * 'camp' skill command
 */

SKILL_RUNP( camp )
{
  Affect af2;

  if (ch->is_npc() || !gsn_camp->usable( ch ) )
    {
      ch->pecho("Чего?");
      return;
    }

  if (ch->isAffected(gsn_camp))
    {
      ch->pecho("Ты пока не можешь разбить лагерь, подожди немного.");
      return;
    }


  if ( number_percent( ) > gsn_camp->getEffective( ch ) )
  {
        ch->pecho("Ты пытаешься разбить лагерь, но у тебя ничего не получается.");
        gsn_camp->improve( ch, true );
        return;
  }

  if (!RoomUtils::isNature(ch->in_room))
  {
    ch->pecho("Разбить полевой лагерь можно только на лоне природы.");
    return;
  }

  if ( ch->mana < gsn_camp->getMana( ))
  {
     ch->pecho("У тебя не хватает энергии для разбивки лагеря.");
     return;
  }

  gsn_camp->improve( ch, true );
  ch->mana -= gsn_camp->getMana( );
  ch->setWait( gsn_camp->getBeats( ) );

  oldact("Ты разбиваешь полевой лагерь.", ch, 0, 0, TO_CHAR);
  act("%^C1 разбивает полевой лагерь.", ch, 0, 0, TO_ROOM);

  int slevel = skill_level(*gsn_camp, ch);
    
  postaffect_to_char(ch, gsn_camp, 12);


  af2.type              = gsn_camp;
  af2.level              = slevel;
  af2.duration           = slevel / 20;
  af2.modifier           = 4 * slevel;
  af2.location.setTable(&apply_room_table);
  af2.location = APPLY_ROOM_HEAL;
  ch->in_room->affectTo( &af2);

  af2.modifier           = 2 * slevel;
  af2.location = APPLY_ROOM_MANA;
  ch->in_room->affectTo( &af2);
}

AFFECT_DECL(Camp);
VOID_AFFECT(Camp)::toStream( ostringstream &buf, Affect *paf ) 
{
    if (paf->location == APPLY_ROOM_HEAL)
        buf << fmt( 0, "Разбитый здесь лагерь улучшит восстановление здоровья на {W%2$d{x в течение {W%1$d{x ча%1$Iса|сов|сов.",
                    paf->duration, paf->modifier) << endl;
    else if (paf->location == APPLY_ROOM_MANA)
        buf << fmt( 0, "Разбитый здесь лагерь улучшит восстановление маны на {W%2$d{x в течение {W%1$d{x ча%1$Iса|сов|сов.",
                    paf->duration, paf->modifier) << endl;
}


/*
 * 'bear call' skill command
 */

SKILL_RUNP( bearcall )
{
    SpellTarget::Pointer target;
    ostringstream errbuf;

  if (!gsn_bear_call->usable( ch ) )
    {
      ch->pecho("Чего?");
      return;
    }

  if ( ch->mana < gsn_bear_call->getMana( ))
  {
     ch->pecho("У тебя не хватает сил, чтобы призвать на помощь медведей.");
     return;
  }

  target = gsn_bear_call->getSpell( )->locateTargets( ch, argument, errbuf );
  if (target->error) {
      ch->send_to( errbuf );
      return;
  }

  if ( number_percent( ) > gsn_bear_call->getEffective( ch ) )
  {
        ch->pecho("Медведи не слышат твой зов, стоит еще попрактиковаться.");
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
    int slevel;
    slevel = skill_level(*gsn_bear_call, ch);
    
    return createMobileAux( ch, level, 
                         (ch->is_npc( ) ? ch->max_hit : ch->getPC( )->perm_hit), 
                         (ch->is_npc( ) ? ch->max_mana : ch->getPC( )->perm_mana),
                         number_range(slevel/15, slevel/10),
                         number_range(slevel/3, slevel/2),
                         number_range(slevel/8, slevel/6) );
}

TYPE_SPELL(bool, BearCall)::canSummonHere( Character *ch ) const 
{
  if (IS_SET(ch->in_room->room_flags, ROOM_NO_MOB|ROOM_SAFE|ROOM_PRIVATE|ROOM_SOLITARY) )
  {
     ch->pecho("В эту местность медведям путь заказан.");
     return false;
  }

  if (!ch->in_room->hasExits())
  {
    ch->pecho("Медведи не смогут пройти к тебе.");
    return false;
  }
    
  if (!RoomUtils::isNature(ch->in_room))
  {
    ch->pecho("Медведи придут на помощь только на лоне природы.");
    return false;
  }
    
  return true;
}    

/*
 * 'lion call' skill command
 */

SKILL_RUNP( lioncall )
{
    SpellTarget::Pointer target;
    ostringstream errbuf;

  if (!gsn_lion_call->usable( ch ) )
    {
          ch->pecho("Чего?");
          return;
        }

  if ( ch->mana < gsn_lion_call->getMana( ))
  {
       ch->pecho("У тебя не хватает энергии, чтобы позвать львов.");
       return;
    }

  target = gsn_lion_call->getSpell( )->locateTargets( ch, argument, errbuf );
  if (target->error) {
        ch->send_to( errbuf );
        return;
  }

  if ( number_percent( ) > gsn_lion_call->getEffective( ch ) )
  {
    ch->pecho("Львы не слышат твой зов, стоит еще попрактиковаться.");
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
    int slevel;
    slevel = skill_level(*gsn_lion_call, ch);
    
    return createMobileAux( ch, level, 
                         (ch->is_npc( ) ? ch->max_hit : ch->getPC( )->perm_hit), 
                         (ch->is_npc( ) ? ch->max_mana : ch->getPC( )->perm_mana),
                         number_range(slevel/15, slevel/10),
                         number_range(slevel/3, slevel/2),
                         number_range(slevel/8, slevel/6) );
}

TYPE_SPELL(bool, LionCall)::canSummonHere( Character *ch ) const 
{
  if (IS_SET(ch->in_room->room_flags, ROOM_NO_MOB|ROOM_SAFE|ROOM_PRIVATE|ROOM_SOLITARY) )
  {
     ch->pecho("В эту местность львам путь заказан.");
     return false;
  }

  if (!ch->in_room->hasExits())
  {
    ch->pecho("Львы не смогут пройти к тебе.");
    return false;
  }
    
  if (!RoomUtils::isNature(ch->in_room))
  {
    ch->pecho("Львы придут на помощь только на лоне природы.");
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

    tohit.type               = gsn_make_arrow;
    tohit.level              = level;
    tohit.duration           = -1;
    tohit.location = APPLY_HITROLL;
    tohit.modifier           = level / 10 + 1;
    affect_to_obj( arrow, &tohit);

    todam.type               = gsn_make_arrow;
    todam.level              = level;
    todam.duration           = -1;
    todam.location = APPLY_DAMROLL;
    todam.modifier           = level / 10 + 1;
    affect_to_obj( arrow, &todam);

    if (color != 0 && color != gsn_make_arrow)
    {
        Affect saf;

        saf.bitvector.setTable(&weapon_type2);
        saf.type               = color;
        saf.level              = level;
        saf.duration           = -1;
        
        saf.modifier           = 0;

        if ( color == gsn_green_arrow )
        {
            saf.bitvector.setValue(WEAPON_POISON);
            str_name = "green зеленая";
            str_long = "{GЗеленая";
            str_short = "{Gзелен|ая|ой|ой|ую|ой|ой";
            arrow->value1(4 + level / 12);
            arrow->value2(4 + level / 10);
        }
        else if (color == gsn_red_arrow)
        {
            saf.bitvector.setValue(WEAPON_FLAMING);
            str_name = "red красная";
            str_long = "{RКрасная";
            str_short = "{Rкрасн|ая|ой|ой|ую|ой|ой";
            arrow->value1(4 + level / 15);
            arrow->value2(4 + level / 30);
        }
        else if (color == gsn_white_arrow)
        {
            saf.bitvector.setValue(WEAPON_FROST);
            str_name = "white белая";
            str_long = "{WБелая";
            str_short = "{Wбел|ая|ой|ой|ую|ой|ой";
            arrow->value1(4 + level / 15);
            arrow->value2(4 + level / 30);
        }
        else
        {
            saf.bitvector.setValue(WEAPON_SHOCKING);
            str_name = "blue голубая";
            str_long = "{CГолубая";
            str_short = "{Cголуб|ая|ой|ой|ую|ой|ой";
            arrow->value1(4 + level / 15);
            arrow->value2(4 + level / 30);
        }

        affect_to_obj( arrow, &saf);
    }
    else
    {
        str_name = "wooden деревянная";
        str_long = "{yДеревянная";
        str_short = "{yдеревянн|ая|ой|ой|ую|ой|ой";
        arrow->value1(4 + level / 12);
        arrow->value2(4 + level / 10);
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
        ch->pecho("Ты не умеешь изготавливать стрелы.");
        return;
    }

    if (!RoomUtils::isNature(ch->in_room))
    {
        ch->pecho("В этой местности тебе не удается найти древесины для изготовления стрел.");
        return;
    }

    mana = gsn_make_arrow->getMana( );
    wait = gsn_make_arrow->getBeats( );

    argument = one_argument(argument, arg);
    
    if (arg[0] == '\0')        {
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
            ch->pecho("Можно изготовить только зеленые, красные, белые или голубые стрелы.");
            return;
        }

        if (!arrowSkill->usable( ch )) {
            ch->pecho("Ты не умеешь изготавливать такие стрелы.");
            return;
        }

        mana += arrowSkill->getMana( );
        wait += arrowSkill->getBeats( );
    }

    if (ch->mana < mana) {
        ch->pecho("У тебя не хватает энергии для изготовления стрел.");
        return;
    }

    ch->mana -= mana;
    ch->setWait( wait );

    ch->pecho("Ты сосредотачиваешься на изготовлении стрел!");
    act("%^C1 сосредотачивается на изготовлении стрел!",ch,0,0,TO_ROOM);

    if (number_percent() > arrowSkill->getEffective( ch )) {
        ch->pecho("Неудача. Придется еще попрактиковаться...");
        arrowSkill->improve( ch, false );
        return;
    }
    
    int slevel = skill_level(*gsn_make_arrow, ch);
    count = slevel / 5;

    for (int i = 0; i < count; i++) {
        if (number_percent( ) > gsn_make_arrow->getEffective( ch )) {
            ch->pecho("Ты пытаешься изготовить стрелу... но она ломается.");
            gsn_make_arrow->improve( ch, false );
            continue;
        }

        ch->pecho("Ты изготавливаешь стрелу.");
        obj_to_char( create_arrow( arrowSkill->getIndex( ), slevel ), ch );
    }

    arrowSkill->improve( ch, true );
}



/*
 * 'make bow' skill command
 */

SKILL_RUNP(makebow)
{
    Object *bow;
    int mana, wait;

    if (ch->is_npc())
        return;

    if (!gsn_make_bow->usable(ch)) {
        ch->pecho("Ты не знаешь как изготовить лук.");
        return;
    }

    if (!RoomUtils::isNature(ch->in_room))
    {
        ch->pecho("В этой местности тебе не удается найти древесины для изготовления лука.");
        return;
    }

    mana = gsn_make_bow->getMana();
    wait = gsn_make_bow->getBeats();

    if (ch->mana < mana) {
        ch->pecho("У тебя не хватает энергии для изготовления лука.");
        return;
    }
    ch->mana -= mana;
    ch->setWait(wait);

    if (number_percent() > gsn_make_bow->getEffective(ch)) {
        ch->pecho("Ты пытаешься изготовить лук... но он ломается.");
        gsn_make_bow->improve(ch, false);
        return;
    }
    ch->pecho("Ты изготавливаешь лук.");
    gsn_make_bow->improve(ch, true);

    bow = create_object(get_obj_index(OBJ_VNUM_RANGER_BOW), ch->getModifyLevel());
    bow->level = ch->getModifyLevel();

    WeaponGenerator()
        .item(bow)
        .skill(gsn_make_arrow)
        .valueTier(3)
        .hitrollTier(IS_GOOD(ch) ? 2 : 3)
        .damrollTier(IS_EVIL(ch) ? 2 : 3)
        .assignValues()
        .assignHitroll()
        .assignDamroll();

    obj_to_char(bow, ch);
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
        ch->pecho("Ты не владеешь этим умением.");
        return;
    }
    
    argument = one_argument( argument, arg );

    if (!*arg) {
        ch->pecho("Использование: {lRлес атака|защита|выкл{lEforest attack|defence|off{x");
        return;
    }

    if (arg_is_switch_off(arg) || arg_oneof(arg, "normal")) {
        if (!ch->isAffected(gsn_forest_fighting)) {
            ch->pecho("Ты не используешь в бою твои знания о лесе.");
        }
        else {
            ch->pecho("Ты прекращаешь использовать в бою твои знания о лесе.");
            affect_strip(ch, gsn_forest_fighting);
        }

        return;
    }

    if (arg_oneof(arg, "защита", "defense", "defence")) 
        attack = false;
    else if (arg_oneof(arg, "атака", "attack"))
        attack = true;
    else {
        run(ch, str_empty);
        return;
    }

    mana = gsn_forest_fighting->getMana( );

    if (ch->mana < mana) {
        ch->pecho("У тебя недостаточно энергии (mana).");
        return;
    }
    
    ch->mana -= mana;
    ch->setWait( gsn_forest_fighting->getBeats( ) );
    

    if (ch->isAffected(gsn_forest_fighting))
        affect_strip(ch, gsn_forest_fighting);
    
    int slevel = skill_level(*gsn_forest_fighting, ch);
    
    af.type           = gsn_forest_fighting;
    af.level          = slevel;
    af.duration         = (6 + slevel / 2);

    if (attack) {
        af.modifier  = FOREST_ATTACK; 
        oldact_p("Ты чувствуешь себя дик$gим|им|ой!", ch, 0, 0, TO_CHAR, POS_DEAD);
        oldact("$c1 выглядит дик$gим|им|ой.", ch, 0, 0, TO_ROOM);
    }
    else {
        af.modifier  = FOREST_DEFENCE;
        oldact_p("Ты чувствуешь себя защищенн$gым|ым|ой.", ch, 0, 0, TO_CHAR, POS_DEAD);
        oldact("$c1 выглядит защищенн$gым|ым|ой.", ch, 0, 0, TO_ROOM);
    }

    affect_to_char(ch, &af);
}

BOOL_SKILL( forest )::run( Character *ch, int type ) 
{
    if (!RoomUtils::isNature(ch->in_room))
        return false;
    
    if (ch->is_npc( ))
        return gsn_forest_fighting->usable( ch );

    for (auto &paf: ch->affected) 
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
                ch->pecho("Разделать что?");
                return;
        }

        if ( (obj = get_obj_here(ch,arg)) == 0 )
        {
                ch->pecho("Ты не видишь этого здесь.");
                return;
        }

        if ( obj->item_type != ITEM_CORPSE_PC && obj->item_type != ITEM_CORPSE_NPC )
        {
                ch->pecho("Ты не сможешь разделать это на мясо.");
                return;
        }

        if ( obj->carried_by != 0 )
        {
                ch->pecho("Сперва положи это на землю.");
                return;
        }

        if ( gsn_butcher->getEffective( ch ) < 1 )
        {
                ch->pecho("Для этого у тебя недостаточно опыта!");
                return;
        }

        if ( obj->value0() <= 0 )
        {
                ch->pecho("Ты разве видишь мясо в этом трупе?!");
                return;
        }

        oldact_p("$c1 ковыряет кинжалом $o4, надеясь срезать немного мяса.",
                ch,obj,0,TO_ROOM,POS_RESTING);

        int numsteaks;

        numsteaks = number_bits(2) + 1;

        if ( numsteaks > obj->value0() )
                numsteaks = obj->value0();

        obj->value0(obj->value0() - numsteaks);

        if ( number_percent() < gsn_butcher->getEffective( ch ) + skill_level_bonus(*gsn_butcher, ch) )
        {
                int i;
                Object *steak;

                sprintf(buf, "Ты срезаешь с $o2 %i кус%s мяса.",
                        numsteaks,
                        GET_COUNT(numsteaks,"ок","ка","ков"));
                oldact(buf,ch,obj,0,TO_CHAR);

                gsn_butcher->improve( ch, true );

                dreamland->removeOption( DL_SAVE_OBJS );

                for ( i=0; i < numsteaks; i++ )
                {
                        steak = create_object(get_obj_index(OBJ_VNUM_STEAK),0);
                        steak->fmtShortDescr( steak->getShortDescr( ), obj->getShortDescr( '2' ).c_str( ) );
                        steak->fmtDescription( steak->getDescription( ), obj->getShortDescr( '2' ).c_str( ));
                        
                        /* save originating mob vnum */
                        steak->value2(obj->value3());
                                                
                        obj_to_room(steak,ch->in_room);
                }

                dreamland->resetOption( DL_SAVE_OBJS );
                save_items( ch->in_room );
        }        
        else
        {
                oldact("Неумеха! Ты испорти$gлo|л|лa столько мяса!",ch,0,0,TO_CHAR);
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
        ch->pecho("Что?");
        return;
    }
    act("%^C1 призывает силу десяти тигров!",ch,0,0,TO_ROOM);

    if (IS_AFFECTED(ch,AFF_BERSERK) || ch->isAffected(gsn_berserk) ||
    ch->isAffected(gsn_tiger_power) || ch->isAffected(gsn_frenzy))
    {
        ch->pecho("Ты уже в состоянии боевой ярости!");
        return;
    }

    if (IS_AFFECTED(ch,AFF_CALM))
    {
        ch->pecho("Ты слишком миролюбив{Sfа{Sx для этого.");
        return;
    }
    if (!RoomUtils::isNature(ch->in_room))
  {
    ch->pecho("Это умение сработает только на лоне природы.");
    return;
  }

    mana = gsn_tiger_power->getMana( );
    
    if (ch->mana < mana)
    {
        ch->pecho("У тебя не хватает энергии для этого.");
        return;
    }

    /* modifiers */

    /* fighting */
    if (ch->position == POS_FIGHTING)
        chance += 10;

    hp_percent = HEALTH(ch);
    chance += 25 - hp_percent/2;
    chance += skill_level_bonus(*gsn_tiger_power, ch);

    if (number_percent() < chance)
    {
        Affect af;

        ch->setWaitViolence( 1 );
        ch->mana -= mana;

        int slevel = skill_level(*gsn_tiger_power, ch);
            
        /* heal a little damage */
        ch->hit += slevel * 2;
        ch->hit = min(ch->hit,ch->max_hit);

        ch->pecho("10 тигров приходят на твой призыв, когда ты зовешь их!");
        oldact_p("10 тигров приходят на призыв $c2, и присоединяются к не$gму|му|й.",
               ch,0,0,TO_ROOM,POS_RESTING);
        gsn_tiger_power->improve( ch, true );

        af.type                = gsn_tiger_power;
        af.level        = slevel;
        af.duration        = number_fuzzy( slevel / 8);

        af.modifier        = max( 1, slevel / 5 );
        af.location = APPLY_HITROLL;
        affect_to_char(ch,&af);

        af.location = APPLY_DAMROLL;
        af.bitvector.setTable(&affect_flags);
        af.bitvector.setValue(AFF_BERSERK);
        affect_to_char(ch,&af);

        af.modifier        = max( 10, 10 * ( slevel / 5 ) );
        af.location = APPLY_AC;
        af.bitvector.clear();
        affect_to_char(ch,&af);
    }

    else
    {
        ch->setWaitViolence( 2 );
        ch->mana -= mana / 2;

        ch->pecho("Тебе не удается войти в боевую ярость.");
        gsn_tiger_power->improve( ch, false );
    }
}

/*----------------------------------------------------------------------------
 * Ambush 
 *---------------------------------------------------------------------------*/
class AmbushOneHit: public SkillWeaponOneHit {
public:
    AmbushOneHit( Character *ch, Character *victim );

    virtual void calcTHAC0( );
    virtual void calcDamage( );
};

AmbushOneHit::AmbushOneHit( Character *ch, Character *victim )
            : Damage(ch, victim, 0, 0, DAMF_WEAPON), SkillWeaponOneHit( ch, victim, gsn_ambush )
{
}

void AmbushOneHit::calcDamage( ) 
{
    damBase( );
    damApplyEnhancedDamage( );
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
            ch->pecho("Ты не можешь быть в засаде верхом!");
            return;
    }

    one_argument( argument, arg );

    if ( ch->is_npc() || !gsn_ambush->usable( ch ) )
    {
            ch->pecho("Ты не знаешь, как устроить засаду.");
            return;
    }

    if ( arg[0] == '\0' )
    {
            if ( ch->ambushing[0] == '\0' )
            {
                    ch->pecho("Засаду кому?");
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
                    ch->pecho("Сначала тебе стоит замаскироваться.");
                    return;
            }
            ch->pecho("Ты готовишься к засаде.");
            ch->ambushing = str_dup(arg);
            return;
    }

    if ( victim == ch )
    {
            ch->pecho("Засаду себе? Это еще как?!");
            return;
    }

    if ( !IS_AFFECTED(ch,AFF_CAMOUFLAGE) )
    {
            ch->pecho("Сначала тебе стоит замаскироваться.");
            return;
    }   

    if ( victim->can_see(ch) )
    {
            ch->pecho("Твоя жертва тебя видит.");
            return;
    }

    if ( is_safe( ch, victim ) )
            return;

    ch->setWait( gsn_ambush->getBeats( )  );
    AmbushOneHit amb( ch, victim );
    
    try {
        if ( !IS_AWAKE(victim)
                || ch->is_npc()
                || number_percent( ) < gsn_ambush->getEffective( ch ) + skill_level_bonus(*gsn_ambush, ch) )
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
            ch->pecho( "{YТЫ ВЫСКАКИВАЕШЬ ИЗ ЗАСАДЫ!{x" );
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
                ch->pecho("Ты не знаешь, как замаскировать себя.");
                return;
        }

        if ( MOUNTED(ch) )
        {
                ch->pecho("Ты не можешь замаскироваться, когда ты в седле.");
                return;
        }

        if ( RIDDEN(ch) )
        {
                ch->pecho("Ты не можешь замаскироваться, когда ты оседлан%Gо||а.", ch);
                return;
        }

        if ( IS_AFFECTED( ch, AFF_FAERIE_FIRE ) )
        {
                ch->pecho("Ты не можешь замаскироваться, когда светишься.");
                return;
        }

        if (!RoomUtils::isNature(ch->in_room))
        {
                ch->pecho("Ты можешь замаскироваться только в дикой местности.");
                act("%^C1 пытается замаскироваться, но не может найти укрытия.",ch,0,0,TO_ROOM);
                return;
        }

        int k = ch->getLastFightDelay( );

        if ( k >= 0 && k < FIGHT_DELAY_TIME )
                k = k * 100 /        FIGHT_DELAY_TIME;
        else
                k = 100;

        ch->setWait( gsn_camouflage->getBeats( )  );

        if ( IS_AFFECTED(ch, AFF_CAMOUFLAGE) )
        {
                REMOVE_BIT(ch->affected_by, AFF_CAMOUFLAGE);
                ch->ambushing = &str_empty[0];
        }

        if ( ch->is_npc()
                || number_percent( ) < ( gsn_camouflage->getEffective( ch ) + skill_level_bonus(*gsn_camouflage, ch) ) * k / 100 )
        {
                ch->pecho("Ты маскируешься на местности.");
                SET_BIT(ch->affected_by, AFF_CAMOUFLAGE);
                gsn_camouflage->improve( ch, true );
        }
        else {
                ch->pecho("Твоя попытка замаскироваться закончилась неудачей.");
                gsn_camouflage->improve( ch, false );
        }

        return;
}

SPELL_DECL(RangerStaff);
VOID_SPELL(RangerStaff)::run(Character *ch, char *, int sn, int level)
{
    Object *staff;

    staff = create_object(get_obj_index(OBJ_VNUM_RANGER_STAFF), level);
    ch->pecho("Ты создаешь посох рейнджера!");
    act("%^C1 создает посох рейнджера!", ch, 0, 0, TO_ROOM);

    staff->timer = level;
    staff->level = ch->getModifyLevel();

    WeaponGenerator()
        .item(staff)
        .skill(sn)
        .valueTier(3)
        .hitrollTier(IS_GOOD(ch) ? 2 : 3)
        .damrollTier(IS_EVIL(ch) ? 2 : 3)
        .assignValues()
        .assignHitroll()
        .assignDamroll();

    obj_to_char(staff, ch);
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

    oldact_p("{BТвой посох рейнджера вспыхивает голубым светом!{x", ch, 0, 0, TO_CHAR, POS_DEAD );
    act("{BПосох рейнджера %C2 вспыхивает голубым светом!{x", ch, 0, 0, TO_ROOM );

    spell( gsn_cure_critical, ch->getModifyLevel( ), ch, ch, FSPELL_BANE );
}

bool RangerStaff::death( Character *ch )
{
    oldact_p("Твой посох рейнджера исчезает.", ch, 0, 0, TO_CHAR, POS_DEAD );
    act("Посох рейнджера %C2 исчезает.", ch, 0, 0, TO_ROOM );
    extract_obj( obj );
    return false;
}

bool RangerStaff::canEquip( Character *ch )
{
  if (ch->getProfession( ) != prof_ranger) {
        ch->pecho("Ты не знаешь как использовать эту вещь.");
        oldact("Посох рейнджера выскальзывает из твоих рук.", ch, 0, 0, TO_CHAR );
        act("Посох рейнджера выскальзывает из рук %C2.", ch, 0, 0, TO_ROOM );
        obj_from_char( obj );
        obj_to_room( obj, ch->in_room );
        return false;
    }

    return true;
}

RangerCreature::~RangerCreature( )
{
}

