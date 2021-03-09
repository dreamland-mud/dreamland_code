/* $Id: group_detection.cpp,v 1.1.2.22.6.20 2010-09-01 21:20:45 rufina Exp $
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

#include "spelltemplate.h"
#include "skillcommandtemplate.h"

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"
#include "fenia_utils.h"

#include "char.h"
#include "so.h"
#include "pcharacter.h"
#include "room.h"
#include "core/object.h"
#include "affect.h"
#include "liquid.h"
#include "magic.h"
#include "fight.h"
#include "material.h"
#include "interp.h"
#include "gsn_plugin.h"
#include "../anatolia/handler.h"
#include "comm.h"
#include "math_utils.h"
#include "recipeflags.h"
#include "act_move.h"
#include "act_lock.h"
#include "attacks.h"
#include "occupations.h"

#include "merc.h"
#include "mercdb.h"
#include "act.h"
#include "def.h"
#include "skill_utils.h"

GSN(none);
DLString quality_percent( int ); /* XXX */

/* From act_info.cpp */
void lore_fmt_item( Character *ch, Object *obj, ostringstream &buf, bool showName );
void lore_fmt_affect( Object *obj, Affect *paf, ostringstream &buf );

SPELL_DECL(AcuteVision);
VOID_SPELL(AcuteVision)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    Affect af;

    if ( CAN_DETECT(victim, ACUTE_VISION) )
    {
        if (victim == ch)
          ch->pecho("Твое зрение уже обострено до предела. ");
        else
          oldact("Зрение $C2 уже обострено до предела.", ch,0,victim,TO_CHAR);
        return;
    }
    af.bitvector.setTable(&detect_flags);
    af.type      = sn;
    af.level     = level;
    af.duration  = level;
    
    af.modifier  = 0;
    af.bitvector.setValue(ACUTE_VISION);
    affect_to_char( victim, &af );
    victim->pecho("Твое зрение обостряется.");
    if ( ch != victim )
        ch->pecho("Получилось.");
    return;
}



SPELL_DECL(DetectEvil);
VOID_SPELL(DetectEvil)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;

    if ( CAN_DETECT(victim, DETECT_EVIL) )
    {
        if (victim == ch)
          ch->pecho("Ты уже чувствуешь присутствие дьявольских сил.");
        else
          oldact("$C1 уже чувствует присутствие дьявольских сил.", ch,0,victim,TO_CHAR);
        return;
    }
    af.bitvector.setTable(&detect_flags);
    af.type      = sn;
    af.level         = level;
    af.duration  = (5 + level / 3);
    af.modifier  = 0;
    
    af.bitvector.setValue(DETECT_EVIL);
    affect_to_char( victim, &af );
    victim->pecho("Теперь ты чувствуешь {Dзло{x.");
    if ( ch != victim )
        ch->pecho("Ok.");
    return;

}



SPELL_DECL(DetectGood);
VOID_SPELL(DetectGood)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;

    if ( CAN_DETECT(victim, DETECT_GOOD) )
    {
        if (victim == ch)
          ch->pecho("Ты уже чувствуешь присутствие добрых сил.");
        else
          oldact("$C1 уже чувствует присутствие добрых сил.", ch,0,victim,TO_CHAR);
        return;
    }
    af.bitvector.setTable(&detect_flags);
    af.type      = sn;
    af.level     = level;
    af.duration  = (5 + level / 3);
    af.modifier  = 0;
    
    af.bitvector.setValue(DETECT_GOOD);
    affect_to_char( victim, &af );
    victim->pecho("Теперь ты чувствуешь присутствие {Wдобра{x.");
    if ( ch != victim )
        ch->pecho("Ok.");
    return;

}


