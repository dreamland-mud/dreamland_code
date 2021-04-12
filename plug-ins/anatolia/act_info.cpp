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
 *     ANATOLIA has been brought to you by ANATOLIA consortium             *
 *         Serdar BULUT {Chronos}             bulut@rorqual.cc.metu.edu.tr *
 *         Ibrahim Canpunar  {Asena}       canpunar@rorqual.cc.metu.edu.tr *
 *         Murat BICER  {KIO}                mbicer@rorqual.cc.metu.edu.tr *
 *         D.Baris ACAR {Powerman}        dbacar@rorqual.cc.metu.edu.tr    *
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

#include <map>
#include <sstream>
#include <cmath>
#include <algorithm>

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"
#include "char.h"
#include "logstream.h"
#include "grammar_entities_impl.h"
#include "morphology.h"

#include "skill.h"
#include "skillmanager.h"
#include "skillgroup.h"
#include "skillcommand.h"
#include "spell.h"
#include "affecthandler.h"
#include "mobilebehavior.h"
#include "xmlattributeticker.h"
#include "commonattributes.h"
#include "commandtemplate.h"
#include "playerattributes.h"

#include "affect.h"
#include "core/object.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "pcrace.h"
#include "room.h"
#include "desire.h"
#include "helpmanager.h"
#include "attacks.h"
#include "areahelp.h"

#include "dreamland.h"
#include "merc.h"
#include "descriptor.h"
#include "comm.h"
#include "weapons.h"
#include "colour.h"
#include "material.h"
#include "mudtags.h"
#include "websocketrpc.h"
#include "bugtracker.h"
#include "act.h"
#include "alignment.h"
#include "interp.h"
#include "levenshtein.h"

#include "occupations.h"
#include "raceflags.h"
#include "recipeflags.h"
#include "gsn_plugin.h"
#include "def.h"
#include "act_move.h"
#include "act_lock.h"
#include "handler.h"
#include "stats_apply.h"
#include "arg_utils.h"
#include "vnum.h"
#include "mercdb.h"
#include "commandflags.h"
#include "damageflags.h"
#include "autoflags.h"

using std::endl;
using std::min;
using std::max;

PROF(none);
PROF(samurai);
PROF(anti_paladin);
RELIG(none);
GSN(none);
GSN(gratitude);

/* command procedures needed */
bool omprog_give( Object *obj, Character *ch, Character *victim );
void password_set( PCMemoryInterface *pci, const DLString &plainText );
bool password_check( PCMemoryInterface *pci, const DLString &plainText );
DLString quality_percent( int ); /* XXX */
DLString help_article_disambig(const HelpArticle *help);

CMDRUNP( prompt )
{
    DLString old;

    if ( argument[0] == '\0' )
    {
        if (IS_SET(ch->comm,COMM_PROMPT))
        {
            ch->pecho("Вывод строки состояния (prompt) выключен.");
            REMOVE_BIT(ch->comm,COMM_PROMPT);
        }
        else
        {
            ch->pecho("Вывод строки состояния (prompt) включен.");
            SET_BIT(ch->comm,COMM_PROMPT);
        }
        return;
    }

    if (arg_is_all( argument )) {
        old = ch->prompt;
        ch->prompt = "<{r%h{x/{R%H{xзд {c%m{x/{C%M{xман %v/%Vшг {W%X{xоп Вых:{g%d{x>%c";
    }
    else if (arg_is_show( argument )) {
        ch->pecho( "Текущая строка состояния:" );
        ch->desc->send( ch->prompt.c_str( ) );
        ch->pecho("");
        return;
    }    
    else {
          old = ch->prompt;
        ch->prompt = argument;
    }
    
    if (!old.empty( )) {
            ch->send_to( "Предыдущая строка состояния: " );
            ch->desc->send(  old.c_str( ) );   
               ch->pecho("");
    }
    ch->printf("Новая строка состояния: %s\n\r",ch->prompt.c_str( ) );
}

CMDRUNP( battleprompt )
{
    DLString old;

   if ( argument[0] == '\0' )
   {
      ch->pecho("Необходимо указать вид строки состояния.\nДля получения более подробной информации используй 'help prompt'");
      return;
   }

    if (arg_is_all( argument )) {
        old = ch->batle_prompt;
        ch->batle_prompt = "<{r%h{x/{R%H{xзд {c%m{x/{C%M{xман %v/%Vшг %Xоп Вых:{g%d{x> [{r%y{x:{Y%o{x]%c";
    }
    else if (arg_is_show( argument )) {
        ch->pecho( "Текущая строка состояния в бою:" );
        ch->desc->send( ch->batle_prompt.c_str( ) );
        ch->pecho("");
        return;
    }    
    else {
        old = ch->batle_prompt;
        ch->batle_prompt = argument;
    }

    if (!old.empty( )) {
            ch->send_to( "Предыдущая строка состояния в бою: " );
            ch->desc->send(  old.c_str( ) );   
               ch->pecho("");
    }
    ch->printf("Новая строка состояния в бою: %s\n\r",ch->batle_prompt.c_str( ) );
}

static DLString show_money( int g, int s )
{
    ostringstream buf;

    if (g > 0 || s > 0)
        buf << (g > 0 ? "{Y%1$d{x золот%1$Iая|ые|ых" : "")
            << (g * s == 0 ? "" : " и ")
            << (s > 0 ? "{W%2$d{x серебрян%2$Iая|ые|ых" : "")
            << " моне%" << (s == 0 ? "1" : "2") << "$Iта|ты|т.";
    else
        buf << "нет денег.";
    
    return fmt( NULL, buf.str( ).c_str( ), g, s );
}

static DLString show_experience( PCharacter *ch )
{
    return fmt( NULL, "У тебя %1$d очк%1$Iо|а|ов опыта. "
               "До следующего уровня осталось %2$d очк%2$Iо|а|ов из %3$d.",
               ch->exp.getValue( ),
               ch->getExpToLevel( ),
               ch->getExpPerLevel( ch->getLevel( ) + 1 ) - ch->getExpPerLevel( ) );
}

CMDRUNP( worth )
{
    ch->send_to( "У тебя " );
    ch->pecho( show_money( ch->gold, ch->silver ) );

    if ( ch->is_npc() )
            return;

    ch->pecho( show_experience( ch->getPC( ) ) );

    ch->pecho("Ты уби%Gло|л|ла %3d %s и %3d %s персонажей.",
            ch, 
            ch->getPC( )->has_killed.getValue( ),
            IS_GOOD(ch) ? "не добрых" : IS_EVIL(ch) ? "не злых" : "не нейтральных",
            ch->getPC( )->anti_killed.getValue( ),
            IS_GOOD(ch) ? "добрых" : IS_EVIL(ch) ? "злых" : "нейтральных" );
}


#define MAX_MSGTABLE_SIZE 25
struct msgpair_t {
    int value;
    const char *msg;
};
typedef msgpair_t msgtable_t [MAX_MSGTABLE_SIZE];

const char * msgtable_lookup( const msgtable_t &table, int value )
{
    for (int i = 0; table[i].value != -1; i++)
        if (table[i].value > value)
            return (i == 0 ? table[i].msg : table[i-1].msg);
        else if (table[i].value == value)
            return table[i].msg;
        else if (table[i+1].value == -1)
            return table[i].msg;


    return "";
}

msgtable_t msg_positions = {
    { POS_DEAD,     "Ты ТРУП!!!"                  },
    { POS_MORTAL,   "Ты при смерти."               },
    { POS_INCAP,    "Ты в беспомощном состоянии." },
    { POS_STUNNED,  "Тебя оглушили."              },
    { POS_SLEEPING, "Ты спишь."                   },
    { POS_RESTING,  "Ты отдыхаешь."               },
    { POS_SITTING,  "Ты сидишь."                  },
    { POS_FIGHTING, "Ты сражаешься."              },
    { POS_STANDING, "Ты стоишь."                  },
    { -1 }
};


