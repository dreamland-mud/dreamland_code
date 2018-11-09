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

#include <map>
#include <sstream>

#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"
#include "char.h"
#include "logstream.h"
#include "grammar_entities_impl.h"

#include "skill.h"
#include "skillmanager.h"
#include "spell.h"
#include "affecthandler.h"
#include "areabehaviormanager.h"
#include "mobilebehavior.h"
#include "xmlattributeticker.h"
#include "commonattributes.h"
#include "commandtemplate.h"
#include "playerattributes.h"

#include "affect.h"
#include "object.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "pcrace.h"
#include "room.h"
#include "desire.h"
#include "helpmanager.h"

#include "dreamland.h"
#include "merc.h"
#include "descriptor.h"
#include "comm.h"
#include "colour.h"
#include "mudtags.h"
#include "bugtracker.h"
#include "act.h"
#include "alignment.h"
#include "interp.h"

#include "occupations.h"
#include "raceflags.h"
#include "recipeflags.h"
#include "gsn_plugin.h"
#include "def.h"
#include "act_move.h"
#include "act_lock.h"
#include "handler.h"
#include "stats_apply.h"
#include "vnum.h"
#include "mercdb.h"

using std::endl;
using std::min;
using std::max;

PROF(none);
PROF(universal);
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

NPCharacter * find_mob_with_act( Room *room, bitstring_t act )
{    
    for (Character* rch = room->people; rch != 0; rch = rch->next_in_room )
       if (rch->is_npc() && IS_SET(rch->act, act))
          return rch->getNPC( );
    return NULL;
}

CMDRUNP( rules )
{
    do_help(ch,"worldrules");
}

#define MAX_PROMPT_SIZE 75

CMDRUNP( prompt )
{
    DLString old;

    if ( argument[0] == '\0' )
    {
        if (IS_SET(ch->comm,COMM_PROMPT))
        {
            ch->send_to("Вывод строки состояния (prompt) выключен.\n\r");
            REMOVE_BIT(ch->comm,COMM_PROMPT);
        }
        else
        {
            ch->send_to("Вывод строки состояния (prompt) включен.\n\r");
            SET_BIT(ch->comm,COMM_PROMPT);
        }
        return;
    }

    if (arg_is_all( argument )) {
        old = ch->prompt;
        ch->prompt = "<{r%h{x/{R%H{xзд {c%m{x/{C%M{xман %v/%Vшг {W%X{xоп Вых:{g%d{x>%c";
    }
    else if (arg_is_show( argument )) {
        ch->println( "Текущая строка состояния:" );
        ch->desc->send( ch->prompt.c_str( ) );
        ch->send_to( "\n\r" );
        return;
    }    
    else {
          old = ch->prompt;
        ch->prompt = argument;
        ch->prompt.cutSize( MAX_PROMPT_SIZE );
    }
    
    if (!old.empty( )) {
            ch->send_to( "Предыдущая строка состояния: " );
            ch->desc->send(  old.c_str( ) );   
               ch->send_to( "\n\r" );
    }
    ch->printf("Новая строка состояния: %s\n\r",ch->prompt.c_str( ) );
}

CMDRUNP( battleprompt )
{
    DLString old;

   if ( argument[0] == '\0' )
   {
      ch->send_to("Необходимо указать вид строки состояния.\nДля получения более подробной информации используй 'help prompt'\n\r");
      return;
   }

    if (arg_is_all( argument )) {
        old = ch->batle_prompt;
        ch->batle_prompt = "<{r%h{x/{R%H{xзд {c%m{x/{C%M{xман %v/%Vшг %Xоп Вых:{g%d{x> [{r%y{x:{Y%o{x]%c";
    }
    else if (arg_is_show( argument )) {
        ch->println( "Текущая строка состояния в бою:" );
        ch->desc->send( ch->batle_prompt.c_str( ) );
        ch->send_to( "\n\r" );
        return;
    }    
    else {
        old = ch->batle_prompt;
        ch->batle_prompt = argument;
        ch->batle_prompt.cutSize( MAX_PROMPT_SIZE );
    }

    if (!old.empty( )) {
            ch->send_to( "Предыдущая строка состояния в бою: " );
            ch->desc->send(  old.c_str( ) );   
               ch->send_to( "\n\r" );
    }
    ch->printf("Новая строка состояния в бою: %s\n\r",ch->batle_prompt.c_str( ) );
}

CMDRUNP( clear )
{
    if (!ch->is_npc( ))
        ch->send_to("\033[0;0H\033[2J");
}

