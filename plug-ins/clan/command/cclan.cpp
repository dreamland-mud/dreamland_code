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
#include "grammar_entities_impl.h"
#include "pcharactermemory.h"
#include "pcharacter.h"
#include "pcharactermanager.h"
#include "object.h"
#include "race.h"

#include "merc.h"
#include "descriptor.h"
#include "clanreference.h"
#include "wearloc_utils.h"
#include "loadsave.h"
#include "infonet.h"
#include "messengers.h"
#include "act.h"

#include "screenreader.h"

#include "clantypes.h"
#include "clantitles.h"
#include "clanorg.h"
#include "cclan.h"
#include "xmlattributeinduct.h"
#include "msgformatter.h"
#include "def.h"

CLAN(none);

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
  const char *state_name;
  const char *state_ruscase;
};

struct clan_diplomacy_names clan_diplomacy_names_table[] =
{
  {"alliance",      "аль.", "{W", "альянс",     "В альянсе с",   "5" },
  {"peace",         "мир ", "{G", "мир",        "В мире с",      "5" },
  {"truce",         "пер.", "{Y", "перемирие",  "В перемирии с", "5" },
  {"distrust",      "нед.", "{B", "недоверие",  "Не доверяет",   "3" },
  {"aggression",    "агр.", "{r", "агрессия",   "В агрессии с",  "5" },
  {"war",           "вой.", "{R", "война",      "Враждует с",    "5" },
  {"subordination", "под.", "{Y", "подчинение", "Подчиняется",   "3" },
  {"oppression",    "угн.", "{Y", "угнетение",  "Угнетает",      "4" },
};

const int clan_diplomacy_max = 5;