CMDRUNP( oscore )
{
    ostringstream buf;
    Room *room = 0;
    PCharacter *pch = ch->getPC( );

    buf << fmt( 0, "Ты {W%1$s%2$s{x, уровень {C%3$d{w",
                   ch->seeName( ch, '1' ).c_str( ),
                   ch->is_npc( ) ? "" : ch->getPC( )->getParsedTitle( ).c_str( ),
                   ch->getRealLevel( ));
    
    if (!ch->is_npc( ))
        buf << fmt( 0, ", тебе %1$d %1$Iгод|года|лет (%2$d ча%2$Iс)|са)|сов).",
                        pch->age.getYears( ), pch->age.getHours( ) ); 
    
    buf << endl;

    if (ch->getRealLevel( ) != ch->get_trust( ))
        buf << "Уровень доверия к тебе составляет " << ch->get_trust( ) << "." << endl;

    buf << "{wРаса:{W " << ch->getRace( )->getNameFor( ch, ch )
    << "  {wРазмер:{W " << size_table.message( ch->size )    
        << "  {wПол:{W " << sex_table.message( ch->getSex( ) )
        << "  {wКласс:{W " << ch->getProfession( )->getNameFor( ch );
    
    if (!ch->is_npc( ))
        room = get_room_instance( ch->getPC()->getHometown( )->getAltar() );
    else
        room = get_room_instance( ROOM_VNUM_TEMPLE );
    
    buf << "  {wДом:{W " << (room ? room->areaName() : "Потерян" ) << "{x" << endl
        << dlprintf( "У тебя {R%d{x/{r%d{x жизни, {C%d{x/{c%d{x энергии и %d/%d движения.\n\r",
                    ch->hit.getValue( ), ch->max_hit.getValue( ), 
                    ch->mana.getValue( ), ch->max_mana.getValue( ), 
                    ch->move.getValue( ), ch->max_move.getValue( ));
    
    if (!ch->is_npc( )) 
        buf << fmt( 0, "У тебя {Y%1$d{x практи%1$Iка|ки|к и {Y%2$d{x тренировочн%2$Iая|ые|ых сесси%2$Iя|и|й.",
                       pch->practice.getValue( ), pch->train.getValue( ) )
            << endl;
    
    buf << dlprintf( "Ты несешь {W%d/%d{x вещей с весом {W%d/%d{x фунтов.\n\r",
                ch->carry_number, ch->canCarryNumber( ),
                ch->getCarryWeight( )/10, ch->canCarryWeight( )/10 );

    if (ch->is_npc( )) {
        buf << dlprintf( 
            "Твои параметры: исходные, (текущие)\n\r"
            "      Сила(Str): %d(%d) Интеллект(Int): %d(%d)\n\r"
            "  Мудрость(Wis): %d(%d)  Ловкость(Dex): %d(%d)\n\r"
            "  Сложение(Con): %d(%d)   Обаяние(Cha): %d(%d)\n\r",
            ch->perm_stat[STAT_STR], ch->getCurrStat(STAT_STR),
            ch->perm_stat[STAT_INT], ch->getCurrStat(STAT_INT),
            ch->perm_stat[STAT_WIS], ch->getCurrStat(STAT_WIS),
            ch->perm_stat[STAT_DEX], ch->getCurrStat(STAT_DEX),
            ch->perm_stat[STAT_CON], ch->getCurrStat(STAT_CON),
            ch->perm_stat[STAT_CHA], ch->getCurrStat(STAT_CHA) );

    } else {
        buf << dlprintf( 
            "Твои параметры: исходные, {c({Wтекущие{c){x, [{Cмаксимальные{x]\n\r"
            "      Сила(Str): %d{c({W%d{c){x [{C%d{x] Интеллект(Int): %d{c({W%d{c){x [{C%d{x]\n\r"
            "  Мудрость(Wis): %d{c({W%d{c){x [{C%d{x]  Ловкость(Dex): %d{c({W%d{c){x [{C%d{x]\n\r"
            "  Сложение(Con): %d{c({W%d{c){x [{C%d{x]   Обаяние(Cha): %d{c({W%d{c){x [{C%d{x]\n\r",
            ch->perm_stat[STAT_STR], ch->getCurrStat(STAT_STR), pch->getMaxStat(STAT_STR),
            ch->perm_stat[STAT_INT], ch->getCurrStat(STAT_INT), pch->getMaxStat(STAT_INT),
            ch->perm_stat[STAT_WIS], ch->getCurrStat(STAT_WIS), pch->getMaxStat(STAT_WIS),
            ch->perm_stat[STAT_DEX], ch->getCurrStat(STAT_DEX), pch->getMaxStat(STAT_DEX),
            ch->perm_stat[STAT_CON], ch->getCurrStat(STAT_CON), pch->getMaxStat(STAT_CON),
            ch->perm_stat[STAT_CHA], ch->getCurrStat(STAT_CHA), pch->getMaxStat(STAT_CHA) );

    }

    buf << dlprintf( "У тебя {W%d{x очков опыта, и %s\n\r",
                  ch->exp.getValue( ),
                  show_money( ch->gold, ch->silver ).c_str( ) );

    /* KIO shows exp to level */
    if (!ch->is_npc() && ch->getRealLevel( ) < LEVEL_HERO - 1)
        buf << dlprintf( "Тебе нужно набрать {W%d{x очков опыта до следующего уровня.\n\r",
                    ch->getPC()->getExpToLevel( ) );

    if (!ch->is_npc( )) {
        XMLAttributeTimer::Pointer qd = pch->getAttributes( ).findAttr<XMLAttributeTimer>( "questdata" );
        int qtime = qd ? qd->getTime( ) : 0;
        bool hasQuest = pch->getAttributes( ).isAvailable( "quest" );
        
        buf << fmt( 0, "У тебя {Y%1$d{x квестов%1$Iая|ые|ых едини%1$Iца|цы|ц. ",
                       pch->getQuestPoints() );
        if (qtime == 0)
            buf << "У тебя сейчас нет задания.";
        else
            buf << fmt( 0, "До %1$s квеста осталось {Y%2$d{x ти%2$Iк|ка|ков.",
                       hasQuest ? "конца" : "следующего",
                       qtime );

        buf << endl;

        bool newline = false;

        if (ch->getProfession( ) != prof_samurai) {
            if (ch->wimpy > 0) {
                buf << dlprintf( "Ты попытаешься убежать при %d жизни.  ", ch->wimpy.getValue( ) );
                newline = true;
            }
        } else {
            if (ch->getPC()->death > 0)
                buf << fmt(0, "Тебя убили уже {r%1$d{x ра%1$Iз|за|з.", ch->getPC()->death.getValue());
            else
                buf << "Тебя еще ни разу не убивали.";
            newline = true;
        }
        
        if (ch->getPC()->guarding != 0) {
            buf << dlprintf( "Ты охраняешь: %s. ", ch->seeName( ch->getPC()->guarding, '4' ).c_str( ) );
            newline = true;
        }

        if (ch->getPC()->guarded_by != 0) {
            buf << dlprintf( "Ты охраняешься: %s.", ch->seeName( ch->getPC()->guarded_by, '5' ).c_str( ) );
            newline = true;
        }
        
        if (newline)
            buf << endl;
    }

    // Report current desire status, as progress bar or percents.
    if (!ch->is_npc( )) {
        ostringstream dbuf;

        for (int i = 0; i < desireManager->size( ); i++) {
            ostringstream b;
            
            desireManager->find( i )->report( ch->getPC( ), b );
            
            if (!b.str( ).empty( ))
                dbuf << b.str( ) << " ";
        }

        if (!dbuf.str( ).empty( ))
            buf << dbuf.str( ) << endl;
    }
    
    buf << msgtable_lookup( msg_positions, ch->position );

    if (ch->is_adrenalined( ) && ch->position > POS_INCAP)
        buf << " Твоя кровь полна адреналина!";
    
    buf << endl;

    /* print AC values */
    buf << dlprintf( "Защита от укола {W%d{x, от удара {W%d{x, от разрезания {W%d{x, от экзотики {W%d{x.\n\r",
            GET_AC(ch,AC_PIERCE),
            GET_AC(ch,AC_BASH),
            GET_AC(ch,AC_SLASH),
            GET_AC(ch,AC_EXOTIC));
    buf << dlprintf( "{lRТочность{lEHitroll{lx: {C%d{x  {lRУрон{lEDamroll{lx: {C%d{x  {lRЗащита от заклинаний{lESaves vs Spell{lx: {C%d{x\n\r",
                ch->hitroll.getValue( ), ch->damroll.getValue( ), ch->saving_throw.getValue( ) );

    buf << dlprintf( "У тебя %s натура.  ", align_name( ch ).ruscase( '1' ).c_str( ) );
    
    switch (ch->ethos.getValue( )) {
    case ETHOS_LAWFUL:
            buf << "Ты почитаешь порядок.\n\r";
            break;
    case ETHOS_NEUTRAL:
            buf << "У тебя нейтральный этос.\n\r";
            break;
    case ETHOS_CHAOTIC:
            buf << "Ты хаотик.\n\r";
            break;
    default:
            if (!ch->is_npc( ))
                buf << "У тебя нет этоса, сообщи об этом Богам!\n\r";
            else
                buf << "\n\r";
            break;
    }
    
    if (!ch->is_npc( )) {
        if (ch->getReligion( ) == god_none)
            buf << "Ты не веришь в бога.  ";
        else
            buf << dlprintf( "Твоя религия: {C%s{x.  ", ch->getReligion( )->getNameFor( ch ).ruscase( '1' ).c_str( ));
        
        buf << dlprintf("Твои заслуги перед законом:  %d.\n\r", ch->getPC( )->loyalty.getValue( ));
    }
    
    /* RT wizinvis and holy light */
    if (ch->is_immortal( )) 
        buf << dlprintf( "Божественный взор %s. Невидимость %d уровня, инкогнито %d уровня.",
                   (IS_SET(ch->act, PLR_HOLYLIGHT) ? "включен" : "выключен"),
                   ch->getPC( )->invis_level.getValue( ),
                   ch->getPC( )->incog_level.getValue( ) )
            << endl;

    // Collect information from various attributes, such as craft professions.    
    if (pch) {
        list<DLString> attrLines;
        if (pch->getAttributes( ).handleEvent( ScoreArguments( pch, attrLines ) ))
                for (list<DLString>::iterator l = attrLines.begin( ); l != attrLines.end( ); l++) {
                                buf << *l << endl;
                        }
    }

    if (IS_GHOST(ch)) {
        buf << fmt(0, "{xТы призрак и обретёшь плоть через {Y%1$3d {xсекун%1$-1Iду.|ды.|д.",
                 pch->ghost_time*(PULSE_MOBILE/dreamland->getPulsePerSecond()))
        << endl;
    }

    ch->send_to( buf );

    if (IS_SET(ch->comm, COMM_SHOW_AFFECTS))
        interpret_raw( ch, "affects", "nocolor noempty" );
}

CMDRUNP( compare )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    Object *obj1;
    Object *obj2;
    int value1;
    int value2;
    const char *msg;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( arg1[0] == '\0' )
    {
        ch->pecho("Сравнить что и с чем.?");
        return;
    }

    if ( ( obj1 = get_obj_carry( ch, arg1 ) ) == 0 )
    {
        ch->pecho("У тебя нет этого.");
        return;
    }

    if (arg2[0] == '\0')
    {
        for (obj2 = ch->carrying; obj2 != 0; obj2 = obj2->next_content)
        {
            if (obj2->wear_loc != wear_none
            &&  ch->can_see(obj2)
            &&  obj1->item_type == obj2->item_type
            &&  (obj1->wear_flags & obj2->wear_flags & ~ITEM_TAKE) != 0 )
                break;
        }

        if (obj2 == 0)
        {
            ch->pecho("На тебе нет ничего, с чем можно было бы сравнить.");
            return;
        }
    }

    else if ( (obj2 = get_obj_carry(ch,arg2) ) == 0 )
    {
        ch->pecho("У тебя нет этого.");
        return;
    }

    msg                = 0;
    value1        = 0;
    value2        = 0;

    if ( obj1 == obj2 )
    {
        msg = "Ты сравниваешь %1$O4 с сам%1$Gим|им|ой|ими собой. Выглядят одинаково.";
    }
    else if ( obj1->item_type != obj2->item_type )
    {
        msg = "Ты не можешь сравнить %1$O4 и %2$O4.";
    }
    else
    {
        switch ( obj1->item_type )
        {
        default:
            msg = "Ты не можешь сравнить %1$O4 и %2$O4.";
            break;

        case ITEM_ARMOR:
            value1 = obj1->value0() + obj1->value1() + obj1->value2();
            value2 = obj2->value0() + obj2->value1() + obj2->value2();
            break;

        case  ITEM_WEAPON:
            value1 = weapon_ave(obj1);
            value2 = weapon_ave(obj2);
            break;
        }
    }

    if ( msg == 0 )
    {
             if ( value1 == value2 ) msg = "%1$^O1 и %2$O1 выглядят одинаково.";
        else if ( value1  > value2 ) msg = "%1$^O1 выгляд%1$nит|ят лучше чем %2$O1.";
        else                         msg = "%1$^O1 выгляд%1$nит|ят хуже чем %2$O1.";
    }
    
    ch->pecho( msg, obj1, obj2 );
}