static DLString show_money( int g, int s )
{
    ostringstream buf;

    if (g > 0 || s > 0)
        buf << (g > 0 ? "%1$d золот%1$Iая|ые|ых" : "")
            << (g * s == 0 ? "" : " и ")
            << (s > 0 ? "%2$d серебрян%2$Iая|ые|ых" : "")
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

CMDRUNP( money )
{
    ch->send_to( "У тебя в кошельке " );
    ch->println( show_money( ch->gold, ch->silver ) );
}

CMDRUNP( experience )
{
    if (ch->is_npc( ))
        return;
    
    ch->println( show_experience( ch->getPC( ) ) );
}

CMDRUNP( worth )
{
    ch->send_to( "У тебя " );
    ch->println( show_money( ch->gold, ch->silver ) );

    if ( ch->is_npc() )
            return;

    ch->println( show_experience( ch->getPC( ) ) );

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
    { POS_MORTAL,   "Ты присмерти."               },
    { POS_INCAP,    "Ты в беспомощном состоянии." },
    { POS_STUNNED,  "Тебя оглушили."              },
    { POS_SLEEPING, "Ты спишь."                   },
    { POS_RESTING,  "Ты отдыхаешь."               },
    { POS_SITTING,  "Ты сидишь."                  },
    { POS_FIGHTING, "Ты сражаешься."              },
    { POS_STANDING, "Ты стоишь."                  },
    { -1 }
};

msgtable_t msg_stat_cha = {        
    {  0, "Mongol     " },         
    { 10, "Poor       " },         
    { 14, "Average    " },         
    { 18, "Good       " },         
    { 20, "Familier   " },         
    { 22, "Charismatic" },         
    { -1 }                         
};                                 

msgtable_t msg_stat_con = {        
    {  0, "Fragile    " },         
    { 10, "Poor       " },         
    { 14, "Average    " },         
    { 18, "Healthy    " },         
    { 20, "Hearty     " },         
    { 22, "Iron       " },         
    { -1 }                         
};                                 

msgtable_t msg_stat_str = {        
    {  0, "Weak       " },         
    { 10, "Poor       " },         
    { 14, "Average    " },         
    { 18, "Strong     " },         
    { 20, "Herculian  " },         
    { 22, "Titantic   " },         
    { -1 }                         
};                                 

msgtable_t msg_stat_dex = {
    {  0, "Slow       " },
    { 10, "Clumsy     " },
    { 14, "Average    " },
    { 18, "Dextrous   " },
    { 20, "Quick      " },
    { 22, "Fast       " },
    { -1 }
};
                            
msgtable_t msg_stat_int = {
    {  0, "Hopeless   " },
    { 10, "Poor       " },
    { 14, "Average    " },
    { 18, "Good       " },
    { 20, "Clever     " },
    { 22, "Genious    " },
    { -1 }
};
                            
msgtable_t msg_stat_wis = {
    {  0, "Fool       " },
    { 10, "Dim        " },
    { 14, "Average    " },
    { 18, "Good       " },
    { 20, "Wise       " },
    { 22, "Excellent  " },
    { -1 }
};


msgtable_t msg_armor_oscore = {
    { -101, "божественно защищен"   },
    { -100, "неуязвимо защищен"     },
    {  -80, "первоклассно защищен"  },
    {  -60, "отлично защищен"       },
    {  -40, "очень хорошо защищен"  },
    {  -20, "хорошо защищен"        },
    {    0, "защищен"               },
    {   20, "кое-как защищен"       },
    {   40, "слабо защищен"         },
    {   60, "едва защищен"          },
    {   80, "не защищен"            },
    {  101, "совершенно не защищен" },
    { -1 }
};

msgtable_t msg_armor = {
    { -101, "божественно"   },
    { -100, "неуязвимо"     },
    {  -80, "первоклассно"  },
    {  -60, "отлично"       },
    {  -40, "очень хорошо"  },
    {  -20, "хорошо"        },
    {    0, "защищен"       },
    {   20, "хоть как-то"   },
    {   40, "слегка"        },
    {   60, "едва"          },
    {   80, "очень слабо"   },
    {  101, "не защищен"    },
    { -1 }
};

CMDRUNP( oscore )
{
    ostringstream buf;
    Room *room = 0;
    int i;
    PCharacter *pch = ch->getPC( );

    buf << fmt( 0, "Ты %1$s%2$s{x, уровень %3$d",
                   ch->seeName( ch, '1' ).c_str( ),
                   ch->is_npc( ) ? "" : ch->getPC( )->getParsedTitle( ).c_str( ),
                   ch->getRealLevel( ));
    
    if (!ch->is_npc( ))
        buf << fmt( 0, ", тебе %1$d %1$Iгод|года|лет (%2$d ча%2$Iс)|са)|сов).",
                        pch->age.getYears( ), pch->age.getHours( ) ); 
    
    buf << endl;

    if (ch->getRealLevel( ) != ch->get_trust( ))
        buf << "Уровень доверия к тебе составляет " << ch->get_trust( ) << "." << endl;

    buf << "Раса " << ch->getRace( )->getNameFor( ch, ch )
        << "  Пол: " << sex_table.message( ch->getSex( ) )
        << "  Класс: " << ch->getProfession( )->getNameFor( ch );
    
    if (!ch->is_npc( ))
        if (pch->getSubProfession( ) != prof_none)
            buf << "(" << pch->getSubProfession( )->getNameFor( ch ) << ")";
    
    if (!ch->is_npc( ))
        room = get_room_index( ch->getPC()->getHometown( )->getAltar() );
    else
        room = get_room_index( ROOM_VNUM_TEMPLE );
    
    buf << "  Дом: " << (room ? room->area->name : "Потерян" ) << endl
        << dlprintf( "У тебя %d/%d жизни, %d/%d энергии и %d/%d движения.\n\r",
                    ch->hit.getValue( ), ch->max_hit.getValue( ), 
                    ch->mana.getValue( ), ch->max_mana.getValue( ), 
                    ch->move.getValue( ), ch->max_move.getValue( ));
    
    if (!ch->is_npc( )) 
        buf << fmt( 0, "У тебя %1$d практи%1$Iка|ки|к и %2$d тренировочн%2$Iая|ые|ых сесси%2$Iя|и|й.",
                       pch->practice.getValue( ), pch->train.getValue( ) )
            << endl;
    
   buf << dlprintf( "Ты несешь %d/%d вещей с весом %d/%d фунтов.\n\r",
                ch->carry_number, ch->canCarryNumber( ),
                ch->getCarryWeight( )/10, ch->canCarryWeight( )/10 );

    buf << dlprintf( 
            "Твои параметры:   Сила(Str): %d(%d) Интеллект(Int): %d(%d)\n\r"
            "              Мудрость(Wis): %d(%d)  Ловкость(Dex): %d(%d)\n\r"
            "              Сложение(Con): %d(%d)   Обаяние(Cha): %d(%d)\n\r",
            ch->perm_stat[STAT_STR], ch->getCurrStat(STAT_STR),
            ch->perm_stat[STAT_INT], ch->getCurrStat(STAT_INT),
            ch->perm_stat[STAT_WIS], ch->getCurrStat(STAT_WIS),
            ch->perm_stat[STAT_DEX], ch->getCurrStat(STAT_DEX),
            ch->perm_stat[STAT_CON], ch->getCurrStat(STAT_CON),
            ch->perm_stat[STAT_CHA], ch->getCurrStat(STAT_CHA) );

    buf << dlprintf( "У тебя %d очков опыта, и %s\n\r",
                  ch->exp.getValue( ),
                  show_money( ch->gold, ch->silver ).c_str( ) );

    /* KIO shows exp to level */
    if (!ch->is_npc() && ch->getRealLevel( ) < LEVEL_HERO - 1)
        buf << dlprintf( "Тебе нужно набрать %d очков опыта до следующего уровня.\n\r",
                    ch->getPC()->getExpToLevel( ) );

    if (!ch->is_npc( )) {
        XMLAttributeTimer::Pointer qd = pch->getAttributes( ).findAttr<XMLAttributeTimer>( "questdata" );
        int qtime = qd ? qd->getTime( ) : 0;
        bool hasQuest = pch->getAttributes( ).isAvailable( "quest" );
        
        buf << fmt( 0, "У тебя %1$d квестов%1$Iая|ые|ых едини%1$Iца|цы|ц. ",
                       pch->questpoints.getValue( ) );
        if (qtime == 0)
            buf << "У тебя сейчас нет задания.";
        else
            buf << fmt( 0, "До %1$s квеста осталось %2$d ти%2$Iк|ка|ков.",
                       hasQuest ? "конца" : "следующего",
                       qtime );

        buf << endl;

        if (ch->getProfession( ) != prof_samurai)
            buf << dlprintf( "Ты попытаешься убежать при %d жизни.  ", ch->wimpy.getValue( ) );
        else
            buf << dlprintf( "Тебя убили уже %d раз.  ", ch->getPC( )->death.getValue( ));

        if (ch->getPC()->guarding != 0)
            buf << dlprintf( "Ты охраняешь: %s. ", ch->seeName( ch->getPC()->guarding, '4' ).c_str( ) );

        if (ch->getPC()->guarded_by != 0)
            buf << dlprintf( "Ты охраняешься: %s.", ch->seeName( ch->getPC()->guarded_by, '5' ).c_str( ) );
        
        buf << endl;
    }

    if (!ch->is_npc( )) {
        ostringstream dbuf;

        for (int i = 0; i < desireManager->size( ); i++) {
            ostringstream b;
            
            desireManager->find( i )->report( ch->getPC( ), b );
            
            if (!b.str( ).empty( ))
                dbuf << b.str( ) << " ";
        }

        if (!dbuf.str( ).empty( ))
            buf << dbuf.str( ); 
    }
    
    buf << msgtable_lookup( msg_positions, ch->position );

    if (ch->is_adrenalined( ) && ch->position > POS_INCAP)
        buf << " Твоя кровь полна адреналина!";
    
    buf << endl;

    /* print AC values */
    if (ch->getRealLevel( ) >= 20) {        
        buf << dlprintf( "Защита от укола %d, от удара %d, от разрезания %d, от экзотики %d.\n\r",
                GET_AC(ch,AC_PIERCE),
                GET_AC(ch,AC_BASH),
                GET_AC(ch,AC_SLASH),
                GET_AC(ch,AC_EXOTIC));
        buf << dlprintf( "{lRТочность{lEHitroll{lx: %d  {lRУрон{lEDamroll{lx: %d  {lRЗащита от заклинаний{lESaves vs Spell{lx: %d\n\r",
                    ch->hitroll.getValue( ), ch->damroll.getValue( ), ch->saving_throw.getValue( ) );
    }
    else
        for (i = 0; i < ac_type.size; i++)
            buf << dlprintf( "Ты %s от %s.\n\r",
                        msgtable_lookup( msg_armor_oscore, GET_AC(ch, i) ),
                        ac_type.message( i, '2' ).c_str( ) );

    buf << dlprintf( "У тебя %s характер.  ", align_name( ch ).ruscase( '1' ).c_str( ) );
    
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
            buf << dlprintf( "Твоя религия: %s.  ",
                        ch->getReligion( )->getShortDescr( ).c_str( ) );
        
        buf << dlprintf("Твои заслуги перед законом:  %d.\n\r", ch->getPC( )->loyalty.getValue( ));

        if (ch->getPC( )->curse != 100)
            buf << dlprintf( "Проклятье, наложенное на тебя, ухудшает все твои умения на %d%%.\n\r",
                        100 - ch->getPC( )->curse.getValue( ));

        if (ch->getPC( )->bless)
            buf << dlprintf( "Благословение богов улучшает все твои умения на %d%%.\n\r",
                        ch->getPC( )->bless.getValue( ));
    }
#if 0
    if (ch->getProfession( ) == prof_universal)
        buf << dlprintf( "У тебя %d/%d {lRочков умений{lEskill points{lx.\n\r",
                    ch->getPC()->skill_points(),
                    ch->getPC()->max_skill_points.getValue( ));
#endif
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

    ch->send_to( buf );

    if (IS_SET(ch->comm, COMM_SHOW_AFFECTS))
        interpret_raw( ch, "affects", "nocolor noempty" );
}

