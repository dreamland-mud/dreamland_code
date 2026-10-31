/* $Id: cquest.cpp,v 1.1.4.8.6.7 2009/02/15 01:44:56 rufina Exp $
 *
 * ruffina, 2003
 */

#include "class.h"

#include "core/behavior/behavior_utils.h"
#include "pcharacter.h"
#include "npcharacter.h"
#include "pcharactermanager.h"
#include "room.h"
#include "hometown.h"

#include "feniamanager.h"
#include "wrappermanagerbase.h"
#include "wrapperbase.h"
#include "register-impl.h"
#include "lex.h"
#include "subr.h"
#include "native.h"
#include "msgformatter.h"
#include "occupations.h"
#include "occupations.h"
#include "act.h"
#include "merc.h"
#include "loadsave.h"


// Before questor.h: it pulls in def.h, whose "#define C 4" breaks the
// template<typename C> in xmlattributeplugin.h.
#include "xmlattributequestdata.h"
#include "cquest.h"
#include "quest.h"
#include "questregistrator.h"
#include "questmanager.h"
#include "questor.h"
#include <vector>
#include "arg_utils.h"
#include "def.h"
#include "l10n.h"

HOMETOWN(frigate);
enum {
    QCMD_NONE = 0,
    QCMD_BUY,
    QCMD_LIST,
    QCMD_TROUBLE,
    QCMD_REQUEST,
    QCMD_CANCEL,
    QCMD_COMPLETE,
    QCMD_FIND
};        

static const char * qcmd_names [] = {
    "", "buy", "list", "trouble", "request", "cancel", "complete", "find"
};

static void mprog_quest( Character *questor, Character *client, const DLString &cmd, const DLString &args )
{
    FENIA_VOID_CALL(questor, "Quest", "Css", client, cmd.c_str(), args.c_str());
    FENIA_NDX_VOID_CALL(questor->getNPC(), "Quest", "CCss", questor, client, cmd.c_str(), args.c_str());
}

static bool gprog_quest( PCharacter *ch, const DLString &cmd, const DLString &arg )
{
    static Scripting::IdRef ID_TMP( "tmp" ), ID_QUEST( "quest" );
    Scripting::IdRef ID_CMD( cmd );
    Scripting::Register tmpQuest;
    Scripting::RegisterList regList;

    if (!FeniaManager::wrapperManager)
        return false;

    try {
        tmpQuest = *(*Scripting::Context::root[ID_TMP])[ID_QUEST];
        regList.push_back( FeniaManager::wrapperManager->getWrapper( ch ) );
        regList.push_back(Scripting::Register(arg));
        return tmpQuest[ID_CMD]( regList ).toBoolean( );
    }
    catch (const Exception &ex) {
        FeniaManager::getThis()->croak(0, Scripting::Register(".tmp.quest"), ex);
        return false;
    } 
}

static DLString join_words( const std::vector<DLString> &words, size_t from, size_t to )
{
    DLString out;

    for (size_t i = from; i < to; i++) {
        if (!out.empty( ))
            out += " ";
        out += words[i];
    }

    return out;
}

/** The questor pch talks to. A trailing "from/у <name>" naming a questor here
 *  picks it and is cut off arguments. Otherwise the first questor that can see
 *  pch and is not fighting, else the first one in the room (its own refusal
 *  then explains why). */
static Questor::Pointer find_questor( PCharacter *pch, DLString &arguments )
{
    Questor::Pointer named, ready, first;
    std::vector<DLString> words;
    DLString rest = arguments;

    while (!rest.empty( )) {
        DLString w = rest.getOneArgument( );
        if (!w.empty( ))
            words.push_back( w );
    }

    for (size_t i = 0; i + 1 < words.size( ); i++) {
        if (!arg_is_strict( words[i], "from" ) && !arg_is_strict( words[i], "in" ))
            continue;

        Character *rch = get_char_room( pch, join_words( words, i + 1, words.size( ) ) );
        NPCharacter *mob = rch ? rch->getNPC( ) : 0;

        if (mob && mob->behavior
            && IS_SET( mob->behavior->getOccupation( ), (1 << OCC_QUEST_MASTER) ))
            named = mob->behavior.getDynamicPointer<Questor>( );

        if (named) {
            arguments = join_words( words, 0, i );
            return named;
        }
    }

    for (Character *rch = pch->in_room->people; rch; rch = rch->next_in_room) {
        NPCharacter *mob = rch->getNPC( );
        if (!mob || !mob->behavior
            || !IS_SET( mob->behavior->getOccupation( ), (1 << OCC_QUEST_MASTER) ))
            continue;

        Questor::Pointer q = mob->behavior.getDynamicPointer<Questor>( );
        if (!q)
            continue;

        if (!first)
            first = q;

        if (!ready && !mob->fighting && mob->can_see( pch ))
            ready = q;
    }

    return ready ? ready : first;
}

