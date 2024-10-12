/* $Id: cgquest.cpp,v 1.1.2.1.6.8 2009/09/06 21:48:28 rufina Exp $
 *
 * ruffina, 2003
 */
#include <string.h>

#include "logstream.h"
#include "cgquest.h"
#include "globalquestmanager.h"
#include "globalquestinfo.h"
#include "globalquest.h"
#include "gqchannel.h"
#include "xmlattributeglobalquest.h"

#include "feniamanager.h"
#include "wrappermanagerbase.h"
#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"
#include "subr.h"
#include "native.h"

#include "pcharacter.h"
#include "pcharactermanager.h"
#include "act.h"
#include "loadsave.h"

#include "merc.h"
#include "def.h"

COMMAND(CGQuest, "gquest")
{
    DLString cmd;
    DLString arguments = constArguments;
    PCharacter *pch = ch->getPC( );

    arguments.stripWhiteSpace( );
    cmd = arguments.getOneArgument( );
    
    if (!pch) {
        ch->pecho("Тебе нельзя.");
        return;
    }
    
    if (cmd.empty( ))
        usage( pch );
    else if (arg_is_info( cmd ))
        doInfo( pch );
    else if (arg_oneof( cmd, "progress", "прогресс" ))
        doProgress( pch );
    else if (arg_oneof( cmd, "noexp", "безопыта" ))
        doNoExp( pch, arguments );
    else if (arg_oneof( cmd, "victory", "победа", "победы" ))
        doVictory( pch );
    else if (arg_oneof( cmd, "stat", "статистика" ))
        doStat( pch );
    else if (pch->is_immortal( )) {
        if (arg_is_list( cmd ))
            doList( pch );
        else if (arg_oneof( cmd, "start", "старт" ))
            doStart( pch, arguments );
        else if (arg_oneof( cmd, "stop", "стоп" ))
            doStop( pch, arguments );
        else if (arg_oneof( cmd, "time", "время" ))
            doTime( pch, arguments );
        else if (arg_oneof( cmd, "talk", "сказать", "говорить" ))
            doTalk( pch, arguments );
        else if (arg_oneof( cmd, "auto", "авто" ))
            doAuto( pch, arguments );
        else if (pch->isCoder( )) {
            if (arg_oneof( cmd, "set", "установить" ))
                doSet( pch, arguments );
            else if (arg_oneof( cmd, "read", "прочесть" ))
                doRead( pch, arguments );
            else
                usage( pch );
        }
        else 
            usage( pch );
    }
    else
        usage( pch );

}

bool CGQuest::gqprog( PCharacter *ch, Scripting::IdRef &id )
{
    static Scripting::IdRef ID_TMP( "tmp" ), ID_GQUEST( "gquest" );
    Scripting::Register tmpGQuest;
    Scripting::RegisterList regList;

    if (!FeniaManager::wrapperManager)
        return false;

    try {
        tmpGQuest = *(*Scripting::Context::root[ID_TMP])[ID_GQUEST];
        regList.push_front( FeniaManager::wrapperManager->getWrapper( ch ) );
        return tmpGQuest[id]( regList ).toBoolean( );
    }
    catch (const Scripting::Exception &e) {
        LogStream::sendWarning( ) << "gquest: " << e.what( ) << endl;
        return false;
    }
}

bool CGQuest::gqprog_info( PCharacter *ch )
{
    static Scripting::IdRef ID_INFO( "info" );
    return gqprog( ch, ID_INFO );
}

bool CGQuest::gqprog_progress( PCharacter *ch )
{
    static Scripting::IdRef ID_PROGRESS( "progress" );
    return gqprog( ch, ID_PROGRESS );
}

bool CGQuest::gqprog_notify( PCharacter *ch )
{
    static Scripting::IdRef ID_NOTIFY( "notify" );
    return gqprog( ch, ID_NOTIFY );
}