static void format_where( Character *ch, Character *victim )
{
    bool fPK, fAfk;
    
    fPK = (!victim->is_npc( ) 
            && victim->getModifyLevel( ) >= PK_MIN_LEVEL 
            && !is_safe_nomessage( ch, victim->getDoppel( ch ) ));
    fAfk = IS_SET(victim->comm, COMM_AFK);

    ch->pecho( "%-25C1 {x%s{x%s %-42s{x",
                victim,
                fPK  ? "({rPK{x)"  : "    ",
                fAfk ? "[{CAFK{x]" : "     ",
                victim->in_room->getName() );
}

static bool rprog_where( Character *ch, const char *arg )
{
    FENIA_CALL( ch->in_room, "Where", "Cs", ch, arg );
    return false;
}

CMDRUNP( where )
{
    Character *victim = 0;
    Descriptor *d;
    bool found;
    bool fPKonly = false;
    DLString arg( argument );

    ch->setWaitViolence( 1 );

    if (eyes_blinded( ch )) {
        ch->pecho( "Ты не можешь видеть вещи!" );
        return;
    }
    
    if (eyes_darkened( ch )) {
        ch->pecho( "Ты ничего не видишь! Слишком темно!" );
        return;
    }

    arg.stripWhiteSpace( );
    arg.toLower( );
    
    if (arg_is_pk( arg ))
        fPKonly = true;
    
    if (rprog_where( ch, arg.c_str( ) ))
        return;

    if (arg.empty( ) || fPKonly)
    {
        ch->printf( "Ты находишься в местности {W{hh%s{x. Недалеко от тебя:\r\n",
                     ch->in_room->areaName() );
        found = false;

        for ( d = descriptor_list; d; d = d->next )
        {
            if (d->connected != CON_PLAYING)
                continue;
            if (( victim = d->character ) == 0)
                continue;
            if (victim->is_npc( ))
                continue;
            if (!victim->in_room || victim->in_room->area != ch->in_room->area)
                continue;
            if (IS_SET(victim->in_room->room_flags, ROOM_NOWHERE))
                continue;
            if (!ch->can_see( victim ))
                continue;
            if (fPKonly && is_safe_nomessage( ch, victim ))
                continue;
            
            found = true;
            format_where( ch, victim );
        }

        if (!found)
            ch->pecho("Никого.");
    }
    else
    {
        found = false;
        for ( victim = char_list; victim != 0; victim = victim->next )
        {
            if ( victim->in_room != 0
                    && victim->in_room->area == ch->in_room->area
                    && ( !victim->is_npc()
                    || ( victim->is_npc() && !IS_SET(victim->act, ACT_NOWHERE) ) )
                    && ch->can_see( victim )
                    && is_name( arg.c_str(), victim->getNameP( '7' ).c_str() )
                    && !IS_SET(victim->in_room->room_flags, ROOM_NOWHERE))
            {
                found = true;
                format_where( ch, victim );
            }
        }

        if (!found)
            oldact("Ты не находишь $T.", ch, 0, arg.c_str(), TO_CHAR);
    }
}




CMDRUNP( consider )
{
    char arg[MAX_INPUT_LENGTH];
    Character *victim;
    const char *msg;
    const char *align;
    int diff;

    one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
        ch->pecho("Сравнить свои силы с кем?");
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == 0 )
    {
        ch->pecho("Его нет здесь.");
        return;
    }

    if (is_safe(ch,victim))
    {
        ch->pecho("Даже не думай об этом.");
        return;
    }

    victim = victim->getDoppel( );

    diff = victim->getModifyLevel() - ch->getModifyLevel();

         if ( diff <= -10 ) msg = "Ты можешь убить $C4 даже без оружия.";
    else if ( diff <=  -5 ) msg = "$C1 не соперн$Gик|ик|ица тебе.";
    else if ( diff <=  -2 ) msg = "Ты похоже легко убьешь $C4.";
    else if ( diff <=   1 ) msg = "Прекрасный поединок!";
    else if ( diff <=   4 ) msg = "$C1 говорит 'Чувствуешь удачу, шпана?'.";
    else if ( diff <=   9 ) msg = "$C1 смеется над твоей беспомощностью.";
    else                    msg = "Ты станешь приятным подарком СМЕРТИ!";

    if (IS_EVIL(ch) && IS_EVIL(victim))
      align = "$C1 злобно усмехается тебе.";
    else if (IS_GOOD(victim) && IS_GOOD(ch))
      align = "$C1 радушно приветствует тебя.";
    else if (IS_GOOD(victim) && IS_EVIL(ch))
      align = "$C1 улыбается тебе, надеясь привлечь тебя на путь добра.";
    else if (IS_EVIL(victim) && IS_GOOD(ch))
      align = "$C1 злобно смеется над тобой.";
    else if (IS_NEUTRAL(ch) && IS_EVIL(victim))
      align = "$C1 злобно усмехается.";
    else if (IS_NEUTRAL(ch) && IS_GOOD(victim))
      align = "$C1 счастливо улыбается.";
    else if (IS_NEUTRAL(ch) && IS_NEUTRAL(victim))
      align = "$C1 выглядит довольно непривлекательно.";
    else
      align = "$C1 выглядит совершенно непривлекательно.";

    oldact( msg, ch, 0, victim, TO_CHAR);
    oldact( align, ch, 0, victim, TO_CHAR);
    return;
}

static bool fix_title( PCharacter *ch, DLString &title )
{
    if (DLString( "{" ).strSuffix( title )
        && !DLString( "{{" ).strSuffix( title ))
    {
        title.cutSize( title.length( ) - 1 );
    }

    title.replaces( "{/", "" );
    title.replaces( "{*", "" );
    title.replaces( "{+", "" );

    if (title.colorLength( ) > 50) {
        ch->pecho( "Слишком длинный титул." );
        return false;
    }
    
    return true;
}

CMDRUNP( title )
{
    DLString arg = argument;
    PCharacter *pch = ch->getPC( );

    if (ch->is_npc( ))
        return;

    if (IS_SET(ch->act, PLR_NO_TITLE)) {
        ch->pecho( "Ты не можешь сменить титул." );
        return;
    }
    
    if (arg.empty( ) || arg_is_show( arg )) {
        ostringstream buf;
        const DLString &title = pch->getTitle( );
        DLString parsed = pch->getParsedTitle( );
        parsed.stripWhiteSpace( );
        
        if (parsed.empty( )) {
            buf << "У тебя нет титула." << endl;
        }
        else {
            buf << "Ты носишь титул " << parsed << "{x";
            if (parsed != title)
                buf << " (" << title << "{x)";
            buf << "." << endl;
        }

        pch->send_to( buf );
        return;
    }

    if (arg == "clear" || arg == "очистить") {
        pch->setTitle( DLString::emptyString );
        pch->pecho( "Титул удален." );
        return;
    }

    if (!fix_title( pch, arg ))
        return;

    pch->setTitle( arg );

    pch->printf( "Теперь ты {W%s{x%s{x\n\r", 
                 pch->getName( ).c_str( ), 
                 pch->getParsedTitle( ).c_str( ) );
}

static bool fix_pretitle( PCharacter *ch, DLString &title )
{
    ostringstream buf;
    mudtags_convert( 
        title.c_str( ), buf, 
        TAGS_CONVERT_COLOR|TAGS_ENFORCE_NOWEB|TAGS_ENFORCE_NOCOLOR|TAGS_ENFORCE_RAW );

    DLString stripped = buf.str( );
    DLString nospace = stripped;
    nospace.stripWhiteSpace( );
   
    if (stripped.size( ) > 25) {
        ch->pecho( "Слишком длинный претитул!" );
        return false;
    }
    
    if (nospace.size( ) != stripped.size( )) {
        ch->pecho( "В начале или в конце претитула не должно быть пробелов." );
        return false;
    }
    
    for (unsigned int i = 0; i < stripped.size( ); i++)
        if (!dl_isalpha( stripped[i] ) 
                && stripped[i] != ' ' 
                && stripped[i] != '\'') 
        {
            ch->pecho( "В претитуле разрешено использовать только буквы, пробелы и одинарные кавычки." );
            return false;
        }

    if (stripped.size( ) != title.size( )) {
        DLString buf;
        buf << "{1" << title << "{x{2";
        title = buf;
    }

    return true;
}

CMDRUNP( pretitle )
{
    PCharacter *pch = ch->getPC( );
    DLString arg = argument;
    DLString rus;

    if (!pch)
        return;

    if (IS_SET(pch->act, PLR_NO_TITLE)) {
         pch->pecho( "Ты не можешь изменить претитул.");
         return;
    }
    
    if (arg.empty( ) || arg_is_show( arg )) {
        DLString eng = pch->getPretitle( );
        DLString rus = pch->getRussianPretitle( );

        pch->printf( "Твой претитул: %s\r\nРусский претитул: %s\r\n",
                     (eng.empty( ) ? "(нет)" : eng.c_str( )),
                     (rus.empty( ) ? "(нет)" : rus.c_str( )) );
        return;
    }
    
    if (arg == "clear" || arg == "очистить") {
        pch->setPretitle( DLString::emptyString );
        pch->setRussianPretitle( DLString::emptyString );
        pch->pecho("Русский и английский претитулы очищены.");
        return;
    }
    
    rus = arg.getOneArgument( );

    if (rus == "rus" || rus == "рус") {
        if (!fix_pretitle( pch, arg ))
            return;

        pch->setRussianPretitle( arg );
        pch->printf( "Русский претитул: %s\r\n", arg.c_str( ) );
    }
    else { 
        arg = argument;

        if (!fix_pretitle( pch, arg ))
            return;

        pch->setPretitle( arg );
        pch->printf( "Твой претитул: %s\r\n", arg.c_str( ) );
    }
}

static void sortCommandsFor(vector<Skill::Pointer> &skills, Character *looker)
{
    sort(skills.begin(), skills.end(), [looker](const Skill::Pointer& left, const Skill::Pointer& right)
        {
            return left->getCommand()->getNameFor(looker)
                .compareRussian(right->getCommand()->getNameFor(looker)) < 0;
        });
}

static void sortSkillsFor(vector<Skill::Pointer> &skills, Character *looker)
{
    sort(skills.begin(), skills.end(), [looker](const Skill::Pointer& left, const Skill::Pointer& right)
        {
            return left->getNameFor(looker)
                .compareRussian(right->getNameFor(looker)) < 0;
        });
}

/**
 * Don't output specific skills in report function
 */
static bool skill_is_invalid(int sn, bool noCarry)
{
    if(noCarry){
        if(sn == gsn_lash
         || sn == gsn_bash
         || sn == gsn_hand_to_hand
         || sn == gsn_sword
         || sn == gsn_polearm
         || sn == gsn_dagger
         || sn == gsn_whip
         || sn == gsn_grip
         || sn == gsn_axe
         || sn == gsn_mace
         || sn == gsn_shield_block
         || sn == gsn_flail
         || sn == gsn_slice
         || sn == gsn_second_weapon
         || sn == gsn_pick_lock    
         ) return true;
    }
    return false;
}

