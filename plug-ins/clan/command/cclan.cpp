/* $Id: cclan.cpp,v 1.1.6.15.4.15 2010-09-01 21:20:44 rufina Exp $
 *
 * ruffina, 2004
 * based on CClan by NoFate, 2001
 */

#include <sstream>
#include <iomanip>
#include <iostream>

#include "class.h"
#include "logstream.h"
#include "pcharactermemory.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "object.h"
#include "race.h"

#include "merc.h"
#include "descriptor.h"
#include "clanreference.h"
#include "gsn_plugin.h"
#include "handler.h"
#include "infonet.h"
#include "messengers.h"
#include "act.h"
#include "mercdb.h"

#include "clantypes.h"
#include "clantitles.h"
#include "clanorg.h"
#include "cclan.h"
#include "xmlattributeinduct.h"

#include "def.h"

CLAN(none);
CLAN(outsider);

using namespace std;

#define OBJ_VNUM_DIAMOND          3377

enum {
    CB_MODE_DEPOSIT  = 1,
    CB_MODE_WITHDRAW = 2,
};

enum {
    CB_CURR_QP      = 1,
    CB_CURR_GOLD    = 2,
    CB_CURR_SILVER  = 3,
    CB_CURR_DIAMOND = 4,
};

struct clan_diplomacy_names {
  const char *eng_name;
  const char *abbr;
  const char *color;
  const char *long_name;
};

struct clan_diplomacy_names clan_diplomacy_names_table[] =
{
  {"aliance",            "аль.", "{W",    "альянс"            },
  {"peace",            "мир ", "{G",    "мир"            },
  {"truce",            "пер.", "{Y",    "перемирие"    },
  {"distrust",            "нед.", "{B",    "недоверие"    },
  {"aggresive",            "агр.", "{r",    "агрессия"            },
  {"war",            "вой.", "{R",    "война"            },
  {"subordination", "под.", "{Y",    "подчинение"   },
  {"oppress",            "угн.", "{Y",    "угнетение"    },
};

const int clan_diplomacy_max = 5;

COMMAND(CClan, "clan")
{
    PCharacter *pc = ch->getPC( );

    if (!pc)
        return;
    
    if (IS_CHARMED(pc)) {
        if (pc->master)
            pc->master->send_to( "Ничего не выйдет.\r\n" );

        pc->send_to( "..но ничего не происходит.\r\n" );
        return;
    }

    if (constArguments.length( ) == 0) {
        clanList( pc );
    }
    else {
        DLString argument = constArguments;
        DLString argumentOne = argument.getOneArgument( );
        
        if (arg_is_list( argumentOne ) || arg_oneof( argumentOne, "info", "инфо" ))
            clanList( pc );
        else if (arg_oneof( argumentOne, "count", "счет", "количество" )) 
            clanCount( pc );
        else if( arg_oneof( argumentOne, "bank", "банк" ) )
            clanBank( pc, argument );
        else if( arg_oneof( argumentOne, "remove", "выгнать", "уйти" ) )
            clanRemove( pc, argument );
        else if( arg_oneof( argumentOne, "level", "уровень" ) )
            clanLevel( pc, argument );
        else if( arg_oneof( argumentOne, "member", "члены", "состав" ) )
            clanMember( pc, argument );
        else if( arg_oneof( argumentOne, "petition", "петиция", "прошение" ) )
            clanPetition( pc, argument );
        else if( arg_oneof( argumentOne, "diplomacy", "дипломатия" ) )
            clanDiplomacy( pc, argument );
        else if (pc->is_immortal( )) {
            if (arg_oneof( argumentOne, "rating", "рейтинг" ))
                clanRating( pc );
            else if(arg_oneof( argumentOne, "status", "статус" ))
                clanStatus( pc );
            else if (arg_oneof( argumentOne, "scan", "осмотр" ))
                clanScan( pc );
            else if (arg_oneof( argumentOne, "induct", "принять" ))
                clanInduct( pc, argument );
            else
                usage( pc );
        }
        else
            usage( pc );
    }        
}

void CClan::usage( PCharacter *pc )
{    
    basic_ostringstream<char> buf;

    buf << "{W{lRклан список{lEclan list  {lx{x     показать список всех кланов" << endl
        << "{W{lRклан счет {lEclan count{lx{x      показать количество игроков в кланах" << endl
        << "{W{lRклан банк{lEclan bank{lx{x       операции с клановым банком (подробее см. {W{lRклан банк помощь{lEclan bank help{lx{x)" << endl
        << "{W{lRклан выгнать{lEclan remove {lx{x    выйти (выгнать кого-либо) из клана (см. {W{lRклан выгнать помощь{lEclan remove help{lx{x)" << endl
        << "{W{lRклан уровень{lEclan level  {lx{x    посмотреть/установить клановый уровень (cм. {W{lRклан уровень помощь{lEclan level help{lx{x)" << endl
        << "{W{lRклан состав{lEclan member{lx{x     показывает лидеру список членов клана (см. {W{lRклан состав помощь{lEclan member help{lx{x)" << endl
        << "{W{lRклан петиция {lEclan petition{lx{x   написать/принять/отклонить петицию на вступление в клан" << endl
        << "                (см. {W{lRклан петиция помощь{lEclan petition help{lx{x)" << endl           
        << "{W{lRклан дипломатия{lEclan diplomacy {lx{x посмотреть/установить клановую дипломатию (см. {W{lRклан дипломатия помощь{lEclan dipl help{lx{x)" << endl;        

    if (pc->is_immortal( ))
        buf << "{W{lRклан принять{lEclan induct {lx{x    принять кого-то в клан" << endl
            << "{W{lRклан статус{lEclan status{lx{x     показать статистику побед/поражений по уровням" << endl
            << "{W{lRклан рейтинг{lEclan rating {lx{x    рейтинг клана согласно статистике побед/поражений" << endl;
    
    pc->send_to( buf );
}

/*
 * clan
 * clan list
 */
void CClan::clanList( PCharacter* pc )
{
    pc->send_to( "В мире есть такие кланы:\n\r" );                              
        
    for (int i = 0; i < ClanManager::getThis( )->size( ); i++) {
        Clan *clan = ClanManager::getThis( )->find( i );
        
        if (!clan->isHidden( )) {
            basic_ostringstream<char> buf;                                          
            buf << setw( 33 ) << clan->getLongName( ) << " [{"
                << clan->getColor( ) << clan->getPaddedName( ) 
                << "{x]" << endl;
            pc->send_to( buf );
        }
    }

    pc->send_to( "\n\rПодробнее смотри команду {lRклан ?{lEclan ?{lx{x.\n\r" );                              
}

/*
 * clan count
 */ 
