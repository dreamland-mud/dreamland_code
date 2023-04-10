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

#include "act_wiz.h"
#include "act.h"
#include "clanreference.h"
#include "arg_utils.h"
#include "save.h"
#include "mercdb.h"
#include "handler.h"
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

int decode_flags(char * arg, int * value_add, int * value_sub);

void mset( Character*, char* );
void oset( Character*, char* );
void rset( Character*, char* );
void sset( Character*, char* );
void hset( Character*, char* );

T_SET tab_set[] = {
  { "char",        mset,        "Изменить характеристики игрока"        },
  { "mob",        mset,        "Изменить характеристики монстра"        },
  { "obj",        oset,        "Изменить характеристики предмета"        },
  { "room",        rset,        "Изменить характеристики комнаты"        },
  { "skill",        sset,        "Изменить skills/spells"                },
  { "help",        hset,        "Это то, что на экране"                        },
  { NULL,        NULL,        NULL,                                        }
};

void chg_mob_str( Character*, char* );
void chg_mob_int( Character*, char* );
void chg_mob_wis( Character*, char* );
void chg_mob_dex( Character*, char* );
void chg_mob_con( Character*, char* );
void chg_mob_cha( Character*, char* );
void chg_mob_hp( Character*, char* );
void chg_mob_mana( Character*, char* );
void chg_mob_move( Character*, char* );
void chg_mob_align( Character*, char* );
void chg_mob_relig( Character*, char* );
void chg_mob_questp( Character*, char* );
void chg_mob_questt( Character*, char* );
void chg_mob_ethos( Character*, char* );
void chg_mob_level( Character*, char* );
void chg_mob_train( Character*, char* );
void chg_mob_practice( Character*, char* );
void chg_mob_gold( Character*, char* );
void chg_mob_killer( Character*, char* );
void chg_mob_violent( Character*, char* );
void chg_mob_slain( Character*, char* );
void chg_mob_help( Character*, char* );
void chg_mob_act( Character*, char* );
void chg_mob_off( Character*, char* );
void chg_mob_imm( Character*, char* );
void chg_mob_res( Character*, char* );
void chg_mob_vuln( Character*, char* );
void chg_mob_class( Character*, char* );
void chg_mob_attr( Character*, char* );
void chg_mob_qp( Character*, char* );

T_SET_MOB tab_set_mob[] = {
  { "strength",        chg_mob_str,        true,  S_V,                "Сила(str)"                        },
  { "int",        chg_mob_int,        true,  S_V,                "Интеллект(int)"                },
  { "wis",        chg_mob_wis,        true,  S_V,                "Мудрость(wis)"                 },
  { "dex",        chg_mob_dex,        true,  S_V,                "Проворность(dex)"                },
  { "con",        chg_mob_con,        true,  S_V,                "Сложение(con)"                        },
  { "cha",        chg_mob_cha,        true,  S_V,                "Обаяние(cha)"                        },
  { "hp",        chg_mob_hp,        true,  S_V,                "Кол-во жизни(hp)"                },
  { "mana",        chg_mob_mana,        true,  S_V,                "Кол-во маг-ой энергии(mana)"        },
  { "move",        chg_mob_move,        true,  S_V,                "Кол-во физ-ой энергии(move)"        },
  { "align",        chg_mob_align,        true,  S_V,                "Злой/нейтральный/добрый"        },
  { "religion",        chg_mob_relig,        false, S_V + S_S,        "Религия"                        },
  { "ethos",        chg_mob_ethos,        false, S_V + S_S,        "Законопослушность"                },
  { "questp",        chg_mob_qp,        true,  S_V,                "Кол-во quest points"                },
  { "questt",        chg_mob_questt,        true,  S_V,                "Квестовое время"                   },
  { "level",        chg_mob_level,        true,  S_V,                "Уровень моба"                        },
  { "train",        chg_mob_train,        true,  S_V,                "Количество тренировок"                },
  { "prac",        chg_mob_practice,        true,  S_V,                "Количество практик"                },
  { "gold",        chg_mob_gold,        true,  S_V,                "Количество золотых монет"        },
  { "killer",        chg_mob_killer,        false, S_S,                "Флаг {RKILLER{x"                },
  { "violent",        chg_mob_violent,false, S_S,                "Флаг {BVIOLENT{x"                },
  { "slain",        chg_mob_slain,        false, S_S,                "Флаг {DSLAIN{x"                },
        { "act",                chg_mob_act, false, S_NONE, "Флаги поведения NPC (help actflags)" },
        { "off",                chg_mob_off, false, S_NONE, "Умения NPC (help offflags)" },
        { "imm",                chg_mob_imm, false, S_NONE, "Иммуны NPC (help immflags)" },
        { "res",                chg_mob_res, false, S_NONE, "Резисты NPC (help immflags)" },
        { "vuln",                chg_mob_vuln, false, S_NONE, "Вульны NPC (help immflags)" },
        { "attr",                chg_mob_attr, false, S_NONE, "Установка аттрибута" },
  { "help",        chg_mob_help,        false, S_NONE,                "Это то что на экране"                },
  { NULL,        NULL,                false, S_NONE,                NULL                                }
};

