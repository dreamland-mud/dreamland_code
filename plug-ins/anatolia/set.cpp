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

#include "commandtemplate.h"
#include "xmlattributeticker.h"
#include "commonattributes.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "pcharactermanager.h"
#include "pcrace.h"
#include "object.h"

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
CLAN(outsider);
PROF(none);
PROF(universal);

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
void chg_mob_skillpoints( Character*, char* );
void chg_mob_help( Character*, char* );
void chg_mob_act( Character*, char* );
void chg_mob_off( Character*, char* );
void chg_mob_imm( Character*, char* );
void chg_mob_res( Character*, char* );
void chg_mob_vuln( Character*, char* );
void chg_mob_class( Character*, char* );
void chg_mob_uniclass( Character*, char* );
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
        { "skillpoints", chg_mob_skillpoints, true, S_V, "максимум skillpoints" },
        { "act",                chg_mob_act, false, S_NONE, "Флаги поведения NPC (help actflags)" },
        { "off",                chg_mob_off, false, S_NONE, "Умения NPC (help offflags)" },
        { "imm",                chg_mob_imm, false, S_NONE, "Иммуны NPC (help immflags)" },
        { "res",                chg_mob_res, false, S_NONE, "Резисты NPC (help immflags)" },
        { "vuln",                chg_mob_vuln, false, S_NONE, "Вульны NPC (help immflags)" },
        { "class",                chg_mob_class, false, S_NONE, "Смена класса (только для Universal)" },
        { "uniclass",                chg_mob_uniclass, false, S_NONE, "Смена подкласса для Universal" },
        { "attr",                chg_mob_attr, false, S_NONE, "Установка аттрибута" },
  { "help",        chg_mob_help,        false, S_NONE,                "Это то что на экране"                },
  { NULL,        NULL,                false, S_NONE,                NULL                                }
};

Character* get_CHAR( Character* ch, char** argument ) {
  char buf[MAX_STRING_LENGTH];
  Character* victim;

  *argument = one_argument( *argument, buf );

  if( !buf[0] || !( victim = get_char_world( ch, buf ) ) ) {
    ch->send_to( "Игрок(моб) с таким названием не найден.\n\r" );
    return NULL;
  }
  return victim;
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
    ch->send_to("Ошибочные данные.\n\r" );
  }
}


CMDWIZP( set )
{
  char arg[MAX_INPUT_LENGTH];
  int i = 0;
    
  if (ch->getPC( ) && ch->getPC( )->getAttributes( ).isAvailable( "noset" )) {
      ch->println( "It's not a good idea." );
      return;
  }

  argument = one_argument( argument, arg );
  if( arg[0] ) {
    while( tab_set[i].name ) {
      if( !str_prefix( arg, tab_set[i].name ) ) {
        if( tab_set[i].func ) {
          tab_set[i].func( ch, argument );
          return;
        } else {
          ch->send_to("Извините, но данная возможность находится в стадии разработки.\n\r" );
          return;
        }
      }
      i++;
    }
  }
  hset( ch, NULL );
}

void hset( Character* ch, char* ) {
  char buf[MAX_STRING_LENGTH];
  char buf1[MAX_STRING_LENGTH];
  int i = 0;

  buf[0] = '\0';
  while( tab_set[i].name ) {
    sprintf( buf1, "  {Cset {y%-5s{x help   {C[{x %s {C]{x\n\r", tab_set[i].name, tab_set[i].help );
    strcat( buf, buf1 );
    i++;
  }
  sprintf( buf1, "%s:\n\r%s", "Синтаксис", buf );
  ch->send_to(buf1);
}

void mset( Character* ch, char* argument ) {
  char arg[MAX_INPUT_LENGTH];
  int i = 0;

  argument = one_argument( argument, arg );
  if( arg[0] ) {
    while( tab_set_mob[i].name ) {
      if( !str_prefix( arg, tab_set_mob[i].name ) ) {
        if( tab_set_mob[i].func ) {
          tab_set_mob[i].func( ch, argument );
          return;
        } else {
          ch->send_to("Извините, но данная возможность находится в стадии разработки.\n\r");
          return;
        }
      }
      i++;
    }
  }
  chg_mob_help( ch, NULL );
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
      ch->send_to("Ошибочные данные.\n\r");
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
      ch->send_to("Ошибочные данные.\n\r");
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
      ch->send_to("Ошибочные данные.\n\r");
    }
  }
}