void CGQuest::doInfo( PCharacter *ch ) 
{
    GlobalQuestInfo::Pointer gqi;
    GlobalQuest::Pointer gq;
    GlobalQuestManager::RunList::iterator i;
    GlobalQuestManager *manager = GlobalQuestManager::getThis( );
    GlobalQuestManager::RunList &rl = manager->getRunning( );
    bool found = false;
    
    for (i = rl.begin( ); i != rl.end( ); i++) {
        ostringstream buf;

        gq = i->second;
        gqi = manager->findGlobalQuestInfo( gq->getQuestID( ) );
        
        if (gq->isHidden( ))
            continue;

        buf << GQChannel::BOLD << "\"" << gqi->getQuestName( ) << "\"" << GQChannel::NORMAL << endl;
        gq->getQuestDescription( buf );
        buf << GQChannel::NORMAL;
        gq->report( buf, ch );
        GQChannel::pecho( ch, buf );
        found = true;
    }

    if (gqprog_info( ch ))
        found = true;
    
    if (!found)
        ch->pecho("Сейчас нет ни одного глобального задания.");
}

void CGQuest::doProgress( PCharacter *ch ) 
{
    GlobalQuestInfo::Pointer gqi;
    GlobalQuest::Pointer gq;
    GlobalQuestManager::RunList::iterator i;
    GlobalQuestManager *manager = GlobalQuestManager::getThis( );
    GlobalQuestManager::RunList &rl = manager->getRunning( );
    
    for (i = rl.begin( ); i != rl.end( ); i++) {
        ostringstream buf;

        gq = i->second;
        gqi = manager->findGlobalQuestInfo( gq->getQuestID( ) );

        if (gq->isHidden( ))
            continue;

        buf << GQChannel::NORMAL << "Квест "<< GQChannel::BOLD << "\""<< gqi->getQuestName( ) << "\""
            << GQChannel::NORMAL << " (для ";
            
        if (gq->hasLevels( ))
            buf << GQChannel::BOLD << gq->getMinLevel( ) 
                << "-" << gq->getMaxLevel( ) << GQChannel::NORMAL;
        else
            buf << "всех";
        
        buf << " уровней)" << endl;
        gq->progress( buf );
        GQChannel::pecho( ch, buf );
    }

    gqprog_progress( ch );
}

void CGQuest::doNoExp( PCharacter *ch, DLString& arguments ) 
{
    std::basic_ostringstream<char> buf;
    XMLAttributeGlobalQuest::Pointer attribute;
   
    try {
        attribute = ch->getAttributes( ).getAttr<XMLAttributeGlobalQuest>( "gquest" ); 
    } catch (const Exception &e) {
        LogStream::sendError( ) << e.what( ) << endl;
        return;
    }

    if (!arguments.empty( )) {
        if (arg_is_yes( arguments )) {
            attribute->setNoExp( true );
        } else if (arg_is_no( arguments )) {
            attribute->setNoExp( false );
        } else {
            ch->pecho("Используй 'гквест безопыта да' или 'гквест безопыта нет'.");
            return;
        }

        PCharacterManager::saveMemory( ch );
    }

    if (attribute->getNoExp( ) == true) {
        ch->pecho("Ты не будешь получать опыт как награду за глобальные квесты.");
    } else {
        ch->pecho("Ты будешь получать опыт как награду за глобальные квесты.");
    }
}

void CGQuest::doVictory( PCharacter *ch )
{
    ostringstream buf;
    XMLAttributeGlobalQuest::Pointer gqAttr;
    int cnt = 0;

    buf << "Твои победы в глобальных квестах:" << endl;

    gqAttr = ch->getAttributes( ).findAttr<XMLAttributeGlobalQuest>( "gquest" );
    if (gqAttr) {
        GlobalQuestManager::RegistryList::iterator i;
        GlobalQuestManager::RegistryList& reg = GlobalQuestManager::getThis( )->getRegistry( );

        for (i = reg.begin( ); i != reg.end( ); i++) {
            GlobalQuestInfo::Pointer gqi = i->second;
            int vct = gqAttr->getVictories( gqi->getQuestID( ) );

            if (vct > 0) {
                cnt++;
                buf << GQChannel::BOLD << "\"" << gqi->getQuestName( ) << "\"" << GQChannel::NORMAL << endl
                    << "    " << vct << endl << endl;
            }
        }
    }
    
    if (cnt == 0) 
        buf << "    ни одной, увы" << endl;

    GQChannel::pecho( ch, buf );
}