/**
 * Don't output specific skills in report function
 */
static bool skill_is_invalid_in_fight(int sn)
{
    if(sn == gsn_recall
         || sn == gsn_concentrate
         || sn == gsn_hide
         || sn == gsn_sneak
         || sn == gsn_detect_hide
         || sn == gsn_pick_lock
         || sn == gsn_bash_door
         ) return true;
    
    return false;
}

CMDRUNP(report)
{
    DLString args = argument;
    DLString arg = args.getOneArgument();

    if (!ch->is_npc() || !IS_CHARMED(ch)) {
        char buf[MAX_INPUT_LENGTH];
        sprintf(buf,
                "У меня %d/%d жизни (hp) %d/%d энергии (mana) %d/%d движения (mv).",
                ch->hit.getValue(), ch->max_hit.getValue(),
                ch->mana.getValue(), ch->max_mana.getValue(),
                ch->move.getValue(), ch->max_move.getValue());
        do_say(ch, buf);
        return;
    }

    NPCharacter *pet = ch->getNPC();
    if (!pet->master || pet->master->is_npc())
        return;

    vector<Skill::Pointer> skills, skillsFight, spells, passives;
    ostringstream result;
    bool showAll = arg_oneof(arg, "all", "все", "full", "полный");
    bool shown = false;
    bool noCarry = pet->canCarryNumber( ) == 0;

    for (int sn = 0; sn < SkillManager::getThis()->size(); sn++) {
        Skill::Pointer skill = SkillManager::getThis()->find(sn);
        Spell::Pointer spell = skill->getSpell();
        Command::Pointer cmd = skill->getCommand().getDynamicPointer<Command>();
        bool passive = skill->isPassive();

        if (!skill->usable(pet, false))
            continue;

        if (cmd && !cmd->getExtra().isSet(CMD_NO_INTERPRET)) {
            bool canOrder = cmd->properOrder(pet) == RC_ORDER_OK;
            pet->fighting = ch;
            bool canOrderFight = cmd->properOrder(pet) == RC_ORDER_OK;
            pet->fighting = 0;

            if(sn == gsn_second_weapon && !skill_is_invalid(sn, noCarry)){
                passives.push_back(skill);
                continue;
            }

            if(skill_is_invalid(sn, noCarry))
                continue;
            
            if (canOrder && showAll) {
                skills.push_back(skill);
            }

            else if (canOrderFight && !skill_is_invalid_in_fight(sn)) {
                skillsFight.push_back(skill);
            }

            continue;
        }

        if (spell && spell->isCasted()) {
            if (spell->properOrder(pet)) {
                if (showAll) {
                    spells.push_back(skill);
                    continue;
                } else if (IS_SET(spell->getTarget(), TAR_CHAR_ROOM)) {
                    spells.push_back(skill);
                    continue;
                }
            }
            continue;
        }

        if (showAll && passive && !skill_is_invalid(sn, noCarry) )
            passives.push_back(skill);
    }

    result << fmt(0, "%1$^C1 говорит тебе \'{G%N2, я %d уровня, у меня %d/%d жизни и %d/%d маны.{x\'",
               pet,
               GET_SEX(pet->master, "Хозяин", "Хозяин", "Хозяйка"),
               pet->getModifyLevel(),
               pet->hit.getValue(), pet->max_hit.getValue(),
               pet->mana.getValue(), pet->max_mana.getValue())
            << endl;

    if (!skills.empty()) {
        ostringstream buf;
        sortCommandsFor(skills, pet->master);
        for (auto it = skills.begin(); it != skills.end();) {
            buf << "{G"
                << "{hh" << (*it)->getSkillHelp()->getID()
                << (*it)->getCommand()->getNameFor(pet->master)
                << "{x";
            if (++it != skills.end())
                buf << ", ";
            else
                buf << "{x" << endl;
        }

        result << "Небоевые умения: " << buf.str();
        shown = true;
    }

    if (!skillsFight.empty()) {
        ostringstream buf;
        sortCommandsFor(skillsFight, pet->master);
        for (auto it = skillsFight.begin(); it != skillsFight.end();) {
            buf << "{Y"
                << "{hh" << (*it)->getSkillHelp()->getID()
                << (*it)->getCommand()->getNameFor(pet->master)
                << "{x";
            if (++it != skillsFight.end())
                buf << ", ";
            else
                buf << "{x" << endl;
        }

        result << "В бою мне можно приказать: " << buf.str();
        shown = true;
    }

    if (!spells.empty()) {
        ostringstream buf;
        sortSkillsFor(spells, pet->master);
        for (auto it = spells.begin(); it != spells.end();) {
            buf << "{g"
                << "{hh" << (*it)->getSkillHelp()->getID()
                << (*it)->getNameFor(pet->master)
                << "{x";
            if (++it != spells.end())
                buf << ", ";
            else
                buf << "{x" << endl;
        }

        if (showAll)
            result << "Я владею такими заклинаниями: " ;
        else 
            result << "Я могу наложить на тебя такие заклинания: ";
        result << buf.str();
        shown = true;
    }

    if (showAll && !passives.empty()) {
        ostringstream buf;
        sortSkillsFor(passives, pet->master);
        for (auto it = passives.begin(); it != passives.end();) {
            buf << "{W"
                << "{hh" << (*it)->getSkillHelp()->getID()
                << (*it)->getNameFor(pet->master)
                << "{x";
            if (++it != passives.end())
                buf << ", ";
            else
                buf << "{x" << endl;
        }

        result << "Мои пассивные умения: " << buf.str();
        shown = true;
    }

    if (IS_SET(pet->act, ACT_RIDEABLE))
        result << "На мне можно {y{hh1376ездить верхом{x! ";
    if (can_fly(pet))
        result << "Я умею {hh1018летать{x. ";
    if (!pet->master->getPC()->pet || pet->master->getPC()->pet != pet)
        result << "Мне можно {hh1024дать{x вещи и приказать их {hh990надеть{x. ";
    result << "Мне можно приказать {hh1020спать{x и другие стандартные команды." << endl;

    if (shown) {

        if (!showAll) {
            DLString petName = Syntax::noun(pet->getNameP('1'));
            result << fmt(0, "Напиши {y{hc{lRприказать %1$s рапорт все{lEorder %1$s report all{x, и я расскажу, что ещё я умею делать.", 
                             petName.c_str())
                   << endl;
        }

        result << endl << "См. также {y{hh1091{lR? приказать{lE? order{x и {y{hh1005{lR? рапорт{lE? report{x" 
               << endl;

        page_to_char(result.str().c_str(), pet->master);

    }
    else {
        page_to_char(result.str().c_str(), pet->master);
        tell_raw(pet->master, pet, "%s, больше я ничегошеньки не умею!", GET_SEX(pet->master, "Хозяин", "Хозяин", "Хозяйка"));
        interpret_raw(pet, "abat", "");
    }
}

/*
 * 'Wimpy' originally by Dionysos.
 */
CMDRUNP( wimpy )
{
    char buf[MAX_STRING_LENGTH];
    char arg[MAX_INPUT_LENGTH];
    int wimpy;

    one_argument( argument, arg );

    if ((ch->getProfession( ) == prof_samurai) && (ch->getRealLevel( ) >=10))
        {
         sprintf(buf,"Стыдись!!! Это будет слишком большим позором для Самурая.\n\r");
         ch->send_to(buf);
         if (ch->wimpy != 0) ch->wimpy = 0;
         return;
        }

    if ( arg[0] == '\0' )
        wimpy = ch->max_hit / 5;
    else  wimpy = atoi( arg );

    if ( wimpy < 0 )
    {
        ch->pecho("Твоя отвага превосходит твою мудрость.");
        return;
    }

    if ( wimpy > ch->max_hit/2 )
    {
        ch->pecho("Это будет слишком большим позором для тебя.");
        return;
    }

    ch->wimpy        = wimpy;

    sprintf( buf, "Ты попытаешься убежать при %d жизни (hit points).\n\r", wimpy );
    ch->send_to( buf);
    return;
}



CMDRUNP( password )
{
    char arg1[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    char *pArg;
    char *pwdnew;
    char *p;
    char cEnd;

    if ( ch->is_npc() )
        return;

    /*
     * Can't use one_argument here because it smashes case.
     * So we just steal all its code.  Bleagh.
     */
    /* TODO: rework with DLString */
    pArg = arg1;
    while ( dl_isspace(*argument) )
        argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
        cEnd = *argument++;

    while ( *argument != '\0' )
    {
        if ( *argument == cEnd )
        {
            argument++;
            break;
        }
        *pArg++ = *argument++;
    }
    *pArg = '\0';

    pArg = arg2;
    while ( dl_isspace(*argument) )
        argument++;

    cEnd = ' ';
    if ( *argument == '\'' || *argument == '"' )
        cEnd = *argument++;

    while ( *argument != '\0' )
    {
        if ( *argument == cEnd )
        {
            argument++;
            break;
        }
        *pArg++ = *argument++;
    }
    *pArg = '\0';
    
    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        ch->pecho("Синтаксис: {lRпароль{lEpassword{lx <старый> <новый>.");
        return;
    }
    
    if (!password_check( ch->getPC( ), arg1 ))
    {
        ch->setWait(40 );
        ch->pecho("Неверный пароль. Подождите 10 секунд.");
        return;
    }

    if ( strlen(arg2) < 5 )
    {
        ch->pecho("Новый пароль должен содержать более пяти символов.");
        return;
    }

    /*
     * No tilde allowed because of player file format.
     * TODO: obsolete restriction, remove.
     */
    pwdnew = arg2;
    for ( p = pwdnew; *p != '\0'; p++ )
    {
        if ( *p == '~' )
        {
            ch->pecho("Новый пароль неприемлем, попробуй еще раз.");
            return;
        }
    }
   
    password_set( ch->getPC( ), pwdnew );
    ch->getPC( )->save( );
    ch->pecho("Ok.");
    return;
}