COMMAND(CClan, "clan")
{
    PCharacter *pc = ch->getPC( );

    if (!pc)
        return;
    
    if (IS_CHARMED(pc)) {
        if (pc->master)
            pc->master->pecho("Нельзя проникнуть в тайны чужого клана с помощью колдовства.");

        pc->pecho("Тебя пытаются принудить выдать тайны своего клана, но ты не поддаешься.");
        return;
    }

    if (constArguments.length( ) == 0) {
        clanList( pc );
    }
    else {
        DLString argument = constArguments;
        DLString argumentOne = argument.getOneArgument( );
        
        if (arg_is_list( argumentOne ) || arg_is(argumentOne, "info"))
            clanList( pc );
        else if (arg_is(argumentOne, "count")) 
            clanCount( pc );
        else if( arg_is(argumentOne, "bank") )
            clanBank( pc, argument );
        else if( arg_is(argumentOne, "remove") )
            clanRemove( pc, argument );
        else if( arg_is(argumentOne, "level") )
            clanLevel( pc, argument );
        else if( arg_is(argumentOne, "member") )
            clanMember( pc, argument );
        else if( arg_is(argumentOne, "petition") )
            clanPetition( pc, argument );
        else if( arg_is(argumentOne, "diplomacy") )
            clanDiplomacy( pc, argument );
        else if (pc->is_immortal( )) {
            if (arg_is(argumentOne, "scan"))
                clanScan( pc );
            else if (arg_is(argumentOne, "induct"))
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

    buf << "{Wклан список{x     показать список всех кланов" << endl
        << "{Wклан счет {x      показать количество игроков в кланах" << endl
        << "{Wклан банк{x       операции с клановым банком (подробнее см. {Wклан банк помощь{x)" << endl
        << "{Wклан выгнать{x    выйти (выгнать кого-либо) из клана (см. {Wклан выгнать помощь{x)" << endl
        << "{Wклан уровень{x    посмотреть/установить клановый уровень (см. {Wклан уровень помощь{x)" << endl
        << "{Wклан состав{x     показывает лидеру список членов клана (см. {Wклан состав помощь{x)" << endl
        << "{Wклан петиция {x   написать/принять/отклонить петицию на вступление в клан" << endl
        << "                (см. {Wклан петиция помощь{x)" << endl           
        << "{Wклан дипломатия{x посмотреть/установить клановую дипломатию (см. {Wклан дипломатия помощь{x)" << endl;

    if (pc->is_immortal( ))
        buf << "{Wклан принять{x    принять кого-то в клан" << endl
            << "{Wклан рейтинг{x    рейтинг клана согласно статистике побед/поражений" << endl
            << "{Wклан статус{x     показать статистику побед/поражений по уровням" << endl;        
    
    pc->send_to( buf );
}

/*
 * clan
 * clan list
 */
void CClan::clanList( PCharacter* pc )
{
    pc->pecho("В мире есть такие кланы:");                              
        
    for (int i = 0; i < ClanManager::getThis( )->size( ); i++) {
        Clan *clan = ClanManager::getThis( )->find( i );
        
        if (!clan->isHidden( )) {
            basic_ostringstream<char> buf;                                          
            buf << setw( 40 ) << clan->getLongName( ) << " [{"
                << clan->getColor( ) << clan->getPaddedName( ) 
                << "{x]" << endl;
            pc->send_to( buf );
        }
    }

    pc->pecho("\n\rПодробнее смотри команду клан ?{x.");                              
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
    
    pc->pecho("      Клан         кол.");                               

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

    pc->pecho("Клан      Рейтинг");                                     
    
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

    pc->pecho("      {BКлан        ...20        21-40       41-60       61-80       81...{x");

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
        pc->pecho("У тебя нет кланового банка!");
        return;
    }

    if (argumentOne.empty( )) // Checking status
    {
        bool fAll = pc->is_immortal( );

        pc->pecho("{g\t\t  Состояние банка твоего клана{x.\n\r");
        pc->pecho("Клан            |{BКвестовых единиц{x|{YЗолотых монет{x|{WСеребряных монет{x|{CБриллиантов{x|");

        for (int i = 0; i < cm->size( ); i++) {
            clan = cm->find( i );
            
            if (!clan->isValid( ) || !clan->getData( ))
                continue;

            ClanBank::Pointer bank = clan->getData( )->getBank( );
            
            if (!bank || (pc->getClan( ) != clan && !fAll))
                continue;

            pc->pecho( "{%s%-16s{x|%16ld|%13ld|%16ld|%11ld|",
                    clan->getColor( ).c_str( ), 
                    clan->getShortName( ).c_str( ),
                    bank->questpoints.getValue( ),
                    bank->gold.getValue( ),
                    bank->silver.getValue( ),
                    bank->diamonds.getValue( ) );
        }

        return;
    }

    if (pc->is_immortal( )) {
        clan = cm->findUnstrict( argumentOne );
        
        if (!clan) {
            pc->pecho("Такого клана пока не существует.");
            return;
        }
        
        if (!clan->getData( ) || !clan->getData( )->getBank( )) {
            pc->pecho("У этого клана нет банка!");
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

    if (arg_is(argumentOne, "deposit"))    
        mode = CB_MODE_DEPOSIT;
    else if (arg_is(argumentOne, "withdraw")) 
        mode = CB_MODE_WITHDRAW;
    else {
        pc->pecho("Можно задать только режим 'положить' или 'снять'.");
        return;
    }

    argumentOne = argument.getOneArgument( );
    
    if (argumentOne.empty( ) || !argumentOne.isNumber( )) {
        pc->pecho("Укажи сумму перевода.");
        return;
    }
    
    try {
        amount = argumentOne.toInt( );
    } catch (const ExceptionBadType& e) {
        pc->pecho("Сумма перевода задана неправильно!");
        return;
    }

    if (amount <= 0) {
        pc->pecho("Сумма должна быть больше нуля.");
        return;
    }

    argumentOne = argument.getOneArgument( );
    
    if (argumentOne.empty( )) {
        pc->pecho("Укажи единицу расчета (кп, золото, серебро, бриллианты).");
        return;
    }
    
    if (arg_is(argumentOne, "qp"))    
        currency = CB_CURR_QP;
    else if (arg_is(argumentOne, "gold")) 
        currency = CB_CURR_GOLD;
    else if (arg_is(argumentOne, "silver")) 
        currency = CB_CURR_SILVER;
    else if (arg_is(argumentOne, "diamond")) 
        currency = CB_CURR_DIAMOND;
    else
    {
        pc->pecho("Кланбанк оперирует только с кп, золото, серебро, бриллианты.");
        return;
    }

    if (mode == CB_MODE_DEPOSIT) {
        if (!clanBankDeposit( pc, clan, currency, amount, buf )) 
            pc->pecho( "Это больше, чем ты имеешь." );
        else {
            pc->send_to( buf );
            clan->getData( )->save( );
            pc->save( );
        }

        return;
    }

    if (!pc->is_immortal() && !clan->isRecruiter( pc ))
    {
        pc->pecho("Это могут сделать только руководители кланов.");
        return;
    }

    argumentOne = argument.getOneArgument( );

    if (!argumentOne.empty( )) // Has a destination
    {
        if (arg_is(argumentOne, "clan")) {
            argumentOne = argument.getOneArgument( );
            acc_clan = cm->findUnstrict( argumentOne );

            if (!acc_clan) {
                pc->pecho("Клан-получатель указан неверно.");
                return;
            }
            
            if (!acc_clan->getData( ) || !acc_clan->getData( )->getBank( )) {
                pc->pecho("У клана-получателя нет банка!");
                return;
            }
        }
        else if (arg_is(argumentOne, "character")) {
            argumentOne = argument.getOneArgument( );
            vch = get_char_world( pc, argumentOne.c_str( ) );

            if (!vch || !(victim = vch->getPC( ))) {
                pc->pecho("Игрока-получателя нет в мире.");
                return;
            }
        }
        else
        {
            pc->pecho("Непонятно, чего ты хочешь?");
            return;
        }
    }

    if (!acc_clan)
        acc_clan = clan;

    if (!victim)
        victim = pc;

    if (currency != CB_CURR_QP && (clan != acc_clan || pc != victim )) {
        pc->pecho("Взаиморасчеты между кланами (или игроками) осуществляются в квестовых очках.");
        return;
    }
                
    if (currency == CB_CURR_QP && victim && pc->getClan() != victim->getClan())
    {
        pc->pecho("Ты можешь отдать квестовые очки или какому-либо клану, или своему соклановику.");
        return;
    }
    
    if (!clanBankWithdraw( pc, victim, clan, acc_clan, currency, amount, buf )) {
        pc->pecho("В банке твоего клана столько нету.");
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
        && obj->wear_loc == wear_none)        
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

        buf << "На банковский счет " 
            << acc_clan->getRussianName( ).ruscase('2') 
            << " переведено: " << amount << " квестов"
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
        
        buf << "На банковский счет " 
            << acc_clan->getRussianName( ).ruscase('2') 
            << " переведено: " << amount << " золот"
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

        buf << "На банковский счет " 
            << acc_clan->getRussianName( ).ruscase('2') 
            << " переведено: " << amount << " серебрян"
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

        buf << "На банковский счет " 
            << acc_clan->getRussianName( ).ruscase('2')
            << " переведено: " << amount << " бриллиант"
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
            buf << "На банковский счет "
                << acc_clan->getRussianName( ).ruscase('2') <<" "
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
    
    buf << "{Wклан банк положить{x <кол-во> {Wкп{x|{Wзолото{x|{Wсеребро{x|{Wбриллианты{x" << endl
        << "          - положить деньги(qp, бриллианты..) в свой кланбанк" << endl
        << endl
        << "Для лидеров:" << endl
        << "{Wклан банк снять {x<кол-во> {Wкп{x|{Wзолото{x|{Wсеребро{x|{Wбриллианты{x" << endl
        << "          - снять деньги(qp) со счета своего кланбанка" << endl
        << "{Wклан банк снять {x<кол-во> {Wкп клану {x<клан>" << endl
        << "          - перевести qp на кланбанк другого клана" << endl
        << "{Wклан банк снять {x<кол-во> {Wкп персонажу {x<имя>" << endl
        << "          - отдать qp с кланбанка соклановику" << endl;
    
    if (pc->is_immortal( ))
        buf << endl
            << "Для Бессмертных: обязательно указывать имя клана, т.е." << endl
            << "{Wклан банк {x<клан> {Wположить{x|{Wснять{x ..." << endl;

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
        pc->pecho("Игрок с таким именем не найден.");
        return;
    }
    
    Clan &clan = *victim->getClan( );
    member = clan.getMembership( );

    if (pc == victim) {
        if (!member) {
            pc->pecho("А откуда еще тебе хотелось бы уйти?");
            return;
        }
        
        if (!pc->is_immortal( )) 
            if (!member->removable) {
                pc->pecho( "Из твоего клана невозможно уйти по собственной воле." );
                return;
            }

        buf << "Ты решаешь покинуть [" 
            << clan.getRussianName( ).ruscase('4') << "].";
                
        pc->setClan( member->removeSelf );

    } else {
        if (!pc->is_immortal() && !clan.isRecruiter( pc )) {
            pc->pecho("Это могут сделать только руководители кланов.");
            return;
        }

        if (victim->getClan( ) != pc->getClan( )) {
            pc->pecho( "%s не в твоем клане.", victim->getName( ).c_str( ) );
            return;
        }

        if (member && !member->removable) {
            pc->pecho( "Из твоего клана невозможно никого выгнать." );
            return;
        }
        
        if (clan.isRecruiter( victim ) && !dynamic_cast<PCharacter *>( victim )) 
        {
            if ( !pc->isCoder() && !pc->is_immortal() ) {
                pc->pecho("Выгонять руководство кланов можно только при очной ставке -- дождись, когда они зайдут в мир.");
                return;
            }
        }

        buf << "Тебя заставили покинуть [" << clan.getRussianName( ).ruscase('4') << "].";
        victim->setClan( member->removeBy );
    }        

    pc->pecho("Ok.");

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
    
    buf   << "{Wклан выгнать себя{x - уйти из клана" << endl
          << endl
          << "Для лидеров:" << endl
          << "{Wклан выгнать{x <имя> - выгнать кого-то из клана" << endl;

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
            pc->pecho("Игрок с таким именем не найден.");
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
        pc->pecho("Клановых званий в твоем клане не обнаружено.");
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
            pc->pecho("Клановых рангов в твоем клане не обнаружено.");
        else
            pc->pecho("В его/ее клане нет клановых рангов.");
    }
    else {
        if (victim == pc)
            pc->pecho( "Твой ранг [{%s%s{x].", 
                        clan->getColor( ).c_str( ), 
                        clan->getTitle( pc ).c_str( ) );
        else
            pc->pecho( "%s имеет ранг [{%s%s{x].", 
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
        pc->pecho("Неверный клановый ранг.");
        return;
    }
    
    if (pc->get_trust( ) < CREATOR) {
        if (!pc->getClan( )->isRecruiter( pc )) {
            pc->pecho("Это могут сделать только руководители кланов.");
            return;
        }
        
        if (pc->getClan( ) != clan) {
            pc->pecho("Не лезь в чужой клан.");
            return;
        }
    }
    
    if (clan.getTitles( ))
        size = clan.getTitles( )->size( );
    else
        size = 0;

    if (size == 0) {
        pc->pecho("В этом клане нет клановых рангов.");
        return;
    }

    if (i < 0 || i >= size) {
        pc->pecho( "Можно использовать только цифры от 0 до %d", size - 1 );
        return;
    }

    if ( !pc->isCoder() && !pc->is_immortal() ) {
        if (pc == victim && pc->getClanLevel( ) < i) {
            pc->pecho("И кто же тебе это позволит?");
            return;
        }

        if (victim->getClanLevel( ) > i) {
            if (clan.isRecruiter( victim ) && !dynamic_cast<PCharacter *>( victim )) {
                pc->pecho("Смещать руководство кланов можно только при очной ставке -- дождись, когда они зайдут в мир.");
                return;           
            }
        }
    }

    if (victim->getClanLevel( ) == i) {
        pc->pecho("Тихий голосок в сознании шепчет:\n\rЕсть и более глупые, чем ты...\r\nНо много ли таких?");
        return;
    }
    
    int oldLevel = victim->getClanLevel();
    victim->setClanLevel( i );
    pc->pecho("Ok.");

    buf << "Ты получаешь клановый ранг [{"
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
        DLString what = fmt(0, "{W%s становится %s %s.{x", 
            victim->getNameP('1').c_str(),
            (clan.isLeader(victim) ? "лидером" : "рекрутером"),
            clan.getRussianName().ruscase('2').c_str());

        infonet(pcVictim, 0, "{CТихий голос из $o2: ", what.c_str());
        send_discord_clan(what);
        send_telegram(what);
    }
}

/*
 * clan level help
 */
void CClan::clanLevelHelp( PCharacter *pc )
{
    basic_ostringstream<char> buf;
    
    buf << "{Wклан уровень{x                  - показывает твой клановый ранг" << endl
        << "{Wклан уровень список{x           - показывает список рангов для твоего клана" << endl
        << "{Wклан уровень  {x<имя|{Wя{x>         - показывает клановый ранг соклановика" << endl
        << endl
        << "Для лидеров:" << endl
        << "{Wклан уровень {x<имя|{Wсебе{x> <число> - устанавливает новый клановый ранг" << endl;

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

    if (pc->getClan()->isDispersed()) {
        if (pc->getClan() == clan_none)
            pc->pecho("Сначала присоединись к одному из кланов.");
        else
            pc->pecho("Ты не можешь увидеть список своих соклановиков.");
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
        
        if (arg_is(argumentOne, "date"))
            members.sort( __member_cmp_date__ );
        else if (arg_is(argumentOne, "name")) 
            ;
        else if (arg_is(argumentOne, "level"))
            members.sort( __member_cmp_level__ );
        else if (arg_is(argumentOne, "clanlevel"))
            members.sort( __member_cmp_clanlevel__ );
        else {
            clanMemberHelp( pc );
            return;
        }            
    }   
    
    for (MemberList::iterator i = members.begin( ); i != members.end( ); i++) {
        PCMemoryInterface *pcm = *i;
        buf << fmt(0, "%-10s %-10s %-12s %2d %3d  %-15s %s\r\n",
                   pcm->getName().c_str(),
                   pcm->getRace()->getName().c_str(),
                   pcm->getProfession( )->getNameFor(pc).c_str(),
                   pcm->getRemorts().size(), 
                   pcm->getLevel(),
                   pcm->getClan()->getTitle(pcm).c_str(),
                   pcm->getLastAccessTime( ).getTimeAsString("%d/%m/%y %H:%M").c_str());
    }

    pc->pecho("\n\r{BИмя         раса        класс         уровень звание           last time{x");
    pc->send_to( buf );
}

/*
 * clan member help
 */
void CClan::clanMemberHelp( PCharacter *pc )
{
    basic_ostringstream<char> buf;
    
    buf   << "{Wклан состав{x           - показывает список всех членов клана, в алфавитном порядке" << endl
          << "{Wклан состав дата{x      - сортирует список по дате последнего захода в мир" << endl
          << "{Wклан состав уровень{x     - сортирует список по рангу" << endl
          << "{Wклан состав клануровень{x - сортирует список по клановому рангу" << endl;

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
            pc->pecho("Укажи название клана.");
            return;
        }
        
        if (!pc->getPetition( )->isValid( )) {
            pc->pecho("Клан, в который ты желаешь вступить, временно недоступен.");
            return;
        }
        
        buf << "Ты желаешь вступить в ["            
            << pc->getPetition( )->getRussianName( ).ruscase('4')
            << "]" << endl;
                
        pc->send_to( buf );
        return;
    }

    if (pc->getClan( )->isRecruiter( pc ) || pc->is_immortal( )) {
        if (arg_is_list( argumentOne )) {
            clanPetitionList( pc );
            return; 
        } else if (arg_is(argumentOne, "accept")) {
            clanPetitionAccept( pc, argumentTwo );
            return;
        } else if (arg_is(argumentOne, "reject")) {
            clanPetitionReject( pc, argumentTwo );
            return;
        }
    }

    
    /*
     * Пишем петицию на вступление в клан
     */
    
    if (!IS_SET(pc->act, PLR_CONFIRMED)) {
        pc->pecho( "Твой персонаж еще не подтвержден Богами." );
        return;
    }
    
    clan = ClanManager::getThis( )->findUnstrict( argumentOne );

    if (!clan) {
        pc->pecho("Такого клана не существует.");
        return;
    }
    
    if (pc->getClan( ) == clan) {
        pc->pecho("И не лень тебе в свой клан пытаться еще раз вступить?");
        return;
    }
    
    mymember = pc->getClan( )->getMembership( );
    member = clan->getMembership( );
    
    if (mymember) {
        if (!mymember->removable && mymember->mode.getValue( ) != PETITION_ALWAYS) {
            pc->pecho("Это насовсем...");
            return;
        }

        if (mymember->removeSelf == clan) {
            pc->pecho("Если ты очень хочешь, то просто покинь свой клан!");
            return;
        }
        
        if (mymember->removeBy == clan) {
            pc->pecho("Если ты очень хочешь, то заставь лидера выгнать тебя из клана!");
            return;
        }
    }

    if (!member || !clan->canInduct( pc )) {
        pc->pecho("Ты не можешь вступить в этот клан.");
        return;
    }

    if (pc->getRealLevel( ) < member->minLevel) {
        pc->pecho( "В этот клан можно вступить только с %d-го уровня.",
                    member->minLevel.getValue( ) );
        return;
    }

    if (member->mode.getValue( ) == PETITION_NEVER) {
        pc->pecho("В этот клан нельзя попасть, написав петицию.");
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
        pc->pecho("Петиция на вступление в клан подана.");
                
        // Если есть лидеры, сообщить им
        for (d = descriptor_list; d; d = d->next) {
            Character* victim = d->character;

            if (d->connected == CON_PLAYING 
                && victim
                && !victim->is_npc( )
                && victim->getClan( ) == pc->getPetition( )
                && victim->getClan( )->isRecruiter( victim->getPC( ) ))
            {
                victim->pecho("Есть желающие в клан.");
                run( victim, "petition list" );
                found = true;
            }
        }
        
        if (!found)
            pc->pecho("(сейчас в мире нет никого из руководства этого клана)");

        DLString what = fmt(0, "{W%1$^C1 подал%1$Gо||а петицию в %s.{x", pc, clan->getRussianName( ).ruscase('4').c_str());
        infonet(pc, 0, "{CТихий голос из $o2: ", what.c_str());
        send_discord_clan(what);
        send_telegram(what);
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
            buf << fmt(0, "%-10s %-10s %-12s %2d %3d\r\n",
                    pcm->getName().c_str(),
                    pcm->getRace()->getName().c_str(),
                    pcm->getProfession( )->getNameFor(pc).c_str(),
                    pcm->getRemorts().size(), 
                    pcm->getLevel());
    }

    if (buf.str( ).empty( ))
        pc->pecho("\n\rНет ни одной заявки.");
    else {
        pc->pecho("\n\r{BИмя         раса        класс         уровень{x");
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
        pc->pecho("Игрок с таким именем не найден.");
        return;
    }
    
    if (victim->getPetition( ) != pc->getClan( )) {
        pc->pecho("%s не собирается вступать в твой клан.", victim->getName( ).c_str( ) ); 
        return;
    }

    if (victim->getClan( ) == victim->getPetition( )) {
        pc->pecho("Но %s и так состоит в твоем клане.", victim->getName( ).c_str( ) ); 
        victim->setPetition( clan_none );
        return;
    }

    pc->pecho( "Ok." );
    
    doInduct( victim, *pc->getClan( ) );
}