static Character* get_CHAR(Character* ch, char** argument) {
  char buf[MAX_STRING_LENGTH];
  Character* victim;

  *argument = one_argument(*argument, buf);

  if (!buf[0] || !(victim = get_char_world(ch, buf))) {
    ch->pecho("Персонаж с таким именем не найден.");
    return NULL;
  }
  return victim;
}

static NPCharacter *get_mob(Character *ch, char **argument)
{
  char buf[MAX_STRING_LENGTH];
  Character* victim;

  *argument = one_argument(*argument, buf);

  if (!buf[0] || !(victim = get_char_world(ch, buf, FFIND_MOB_ONLY))) {
    ch->pecho("Моб с таким именем не найден.");
    return NULL;
  }
  return victim->getNPC();
}

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

void modif_STAT( Character* ch, Character* victim, char *argument, int st ) {
  char buf[MAX_INPUT_LENGTH];
  int value;
  int adv_value, min_value, max_value;

  if( is_number( argument ) ) {
    value = atoi( argument );

    if( argument[0] == '-' || argument[0] == '+' ) {
      adv_value = victim->perm_stat[st] + value;
    } else {
      adv_value = value;
    }
    
    min_value = MIN_STAT;
    max_value = (victim->is_npc( ) ? MAX_STAT : victim->getPC( )->getMaxTrain( st ));

    if( adv_value < min_value || adv_value > max_value ) {
      sprintf( buf, "Значение должно лежать в пределах %d...%d.\n\r", 
                    min_value, max_value );
      ch->send_to(buf);
      return;
    }

    victim->perm_stat[st]  = adv_value;
    sprintf( buf, "%s %d.\n\r", "Текущее значение:",
                                adv_value );
    ch->send_to(buf);
    return;
  } else {
    ch->pecho("Ошибочные данные.");
  }
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
        buf << dlprintf("  {Cset {y%-5s{x help   {C[{x %s {C]{x\n\r", tab_set[i].name, tab_set[i].help);
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

void chg_mob_str( Character* ch, char* argument ) {
  Character *victim;

  if( ( victim = get_CHAR( ch, &argument ) ) ) {
    modif_STAT( ch, victim, argument, STAT_STR );
  }
  return;
}

void chg_mob_int( Character* ch, char* argument ) {
  Character *victim;

  if( ( victim = get_CHAR( ch, &argument ) ) ) {
    modif_STAT( ch, victim, argument, STAT_INT );
  }
  return;
}

void chg_mob_wis( Character* ch, char* argument ) {
  Character *victim;

  if( ( victim = get_CHAR( ch, &argument ) ) ) {
    modif_STAT( ch, victim, argument, STAT_WIS );
  }
  return;
}

void chg_mob_dex( Character* ch, char* argument ) {
  Character *victim;

  if( ( victim = get_CHAR( ch, &argument ) ) ) {
    modif_STAT( ch, victim, argument, STAT_DEX );
  }
  return;
}

void chg_mob_con( Character* ch, char* argument ) {
  Character *victim;

  if( ( victim = get_CHAR( ch, &argument ) ) ) {
    modif_STAT( ch, victim, argument, STAT_CON );
  }
  return;
}

void chg_mob_cha( Character* ch, char* argument ) {
  Character *victim;

  if( ( victim = get_CHAR( ch, &argument ) ) ) {
    modif_STAT( ch, victim, argument, STAT_CHA );
  }
  return;
}

void chg_mob_hp( Character* ch, char* argument ) {
  Character* victim;
  char buf[MAX_INPUT_LENGTH];
  int value;
  int adv_value;

  if( ( victim = get_CHAR( ch, &argument ) ) ) {
    if( is_number( argument ) ) {
      value = atoi( argument );

      if( argument[0] == '-' || argument[0] == '+' ) {
        adv_value = !victim->is_npc() ? victim->getPC( )->perm_hit + value :
                                        (int)victim->max_hit;
      } else {
        adv_value = value;
      }
      if( adv_value <= 0 || adv_value > 30000 ) {
        sprintf( buf, "Значение должно лежать в пределах %d...%d.\n\r", 0, 30000 );
        ch->send_to(buf);
        return;
      }

      if( victim->is_npc() ) {
        victim->max_hit  = adv_value;
      } else {
        victim->max_hit  += ( adv_value - victim->getPC( )->perm_hit );
        victim->getPC( )->perm_hit = adv_value;
      }
      sprintf( buf, "%s %d.\n\r", "Текущее значение:",
                                  adv_value );
      ch->send_to(buf);
      return;
    } else {
      ch->pecho("Ошибочные данные.");
    }
  }
}

void chg_mob_mana( Character* ch, char* argument ) {
  Character* victim;
  char buf[MAX_INPUT_LENGTH];
  int value;
  int adv_value;

  if( ( victim = get_CHAR( ch, &argument ) ) ) {
    if( is_number( argument ) ) {
      value = atoi( argument );

      if( argument[0] == '-' || argument[0] == '+' ) {
        adv_value = !victim->is_npc() ? victim->getPC( )->perm_mana + value :
                                        (int)victim->max_mana;
      } else {
        adv_value = value;
      }
      if( adv_value <= 0 || adv_value > 60000 ) {
        sprintf( buf, "Значение должно лежать в пределах %d...%d.\n\r", 0, 60000 );
        ch->send_to(buf);
        return;
      }

      if( victim->is_npc() ) {
        victim->max_mana  = adv_value;
      } else {
        victim->max_mana  += ( adv_value - victim->getPC( )->perm_mana );
        victim->getPC( )->perm_mana = adv_value;
      }
      sprintf( buf, "%s %d.\n\r", "Текущее значение:",
                                  adv_value );
      ch->send_to(buf);
      return;
    } else {
      ch->pecho("Ошибочные данные.");
    }
  }
}

void chg_mob_move( Character* ch, char* argument ) {
  Character* victim;
  char buf[MAX_INPUT_LENGTH];
  int value;
  int adv_value;

  if( ( victim = get_CHAR( ch, &argument ) ) ) {
    if( is_number( argument ) ) {
      value = atoi( argument );

      if( argument[0] == '-' || argument[0] == '+' ) {
        adv_value = !victim->is_npc() ? victim->getPC( )->perm_move + value :
                                        (int)victim->max_move;
      } else {
        adv_value = value;
      }
      if( adv_value <= 0 || adv_value > 60000 ) {
        sprintf( buf, "Значение должно лежать в пределах %d...%d.\n\r", 0, 60000 );
        ch->send_to(buf);
        return;
      }

      if( victim->is_npc() ) {
        victim->max_move  = adv_value;
      } else {
        victim->max_move  += ( adv_value - victim->getPC( )->perm_move );
        victim->getPC( )->perm_move = adv_value;
      }
      sprintf( buf, "%s %d.\n\r", "Текущее значение:",
                                  adv_value );
      ch->send_to(buf);
      return;
    } else {
      ch->pecho("Ошибочные данные.");
    }
  }
}

void chg_mob_act( Character* ch, char* argument )
{
        Character* victim;
        int value;
        int add_value;
        int sub_value;

        if( ( victim = get_mob( ch, &argument ) ) )
        {
                value = decode_flags( argument, &add_value, &sub_value );

                if ( value == 0
                        && ( add_value || sub_value ) )
                {
                        victim->act.setBit( add_value );
                        victim->act.removeBit( sub_value );
                }
                else
                {
                        victim->act.setValue(value);
                }
        }
}

void chg_mob_off( Character* ch, char* argument )
{
        Character* victim;
        int value;
        int add_value;
        int sub_value;

        if( ( victim = get_mob( ch, &argument ) ) )
        {
                value = decode_flags( argument, &add_value, &sub_value );

                if ( value == 0
                        && ( add_value || sub_value ) )
                {
                        SET_BIT( victim->getNPC()->off_flags, add_value );
                        REMOVE_BIT( victim->getNPC()->off_flags, sub_value );
                }
                else
                {
                        victim->getNPC()->off_flags=value;
                }
        }
}

void chg_mob_imm( Character* ch, char* argument )
{
        Character* victim;
        int value;
        int add_value;
        int sub_value;

        if( ( victim = get_mob( ch, &argument ) ) )
        {
                value = decode_flags( argument, &add_value, &sub_value );

                if ( value == 0
                        && ( add_value || sub_value ) )
                {
                        victim->imm_flags.setBit( add_value );
                        victim->imm_flags.removeBit( sub_value );
                }
                else
                {
                        victim->imm_flags.setValue(value);
                }
        }
}

void chg_mob_res( Character* ch, char* argument )
{
        Character* victim;
        int value;
        int add_value;
        int sub_value;

        if( ( victim = get_mob( ch, &argument ) ) )
        {
                value = decode_flags( argument, &add_value, &sub_value );

                if ( value == 0
                        && ( add_value || sub_value ) )
                {
                        victim->res_flags.setBit( add_value );
                        victim->res_flags.removeBit( sub_value );
                }
                else
                {
                        victim->res_flags.setValue(value);
                }
        }
}

void chg_mob_vuln( Character* ch, char* argument )
{
        Character* victim;
        int value;
        int add_value;
        int sub_value;

        if( ( victim = get_mob( ch, &argument ) ) )
        {
                value = decode_flags( argument, &add_value, &sub_value );

                if ( value == 0
                        && ( add_value || sub_value ) )
                {
                        victim->vuln_flags.setBit( add_value );
                        victim->vuln_flags.removeBit( sub_value );
                }
                else
                {
                        victim->vuln_flags.setValue(value);
                }
        }
}

void chg_mob_align( Character* ch, char* argument ) {
  Character* victim;
  char buf[MAX_INPUT_LENGTH];
  int value;
  int adv_value;

  if( ( victim = get_CHAR( ch, &argument ) ) ) {
    if( is_number( argument ) ) {
      value = atoi( argument );

      if( argument[0] == '-' || argument[0] == '+' ) {
        adv_value = victim->alignment + value;
      } else {
        adv_value = value;
      }
      if( adv_value < -1000 || adv_value > 1000 ) {
        sprintf( buf, "Значение должно лежать в пределах %d...%d.\n\r", -1000, 1000 );
        ch->send_to(buf);
        return;
      }
      victim->alignment = adv_value;
      sprintf( buf, "%s %d.\n\r", "Текущее значение:",
                                  adv_value );
      ch->send_to(buf);
      return;
    } else {
      ch->pecho("Ошибочные данные.");
    }
  }
}

void chg_mob_relig( Character* ch, char* argument ) {
  Character* victim;
  Religion *god;

  if( ( victim = get_player( ch, &argument ) ) ) {
      if (!( god = religionManager->findUnstrict( argument ) )) {
          ch->pecho("Религия не найдена.");
          return;
      }

      victim->setReligion( god->getName( ) );
      ch->printf("Текущее значение: %s.\r\n", god->getName( ).c_str( ));
  }
}

void chg_mob_ethos( Character* ch, char* argument ) {
  Character* victim;
  int value;

  if( ( victim = get_CHAR( ch, &argument ) ) ) {
    if( is_number( argument ) ) {
      value = atoi( argument );
      if( value < 1 || value > 3 ) {
        ch->printf( "Значение должно лежать в пределах %d...%d.\n\r", 1, 3 );
        return;
      }
      victim->ethos.setValue(value);
    } else {
      victim->ethos = ethos_table.value( argument, false );
    }
    ch->printf( "Текущее значение %s.\r\n", 
                ethos_table.name( victim->ethos ).c_str( ) );
  }
}

void chg_mob_questp( Character* ch, char* argument ) {
  Character* victim;
  char buf[MAX_INPUT_LENGTH];
  int value;
  int adv_value;

  if( ( victim = get_player( ch, &argument ) ) ) {
    if( is_number( argument ) ) {
      value = atoi( argument );

      if( argument[0] == '-' || argument[0] == '+' ) {
        adv_value = victim->getPC( )->addQuestPoints(value);
      } else {
        adv_value = value;
      }
      if( adv_value < 0 || adv_value > INT_MAX ) {
        sprintf( buf, "Значение должно лежать в пределах %d...%d.\n\r", 0, INT_MAX );
        ch->send_to(buf);
        return;
      }
      victim->getPC( )->setQuestPoints(adv_value);
      sprintf( buf, "%s %d.\n\r", "Текущее значение:",
                                  adv_value );
      ch->send_to(buf);
      return;
    } else {
      ch->pecho("Ошибочные данные.");
    }
  }
}


void chg_mob_questt( Character* ch, char* argument ) {
  Character* victim;
  char buf[MAX_INPUT_LENGTH];
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
        sprintf( buf, "%s %d.\n\r", "Текущее значение:",  adv_value );
        ch->send_to(buf);
        return;
    } else {
      ch->pecho("Ошибочные данные.");
    }
  }
}