CMDRUNP( request )
{
        char arg1 [MAX_INPUT_LENGTH];
        char arg2 [MAX_INPUT_LENGTH];
        Character *victim;
        Object  *obj;

        if ( ch->isAffected(gsn_gratitude))
        {
                ch->pecho("Подожди немного.");
                return;
        }

        argument = one_argument( argument, arg1 );
        argument = one_argument( argument, arg2 );

        if (ch->is_npc())
                return;

        if ( arg1[0] == '\0' || arg2[0] == '\0' )
        {
                ch->pecho("Что и у кого ты хочешь попросить?");
                return;
        }

        if ( ( victim = get_char_room( ch, arg2 ) ) == 0 )
        {
                ch->pecho("Здесь таких нет.");
                return;
        }

        if (!victim->is_npc())
        {
                ch->pecho("На игроков такие штучки не пройдут. Просто поговори с ними!");
                return;
        }

          if ( victim->position <= POS_SLEEPING )
        {
                oldact("$C1 не в состоянии выполнить твою просьбу.", ch, 0, victim, TO_CHAR);
                return;
        }

        if (victim->getNPC()->behavior 
            && IS_SET(victim->getNPC()->behavior->getOccupation( ), (1 << OCC_SHOPPER)))
        {
                ch->pecho("Хочешь -- купи!");
                return;
        }

        if (ch->move < (50 + ch->getRealLevel( )))
        {
                do_say(victim, "Ты выглядишь устало, может, отдохнешь сначала?");
                return;
        }

        Flags att = victim->getRace( )->getAttitude( *ch->getRace( ) );

    /* Donating races (e.g. centaurs) donate regardless of alignment. 
     * Otherwise good mobs would donate to good players.
     */     
        if (!att.isSet( RACE_DONATES ))
        {
            if (!IS_GOOD(ch) || !IS_GOOD(victim))
            {
                if (IS_EVIL(ch) && !IS_EVIL(victim))
                {
                    do_say(victim, "У тебя нечистая душа, я ничего тебе не дам!");
                } else
                {
                    do_say(victim, "Я не дам тебе ничего!!");
                }
                if (ch->getModifyLevel( ) > 30 && number_percent() > 75) 
                {
                    interpret_raw( victim, "murder", ch->getNameP( ));
                }
                return;
            }
        }

        ch->setWaitViolence( 1 );
        ch->move -= 10;
        ch->move = max((int)ch->move, 0);

        if (victim->getModifyLevel( ) >= ch->getModifyLevel( ) + 10 || victim->getModifyLevel( ) >= ch->getModifyLevel( ) * 2)
        {
                say_fmt("Всему свое время, малыш%2$G||ка.", victim, ch);
                return;
        }

        if ( ( ( obj = get_obj_carry(victim , arg1 ) ) == 0
                        && (obj = get_obj_wear(victim, arg1)) == 0)
                || IS_SET(obj->extra_flags, ITEM_INVENTORY))
        {
                do_say(victim, "Извини, у меня нет этого.");
                return;
        }

    if ( !can_drop_obj( ch, obj )
    || ( obj_is_worn(obj) && IS_OBJ_STAT(obj, ITEM_NOREMOVE)  ))
    {
      do_say(victim,
        "Извини, но эта вещь проклята, и я не могу избавиться от нее.");
      return;
    }

        if ( ch->carry_number + obj->getNumber( ) > ch->canCarryNumber( ) )
        {
                ch->pecho("Твои руки полны.");
                return;
        }

        if ( ch->carry_weight + obj->getWeight( ) > ch->canCarryWeight( ) )
        {
                ch->pecho("Ты не можешь нести такой вес.");
                return;
        }

        if ( !ch->can_see( obj ) )
        {
                ch->pecho("Ты не видишь этого.");
                return;
        }
          if ( !victim->can_see( ch ) )
         {
                  do_say(victim,
                 "Извини, я не вижу тебя.");
                 return;
        }

        if ( !victim->can_see( obj ) )
            {
                do_say(victim,
                "Извини, я не вижу этой вещи.");
            return;
            }



        if ( obj->pIndexData->vnum == 520 ) // Knight's key
        {
                ch->pecho("Извини, он не отдаст тебе это.");
                return;
        }

  if ( is_safe(ch, victim) )
    {
      return;
    }

    if ( obj->pIndexData->limit >= 0 && obj->isAntiAligned( ch ) )
    {
        ch->pecho("%2$^s не позволяют тебе завладеть %1$O5.",
                          obj,
                          IS_NEUTRAL(ch) ? "силы равновесия" : IS_GOOD(ch) ? "священные силы" : "твои демоны");
      return;
    }


        obj_from_char( obj );
        obj_to_char( obj, ch );
        oldact("$c1 просит $o4 у $C2.", ch, obj, victim, TO_NOTVICT);
        oldact("Ты просишь $o4 у $C2.",   ch, obj, victim, TO_CHAR);
        oldact("$c1 просит $o4 у тебя.", ch, obj, victim, TO_VICT);
        
        omprog_give( obj, victim, ch );

        ch->move -= ( 50 + ch->getModifyLevel() );
        ch->move = max( (int)ch->move, 0 );
        ch->hit -= 3 * ( ch->getModifyLevel() / 2 );
        ch->hit = max( (int)ch->hit, 0 );

        oldact("Ты чувствуешь благодарность за доверие $C2.", ch, 0, victim,TO_CHAR);
        postaffect_to_char(ch, gsn_gratitude, ch->getModifyLevel() / 10);
}



CMDRUNP( demand )
{
  char arg1 [MAX_INPUT_LENGTH];
  char arg2 [MAX_INPUT_LENGTH];
  Character *victim;
  Object  *obj;
  int chance;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if (ch->is_npc())
        return;

  if (ch->getProfession( ) != prof_anti_paladin)
    {
        ch->pecho( "Ты никого не запугаешь своим видом." );
      return;
    }

  if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        ch->pecho( "Потребовать что и у кого?" );
      return;
    }

  if ( ( victim = get_char_room( ch, arg2 ) ) == 0 )
    {
        ch->pecho( "Таких тут нет." );
      return;
    }

    if (!victim->is_npc( )) {
        ch->pecho( "Просто убей и отбери." );
        return;
    }

    if (IS_SET(victim->act, ACT_NODEMAND)) {
        oldact("$C1 не подчинится твоему требованию.", ch, 0, victim, TO_CHAR);
        return;
    }

  if ( victim->position <= POS_SLEEPING )
    {
       oldact("$C1 не в состоянии исполнить твой приказ.", ch, 0, victim, TO_CHAR);
      return;
    }
  
    if (victim->getNPC()->behavior 
        && IS_SET(victim->getNPC()->behavior->getOccupation( ), (1 << OCC_SHOPPER))) 
    {
        ch->pecho("Хочешь -- купи!");
        return;
    }
  
  ch->setWaitViolence( 1 );

  chance = IS_EVIL(victim) ? 10 : IS_GOOD(victim) ? -5 : 0;
  chance += (ch->getCurrStat(STAT_CHA) - 15) * 10;
  chance += ch->getModifyLevel() - victim->getModifyLevel();

  if( victim->getModifyLevel() >= ch->getModifyLevel() + 10 || victim->getModifyLevel() >= ch->getModifyLevel() * 2)
        chance = 0;

    if (number_percent() > chance) {
         do_say(victim, "Я не собираюсь ничего отдавать тебе!");
         interpret_raw( victim, "murder", ch->getNameP( ));
         return;
    }

  if (( ( obj = get_obj_carry(victim , arg1 ) ) == 0
      && (obj = get_obj_wear(victim, arg1)) == 0)
      || IS_SET(obj->extra_flags, ITEM_INVENTORY))
    {
      do_say(victim, "Извини, у меня нет этого.");
      return;
    }

    if ( !can_drop_obj( ch, obj )
    || ( obj_is_worn(obj) && IS_OBJ_STAT(obj, ITEM_NOREMOVE)  ))
    {
      do_say(victim,
        "Эта вещь проклята, и я не могу избавиться от нее.");
      return;
    }


  if ( ch->carry_number + obj->getNumber( ) > ch->canCarryNumber( ) )
    {
      ch->pecho("Твои руки полны.");
      return;
    }

  if ( ch->carry_weight + obj->getWeight( ) > ch->canCarryWeight( ) )
    {
      ch->pecho("Ты не сможешь нести такую тяжесть.");
      return;
    }

  if ( !ch->can_see( obj ) )
    {
      oldact("Ты не видишь этого.", ch, 0, victim, TO_CHAR);
      return;
    }

  if ( !victim->can_see( ch ) )
    {
        do_say(victim,
        "Извини, я не вижу тебя.");
      return;
    }

  if ( !victim->can_see( obj ) )
    {
        do_say(victim,
        "Извини, я не вижу этой вещи.");
      return;
    }

  if ( is_safe(ch, victim) )
    {
      return;
    }

      if ( obj->pIndexData->limit >= 0 && obj->isAntiAligned( ch ) )
    {
        ch->pecho("%2$^s не позволяют тебе завладеть %1$O5.",
                          obj,
                          IS_NEUTRAL(ch) ? "силы равновесия" : IS_GOOD(ch) ? "священные силы" : "твои демоны");
      return;
    }

    oldact("$c1 требует $o4 у $C2.", ch, obj, victim, TO_NOTVICT);
    oldact("Ты требуешь $o4 у $C2.",   ch, obj, victim, TO_CHAR);
    oldact("$c1 требует у тебя $o4.", ch, obj, victim, TO_VICT);

    obj_from_char( obj );
    obj_to_char( obj, ch );

    omprog_give( obj, victim, ch );

    ch->pecho("Твое могущество повергает всех в трепет.");
}

/**
 * Output each score piece separately, as requested by the argument.
 */