void CClan::clanCount( PCharacter* pc )
{
    vector<int> counts;
    PCharacterMemoryList::const_iterator pos;
    const PCharacterMemoryList& list = PCharacterManager::getPCM( );
    ClanManager *cm = ClanManager::getThis( );
    
    counts.resize( cm->size( ) );
    
    for (int i = 0; i < cm->size( ); i++)
        counts[i] = 0;

    for (pos = list.begin( ); pos != list.end( ); pos++) {
        PCMemoryInterface *pcm = pos->second;
        
        if (pcm->getLevel( ) < 102 && !pcm->getClan( )->isHidden( ))
            counts[pcm->getClan( )]++;
    }
    
    pc->send_to( "      Клан         кол.\n\r" );                               

    for (int i = 0; i < cm->size( ); i++) {
        Clan *clan = cm->find( i );
        
        if (!clan->isHidden( )) {
            basic_ostringstream<char> buf;
            buf << "  [{" << clan->getColor( )
                << clan->getPaddedName( ) << "{x] "
                << setw( 5 ) << counts[i] << endl;
            pc->send_to( buf );
        }
    }
}

/*
 * clan rating
 */ 
void CClan::clanRating( PCharacter* pc )
{
    ClanManager *cm = ClanManager::getThis( );

    pc->send_to( "Клан      Рейтинг\n\r" );                                     
    
    for (int i = 0; i < cm->size( ); i++) {
        Clan *clan = cm->find( i );
        
        if (!clan->isHidden( ) && clan->getData( )) {
            basic_ostringstream<char> buf;                                          
            buf << "  [{" << clan->getColor( ) << clan->getPaddedName( ) 
                << "{x] " << setw( 5 ) << clan->getData( )->rating << endl;
            pc->send_to( buf );
        }
    }
}

/* 
 * clan status
 */ 
void CClan::clanStatus( PCharacter* pc )
{
    ClanManager *cm = ClanManager::getThis( );

    pc->send_to( "      {BКлан        ...20        21-40       41-60       61-80       81...{x\r\n" );

    for (int i = 0; i < cm->size( ); i++) {
        basic_ostringstream<char> buf;                                          
        Clan *clan = cm->find( i );
        ClanData * cd = clan->getData( );
            
        if (clan->isHidden( ) || !cd)
            continue;
        
        buf << "  [{" << clan->getColor( ) << clan->getPaddedName( ) << "{x]{C";
        
        for (int j = 0; j < 5; j++)
            buf << " " << setw( 5 ) << cd->victory[j] 
                << "{W/{C" << setiosflags( ios::left ) << setw( 5 ) 
                << cd->defeat[j] << resetiosflags( ios::left );

        buf << "{x" << endl;
        pc->send_to( buf );
    }
}

/*
 * clan bank <dep|wit> <amnt> <qp|go|si|di> [[clan <clan>]|[char <char>]]
 */ 
void CClan::clanBank( PCharacter* pc, DLString& argument )
{
    ostringstream buf;
    Clan *acc_clan = NULL, *clan = NULL;
    long amount = 0;
    int mode = 0;
    int currency = 0;
    PCharacter *victim = 0;
    Character *vch = 0;
    ClanManager *cm = ClanManager::getThis( );

    DLString argumentOne = argument.getOneArgument( );
    
    if (arg_is_help( argumentOne )) {
        clanBankHelp( pc );
        return;
    }
    
    if ((!pc->getClan( )->getData( ) || !pc->getClan( )->getData( )->getBank( ))
        && !pc->is_immortal( )) 
    {
        pc->send_to ("У тебя нет кланового банка!\n\r");
        return;
    }

    if (argumentOne.empty( )) // Checking status
    {
        bool fAll = pc->is_immortal( );

        pc->send_to( "{g\t\t  Состояние банка твоего клана{x.\n\r\n\r" );
        pc->send_to( "Клан            |{BКвестовых единиц{x|{YЗолотых монет{x|{WСеребряных монет{x|{CБриллиантов{x|\n\r" );

        for (int i = 0; i < cm->size( ); i++) {
            clan = cm->find( i );
            
            if (!clan->isValid( ) || !clan->getData( ))
                continue;

            ClanBank::Pointer bank = clan->getData( )->getBank( );
            
            if (!bank || (pc->getClan( ) != clan && !fAll))
                continue;

            pc->printf( "{%s%-16s{x|%16ld|%13ld|",
                    clan->getColor( ).c_str( ), clan->getShortName( ).c_str( ),
                    bank->questpoints.getValue( ),
                    bank->gold.getValue( ));
            pc->printf( "%16ld|%11ld|\n",
                    bank->silver.getValue( ),
                    bank->diamonds.getValue( ) );
        }

        return;
    }

    if (pc->is_immortal( )) {
        clan = cm->findUnstrict( argumentOne );
        
        if (!clan) {
            pc->send_to("Такого клана НЕ СУЩЕСТВУЕТ!\n\r");
            return;
        }
        
        if (!clan->getData( ) || !clan->getData( )->getBank( )) {
            pc->send_to( "У этого клана нет банка!\r\n" );
            return;
        }
        
        argumentOne = argument.getOneArgument( );

        if (argumentOne.empty( )) {
            clanBankHelp( pc );
            return;
        }
    }
    else
        clan = &*pc->getClan( );

    if (arg_oneof( argumentOne, "deposit", "насчет", "положить" ))    
        mode = CB_MODE_DEPOSIT;
    else if (arg_oneof( argumentOne, "withdraw", "сосчета", "снять" )) 
        mode = CB_MODE_WITHDRAW;
    else {
        pc->send_to( "Можно задать только режим {lR'положить'{lEdeposit{lx или {lR'снять'{lEwithdraw{lx.\n\r" );
        return;
    }

    argumentOne = argument.getOneArgument( );
    
    if (argumentOne.empty( ) || !argumentOne.isNumber( )) {
        pc->send_to( "Укажите сумму перевода.\r\n" );
        return;
    }
    
    try {
        amount = argumentOne.toInt( );
    } catch (const ExceptionBadType& e) {
        pc->send_to("Сумма перевода задана неправильно!\n\r");
        return;
    }

    if (amount <= 0) {
        pc->send_to("Сумма должна быть больше нуля.\n\r");
        return;
    }

    argumentOne = argument.getOneArgument( );
    
    if (argumentOne.empty( )) {
        pc->send_to( "Укажите валютную единицу ({lRкп, золото, серебро, бриллианты{lEqp, gold, silver, diamond{lx).\r\n" );
        return;
    }
    
    if (arg_oneof( argumentOne, "qp", "кп" ))    
        currency = CB_CURR_QP;
    else if (arg_oneof( argumentOne, "gold", "золото" )) 
        currency = CB_CURR_GOLD;
    else if (arg_oneof( argumentOne, "silver", "серебро" )) 
        currency = CB_CURR_SILVER;
    else if (arg_oneof( argumentOne, "diamond", "бриллианты" )) 
        currency = CB_CURR_DIAMOND;
    else
    {
        pc->send_to( "Кланбанк оперирует только с {lRкп, золото, серебро, бриллианты{lEqp, gold, silver, diamond{lx.\n\r" );
        return;
    }

    if (mode == CB_MODE_DEPOSIT) {
        if (!clanBankDeposit( pc, clan, currency, amount, buf )) 
            pc->println( "Это больше, чем ты имеешь." );
        else {
            pc->send_to( buf );
            clan->getData( )->save( );
            pc->save( );
        }

        return;
    }

    if (!pc->is_immortal() && !clan->isRecruiter( pc ))
    {
        pc->send_to( "Ты не можешь сделать этого - дорасти сначала до Рекруитера.\n\r" );
        return;
    }

    argumentOne = argument.getOneArgument( );

    if (!argumentOne.empty( )) // Has a destination
    {
        if (arg_oneof( argumentOne, "clan", "клану" )) {
            argumentOne = argument.getOneArgument( );
            acc_clan = cm->findUnstrict( argumentOne );

            if (!acc_clan) {
                pc->send_to("Клана-получателя НЕ СУЩЕСТВУЕТ !\n\r");
                return;
            }
            
            if (!acc_clan->getData( ) || !acc_clan->getData( )->getBank( )) {
                pc->send_to("У клана-получателя НЕТ БАНКА!\n\r");
                return;
            }
        }
        else if (arg_oneof( argumentOne, "character", "персонажу" )) {
            argumentOne = argument.getOneArgument( );
            vch = get_char_world( pc, argumentOne.c_str( ) );

            if (!vch || !(victim = vch->getPC( ))) {
                pc->send_to( "Игрока-получателя нет в мире.\n\r" );
                return;
            }
        }
        else
        {
            pc->send_to( "Непонятно, чего ты хочешь?\n\r" );
            return;
        }
    }

    if (!acc_clan)
        acc_clan = clan;

    if (!victim)
        victim = pc;

    if (currency != CB_CURR_QP && (clan != acc_clan || pc != victim )) {
        pc->send_to( "Взаиморасчеты между кланами (или игроками) осуществляются в QP\n\r" );
        return;
    }
                
    if (currency == CB_CURR_QP && victim && pc->getClan() != victim->getClan())
    {
        pc->send_to ("Ты можешь отдать QP или какому-либо клану, или своему соклановику.\n\r");
        return;
    }
    
    if (!clanBankWithdraw( pc, victim, clan, acc_clan, currency, amount, buf )) {
        pc->send_to( "В банке твоего клана столько не будет..\n\r" );
        return;
    }

    pc->send_to( buf );
    clan->getData( )->save( );

    if (clan != acc_clan)
        acc_clan->getData( )->save( );

    pc->save( );

    if (pc != victim)
        victim->save( );
}