SPELL_DECL(DetectInvis);
VOID_SPELL(DetectInvis)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;

    if ( CAN_DETECT(victim, DETECT_INVIS) )
    {
        if (victim == ch)
          ch->pecho("Ты уже видишь невидимое.");
        else
          oldact("$C1 уже видит невидимое.", ch,0,victim,TO_CHAR);
        return;
    }

    af.bitvector.setTable(&detect_flags);
    af.type      = sn;
    af.level     = level;
    af.duration  = (5 + level / 3);
    af.modifier  = 0;
    
    af.bitvector.setValue(DETECT_INVIS);
    affect_to_char( victim, &af );
    victim->pecho("Теперь ты видишь невидимое.");
    if ( ch != victim )
        ch->pecho("Ok.");
    return;

}


SPELL_DECL(DetectMagic);
VOID_SPELL(DetectMagic)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;

    if ( CAN_DETECT(victim, DETECT_MAGIC) )
    {
        if (victim == ch)
          ch->pecho("Ты уже чувствуешь магическую ауру вокруг предметов.");
        else
          oldact("$C1 уже чувствует магическую ауру вокруг предметов.", ch,0,victim,TO_CHAR);
        return;
    }

    af.bitvector.setTable(&detect_flags);
    af.type      = sn;
    af.level         = level;
    af.duration  = (5 + level / 3);
    af.modifier  = 0;
    
    af.bitvector.setValue(DETECT_MAGIC);
    affect_to_char( victim, &af );
    victim->pecho("Теперь ты чувствуешь магическую ауру вокруг предметов.");
    if ( ch != victim )
        ch->pecho("Получилось.");
    return;

}


SPELL_DECL(DetectPoison);
VOID_SPELL(DetectPoison)::run( Character *ch, Object *obj, int sn, int level ) 
{ 
    if ( obj->item_type == ITEM_DRINK_CON || obj->item_type == ITEM_FOOD )
    {
        if (IS_SET(obj->value3(), DRINK_POISONED))
            ch->pecho("Ты чувствуешь запах яда!");
        else
            ch->pecho("Уф, кажется, не отравлено.");
    }
    else
    {
        ch->pecho("Это заклинание подействует только на пищу или емкости для жидкости.");
    }

    return;

}


SPELL_DECL(DetectUndead);
VOID_SPELL(DetectUndead)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;

    if ( CAN_DETECT(ch, DETECT_UNDEAD) )
    {
                ch->pecho("Ты уже чувствуешь нежить.");

                return;
    }

    af.bitvector.setTable(&detect_flags);
    af.type      = sn;
    af.level         = level;
    af.duration  = (5 + level / 3);
    af.modifier  = 0;
    
    af.bitvector.setValue(DETECT_UNDEAD);
    affect_to_char( victim, &af );
    ch->pecho("Теперь ты чувствуешь нежить.");

    return;

}


Room * check_place( Character *ch, char *argument )
{
     char arg[MAX_INPUT_LENGTH];
     EXIT_DATA *pExit;
     Room *dest_room;
     int number, door;
     int range = ( ch->getModifyLevel() / 10) + 1;

     number = number_argument(argument,arg);
     if ((door = direction_lookup( arg )) < 0) 
         return 0;

     dest_room = ch->in_room;

     while (number > 0) {
        number--;
        
        if (--range < 1) 
            return 0;
            
        if ( (pExit = dest_room->exit[door]) == 0
          || !ch->can_see( pExit )
          || IS_SET(pExit->exit_info,EX_CLOSED) )
        {    
            break;
        }
        
        dest_room = pExit->u1.to_room;
        if (number < 1)    
            return dest_room;
     }

     return 0;
}

SPELL_DECL(Farsight);
VOID_SPELL(Farsight)::run( Character *ch, char *target_name, int sn, int level ) 
{ 
    Room *room;

    if ( (room = check_place(ch,target_name)) == 0)
      {
        ch->pecho("Так далеко тебе не видно.");
        return;
      }
    
    do_look_auto( ch, room );
}