void CGQuest::doStat( PCharacter *ch )
{
    ostringstream buf;
    XMLAttributeStatistic::Statistic stat;
    XMLAttributeStatistic::Statistic::iterator s;
    static DLString pad = "          ";
   
    stat = XMLAttributeStatistic::gatherAll( "gquest" );

    buf << "Лучшие квестодеятели Мира Грез: " << endl;

    for (s = stat.begin( ); s != stat.end( ); s++) {
        XMLAttributeStatistic::StatRecordList::iterator r;
        GlobalQuestInfo::Pointer gqi;
        int last = 0, cnt = 0;
        
        gqi = GlobalQuestManager::getThis( )->findGlobalQuestInfo( s->first );

        if (!gqi)
            continue;

        XMLAttributeStatistic::StatRecordList &records = s->second;
        
        buf << GQChannel::BOLD << "\"" << gqi->getQuestName( ) << "\"" << GQChannel::NORMAL << endl;

        for (r = records.begin( ); r != records.end( ) && cnt < 10; cnt++) {
            last = r->second;
            buf << pad << GQChannel::BOLD 
                << fmt(0, "%4d", last ) << GQChannel::NORMAL << " ";

            for ( ; r != records.end( ) && r->second == last; r++)
                buf << r->first << " ";

            buf << endl;
        }
        
        buf << endl;
    }
    
    GQChannel::pecho( ch, buf );
}

void CGQuest::doList( PCharacter *ch ) 
{
    GlobalQuestManager::RegistryList::iterator i;
    GlobalQuestManager::RegistryList& reg = GlobalQuestManager::getThis( )->getRegistry( );

    ch->pecho("%sСписок глобальных квестов Мира Грез", GQChannel::NORMAL );
   
    ch->pecho("%s%-10s %-10s %s %-4s %s %7s %9s %s%s",
            GQChannel::BOLD, "Название", "ID", "A", "idle", "R", "Уровни", "Время", "Описание", GQChannel::NORMAL );

    ostringstream buf;

    for (i = reg.begin( ); i != reg.end( ); i++) {
        GlobalQuestInfo::Pointer gqip = i->second;
        GlobalQuest::Pointer gq = GlobalQuestManager::getThis( )->findGlobalQuest( gqip->getQuestID( ) );

        buf << fmt(0, "%s%-10.10s %-10s ",
                GQChannel::NORMAL,
                gqip->getQuestName( ).c_str( ),
                gqip->getQuestID( ).c_str( ));

        if (gqip->getAutostart( ))
            buf << fmt(0, "* %-4d", (int)(gqip->getWaitingTime( ) / 60));
        else 
            buf << "      " ;

        if (gq) {
            buf << " * " ;

            if (gq->hasLevels( ))
                buf << fmt(0, "%3d-%-3d",
                         gq->getMinLevel( ), gq->getMaxLevel( ));
            else
                buf << fmt(0, "%7s", "" );
            
            buf << fmt(0, " %4d/%-4d",
                                     gq->getElapsedTime( ), gq->getTotalTime( ));
        }
        else 
            buf << fmt(0, "%-2s %7s %9s", "", "", "");
        
        buf << fmt(0, " %s{x\r\n", gqip->getQuestShortDescr( ).c_str( ) );
    }

    ch->send_to( buf );

    ch->send_to( "\r\nПоля: "
                 "A - автостарт, "
                 "idle - минут между автостартами, "
                 "R - running{x\r\n" );
}

void CGQuest::doStart( PCharacter *ch, DLString& arguments ) 
{
    ostringstream buf;
    GlobalQuestInfo::Pointer gqi;
    GlobalQuestInfo::Config config;
    
    if (arguments.empty( )) {
        ch->pecho("Укажите ID глобального квеста.");
        return;
    }
    
    gqi = GlobalQuestManager::getThis( )->findGlobalQuestInfo( arguments.getOneArgument( ) );

    if (!gqi) {
        ch->pecho("Неправильный ID.");
        return;
    }

    if (!gqi->parseArguments( arguments, config, buf ))
        ch->send_to( buf );
    else
        try {
            gqi->tryStart( config );
        }
        catch (const Exception &e) {
            ch->send_to( e.what( ) );
        }
}