static bool obj_is_diamond( Object *obj )
{
    if (obj->pIndexData->vnum == OBJ_VNUM_DIAMOND
        && obj->wear_loc == wear_none
        && obj->getRealName( ) == 0
        && obj->getRealShortDescr( ) == 0
        && obj->getRealDescription( ) == 0)
        
        return true;
    else
        return false;
}

/*
 * clan bank deposit <amount> <currency>
 */
bool CClan::clanBankDeposit( PCharacter *pc, Clan *acc_clan,
                             int currency, int amount, ostringstream &buf )
{
    switch (currency) {
    case CB_CURR_QP:
        if (!pc->is_immortal( )) {
            if (pc->getQuestPoints() < amount)
                return false;

            pc->addQuestPoints(-amount);
        }

        buf << "На банковский счет клана {" 
            << acc_clan->getColor( ) << acc_clan->getShortName( ) 
            << "{x переведено: " << amount << " квестов"
            << GET_COUNT(amount,"ая единица","ые единицы","ых единиц")
            << "." << endl;
        
        acc_clan->getData( )->getBank( )->questpoints += amount;
        return true;

    case CB_CURR_GOLD:
        if (!pc->is_immortal( )) {
            if (pc->gold < amount)
                return false;

            pc->gold -= amount;
        }
        
        buf << "На банковский счет клана {" 
            << acc_clan->getColor( ) << acc_clan->getShortName( ) 
            << "{x переведено: " << amount << " золот"
            << GET_COUNT(amount,"ая монета","ые монеты","ых монеты")
            << "." << endl;
        
        acc_clan->getData( )->getBank( )->gold += amount;
        return true;

    case CB_CURR_SILVER:
        if (!pc->is_immortal( )) {
            if (pc->silver < amount)
                return false;

            pc->silver -= amount;
        }

        buf << "На банковский счет клана {" 
            << acc_clan->getColor( ) << acc_clan->getShortName( ) 
            << "{x переведено: " << amount << " серебрян"
            << GET_COUNT(amount,"ая монета","ые монеты","ых монеты")
            << "." << endl;
        
        acc_clan->getData( )->getBank( )->silver += amount;
        return true;

    case CB_CURR_DIAMOND:
        if (!pc->is_immortal( )) {
            Object *obj, *obj_next;
            int count = 0;

            for (obj = pc->carrying; obj; obj = obj->next_content)
                if (obj_is_diamond( obj ))
                    count++;

            if (count < amount)
                return false;
            
            for (obj = pc->carrying; obj && count > 0; obj = obj_next) {
                obj_next = obj->next_content;
                
                if (obj_is_diamond( obj )) {
                    extract_obj( obj );
                    count--;
                }
            }

        }

        buf << "На банковский счет клана {" 
            << acc_clan->getColor( ) << acc_clan->getShortName( ) 
            << "{x переведено: " << amount << " бриллиант"
            << GET_COUNT(amount,"","а","ов")
            << "." << endl;
        
        acc_clan->getData( )->getBank( )->diamonds += amount;
        return true;

    default:
        return false;
    }
}

/*
 * clan bank withdraw <amount> <currency> [clan <clan>|char <char>]
 */
bool CClan::clanBankWithdraw( PCharacter *pc, PCharacter *victim,
                              Clan *clan, Clan *acc_clan,
                              int currency, int amount, ostringstream &buf )
{
    ClanBank::Pointer bank = clan->getData( )->getBank( ), 
                      acc_bank = acc_clan->getData( )->getBank( );

    switch (currency) {
    case CB_CURR_QP:
        if (bank->questpoints < amount)
            return false;
        
        bank->questpoints -= amount;

        if (acc_clan && acc_clan != clan) {
            buf << "На банковский счет клана {"
                << acc_clan->getColor( ) << acc_clan->getShortName( ) <<"{x "
                << "переведено: " << amount << " квестов"
                << GET_COUNT(amount,"ая единица","ые единицы","ых единиц")
                << " со счета твоего клана." << endl;
            
            acc_bank->questpoints += amount;
        }
        else if (victim && pc != victim)
        {
            buf << fmt( pc, "Для %1$#^C2 переведено: %2$d квестов%2$Iая|ые|ых едини%2$Iца|цы|ц со счета твоего клана.",
                        victim, amount )
                << endl;

            if (!victim->is_immortal( ))
                victim->addQuestPoints(amount);
        }
        else
        {
            buf << "Ты снимаешь " << amount << " квестов"
                << GET_COUNT(amount,"ую единицу","ые единицы","ых единиц")
                << " со счета клана." << endl;

            if (!victim->is_immortal( ))
                victim->addQuestPoints(amount);
        }

        return true;

    case CB_CURR_GOLD:
        if (bank->gold < amount)
            return false;

        bank->gold -= amount;
        
        buf << "Ты снимаешь " << amount << " золот"
            << GET_COUNT(amount,"ую монету","ые монеты","ых монет")
            << " со счета клана." << endl;

        if (!pc->is_immortal( ))
            pc->gold += amount;

        return true;

    case CB_CURR_SILVER:
        if (bank->silver < amount)
            return false;

        bank->silver -= amount;

        buf << "Ты снимаешь " << amount << " серебрян"
            << GET_COUNT(amount,"ую монету","ые монеты","ых монет")
            << " со счета клана." << endl;

        if (!pc->is_immortal( ))
            pc->silver += amount;

        return true;
        
    case CB_CURR_DIAMOND:
        if (bank->diamonds < amount)
            return false;

        bank->diamonds -= amount;

        buf << "Ты снимаешь " << amount << " бриллиан"
            << GET_COUNT(amount,"т","та","тов")
            << " со счета клана." << endl;

        if (!pc->is_immortal( )) 
            for (int i = 0; i < amount; i++)
                obj_to_char( create_object( get_obj_index(OBJ_VNUM_DIAMOND), 0 ),
                             pc );
        
        return true;

    default:
        return false;
    }
}