void chg_mob_gold( Character* ch, char* argument ) {
  Character* victim;
  char buf[MAX_INPUT_LENGTH];
  int value;
  int adv_value;

  if( ( victim = get_CHAR( ch, &argument ) ) ) {
    if( is_number( argument ) ) {
      value = atoi( argument );

      if( argument[0] == '-' || argument[0] == '+' ) {
        adv_value = victim->gold + value;
      } else {
        adv_value = value;
      }
      if( adv_value < 0 || adv_value > 6000 ) {
        sprintf( buf, "Значение должно лежать в пределах %d...%d.\n\r", 0, 6000 );
        ch->send_to(buf);
        return;
      }
      victim->gold = adv_value;
      sprintf( buf, "%s %d.\n\r", "Текущее значение:",
                                  adv_value );
      ch->send_to(buf);
      return;
    } else {
      ch->pecho("Ошибочные данные.");
    }
  }
}



void chg_mob_level( Character* ch, char* argument ) {
  Character* victim;
  char buf[MAX_INPUT_LENGTH];
  int value;
  int adv_value;

  if( ( victim = get_mob( ch, &argument ) ) ) {
    if( is_number( argument ) ) {
      value = atoi( argument );

      if( argument[0] == '-' || argument[0] == '+' ) {
        adv_value = victim->getRealLevel( ) + value;
      } else {
        adv_value = value;
      }
      if( adv_value <= 0 || adv_value > 110 ) {
        sprintf( buf, "Значение должно лежать в пределах %d...%d.\n\r", 0, 110 );
        ch->send_to(buf);
        return;
      }
      victim->setLevel( adv_value );
      sprintf( buf, "%s %d.\n\r", "Текущее значение:",
                                  adv_value );
      ch->send_to(buf);
      return;
    } else {
      ch->pecho("Ошибочные данные.");
    }
  }
}