void CGQuest::doStop( PCharacter *ch, DLString& arguments ) 
{
    GlobalQuest::Pointer gq;
    
    if (arguments.empty( )) {
        ch->pecho("Укажите ID глобального квеста.");
        return;
    }
    
    gq = GlobalQuestManager::getThis( )->findGlobalQuest( arguments.getOneArgument( ) );
    if (!gq) {
        ch->pecho("Неправильный ID, либо квест не запущен.");
        return;
    }
    
    try {
        gq->scheduleDestroy( );
    }  catch (const Exception &e) {
        ch->send_to( e.what( ) );
        return;
    }
    ch->pecho( "Глобальный квест остановлен." );
}

void CGQuest::doTime( PCharacter *ch, DLString& arguments ) 
{
    GlobalQuest::Pointer gq;
    
    if (arguments.empty( )) {
        ch->pecho("Укажите ID глобального квеста.");
        return;
    }
    
    gq = GlobalQuestManager::getThis( )->findGlobalQuest( arguments.getOneArgument( ) );
    if (!gq) {
        ch->pecho("Неправильный ID, либо квест не запущен.");
        return;
    }
    
    try {
        int newTotalTime = arguments.getOneArgument( ).toInt( );
        int minTotalTime = gq->getElapsedTime( ) + 1;
        if (newTotalTime < minTotalTime) {
            ch->pecho( "Неверное время, минимум %d минут.", minTotalTime );
            return;
        }

        gq->suspend( );
        gq->setTotalTime( newTotalTime );
        gq->resume( );
        ch->pecho( "Новое время квеста %d минут, до конца остается %d минут.",
                gq->getTotalTime( ), gq->getRemainingTime( ) );
    }  catch (const Exception &e) {
        ch->send_to( e.what( ) );
        return;
    }
}

void CGQuest::doTalk( PCharacter *ch, DLString& arguments ) 
{
    GlobalQuestInfo::Pointer gqi;
    DLString id, arg = arguments;
    
    if (arguments.empty( )) {
        ch->pecho("Сказать что?");
        return;
    }
   
    id = arguments.getOneArgument( );
    gqi = GlobalQuestManager::getThis( )->findGlobalQuestInfo( id );

    if (gqi) {
        if (arguments.empty( ))
            ch->pecho("Сказать что?");
        else 
            GQChannel::gecho( *gqi, arguments );
    }
    else 
        GQChannel::gecho( arg );
    
}

void CGQuest::doAuto( PCharacter *ch, DLString& arguments ) 
{
    std::basic_ostringstream<char> buf;
    DLString id, on;
    GlobalQuestInfo::Pointer gqi;
    bool autostart;
    int time = 0;

    if (arguments.empty( )) {
        ch->pecho("Укажите ID глобального квеста.");
        return;
    }

    id = arguments.getOneArgument( );
    gqi = GlobalQuestManager::getThis( )->findGlobalQuestInfo( id );

    if (!gqi) {
        ch->pecho("Неправильный ID.");
        return;
    }

    if (arguments.empty( )) {
        autostart = !gqi->getAutostart( );
    }
    else {
        on = arguments.getOneArgument( );
        
        if (arg_is_yes( on ) || arg_is_switch_on( on ))
            autostart = true;
        else if (arg_is_no( on ) || arg_is_switch_off( on ))
            autostart = false;
        else {
            ch->pecho( "Неправильный параметр: пишите вкл или выкл, да или нет." );
            return;
        }

        if (!arguments.empty( )) {
            try {
                time = arguments.getOneArgument( ).toInt( );
            } catch (const ExceptionBadType &e) {         
            } 
            
            if (time <= 0) {
                ch->pecho("Неправильное время.");
                return;
            }
        }
    }

    gqi->setAutostart( autostart );
    time = ( time ? time : 180 );
    gqi->setWaitingTime( time * 60 );

    if (autostart) 
        buf << "Квест " << id << " будет стартовать автоматически"
            << " с интервалом в " << time << " минут." << endl;
    else
        buf << "Квест " << id << " не будет стартовать автоматически." << endl;

    ch->send_to( buf );
}
    