static void see_also( PCharacter *pch )
{
    pch->pecho( _("Смотри также {y{hcквест ?{x для списка всех возможных действий.") );
}

COMMAND(CQuest, "quest")
{
    Questor::Pointer questman;
    DLString arguments = constArguments;
    DLString cmd = arguments.getOneArgument( );
    PCharacter *pch = ch->getPC( );
        
    if (!pch)
        return;

    if (IS_GHOST( pch )) {
        pch->pecho(_("Наслаждение жизнью недоступно призракам."));
        return;
    }

    // Parse commands that can be done anywhere.
    if (cmd.empty() || cmd.isNumber()) {
        // Syntax: 'quest'
        //         'quest <number>'
        doSummary(pch, cmd);        
        return;
    }
    else if (arg_is_info(cmd)) {
        // Syntax: 'quest info' legacy call
        doInfo(pch);
        return;
    }
    else if (arg_is(cmd, "points")) {
        doPoints( pch );
        return;
    }
    else if (arg_is_time( cmd )) {
        doTime( pch );
        return;
    }
    else if (arg_is(cmd, "stat")) {
        doStat( pch );
        return;
    }
    else if (arg_is(cmd, "set") && pch->isCoder( )) {
        doSet( pch, arguments );
        return;
    } else if (arg_is_help( cmd )) {
        usage( pch );
        return;
    }
   
    int qcmd = QCMD_NONE;

    // Quest trade lives in Fenia (utils/questtrade, command/quest/runFunc).
    // Reaching here means the Fenia engine is not loaded.
    if (arg_is_list( cmd ) || arg_is(cmd, "buy") || arg_is(cmd, "trouble")) {
        pch->pecho(_("Торговля квестовыми наградами временно закрыта."));
        return;
    }
   
    // Parse questor commands.
    if (arg_is(cmd, "request")) 
        qcmd = QCMD_REQUEST;
    else if (arg_is(cmd, "complete")) 
        qcmd = QCMD_COMPLETE;
    else if (arg_is(cmd, "cancel")) 
        qcmd = QCMD_CANCEL;
    else if (arg_is(cmd, "find")) 
        qcmd = QCMD_FIND;

    if (qcmd == QCMD_NONE) {
        usage( pch ); 
        return;
    }

    questman = find_questor( pch, arguments );

    // 'quest cancel <number>' is handled separately by Fenia.
    if (qcmd == QCMD_CANCEL && !arguments.empty()) {
        gprog_quest(pch, "cancel", arguments.getOneArgument());
        return;
    }

    if (!questman) {
        if (pch->getHometown( ) != home_frigate) {
            switch (qcmd) {
                case QCMD_CANCEL:
                    pch->pecho(_("Для отмены задания квестора необходимо стоять рядом с ним."));
                    break;
                default:
                    pch->pecho(_("Эту команду можно выполнить только рядом с квестором."));
                    break;
            }
            see_also( pch );
            return;
        }

        // Special handling for newbie quests. TODO: add Fenia triggers for each command.
        switch(qcmd) {
            case QCMD_CANCEL:
                pch->pecho(_("Твое задание не нужно отменять, оно исчезнет как только ты сойдешь с корабля."));
                break;
            case QCMD_REQUEST:
            case QCMD_FIND:
            case QCMD_COMPLETE:
                pch->pecho(_("Эта команда недоступна пока ты на корабле."));
                pch->pecho(_("Список текущих квестов показывает команда {yквест{x."));
                break;
        }
        return;
    }

    if (!questman->canGiveQuest( pch ))
        return;
   
    // Execute questor commands.
    switch(qcmd) {
        case QCMD_REQUEST:
            questman->doRequest( pch, arguments );
            break;
        case QCMD_COMPLETE:
            questman->doComplete( pch, arguments );
            break;
        case QCMD_CANCEL:
            questman->doCancel( pch );
            break;
        case QCMD_FIND:
            questman->doFind( pch );
            break;
        default:
            return;
    }

    mprog_quest(questman->getChar(), pch, qcmd_names[qcmd], arguments);
}