void chg_mob_train( Character* ch, char* argument ) {
  Character* victim;
  char buf[MAX_INPUT_LENGTH];
  int value;
  int adv_value;

  if( ( victim = get_player( ch, &argument ) ) ) {
    if( is_number( argument ) ) {
      value = atoi( argument );

      if( argument[0] == '-' || argument[0] == '+' ) {
        adv_value = victim->getPC()->train + value;
      } else {
        adv_value = value;
      }
      if( adv_value < 0 || adv_value > 110 ) {
        sprintf( buf, "Значение должно лежать в пределах %d...%d.\n\r", 0, 110 );
        ch->send_to(buf);
        return;
      }
      victim->getPC()->train = adv_value;
      sprintf( buf, "%s %d.\n\r", "Текущее значение:",
                                  adv_value );
      ch->send_to(buf);
      return;
    } else {
      ch->pecho("Ошибочные данные.");
    }
  }
}

void chg_mob_practice( Character* ch, char* argument ) {
  Character* victim;
  char buf[MAX_INPUT_LENGTH];
  int value;
  int adv_value;

  if( ( victim = get_player( ch, &argument ) ) ) {
    if( is_number( argument ) ) {
      value = atoi( argument );

      if( argument[0] == '-' || argument[0] == '+' ) {
        adv_value = victim->getPC()->practice + value;
      } else {
        adv_value = value;
      }
      if( adv_value < 0 || adv_value > 800 ) {
        sprintf( buf, "Значение должно лежать в пределах %d...%d.\n\r", 0, 800 );
        ch->send_to(buf);
        return;
      }
      victim->getPC()->practice = adv_value;
      sprintf( buf, "%s %d.\n\r", "Текущее значение:",
                                  adv_value );
      ch->send_to(buf);
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
        buf << dlprintf("  {Cset{x mob {y%-8s{x {C<{xназвание{C>{x %s%s {C[{x %s {C]{x\n\r",
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
          ch->printf("Персонажу %s изменено кол-во квестовых единиц с %d на %d, разница %d.\r\n",
                     pcm->getName().c_str(), qpNow, pcm->getQuestPoints(), qpDelta);
          return;
      }
    } else {
      Integer qpValue;
      if (Integer::tryParse(qpValue, qpArg)) {
          pcm->setQuestPoints(qpValue);
          PCharacterManager::saveMemory( pcm );
          ch->printf("Персонажу %s изменено кол-во квестовых единиц с %d на %d.\r\n",
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
            ch->printf( "Аттрибут '%s' нельзя удалить этой командой.\r\n", attrName.c_str( ) );
            return;
        }
        else {
            attrs->eraseAttribute( attrName );
            ch->printf("Аттрибут '%s' удален.\r\n", attrName.c_str( ));
        }
    }
    else if (attrValue.empty( )) {
        attrs->getAttr<XMLEmptyAttribute>( attrName );
        ch->printf("Аттрибут '%s' установлен.\r\n", attrName.c_str( ));
    }
    else if (attrValue.isNumber( )) {
        attrs->getAttr<XMLIntegerAttribute>( attrName )->setValue( attrValue.toInt( ) );
        ch->printf("Численный аттрибут '%s' со значением '%s' установлен.\r\n", 
                attrName.c_str( ), attrValue.c_str( ));
    }
    else {
        attrs->getAttr<XMLStringAttribute>( attrName )->setValue( attrValue );
        ch->printf("Строковый аттрибут '%s' со значением '%s' установлен.\r\n", 
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
        int value = 0, value_add = 0, value_sub = 0;

        argument = one_argument( argument, arg1 );
        argument = one_argument( argument, arg2 );
        strcpy( arg3, argument );

        if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' )
        {
                ch->pecho("Syntax:");
                ch->pecho("  set obj <object> <field> <value> [<value>]");
                ch->pecho("  Field being one of:");
                ch->pecho("    value0 value1 value2 value3 value4 (v1-v4) ");
                ch->pecho("    cost extra level material owner timer wear");
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

            obj->properties[key] = value;
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
                if ( !str_cmp( arg2, "extra" )
                        || !str_cmp( arg2, "wear" )
                        || !str_cmp( arg2, "value0" ) || !str_cmp( arg2, "v0" )
                        || !str_cmp( arg2, "value1" ) || !str_cmp( arg2, "v1" )
                        || !str_cmp( arg2, "value2" ) || !str_cmp( arg2, "v2" )
                        || !str_cmp( arg2, "value3" ) || !str_cmp( arg2, "v3" )
                        || !str_cmp( arg2, "value4" ) || !str_cmp( arg2, "v4" ) )
                {
                        value = decode_flags( arg3, &value_add, &value_sub);
                }
                else
                        value = atoi( arg3 );

                if ( !str_cmp( arg2, "value0" ) || !str_cmp( arg2, "v0" ) )
                {
                        if ( value == 0 && ( value_add || value_sub ) )
                        {
                                obj->value0(obj->value0() | value_add );
                                obj->value0(obj->value0() & ~value_sub );
                        }
                        else
                                obj->value0(value);

                        obj->value0(min(101,obj->value0()));
                }
                else
                if ( !str_cmp( arg2, "value1" ) || !str_cmp( arg2, "v1" ) )
                {
                        if ( value == 0 && ( value_add || value_sub ) )
                        {
                                obj->value1(obj->value1() | value_add );
                                obj->value1(obj->value1() & ~value_sub );
                        }
                        else
                                obj->value1(value);
                }
                else
                if ( !str_cmp( arg2, "value2" ) || !str_cmp( arg2, "v2" ) )
                {
                        if ( value == 0 && ( value_add || value_sub ) )
                        {
                                obj->value2(obj->value2() | value_add );
                                obj->value2(obj->value2() & ~value_sub );
                        }
                        else
                                obj->value2(value);
                }
                else
                if ( !str_cmp( arg2, "value3" ) || !str_cmp( arg2, "v3" ) )
                {
                        if ( value == 0 && ( value_add || value_sub ) )
                        {
                                obj->value3(obj->value3() | value_add );
                                obj->value3(obj->value3() & ~value_sub );
                        }
                        else
                                obj->value3(value);
                }
                else
                if ( !str_cmp( arg2, "value4" ) || !str_cmp( arg2, "v4" ) )
                {
                        if ( value == 0 && ( value_add || value_sub ) )
                        {
                                obj->value4(obj->value4() | value_add );
                                obj->value4(obj->value4() & ~value_sub );
                        }
                        else
                                obj->value4(value);
                }
                else
                if ( !str_prefix( arg2, "extra" ) )
                {
                        if ( value == 0 && ( value_add || value_sub ) )
                        {
                                SET_BIT( obj->extra_flags, value_add );
                                REMOVE_BIT( obj->extra_flags, value_sub );
                        }
                        else
                                obj->extra_flags = value;
                }
                else
                if ( !str_prefix( arg2, "wear" ) )
                {
                        if ( value == 0 && ( value_add || value_sub ) )
                        {
                                SET_BIT( obj->wear_flags, value_add );
                                REMOVE_BIT( obj->wear_flags, value_sub );
                        }
                        else
                                obj->wear_flags = value;
                }
                else
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

void rset( Character* ch, char* argument ) 
{
}

void sset( Character *ch, char *argument )
{
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    char arg3 [MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];

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
       sprintf( buf, "%s %d.\n\r", "Текущее значение:", victim->getPC( )->getSkillData( sn ).learned.getValue( ) );
       ch->send_to(buf);
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
        sprintf( buf, "Значение должно лежать в пределах %d...%d.\n\r", 0, 100 );
        ch->send_to(buf);
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
        sprintf( buf, "%s %d.\n\r", "Текущее значение:", value );
        ch->send_to(buf);
    }
    
    victim->getPC( )->updateSkills( );
    return;
}