void chg_mob_act( Character* ch, char* argument )
{
        Character* victim;
        int value;
        int add_value;
        int sub_value;

        if( ( victim = get_CHAR( ch, &argument ) ) )
        {
                if ( !victim->is_npc() )
                {
                        ch->send_to ("Это не для игроков!\n\r");
                        return;
                }

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
        else
        {
                ch->send_to("Такого NPC в Мире нет.\n\r");
        }
}

void chg_mob_off( Character* ch, char* argument )
{
        Character* victim;
        int value;
        int add_value;
        int sub_value;

        if( ( victim = get_CHAR( ch, &argument ) ) )
        {
                if ( !victim->is_npc() )
                {
                        ch->send_to ("Это не для игроков!\n\r");
                        return;
                }

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
        else
        {
                ch->send_to("Такого NPC в Мире нет.\n\r");
        }
}

void chg_mob_imm( Character* ch, char* argument )
{
        Character* victim;
        int value;
        int add_value;
        int sub_value;

        if( ( victim = get_CHAR( ch, &argument ) ) )
        {
                if ( !victim->is_npc() )
                {
                        ch->send_to ("Это не для игроков!\n\r");
                        return;
                }

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
        else
        {
                ch->send_to("Такого NPC в Мире нет.\n\r");
        }
}

void chg_mob_res( Character* ch, char* argument )
{
        Character* victim;
        int value;
        int add_value;
        int sub_value;

        if( ( victim = get_CHAR( ch, &argument ) ) )
        {
                if ( !victim->is_npc() )
                {
                        ch->send_to ("Это не для игроков!\n\r");
                        return;
                }

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
        else
        {
                ch->send_to("Такого NPC в Мире нет.\n\r");
        }
}

void chg_mob_vuln( Character* ch, char* argument )
{
        Character* victim;
        int value;
        int add_value;
        int sub_value;

        if( ( victim = get_CHAR( ch, &argument ) ) )
        {
                if ( !victim->is_npc() )
                {
                        ch->send_to ("Это не для игроков!\n\r");
                        return;
                }

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
        else
        {
                ch->send_to("Такого NPC в Мире нет.\n\r");
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
      ch->send_to("Ошибочные данные.\n\r");
    }
  }
}

void chg_mob_relig( Character* ch, char* argument ) {
  Character* victim;
  Religion *god;

  if( ( victim = get_CHAR( ch, &argument ) ) ) {
      if (victim->is_npc()) {
          ch->send_to("Not for NPC!\r\n");
          return;
      }

      if (!( god = religionManager->findUnstrict( argument ) )) {
          ch->println("Религия не найдена.");
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

  if( ( victim = get_CHAR( ch, &argument ) ) ) {
    if( victim->is_npc() ) {
      ch->send_to("Данный параметр можно изменить только игрокам.\n\r");
      return;
    }
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
      ch->send_to("Ошибочные данные.\n\r");
    }
  }
}


void chg_mob_questt( Character* ch, char* argument ) {
  Character* victim;
  char buf[MAX_INPUT_LENGTH];
  int value;
  int adv_value;

  if( ( victim = get_CHAR( ch, &argument ) ) ) {
    if( victim->is_npc() ) {
      ch->send_to("Данный параметр можно изменить только игрокам.\n\r");
      return;
    }
    
    XMLAttributeTimer::Pointer qd = victim->getPC( )->getAttributes( ).findAttr<XMLAttributeTimer>( "questdata" );        
    if (!qd) {
        ch->send_to("Это сейчас невозможно.\n\r");
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
                ch->send_to( "Значение должно лежать в пределах 2...60.\n\r" );
                return;
            }
        }
        else {
            if( adv_value < 0 || adv_value > 60 ) {
                ch->send_to( "Значение должно лежать в пределах 0...60.\n\r" );
                return;
            }
        }

        qd->setTime( adv_value );
        sprintf( buf, "%s %d.\n\r", "Текущее значение:",  adv_value );
        ch->send_to(buf);
        return;
    } else {
      ch->send_to("Ошибочные данные.\n\r");
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
      ch->send_to("Ошибочные данные.\n\r");
    }
  }
}



void chg_mob_level( Character* ch, char* argument ) {
  Character* victim;
  char buf[MAX_INPUT_LENGTH];
  int value;
  int adv_value;

  if( ( victim = get_CHAR( ch, &argument ) ) ) {
    if( !victim->is_npc() ) {
      ch->send_to("Данный параметр можно изменить только мобам.\n\rИспользуйте advance <char> level.\n\r");
      return;
    }
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
      ch->send_to("Ошибочные данные.\n\r");
    }
  }
}

void chg_mob_train( Character* ch, char* argument ) {
  Character* victim;
  char buf[MAX_INPUT_LENGTH];
  int value;
  int adv_value;

  if( ( victim = get_CHAR( ch, &argument ) ) ) {
      if (victim->is_npc()) {
          ch->send_to("Not for NPC!\r\n");
          return;
      }

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
      ch->send_to("Ошибочные данные.\n\r");
    }
  }
}

void chg_mob_practice( Character* ch, char* argument ) {
  Character* victim;
  char buf[MAX_INPUT_LENGTH];
  int value;
  int adv_value;

  if( ( victim = get_CHAR( ch, &argument ) ) ) {
      if (victim->is_npc()) {
          ch->send_to("Not for NPC!\r\n");
          return;
      }

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
      ch->send_to("Ошибочные данные.\n\r");
    }
  }
}