/*
 * clan bank help
 */
void CClan::clanBankHelp( PCharacter *pc )
{
    basic_ostringstream<char> buf;
    
    buf << "{W{lRклан банк положить{lEclan bank deposit{lx{x <кол-во> {W{lRкп{lEqp{lx{x|{W{lRзолото{lEgold{lx{x|{W{lRсеребро{lEsilver{lx{x|{W{lRбриллианты{lEdiamonds{lx{x" << endl
        << "          - положить деньги(qp, бриллианты..) в свой кланбанк" << endl
        << endl
        << "Для лидеров:" << endl
        << "{W{lRклан банк снять{lEclan bank withdraw{lx {x<кол-во> {W{lRкп{lEqp{lx{x|{W{lRзолото{lEgold{lx{x|{W{lRсеребро{lEsilver{lx{x|{W{lRбриллианты{lEdiamonds{lx{x" << endl
        << "          - снять деньги(qp) со счета своего кланбанка" << endl
        << "{W{lRклан банк снять{lEclan bank withdraw{lx {x<кол-во> {W{lRкп{lEqp{lx {lRклану{lEclan{lx {x<клан>" << endl
        << "          - перевести qp на кланбанк другого клана" << endl
        << "{W{lRклан банк снять{lEclan bank withdraw{lx {x<кол-во> {W{lRкп{lEqp{lx {lRперсонажу{lEchar{lx {x<имя>" << endl
        << "          - отдать qp с кланбанка соклановику" << endl;
    
    if (pc->is_immortal( ))
        buf << endl
            << "Для Бессмертных: обязательно указывать имя клана, т.е." << endl
            << "{W{lRклан банк{lEclan bank{lx {x<клан> {W{lRположить{lEdeposit{lx{x|{W{lRснять{lEwithdraw{lx{x ..." << endl;

    pc->send_to( buf );
}

/*
 * clan remove <victim>
 */ 
void CClan::clanRemove( PCharacter* pc, DLString& argument )
{
    basic_ostringstream<char> buf;
    PCMemoryInterface* victim;
    XMLAttributeInduct::Pointer attr; 
    ClanMembership *member;
    DLString argumentOne = argument.getOneArgument( );
    
    if (arg_is_help( argumentOne )) {
        clanRemoveHelp( pc );
        return;
    }
    
    if (arg_is_self( argumentOne )) 
        argumentOne = pc->getName( );

    victim = PCharacterManager::find( argumentOne );
    if (!victim) {
        pc->send_to( "Игрок с таким именем не найден.\r\n" );
        return;
    }
    
    Clan &clan = *victim->getClan( );
    member = clan.getMembership( );

    if (pc == victim) {
        if (!member) {
            pc->send_to( "А откуда еще тебе хотелось бы уйти?\n\r" );
            return;
        }
        
        if (!pc->is_immortal( )) 
            if (!member->removable) {
                pc->println( "Из твоего клана невозможно уйти по собственной воле." );
                return;
            }

        buf << "Ты уходишь из клана [{" 
            << clan.getColor( ) << clan.getShortName( ) << "{x].";
                
        pc->setClan( member->removeSelf );

    } else {
        if (!pc->is_immortal() && !clan.isRecruiter( pc )) {
            pc->send_to( "Ты не лидер.\n\r" );
            return;
        }

        if (victim->getClan( ) != pc->getClan( )) {
            pc->printf( "%s не в твоем клане.\r\n", victim->getName( ).c_str( ) );
            return;
        }

        if (member && !member->removable) {
            pc->println( "Из твоего клана невозможно никого выгнать." );
            return;
        }
        
            if (pc->get_trust( ) < CREATOR 
            && clan.isRecruiter( victim ) 
            && !dynamic_cast<PCharacter *>( victim )) 
        {
            pc->send_to( "Ты не можешь выгнать из клана рекрутера(лидера), когда его нет в мире.\r\n" );
            return;
        }

        buf << "Тебя выгнали из клана [{" << clan.getColor( ) << clan.getShortName( ) << "{x].";
        victim->setClan( member->removeBy );
    }        

    pc->send_to( "Ok.\n\r" );

    victim->setClanLevel( 0 );
    ClanOrgs::delAttr( victim );

    attr = victim->getAttributes( ).getAttr<XMLAttributeInduct>( "induct" );
    attr->addEntry( buf.str( ) );

    if (PCharacter *pcVictim = dynamic_cast<PCharacter *>( victim )) 
        attr->run( pcVictim );
    else
        PCharacterManager::saveMemory( victim );
}

/*
 * clan remove help
 */
void CClan::clanRemoveHelp( PCharacter *pc )
{
    basic_ostringstream<char> buf;
    
    buf   << "{W{lRклан выгнать себя{lEclan remove self{lx{x - уйти из клана" << endl
          << endl
          << "Для лидеров:" << endl
          << "{W{lRклан выгнать{lEclan remove{lx{x <имя> - выгнать кого-то из клана" << endl;

    pc->send_to( buf );
}

/* 
 * clan level [list|<victim> [<number>]]
 */ 
void CClan::clanLevel( PCharacter *pc, DLString& argument )
{        
    basic_ostringstream<char> buf;
    PCMemoryInterface *victim;

    DLString argumentOne = argument.getOneArgument( );
    DLString argumentTwo = argument.getOneArgument( );

    if (arg_is_help( argumentOne )) {
        clanLevelHelp( pc );
        return;
    }
    else if (arg_is_list( argumentOne )) {
        clanLevelList( pc );
        return;
    }
    else if (argumentOne.empty( ) || arg_is_self( argumentOne )) {
        victim = pc;
    }
    else {
        victim = PCharacterManager::find( argumentOne );

        if (!victim) {
            pc->send_to( "Игрок с таким именем не найден.\r\n" );
            return;
        }                
    }
    
    if (argumentTwo.empty( )) 
        clanLevelShow( pc, victim );
    else 
        clanLevelSet( pc, victim, argumentTwo );
}

/*
 * clan level list
 */