CMDRUNP( count )
{
    int count, max_on;
    Descriptor *d;
    char buf[MAX_STRING_LENGTH];

    count = 0;

    for ( d = descriptor_list; d != 0; d = d->next )
        if (d->connected == CON_PLAYING 
            && d->character
            && ch->can_see( d->character ))
            count++;

    max_on = Descriptor::getMaxOnline( );

    if (max_on == count)
        sprintf(buf,"Всего %d %s. Это максимум на сегодня.\n\r",count,
                count == 1 ? "игрок" : ((count > 1 && count < 5) ? "игрока" : "игроков"));
    else
        sprintf(buf,"Всего %d %s. Максимум на сегодня был: %d.\n\r",count,
                count == 1 ? "игрок" : ((count > 1 && count < 5) ? "игрока" : "игроков"),
                max_on);

    ch->send_to(buf);
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
        ch->send_to( "Сравнить что и с чем.?\n\r");
        return;
    }

    if ( ( obj1 = get_obj_carry( ch, arg1 ) ) == 0 )
    {
        ch->send_to( "У тебя нет этого.\n\r");
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
            ch->send_to("На тебе нет ничего, с чем можно было бы сравнить.\n\r");
            return;
        }
    }

    else if ( (obj2 = get_obj_carry(ch,arg2) ) == 0 )
    {
        ch->send_to("У тебя нет этого.\n\r");
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
            value1 = obj1->value[0] + obj1->value[1] + obj1->value[2];
            value2 = obj2->value[0] + obj2->value[1] + obj2->value[2];
            break;

        case ITEM_WEAPON:
            if (obj1->pIndexData->new_format)
                value1 = (1 + obj1->value[2]) * obj1->value[1];
            else
                    value1 = obj1->value[1] + obj1->value[2];

            if (obj2->pIndexData->new_format)
                value2 = (1 + obj2->value[2]) * obj2->value[1];
            else
                    value2 = obj2->value[1] + obj2->value[2];
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



CMDRUNP( credits )
{
    do_help( ch, "credits" );
    return;
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
                victim->in_room->name );
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
        ch->println( "Ты не можешь видеть вещи!" );
        return;
    }
    
    if (eyes_darkened( ch )) {
        ch->println( "Ты ничего не видишь! Слишком темно!" );
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
        ch->printf( "Ты находишься в местности {W%s{x. Недалеко от тебя:\r\n",
                     ch->in_room->area->name );
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
            ch->send_to( "Никого.\n\r");
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
            act_p( "Ты не находишь $T.", ch, 0, arg.c_str(), TO_CHAR,POS_RESTING );
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
        ch->send_to( "Сравнить свои силы с кем?\n\r");
        return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == 0 )
    {
        ch->send_to( "Его нет здесь.\n\r");
        return;
    }

    if (is_safe(ch,victim))
    {
        ch->send_to("Даже не думай об этом.\n\r");
        return;
    }

    victim = victim->getDoppel( );

    diff = victim->getModifyLevel() - ch->getModifyLevel();

         if ( diff <= -10 ) msg = "Ты можешь убить $C4 даже без оружия.";
    else if ( diff <=  -5 ) msg = "$C1 не соперник тебе.";
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

    act_p( msg, ch, 0, victim, TO_CHAR,POS_RESTING );
    act_p( align, ch, 0, victim, TO_CHAR,POS_RESTING);
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
        ch->println( "Слишком длинный титул." );
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
        ch->println( "Ты не можешь сменить титул." );
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
        pch->println( "Титул удален." );
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
    mudtags_raw( title.c_str( ), buf );

    DLString stripped = buf.str( );
    DLString nospace = stripped;
    nospace.stripWhiteSpace( );
    
    if (stripped.size( ) > 25) {
        ch->println( "Слишком длинный претитул!" );
        return false;
    }
    
    if (nospace.size( ) != stripped.size( )) {
        ch->println( "В начале или в конце претитула не должно быть пробелов." );
        return false;
    }
    
    for (unsigned int i = 0; i < stripped.size( ); i++)
        if (!dl_isalpha( stripped[i] ) 
                && stripped[i] != ' ' 
                && stripped[i] != '\'') 
        {
            ch->println( "В претитуле разрешено использовать только буквы, пробелы и одинарные кавычки." );
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
         pch->println( "Ты не можешь изменить претитул.");
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
        pch->println("Русский и английский претитулы очищены.");
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


CMDRUNP( report )
{
    char buf[MAX_INPUT_LENGTH];

    sprintf( buf,
        "У меня %d/%d жизни (hp) %d/%d энергии (mana) %d/%d движения (mv).",
        ch->hit.getValue( ),  ch->max_hit.getValue( ),
        ch->mana.getValue( ), ch->max_mana.getValue( ),
        ch->move.getValue( ), ch->max_move.getValue( ) );
    do_say( ch, buf );

    return;
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
        ch->send_to( "Твоя отвага превосходит твою мудрость.\n\r");
        return;
    }

    if ( wimpy > ch->max_hit/2 )
    {
        ch->send_to( "Это будет слишком большим позором для тебя.\n\r");
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
        ch->send_to( "Синтаксис: {lRпароль{lEpassword{lx <старый> <новый>.\n\r");
        return;
    }
    
    if (!password_check( ch->getPC( ), arg1 ))
    {
        ch->setWait(40 );
        ch->send_to( "Неверный пароль. Подождите 10 секунд.\n\r");
        return;
    }

    if ( strlen(arg2) < 5 )
    {
        ch->send_to("Новый пароль должен содержать более пяти символов.\n\r");
        return;
    }

    /*
     * No tilde allowed because of player file format.
     */
    pwdnew = arg2;
    for ( p = pwdnew; *p != '\0'; p++ )
    {
        if ( *p == '~' )
        {
            ch->send_to("Новый пароль неприемлем, попробуй еще раз.\n\r");
            return;
        }
    }
   
    password_set( ch->getPC( ), pwdnew );
    ch->getPC( )->save( );
    ch->send_to( "Ok.\n\r");
    return;
}

/* RT configure command */


CMDRUNP( request )
{
        char arg1 [MAX_INPUT_LENGTH];
        char arg2 [MAX_INPUT_LENGTH];
        Character *victim;
        Object  *obj;
        Affect af;

        if ( ch->isAffected(gsn_gratitude))
        {
                ch->send_to("Подожди немного.\n\r");
                return;
        }

        argument = one_argument( argument, arg1 );
        argument = one_argument( argument, arg2 );

        if (ch->is_npc())
                return;

        if ( arg1[0] == '\0' || arg2[0] == '\0' )
        {
                ch->send_to( "Что и у кого ты хочешь попросить?\n\r");
                return;
        }

        if ( ( victim = get_char_room( ch, arg2 ) ) == 0 )
        {
                ch->send_to( "Здесь таких нет.\n\r");
                return;
        }

        if (!victim->is_npc())
        {
                ch->send_to("Проси другой командой: say <Подари мне ЭТО>!\n\r");
                return;
        }

        if (victim->getNPC()->behavior 
            && IS_SET(victim->getNPC()->behavior->getOccupation( ), (1 << OCC_SHOPPER)))
        {
                ch->send_to("Хочешь -- купи!\n\r");
                return;
        }

        if (!IS_AWAKE(victim)) {
            interpret_raw( victim, "snore" );
            return;
        }

        if ((!IS_GOOD(ch) && !victim->getRace( )->getAttitude( *ch->getRace( ) ).isSet( RACE_DONATES ))
                || IS_EVIL(ch))
        {
                do_say(victim, "У тебя нечистая душа, я ничего тебе не дам!");
                return;
        }

        if (ch->move < (50 + ch->getRealLevel( )))
        {
                do_say(victim, "Ты выглядишь устало, может, отдохнешь сначала?");
                return;
        }

        ch->setWaitViolence( 1 );
        ch->move -= 10;
        ch->move = max((int)ch->move, 0);

        if (victim->getModifyLevel( ) >= ch->getModifyLevel( ) + 10 || victim->getModifyLevel( ) >= ch->getModifyLevel( ) * 2)
        {
                do_say(victim, "Всему свое время, малыш.");
                return;
        }

        if ( ( ( obj = get_obj_carry(victim , arg1 ) ) == 0
                        && (obj = get_obj_wear(victim, arg1)) == 0)
                || IS_SET(obj->extra_flags, ITEM_INVENTORY))
        {
                do_say(victim, "Извини, у меня нет этого.");
                return;
        }
        
        if (victim->getRace( )->getAttitude( *ch->getRace( ) ).isSet( RACE_DONATES ))
        {
            if (IS_EVIL( victim )) {
                interpret( victim, "grin" );
                return;
            }

        } else if (!IS_GOOD(victim))
        {
                do_say(victim, "Я не дам тебе ничего!!");
                interpret_raw( victim, "murder", ch->getNameP( ));
                return;
        }

        if ( obj->wear_loc != wear_none )
                unequip_char(victim, obj);

        if ( !can_drop_obj( ch, obj ) )
        {
                do_say(victim, "Извини, но эта вешь проклята, я не могу избавиться от нее.");
                return;
        }

        if ( ch->carry_number + obj->getNumber( ) > ch->canCarryNumber( ) )
        {
                ch->send_to( "Твои руки полны.\n\r");
                return;
        }

        if ( ch->carry_weight + obj->getWeight( ) > ch->canCarryWeight( ) )
        {
                ch->send_to( "Ты не можешь нести такой вес.\n\r");
                return;
        }

        if ( !ch->can_see( obj ) )
        {
                ch->send_to( "Ты не видишь этого.\n\r");
                return;
        }

        if ( obj->pIndexData->vnum == 520 ) // Knight's key
        {
                ch->send_to("Извини, он не отдаст тебе это.\n\r");
                return;
        }

        obj_from_char( obj );
        obj_to_char( obj, ch );
        act_p( "$c1 просит $o4 у $C2.", ch, obj, victim, TO_NOTVICT,POS_RESTING );
        act_p( "Ты просишь $o4 у $C2.",   ch, obj, victim, TO_CHAR,POS_RESTING    );
        act_p( "$c1 просит $o4 у тебя.", ch, obj, victim, TO_VICT,POS_RESTING    );
        
        omprog_give( obj, victim, ch );

        ch->move -= ( 50 + ch->getModifyLevel() );
        ch->move = max( (int)ch->move, 0 );
        ch->hit -= 3 * ( ch->getModifyLevel() / 2 );
        ch->hit = max( (int)ch->hit, 0 );

        act_p("Ты чувствуешь благодарность за доверие $C2.", ch, 0, victim,TO_CHAR,POS_RESTING);

        af.type = gsn_gratitude;
        af.where = TO_AFFECTS;
        af.level = ch->getModifyLevel();
        af.duration = ch->getModifyLevel() / 10;
        af.location = APPLY_NONE;
        af.modifier = 0;
        af.bitvector = 0;
        affect_to_char ( ch,&af );

        return;
}