static void do_score_args(Character *ch, const DLString &arg)
{
    PCharacter *pch = ch->getPC();
    if (!pch)
        return;

    int stat = -1;
    for (int i = 0; i < stat_table.size; i++)
        if (arg.strPrefix( stat_table.fields[i].name )
            || arg.strPrefix( russian_case(stat_table.fields[i].message, '1') )
            || arg.strPrefix( russian_case(stat_table.fields[i].message, '4') ))
        {
            stat = i;
            break;
        }
    
    if (arg == "ум")
        stat = STAT_INT;

    if (stat >=0) {
        ch->pecho("%^N1 %d (%d), максимум %d.", 
            stat_table.fields[stat].message, ch->perm_stat[stat], 
            ch->getCurrStat(stat), pch->getMaxStat(stat));
        return;
    }
   
    if (arg_oneof(arg, "hitpoint", "здоровье", "hp")) {
        ch->pecho("Здоровье %d из %d.", ch->hit, ch->max_hit);
        return;
    } 
    if (arg_oneof(arg, "mana", "мана", "энергия")) {
        ch->pecho("Мана %d из %d.", ch->mana, ch->max_mana);
        return;
    } 
    if (arg_oneof(arg, "moves", "движение", "шаги")) {
        ch->pecho("Шагов %d из %d.", ch->move, ch->max_move);
        return;
    } 
    if (arg_oneof(arg, "level", "уровень")) {
        ch->pecho("Уровень %d.", ch->getRealLevel());
        return;
    } 
    if (arg_oneof(arg, "race", "раса")) {
        if (ch->getRace()->isPC()) {
            PCRace::Pointer pcRace = ch->getRace()->getPC(); 
            ch->pecho("Ты %N1.", GET_SEX(ch,
                            pcRace->getMaleName().c_str(),
                            pcRace->getMaleName().c_str(),
                            pcRace->getFemaleName().c_str()));
        }
        return;
    } 
    if (arg_oneof(arg, "sex", "gender", "пол")) {   
        ch->pecho("Пол %s.", GET_SEX(ch, "потерян", "мужской", "женский"));
        return;
    }
    if (arg_oneof(arg, "class", "класс", "profession", "профессия")) {
        ch->pecho("Ты %N1.", ch->getProfession()->getRusName().c_str());
        return;
    } 
    if (arg_oneof(arg, "alignment", "натура")) {
        ch->pecho("У тебя %s натура.", align_name_short(ch, Grammar::MultiGender::FEMININE));
        return;
    } 
    if (arg_oneof(arg, "ethos", "этос")) {
        ch->pecho("У тебя %s этос.", ethos_table.message(ch->ethos, '1').c_str());
        return;
    } 
    if (arg_oneof(arg, "hometown", "дом")) {
        Room *room = get_room_instance(pch->getHometown()->getAltar());
        ch->pecho("Твой дом - %s.", room ? room->areaName() : "потерян");
        return;
    } 
    if (arg_oneof(arg, "religion", "религия")) {
        if (ch->getReligion() == god_none)
            ch->pecho("Ты атеист%1$G||ка.", ch);
        else
            ch->pecho("Религия %s.", ch->getReligion()->getRussianName().ruscase('1').c_str());
        return;
    } 
    if (arg_oneof(arg, "practice", "практики")) {
        ch->pecho("Практик %d.", pch->practice);
        return;
    } 
    if (arg_oneof(arg, "train", "тренировки")) {
        ch->pecho("Тренировки %d.", pch->train);
        return;
    } 
    if (!str_prefix("quest", arg.c_str()) || !str_prefix("квест", arg.c_str())) {
        ch->pecho("Используй команды 'квест время' и 'квест очки'.");
        return;
    } 
    if (arg_oneof(arg, "wimpy", "трусость")) {
        ch->pecho("Трусость %d.", ch->wimpy);
        return;
    } 
    if (arg_oneof(arg, "death", "смертей", "смерть")) {
        ch->pecho("Смертей %d.", pch->death);
        return;
    } 
    if (arg_oneof(arg, "position", "положение", "позиция")) {
        ch->pecho(msgtable_lookup(msg_positions, ch->position));
        return;
    }
    if (arg_oneof(arg, "gold", "золото")) {
        ch->pecho("Золота %d.", ch->gold);
        return;
    } 
    if (arg_oneof(arg, "silver", "серебро")) {
        ch->pecho("Серебра %d.", ch->silver);
        return;
    } 
    if (arg_oneof(arg, "weight", "вес")) {
        ch->pecho("Вес %d из %d.", ch->getCarryWeight()/10, ch->canCarryWeight()/10);
        return;
    } 
    if (arg_oneof(arg, "items", "вещи")) {
        ch->pecho("Вещи %d из %d.", ch->carry_number, ch->canCarryNumber());
        return;
    } 
    if (arg_oneof(arg, "experience", "опыт")) {
        ch->pecho("Опыта до уровня %d.", pch->getExpToLevel());
        return;
    }
    if (arg_oneof(arg, "age", "возраст")) {
        ch->pecho("Возраст %d.", pch->age.getYears());
        return;
    }

    if (arg_oneof(arg, "hitroll", "точность")) {
        ch->pecho("Точность %d.", ch->hitroll);
        return;
    } 
    if (arg_oneof(arg, "damroll", "урон")) {
        ch->pecho("Урон %d.", ch->damroll);
        return;
    } 
    if (arg_oneof(arg, "armor class", "класс брони", "защита")) {
        ch->pecho("Защита от уколов %d, ударов %d, разрезов %d, экзотики %d.", 
                    GET_AC(ch, AC_PIERCE), GET_AC(ch, AC_BASH),
                    GET_AC(ch, AC_SLASH), GET_AC(ch, AC_EXOTIC));
        return;
    }
    if (arg_oneof(arg, "saves", "спассбросок", "защита от заклинаний")) {
        ch->pecho("Защита от заклинаний %d.", ch->saving_throw);
        return;
    }
   
    ch->pecho("Такого параметра не существует или он скрыт от тебя, попробуй что-то еще."); 
}


#define MILD(ch)     (IS_SET((ch)->comm, COMM_MILDCOLOR))

CMDRUNP( score )
{
    int ekle=0;
    PCharacter *pch = ch->getPC( );
    
    const char *CLR_FRAME = MILD(ch) ? "{Y" : "{G";
    const char *CLR_BAR   = MILD(ch) ? "{D" : "{C";
    const char *CLR_CAPT  = MILD(ch) ? "{g" : "{R";
    
    if (ch->is_npc( )) {
        interpret_raw( ch, "oscore" );
        return;
    }
    
    XMLAttributeTimer::Pointer qd = pch->getAttributes( ).findAttr<XMLAttributeTimer>( "questdata" );
    int age = pch->age.getYears( );
    Room *room = get_room_instance( pch->getHometown( )->getAltar( ) );
    DLString profName = ch->getProfession( )->getNameFor( ch );

    ostringstream name;
    DLString title = pch->getParsedTitle( );
    name << ch->seeName( ch, '1' ) << "{x ";
    mudtags_convert(title.c_str( ), name, TAGS_CONVERT_VIS, ch);

    // Output one piece of the score if there is an argument provided.
    DLString arg = argument;
    if (!arg.empty()) {
        do_score_args(ch, arg);
        return;
    }

    ch->printf( 
"%s\n\r"
"      /~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~/~~\\\n\r", 
             CLR_FRAME);
    ch->pecho(
        fmt ( 0, "     %s|   %s%-50.50s {y%3d{x %4s   %s|____|",
                CLR_FRAME,
                CLR_CAPT,
                name.str( ).c_str( ),
                age,
                GET_COUNT(age, "год", "года", "лет"),
                CLR_FRAME ) );
                
        
    ch->printf(
"     %s|%s+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+%s|\n\r" 
"     | %sУровень:{x  %3d        %s|%s {lRСила:{lE Str:{lx{x %2d{c({x%2d{c){x {C%2d{x %s| %sРелигия:{x %-14.14s%s|\n\r"
"     | %sРаса :{x  %-12s %s| %s{lRУм  :{lE Int:{lx{x %2d{c({x%2d{c){x {C%2d{x %s| %sПрактик   :{x   %3d      %s|\n\r"
"     | %sПол  :{x  %-11s  %s| %s{lRМудр:{lE Wis:{lx{x %2d{c({x%2d{c){x {C%2d{x %s| %sТренировок:{x   %3d      %s|\n\r"
"     | %sКласс:{x  %-13s%s| %s{lRЛовк:{lE Dex:{lx{x %2d{c({x%2d{c){x {C%2d{x %s| %sКвест. единиц:{x  %-5d%s  |\n\r"
"     | %sНатура:{x %-11s  %s| %s{lRСлож:{lE Con:{lx{x %2d{c({x%2d{c){x {C%2d{x %s| %sКвест. время:{x   %-3d %s   |\n\r"
"     | %sЭтос :{x  %-12s %s| %s{lRОбая:{lE Cha:{lx{x %2d{c({x%2d{c){x {C%2d{x %s| %s%s :{x   %3d      %s|\n\r"
"     | %sДом  :{x  %-30s %s| {Y%-22s %s|\n\r"                
"     |%s+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+%s|\n\r",

            CLR_FRAME, CLR_BAR, CLR_FRAME,

            CLR_CAPT,
            ch->getRealLevel( ),
            CLR_BAR,
            CLR_CAPT,
            ch->perm_stat[STAT_STR], ch->getCurrStat(STAT_STR), pch->getMaxStat(STAT_STR),
            CLR_BAR,
            CLR_CAPT,
            ch->getReligion( )->getNameFor( ch ).ruscase( '1' ).c_str( ),
            CLR_FRAME,

            CLR_CAPT,
            pch->getRace( )->getPC( )->getScoreNameFor( ch, ch ).c_str( ), 
            CLR_BAR,
            CLR_CAPT,
            ch->perm_stat[STAT_INT], ch->getCurrStat(STAT_INT), pch->getMaxStat(STAT_INT),
            CLR_BAR, 
            CLR_CAPT,
            pch->practice.getValue( ),
            CLR_FRAME,

            CLR_CAPT,
            ch->getSex( ) == 0 ? "потерян" : ch->getSex( ) == SEX_MALE ? "мужской" : "женский",
            CLR_BAR,
            CLR_CAPT,
            ch->perm_stat[STAT_WIS], ch->getCurrStat(STAT_WIS), pch->getMaxStat(STAT_WIS),
            CLR_BAR,
            CLR_CAPT,
            pch->train.getValue( ),
            CLR_FRAME,

            CLR_CAPT,
            profName.c_str( ),
            CLR_BAR,
            CLR_CAPT,
            ch->perm_stat[STAT_DEX], ch->getCurrStat(STAT_DEX), pch->getMaxStat(STAT_DEX),
            CLR_BAR,
            CLR_CAPT,
            pch->getQuestPoints(),
            CLR_FRAME,

            CLR_CAPT,
            align_name_short(ch, Grammar::MultiGender::FEMININE),
            CLR_BAR,
            CLR_CAPT,
            ch->perm_stat[STAT_CON], ch->getCurrStat(STAT_CON), pch->getMaxStat(STAT_CON),
            CLR_BAR,
            CLR_CAPT,
            qd ? qd->getTime( ) : 0,
            CLR_FRAME,

            CLR_CAPT,
            ethos_table.message( ch->ethos, '1' ).cutSize( 12 ).c_str( ),
            CLR_BAR,
            CLR_CAPT,
            ch->perm_stat[STAT_CHA], ch->getCurrStat(STAT_CHA), pch->getMaxStat(STAT_CHA),
            CLR_BAR,
            CLR_CAPT,
            ch->getProfession( ) == prof_samurai 
                ?  "{lRСмертей  {lEDeath    {lx" : "{lRТрусость {lEWimpy    {lx" ,
            ch->getProfession( ) == prof_samurai 
                ? pch->death.getValue( ) : ch->wimpy.getValue( ),
            CLR_FRAME,

            CLR_CAPT,
            room ? room->areaName() : "Потерян",
            CLR_BAR,
            msgtable_lookup( msg_positions, ch->position ),
            CLR_FRAME,

            CLR_BAR, CLR_FRAME);

    if (pch->guarding != 0) {
        ekle = 1;
        ch->printf( 
"     %s| {wТы охраняешь    :{Y %-10s                                    %s|\n\r",
            CLR_FRAME,
            ch->seeName( pch->guarding, '4' ).c_str(),
            CLR_FRAME);
    }

    if (pch->guarded_by != 0) {
        ekle = 1;
        ch->printf( 
"     %s| {wТебя охраняет     :{Y %-10s                                  %s|\n\r",
        CLR_FRAME,
        ch->seeName( pch->guarded_by, '1' ).c_str(),
        CLR_FRAME);
    }

    // Report only active desires in 'score'.
    for (int i = 0; i < desireManager->size( ); i++) {
        Desire *desire = desireManager->find(i);
        if (desire->isActive(ch->getPC())) {
            ostringstream buf;
        
            desire->report(ch->getPC(), buf);

            if (!buf.str( ).empty( )) {
                ekle = 1;
                ch->printf( "     %s| {w%-64s%s|\r\n", 
                            CLR_FRAME,
                            buf.str( ).c_str( ),
                            CLR_FRAME );
            }
        }
    }

    if (ch->is_adrenalined()) {
        ekle = 1;
        ch->printf( 
"     %s| {yАдреналин кипит в твоих венах!                                  %s|\n\r",
                 CLR_FRAME,
                 CLR_FRAME );
    }

    if (IS_GHOST(ch)) {
        ekle = 1;
        ch->pecho( 
"     %1$s| {xТы призрак и обретёшь плоть через {Y%2$3d {xсекунд%2$-1Iу|ы|.                  %1$s|",
                 CLR_FRAME,
                 pch->ghost_time*(PULSE_MOBILE/dreamland->getPulsePerSecond()),
                 CLR_FRAME );
    }

    if (ch->is_immortal()) {
        ekle = 1;
        ch->printf( 
"     %s| {w{lRНевидимость:{lEInvisible:  {lx {lRуровня{lElevel{lx %3d   "
         "{lRИнкогнито{lEIncognito  {lx: {lRуровня{lElevel{lx %3d                 %s|\n\r",
              CLR_FRAME,
              pch->invis_level.getValue( ),
              pch->incog_level.getValue( ),
              CLR_FRAME);
    }

    list<DLString> attrLines;
    if (ch->getPC()->getAttributes( ).handleEvent( ScoreArguments( ch->getPC(), attrLines ) )) {
        ekle = 1;
        for (list<DLString>::iterator l = attrLines.begin( ); l != attrLines.end( ); l++) {
            ch->printf("     %s| {w%-64s%s|\r\n", 
                        CLR_FRAME,
                        l->c_str(),
                        CLR_FRAME);
        }
    }

    if (ekle) {
        ch->printf( 
"     %s|%s+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+%s|\n\r",
                CLR_FRAME,
                CLR_BAR,
                CLR_FRAME);
    }


    ch->printf( 
"     %s| %sВещи          :{x     %3d/%-4d        %sЗащита от уколов:{x   %-5d   %s|\n\r"
"     | %sВес           :{x  %6d/%-8d    %sЗащита от ударов:{x   %-5d   %s|\n\r"
"     | %sЗолото        :{Y %-10d          %sЗащита от разрезов:{x %-5d   %s|\n\r"
"     | %sСеребро       :{W %-10d          %sЗащита от экзотики:{x %-5d   %s|\n\r"
"     | %sЕдиниц опыта  :{x %-6d              %s{lRЗащита от заклинаний{lESaves vs Spell      {lx:{x %4d  %s|\n\r",
        CLR_FRAME,
        CLR_CAPT,
        ch->carry_number, ch->canCarryNumber( ),
        CLR_CAPT,
        GET_AC(ch,AC_PIERCE),
        CLR_FRAME,

        CLR_CAPT,
        ch->getCarryWeight( )/10, ch->canCarryWeight( )/10,
        CLR_CAPT,
        GET_AC(ch,AC_BASH),
        CLR_FRAME,

        CLR_CAPT,
        ch->gold.getValue( ),
        CLR_CAPT,
        GET_AC(ch,AC_SLASH),
        CLR_FRAME,

        CLR_CAPT,
        ch->silver.getValue( ),
        CLR_CAPT,
        GET_AC(ch,AC_EXOTIC),
        CLR_FRAME,

        CLR_CAPT,
        ch->exp.getValue( ),
        CLR_CAPT,
        ch->saving_throw.getValue( ),
        CLR_FRAME);

    ch->printf( 
"     %s| %sОпыта до уровня:{x %-6d                                         %s|\n\r"
"     |                                    %sЖизни:{x %5d / %5d         %s|\n\r",
        CLR_FRAME,
        CLR_CAPT,
        pch->getExpToLevel( ),
        CLR_FRAME,

        CLR_CAPT,
        ch->hit.getValue( ), ch->max_hit.getValue( ),
        CLR_FRAME);

    ch->printf( 
"     %s| %s{lRТочность{lEHitroll {lx      :{x   %-3d            %sЭнергии:{x %5d / %5d         %s|\n\r"
"     | %s{lRУрон   {lEDamroll{lx       :{x   %-3d           %sДвижения:{x %5d / %5d         %s|\n\r",
        CLR_FRAME,
        CLR_CAPT,
        ch->hitroll.getValue( ),
        CLR_CAPT,
        ch->mana.getValue( ), ch->max_mana.getValue( ),
        CLR_FRAME,

        CLR_CAPT,
        ch->damroll.getValue( ),
        CLR_CAPT,
        ch->move.getValue( ), ch->max_move.getValue( ),
        CLR_FRAME);


    ch->printf( 
"  %s/~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~/   |\n\r"
"  \\________________________________________________________________\\__/{x\n\r",
        CLR_FRAME);

    if (IS_SET(ch->comm, COMM_SHOW_AFFECTS))
        interpret_raw( ch, "affects", "noempty");
}