void chg_mob_help( Character* ch, char* argument ) {
  char buf[MAX_STRING_LENGTH];
  char buf1[MAX_STRING_LENGTH];
  int i = 0;

  buf[0] = '\0';
  while( tab_set_mob[i].name ) {
    sprintf( buf1, "  {Cset{x mob {y%-8s{x {C<{xназвание{C>{x %s%s {C[{x %s {C]{x\n\r",
             tab_set_mob[i].name,
             tab_set_mob[i].inc_dec ? "[+/-]" : "     ",
             IS_SET( tab_set_mob[i].DS, S_V ) &&
             IS_SET( tab_set_mob[i].DS, S_S ) ?
               "{C<{xчис{C/{xназв{C>{x" :
               IS_SET( tab_set_mob[i].DS, S_V ) ?
                 "{C<{x число  {C>{x" :
                 IS_SET( tab_set_mob[i].DS, S_S ) ?
                   "{C<{x  назв. {C>{x" :
                   "          ",
             tab_set_mob[i].help );
    strcat( buf, buf1 );
    i++;
  }
  sprintf( buf1, "%s:\n\r%s", "Синтаксис", buf );
  ch->send_to(buf1);
}

void chg_mob_killer( Character* ch, char* argument ) {
  Character* victim;

  if( ( victim = get_CHAR( ch, &argument ) ) ) {
    if( victim->is_npc() ) {
      ch->send_to("Это не игрок!\n\r");
      return;
    }
    if( IS_KILLER( victim ) || victim->is_immortal() ) {
      REMOVE_KILLER( victim );
      ch->send_to("{RKILLER{x remove.\n\r");
    } else {
      set_killer( victim );
      ch->send_to("{RKILLER{x set.\n\r");
    }
  }
}

void chg_mob_violent( Character* ch, char* argument ) {
  Character* victim;

  if( ( victim = get_CHAR( ch, &argument ) ) ) {
    if( victim->is_npc() ) {
      ch->send_to("Это не игрок!\n\r");
      return;
    }
    if( IS_VIOLENT( victim ) || victim->is_immortal() ) {
      REMOVE_VIOLENT( victim );
      ch->send_to("{BVIOLENT{x remove.\n\r");
    } else {
      set_violent( victim );
      ch->send_to("{BVIOLENT{x set.\n\r");
    }
  }
}