SPELL_DECL(ImprovedDetect);
VOID_SPELL(ImprovedDetect)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
    
    Affect af;

    if ( CAN_DETECT(victim, DETECT_IMP_INVIS) )
    {
        if (victim == ch)
          ch->pecho("Ты уже чувствуешь присутствие очень невидимых существ.");
        else
          oldact("$C1 уже чувствует присутствие очень невидимых существ.", ch,0,victim,TO_CHAR);
        return;
    }

    af.bitvector.setTable(&detect_flags);
    af.type      = sn;
    af.level     = level;
    af.duration  = level / 3;
    af.modifier  = 0;
    
    af.bitvector.setValue(DETECT_IMP_INVIS);
    affect_to_char( victim, &af );
    victim->pecho("Теперь ты чувствуешь присутствие очень невидимых существ.");
    if ( ch != victim )
        ch->pecho("Ok.");
    return;

}

SPELL_DECL(KnowAlignment);
VOID_SPELL(KnowAlignment)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
        
        const char *msg;

        if ( IS_GOOD(victim) )
                msg = "$C1 имеет светлую и чистую ауру.";
        else if ( IS_NEUTRAL(victim) )
                msg = "$C1 имеет серую ауру.";
        else
                msg = "$C1 -- воплощение {Dзла{x!.";

        oldact( msg, ch, 0, victim, TO_CHAR);

        if (!victim->is_npc())
        {
                switch (victim->ethos.getValue( )) {
                case ETHOS_LAWFUL:
                        msg = "$C1 чтит закон.";
                        break;
                case ETHOS_NEUTRAL:
                        msg = "$C1 относится к закону нейтрально.";
                        break;
                case ETHOS_CHAOTIC:
                        msg = "$C1 имеет хаотический этос.";
                        break;
                default:
                        msg = "$C1 понятия не имеет, как относиться к законам.";
                        break;
                }
                oldact( msg, ch, 0, victim, TO_CHAR);
        }
        return;

}


SPELL_DECL(LocateObject);
VOID_SPELL(LocateObject)::run( Character *ch, char *target_name, int sn, int level ) 
{ 
    char buf[MAX_INPUT_LENGTH];
    ostringstream buffer;
    Object *obj;
    Object *in_obj;
    bool found;
    int number = 0, max_found;    
    DLString args = arg_unquote(target_name);

    found = false;
    number = 0;
    max_found = ch->is_immortal() ? 200 : 2 * level;

    for ( obj = object_list; obj != 0; obj = obj->next )
    {
        if ( !ch->can_see( obj ) 
            || IS_OBJ_STAT(obj, ITEM_NOLOCATE) 
            || number_percent() > 2 * level
            || ch->getModifyLevel() < obj->level)
            continue;

        if (!obj_has_name(obj, args, ch))
            continue;

        found = true;
        number++;

        for ( in_obj = obj; in_obj->in_obj != 0; in_obj = in_obj->in_obj )
            ;

        if ( in_obj->carried_by != 0 && ch->can_see(in_obj->carried_by))
        {
            sprintf( buf, "Имеется у %s.\n\r",
                ch->sees(in_obj->carried_by, '2').c_str() );
        }
        else
        {
            if (ch->is_immortal() && in_obj->in_room != 0)
                sprintf( buf, "находится в %s [Комната %d]\n\r",
                    in_obj->in_room->getName(), in_obj->in_room->vnum);
            else
                sprintf( buf, "находится в %s.\n\r",
                    in_obj->in_room == 0
                        ? "неизвестном месте" : in_obj->in_room->getName() );
        }

        buf[0] = Char::upper(buf[0]);
        buffer << buf;

        if (number >= max_found)
            break;
    }

    if ( !found )
        ch->pecho("Ни на земле, ни в небесах не найдено, увы и ах.");
    else
        page_to_char( buffer.str( ).c_str( ), ch );
}


SPELL_DECL(Observation);
VOID_SPELL(Observation)::run( Character *ch, Character *victim, int sn, int level ) 
{ 
  Affect        af;

  if( CAN_DETECT( victim, DETECT_OBSERVATION ) ) {
    ch->pecho("Ты уже можешь диагностировать состояние других существ.");
    return;
  }

  af.bitvector.setTable(&detect_flags);
  af.type        = sn;
  af.level        = level;
  af.duration        = ( 10 + level / 5 );
  
  af.modifier        = 0;
  af.bitvector.setValue(DETECT_OBSERVATION);
  affect_to_char( victim, &af );
  ch->pecho("Теперь ты можешь диагностировать состояние других существ.");
  return;

}