void CClan::clanLevelList( PCharacter *pc )
{
    basic_ostringstream<char> buf;
    const ClanTitles *titles;

    titles = pc->getClan( )->getTitles( );

    if (!titles) {
        pc->send_to( "Клановых званий в твоем клане не обнаружено.\r\n" );
        return;
    }
    
    titles->toStream( buf );
    pc->send_to( buf );
}

/*
 * clan level [<victim>|self]
 */
void CClan::clanLevelShow( PCharacter *pc, PCMemoryInterface *victim )
{
    Clan *clan = &*victim->getClan( );

    if (!clan->getTitles( )) {
        if (victim == pc)
            pc->send_to( "Клановых званий в твоем клане не обнаружено.\r\n" );
        else
            pc->send_to( "В его/ее клане нет клановых званий.\r\n" );
    }
    else {
        if (victim == pc)
            pc->printf( "Твое звание [{%s%s{x].\r\n", 
                        clan->getColor( ).c_str( ), 
                        clan->getTitle( pc ).c_str( ) );
        else
            pc->printf( "%s имеет звание [{%s%s{x].\r\n", 
                        victim->getName( ).c_str( ),
                        clan->getColor( ).c_str( ), 
                        clan->getTitle( victim ).c_str( ) );
    }
    
}

/*
 * clan level <victim>|self <number>
 */
void CClan::clanLevelSet( PCharacter *pc, PCMemoryInterface *victim, const DLString& arg )
{
    int i, size;
    ostringstream buf;
    XMLAttributeInduct::Pointer attr;
    const Clan &clan = *victim->getClan( );
    
    try {
        i = arg.toInt( );
    } catch (const ExceptionBadType &e) {
        pc->send_to( "Неверный клановый уровень.\n\r" );
        return;
    }
    
    if (pc->get_trust( ) < CREATOR) {
        if (!pc->getClan( )->isRecruiter( pc )) {
            pc->send_to( "Ты не являешься лидером клана.\n\r" );
            return;
        }
        
        if (pc->getClan( ) != clan) {
            pc->send_to( "Не лезь в чужой клан.\n\r" );
            return;
        }
    }
    
    if (clan.getTitles( ))
        size = clan.getTitles( )->size( );
    else
        size = 0;

    if (size == 0) {
        pc->send_to( "В этом клане нет клановых уровней.\r\n" );
        return;
    }

    if (i < 0 || i >= size) {
        pc->printf( "Можно использовать только 0..%d\r\n", size - 1 );
        return;
    }

    if (pc->get_trust( ) < CREATOR) {
        if (pc == victim && pc->getClanLevel( ) < i) {
            pc->send_to( "И кто же тебе это позволит?\n\r" );
            return;
        }

        if (victim->getClanLevel( ) > i) {
            if (clan.isRecruiter( victim ) && !dynamic_cast<PCharacter *>( victim )) {
                pc->send_to( "Ты не можешь сместить с поста лидера или рекрутера, когда их нет в мире.\r\n" );
                return;
            }
        }
    }

    if (victim->getClanLevel( ) == i) {
        pc->send_to( "Тихий голосок в сознании шепчет:\n\rЕсть и более глупые, чем ты...\r\nНо много ли таких?\n\r" );
        return;
    }
    
    int oldLevel = victim->getClanLevel();
    victim->setClanLevel( i );
    pc->send_to( "Ok.\n\r" );

    buf << "Ты получаешь клановый уровень, [{"
        << clan.getColor( ) << clan.getTitle( victim ) << "{x].";

    attr = victim->getAttributes( ).getAttr<XMLAttributeInduct>( "induct" );
    attr->addEntry( buf.str( ) );

    PCharacter *pcVictim = victim->getPlayer();
    if (pcVictim)
        attr->run( pcVictim );
    else
        PCharacterManager::saveMemory( victim );

    // Notify about level upgrades otherwise noticeable in 'who'.
    if (oldLevel < i && clan.isRecruiter(victim)) {
        DLString what = fmt(0, "{W%s становится %s клана %s.{x", 
            victim->getName().c_str(), 
            (clan.isLeader(victim) ? "лидером" : "рекрутером"),
            clan.getShortName().c_str());

        infonet(pcVictim, 0, "{CТихий голос из $o2: ", what.c_str());
        send_discord_clan(what);
    }
}

/*
 * clan level help
 */
void CClan::clanLevelHelp( PCharacter *pc )
{
    basic_ostringstream<char> buf;
    
    buf << "{W{lRклан уровень{lEclan level  {lx{x                  - показывает твое клановое звание" << endl
        << "{W{lRклан уровень список{lEclan level list    {lx{x           - показывает список званий для твоего клана" << endl
        << "{W{lRклан уровень {lEclan level{lx {x<имя|{W{lRя{lEself{lx{x>         - показывает клановое звание соклановика" << endl
        << endl
        << "Для лидеров:" << endl
        << "{W{lRклан уровень{lEclan level{lx {x<имя|{W{lRсебе{lEself{lx{x> <число> - устанавливает новый клановый уровень" << endl;

    pc->send_to( buf );
}

/*
 * clan member [date|name|level|clanlevel]
 */
static bool __member_cmp_date__( PCMemoryInterface *a, PCMemoryInterface *b )
{
    return a->getLastAccessTime( ).getTime( ) > b->getLastAccessTime( ).getTime( );
}    
static bool __member_cmp_level__( PCMemoryInterface *a, PCMemoryInterface *b )
{
    return a->getLevel( ) > b->getLevel( );
}    
static bool __member_cmp_clanlevel__( PCMemoryInterface *a, PCMemoryInterface *b )
{
    return a->getClanLevel( ) > b->getClanLevel( );
}    

void CClan::clanMember( PCharacter *pc, DLString& argument )
{        
    basic_ostringstream<char> buf;
    PCharacterMemoryList::const_iterator pos;
    typedef std::list<PCMemoryInterface *> MemberList;
    MemberList members;

    DLString argumentOne = argument.getOneArgument( );

    if (arg_is_help( argumentOne )) {
        clanMemberHelp( pc );
        return;
    }

    if (!pc->getClan( )->isRecruiter( pc ) && !pc->is_immortal( )) {
        pc->send_to( "У тебя недостаточно полномочий для этого.\r\n" );
        return;
    }        

    const PCharacterMemoryList& list = PCharacterManager::getPCM( );

    for (pos = list.begin( ); pos != list.end( ); pos++) {
        PCMemoryInterface *pcm = pos->second;

        if (pcm->getClan( ) != pc->getClan( ) || pcm->getLevel( ) >= 102)
            continue;
        
        members.push_back( pcm );
    }
    
    if (!argumentOne.empty( )) {
        
        if (arg_oneof( argumentOne, "date", "дата", "время" ))
            members.sort( __member_cmp_date__ );
        else if (arg_oneof( argumentOne, "name", "имя" )) 
            ;
        else if (arg_oneof( argumentOne, "level", "уровень" ))
            members.sort( __member_cmp_level__ );
        else if (arg_oneof( argumentOne, "clanlevel", "клануровень" ))
            members.sort( __member_cmp_clanlevel__ );
        else {
            clanMemberHelp( pc );
            return;
        }            
    }   
    
    for (MemberList::iterator i = members.begin( ); i != members.end( ); i++) 
        player_fmt( "%-10n %-10R %-12P %b %-3l %-15t %-a\r\n", *i, buf, pc );

    pc->send_to( "\n\r{BИмя         раса        класс         уровень звание           last time{x\n\r" );
    pc->send_to( buf );
}