void chg_mob_slain( Character* ch, char* argument ) {
  Character* victim;

  if( ( victim = get_CHAR( ch, &argument ) ) ) {
    if( victim->is_npc() ) {
      ch->send_to("Это не игрок!\n\r");
      return;
    }
    if( IS_SLAIN( victim ) || victim->is_immortal() ) {
      REMOVE_SLAIN( victim );
      ch->send_to("{DSLAIN{x remove.\n\r");
    } else {
      set_slain( victim );
      ch->send_to("{DSLAIN{x set.\n\r");
    }
  }
}


void chg_mob_skillpoints( Character* ch, char* argument )
{
  Character* victim;
  char buf[MAX_INPUT_LENGTH];
  int value;
  int adv_value;

  if( ( victim = get_CHAR( ch, &argument ) ) )
        {
                if( victim->is_npc() )
                {
      ch->send_to("Данный параметр можно изменить только игрокам.\n\r");
      return;
    }

    if( is_number( argument ) )
                {
      value = atoi( argument );

                        if( argument[0] == '-' || argument[0] == '+' )
                        {
                                adv_value = victim->getPC()->max_skill_points + value;
                        }
                        else
                        {
                                adv_value = value;
                        }
                        if( adv_value < 0 || adv_value > 100000 )
                        {
                                sprintf( buf, "Значение должно лежать в пределах %d...%d.\n\r", 0, 100000 );
                                ch->send_to(buf);
                                return;
                        }
                        victim->getPC()->max_skill_points = adv_value;
                        sprintf( buf, "%s %d.\n\r", "Текущее значение:",adv_value );
                        ch->send_to(buf);
                        return;
                }
                else
                {
                        ch->send_to("Ошибочные данные.\n\r");
                }
        }
}

/*
 * UNIVERSAL PROFESSIONS
 */
void chg_mob_class( Character* ch, char* argument )
{
  Character* vict;
  char arg[MAX_STRING_LENGTH];
  int sn;

  if( ( vict = get_CHAR( ch, &argument ) ) )        {
        PCharacter *victim;
        Profession *prof;

        if( vict->is_npc() )        {
            ch->send_to("Данный параметр можно изменить только игрокам.\n\r");
            return;
        }
        
        victim = vict->getPC();

        if (victim->getProfession() != prof_universal) {
            ch->send_to("Новый класс устанавливается только для Universals.\n\r");
            return;
        }
    
        argument = one_argument(argument, arg );
        prof = professionManager->findUnstrict( arg ); 
        
        if (!prof || prof_universal == prof) {
            ch->send_to("Неверное имя класса.\n\r");
            return;
        }
        
        if (victim->getRace( )->getPC( )->getClasses( )[prof->getIndex( )] <= 0) {
            ch->send_to("Новый класс недоступен для такой рассы.\n\r");
            return;
        }
        
        if (!prof->getSex( ).isSetBitNumber( victim->getSex( ) )) {
            ch->send_to("Для этого придется сделать операцию по смене пола.\n\r");
            return;
        }
        
        if (!prof->getAlign( ).isSetBitNumber( ALIGNMENT(victim) )) {
            ch->send_to("Неподходящий характер.\n\r");
            return;
        }

        if (!prof->getEthos( ).isSetBitNumber( victim->ethos )) {
            ch->send_to( "Неподходящий ethos.\r\n" );
            return;
        }

        if (PCharacterManager::pfBackup(victim->getName()))
            ch->send_to("Profile backuped. Use 'recover' command for roll back.\n\r");
        else {
            ch->send_to("Unable to backup the profile.\n\r");
            return;
        }
        
        victim->setProfession( prof->getName( ) );
        victim->setSubProfession( prof_none );

        for (sn = 0; sn < skillManager->size( ); sn++) {
            PCSkillData &pcskill = victim->getSkillData( sn );
            
            if (!skillManager->find( sn )->visible( victim )) {
                if (pcskill.learned > 1)
                    victim->practice++;
                    
                pcskill.learned = 0;
            }
            else if (pcskill.learned < 75 && pcskill.learned >= 50) {
                pcskill.learned = 75;
            }

            pcskill.forgetting = false;
            pcskill.timer = 0;
        }

        victim->max_skill_points = 1000;
        victim->exp = victim->getExpPerLevel( victim->getLevel( ) );
        
        if (victim->getClan( ) != clan_none && !victim->getClan( )->canInduct( victim )) {
            victim->setClan( clan_outsider );
            victim->setClanLevel( 0 );
            
            victim->printf("You have been inducted into %s.", victim->getClan( )->getLongName( ).c_str( ));
            ch->send_to("Forced induct into Outsiders.\n\r");
        }

        victim->updateSkills( );        
        victim->save( );

        ch->printf( "%s теперь %s!", victim->getNameP( '1' ).c_str( ), prof->getName( ).c_str( ) );
        victim->printf( "Теперь ты %s!", prof->getName( ).c_str( ) );        

  }

}