/*
 * 'detect hide' skill command
 */

SKILL_RUNP( detect )
{
        Affect af;

        if (!gsn_detect_hide->usable( ch ))
        {
                ch->pecho("Чего?");
                return;
        }

        if ( CAN_DETECT(ch, DETECT_HIDDEN) )
        {
                if(IS_CHARMED(ch))
                ch->master->pecho("%1$#^C1 уже видит скрытое.\n\r", ch);
                ch->pecho("Ты уже видишь скрытое. ");
                return;
        }

        ch->setWait( gsn_detect_hide->getBeats( ) );
        
        if ( number_percent( ) > gsn_detect_hide->getEffective( ch ) + skill_level_bonus(*gsn_detect_hide, ch) )
        {
                if(IS_CHARMED(ch))
                ch->master->pecho("%1$#^C1 пытается увидеть скрытое, но у не%1$Gго|го|е ничего не выходит.\n\r", ch);
                ch->pecho("Ты пытаешься увидеть скрытое, но у тебя ничего не выходит.");
                gsn_detect_hide->improve( ch, false );
                return;
        }

	int slevel = skill_level(*gsn_detect_hide, ch);
	
        af.bitvector.setTable(&detect_flags);
        af.type      = gsn_detect_hide;
        af.level     = slevel;
        af.duration  = slevel;
        
        af.modifier  = 0;
        af.bitvector.setValue(DETECT_HIDDEN);
        affect_to_char( ch, &af );
        if(IS_CHARMED(ch))
        ch->master->pecho("Теперь %1$#C1 видит скрытое.\n\r", ch);
        ch->pecho("Теперь ты видишь скрытое. ");
        gsn_detect_hide->improve( ch, true );
        return;
}


SPELL_DECL(Identify);
VOID_SPELL(Identify)::run( Character *ch, Object *obj, int sn, int level ) 
{
    ostringstream buf;

    lore_fmt_item( ch, obj, buf, true );
    ch->send_to( buf );
}


/** onLore can display more info for 'lore' command. */
static void oprog_lore(Object *obj, Character *ch)
{   
    gprog("onLore", "CO", ch, obj);
}

/*
 * 'lore' skill command
 */