/*
 * clan member help
 */
void CClan::clanMemberHelp( PCharacter *pc )
{
    basic_ostringstream<char> buf;
    
    buf   << "Для лидеров:" << endl
          << "{W{lRклан состав{lEclan member{lx{x           - показывает список всех членов клана, в алфавитном порядке" << endl
          << "{W{lRклан состав дата{lEclan member date{lx{x      - сортирует список по дате последнего захода в мир" << endl
          << "{W{lRклан состав уровень{lEclan member level{lx{x     - сортирует список по уровню" << endl
          << "{W{lRклан состав клануровень{lEclan member clanlevel{lx{x - сортирует список по клановому уровню" << endl;

    pc->send_to( buf );
}

/*
 * clan petition [<clan>|list|accept <victim>|reject <victim>]
 */ 
void CClan::clanPetition( PCharacter *pc, DLString& argument )
{
    Clan *clan;
    ClanMembership *member, *mymember;
    ostringstream buf;
    DLString argumentOne = argument.getOneArgument( );
    DLString argumentTwo = argument.getOneArgument( );

    if (arg_is_help( argumentOne )) {
        clanPetitionHelp( pc );
        return;
    }

    if (argumentOne.empty( )) {
        if (pc->getPetition( ) == clan_none) {
            pc->send_to( "Укажи название клана.\r\n" );
            return;
        }
        
        if (!pc->getPetition( )->isValid( )) {
            pc->send_to( "Клан, в который ты желаешь вступить, временно недоступен.\r\n" );
            return;
        }
        
        buf << "Ты желаешь вступить в клан [{"
            << pc->getPetition( )->getColor( )
            << pc->getPetition( )->getShortName( )
            << "{x]" << endl;
                
        pc->send_to( buf );
        return;
    }

    if (pc->getClan( )->isRecruiter( pc ) || pc->is_immortal( )) {
        if (arg_is_list( argumentOne )) {
            clanPetitionList( pc );
            return; 
        } else if (arg_oneof( argumentOne, "accept", "принять" )) {
            clanPetitionAccept( pc, argumentTwo );
            return;
        } else if (arg_oneof( argumentOne, "reject", "отклонить" )) {
            clanPetitionReject( pc, argumentTwo );
            return;
        }
    }

    
    /*
     * Пишем петицию на вступление в клан
     */
    
    if (!IS_SET(pc->act, PLR_CONFIRMED)) {
        pc->println( "Твой персонаж еще не подтвержден Богами." );
        return;
    }
    
    clan = ClanManager::getThis( )->findUnstrict( argumentOne );

    if (!clan) {
        pc->send_to( "Такого клана не существует.\r\n" );
        return;
    }
    
    if (pc->getClan( ) == clan) {
        pc->send_to ("И не лень тебе в свой клан пытаться еще раз вступить?\n\r");
        return;
    }
    
    mymember = pc->getClan( )->getMembership( );
    member = clan->getMembership( );
    
    if (mymember) {
        if (!mymember->removable && mymember->mode.getValue( ) != PETITION_ALWAYS) {
            pc->send_to("Это насовсем...\r\n");
            return;
        }

        if (mymember->removeSelf == clan) {
            pc->send_to ("Если ты очень хочешь, то просто покинь свой клан!\n\r");
            return;
        }
        
        if (mymember->removeBy == clan) {
            pc->send_to ("Если ты очень хочешь, то заставь лидера выгнать тебя из клана!\n\r");
            return;
        }
    }

    if (!member || !clan->canInduct( pc )) {
        pc->send_to( "Ты не можешь вступить в этот клан.\n\r" );
        return;
    }

    if (pc->getRealLevel( ) < member->minLevel) {
        pc->printf( "В этот клан можно вступить только с %d-го уровня.\r\n",
                    member->minLevel.getValue( ) );
        return;
    }

    if (member->mode.getValue( ) == PETITION_NEVER) {
        pc->send_to( "В этот клан нельзя попасть, написав петицию.\r\n" );
        return;
    }
    
    if (member->mode.getValue( ) == PETITION_ALWAYS) {
        doInduct( pc, *clan );
        return;
    }

    if (member->mode.getValue( ) == PETITION_VERIFY) {
        Descriptor *d;
        int found = false;

        pc->setPetition( clan->getName( ) );
        pc->send_to( "Петиция на вступление в клан подана.\r\n" );
                
        // Если есть лидеры, сообщить им
        for (d = descriptor_list; d; d = d->next) {
            Character* victim = d->character;

            if (d->connected == CON_PLAYING 
                && victim
                && !victim->is_npc( )
                && victim->getClan( ) == pc->getPetition( )
                && victim->getClan( )->isRecruiter( victim->getPC( ) ))
            {
                victim->send_to("Есть желающие в клан.\n\r");
                run( victim, "petition list" );
                found = true;
            }
        }
        
        if (!found)
            pc->send_to("(сейчас в мире нет ни одного рекрутера этого клана)\n\r");

        DLString what = fmt(0, "{W%1$^C1 подал%1$Gо||а петицию в клан %s.{x", pc, clan->getShortName().c_str());
        infonet(pc, 0, "{CТихий голос из $o2: ", what.c_str());
        send_discord_clan(what);
    }
}

/* 
 * clan petition list
 */ 
void CClan::clanPetitionList( PCharacter *pc )
{    
    ostringstream buf;
    PCharacterMemoryList::const_iterator pos;

    const PCharacterMemoryList& list = PCharacterManager::getPCM( );

    for (pos = list.begin( ); pos != list.end( ); pos++) {
        PCMemoryInterface *pcm = pos->second;

        if (pcm->getPetition( ) == pc->getClan( ))
            player_fmt( "%-10n %-10R %-12P %b %-3l\r\n", pcm, buf, pc );
    }

    if (buf.str( ).empty( ))
        pc->send_to( "\n\rНет ни одной заявки.\n\r" );
    else {
        pc->send_to( "\n\r{BИмя         раса        класс         уровень{x\n\r" );
        pc->send_to( buf );
    }                
}

/* 
 * clan petition accept <victim>
 */ 
void CClan::clanPetitionAccept( PCharacter *pc, DLString& argument )
{    
    PCMemoryInterface *victim = PCharacterManager::find( argument );

    if (!victim) {
        pc->send_to( "Игрок с таким именем не найден.\r\n" );
        return;
    }
    
    if (victim->getPetition( ) != pc->getClan( )) {
        pc->printf("%s не собирается вступать в твой клан.\r\n", victim->getName( ).c_str( ) ); 
        return;
    }

    if (victim->getClan( ) == victim->getPetition( )) {
        pc->printf("Но %s и так состоит в твоем клане.\r\n", victim->getName( ).c_str( ) ); 
        victim->setPetition( clan_none );
        return;
    }

    pc->println( "Ok." );
    
    doInduct( victim, *pc->getClan( ) );
}