void CClan::doInduct( PCMemoryInterface *victim, const Clan &clan )
{
    basic_ostringstream<char> buf;

    victim->setClan( clan.getName( ) );
    victim->setPetition( clan_none );
    victim->setClanLevel( 0 );

    buf << "Ты приня$gто|т|та в [" 
        << clan.getRussianName( ).ruscase('4') 
        << "].";
    
    XMLAttributeInduct::Pointer attr = victim->getAttributes( ).getAttr<XMLAttributeInduct>( "induct" );
    attr->addEntry( buf.str( ) );
    
    if (victim->isOnline( ))
        attr->run( victim->getPlayer( ) );
    else
        PCharacterManager::saveMemory( victim );
    
    if (victim->getLevel() <= LEVEL_MORTAL) {
        DLString what;
        if (victim->getClan() == clan_none)
            what = fmt(0, "{W%s становится внекланов%s.{x", victim->getNameP('1').c_str(), GET_SEX(victim, "ым", "ым", "ой"));
        else
            what = fmt(0, "{W%s вступает в %s.{x", victim->getNameP('1').c_str(), clan.getRussianName( ).ruscase('4').c_str());
            
        infonet(victim->getPlayer(), 0, "{CТихий голос из $o2: ", what.c_str());
        send_discord_clan(what);
        send_telegram(what);
    }
}