CMDRUNP( nohelp )
{
    DLString txt = argument;
    txt.stripWhiteSpace( );
    if (txt.empty( )) {
        ch->pecho("Об отсутствии какого раздела справки ты хочешь сообщить?");
        return;
    }

    bugTracker->reportNohelp( ch, txt );
    ch->pecho("Записано.");
}

CMDRUNP( bug )
{
    DLString txt = argument;
    txt.stripWhiteSpace( );
    if (txt.empty( )) {
        ch->pecho("О какой именно ошибке ты хочешь сообщить?");
        return;
    }

    bugTracker->reportBug( ch, txt );
    ch->pecho( "Ошибка записана.");
}

CMDRUNP( typo )
{
    DLString txt = argument;
    txt.stripWhiteSpace( );
    if (txt.empty( )) {
        ch->pecho("О какой именно опечатке ты хочешь сообщить?");
        return;
    }

    bugTracker->reportTypo( ch, txt );
    ch->pecho( "Опечатка записана.");
}

CMDRUNP( iidea )
{
    DLString txt = argument;
    txt.stripWhiteSpace( );
    if (txt.empty( )) {
        ch->pecho("О какой именно идее ты хочешь сообщить?");
        return;
    }

    bugTracker->reportIdea( ch, txt );
    ch->pecho( "Идея записана.");
}

/*---------------------------------------------------------------------------*
 * Help
 *---------------------------------------------------------------------------*/

/**
 * An attempt to implement fuzzy search, matching input against keywords using
 * Levenshtein algorithm.
 */
struct FuzzySearch {
    FuzzySearch(Character *ch, const char *argument) 
    {
        arg = argument;
        arg.toLower();

        // For short user input, only look for very exact matches (distance 1).
        min_distance = arg.length() > 5 ? 3 : arg.length() > 2 ? 2 : 1;

        candidates.resize(min_distance + 1);

        empty = true;

        // Collect all matching articles.
        for (auto &a : helpManager->getArticles()) {
            if ((*a)->visible(ch))
                searchArticle(a);
        }
    }

    bool hasResults() 
    {
        return !empty;
    }

    void printResults(Character *ch) 
    {
        ostringstream buf;
        int max_output = 5;    

        buf << "Справка не найдена. Возможно, имелось в виду:" << endl;

        // Output matches starting with best distance ones, but no more than max_output.
        for (int i = 1; i <= min_distance && max_output > 0; i++) {
            auto & matches = candidates[i];

            for (auto &pair: matches) {
                buf << "    ";
                if (pair.second->getID() > 0)
                    buf << "{hh" << pair.second->getID();

                buf << pair.first << "{x "
                    << "(" << pair.second->getTitle(DLString::emptyString).colourStrip() << ")" 
                    << endl;

                if ((--max_output) <= 0)
                    break;
            }
        }

        ch->send_to(buf);
    }

private:

    void searchArticle(const HelpArticle::Pointer &a) 
    {
        int d;
        StringSet keywords; // contains main keywords and additional ones.
        keywords.insert(a->getAllKeywords().begin(), a->getAllKeywords().end());
        keywords.insert(a->aka.begin(), a->aka.end());

        // See if any of the article's keywords matches the input.
        for (auto &keyword: keywords) {
            DLString kw = keyword;
            kw.replaces("'", "");
            kw.toLower(); 
       
            // First try to match full keyword (with spaces but without quotes). 
            if ((d = getDistance(kw)) <= min_distance) { 
                candidates[d].push_back(make_pair(kw, a));
                empty = false;
                return;
            }

            // If keyword contains spaces, split it into words and try again.
            if (kw != keyword) {
                DLString word;
                while (!(word = kw.getOneArgument()).empty()) {
                    if ((d = getDistance(word)) <= min_distance) { 
                        candidates[d].push_back(make_pair(word, a));
                        empty = false;
                        return;
                    }
                }
            }
        }
    }

    int getDistance(const DLString &keyword)
    {
        // Return Levenshtein distance between user input and the keyword. 
        // The keyword is cut to match the input size (unless input is too short already),
        // this allows for prefix matches.
        DLString kw = keyword;

        if (arg.length() > 3 && arg.length() < kw.length())
            kw.cutSize(arg.length());

        return levenshtein(arg.c_str(), kw.c_str(), 1, 1, 1, 1);
    }

    DLString arg;

    // Keep a list of matches for each distance. A match (pair) contains the exact keyword and the article.
    vector<
        list<pair<DLString, HelpArticle::Pointer> > > candidates;

    // Cut-off distance.
    int min_distance;

    bool empty;
};

struct HelpFinder {
    typedef vector<HelpArticle::Pointer> ArticleArray;

    HelpFinder(Character *ch, const char *argument) {

        parseArgs(argument);

        // Find help by ID.
        Integer id;
        if (Integer::tryParse(id, args)) {
            HelpArticle::Pointer exact = helpManager->getArticle(id);
            
            if (exact)
                articles.push_back(exact);
            return;
        }

        // Find help by keyword.
        findMatchingArticles(ch);

        // Our smartassery yielded nothing, just search for the whole argument.
        if (articles.empty() && !preferredLabels.empty()) {
            preferredLabels.clear();
            findMatchingArticles(ch);
        }
    }
    