void chg_mob_uniclass( Character* ch, char* argument )
{
    char arg[MAX_INPUT_LENGTH];
    Character* vict;

  if( ( vict = get_CHAR( ch, &argument ) ) )        {
        Profession *prof;
        PCharacter *victim;

        if( vict->is_npc() )        {
            ch->send_to("Данный параметр можно изменить только игрокам.\n\r");
            return;
        }
        
        victim = vict->getPC();

        if (victim->getProfession( ) != prof_universal) {
            ch->send_to("Подклассы устанавливаются только для Universals.\n\r");
            return;
        }
    
        argument = one_argument(argument, arg);
        prof = professionManager->findUnstrict( arg );
                
        if (!prof || prof_universal == prof) {
            ch->send_to("Неверное имя класса.\n\r");
            return;
        }
        
        if (!prof->getSex( ).isSetBitNumber( victim->getSex( ) )) {
            ch->send_to("Для этого придется сделать операцию по смене пола.\n\r");
            return;
        }
        
        if (!prof->getAlign( ).isSetBitNumber( ALIGNMENT(victim) )) {
            ch->send_to("Неподходящий характер.\n\r");
            return;
        }

        if (!prof->getEthos( ).isSetBitNumber( victim->ethos )) {
            ch->send_to( "Неподходящий ethos.\r\n" );
            return;
        }
        
        victim->setSubProfession( prof->getName( ) );
        victim->updateSkills( );
        victim->save( );

        ch->send_to("Ok.\n\r");
        ch->printf( "Теперь ты будешь изображать из себя %s.\r\n", prof->getName( ).c_str( ) );
  }


}