SKILL_RUNP( lore )
{
  char arg1[MAX_INPUT_LENGTH];
  Object *obj;
  ostringstream buf;
  int mana, learned;
  Keyhole::Pointer keyhole;

  argument = one_argument( argument, arg1 );

    if (arg1[0] == '\0') {
        ch->pecho("Используй это умение на предмет: {y{lRлегенды{lElore{x {Dпредмет{x.");
        return;
    }

  if ( ( obj = get_obj_carry( ch, arg1 ) ) == 0 )
    {
      ch->pecho("У тебя нет этого.");
      return;
    }

    mana = gsn_lore->getMana( );
    learned = gsn_lore->getEffective( ch ) + skill_level_bonus(*gsn_lore, ch);
    
  if (ch->mana < mana)
    {
      ch->pecho("У тебя недостаточно энергии для этого.");
      return;
    }

  if (!gsn_lore->usable( ch ) || learned < 2)
    {
      ch->pecho("Ты недостаточно хорошо знаешь легенды.");
      return;
    }


  if (learned >= 2)
    {
      buf << "{W" << obj->getShortDescr( '1' ).upperFirstCharacter() << "{x"
          << " -- это {W" << item_table.message(obj->item_type )
          << " " << obj->level << "{x уровня";

      for (int i = 0; i < wearlocationManager->size( ); i++) {
        Wearlocation *loc = wearlocationManager->find( i );
        if (loc->matches( obj ) && !loc->getPurpose().empty() && !(obj->item_type == ITEM_WEAPON && IS_SET(obj->wear_flags, ITEM_WIELD)) ) {
            buf << ", " << loc->getPurpose( ).toLower( );
            break;
           }
      }
      buf << "." << endl
          << "Взаимодействует по именам: '{W" << obj->getName( ) << "{x'" << endl;   

      if (obj->weight >= 10)
        buf << "Весит {W" << obj->weight / 10 << "{x фун" << GET_COUNT(obj->weight/10, "т", "та", "тов"); 
      else
        buf << "Ничего не весит";

      buf << ", ";

      if (obj->cost)
        buf << "стоит {W" << obj->cost << "{x серебра";
      else
        buf << "ничего не стоит";

      // XXX 'изготовлено из' + падежи
        DLString mat = material_rname(obj, '1');
        if (!mat.empty())
            buf << ", материал {W" << mat << "{x";

      buf << endl;
    }

  if (learned >= 50)
    {
      int lim = obj->pIndexData->limit;

      if (lim != -1 && lim < 100)
        buf << (learned >= 80 ? fmt(0,"{RТаких вещей в мире может быть не более {W%d{x!\r\n", lim) : "{RВещь, похоже, редкая{x.\r\n");

      if (learned < 90 && obj_is_special(obj))
        buf << "{WЭтот предмет обладает неведомыми, но мощными свойствами.{x" << endl;

      if (obj->timer != 0)
        buf << (learned >= 80 ? fmt(0, "{WЭтот предмет исчезнет через %1$d мину%1$Iту|ты|т.{x\r\n", obj->timer): "{WЭтот предмет скоро исчезнет.{x\r\n");
   
      if (IS_SET(obj->extra_flags, ITEM_NOIDENT)) {
        ch->mana -= mana;
        gsn_lore->improve( ch, true );
        ch->send_to(buf.str().c_str());

        if(learned >= 90)
        oprog_lore(obj, ch);

        ch->pecho("\n\rБолее про эту вещь невозможно ничего сказать.");
        return;
      }
    }  
 
  if (learned >= 60)
    {
      bitstring_t extra = obj->extra_flags;
      REMOVE_BIT(extra, ITEM_WATER_STAND|ITEM_INVENTORY|ITEM_HAD_TIMER|ITEM_DELETED);

      if (extra)
        buf << "Особые свойства: " << extra_flags.messages(extra, true ) << endl;
    }

  if (learned >= 70)
    {  
    Liquid *liquid;
    switch (obj->item_type) {
    case ITEM_KEY:
        if (( keyhole = Keyhole::locate( ch, obj ) ))
            keyhole->doLore( buf );
        break;
    case ITEM_KEYRING:
        buf << fmt(0, "Нанизан%1$I|ы|о %1$d ключ%1$I|а|ей из возможных %2$d.", count_obj_in_obj(obj), obj->value0()) << endl;
        break;
    case ITEM_LOCKPICK:
        if (obj->value0() == Keyhole::LOCK_VALUE_BLANK) {
            buf << "Это заготовка для ключа или отмычки." << endl;
        }
        else {
            if (obj->value0() == Keyhole::LOCK_VALUE_MULTI)
                buf << "Открывает любой замок. ";
            else
                buf << "Открывает один из видов замков. ";
            
            buf << "Отмычка " 
                << quality_percent( obj->value1() ).colourStrip( ).ruscase( '2' ) 
                << " качества." << endl;
        }
        break;
    case ITEM_SPELLBOOK:
        buf << "Всего страниц: " << obj->value0() << ", из них использовано: " << obj->value1() << "." << endl
            << "Максимальное качество заклинаний в книге: " << obj->value2() << "." << endl;
        break;

    case ITEM_TEXTBOOK:
        buf << "Всего страниц: " << obj->value0() << ", из них использовано: " << obj->value1() << "." << endl
            << "Максимальное качество записей в учебнике: " << obj->value2() << "." << endl;
        break;

    case ITEM_RECIPE:
        buf << "Сложность рецепта: " << obj->value2() << ". " 
            << "Применяется для создания " << recipe_flags.messages(obj->value0(), true) << "." << endl;
        break;

    case ITEM_SCROLL:
    case ITEM_POTION:
    case ITEM_PILL:
        buf << "Заклинания " << obj->value0() << " уровня:";

        for (int i = 1; i <= 4; i++) 
            if (( skill = SkillManager::getThis( )->find( obj->valueByIndex(i) ) ))
                if (skill->getIndex( ) != gsn_none)
                    buf << " '" << skill->getNameFor( ch ) << "'";
        
        buf << endl;
        break;

    case ITEM_WAND:
    case ITEM_STAFF:
        buf << "Имеет " << obj->value2() << " заклинани" << GET_COUNT(obj->value2(), "е", "я", "й") << " " 
            << obj->value0() << " уровня:";
        
        if (( skill = SkillManager::getThis( )->find( obj->value3() ) ))
            if (skill->getIndex( ) != gsn_none)
                buf << " '" << skill->getNameFor( ch ) << "'";

        buf << endl;
        break;

    case ITEM_DRINK_CON:
        liquid = liquidManager->find( obj->value2() );
        int sips, sipsf;
        sips = max( 0, obj->value1() / liquid->getSipSize( ) );
        sipsf = max( 0, obj->value0() / liquid->getSipSize( ) );

        if (sipsf * liquid->getSipSize( ) < obj->value0()) {
            sipsf +=1;
            if (obj->value1() > 0) sips +=1;
        }

        if (obj->value1() > 0)
            buf << "Содержит " 
                << liquid->getShortDescr( ).ruscase( '4' ) << " "
                << liquid->getColor( ).ruscase( '2' ) 
                << " цвета. Осталось " << sips 
                << " из "  << sipsf << " глотков." << endl;
        else
            buf << "Видны следы "
                << liquid->getShortDescr( ).ruscase( '2' ) << " "
                << liquid->getColor( ).ruscase( '2' ) << " цвета. " 
                << fmt(0, "Объем емкости %1$d глот%1$Iок|ка|ков.", sipsf) << endl;

        break;

    case ITEM_CONTAINER:
        buf << "Вместительность: " << obj->value0() << "  "
            << "Максим. вес: " << obj->value3() << " фун" << GET_COUNT(obj->value3(), "т", "та", "тов") << " ";
        
        if (obj->value4() != 100)
            buf << " Коэф. снижения веса: " << obj->value4() << "%";
            
        if (obj->value1())
            buf << endl << "Особенности: " << container_flags.messages(obj->value1(), true );
        
        buf << endl;
        break;

    case ITEM_WEAPON:
        buf << "Тип оружия: " 
            << weapon_class.message(obj->value0() ) << " "
            << "(" << weapon_class.name( obj->value0() ) << "), ";
        
        buf << "повреждения " << obj->value1() << "d" << obj->value2() << " "
            << "(среднее " << weapon_ave(obj) << ")" << endl;
            
        if (obj->value3())  /* weapon damtype */
            buf << "Тип повреждений: " << attack_table[obj->value3()].noun << endl;            
    
        if (obj->value4())  /* weapon flags */
            buf << "Особенности оружия: " << weapon_type2.messages(obj->value4(), true ) << endl;

        break;

    case ITEM_ARMOR:
        buf << "Класс брони: ";

        for (int i = 0; i <= 3; i++)
            buf << -obj->valueByIndex(i) << " " << ac_type.message(i )
                << (i == 3 ? "" : ", ");

        buf << endl;
        break;
    }

    }

  if (learned >= 80)
    {
        if (!obj->enchanted)
          for (auto &paf: obj->pIndexData->affected)
            lore_fmt_affect( obj, paf, buf );

          for (auto &paf: obj->affected)
            lore_fmt_affect( obj, paf, buf );
    }

  ch->mana -= mana;
  gsn_lore->improve( ch, true );
  ch->send_to(buf.str().c_str());

  if (learned >= 90) oprog_lore(obj, ch);

  return;
}