void CQuest::autoQuestInfo(PCharacter *ch, ostringstream &buf)
{
    // Output questor's quest info to buf.
    int time;
    Quest::Pointer quest;
    
    quest = ch->getAttributes( ).findAttr<Quest>( "quest" );
    time = ch->getAttributes( ).getAttr<XMLAttributeQuestData>( "questdata" )->getTime( );

    if (!quest) {
        if (ch->getAttributes( ).isAvailable( "quest" )) 
            buf << fmt(ch, _("Твое задание невозможно ни выполнить, ни отменить.")) << endl;
    } else {
        // A quest whose scenario is missing from the config throws
        // QuestRuntimeException out of getScenario, and nothing between here and
        // main() catches it: Scheduler::tick rethrows, DreamLand::run does not
        // catch, so an uncaught throw takes the process down for everyone -- from
        // the most ordinary command in the game. Contain it and tell the player
        // to cancel. Built into a scratch stream so a throw part way through
        // cannot leave a torn half-line in the real output.
        ostringstream info;

        try {
            quest->info( info, ch );
        } catch (const ::Exception &e) {
            LogStream::sendError( ) << "Broken quest for " << ch->getName( )
                                    << ": " << e.what( ) << endl;
            buf << fmt(ch, _("Твое задание испорчено, его придется отменить. См. {y{hcквест ?{x.")) << endl;
            return;
        }

        buf << info.str( );
        buf << fmt(ch, _("У тебя {Y%1$d{x минут%1$Iа|ы| на выполнение задания."), time) << endl;
    }
}

void CQuest::doInfo( PCharacter *ch ) 
{    
    // Legacy 'quest info' command shows auto-quests:
    ostringstream buf;
    autoQuestInfo(ch, buf);
    
    if (buf.str().empty())
        ch->pecho(_("У тебя сейчас нет задания от квестора. См. также команду {y{hcквест{x."));
    else
        ch->send_to(buf);
}

void CQuest::doSummary( PCharacter *ch, const DLString &arguments ) 
{    
    // Output questor's quest info to buf.
    ostringstream buf;
    autoQuestInfo(ch, buf);
    bool autoquest = !buf.str().empty();

    // Output Fenia quest list directly to char.
    bool feniaquest = gprog_quest(ch, "newinfo", arguments);

    // 'q <number>' command is fully handled in Fenia.
    if (!arguments.empty())
        return;

    // No active Fenia quest or questor's quest. 
    if (!feniaquest && !autoquest) {
        ch->pecho(_("У тебя сейчас нет задания. Подробности читай по команде {y{hcсправка квесты{x."));
        return;
    }

    // Space between fenia quests and questor's quest:
    if (feniaquest && autoquest)
        ch->pecho("{x");

    // Questor's quest and footer:
    if (autoquest) {
        ch->pecho(_("{WЗадание квестора{x"));
        ch->send_to(buf);
        ch->pecho(_("{wКоманды{x: {y{hcквест сдать{x, {y{hcквест сдать опыт{x, {y{hcквест найти{x, {y{hcквест отменить{x."));
    }
}

void CQuest::doPoints( PCharacter *ch )  
{
    int points = ch->getQuestPoints();

    ch->pecho(_("У тебя {Y%1$d{x квестов%1$Iая|ых|ых единиц%1$Iа|ы|."), points);
}

void CQuest::doTime( PCharacter *ch ) 
{
    std::basic_ostringstream<char> buf;
    int time;
    Quest::Pointer quest;
    
    quest = ch->getAttributes( ).findAttr<Quest>( "quest" );
    time = ch->getAttributes( ).getAttr<XMLAttributeQuestData>( "questdata" )->getTime( );
    
    if (!quest) {
        if (ch->getAttributes( ).isAvailable( "quest" )) 
            buf << fmt(ch, _("Твое задание невозможно ни выполнить, ни отменить.")) << endl;
        else {
            buf << fmt(ch, _("У тебя сейчас нет задания.")) << endl;
            
            if (time > 1) 
                buf << fmt(ch, _("До того, как ты снова сможешь получить задание, {Y%1$d{x минут%1$Iа|ы|."), time) << endl;
            else if (time == 1) 
                buf << fmt(ch, _("Осталось меньше минуты до того, как ты снова сможешь получить задание.")) << endl;
        }
        
    }
    else {
         buf << fmt(ch, _("У тебя {Y%1$d{x минут%1$Iа|ы| на выполнение задания."), time) << endl;
    }

    ch->send_to( buf );
}