CMDRUNP( identify )
{
    Object *obj;
    Character *rch;

    if ( ( obj = get_obj_carry( ch, argument ) ) == 0 )
    {
       ch->send_to( "У тебя нет этого.\n\r");
       return;
    }

    rch = find_mob_with_act( ch->in_room, ACT_SAGE );

    if (!rch)
    {
       ch->send_to("Тут никто ничего толкового не скажет об этой вещи.\n\r");
       return;
    }

    if (ch->is_immortal( )) {
        act_p( "$c1 смотрит на тебя!\n\r", rch, obj, ch, TO_VICT,POS_RESTING );
    }
    else if (ch->gold < 20) {
        tell_dim( ch, rch, "У тебя даже 20 золотых нету, чтобы мне заплатить!" );
        return;
    }
    else {
       ch->gold -= 20;
       ch->send_to("Твой кошелек становится значительно легче.\n\r");
    }

    act_p( "$c1 изучающе смотрит на $o4.", rch, obj, 0, TO_ROOM,POS_RESTING );
    
    if (gsn_identify->getSpell( ))
        gsn_identify->getSpell( )->run( ch, obj, gsn_identify, 0 );
}

/* room affects */
CMDRUNP( raffects )
{
    ostringstream buf;
    
    for (Affect *paf = ch->in_room->affected; paf != 0; paf = paf->next)
        if (paf->type->getAffect( ))
            paf->type->getAffect( )->toStream( buf, paf );
    
    if (buf.str( ).empty( )) 
        buf << "В этом месте нет ничего необычного." << endl;

    ch->send_to( buf );
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

  if (ch->getTrueProfession( ) != prof_anti_paladin)
    {
        ch->println( "Ты никого не запугаешь своим видом." );
      return;
    }

  if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
        ch->println( "Потребовать что и у кого?" );
      return;
    }

  if ( ( victim = get_char_room( ch, arg2 ) ) == 0 )
    {
        ch->println( "Таких тут нет." );
      return;
    }

    if (!victim->is_npc( )) {
        ch->println( "Просто убей и отбери." );
        return;
    }

    if (IS_SET(victim->act, ACT_NODEMAND)) {
        act( "$C1 не подчинится твоему требованию.", ch, 0, victim, TO_CHAR);
        return;
    }
  
    if (victim->getNPC()->behavior 
        && IS_SET(victim->getNPC()->behavior->getOccupation( ), (1 << OCC_SHOPPER))) 
    {
        ch->send_to("Хочешь -- купи!\n\r");
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


  if ( obj->wear_loc != wear_none )
    unequip_char(victim, obj);

  if ( !can_drop_obj( ch, obj ) )
    {
      do_say(victim,
        "Эта вещь проклята, и я не могу избавиться от нее. Прости меня, повелитель.");
      return;
    }

  if ( ch->carry_number + obj->getNumber( ) > ch->canCarryNumber( ) )
    {
      ch->send_to( "Твои руки полны.\n\r");
      return;
    }

  if ( ch->carry_weight + obj->getWeight( ) > ch->canCarryWeight( ) )
    {
      ch->send_to( "Ты не сможешь нести такую тяжесть.\n\r");
      return;
    }

  if ( !ch->can_see( obj ) )
    {
      act_p( "Ты не видишь этого.", ch, 0, victim, TO_CHAR,POS_RESTING );
      return;
    }

    act( "$c1 требует $o4 у $C2.", ch, obj, victim, TO_NOTVICT);
    act( "Ты требуешь $o4 у $C2.",   ch, obj, victim, TO_CHAR);
    act( "$c1 требует у тебя $o4.", ch, obj, victim, TO_VICT);

    obj_from_char( obj );
    obj_to_char( obj, ch );

    omprog_give( obj, victim, ch );

    ch->println("Твое могущество повергает всех в трепет.");
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
    Room *room = get_room_index( pch->getHometown( )->getAltar( ) );
    bool fRus = ch->getConfig( )->rucommands;
    DLString profName = ch->getProfession( )->getNameFor( ch );

    if (ch->getProfession( ) == prof_universal) 
        profName << "+"
                 << (pch->getSubProfession( ) != prof_none ? 
                        pch->getSubProfession( )->getWhoNameFor( ch ) : "   ");
        
    ostringstream name;
    DLString title = pch->getParsedTitle( );
    name << ch->seeName( ch, '1' ) << "{x ";
    vistags_convert( title.c_str( ), name, ch );

    ch->printf( 
"%s\n\r"
"      /~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~/~~\\\n\r", 
             CLR_FRAME);
    ch->println(
        fmt ( 0, "     %s|   %s%-50.50s {y%3d{x %4s   %s|____|",
                CLR_FRAME,
                CLR_CAPT,
                name.str( ).c_str( ),
                age,
                GET_COUNT(age, "год", "года", "лет"),
                CLR_FRAME ) );
                
        
    ch->printf(
"     |%s+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+%s|\n\r" 
"     | %sУровень:{x  %3d        %s|%s {lRСила:{lE Str:{lx{x %2d{c({x%2d{c){x {C%2d{x %s| %sРелигия   :{x %-10s %s|\n\r"
"     | %sРаса :{x  %-12s %s| %s{lRУм  :{lE Int:{lx{x %2d{c({x%2d{c){x {C%2d{x %s| %sПрактик   :{x   %3d      %s|\n\r"
"     | %sПол  :{x  %-11s  %s| %s{lRМудр:{lE Wis:{lx{x %2d{c({x%2d{c){x {C%2d{x %s| %sТренировок:{x   %3d      %s|\n\r"
"     | %sКласс:{x  %-13s%s| %s{lRЛовк:{lE Dex:{lx{x %2d{c({x%2d{c){x {C%2d{x %s| %sКвест. единиц:{x  %-5d%s  |\n\r"
"     | %s{lRНатура:{lEAlign: {lx{x %-11s  %s| %s{lRСлож:{lE Con:{lx{x %2d{c({x%2d{c){x {C%2d{x %s| %sКвест. время:{x   %-3d %s   |\n\r"
"     | %s{lRЭтос {lEEthos{lx:{x  %-12s %s| %s{lRОбая:{lE Cha:{lx{x %2d{c({x%2d{c){x {C%2d{x %s| %s%s :{x   %3d      %s|\n\r"
"     | %sДом  :{x  %-30s %s| {Y%-22s %s|\n\r"                
"     |%s+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+%s|\n\r",

            CLR_BAR, CLR_FRAME,

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
            pch->questpoints.getValue( ),
            CLR_FRAME,
    
            CLR_CAPT,
            fRus ? (IS_GOOD(ch) ? "добрая" : IS_EVIL(ch) ? "злая" : "нейтральная")
                 : align_table.name( ALIGNMENT(ch) ).c_str( ),
            CLR_BAR,
            CLR_CAPT,
            ch->perm_stat[STAT_CON], ch->getCurrStat(STAT_CON), pch->getMaxStat(STAT_CON),
            CLR_BAR,
            CLR_CAPT,
            qd ? qd->getTime( ) : 0,
            CLR_FRAME,

            CLR_CAPT,
            fRus ?  ethos_table.message( ch->ethos ).cutSize( 12 ).c_str( )
                  : ethos_table.name( ch->ethos ).c_str( ),
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
            room ? room->area->name : "Потерян",
            CLR_BAR,
            msgtable_lookup( msg_positions, ch->position ),
            CLR_FRAME,

            CLR_BAR, CLR_FRAME);

    if (pch->guarding != 0) {
        ekle = 1;
        ch->printf( 
"     | {wТы охраняешь    :{Y %-10s                                    %s|\n\r",
            ch->seeName( pch->guarding, '4' ).c_str(),
            CLR_FRAME);
    }

    if (pch->guarded_by != 0) {
        ekle = 1;
        ch->printf( 
"     | {wТебя охраняет     :{Y %-10s                                  %s|\n\r",
        ch->seeName( pch->guarded_by, '1' ).c_str(),
        CLR_FRAME);
    }

    for (int i = 0; i < desireManager->size( ); i++) {
        ostringstream buf;
        
        desireManager->find( i )->report( ch->getPC( ), buf );

        if (!buf.str( ).empty( )) {
            ekle = 1;
            ch->printf( "     | {w%-64s%s|\r\n", 
                        buf.str( ).c_str( ),
                        CLR_FRAME );
        }
    }

    if (ch->is_adrenalined()) {
        ekle = 1;
        ch->printf( 
"     | {yАдреналин кипит в твоих венах!                                  %s|\n\r",
                 CLR_FRAME );
    }

    if (ch->is_immortal()) {
        ekle = 1;
        ch->printf( 
"     | {w{lRНевидимость:{lEInvisible:  {lx {lRуровня{lElevel{lx %3d   "
         "{lRИнкогнито{lEIncognito  {lx: {lRуровня{lElevel{lx %3d                 %s|\n\r",
              pch->invis_level.getValue( ),
              pch->incog_level.getValue( ),
              CLR_FRAME);
    }

    list<DLString> attrLines;
    if (ch->getPC()->getAttributes( ).handleEvent( ScoreArguments( ch->getPC(), attrLines ) )) {
        ekle = 1;
        for (list<DLString>::iterator l = attrLines.begin( ); l != attrLines.end( ); l++) {
            ch->printf("     | {w%-64s%s|\r\n", 
                        l->c_str(),
                        CLR_FRAME);
        }
    }

    if (ekle) {
        ch->printf( 
"     |%s+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+%s|\n\r",
                CLR_BAR,
                CLR_FRAME);
    }


if ( ch->getRealLevel( ) >= 20 )
{
    ch->printf( 
"     | %sВещи          :{x     %3d/%-4d        %sЗащита от уколов:{x   %-5d   %s|\n\r"
"     | %sВес           :{x  %6d/%-8d    %sЗащита от ударов:{x   %-5d   %s|\n\r"
"     | %sЗолото        :{Y %-10d          %sЗащита от разрезов:{x %-5d   %s|\n\r"
"     | %sСеребро       :{W %-10d          %sЗащита от экзотики:{x %-5d   %s|\n\r"
"     | %sЕдиниц опыта  :{x %-6d              %s{lRЗащита от заклинаний{lESaves vs Spell      {lx:{x %4d  %s|\n\r",
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
}
else
{
    ch->printf( 
"     | %sВещи   :{x %3d/%-4d            %sЗащита от уколов:{x   %-12s   %s|\n\r"
"     | %sВес    :{x %6d/%-8d     %sЗащита от ударов:{x   %-12s   %s|\n\r"
"     | %sЗолото :{Y %-10d          %sЗащита от разрезов:{x %-12s   %s|\n\r"
"     | %sСеребро:{W %-10d          %sЗащита от экзотики:{x %-12s   %s|\n\r"
"     | %sЕдиниц опыта:{x   %-6d                                          %s|\n\r",
        CLR_CAPT,
        ch->carry_number, ch->canCarryNumber( ),
        CLR_CAPT,
        msgtable_lookup( msg_armor, GET_AC(ch, AC_PIERCE) ),
        CLR_FRAME,

        CLR_CAPT,
        ch->getCarryWeight( )/10, ch->canCarryWeight( )/10,
        CLR_CAPT,
        msgtable_lookup( msg_armor, GET_AC(ch, AC_BASH) ),
        CLR_FRAME,

        CLR_CAPT,
        ch->gold.getValue( ),
        CLR_CAPT,
        msgtable_lookup( msg_armor, GET_AC(ch, AC_SLASH) ),
        CLR_FRAME,

        CLR_CAPT,
        ch->silver.getValue( ),
        CLR_CAPT,
        msgtable_lookup( msg_armor, GET_AC(ch, AC_EXOTIC) ),
        CLR_FRAME,

        CLR_CAPT,
        ch->exp.getValue( ),
        CLR_FRAME);
}

    ch->printf( 
"     | %sОпыта до уровня:{x %-6d                                         %s|\n\r"
"     |                                    %sЖизни:{x %5d / %5d         %s|\n\r",
        CLR_CAPT,
        pch->getExpToLevel( ),
        CLR_FRAME,

        CLR_CAPT,
        ch->hit.getValue( ), ch->max_hit.getValue( ),
        CLR_FRAME);

if ( ch->getRealLevel( ) >= 20 )
{
    ch->printf( 
"     | %s{lRТочность{lEHitroll {lx      :{x   %-3d            %sЭнергии:{x %5d / %5d         %s|\n\r"
"     | %s{lRУрон   {lEDamroll{lx       :{x   %-3d           %sДвижения:{x %5d / %5d         %s|\n\r",
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
}
else {
    ch->printf( 
"     |                                  %sЭнергии:{x %5d / %5d         %s|\n\r"
"     |                                 %sДвижения:{x %5d / %5d         %s|\n\r",
        CLR_CAPT,
        ch->mana.getValue( ), ch->max_mana.getValue( ),
        CLR_FRAME,

        CLR_CAPT,
        ch->move.getValue( ), ch->max_move.getValue( ),
        CLR_FRAME);
}


    ch->printf( 
"  /~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~/   |\n\r"
"  \\________________________________________________________________\\__/{x\n\r");

    if (IS_SET(ch->comm, COMM_SHOW_AFFECTS))
        interpret_raw( ch, "affects", "noempty");
}

CMDRUNP( areas )
{
    ostringstream buf, areaBuf, clanBuf, mansionBuf;
    int acnt = 0, ccnt = 0, mcnt = 0;
    AREA_DATA *pArea;
    int minLevel, maxLevel, level;
    DLString arguments( argument ), args, arg1, arg2;
    
    arguments.stripWhiteSpace( );
    level = minLevel = maxLevel = -1;
    args = arguments;
    arg1 = arguments.getOneArgument( );
    arg2 = arguments.getOneArgument( );
    
    if (!arg1.empty( ) && !arg2.empty( ) && arg1.isNumber( ) != arg2.isNumber( )) {
        ch->println( "Использование: \r\n"
                     "{lEareas [<level> | <min level> <max level> | <string>]"
                     "{lRзоны [<уровень> | <мин.уровень> <макс.уровень> | <название>]{lx" );
        return;
    }
    
    try {
        if (arg1.isNumber( )) {
            level = minLevel = arg1.toInt( );
            args.clear( );
        }

        if (arg2.isNumber( )) {
            level = -1;
            maxLevel = arg2.toInt( );
            args.clear( );
        }
    } catch (const ExceptionBadType & ) {
        ch->send_to( "Уровень зоны указан неверно.\r\n" );
        return;
    }
    
    if (level != -1) 
        buf << "{YАрии мира Dream Land для уровня " << level << ":{x" << endl;
    else if (!args.empty( ))
        buf << "{YНайдены арии: {x" << endl;
    else if (minLevel != -1 && maxLevel != -1)
        buf << "{YАрии мира Dream Land, для уровней " 
            << minLevel << " - " << maxLevel << ":{x" << endl;
    else
        buf << "{YВсе арии мира Dream Land: {x" << endl;
    
    buf << "{wУровни    Название                       Уровни    Название{x" << endl
        << "------------------------------------------------------------------------" << endl;

    for (pArea = area_first; pArea; pArea = pArea->next) {
        if (IS_SET(pArea->area_flag, AREA_HIDDEN)) 
            continue;
        
        if (level != -1) {
            if (pArea->low_range > level || pArea->high_range < level)
                continue;
        }
        else if (minLevel != -1 && maxLevel != -1) {
            if (pArea->low_range < minLevel || pArea->high_range > maxLevel)
                continue;
        }

        if (!args.empty( )) {
            DLString aname = DLString( pArea->name ).colourStrip( );
            DLString altname = DLString( pArea->altname ).colourStrip( );
            DLString acredits = DLString( pArea->credits ).colourStrip( );
            if (!is_name( args.c_str( ), aname.c_str( ) ) 
                    && !is_name( args.c_str( ), acredits.c_str( ) )
                    && !is_name( args.c_str( ), altname.c_str( ) ))
                continue;
        }
        
        bool isMansion = area_is_mansion(pArea);
        bool isClan = area_is_clan(pArea);    
        ostringstream &str =  isMansion ? mansionBuf : isClan ? clanBuf : areaBuf;
        int &cnt = isMansion ? mcnt : isClan ? ccnt : acnt;

        str << fmt( ch, "[{w%3d{x {w%3d{x] %-30.30s ",
                        pArea->low_range, pArea->high_range, 
                        pArea->name);

        if (++cnt % 2 == 0)
            str << endl;
    }
  
    if (!areaBuf.str().empty()) { 
        buf << areaBuf.str();
        if (acnt % 2)
            buf << endl;
    }

    if (!clanBuf.str().empty()) {
        buf << "{yКлановые территории:{x" << endl << clanBuf.str();
        if (ccnt % 2)
            buf << endl;
    }

    if (!mansionBuf.str().empty()) {
        buf << "{yПригороды под застройку:{x" << endl << mansionBuf.str();
        if (mcnt % 2)
            buf << endl;
    }

    page_to_char( buf.str( ).c_str( ), ch );        
}

/*-----------------------------------------------------------------
 * 'affect' command
 *----------------------------------------------------------------*/
enum {
    FSHOW_LINES = (A),
    FSHOW_TIME = (B),
    FSHOW_COLOR = (C),
    FSHOW_EMPTY = (D),
    FSHOW_RUSSIAN = (E),
};

struct AffectOutput {
    AffectOutput( ) { }
    AffectOutput( Affect *, int flags );
    
    void format_affect( Affect * );
    DLString format_affect_location( Affect * );
    DLString format_affect_bitvector( Affect * );
    DLString format_affect_global( Affect * );
    void show_affect( ostringstream &, int );

    int type;
    int duration;
    DLString name;
    list<DLString> lines;
    bool unitMinutes;
};

struct ShadowAffectOutput : public AffectOutput {
    ShadowAffectOutput( int, int flags );
};

ShadowAffectOutput::ShadowAffectOutput( int shadow, int flags )
{
    duration = shadow * 4; 
    name = "вторая тень";
    type = -1;
    unitMinutes = false;
}

AffectOutput::AffectOutput( Affect *paf, int flags )
{
    type = paf->type;
    name = (IS_SET(flags, FSHOW_RUSSIAN) ? 
             paf->type->getRussianName( ) : paf->type->getName( ));
    duration = paf->duration;
    unitMinutes = true;
}

void AffectOutput::show_affect( ostringstream &buf, int flags )
{
    ostringstream f;
    DLString fmtResult;
    
    if (IS_SET(flags, FSHOW_RUSSIAN))
        f << "{Y%1$-23s{x";
    else
        f << "{rАффект{y: {Y%1$-15s{x";
    
    if (IS_SET(flags, FSHOW_LINES|FSHOW_TIME)) 
        f << "{y:";
    
    if (IS_SET(flags, FSHOW_LINES))
        for (list<DLString>::iterator l = lines.begin( ); l != lines.end( ); l++) {
            if (l != lines.begin( ))
                f << "," << endl << "                        ";

            f << "{y " << *l;
        }
    
    if (IS_SET(flags, FSHOW_TIME)) {
        if (duration < 0)
            f << " {cпостоянно";
        else
            f << " {yв течение {m%2$d{y "
              << (unitMinutes ? "час%2$Iа|ов|ов" : "секун%2$Iды|д|д");
    }
    
    fmtResult = fmt( 0, f.str( ).c_str( ), name.c_str( ), duration );

    buf << fmtResult << "{x" << endl;
}

void AffectOutput::format_affect( Affect *paf )
{
    DLString l;

    if (!( l = format_affect_location( paf ) ).empty( ))
        lines.push_back( l );

    if (!( l = format_affect_bitvector( paf ) ).empty( ))
        lines.push_back( l );

    if (!( l = format_affect_global( paf ) ).empty( ))
        lines.push_back( l );
}

DLString AffectOutput::format_affect_location( Affect *paf )
{
    DLString buf;
    
    if (paf->location != APPLY_NONE) 
        switch (paf->location) {
        case APPLY_HEAL_GAIN:
        case APPLY_MANA_GAIN:
            if (paf->modifier > 100)
                buf << fmt( 0, "улучшает {m%1$s{y на {m%2$d%%{y",
                               apply_flags.message( paf->location ).c_str( ),
                               paf->modifier - 100 );
            else if (paf->modifier < 100 && paf->modifier > 0)
                buf << fmt( 0, "ухудшает {m%1$s{y на {m%2$d%%{y",
                               apply_flags.message( paf->location ).c_str( ),
                               100 - paf->modifier );
            break;
            
        default:
            buf << "изменяет {m" << apply_flags.message( paf->location ) << "{y "
                << "на {m" << paf->modifier << "{y";
            break;
        }

    return buf;
}

DLString AffectOutput::format_affect_bitvector( Affect *paf )
{
    DLString buf;

    if (paf->bitvector != 0) {
        bitstring_t b = paf->bitvector;
        const char *word = 0;
        char gcase = '1';
        const FlagTable *table = 0;

        switch(paf->where) {
        case TO_AFFECTS: 
            table = &affect_flags;
            word = "добавляет";
            gcase = '4';
            break;
        case TO_IMMUNE:        
            table = &imm_flags;
            word = "иммунитет к";
            break;
        case TO_RESIST:        
            table = &imm_flags;
            word = "сопротивляемость к";
            break;
        case TO_VULN:        
            table = &imm_flags;
            word = "уязвимость к";
            break;
        case TO_DETECTS: 
            table = &detect_flags;
            word = (IS_SET(b, ADET_WEB|ADET_FEAR) ?  "добавляет" : "обнаружение");
            gcase = (IS_SET(b, ADET_WEB|ADET_FEAR) ? '4': '2');
            break;
        }

        if (word && table)
            buf << word << " {m" << table->messages( b, true, gcase ) << "{y";
    }

    return buf;
}

DLString AffectOutput::format_affect_global( Affect *paf )
{
    DLString buf;

    if (!paf->global.empty( )) {
        ostringstream s;
        vector<int> bits = paf->global.toArray( );
        
        switch (paf->where) {
        case TO_LIQUIDS:
            for (unsigned int i = 0; i < bits.size( ); i++) {
                Liquid *liq = liquidManager->find( bits[i] );

                if (!s.str( ).empty( ))
                    s << ", ";

                s << "{m" <<  liq->getShortDescr( ).ruscase( '2' ).colourStrip( ) << "{x";
            }

            buf << "запах " << s.str( );
            break;

        case TO_LOCATIONS:
            if (paf->global.isSet( wear_wrist_r ))
                buf << "отрезанная правая рука";
            else if (paf->global.isSet( wear_wrist_l ))
                buf << "отрезанная левая рука";
            else
                buf << "отрезанная конечность";
            break;
        }
    }

    return buf;
}

static bool __aff_sort_time__( const AffectOutput &a, const AffectOutput &b )
{
    if (a.unitMinutes == b.unitMinutes)
        return a.duration < b.duration;
    else
        return a.unitMinutes ? b.duration : a.duration;
}

static bool __aff_sort_name__( const AffectOutput &a, const AffectOutput &b )
{
    return a.name < b.name;
}

struct PermanentAffects {
    PermanentAffects(Character *ch) {
        this->ch = ch;
        my_res = ch->res_flags;
        my_vuln = ch->vuln_flags;
        my_imm = ch->imm_flags;
        my_aff = ch->affected_by;
        my_det = ch->detection;
    }

    void stripBitsFromAffects(Affect *paf) {
        switch(paf->where) {
        case TO_AFFECTS: 
            REMOVE_BIT(my_aff, paf->bitvector);
            break;
        case TO_IMMUNE:        
            REMOVE_BIT(my_imm, paf->bitvector);
            break;
        case TO_RESIST:        
            REMOVE_BIT(my_res, paf->bitvector);
            break;
        case TO_VULN:        
            REMOVE_BIT(my_vuln, paf->bitvector);
            break;
        case TO_DETECTS: 
            REMOVE_BIT(my_det, paf->bitvector);
            break;
        }
    }
    
    void printAll() const {
        print("У тебя иммунитет против", my_imm, imm_flags, '2');
        print("Ты обладаешь сопротивляемостью к", my_res, imm_flags, '3');
        print("Ты уязвим%1$Gо||а к", my_vuln, imm_flags, '3');
        print("Ты способ%1$Gно|ен|на обнаружить", my_det, detect_flags, '4');
        print("Ты под воздействием", my_aff, affect_flags, '2');
    }

    bool isSet() const {
        return my_res || my_vuln || my_imm || my_aff || my_det;
    }

private:
    void print(const char *messagePrefix, const int &my_flags, const FlagTable &my_table, char gcase) const {
        if (my_flags != 0) {
            DLString message = DLString(messagePrefix) + " " + my_table.messages(my_flags, true, gcase) + ".";
            ch->pecho(message.c_str(), ch);
        }
    }

    Character *ch;
    int my_res;
    int my_vuln;
    int my_imm;
    int my_aff;
    int my_det;
};

CMDRUNP( affects )
{
    ostringstream buf;
    list<AffectOutput> output;
    list<AffectOutput>::iterator o;
    int flags = FSHOW_LINES|FSHOW_TIME|FSHOW_COLOR|FSHOW_EMPTY;

    if (ch->getConfig( )->ruskills)
        SET_BIT(flags, FSHOW_RUSSIAN);
   
    // Keep track of res/vuln that are permanent (either from race affects or from items). 
    PermanentAffects permAff(ch);
 
    for (Affect* paf = ch->affected; paf != 0; paf = paf->next ) {
        if (output.empty( ) || output.back( ).type != paf->type) 
            output.push_back( AffectOutput( paf, flags ) );
        
        output.back( ).format_affect( paf );

        // Remove bits that are added via an affect, so that in the end
        // only permanent bits remain.
        permAff.stripBitsFromAffects(paf);
    }
    
    if (HAS_SHADOW(ch))
        output.push_front( ShadowAffectOutput( ch->getPC( )->shadow, flags ) );

    if (arg_has_oneof( argument, "time", "время" ))
        output.sort( __aff_sort_time__ );
    
    if (arg_has_oneof( argument, "name", "имя" ))
        output.sort( __aff_sort_name__ );

    if (arg_has_oneof( argument, "desc", "убыв" ))
        output.reverse( );

    if (arg_has_oneof( argument, "short", "brief", "кратко" ))
        REMOVE_BIT(flags, FSHOW_LINES);

    if (arg_has_oneof( argument, "nocolor", "безцвета" ))
        REMOVE_BIT(flags, FSHOW_COLOR);

    if (arg_has_oneof( argument, "noempty" ))
        REMOVE_BIT(flags, FSHOW_EMPTY);

    if (ch->getLevel( ) < 20)
        REMOVE_BIT(flags, FSHOW_TIME|FSHOW_LINES);
    
    for (o = output.begin( ); o != output.end( ); o++) 
        o->show_affect( buf, flags );

    // Output permanent bits on top.
    permAff.printAll();

    if (buf.str( ).empty( )) {
        if (IS_SET(flags, FSHOW_EMPTY) && !permAff.isSet())
            ch->println( "Ты не находишься под действием каких-либо аффектов." );
    } 
    else {
        if (permAff.isSet())
            ch->send_to("\r\n");
        ch->println( "Ты находишься под действием следующих аффектов:" );
        buf << "{x";

        if (!IS_SET(flags, FSHOW_COLOR)) {
            ostringstream showbuf;
            mudtags_convert_nocolor( buf.str( ).c_str( ), showbuf, ch );        
            ch->send_to( showbuf );
        }
        else
            ch->send_to( buf );
    }
}

CMDRUNP( nohelp )
{
    DLString txt = argument;
    txt.stripWhiteSpace( );
    if (txt.empty( )) {
        ch->println("Об отсутствии какого раздела справки ты хочешь сообщить?");
        return;
    }

    bugTracker->reportNohelp( ch, txt );
    ch->println("Записано.");
}

CMDRUNP( bug )
{
    DLString txt = argument;
    txt.stripWhiteSpace( );
    if (txt.empty( )) {
        ch->println("О какой именно ошибке ты хочешь сообщить?");
        return;
    }

    bugTracker->reportBug( ch, txt );
    ch->println( "Ошибка записана.");
}

CMDRUNP( typo )
{
    DLString txt = argument;
    txt.stripWhiteSpace( );
    if (txt.empty( )) {
        ch->println("О какой именно опечатке ты хочешь сообщить?");
        return;
    }

    bugTracker->reportTypo( ch, txt );
    ch->println( "Опечатка записана.");
}

CMDRUNP( iidea )
{
    DLString txt = argument;
    txt.stripWhiteSpace( );
    if (txt.empty( )) {
        ch->println("О какой именно идее ты хочешь сообщить?");
        return;
    }

    bugTracker->reportIdea( ch, txt );
    ch->println( "Идея записана.");
}

/*---------------------------------------------------------------------------*
 * Help
 *---------------------------------------------------------------------------*/
struct HelpFinder {
    typedef vector<HelpArticle::Pointer> ArticleArray;

    HelpFinder(Character *ch, const char *argument) {
        HelpArticles::const_iterator a;

        for (a = helpManager->getArticles( ).begin( ); a != helpManager->getArticles( ).end( ); a++) {
            if (!(*a)->visible( ch ))
                continue;
            if (!articleMatches(*a, argument))
                continue;
            
            articles.push_back(*a); 
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
    bool articleMatches(const HelpArticle::Pointer &a, const char *argument) const
    {
        const DLString &fullKw = a->getKeyword();

        if (is_name(argument, fullKw.c_str()))
            return true; 

        for (StringSet::const_iterator k = (*a)->getKeywords().begin(); k != (*a)->getKeywords().end(); k++)
            if (is_name(argument, (*k).c_str()))
                return true; 

        return false;
    }
	
	ArticleArray articles;
};

CMDRUNP( help )
{
    std::basic_ostringstream<char> buf;
    DLString origArgument( argument );

    if (argument[0] == '\0') {
        if (ch->getConfig( )->rucommands)
            strcpy(argument, "summary_ru");
        else
            strcpy(argument, "summary_en");
    }

    // Вариант 2.create? - needs exact match.
    if (strchr( argument , '.')){
        char argall[MAX_INPUT_LENGTH];
        int number = number_argument(argument, argall);

        if (number >= 1) {
            HelpArticle::Pointer help = HelpFinder(ch, argall).get(number);
            if (help) {
                page_to_char( help->getText( ch ).c_str( ), ch );
                return;
            }
        }

        ch->send_to( "Нет подсказки по данному слову.\n\r");
        bugTracker->reportNohelp( ch, origArgument.c_str( ) );
        return;
    }
    
    // Поиск по строке без чисел.
    HelpFinder::ArticleArray articles = HelpFinder(ch, argument).getArticles();
    // No match.
    if (articles.empty()) {
        ch->send_to( "Нет подсказки по данному слову.\n\r");
        bugTracker->reportNohelp( ch, origArgument.c_str( ) );
        return;
    }

    // Exact match - bingo.
    if (articles.size() == 1) {
        page_to_char( articles.front()->getText( ch ).c_str( ), ch );
        return;
    }

    // Several matches, display them all with numbers.
    buf << "По запросу '{C" << origArgument << "{x' найдено несколько разделов справки:" << endl << endl;
    for (unsigned int a = 0; a < articles.size(); a++) 
        buf << "    {C{hh" << (a+1) << "." << origArgument << "{x : " << articles[a]->getKeyword() << endl;
    buf << endl
        << "Для выбора необходимого раздела используй {C? 1." << origArgument << "{x, {C? 2." << origArgument << "{x и так далее." 
        << endl;

    ch->send_to(buf.str().c_str());
}                  


/*-----------------------------------------------------------------
 * cast 'identify', shop item properties
 *----------------------------------------------------------------*/

void lore_fmt_affect( Affect *paf, ostringstream &buf )
{
    int b = paf->bitvector,
        d = paf->duration;

    if (paf->location == APPLY_NONE || paf->modifier == 0)
        return;

    buf << "Изменяет " << apply_flags.message(paf->location ) 
        << " на " << paf->modifier;

    if (d > -1)
        buf << ", в течении " << d << " час" << GET_COUNT(d, "а", "ов", "ов");
    
    buf << endl;

    if (!b)
        return;
    
    switch(paf->where) {
        case TO_AFFECTS:
            buf << "Добавляет аффект " << affect_flags.messages(b ) << endl;
            break;
        case TO_IMMUNE:
            buf << "Добавляет иммунитет к " << imm_flags.messages(b ) << endl;
            break;
        case TO_RESIST:
            buf << "Добавляет сопротивляемость к " << res_flags.messages(b ) << endl;
            break;
        case TO_VULN:
            buf << "Добавляет уязвимость к " << vuln_flags.messages(b ) << endl;
            break;
        case TO_DETECTS:
            buf << "Добавляет обнаружение " << detect_flags.messages(b ) << endl;
            break;
    }
}

void lore_fmt_wear( int type, int wear, ostringstream &buf )
{
    if (type == ITEM_LIGHT) {
        buf << "Используется как освещение" << endl;
        return;
    }
    
    if (wear == -1)
        return;

    if (IS_SET( wear, ITEM_WEAR_FINGER ))
       buf << "Надевается на палец" << endl;        
    if (IS_SET( wear, ITEM_WEAR_NECK ))
       buf << "Надевается на шею" << endl;        
    if (IS_SET( wear, ITEM_WEAR_BODY ))
       buf << "Надевается на тело" << endl;        
    if (IS_SET( wear, ITEM_WEAR_HEAD ))
       buf << "Надевается на голову" << endl;        
    if (IS_SET( wear, ITEM_WEAR_EARS ))
       buf << "Надевается в уши" << endl;        
    if (IS_SET( wear, ITEM_WEAR_FACE ))
       buf << "Надевается на лицо" << endl;        
    if (IS_SET( wear, ITEM_WEAR_FEET ))
       buf << "Надевается на ступни" << endl;        
    if (IS_SET( wear, ITEM_WEAR_LEGS ))
       buf << "Надевается на бедра" << endl;        
    if (IS_SET( wear, ITEM_WEAR_HANDS ))
       buf << "Надевается на руки" << endl;        
    if (IS_SET( wear, ITEM_WEAR_ARMS ))
       buf << "Надевается на плечи" << endl;        
    if (IS_SET( wear, ITEM_WEAR_ABOUT ))
       buf << "Накидывается вокруг тела" << endl;        
    if (IS_SET( wear, ITEM_WEAR_WAIST ))
       buf << "Надевается на талию" << endl;        
    if (IS_SET( wear, ITEM_WEAR_WRIST ))
       buf << "Надевается на запястье" << endl;
    if (IS_SET( wear, ITEM_WEAR_SHIELD ))
       buf << "Используется как щит" << endl;
    if (IS_SET( wear, ITEM_WEAR_HORSE ))
        buf << "Надевается на лошадиную часть" << endl;
    if (IS_SET( wear, ITEM_WEAR_HOOVES ))
        buf << "Надевается на копыта" << endl;
}

void lore_fmt_item( Character *ch, Object *obj, ostringstream &buf, bool showName )
{
    int lim;
    Skill *skill;
    Liquid *liquid;
    const char *mat;
    Affect *paf;
    Keyhole::Pointer keyhole;

    buf << "{W" << obj->getShortDescr( '1' ) << "{x";
    
    if (showName)
        buf << ", откликается на имена '{W" << obj->getName( ) << "{x'";

    buf << endl
        << "{W" << item_table.message(obj->item_type ) << "{x, "
        << "уровня {W" << obj->level << "{x" << endl;

    if (obj->weight > 10)
        buf << "весит {W" << obj->weight / 10 << "{x фун" << GET_COUNT(obj->weight/10, "т", "та", "тов"); 
    else
        buf << "ничего не весит";

    if (IS_SET(obj->extra_flags, ITEM_NOIDENT)) {
        buf << endl << "Более про эту вещь невозможно ничего сказать." << endl;
        return;
    }
    
    buf << ", ";
    
    if (obj->cost)
        buf << "стоит {W" << obj->cost << "{x серебра";
    else
        buf << "ничего не стоит";
   
    // XXX 'изготовлено из' + падежи
    mat = obj->getMaterial( );
    if (mat && strcmp( mat, "none" ) && strcmp( mat, "oldstyle" ))
        buf << ", материал {W" << mat << "{x";
    
    buf << endl;
    
    bitstring_t extra = obj->extra_flags;
    REMOVE_BIT(extra, ITEM_WATER_STAND|ITEM_INVENTORY|ITEM_HAD_TIMER|ITEM_DELETED);
    if (extra)
        buf << "Особые свойства: " << extra_flags.messages(extra, true ) << endl;

    lim = obj->pIndexData->limit;
    if (lim != -1 && lim < 100)
        buf << "Таких вещей в мире может быть не более {W" << lim << "{x!" << endl;

    switch (obj->item_type) {
    case ITEM_KEY:
        if (( keyhole = Keyhole::locate( ch, obj ) ))
            keyhole->doLore( buf );
        break;
    case ITEM_KEYRING:
        buf << "Нанизано " << obj->value[1] << " ключей из возможных " << obj->value[0] << "." << endl;
        break;
    case ITEM_LOCKPICK:
        if (obj->value[0] == Keyhole::LOCK_VALUE_BLANK) {
            buf << "Это заготовка для ключа или отмычки." << endl;
        }
        else {
            if (obj->value[0] == Keyhole::LOCK_VALUE_MULTI)
                buf << "Открывает любой замок. ";
            else
                buf << "Открывает один из видов замков. ";
            
            buf << "Отмычка " 
                << quality_percent( obj->value[1] ).colourStrip( ).ruscase( '2' ) 
                << " качества." << endl;
        }
        break;
    case ITEM_SPELLBOOK:
        buf << "Всего страниц: " << obj->value[0] << ", из них использовано: " << obj->value[1] << "." << endl
            << "Максимальное качество заклинаний в книге: " << obj->value[2] << "." << endl;
        break;

    case ITEM_TEXTBOOK:
        buf << "Всего страниц: " << obj->value[0] << ", из них использовано: " << obj->value[1] << "." << endl
            << "Максимальное качество записей в учебнике: " << obj->value[2] << "." << endl;
        break;

    case ITEM_RECIPE:
        buf << "Сложность рецепта: " << obj->value[2] << ". " 
            << "Применяется для создания " << recipe_flags.messages(obj->value[0], true) << "." << endl;
        break;

    case ITEM_SCROLL:
    case ITEM_POTION:
    case ITEM_PILL:
        buf << "Заклинания " << obj->value[0] << " уровня:";

        for (int i = 1; i <= 4; i++) 
            if (( skill = SkillManager::getThis( )->find( obj->value[i] ) ))
                if (skill->getIndex( ) != gsn_none)
                    buf << " '" << skill->getNameFor( ch ) << "'";
        
        buf << endl;
        break;

    case ITEM_WAND:
    case ITEM_STAFF:
        buf << "Имеет " << obj->value[2] << " заклинани" << GET_COUNT(obj->value[2], "е", "я", "й") << " " 
            << obj->value[0] << " уровня:";
        
        if (( skill = SkillManager::getThis( )->find( obj->value[3] ) ))
            if (skill->getIndex( ) != gsn_none)
                buf << " '" << skill->getNameFor( ch ) << "'";

        buf << endl;
        break;

    case ITEM_DRINK_CON:
        liquid = liquidManager->find( obj->value[2] );
        buf << "Содержит " 
            << liquid->getShortDescr( ).ruscase( '4' ) << " "
            << liquid->getColor( ).ruscase( '2' ) 
            << " цвета." << endl;
        break;

    case ITEM_CONTAINER:
        buf << "Вместительность: " << obj->value[0] << "  "
            << "Максим. вес: " << obj->value[3] << " фун" << GET_COUNT(obj->value[3], "т", "та", "тов") << " ";
        
        if (obj->value[4] != 100)
            buf << " Коэф. снижения веса: " << obj->value[4] << "%";
            
        if (obj->value[1])
            buf << endl << "Особенности: " << container_flags.messages(obj->value[1], true );
        
        buf << endl;
        break;

    case ITEM_WEAPON:
        buf << "Тип оружия: " 
            << weapon_class.message(obj->value[0] ) << " "
            << "(" << weapon_class.name( obj->value[0] ) << "), ";
        
        buf << "повреждения " << obj->value[1] << "d" << obj->value[2] << " "
            << "(среднее " << (1 + obj->value[2]) * obj->value[1] / 2 << ")" << endl;
    
        if (obj->value[4])  /* weapon flags */
            buf << "Особенности оружия: " << weapon_type2.messages(obj->value[4], true ) << endl;

        break;

    case ITEM_ARMOR:
        buf << "Класс брони: ";

        for (int i = 0; i <= 3; i++)
            buf << obj->value[i] << " " << ac_type.message(i )
                << (i == 3 ? "" : ", ");

        buf << endl;
        break;
    }
    
    lore_fmt_wear( obj->item_type, obj->wear_flags, buf );

    if (!obj->enchanted)
        for (paf = obj->pIndexData->affected; paf != 0; paf = paf->next)
            lore_fmt_affect( paf, buf );

    for (paf = obj->affected; paf != 0; paf = paf->next)
        lore_fmt_affect( paf, buf );
}

