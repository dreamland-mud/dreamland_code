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
/***************************************************************************
 *     ANATOLIA 2.1 is copyright 1996-1997 Serdar BULUT, Ibrahim CANPUNAR  *        
 *     ANATOLIA has been brought to you by ANATOLIA consortium                   *
 *         Serdar BULUT {Chronos}                bulut@rorqual.cc.metu.edu.tr       *
 *         Ibrahim Canpunar  {Asena}        canpunar@rorqual.cc.metu.edu.tr    *        
 *         Murat BICER  {KIO}                mbicer@rorqual.cc.metu.edu.tr           *        
 *         D.Baris ACAR {Powerman}        dbacar@rorqual.cc.metu.edu.tr           *        
 *     By using this code, you have agreed to follow the terms of the      *
 *     ANATOLIA license, in the file Anatolia/anatolia.licence             *        
 ***************************************************************************/
/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*        ROM 2.4 is copyright 1993-1995 Russ Taylor                           *
*        ROM has been brought to you by the ROM consortium                   *
*            Russ Taylor (rtaylor@pacinfo.com)                                   *
*            Gabrielle Taylor (gtaylor@pacinfo.com)                           *
*            Brian Moore (rom@rom.efn.org)                                   *
*        By using this code, you have agreed to follow the terms of the           *
*        ROM license, in the file Rom24/doc/rom.license                           *
***************************************************************************/
#include "attacks.h"
#include "damageflags.h"
#include "grammar_entities_impl.h"
#include "merc.h"
#include "def.h"

using namespace Grammar;

/* attack table */
struct attack_type        attack_table        []                =
{
    { "none",                "удар",                        -1,                MultiGender::MASCULINE },  /*  0 */
    { "slice",                "разрезающий удар",        DAM_SLASH,        MultiGender::MASCULINE },        
    { "stab",                "выпад",                DAM_PIERCE,        MultiGender::MASCULINE },
    { "slash",                "рубящий удар",                DAM_SLASH,        MultiGender::MASCULINE },
    { "whip",                "хлесткий удар",        DAM_SLASH,        MultiGender::MASCULINE },
    { "claw",                "удар когтями",                DAM_SLASH,        MultiGender::MASCULINE },  /*  5 */
    { "blast",                "залп",                DAM_BASH,        MultiGender::MASCULINE },
    { "pound",                "тяжелый удар",                DAM_BASH,        MultiGender::MASCULINE },
    { "crush",                "дробящий удар",        DAM_BASH,        MultiGender::MASCULINE },
    { "grep",                "захват",                 DAM_SLASH,        MultiGender::MASCULINE },
    { "bite",                "укус",                        DAM_PIERCE,        MultiGender::MASCULINE },  /* 10 */
    { "pierce",                "глубокий выпад",        DAM_PIERCE,        MultiGender::MASCULINE },
    { "suction",        "засасывание",    DAM_DROWNING,        MultiGender::NEUTER },
    { "beating",        "серия ударов",                DAM_BASH,        MultiGender::FEMININE },
    { "digestion",        "кислотная слизь",        DAM_ACID,        MultiGender::FEMININE },
    { "charge",                "удар с разбегу",                DAM_BASH,        MultiGender::MASCULINE},  /* 15 */
    { "slap",                "шлепок",                DAM_BASH,        MultiGender::MASCULINE },
    { "punch",                "удар кулаком",                DAM_BASH,        MultiGender::MASCULINE },
    { "wrath",                "гнев",                  DAM_ENERGY,        MultiGender::MASCULINE },
    { "magic",                "магический удар",        DAM_ENERGY,        MultiGender::MASCULINE },
    { "divine",                "божественная энергия",        DAM_HOLY,        MultiGender::FEMININE},  /* 20 */
    { "cleave",                "раскалывающий удар",        DAM_SLASH,        MultiGender::MASCULINE },
    { "scratch",        "царапающий удар",        DAM_PIERCE,        MultiGender::MASCULINE },
    { "peck",                "удар клювом",                DAM_PIERCE,        MultiGender::MASCULINE },
    { "peckb",                "клюющий удар",                DAM_PIERCE,        MultiGender::MASCULINE },
    { "chop",                "рубящий удар",                DAM_SLASH,        MultiGender::MASCULINE },  /* 25 */
    { "sting",                "жалящий удар",                DAM_PIERCE,        MultiGender::MASCULINE },
    { "smash",                "разбивающий удар",        DAM_BASH,        MultiGender::MASCULINE },
    { "shbite",                "шокирующий укус",      DAM_LIGHTNING,        MultiGender::MASCULINE },
    { "flbite",                "обжигающий укус",        DAM_FIRE,        MultiGender::MASCULINE },
    { "frbite",                "леденящий укус",       DAM_COLD,        MultiGender::MASCULINE },  /* 30 */
    { "acbite",                "окисляющий укус",      DAM_ACID,        MultiGender::MASCULINE },
    { "chomp",                "грызущий удар",                DAM_PIERCE,        MultiGender::MASCULINE },
    { "drain",                "темная энергия",DAM_NEGATIVE,        MultiGender::FEMININE },
    { "thrust",                "выпад",                DAM_PIERCE,        MultiGender::MASCULINE },
    { "slime",                "жижа",        DAM_DROWNING,        MultiGender::FEMININE },
    { "shock",                "разряд",                DAM_LIGHTNING,        MultiGender::MASCULINE },
    { "thwack",                "удар с размаху",        DAM_BASH,        MultiGender::MASCULINE },
    { "flame",                "вспышка",                DAM_LIGHT,        MultiGender::FEMININE},
    { "chill",                "холод",                DAM_COLD,        MultiGender::MASCULINE },
    { "cuff",                "подзатыльник",                DAM_BASH,        MultiGender::MASCULINE },
    { "hooves",                "удар копытами",        DAM_BASH,        MultiGender::MASCULINE },
    { "horns",                "удар рогами",                DAM_BASH,        MultiGender::MASCULINE },
    { "spines",         "удар иголками",              DAM_PIERCE,         MultiGender::MASCULINE }, 
    { "cacophony",      "какофония",              DAM_SOUND,      MultiGender::FEMININE}, 
    { "poisonbite",      "отравляющий укус",              DAM_POISON,      MultiGender::MASCULINE}, 
    { "tearbite",      "разрывающий укус",              DAM_SLASH,      MultiGender::MASCULINE},
    { "mental",      "ментальный удар",              DAM_MENTAL,      MultiGender::MASCULINE},
    { "disease",      "чумные миазмы",              DAM_DISEASE,      MultiGender::PLURAL},
    { "charm",      "неотразимость",              DAM_CHARM,      MultiGender::FEMININE},
    { "sound",      "звуковая волна",              DAM_SOUND,      MultiGender::FEMININE},    
    { 0,                0,                        0                }
};