/* 
 * clan petition reject <victim>
 */ 
void CClan::clanPetitionReject( PCharacter *pc, DLString& argument )
{    
    basic_ostringstream<char> buf;
    PCMemoryInterface *victim = PCharacterManager::find( argument );

    if (!victim) {
        pc->pecho("Игрок с таким именем не найден.");
        return;
    }

    if (victim->getPetition( ) != pc->getClan( )) {
        pc->pecho("%s не собирается вступать в твой клан.", victim->getName( ).c_str( ) ); 
        return;
    }

    pc->pecho("Ok.");
    
    victim->setPetition( clan_none );

    buf << "Твоя заявка на вступление в ["
        << pc->getClan( )->getRussianName( ).ruscase('4')
        << "] отклонена." << endl;

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
   
   buf    << "{Wклан петиция{x              - показать, в какой клан была написана петиция на вступление" << endl
          << "{Wклан петиция{x <клан>       - подать петицию на вступление в клан" << endl
          << endl
          << "Для руководителей клана:" << endl
          << "{Wклан петиция список  {x         - показать список всех заявок на поступление" << endl
          << "{Wклан петиция принять  {x<имя> - принять персонажа, написавшего петицию, в свой клан" << endl
          << "{Wклан петиция отклонить {x<имя> - отклонить прошение на прием в клан" << endl;

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
    else if (arg_is(argumentOne, "proposition"))
        clanDiplomacyProp( pc );
    else if (arg_is(argumentOne, "set"))
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

    if (uses_screenreader(pc)) {
        clanDiplomacyForBlindShow(pc);
        return;
    }

    pc->pecho("Клановая дипломатия :");
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
 * clan diplomacy for the blind
 */
void CClan::clanDiplomacyForBlindShow( PCharacter *pc )
{
    ostringstream buf;
    Clan *clan;
    ClanData *data;
    ClanManager *cm = ClanManager::getThis( );

    pc->pecho("Клановая дипломатия :");
    buf << "********** " << endl;

    for (int i = 0; i < cm->size( ); i++) {
        clan = cm->find( i );
        data = clan->getData( );

        if (clan->getData( ) && clan->hasDiplomacy( )) {
            buf << clan->getRussianName( ).ruscase('1').c_str() << " : ";
        } else {
            continue;
        }

        for (int j = clan_diplomacy_max; j >= 0; j--) {
            int itemCount = 0;

            for (int k = 0; k < cm->size( ); k++) {
                if (i == k)
                    continue;

                Clan *c = cm->find( k );

                if (c->getData( ) && c->hasDiplomacy( ) && j == data->getDiplomacy( c )) {
                    itemCount++;
                }

            }
            if (itemCount > 0) {
                 buf << clan_diplomacy_names_table[j].color
                     << clan_diplomacy_names_table[j].state_name << "{x ";
                char rusCase = *clan_diplomacy_names_table[j].state_ruscase;
                for (int k = 0; k < cm->size( ); k++) {
                    if (i == k)
                        continue;

                    Clan *c = cm->find( k );

                    if (c->getData( ) && c->hasDiplomacy( ) && j == data->getDiplomacy( c )) {
                        itemCount--;
                        buf << c->getRussianName( ).ruscase( rusCase ).c_str();
                        if (itemCount > 1) {
                            buf << ", ";
                        } else if (itemCount == 1) {
                            buf << " и ";
                        } else {
                            buf << ". ";
                        }
                    }
                }
            }

        }
        buf << endl;

    }
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
        pc->pecho("Для твоего клана не существует понятия дипломатии.");
        return;
    }

    buf << "Просмотр пропозиций для " << myclan->getRussianName( ).ruscase('4') << ":" << endl;
    
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
        pc->pecho("Для твоего клана не существует понятия дипломатии.");
        return;
    }
    
    if (!myclan->isRecruiter( pc ) && !pc->is_immortal( )) {
        pc->pecho("Только руководство кланов может менять политику.");
        return; 
    }        

    pc->pecho("Установка политики");

    clan = ClanManager::getThis( )->findUnstrict( argumentOne );

    if (!clan) {
        pc->pecho("Такого клана не существует.");
        return;
    }
    
    data = clan->getData( );

    if (!data || !clan->hasDiplomacy( )) {
        pc->pecho("Для этого клана не существует понятия дипломатии.");
        return;
    }
    
    if (myclan == clan) {
        pc->pecho("Твой клан развалится и без твоей помощи");
        mydata->setDiplomacy( myclan, 0 );
        mydata->save( );
        return;
    }

    argument = argument.getOneArgument( );
    
    try {
        dip = argument.toInt( );
    } catch (const ExceptionBadType &e) {
        pc->pecho("Неверная политика.");
        return;
    }
    
    if (dip < 0 || dip > clan_diplomacy_max) {
        pc->pecho("Неверная политика (см. clan diplomacy list).");
        return;
    }
    
    if (mydata->getDiplomacy( clan ) == dip) {
        pc->pecho("Это ничего не меняет.");
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
        pc->pecho("улучшение");
        
        if (mydata->getProposition( clan) <= dip) {
            // Не лучше предложеного
            mydata->setDiplomacy( clan, dip );
            mydata->setProposition( clan, dip );
            mydata->save( );

            data->setDiplomacy( myclan, dip );
            data->setProposition( myclan, dip );
            data->save( );

            buf << "Установка политики для "
                << clan->getRussianName( ).ruscase('2') << " : "
                << clan_diplomacy_names_table[dip].long_name
                << endl;
            pc->send_to( buf );
        }
        else
        {
            buf << "Ты предлагаешь "
                << clan->getRussianName( ).ruscase('3')
                << " отношение типа "
                << clan_diplomacy_names_table[dip].long_name
                << endl;
            pc->send_to( buf );

            data->setProposition( myclan, dip );
        }
    }
    else
    {
        pc->pecho("УХУДШЕНИЕ");
        
        mydata->setDiplomacy( clan, dip );
        mydata->setProposition( clan, dip );
        mydata->save( );

        data->setDiplomacy( myclan, dip );
        data->setProposition( myclan, dip );
        data->save( );

        buf << "Установка политики для "
            << clan->getRussianName( ).ruscase('2')
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
   
    buf << "{Wклан дипломатия{x             - показать клановую дипломатию" << endl
        << "{Wклан дипломатия предложения{x - показать предложения по изменению дипломатии" << endl
        << "{Wклан дипломатия список{x      - список всех возможных дипломатий" << endl
        << endl
        << "Для лидеров:" << endl
        << "{Wклан дипломатия установить {x<клан> <номер>" << endl
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
        pc->pecho("У тебя нет таких полномочий.");
        return;
    }

    if (arg_is_help( argumentOne ) || argumentOne.empty( ) || argument.empty( )) {
        pc->pecho( "{Wclan induct {x<player> <clan> - принять кого-либо в указанный клан" );
        return;
    }
    
    if (arg_is_self( argumentOne ))
        argumentOne = pc->getName( );

    victim = PCharacterManager::find( argumentOne );
    if (!victim) {
        pc->pecho( "Игрок с таким именем не найден." );
        return;
    }

    new_clan = ClanManager::getThis( )->findUnstrict( argument );

    if (!new_clan) {
        pc->pecho("О таком клане ничего не известно.");
        return;
    }
    
    pc->pecho( "Ok." );
    doInduct( victim, *new_clan );
}    
