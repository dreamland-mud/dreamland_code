#include "pcharacter.h"
#include "npcharacter.h"
#include "room.h"
#include "pcrace.h"
#include "grammar_entities_impl.h"
#include "act.h"
#include "commandtemplate.h"
#include "xmlkillingattribute.h"
#include "player_utils.h"
#include "vnum.h"
#include "xmlattributeticker.h"
#include "alignment.h"
#include "religion.h"
#include "desire.h"
#include "playerattributes.h"
#include "interp.h"
#include "arg_utils.h"
#include "stats_apply.h"
#include "mudtags.h"
#include "string_utils.h"
#include "char_weight.h"
#include "dreamland.h"
#include "merc.h"
#include "def.h"

RELIG(none);
PROF(samurai);

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

    auto killed = ch->getPC()->getAttributes().getAttr<XMLKillingAttribute>("killed");

    ch->pecho("Ты уби%Gло|л|ла {Y%d{x %s, {W%d{x %s и {r%d{x %s персонажей.",
            ch, 
            killed->align[N_ALIGN_GOOD], "добрых",
            killed->align[N_ALIGN_NEUTRAL], "нейтральных",
            killed->align[N_ALIGN_EVIL], "злых");
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
                   ch->is_npc( ) ? "" : Player::title(ch->getPC( )).c_str( ),
                   ch->getRealLevel( ));
    
    if (!ch->is_npc( ))
        buf << fmt( 0, ", тебе %1$d %1$Iгод|года|лет (%2$d ча%2$Iс|са|сов).",
                        pch->age.getYears( ), pch->age.getHours( ) ); 
    
    buf << endl;

    if (ch->getRealLevel( ) != ch->get_trust( ))
        buf << "Уровень доверия к тебе составляет " << ch->get_trust( ) << "." << endl;

    buf << "{wРаса:{W " << ch->getRace( )->getNameFor( ch, ch ).ruscase('1')
    << "  {wРазмер:{W " << size_table.message( ch->size )    
        << "  {wПол:{W " << sex_table.message( ch->getSex( ) )
        << "  {wКласс:{W " << ch->getProfession( )->getNameFor( ch );
    
    if (!ch->is_npc( ))
        room = get_room_instance( ch->getPC()->getHometown( )->getAltar() );
    else
        room = get_room_instance( ROOM_VNUM_TEMPLE );
    
    buf << "  {wДом:{W " << (room ? room->areaName().c_str() : "Потерян" ) << "{x" << endl
        << fmt(0, "У тебя {R%d{x/{r%d{x жизни, {C%d{x/{c%d{x энергии и %d/%d движения.\n\r",
                    ch->hit.getValue( ), ch->max_hit.getValue( ), 
                    ch->mana.getValue( ), ch->max_mana.getValue( ), 
                    ch->move.getValue( ), ch->max_move.getValue( ));
    
    if (!ch->is_npc( )) 
        buf << fmt( 0, "У тебя {Y%1$d{x практи%1$Iка|ки|к и {Y%2$d{x тренировочн%2$Iая|ые|ых сесси%2$Iя|и|й.",
                       pch->practice.getValue( ), pch->train.getValue( ) )
            << endl;
    
    buf << fmt(0, "Ты несешь {W%d/%d{x вещей с весом {W%d/%d{x фунтов.\n\r",
                ch->carry_number, Char::canCarryNumber(ch),
                Char::getCarryWeight(ch)/10, Char::canCarryWeight(ch)/10 );

    if (ch->is_npc( )) {
        buf << fmt(0, 
            "Твои параметры: исходные, (текущие)\n\r"
            "      Сила : %d(%d)    Интеллект : %d(%d)\n\r"
            "  Мудрость : %d(%d)     Ловкость : %d(%d)\n\r"
            "  Сложение : %d(%d)      Обаяние : %d(%d)\n\r",
            ch->perm_stat[STAT_STR], ch->getCurrStat(STAT_STR),
            ch->perm_stat[STAT_INT], ch->getCurrStat(STAT_INT),
            ch->perm_stat[STAT_WIS], ch->getCurrStat(STAT_WIS),
            ch->perm_stat[STAT_DEX], ch->getCurrStat(STAT_DEX),
            ch->perm_stat[STAT_CON], ch->getCurrStat(STAT_CON),
            ch->perm_stat[STAT_CHA], ch->getCurrStat(STAT_CHA) );

    } else {
        buf << fmt(0, 
            "Твои параметры: исходные, {c({Wтекущие{c){x, [{Cмаксимальные{x]\n\r"
            "      Сила: %d{c({W%d{c){x [{C%d{x]   Интеллект: %d{c({W%d{c){x [{C%d{x]\n\r"
            "  Мудрость: %d{c({W%d{c){x [{C%d{x]    Ловкость: %d{c({W%d{c){x [{C%d{x]\n\r"
            "  Сложение: %d{c({W%d{c){x [{C%d{x]     Обаяние: %d{c({W%d{c){x [{C%d{x]\n\r",
            ch->perm_stat[STAT_STR], ch->getCurrStat(STAT_STR), pch->getMaxStat(STAT_STR),
            ch->perm_stat[STAT_INT], ch->getCurrStat(STAT_INT), pch->getMaxStat(STAT_INT),
            ch->perm_stat[STAT_WIS], ch->getCurrStat(STAT_WIS), pch->getMaxStat(STAT_WIS),
            ch->perm_stat[STAT_DEX], ch->getCurrStat(STAT_DEX), pch->getMaxStat(STAT_DEX),
            ch->perm_stat[STAT_CON], ch->getCurrStat(STAT_CON), pch->getMaxStat(STAT_CON),
            ch->perm_stat[STAT_CHA], ch->getCurrStat(STAT_CHA), pch->getMaxStat(STAT_CHA) );

    }

    buf << fmt(0, "У тебя {W%d{x очков опыта, и %s\n\r",
                  ch->exp.getValue( ),
                  show_money( ch->gold, ch->silver ).c_str( ) );

    /* KIO shows exp to level */
    if (!ch->is_npc() && ch->getRealLevel( ) < LEVEL_HERO - 1)
        buf << fmt(0, "Тебе нужно набрать {W%d{x очков опыта до следующего уровня.\n\r",
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
                buf << fmt(0, "Ты попытаешься убежать при %d жизни.  ", ch->wimpy.getValue( ) );
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
            buf << fmt(0, "Ты охраняешь: %s. ", ch->seeName( ch->getPC()->guarding, '4' ).c_str( ) );
            newline = true;
        }

        if (ch->getPC()->guarded_by != 0) {
            buf << fmt(0, "Ты охраняешься: %s.", ch->seeName( ch->getPC()->guarded_by, '5' ).c_str( ) );
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
    buf << fmt(0, "Защита от укола {W%d{x, от удара {W%d{x, от разрезания {W%d{x, от экзотики {W%d{x.\n\r",
            GET_AC(ch,AC_PIERCE),
            GET_AC(ch,AC_BASH),
            GET_AC(ch,AC_SLASH),
            GET_AC(ch,AC_EXOTIC));
    buf << fmt(0, "Точность: {C%d{x  Урон: {C%d{x  Защита от заклинаний: {C%d{x\n\r",
                ch->hitroll.getValue( ), ch->damroll.getValue( ), ch->saving_throw.getValue( ) );

    buf << fmt(0, "У тебя %s натура.  ", align_name( ch ).ruscase( '1' ).c_str( ) );
    
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
            buf << fmt(0, "Ты не определил%Gось|ся|ась с выбором религии.  ", ch);
        else
            buf << fmt(0, "Твоя религия: {C%s{x.  ", ch->getReligion( )->getNameFor( ch ).ruscase( '1' ).c_str( ));
        
        buf << fmt(0, "Твои заслуги перед законом:  %d.\n\r", ch->getPC( )->getLoyalty());

        auto killed = ch->getPC()->getAttributes().getAttr<XMLKillingAttribute>("killed");

       buf << fmt(0, "Ты уби%Gло|л|ла {Y%d{x %s, {W%d{x %s и {r%d{x %s персонажей.\n\r",
                        ch, 
                        killed->align[N_ALIGN_GOOD], "добрых",
                        killed->align[N_ALIGN_NEUTRAL], "нейтральных",
                        killed->align[N_ALIGN_EVIL], "злых");
    }
    
    /* RT wizinvis and holy light */
    if (ch->is_immortal( )) 
        buf << fmt(0, "Божественный взор %s. Невидимость %d уровня, инкогнито %d уровня.",
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
        buf << fmt(0, "{xТы призрак и обретёшь плоть через {Y%1$d {xсекун%1$Iду|ды|д.",
                 pch->ghost_time*(PULSE_MOBILE/dreamland->getPulsePerSecond()))
        << endl;
    }

    ch->send_to( buf );

    if (IS_SET(ch->comm, COMM_SHOW_AFFECTS))
        interpret_raw( ch, "affects", "nocolor noempty" );
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
   
    if (arg_is(arg, "hp")) {
        ch->pecho("Здоровье %d из %d.", ch->hit, ch->max_hit);
        return;
    } 
    if (arg_is(arg, "mana")) {
        ch->pecho("Мана %d из %d.", ch->mana, ch->max_mana);
        return;
    } 
    if (arg_is(arg, "moves")) {
        ch->pecho("Шагов %d из %d.", ch->move, ch->max_move);
        return;
    } 
    if (arg_is(arg, "level")) {
        ch->pecho("Уровень %d.", ch->getRealLevel());
        return;
    } 
    if (arg_is(arg, "race")) {
        if (ch->getRace()->isPC()) {
            PCRace::Pointer pcRace = ch->getRace()->getPC(); 
            ch->pecho("Ты %N1.", GET_SEX(ch,
                            pcRace->getMaleName().c_str(),
                            pcRace->getMaleName().c_str(),
                            pcRace->getFemaleName().c_str()));
        }
        return;
    } 
    if (arg_is(arg, "sex")) {   
        ch->pecho("Пол %s.", GET_SEX(ch, "мужской", "потерян", "женский"));
        return;
    }
    if (arg_is(arg, "class")) {
        ch->pecho("Ты %N1.", ch->getProfession()->getRusName().c_str());
        return;
    } 
    if (arg_is(arg, "align")) {
        ch->pecho("У тебя %s натура.", align_name_short(ch, Grammar::MultiGender::FEMININE));
        return;
    } 
    if (arg_is(arg, "ethos")) {
        ch->pecho("У тебя %s этос.", ethos_table.message(ch->ethos, '1').c_str());
        return;
    } 
    if (arg_is(arg, "hometown")) {
        Room *room = get_room_instance(pch->getHometown()->getAltar());
        ch->pecho("Твой дом - %s.", room ? room->areaName().c_str() : "потерян");
        return;
    } 
    if (arg_is(arg, "religion")) {
        if (ch->getReligion() == god_none)
            ch->pecho("Ты не определил%Gось|ся|ась с выбором религии.", ch);
        else
            ch->pecho("Религия %s.", ch->getReligion()->getRussianName().ruscase('1').c_str());
        return;
    } 
    if (arg_is(arg, "practice")) {
        ch->pecho("Практик %d.", pch->practice);
        return;
    } 
    if (arg_is(arg, "train")) {
        ch->pecho("Тренировки %d.", pch->train);
        return;
    } 
    if (!str_prefix("quest", arg.c_str()) || !str_prefix("квест", arg.c_str())) {
        ch->pecho("Используй команды {y{hcквест время{x и {y{hcквест очки{x.");
        return;
    } 
    if (arg_is(arg, "wimpy")) {
        ch->pecho("Трусость %d.", ch->wimpy);
        return;
    } 
    if (arg_is(arg, "death")) {
        ch->pecho("Смертей %d.", pch->death);
        return;
    } 
    if (arg_is(arg, "position")) {
        ch->pecho(msgtable_lookup(msg_positions, ch->position));
        return;
    }
    if (arg_is(arg, "gold")) {
        ch->pecho("Золота %d.", ch->gold);
        return;
    } 
    if (arg_is(arg, "silver")) {
        ch->pecho("Серебра %d.", ch->silver);
        return;
    } 
    if (arg_is(arg, "weight")) {
        ch->pecho("Вес %d из %d.", Char::getCarryWeight(ch)/10, Char::canCarryWeight(ch)/10);
        return;
    } 
    if (arg_is(arg, "items")) {
        ch->pecho("Вещи %d из %d.", ch->carry_number, Char::canCarryNumber(ch));
        return;
    } 
    if (arg_is(arg, "exp")) {
        ch->pecho("Опыта до уровня %d.", pch->getExpToLevel());
        return;
    }
    if (arg_is(arg, "age")) {
        ch->pecho("Возраст %d.", pch->age.getYears());
        return;
    }

    if (arg_is(arg, "hr")) {
        ch->pecho("Точность %d.", ch->hitroll);
        return;
    } 
    if (arg_is(arg, "dr")) {
        ch->pecho("Урон %d.", ch->damroll);
        return;
    } 
    if (arg_is(arg, "ac")) {
        ch->pecho("Защита от уколов %d, ударов %d, разрезов %d, экзотики %d.", 
                    GET_AC(ch, AC_PIERCE), GET_AC(ch, AC_BASH),
                    GET_AC(ch, AC_SLASH), GET_AC(ch, AC_EXOTIC));
        return;
    }
    if (arg_is(arg, "saves")) {
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
    DLString title = Player::title(pch);
    name << ch->seeName( ch, '1' ) << "{x ";
    mudtags_convert(title.c_str( ), name, TAGS_CONVERT_VIS, ch);

    DLString ethos = ethos_table.message( ch->ethos, '1' );

    // Output one piece of the score if there is an argument provided.
    DLString arg = argument;
    if (!arg.empty()) {
        do_score_args(ch, arg);
        return;
    }

    ch->pecho( 
"%s\n\r"
"      /~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~/~~\\", 
             CLR_FRAME);
    ch->pecho(
        fmt ( 0, "     %s|   %s%-50.50s {y%3d{x %4s   %s|____|",
                CLR_FRAME,
                CLR_CAPT,
                name.str( ).c_str( ),
                age,
                GET_COUNT(age, "год", "года", "лет"),
                CLR_FRAME ) );
                
        
    ch->pecho(
"     %s|%s+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+%s|\n\r" 
"     | %sУровень:{x  %3d        %s|%s Сила:{x %2d{c({x%2d{c){x {C%2d{x %s| %sРелигия:{x %-14.14s%s|\n\r"
"     | %sРаса :{x  %-12s %s| %sУм  :{x %2d{c({x%2d{c){x {C%2d{x %s| %sПрактик   :{x   %3d      %s|\n\r"
"     | %sПол  :{x  %-11s  %s| %sМудр:{x %2d{c({x%2d{c){x {C%2d{x %s| %sТренировок:{x   %3d      %s|\n\r"
"     | %sКласс:{x  %-13s%s| %sЛовк:{x %2d{c({x%2d{c){x {C%2d{x %s| %sКвест. единиц:{x  %-6d%s |\n\r"
"     | %sНатура:{x %-11s  %s| %sСлож:{x %2d{c({x%2d{c){x {C%2d{x %s| %sКвест. время:{x   %-3d %s   |\n\r"
"     | %sЭтос :{x  %-12s %s| %sОбая:{x %2d{c({x%2d{c){x {C%2d{x %s| %s%s :{x   %3d      %s|",

            CLR_FRAME, CLR_BAR, CLR_FRAME,

            CLR_CAPT,
            ch->getRealLevel( ),
            CLR_BAR,
            CLR_CAPT,
            ch->perm_stat[STAT_STR], ch->getCurrStat(STAT_STR), pch->getMaxStat(STAT_STR),
            CLR_BAR,
            CLR_CAPT,
            (ch->getReligion() == god_none ? "не определена": ch->getReligion( )->getNameFor( ch ).ruscase( '1' ).c_str( )),
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
            String::truncate(ethos, 12).c_str( ),
            CLR_BAR,
            CLR_CAPT,
            ch->perm_stat[STAT_CHA], ch->getCurrStat(STAT_CHA), pch->getMaxStat(STAT_CHA),
            CLR_BAR,
            CLR_CAPT,
            ch->getProfession( ) == prof_samurai 
                ?  "Смертей  " : "Трусость " ,
            ch->getProfession( ) == prof_samurai 
                ? pch->death.getValue( ) : ch->wimpy.getValue( ),
            CLR_FRAME);

        ch->pecho(
            fmt ( 0, "     %s| %sДом  :{x  %-30.30s %s| {Y%-22s %s|",
                CLR_FRAME,
                CLR_CAPT,
                room ? room->areaName().c_str() : "Потерян",
                CLR_BAR,
                msgtable_lookup( msg_positions, ch->position ),
                CLR_FRAME,
                CLR_BAR, CLR_FRAME) );

    ch->pecho(
"     |%s+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+%s|",
        CLR_BAR, CLR_FRAME);
            
    if (pch->guarding != 0) {
        ekle = 1;
        ch->pecho( 
"     %s| {wТы охраняешь    :{Y %-10s                                    %s|",
            CLR_FRAME,
            ch->seeName( pch->guarding, '4' ).c_str(),
            CLR_FRAME);
    }

    if (pch->guarded_by != 0) {
        ekle = 1;
        ch->pecho( 
"     %s| {wТебя охраняет     :{Y %-10s                                  %s|",
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
                ch->pecho( "     %s| {w%-64s%s|", 
                            CLR_FRAME,
                            buf.str( ).c_str( ),
                            CLR_FRAME );
            }
        }
    }

    if (ch->is_adrenalined()) {
        ekle = 1;
        ch->pecho( 
"     %s| {yАдреналин кипит в твоих венах!                                  %s|",
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
        ch->pecho( 
"     %s| {wНевидимость: уровня %3d   "
         "Инкогнито: уровня %3d                 %s|",
              CLR_FRAME,
              pch->invis_level.getValue( ),
              pch->incog_level.getValue( ),
              CLR_FRAME);
    }

    list<DLString> attrLines;
    if (ch->getPC()->getAttributes( ).handleEvent( ScoreArguments( ch->getPC(), attrLines ) )) {
        ekle = 1;
        for (list<DLString>::iterator l = attrLines.begin( ); l != attrLines.end( ); l++) {
            ch->pecho("     %s| {w%-64s%s|", 
                        CLR_FRAME,
                        l->c_str(),
                        CLR_FRAME);
        }
    }

    if (ekle) {
        ch->pecho( 
"     %s|%s+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+%s|",
                CLR_FRAME,
                CLR_BAR,
                CLR_FRAME);
    }


    ch->pecho( 
"     %s| %sВещи          :{x     %3d/%-4d        %sЗащита от уколов:{x   %-5d   %s|\n\r"
"     | %sВес           :{x  %6d/%-8d    %sЗащита от ударов:{x   %-5d   %s|\n\r"
"     | %sЗолото        :{Y %-10d          %sЗащита от разрезов:{x %-5d   %s|\n\r"
"     | %sСеребро       :{W %-10d          %sЗащита от экзотики:{x %-5d   %s|\n\r"
"     | %sЕдиниц опыта  :{x %-6d              %sЗащита от заклинаний:{x %4d  %s|",
        CLR_FRAME,
        CLR_CAPT,
        ch->carry_number, Char::canCarryNumber(ch),
        CLR_CAPT,
        GET_AC(ch,AC_PIERCE),
        CLR_FRAME,

        CLR_CAPT,
        Char::getCarryWeight(ch)/10, Char::canCarryWeight(ch)/10,
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

    ch->pecho( 
"     %s| %sОпыта до уровня:{x %-6d                                         %s|\n\r"
"     |                                    %sЖизни:{x %5d / %5d         %s|",
        CLR_FRAME,
        CLR_CAPT,
        pch->getExpToLevel( ),
        CLR_FRAME,

        CLR_CAPT,
        ch->hit.getValue( ), ch->max_hit.getValue( ),
        CLR_FRAME);

    ch->pecho( 
"     %s| %sТочность      :{x   %-3d            %sЭнергии:{x %5d / %5d         %s|\n\r"
"     | %sУрон          :{x   %-3d           %sДвижения:{x %5d / %5d         %s|",
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


    ch->pecho( 
"  %s/~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~/   |\n\r"
"  \\________________________________________________________________\\__/{x",
        CLR_FRAME);

    if (IS_SET(ch->comm, COMM_SHOW_AFFECTS))
        interpret_raw( ch, "affects", "noempty");
}