void CQuest::doSet( PCharacter *ch, DLString& arguments )
{
    int count;
    PCMemoryInterface *pci;
    DLString name, questID, number;
    bool plus;
    XMLAttributeQuestData::Pointer attr;
    QuestRegistratorBase::Pointer qbase;
        
    name = arguments.getOneArgument( );
    questID = arguments.getOneArgument( ); 
    number = arguments.getOneArgument( ); 
    plus = false;
    
    if (name == "clear") {
        PCharacterMemoryList::const_iterator i;
        const PCharacterMemoryList &pcm = PCharacterManager::getPCM( );
        
        for (i = pcm.begin( ); i != pcm.end( ); i++) {
            XMLAttributeQuestData::Pointer attr;
            PCMemoryInterface *pc;

            pc = i->second;
            attr = pc->getAttributes( ).findAttr<XMLAttributeQuestData>( "questdata" );

            if (attr) {
                XMLAttributeStatistic::Victories::const_iterator j;
                const XMLAttributeStatistic::Victories &v = attr->getVictories( );

                for (j = v.begin( ); j != v.end( ); j++)
                    attr->setVictories( j->first, 0 );

                PCharacterManager::saveMemory( pc );
            }
        }
        
        return;
    }
    
    if (name == "reload") {
        int reloaded = QuestManager::getThis( )->reload( );

        // Numbered conversions on purpose: a translation that drops an
        // un-numbered %-code shifts the argument list and segfaults.
        ch->pecho(_("Перечитано конфигов автоквестов: %1$d из %2$d."),
                  reloaded, (int)QuestManager::getThis( )->size( ));
        // Say what reload cannot do, because the surprising half is the silent
        // one: a node deleted from the file is simply never read, so the old
        // value stays live until the next boot.
        ch->pecho(_("Перечитывание только обновляет -- удаленные из файла узлы и сценарии живут до ребута."));
        return;
    }

    if (name.empty( ) || questID.empty( ) || number.empty( )) {
        ch->pecho(_("Использование: quest set <player> <quest id> [+]<num. of victories>"));
        ch->pecho(_("               quest set reload -- перечитать quests/*.xml с диска"));
        return;
    }

    pci = PCharacterManager::find( name );

    if (!pci) {
        ch->pecho(_("%s: имя не найдено."), name.c_str( ));
        return;
    }
    
    qbase = QuestManager::getThis( )->findQuestRegistrator( questID );

    if (!qbase) {
        ch->pecho(_("Неправильный ID."));
        return;
    }
    
    try {
        if (number.at( 0 ) == '+') {
            plus = true;
            number.erase( 0, 1 );
        }
        
        count = number.toInt( );
    } catch (const ExceptionBadType&) {
        ch->pecho(_("Неверное количество побед."));
        return;
    }
   
    attr = pci->getAttributes( ).getAttr<XMLAttributeQuestData>( "questdata" );
    
    if (plus)
        count += attr->getVictories( qbase->getName( ) );

    attr->setVictories( qbase->getName( ), count );
    PCharacterManager::saveMemory( pci );
    ch->pecho("Done.");
}

void CQuest::doStat( PCharacter *ch )
{
    ostringstream buf;
    XMLAttributeStatistic::Statistic stat;
    XMLAttributeStatistic::Statistic::iterator s;
    
    stat = XMLAttributeStatistic::gatherAll( "questdata" );

    buf << fmt(ch, _("{wЛучшие квестодеятели Мира Мечты: {x")) << endl;

    for (s = stat.begin( ); s != stat.end( ); s++) {
        XMLAttributeStatistic::StatRecordList::iterator r;
        QuestRegistratorBase::Pointer qb;
        int last = 0, cnt = 0;
        
        qb = QuestManager::getThis( )->findQuestRegistrator( s->first );

        if (!qb)
            continue;

        bool foundSelf = false;
        XMLAttributeStatistic::StatRecordList &records = s->second;
        
        buf << "{W\"" << qb->getShortDescr( viewerLang(ch) ) << "\"{x" << endl;

        // Print top 5 achievers for the current quest type.
        for (r = records.begin( ); r != records.end( ) && cnt < 5; cnt++) {
            last = r->second;
            buf << fmt(0, "          {W%4d{w ", last ); 

            for ( ; r != records.end( ) && r->second == last; r++) {
                buf << r->first << " ";

                if (r->first == ch->getName())
                    foundSelf = true;
            }

            buf << "{x" << endl;
        }

        // Output your own position in the list, unless you're part of the top 5.
        if (!foundSelf)
            for (; r != records.end(); r++, cnt++)
                if (r->first == ch->getName()) {
                    buf << "               {W....{w " << endl;
                    buf << fmt(ch, _("          {W%4d{w %s (%dе место)\r\n"),
                                     r->second, r->first.c_str(), cnt+1);
                    break;
                }
        
        buf << endl;
    }

    ch->send_to( buf );
}

void CQuest::usage( PCharacter *ch ) 
{
    ostringstream buf;

    buf << fmt(ch, _("Укажи одну из квестовых команд:")) << endl << "    "
        << fmt(ch, _("очки инфо время попросить сдать список купить вернуть найти отменить стат.")) 
        << endl
        << fmt(ch, _("Также смотри {W? квесты{x.")) << endl;
    ch->send_to( buf );
}

    
