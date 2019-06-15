/* $Id: cgquest.cpp,v 1.1.2.1.6.8 2009/09/06 21:48:28 rufina Exp $
 *
 * ruffina, 2003
 */

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
#include "mercdb.h"
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
        ch->send_to( "Тебе нельзя.\n\r" );
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
        ch->println("Сейчас нет ни одного глобального задания.");
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
    } catch (Exception e) {
        LogStream::sendError( ) << e.what( ) << endl;
        return;
    }

    if (!arguments.empty( )) {
        if (arg_is_yes( arguments )) {
            attribute->setNoExp( true );
        } else if (arg_is_no( arguments )) {
            attribute->setNoExp( false );
        } else {
            ch->println("Используй '{lEgquest noexp yes{lRгквест безопыта да{lx' или '{lEgquest noexp no{lRгквест безопыта нет{lx'.");
            return;
        }

        PCharacterManager::saveMemory( ch );
    }

    if (attribute->getNoExp( ) == true) {
        ch->send_to("Ты не будешь получать опыт как награду за глобальные квесты.\r\n" );
    } else {
        ch->send_to("Ты будешь получать опыт как награду за глобальные квесты.\r\n" );
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
                << dlprintf( "%4d", last ) << GQChannel::NORMAL << " ";

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
    char buf[MAX_STRING_LENGTH];
    GlobalQuestManager::RegistryList::iterator i;
    GlobalQuestManager::RegistryList& reg = GlobalQuestManager::getThis( )->getRegistry( );

    sprintf( buf, "%sСписок глобальных квестов Мира Грез\r\n", GQChannel::NORMAL );
    ch->send_to( buf );
   
    sprintf( buf, "%s%-10s %-10s %s %-4s %s %7s %9s %s%s\r\n",
            GQChannel::BOLD, "Название", "ID", "A", "idle", "R", "Уровни", "Время", "Описание", GQChannel::NORMAL );
    ch->send_to( buf );
    
    for (i = reg.begin( ); i != reg.end( ); i++) {
        GlobalQuestInfo::Pointer gqip = i->second;
        GlobalQuest::Pointer gq = GlobalQuestManager::getThis( )->findGlobalQuest( gqip->getQuestID( ) );

        sprintf( buf, "%s%-10s %-10s ",
                GQChannel::NORMAL,
                gqip->getQuestName( ).c_str( ),
                gqip->getQuestID( ).c_str( ));

        if (gqip->getAutostart( ))
            sprintf( buf + strlen(buf), "* %-4d", (int)(gqip->getWaitingTime( ) / 60));
        else 
            strcat( buf, "      " );

        if (gq) {
            sprintf( buf + strlen(buf), " * " );

            if (gq->hasLevels( ))
                sprintf( buf + strlen(buf), "%3d-%-3d",
                         gq->getMinLevel( ), gq->getMaxLevel( ));
            else
                sprintf( buf + strlen(buf), "%7s", "" );
            
            sprintf( buf + strlen(buf), " %4d/%-4d",
                                     gq->getElapsedTime( ), gq->getTotalTime( ));
        }
        else 
            sprintf(buf + strlen(buf), "%-2s %7s %9s", "", "", "");
        
        sprintf(buf + strlen(buf), " %s{x\r\n", gqip->getQuestShortDescr( ).c_str( ) );
        ch->send_to( buf );
    }

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
        ch->send_to( "Укажите ID глобального квеста.\r\n" );
        return;
    }
    
    gqi = GlobalQuestManager::getThis( )->findGlobalQuestInfo( arguments.getOneArgument( ) );

    if (!gqi) {
        ch->send_to( "Неправильный ID.\r\n" );
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
        ch->send_to( "Укажите ID глобального квеста.\r\n" );
        return;
    }
    
    gq = GlobalQuestManager::getThis( )->findGlobalQuest( arguments.getOneArgument( ) );
    if (!gq) {
        ch->send_to( "Неправильный ID, либо квест не запущен.\r\n" );
        return;
    }
    
    try {
        gq->scheduleDestroy( );
    }  catch (const Exception &e) {
        ch->send_to( e.what( ) );
        return;
    }
    ch->println( "Глобальный квест остановлен." );
}