void chg_mob_qp( Character* ch, char* argument ) 
{
    PCMemoryInterface *pcm;
    DLString arguments = argument;
    DLString playerName = arguments.getOneArgument( );
    DLString qpArg = arguments.getOneArgument();

    if (playerName.empty() || qpArg.empty()) {
        ch->println("Укажите имя персонажа и кол-во qp.");
        return;
    }

    pcm = PCharacterManager::find( playerName );
    if (!pcm) {
        ch->send_to( "Персонаж не найден, укажите имя полностью.\r\n" );
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

    ch->println("Использование: set char questp имя_персонажа [+|-]число");
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
        ch->send_to( "Игрок не найден, укажите имя полностью.\r\n" );
        return;
    }

    if (attrName.empty( )) {
        ch->println( "Укажите  название аттрибута." );
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
                ch->send_to("Syntax:\n\r");
                ch->send_to("  set obj <object> <field> <value>\n\r");
                ch->send_to("  Field being one of:\n\r");
                ch->send_to("    value0 value1 value2 value3 value4 (v1-v4) \n\r");
                ch->send_to("    cost extra level material owner timer wear weight personal\n\r");
                return;
        }

        if ( ( obj = get_obj_world( ch, arg1 ) ) == 0 )
        {
                ch->send_to("Nothing like that in heaven or earth.\n\r");
                return;
        }

        // Snarf the value (which need not be numeric).
        if ( !str_cmp( arg2, "material") )
        {
            obj->setMaterial( arg3 );
        }
        else
        if ( !str_cmp( arg2, "owner") )
        {
            obj->setOwner( arg3 );
        }
        else
        if ( !str_cmp( arg2, "personal") )
        {
            obj->setOwner( arg3 );
            SET_BIT(obj->extra_flags, ITEM_NOPURGE|ITEM_NOSAC|ITEM_BURN_PROOF|ITEM_NOSELL);
            obj->setMaterial( "platinum" );

            try {
                AllocateClass::Pointer p = Class::allocateClass( "PersonalQuestReward" );

                if (p) {
                    obj->behavior.setPointer( p.getDynamicPointer<ObjectBehavior>( ) );
                    obj->behavior->setObj( obj );
                }
            } catch (ExceptionClassNotFound e) {
                LogStream::sendError( ) << e.what( ) << endl;
                ch->send_to( e.what( ) );
                return;
            }
        }
        else if (!str_cmp(arg2, "timestamp")) {
            obj->timestamp = atol(arg3);
            return;
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
                                SET_BIT( obj->value[0], value_add );
                                REMOVE_BIT( obj->value[0], value_sub );
                        }
                        else
                                obj->value[0] = value;

                        obj->value[0] = min(101,obj->value[0]);
                }
                else
                if ( !str_cmp( arg2, "value1" ) || !str_cmp( arg2, "v1" ) )
                {
                        if ( value == 0 && ( value_add || value_sub ) )
                        {
                                SET_BIT( obj->value[1], value_add );
                                REMOVE_BIT( obj->value[1], value_sub );
                        }
                        else
                                obj->value[1] = value;
                }
                else
                if ( !str_cmp( arg2, "value2" ) || !str_cmp( arg2, "v2" ) )
                {
                        if ( value == 0 && ( value_add || value_sub ) )
                        {
                                SET_BIT( obj->value[2], value_add );
                                REMOVE_BIT( obj->value[2], value_sub );
                        }
                        else
                                obj->value[2] = value;
                }
                else
                if ( !str_cmp( arg2, "value3" ) || !str_cmp( arg2, "v3" ) )
                {
                        if ( value == 0 && ( value_add || value_sub ) )
                        {
                                SET_BIT( obj->value[3], value_add );
                                REMOVE_BIT( obj->value[3], value_sub );
                        }
                        else
                                obj->value[3] = value;
                }
                else
                if ( !str_cmp( arg2, "value4" ) || !str_cmp( arg2, "v4" ) )
                {
                        if ( value == 0 && ( value_add || value_sub ) )
                        {
                                SET_BIT( obj->value[4], value_add );
                                REMOVE_BIT( obj->value[4], value_sub );
                        }
                        else
                                obj->value[4] = value;
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
        ch->send_to("Syntax:\n\r");
        ch->send_to("  set skill {y<имя>{x <spell или skill> <число>\n\r");
        ch->send_to("  set skill {y<имя>{x <spell или skill> ?\n\r");
        ch->send_to("  set skill {y<имя>{x all <число>\n\r");
        ch->send_to("  set skill {y<имя>{x <временное умение> off\n\r");
        ch->send_to("   (use the name of the skill, not the number)\n\r");
        return;
    }

    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
    {
        ch->send_to("Нет тут такого.\n\r");
        return;
    }

    if ( victim->is_npc() )
    {
        ch->send_to("Not on NPC's.\n\r");
        return;
    }

    fAll = !str_cmp( arg2, "all" );
    
    if (!fAll) {
        sn   = SkillManager::getThis( )->unstrictLookup( arg2, victim );
    
        if (sn < 0) {
            ch->send_to("No such skill or spell.\n\r");
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
            ch->println("Это умение не является временным для персонажа.");
        else {
            data.clear();
            victim->getPC()->save();
            ch->println("Временное умение удалено.");
        }
        return;
    }

    if ( !is_number( arg3 ) )
    {
        ch->send_to("Value must be numeric.\n\r");
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