    HelpArticle::Pointer get(int number) const
    {    
        unsigned int n = (unsigned int)number;
        if (n > articles.size() || n < 0)
            return HelpArticle::Pointer();
        return articles.at(number-1);
    }
    
    const ArticleArray &getArticles() const {
        return articles;
    }
    
private:
    void findMatchingArticles(Character *ch) 
    {
        HelpArticles::const_iterator a;

        for (a = helpManager->getArticles( ).begin( ); a != helpManager->getArticles( ).end( ); a++) {
            if (!(*a)->visible( ch ))
                continue;

            if (!articleMatches(*a))
                continue;
            
            articles.push_back(*a); 
        }
    }

    bool articleMatches(const HelpArticle::Pointer &a) const
    {
        // If first keyword was something like "skill", look for remaining keywords within a certain label.
        if (!preferredLabels.empty() && !a->labels.all.containsAny(preferredLabels))
            return false;

        DLString fullKw = a->getAllKeywordsString() + " " + a->aka.toString();
        fullKw = fullKw.substitute('\'', "");
        const char *lookup = preferredLabels.empty() ? args.c_str() : argRest.c_str();

        if (is_name(lookup, fullKw.c_str()))
            return true; 

        for (StringSet::const_iterator k = (*a)->getAllKeywords().begin(); k != (*a)->getAllKeywords().end(); k++)
            if (is_name(lookup, (*k).c_str()))
                return true; 

        for (auto &aka: (*a)->aka) {
            if (is_name(lookup, aka.c_str()))
                return true;
        }
        return false;
    }

    void parseArgs(const char *argument) {
        args = argument;
        argRest = args;
        arg1 = argRest.getOneArgument();

        // Reduce "help skill bash" to just "help bash".
        if (!argRest.empty()) {
            if (arg_oneof(arg1, "умение", "навык", "skill")) {
                preferredLabels.insert("skill");
                preferredLabels.insert("spell");
            }
            else if (arg_oneof(arg1, "заклинание", "spell"))
                preferredLabels.insert("spell");
            else if (arg_oneof(arg1, "класс", "class"))
                preferredLabels.insert("class");
            else if (arg_oneof(arg1, "раса", "race"))
                preferredLabels.insert("race");
            else if (arg_oneof(arg1, "команда", "command"))
                preferredLabels.insert("cmd");
            else if (arg_oneof(arg1, "зона", "area", "zone"))
                preferredLabels.insert("area");
            else if (arg_oneof(arg1, "религия", "religion"))
                preferredLabels.insert("religion");
            else if (arg_oneof(arg1, "клан", "clan"))
                preferredLabels.insert("clan");
        }
    }
    
    ArticleArray articles;
    DLString args, arg1, argRest;
    StringSet preferredLabels;
};

CMDRUNP( help )
{
    std::basic_ostringstream<char> buf;
    DLString origArgument = DLString(argument).stripWhiteSpace().substitute('\'', "");

    if (!ch->getPC())
        return;

    if (origArgument.empty()) {
        strcpy(argument, "summary");
    }

    // Вариант 2.create? - needs exact match.
    if (origArgument.size() > 1 && strchr( argument , '.')) {
        char argall[MAX_INPUT_LENGTH];
        int number = number_argument(argument, argall);

        if (number >= 1) {
            HelpArticle::Pointer help = HelpFinder(ch, argall).get(number);
            if (help) {
                page_to_char( help->getText( ch ).c_str( ), ch );
                return;
            }
            ch->pecho("Нет подсказки по данному слову.");
            bugTracker->reportNohelp( ch, origArgument.c_str( ) );
            return;
        }

        // Restore original argument without the dot, assume it was a typo.
        strcpy(argument, origArgument.substitute('.', ' ').c_str());
    }
    
    // Поиск по строке без чисел.
    HelpFinder::ArticleArray articles = HelpFinder(ch, argument).getArticles();
    // No match, try fuzzy matching.
    if (articles.empty()) {
        if (origArgument.size() > 1) {
            FuzzySearch fs(ch, argument);
            if (fs.hasResults()) {
                fs.printResults(ch);
                return;
            }
        }
        
        ch->pecho("Нет подсказки по данному слову.");
        bugTracker->reportNohelp( ch, origArgument.c_str( ) );
        return;
    }

    // Exact match - bingo.
    if (articles.size() == 1) {
        page_to_char( articles.front()->getText( ch ).c_str( ), ch );
        return;
    }

    // Several matches, display them all with numbers.
    buf << "По запросу '{C" << origArgument << "{x' найдено несколько разделов справки с такими номерами:" << endl << endl;
    DLString lineFormat = "[{C{hh%5d{x] %s\r\n";
    int firstId = -1;
    for (unsigned int a = 0; a < articles.size(); a++) {
        auto help = articles[a];
        DLString title = help->getTitle(DLString::emptyString);
        DLString disambig = help_article_disambig(*help);

        // Create a line with help ID, title and disambiguation keywords (unless turned off).
        DLString line = title;
        if (!disambig.empty()) {
            if (!IS_SET(ch->getPC()->config, CONFIG_SCREENREADER) 
                && !ch->getPC()->getAttributes().isAvailable("newhelp"))
            {
                line += " ({D" + disambig + "{x)"; 
            }
        }

        if (firstId == -1)
            firstId = help->getID();

        buf << fmt(0, lineFormat.c_str(), help->getID(), line.c_str());
    }

    buf << endl
        << "Для уточнения поиска смотри справку по нужному номеру, например, "
        << "{y{hcсправка " << firstId << "{x." << endl;        

    ch->send_to(buf.str().c_str());
}                  


/*-----------------------------------------------------------------
 * cast 'identify', shop item properties
 *----------------------------------------------------------------*/

void lore_fmt_affect( Object *obj, Affect *paf, ostringstream &buf )
{
    int b = paf->bitvector,
        d = paf->duration;
    const FlagTable *table = paf->bitvector.getTable();
    bool adaptive;

    if (paf->type != gsn_none){ 
        buf << paf->type->getRussianName().upperFirstCharacter().quote();
        buf << ": ";
        }
    if (paf->modifier != 0) {
        switch (paf->location) {
            case APPLY_NONE:
            case APPLY_LEARNED:
                if (paf->global.getRegistry() == skillManager 
                    || paf->global.getRegistry() == skillGroupManager) 
                {
                    buf << (paf->modifier >= 0 ? "Повышает" : "Понижает")
                        << " владение "
                        << (paf->global.getRegistry() == skillGroupManager ? "группой" : "умением")
                        << " " << paf->global.toRussianString().quote() 
                        << " на " << (int)abs(paf->modifier) << endl;
                } else if (paf->location == APPLY_LEARNED) {
                    buf << (paf->modifier >= 0 ? "Повышает" : "Понижает")
                        << " владение всеми умениями на " 
                        << (int)abs(paf->modifier) << endl;                    
                }
                break;

            case APPLY_LEVEL:
                if (paf->global.getRegistry() == skillManager 
                    || paf->global.getRegistry() == skillGroupManager) 
                {
                    buf << (paf->modifier >= 0 ? "Повышает" : "Понижает")
                        << " уровень умения "
                        << (paf->global.getRegistry() == skillGroupManager ? "группы" : "")
                        << " " << paf->global.toRussianString().quote() 
                        << " на " << (int)abs(paf->modifier) << endl;
                    return;
                }

                /* FALL THROUGH */

            default:
                adaptive = obj->item_type == ITEM_RECIPE && obj->getProperty("levelAdaptive") == "true";

                // Don't display exact modifier for level-adaptive affects, as it changes with level.
                buf << "Изменяет " << apply_flags.message(paf->location );
                if (!adaptive)
                    buf << " на " << paf->modifier;

                if (d > -1)
                    buf << ", в течение " << d << " час" << GET_COUNT(d, "а", "ов", "ов");
            
                buf << "." << endl;

                if (adaptive)
                    buf << "Точные свойства этого рецепта зависят от уровня того, кто его использует." << endl;
                break;
        }
    }

    if (b) {
        if (table == &affect_flags)
            buf << "Добавляет аффект " << affect_flags.messages(b ) << endl;
        else if (table == &imm_flags)
            buf << "Добавляет иммунитет к " << imm_flags.messages(b ) << endl;
        else if (table == &res_flags)
            buf << "Добавляет сопротивляемость к " << imm_flags.messages(b ) << endl;
        else if (table == &vuln_flags)
            buf << "Добавляет уязвимость к " << imm_flags.messages(b ) << endl;
        else if (table == &detect_flags)
            buf << "Добавляет обнаружение " << detect_flags.messages(b ) << endl;
        else if (table == &form_flags)
            buf << "Добавляет " << form_flags.messages(b) << " форму тела" << endl;
        else if (table == &weapon_type2){
            buf << "Добавляет флаг " << weapon_type2.messages(b);
            if (d > -1) {
                buf << ", в течение " << d << " час" << GET_COUNT(d, "а", "ов", "ов");
            } 
            buf << endl;
        }
        else if (table == &extra_flags){
            buf << "Добавляет свойство " << extra_flags.messages(b);
            if (d > -1) {
                buf << ", в течение " << d << " час" << GET_COUNT(d, "а", "ов", "ов");
            } 
            buf << endl;
        }
    }
}

void lore_fmt_item( Character *ch, Object *obj, ostringstream &buf, bool showName )
{
    int lim;
    Skill *skill;
    Liquid *liquid;
    Keyhole::Pointer keyhole;
    
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
        
    buf << "." << endl;
    
    if (showName)
        buf << "Взаимодействует по именам: '{W" << obj->getName( ) << "{x'" << endl;

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

    lim = obj->pIndexData->limit;
    if (lim != -1 && lim < 100)
        buf << "{RТаких вещей в мире может быть не более {W" << lim << "{x!" << endl;

    if (obj_is_special(obj))
        buf << "{WЭтот предмет обладает неведомыми, но мощными свойствами.{x" << endl;    

    if (obj->timer != 0)
        buf << fmt(0, "{WЭтот предмет исчезнет через %1$d мину%1$Iту|ты|т.{x\r\n", obj->timer);

    if (IS_SET(obj->extra_flags, ITEM_NOIDENT)) {
        buf << endl << "Более про эту вещь невозможно ничего сказать." << endl;
        return;
    }
    
    bitstring_t extra = obj->extra_flags;
    REMOVE_BIT(extra, ITEM_WATER_STAND|ITEM_INVENTORY|ITEM_HAD_TIMER|ITEM_DELETED);
    if (extra)
        buf << "Особые свойства: " << extra_flags.messages(extra, true ) << endl;

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

    if (!obj->enchanted)
        for (auto &paf: obj->pIndexData->affected)
            lore_fmt_affect( obj, paf, buf );

    for (auto &paf: obj->affected)
        lore_fmt_affect( obj, paf, buf );
}