void CClan::doInduct( PCMemoryInterface *victim, const Clan &clan )
{
    basic_ostringstream<char> buf;

    victim->setClan( clan.getName( ) );
    victim->setPetition( clan_none );
    victim->setClanLevel( 0 );

    buf << "Ты приня$gто|т|та в клан [{" 
        << clan.getColor( ) 
        << clan.getShortName( ) 
        << "{x].";
    
    XMLAttributeInduct::Pointer attr = victim->getAttributes( ).getAttr<XMLAttributeInduct>( "induct" );
    attr->addEntry( buf.str( ) );
    
    if (victim->isOnline( ))
        attr->run( victim->getPlayer( ) );
    else
        PCharacterManager::saveMemory( victim );

    DLString what = fmt(0, "{W%s вступает в клан %s.{x", victim->getName().c_str(), clan.getShortName().c_str());
    infonet(victim->getPlayer(), 0, "{CТихий голос из $o2: ", what.c_str());
    send_discord_clan(what);
}

/* 
 * clan petition reject <victim>
 */ 
void CClan::clanPetitionReject( PCharacter *pc, DLString& argument )
{    
    basic_ostringstream<char> buf;
    PCMemoryInterface *victim = PCharacterManager::find( argument );

    if (!victim) {
        pc->send_to( "Игрок с таким именем не найден.\r\n" );
        return;
    }

    if (victim->getPetition( ) != pc->getClan( )) {
        pc->printf("%s не собирается вступать в твой клан.\r\n", victim->getName( ).c_str( ) ); 
        return;
    }

    pc->send_to( "Ok.\n\r" );
    
    if (victim->getClan( ) == clan_none) {
        victim->setClan( clan_outsider );
        victim->setClanLevel( 0 );
    }

    victim->setPetition( clan_none );

    buf << "Твоя заявка на вступление в клан [{"
        << pc->getClan( )->getColor( ) 
        << pc->getClan( )->getShortName( ) 
        << "{x] отклонена." << endl;

    XMLAttributeInduct::Pointer attr = victim->getAttributes( ).getAttr<XMLAttributeInduct>( "induct" );
    attr->addEntry( buf.str( ) );

    if (PCharacter *pcVictim = dynamic_cast<PCharacter *>( victim )) 
        attr->run( pcVictim );
    else
        PCharacterManager::saveMemory( victim );
}

/*
 * clan petition help
 */
void CClan::clanPetitionHelp( PCharacter *pc )
{
    basic_ostringstream<char> buf;
   
   buf    << "{W{lRклан петиция{lEclan petition{lx{x              - показать, в какой клан была написана петиция на вступление" << endl
          << "{W{lRклан петиция{lEclan petition{lx{x <клан>       - подать петицию на вступление в клан" << endl
          << endl
          << "Для руководителей клана:" << endl
          << "{W{lRклан петиция список  {lEclan petition list{lx{x         - показать список всех заявок на поступление" << endl
          << "{W{lRклан петиция принять {lEclan petition accept{lx {x<имя> - принять персонажа, написавшего петицию, в свой клан" << endl
          << "{W{lRклан петиция отклонить{lEclan petition reject{lx {x<имя> - отклонить прошение на прием в клан" << endl;

    pc->send_to( buf );
}

/*
 * clan diplomacy [prop|set <clan> <dipl#>|list]
 */ 
void CClan::clanDiplomacy( PCharacter *pc, DLString& argument )
{        
    DLString argumentOne = argument.getOneArgument( );

    if (argumentOne.empty( )) 
        clanDiplomacyShow( pc );        
    else if (arg_oneof( argumentOne, "prop", "предложения" ))
        clanDiplomacyProp( pc );
    else if (arg_oneof( argumentOne, "set", "установить" ))
        clanDiplomacySet( pc, argument );
    else if (arg_is_list( argumentOne ))
        clanDiplomacyList( pc );
    else
        clanDiplomacyHelp( pc );
}

/* 
 * clan diplomacy
 */ 
void CClan::clanDiplomacyShow( PCharacter *pc )
{    
    ostringstream buf;
    Clan *clan;
    ClanData *data;
    ClanManager *cm = ClanManager::getThis( );

    pc->send_to( "Клановая дипломатия :\n\r" );
    buf << "********** ";
    
    for (int i = 0; i < cm->size( ); i++) {
        clan = cm->find( i );

        if (clan->getData( ) && clan->hasDiplomacy( )) {
            DLString abbr = clan->getShortName( );
            
            abbr.toLower( );
            abbr.upperFirstCharacter( );
                    
            buf << ' ' << setw( 5 ) << abbr.substr( 0, 3 );
        }
    }
    
    buf << endl;

    for (int i = 0; i < cm->size( ); i++) {
        clan = cm->find( i );
        data = clan->getData( );
        
        if (data && clan->hasDiplomacy( )) {
            buf << clan->getPaddedName( ) << ' ';
            
            for (int j = 0; j < cm->size( ); j++) {
                Clan *c = cm->find( j );

                if (c->getData( ) && c->hasDiplomacy( ))
                    buf << ' ' 
                        << clan_diplomacy_names_table[data->getDiplomacy( c )].color
                        << setw( 5 ) 
                        << clan_diplomacy_names_table[data->getDiplomacy( c )].abbr
                        << "{x";
            }
            
            buf << endl;
        }
    }

    buf << endl;

    for (int i = 0; i <= clan_diplomacy_max; i++) 
        buf << clan_diplomacy_names_table[i].color 
            << clan_diplomacy_names_table[i].abbr 
            << "{x - " 
            << clan_diplomacy_names_table[i].long_name 
            << (i < clan_diplomacy_max ? ", " : " ");

    buf << endl;
    pc->send_to( buf );
}            

/* 
 * clan diplomacy prop
 */ 
void CClan::clanDiplomacyProp( PCharacter *pc )
{    
    ostringstream buf;
    Clan *myclan = &*pc->getClan( );
    ClanData *mydata = myclan->getData( );

    if (!mydata || !myclan->hasDiplomacy( )) {
        pc->send_to( "Для твоего клана не существует понятия дипломатии.\r\n" );
        return;
    }

    buf << "Просмотр пропозиций для " << myclan->getShortName( ) << ":" << endl;
    
    for (int i = 0; i < ClanManager::getThis( )->size( ); i++) {
        Clan *clan = ClanManager::getThis( )->find( i );
        ClanData *data = clan->getData( );

        if (clan == myclan || !clan->isValid( ) || !data || !clan->hasDiplomacy( ))
            continue;
            
        if (mydata->getDiplomacy( clan ) == mydata->getProposition( clan )) 
            continue;
        
        buf << '[' << clan->getShortName( ) << "] " 
            << clan_diplomacy_names_table[mydata->getDiplomacy( clan )].long_name
            << " - " 
            << clan_diplomacy_names_table[mydata->getProposition( clan )].long_name
            << endl;
    }

    pc->send_to( buf );    
}