void CGQuest::doRead( PCharacter *ch, DLString& arguments ) 
{
    GlobalQuestInfo::Pointer gqi;
    
    if (arguments.empty( )) {
        ch->pecho("Укажите ID глобального квеста.");
        return;
    }
    
    gqi = GlobalQuestManager::getThis( )->findGlobalQuestInfo( arguments.getOneArgument( ) );
    if (!gqi) {
        ch->pecho("Неправильный ID.");
        return;
    }
    
    try {
        GlobalQuestManager::getThis( )->load( *gqi );
    }  catch (const Exception &e) {
        ch->send_to( e.what( ) );
        return;
    }
    
    ch->pecho("Конфигурация глобального квеста обновлена.");
}

void CGQuest::doSet( PCharacter *ch, DLString &arguments )
{
    int count;
    PCMemoryInterface *pci;
    DLString name, questID, number;
    bool plus;
    XMLAttributeGlobalQuest::Pointer attr;
        
    name = arguments.getOneArgument( );
    questID = arguments.getOneArgument( ); 
    number = arguments.getOneArgument( ); 
    plus = false;

    if (name.empty( ) || questID.empty( ) || number.empty( )) {
        ch->pecho("Использование: gquest set <player> <quest id> <num. of victories>");
        return;
    }
    
    pci = PCharacterManager::find( name );

    if (!pci) {
        ch->pecho("Укажите имя правильно и полностью.");
        return;
    }
    
    if (!GlobalQuestManager::getThis( )->findGlobalQuestInfo( questID )) {
        ch->pecho("Неправильный ID.");
        return;
    }
    
    try {
        if (number.at( 0 ) == '+') {
            plus = true;
            number.erase( 0, 1 );
        }
        
        count = number.toInt( );
    } catch (const ExceptionBadType&) {
        ch->pecho("Неверное количество побед.");
        return;
    }
    
    attr = pci->getAttributes( ).getAttr<XMLAttributeGlobalQuest>( "gquest" );
    
    if (plus)
        count += attr->getVictories( questID );

    attr->setVictories( questID, count );
    PCharacterManager::saveMemory( pci );
    ch->pecho("Done.");
}

void CGQuest::usage( PCharacter *ch ) 
{
    std::basic_ostringstream<char> buf;

    buf << "{Wгквест инфо        {w - информация о текущих глобальных квестах" << endl
        << "{Wгквест прогресс    {w - прогресс каждого участника глобальных квестов" << endl
        << "{Wгквест стат        {w - показать статистику побед" << endl
        << "{Wгквест победы      {w - показать твои победы" << endl
        << "{Wгквест безопыта да {w - не получать опыт в награду" << endl
        << "{Wгквест безопыта нет{w - получать опыт в награду" << endl;

    if (!ch->is_immortal( )) {
        ch->send_to( buf );
        return;
    }

    buf << "{Wгквест список       {w - список всех существующих глобалов" << endl
        << "{Wгквест старт <id> [<min_level> <max_level>] [<time>] [<arg>] [<playerCnt>]" << endl
        << "                  {w - запуск глобала:" << endl
        << "                  {w - <id> имя глобала, список см. по гквест список" << endl
        << "                  {w - <levels> указывают диапазн уровней для квестов типа gangsters" << endl
        << "                  {w - <time> указывает длительность в минутах, по умолчанию 30" << endl
        << "                  {w - <arg> указывает имя сценария, если они поддерживаются квестом" << endl
        << "                  {w - <playerCnt> имитирует запуск квеста как будто онлайн такое кол-во игроков" << endl
        << "{Wгквест стоп <id>  {w - завершение уже запущенного квеста" << endl
        << "{Wгквест время <id> <time>{w - установить время запущенного квеста в <time> минут" << endl
        << "{Wгквест говорить <text>{w - посылка сообщения в канал [Global Quest]" << endl
        << "{Wгквест говорить <id> <text>{w - сообщение в канал [Global Quest: <имя квеста>]" << endl
        << "{Wгквест авто <id> [вкл|выкл] [<time>]" << endl
        << "                  {w - вкл./выкл. автозапуск с интервалом в <time> минут" << endl;

    if (!ch->isCoder( )) {
        ch->send_to( buf );
        return;
    }
    
    buf << "{Wгквест установить <player> <id> [+]<count>" << endl
        << "                  {w - установить чару статистику побед по этому виду квеста." << endl
        << "{Wгквест прочесть <id>  {w - обновить конфигурацию квеста (перечитать профайл)" << endl;
    ch->send_to( buf );
}

