/* $Id$
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

#include <limits.h>
#include <string.h>

#include "logstream.h"
#include "skill.h"
#include "skillmanager.h"
#include "grammar_entities_impl.h"

#include "commandtemplate.h"
#include "xmlattributeticker.h"
#include "commonattributes.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "pcharactermanager.h"
#include "pcrace.h"
#include "object.h"
#include "objectbehaviormanager.h"
#include "fight.h"
#include "wizard.h"
#include "act.h"
#include "clanreference.h"
#include "arg_utils.h"
#include "save.h"

#include "loadsave.h"
#include "character.h"

#include "merc.h"
#include "def.h"

CLAN(none);
PROF(none);

typedef void                ( *FUNC_SET )( Character*, char* );

typedef        struct S_SET                T_SET;
typedef        struct S_SET_MOB        T_SET_MOB;

struct S_SET {
  const char*                const        name;
  FUNC_SET        const        func;
  const char*                const        help;
};

const                short        S_NONE        = 0;
const                short        S_V        = 1;
const                short        S_S        = 2;

struct S_SET_MOB {
  const char*                const        name;
  FUNC_SET        const        func;
  bool                const        inc_dec;
  short                const        DS;
  const char*                const        help;
};

void mset( Character*, char* );
void oset( Character*, char* );
void sset( Character*, char* );
void hset( Character*, char* );

T_SET tab_set[] = {
  { "char",        mset,        "Изменить характеристики игрока"        },
  { "obj",        oset,        "Изменить характеристики предмета"        },
  { "skill",        sset,        "Изменить skills/spells"                },
  { "help",        hset,        "Это то, что на экране"                        },
  { NULL,        NULL,        NULL,                                        }
};

void chg_mob_questt( Character*, char* );
void chg_mob_killer( Character*, char* );
void chg_mob_violent( Character*, char* );
void chg_mob_slain( Character*, char* );
void chg_mob_help( Character*, char* );
void chg_mob_attr( Character*, char* );
void chg_mob_qp( Character*, char* );

T_SET_MOB tab_set_mob[] = {
  { "questp",        chg_mob_qp,        true,  S_V,                "Кол-во quest points"                },
  { "questt",        chg_mob_questt,        true,  S_V,                "Квестовое время"                   },
  { "killer",        chg_mob_killer,        false, S_S,                "Флаг {RKILLER{x"                },
  { "violent",       chg_mob_violent,false, S_S,                "Флаг {BVIOLENT{x"                },
  { "slain",         chg_mob_slain,        false, S_S,                "Флаг {DSLAIN{x"                },
  { "attr",          chg_mob_attr, false, S_NONE, "Установка аттрибута" },
  { "help",          chg_mob_help,        false, S_NONE,                "Это то что на экране"                },
  { NULL,        NULL,                false, S_NONE,                NULL                                }
};

static PCharacter *get_player(Character *ch, char **argument)
{
  char buf[MAX_STRING_LENGTH];
  Character* victim;

  *argument = one_argument(*argument, buf);

  if (!buf[0] || !(victim = get_char_world(ch, buf, FFIND_PLR_ONLY))) {
    ch->pecho("Игрок с таким именем не найден.");
    return NULL;
  }
  return victim->getPC();
}

CMDWIZP( set )
{
  char arg[MAX_INPUT_LENGTH];
  int i = 0;
    
  argument = one_argument( argument, arg );
  if( arg[0] ) {
    while( tab_set[i].name ) {
      if( !str_prefix( arg, tab_set[i].name ) ) {
        if( tab_set[i].func ) {
          tab_set[i].func( ch, argument );
          return;
        } else {
          ch->pecho("Извините, но данная возможность находится в стадии разработки.");
          return;
        }
      }
      i++;
    }
  }
  hset( ch, NULL );
}

void hset(Character *ch, char *)
{
    ostringstream buf;
    int i = 0;

    buf << "Синтаксис:" << endl;
    while (tab_set[i].name) {
        buf << fmt(0, "  {Cset {y%-5s{x help   {C[{x %s {C]{x\n\r", tab_set[i].name, tab_set[i].help);
        i++;
    }
    ch->send_to(buf);
}

void mset(Character *ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];
    int i = 0;

    argument = one_argument(argument, arg);
    if (arg[0]) {
        while (tab_set_mob[i].name) {
            if (!str_prefix(arg, tab_set_mob[i].name)) {
                if (tab_set_mob[i].func) {
                    tab_set_mob[i].func(ch, argument);
                    return;
                } else {
                    ch->pecho("Извините, но данная возможность находится в стадии разработки.");
                    return;
                }
            }
            i++;
        }
    }
    chg_mob_help(ch, NULL);
}

void chg_mob_questt( Character* ch, char* argument ) {
  Character* victim;
  int value;
  int adv_value;

  if( ( victim = get_player( ch, &argument ) ) ) {
    XMLAttributeTimer::Pointer qd = victim->getPC( )->getAttributes( ).findAttr<XMLAttributeTimer>( "questdata" );        
    if (!qd) {
        ch->pecho("Это сейчас невозможно.");
        return;
    }

    if( is_number( argument ) ) {
        value = atoi( argument );

        if( argument[0] == '-' || argument[0] == '+' ) {
            adv_value = qd->getTime( ) + value;
        } else {
            adv_value = value;
        }

        if (victim->getPC( )->getAttributes( ).isAvailable( "quest" )) {
            if( adv_value < 2 || adv_value > 60 ) {
                ch->pecho("Значение должно лежать в пределах 2...60.");
                return;
            }
        }
        else {
            if( adv_value < 0 || adv_value > 60 ) {
                ch->pecho("Значение должно лежать в пределах 0...60.");
                return;
            }
        }

        qd->setTime( adv_value );
        ch->pecho("Текущее значение: %d",  adv_value );
        return;
    } else {
      ch->pecho("Ошибочные данные.");
    }
  }
}

void chg_mob_help(Character *ch, char *argument)
{
    ostringstream buf;
    int i = 0;

    buf << "Синтаксис:" << endl;
    while (tab_set_mob[i].name) {
        buf << fmt(0, "  {Cset{x mob {y%-8s{x {C<{xназвание{C>{x %s%s {C[{x %s {C]{x\n\r",
                tab_set_mob[i].name,
                tab_set_mob[i].inc_dec ? "[+/-]" : "     ",
                IS_SET(tab_set_mob[i].DS, S_V) &&
                        IS_SET(tab_set_mob[i].DS, S_S)
                    ? "{C<{xчис{C/{xназв{C>{x"
                    : IS_SET(tab_set_mob[i].DS, S_V) ? "{C<{x число  {C>{x" : IS_SET(tab_set_mob[i].DS, S_S) ? "{C<{x  назв. {C>{x" : "          ",
                tab_set_mob[i].help);
        i++;
    }

    ch->send_to(buf);
}

void chg_mob_killer( Character* ch, char* argument ) {
  Character* victim;

  if( ( victim = get_player( ch, &argument ) ) ) {
    if( IS_KILLER( victim )) {
      REMOVE_KILLER( victim );
      ch->pecho("Флаг {RKILLER{x убран.");
    } else {
      set_killer( victim );
      ch->pecho("Флаг {RKILLER{x выставлен.");
    }
  }
}

void chg_mob_violent( Character* ch, char* argument ) {
  Character* victim;

  if( ( victim = get_player( ch, &argument ) ) ) {
    if( IS_VIOLENT( victim ) ) {
      REMOVE_VIOLENT( victim );
      ch->pecho("Флаг {BVIOLENT{x убран.");
    } else {
      set_violent( victim );
      ch->pecho("Флаг {BVIOLENT{x выставлен.");
    }
  }
}

void chg_mob_slain( Character* ch, char* argument ) {
  Character* victim;

  if( ( victim = get_player( ch, &argument ) ) ) {
    if( IS_SLAIN( victim )) {
      REMOVE_SLAIN( victim );
      ch->pecho("Флаг {DSLAIN{x убран.");
    } else {
      set_slain( victim );
      ch->pecho("Флаг {DSLAIN{x выставлен.");
    }
  }
}

void chg_mob_qp( Character* ch, char* argument ) 
{
    PCMemoryInterface *pcm;
    DLString arguments = argument;
    DLString playerName = arguments.getOneArgument( );
    DLString qpArg = arguments.getOneArgument();

    if (playerName.empty() || qpArg.empty()) {
        ch->pecho("Укажите имя персонажа и кол-во qp.");
        return;
    }

    pcm = PCharacterManager::find( playerName );
    if (!pcm) {
        ch->pecho("Персонаж не найден, укажите имя полностью.");
        return;
    }

    int qpNow = pcm->getQuestPoints();
    
    if (qpArg.at(0) == '-' || qpArg.at(0) == '+') {
      Integer qpDelta;
      if (Integer::tryParse(qpDelta, qpArg)) {
          pcm->setQuestPoints(qpNow + qpDelta);
          PCharacterManager::saveMemory( pcm );
          ch->pecho("Персонажу %s изменено кол-во квестовых единиц с %d на %d, разница %d.",
                     pcm->getName().c_str(), qpNow, pcm->getQuestPoints(), qpDelta);
          return;
      }
    } else {
      Integer qpValue;
      if (Integer::tryParse(qpValue, qpArg)) {
          pcm->setQuestPoints(qpValue);
          PCharacterManager::saveMemory( pcm );
          ch->pecho("Персонажу %s изменено кол-во квестовых единиц с %d на %d.",
                      pcm->getName().c_str(), qpNow, pcm->getQuestPoints());
          return;
      }
    }

    ch->pecho("Использование: set char questp имя_персонажа [+|-]число");
}

void chg_mob_attr( Character* ch, char* argument ) 
{
    XMLAttributes *attrs;
    XMLAttribute::Pointer eAttr, sAttr, iAttr;
    PCMemoryInterface *pcm;
    DLString arguments = argument;
    DLString playerName = arguments.getOneArgument( );
    DLString attrName = arguments.getOneArgument( );
    DLString attrValue = arguments;
    
    pcm = PCharacterManager::find( playerName );

    if (!pcm) {
        ch->pecho("Игрок не найден, укажите имя полностью.");
        return;
    }

    if (attrName.empty( )) {
        ch->pecho( "Укажите  название аттрибута." );
        return;
    }
    
    attrs = &pcm->getAttributes( );
    eAttr = attrs->findAttr<XMLEmptyAttribute>( attrName );
    sAttr = attrs->findAttr<XMLStringAttribute>( attrName );
    iAttr = attrs->findAttr<XMLIntegerAttribute>( attrName );

    if (attrs->isAvailable( attrName )) {
        if (!iAttr && !sAttr && !eAttr) {
            ch->pecho( "Аттрибут '%s' нельзя удалить этой командой.", attrName.c_str( ) );
            return;
        }
        else {
            attrs->eraseAttribute( attrName );
            ch->pecho("Аттрибут '%s' удален.", attrName.c_str( ));
        }
    }
    else if (attrValue.empty( )) {
        attrs->getAttr<XMLEmptyAttribute>( attrName );
        ch->pecho("Аттрибут '%s' установлен.", attrName.c_str( ));
    }
    else if (attrValue.isNumber( )) {
        attrs->getAttr<XMLIntegerAttribute>( attrName )->setValue( attrValue.toInt( ) );
        ch->pecho("Численный аттрибут '%s' со значением '%s' установлен.", 
                attrName.c_str( ), attrValue.c_str( ));
    }
    else {
        attrs->getAttr<XMLStringAttribute>( attrName )->setValue( attrValue );
        ch->pecho("Строковый аттрибут '%s' со значением '%s' установлен.", 
                attrName.c_str( ), attrValue.c_str( ));
    }

    PCharacterManager::saveMemory( pcm );
}

void oset( Character* ch, char* argument ) 
{
        char arg1 [MAX_INPUT_LENGTH];
        char arg2 [MAX_INPUT_LENGTH];
        char arg3 [MAX_INPUT_LENGTH];
        Object *obj;
        int value = 0;

        argument = one_argument( argument, arg1 );
        argument = one_argument( argument, arg2 );
        strcpy( arg3, argument );

        if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
        {
                ch->pecho("Syntax:");
                ch->pecho("  set obj <object> <field> <value> [<value>]");
                ch->pecho("  Field being one of:");
                ch->pecho("    cost level material owner timer timestamp");
                ch->pecho("    weight gender personal property");
                return;
        }

        if ( ( obj = get_obj_world( ch, arg1 ) ) == 0 )
        {
                ch->pecho("Nothing like that in heaven or earth.");
                return;
        }

        // Snarf the value (which need not be numeric).
        if ( !str_cmp( arg2, "material") )
        {
            obj->setMaterial( arg3 );
        }
        else if (!str_cmp(arg2, "property")) {
            DLString value = arg3;
            DLString key = value.getOneArgument();

            if (value.empty()) {
                ch->pecho("Syntax: set obj <object> property <key> <value>");
                return;
            }

            obj->setProperty(key, value);
        }
        else
        if ( !str_cmp( arg2, "owner") )
        {
            obj->setOwner( arg3 );
        }
        else if (!str_cmp(arg2, "gender")) {
            MultiGender mg(MultiGender::UNDEF);
            mg.fromString(arg3);
            if (mg == MultiGender::UNDEF) {
              ch->pecho("Неправильное значение грам. рода, используй: neutral, female, male, plural или первые буквы n f m p.");
              return;
            }
            
            obj->gram_gender = mg;
            obj->updateCachedNoun();
            ch->pecho("%1$^O1 {Wизмене{Cн%1$Gо||а|ы.{x", obj);

        } else if ( !str_cmp( arg2, "personal") )
        {
            const DLString behaviorName = "PersonalQuestReward";

            if (obj->behavior && obj->behavior->getType() == behaviorName) {
                // Strip already existing personal item behavior.
                ch->pecho("Удаляю поведение %s с предмета %O1.", behaviorName.c_str(), obj);
                ObjectBehaviorManager::clear(obj);

            } else {
                // Allocate and assign new behavior and all related flags.
                PCMemoryInterface *owner = PCharacterManager::find(arg3);
                if (!owner) {
                    ch->pecho("Персонаж не найден, укажи имя полностью.");
                    return;
                }

                ch->pecho("Устанавливаю поведение %s и владельца %s на предмет %O1.",
                          behaviorName.c_str(), arg3, obj);

                ObjectBehaviorManager::assign(obj, behaviorName);
                if (!obj->behavior) {
                    ch->pecho("Произошла ошибка, проверь логи.");
                    return;
                }

                obj->setOwner(owner->getName());
                SET_BIT(obj->extra_flags, ITEM_NOPURGE|ITEM_NOSAC|ITEM_BURN_PROOF|ITEM_NOSELL);
                obj->setMaterial( "platinum" );
            }
        }
        else if (!str_cmp(arg2, "timestamp")) {
            obj->timestamp = atol(arg3);
        }
        else
        {
                value = atoi( arg3 );

                if ( !str_prefix( arg2, "level" ) )
                {
                        obj->level = value;
                }
                else        
                if ( !str_prefix( arg2, "weight" ) )
                {
                        obj->weight = value;
                }
                else
                if ( !str_prefix( arg2, "cost" ) )
                {
                        obj->cost = value;
                }
                else
                if ( !str_prefix( arg2, "timer" ) )
                {
                        obj->timer = value;
                }
                else
                {
                        // Generate usage message.
                        oset( ch, str_empty );
                        return;
                }
        }

        save_items_at_holder( obj );
}

void sset( Character *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];

    Character *victim;
    int value;
    int sn = -1;
    bool fAll;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    argument = one_argument( argument, arg3 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
    {
        ch->pecho("Syntax:");
        ch->pecho("  set skill {y<имя>{x <spell или skill> <число>");
        ch->pecho("  set skill {y<имя>{x <spell или skill> ?");
        ch->pecho("  set skill {y<имя>{x all <число>");
        ch->pecho("  set skill {y<имя>{x <временное умение> off");
        ch->pecho("   (use the name of the skill, not the number)");
        return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
        ch->pecho("Нет тут такого.");
        return;
    }

    if ( victim->is_npc() )
    {
        ch->pecho("Not on NPC's.");
        return;
    }

    fAll = !str_cmp( arg2, "all" );
    
    if (!fAll) {
        sn   = SkillManager::getThis( )->unstrictLookup( arg2, victim );
    
        if (sn < 0) {
            ch->pecho("No such skill or spell.");
            return;
        }
    }

    /*
     * Snarf the value.
     */
    if ( !str_cmp(arg3,"?") && !fAll )
    {
       ch->pecho("Текущее значение: %d", victim->getPC( )->getSkillData( sn ).learned.getValue( ) );
       return;
    }

    // Clean up temporary skill.
    if (arg_is_switch_off(arg3)) {
        PCSkillData &data = victim->getPC( )->getSkillData( sn );
        if (!data.isTemporary())
            ch->pecho("Это умение не является временным для персонажа.");
        else {
            data.clear();
            victim->getPC()->save();
            ch->pecho("Временное умение удалено.");
        }
        return;
    }

    if ( !is_number( arg3 ) )
    {
        ch->pecho("Value must be numeric.");
        return;
    }

    value = atoi( arg3 );
    if ( value < 0 || value > 100 )
    {
        ch->pecho("Значение должно лежать в пределах %d...%d.", 0, 100 );
        return;
    }

    if ( fAll )
    {
        
        for ( sn = 0; sn < SkillManager::getThis( )->size( ); sn++ )
            if (SkillManager::getThis( )->find( sn )->visible( victim ))
                victim->getPC( )->getSkillData( sn ).learned = value;
    }
    else
    {
        victim->getPC( )->getSkillData( sn ).learned = value;
        ch->pecho("Текущее значение: %d", value );
    }
    
    victim->getPC( )->updateSkills( );
    return;
}