/*
 * clan diplomacy set <clan> <dipl#>
 */ 
void CClan::clanDiplomacySet( PCharacter *pc, DLString& argument )
{    
    ostringstream buf;
    DLString argumentOne = argument.getOneArgument( );
    Clan *clan, *myclan;
    ClanData *data, *mydata;
    int dip;
    
    myclan = &*pc->getClan( );
    mydata = myclan->getData( );

    if (!mydata || !myclan->hasDiplomacy( )) {
        pc->send_to( "Для твоего клана не существует понятия дипломатии.\r\n" );
        return;
    }
    
    if (!myclan->isRecruiter( pc ) && !pc->is_immortal( )) {
        pc->send_to( "Ты не можешь менять политику\n\r" );
        return; 
    }        

    pc->send_to( "Установка политики\n\r" );

    clan = ClanManager::getThis( )->findUnstrict( argumentOne );

    if (!clan) {
        pc->send_to( "Такого клана не существует.\r\n" );
        return;
    }
    
    data = clan->getData( );

    if (!data || !clan->hasDiplomacy( )) {
        pc->send_to( "Для этого клана не существует понятия дипломатии.\r\n" );
        return;
    }
    
    if (myclan == clan) {
        pc->send_to( "Твой клан развалится и без твоей помощи\n\r" );
        mydata->setDiplomacy( myclan, 0 );
        mydata->save( );
        return;
    }

    argument = argument.getOneArgument( );
    
    try {
        dip = argument.toInt( );
    } catch (const ExceptionBadType &e) {
        pc->send_to( "Неверная политика.\n\r" );
        return;
    }
    
    if (dip < 0 || dip > clan_diplomacy_max) {
        pc->send_to( "Неверная политика (см. clan diplomacy list).\r\n" );
        return;
    }
    
    if (mydata->getDiplomacy( clan ) == dip) {
        pc->send_to( "Это ничего не меняет.\n\r" );
        return;
    }
    
    if (clan->isDispersed( )) {
        mydata->setDiplomacy( clan, dip );
        mydata->save( );
        data->setDiplomacy( myclan, dip );
        data->save( );
        
        buf << "Установка политики для "
            << clan->getShortName( ) << " : "
            << clan_diplomacy_names_table[dip].long_name
            << endl;
        pc->send_to( buf );
        return;
    }
    
    if (mydata->getDiplomacy( clan ) > dip) {
        pc->send_to( "улучшение\n\r" );
        
        if (mydata->getProposition( clan) <= dip) {
            // Не лучше предложеного
            mydata->setDiplomacy( clan, dip );
            mydata->setProposition( clan, dip );
            mydata->save( );

            data->setDiplomacy( myclan, dip );
            data->setProposition( myclan, dip );
            data->save( );

            buf << "Установка политики для "
                << clan->getShortName( ) << " : "
                << clan_diplomacy_names_table[dip].long_name
                << endl;
            pc->send_to( buf );
        }
        else
        {
            buf << "Ты предлагаешь "
                << clan->getShortName( ) 
                << " отношение типа "
                << clan_diplomacy_names_table[dip].long_name
                << endl;
            pc->send_to( buf );

            data->setProposition( myclan, dip );
        }
    }
    else
    {
        pc->send_to( "УХУДШЕНИЕ\n\r" );
        
        mydata->setDiplomacy( clan, dip );
        mydata->setProposition( clan, dip );
        mydata->save( );

        data->setDiplomacy( myclan, dip );
        data->setProposition( myclan, dip );
        data->save( );

        buf << "Установка политики для "
            << clan->getShortName( ) 
            << " : " << clan_diplomacy_names_table[dip].long_name
            << endl;
        pc->send_to( buf );
    }
}

/* 
 * clan diplomacy list 
 */ 
void CClan::clanDiplomacyList( PCharacter *pc )
{    
    ostringstream buf;

    buf << "Доступные дипломатии:" << endl;

    for (int i = 0; i <= clan_diplomacy_max; i++)
        buf << i << " - " << clan_diplomacy_names_table[i].color
            << clan_diplomacy_names_table[i].long_name
            << "{x (" << clan_diplomacy_names_table[i].eng_name
            << ')' << endl;
    
    buf << endl;
    pc->send_to( buf );
}

/*
 * clan diplomacy help
 */
void CClan::clanDiplomacyHelp( PCharacter *pc )
{
    basic_ostringstream<char> buf;
   
    buf << "{W{lRклан дипломатия{lEclan diplomacy{lx{x             - показать клановую дипломатию" << endl
        << "{W{lRклан дипломатия предложения{lEclan diplomacy prop       {lx{x - показать предложения по изменению дипломатии" << endl
        << "{W{lRклан дипломатия список{lEclan diplomacy list  {lx{x      - список всех возможных дипломатий" << endl
        << endl
        << "Для лидеров:" << endl
        << "{W{lRклан дипломатия установить{lEclan diplomacy set{lx {x<клан> <номер>" << endl
        << "   - изменить политику по отношению к какому-либо клану" << endl;

    pc->send_to( buf );
}

/*
 * clan scan
 */
void CClan::clanScan( PCharacter *pc )
{
    ostringstream buf;
    ClanManager *cm = ClanManager::getThis( );
    
    for (int j = 0; j < cm->size( ); j++) {
        Clan::Pointer c = cm->find( j );
        
        buf << "[" << j << "] "
            << c->getName( ) << ", " << c->getShortName( ) << " " 
            << (c->isValid( ) ? "valid" : "non-valid") << " "
            << (c->isHidden( ) ? "hidden" : "non-hidden") << " ";

        if (c->getData( )) {
            if (c->getData( )->getBank( ))
                buf << "bank ";
            if (c->getData( )->hasItem( ))
                buf << "item ";
        }

        buf << endl;
    }

    pc->send_to( buf );
}

/*
 * clan induct <player> <clan>
 */
void CClan::clanInduct( PCharacter *pc, DLString &argument )
{
    Clan *new_clan;
    PCMemoryInterface *victim;
    DLString argumentOne;

    argumentOne = argument.getOneArgument( );
    
    if (pc->get_trust( ) < GOD) {
        pc->println("У тебя нет таких полномочий.");
        return;
    }

    if (arg_is_help( argumentOne ) || argumentOne.empty( ) || argument.empty( )) {
        pc->println( "{Wclan induct {x<player> <clan> - принять кого-либо в указанный клан" );
        return;
    }
    
    if (arg_is_self( argumentOne ))
        argumentOne = pc->getName( );

    victim = PCharacterManager::find( argumentOne );
    if (!victim) {
        pc->println( "Игрок с таким именем не найден." );
        return;
    }

    new_clan = ClanManager::getThis( )->findUnstrict( argument );

    if (!new_clan) {
        pc->send_to("О таком клане ничего не известно.\n\r");
        return;
    }
/*    
    if (!new_clan->canInduct( victim )) {
        pc->send_to("Даже ты, при всем своем могуществе, не можешь сделать этого.\n\r");
        return;
    }
*/    
    pc->println( "Ok." );
    doInduct( victim, *new_clan );
}    