void CGQuest::doTime( PCharacter *ch, DLString& arguments ) 
{
    GlobalQuest::Pointer gq;
    
    if (arguments.empty( )) {
        ch->send_to( "Укажите ID глобального квеста.\r\n" );
        return;
    }
    
    gq = GlobalQuestManager::getThis( )->findGlobalQuest( arguments.getOneArgument( ) );
    if (!gq) {
        ch->send_to( "Неправильный ID, либо квест не запущен.\r\n" );
        return;
    }
    
    try {
        int newTotalTime = arguments.getOneArgument( ).toInt( );
        int minTotalTime = gq->getElapsedTime( ) + 1;
        if (newTotalTime < minTotalTime) {
            ch->printf( "Неверное время, минимум %d минут.\r\n", minTotalTime );
            return;
        }

        gq->suspend( );
        gq->setTotalTime( newTotalTime );
        gq->resume( );
        ch->printf( "Новое время квеста %d минут, до конца остается %d минут.\r\n",
                gq->getTotalTime( ), gq->getRemainedTime( ) );
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
        ch->send_to( "Сказать что?\r\n" );
        return;
    }
   
    id = arguments.getOneArgument( );
    gqi = GlobalQuestManager::getThis( )->findGlobalQuestInfo( id );

    if (gqi) {
        if (arguments.empty( ))
            ch->send_to( "Сказать что?\r\n" );
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
        ch->send_to( "Укажите ID глобального квеста.\r\n" );
        return;
    }

    id = arguments.getOneArgument( );
    gqi = GlobalQuestManager::getThis( )->findGlobalQuestInfo( id );

    if (!gqi) {
        ch->send_to( "Неправильный ID.\r\n" );
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
            ch->println( "Неправильный параметр: пишите {lRвкл{lEon{lx или {lRвыкл{lEoff{lx, {lRда{lEyes{lx или {lRнет{lEno{lx." );
            return;
        }

        if (!arguments.empty( )) {
            try {
                time = arguments.getOneArgument( ).toInt( );
            } catch (ExceptionBadType e) {         
            } 
            
            if (time <= 0) {
                ch->send_to( "Неправильное время.\r\n" );
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
        ch->send_to( "Укажите ID глобального квеста.\r\n" );
        return;
    }
    
    gqi = GlobalQuestManager::getThis( )->findGlobalQuestInfo( arguments.getOneArgument( ) );
    if (!gqi) {
        ch->send_to( "Неправильный ID.\r\n" );
        return;
    }
    
    try {
        GlobalQuestManager::getThis( )->load( *gqi );
    }  catch (const Exception &e) {
        ch->send_to( e.what( ) );
        return;
    }
    
    ch->send_to( "Конфигурация глобального квеста обновлена.\r\n" );
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
        ch->send_to("Использование: gquest set <player> <quest id> <num. of victories>\r\n");
        return;
    }
    
    pci = PCharacterManager::find( name );

    if (!pci) {
        ch->send_to("Укажите имя правильно и полностью.\r\n");
        return;
    }
    
    if (!GlobalQuestManager::getThis( )->findGlobalQuestInfo( questID )) {
        ch->send_to("Неправильный ID.\r\n");
        return;
    }
    
    try {
        if (number.at( 0 ) == '+') {
            plus = true;
            number.erase( 0, 1 );
        }
        
        count = number.toInt( );
    } catch (const ExceptionBadType&) {
        ch->send_to("Неверное количество побед.\r\n");
        return;
    }
    
    attr = pci->getAttributes( ).getAttr<XMLAttributeGlobalQuest>( "gquest" );
    
    if (plus)
        count += attr->getVictories( questID );

    attr->setVictories( questID, count );
    PCharacterManager::saveMemory( pci );
    ch->send_to("Done.\r\n");
}

void CGQuest::usage( PCharacter *ch ) 
{
    std::basic_ostringstream<char> buf;

    buf << "{W{lEgquest{lRгквест{lx {lEinfo{lRинфо {lx       {w - информация о текущих глобальных квестах" << endl
        << "{W{lEgquest{lRгквест{lx {lEprogress{lRпрогресс {lx   {w - прогресс каждого участника глобальных квестов" << endl
        << "{W{lEgquest{lRгквест{lx {lEstat{lRстат {lx       {w - показать статистику побед" << endl
        << "{W{lEgquest{lRгквест{lx {lEvictory{lRпобеды  {lx    {w - показать твои победы" << endl
        << "{W{lEgquest{lRгквест{lx {lEnoexp{lRбезопыта{lx {lEyes  {lRда {lx{w - не получать опыт в награду" << endl
        << "{W{lEgquest{lRгквест{lx {lEnoexp{lRбезопыта{lx {lEno   {lRнет{lx{w - получать опыт в награду" << endl;

    if (!ch->is_immortal( )) {
        ch->send_to( buf );
        return;
    }

    buf << "{W{lEgquest{lRгквест{lx {lElist{lRсписок{lx       {w - список всех существующих глобалов" << endl
        << "{W{lEgquest{lRгквест{lx {lEstart{lRстарт{lx <id> [<min_level> <max_level>] [<time>] [<arg>] [<playerCnt>]" << endl
        << "                  {w - запуск глобала:" << endl
        << "                  {w - <id> имя глобала, список см. по {lEgquest list{lRгквест список{lx" << endl
        << "                  {w - <levels> указывают диапазн уровней для квестов типа gangsters" << endl
        << "                  {w - <time> указывает длительность в минутах, по умолчанию 30" << endl
        << "                  {w - <arg> указывает имя сценария, если они поддерживаются квестом" << endl
        << "                  {w - <playerCnt> имитирует запуск квеста как будто онлайн такое кол-во игроков" << endl
        << "{W{lEgquest{lRгквест{lx {lEstop{lRстоп{lx <id>  {w - завершение уже запущенного квеста" << endl
        << "{W{lEgquest{lRгквест{lx {lEtime{lRвремя{lx <id> <time>{w - установить время запущенного квеста в <time> минут" << endl
        << "{W{lEgquest{lRгквест{lx {lEtalk{lRговорить{lx <text>{w - посылка сообщения в канал [Global Quest]" << endl
        << "{W{lEgquest{lRгквест{lx {lEtalk{lRговорить{lx <id> <text>{w - сообщение в канал [Global Quest: <имя квеста>]" << endl
        << "{W{lEgquest{lRгквест{lx {lEauto{lRавто{lx <id> [{lEon|off{lRвкл|выкл{lx] [<time>]" << endl
        << "                  {w - вкл./выкл. автозапуск с интервалом в <time> минут" << endl;

    if (!ch->isCoder( )) {
        ch->send_to( buf );
        return;
    }
    
    buf << "{W{lEgquest{lRгквест{lx {lEset{lRустановить{lx <player> <id> [+]<count>" << endl
        << "                  {w - установить чару статистику побед по этому виду квеста." << endl
        << "{W{lEgquest{lRгквест{lx {lEread{lRпрочесть{lx <id>  {w - обновить конфигурацию квеста (перечитать профайл)" << endl;
    ch->send_to( buf );
}

