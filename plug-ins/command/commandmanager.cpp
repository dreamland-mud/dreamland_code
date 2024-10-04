/* $Id: commandmanager.cpp,v 1.1.6.4.6.5 2011-04-19 01:25:37 rufina Exp $
 *
 * ruffina, 2004
 * based on CommandManager by NoFate
 */
#include <algorithm>
#include "jsoncpp/json/value.h"
#include "commandmanager.h"
#include "commandinterpreter.h"
#include "commandhelp.h"

#include "string_utils.h"
#include "configurable.h"
#include "levenshtein.h"
#include "translit.h"
#include "dl_ctype.h"
#include "xmlfile.h"
#include "logstream.h"
#include "xmlvector.h"
#include "dbio.h"
#include "dreamland.h"
#include "character.h"

// Maps a command or alias name to its position inside priority JSON file.
// Used for sorting commands in a list.
struct priority_t : public map<DLString, int> {
    void fromJson(const Json::Value &value)
    {
        this->clear();

        for (unsigned int i = 0; i < value.size(); i++) {
            DLString cmdName = value[i].asString();
            int position = i;
            (*this)[cmdName] = position;
        }
    }
}; 

// Keeps priorities for all languages.
map<lang_t, priority_t> priorities; 

CONFIGURABLE_LOADED(prio, commands_en)
{
    priorities[EN].fromJson(value);

    commandManager->multiCommands.refreshSorted(EN);
}

CONFIGURABLE_LOADED(prio, commands_ua)
{
    priorities[UA].fromJson(value);

    commandManager->multiCommands.refreshSorted(UA);
}

CONFIGURABLE_LOADED(prio, commands_ru)
{
    priorities[RU].fromJson(value);

    commandManager->multiCommands.refreshSorted(RU);
}

void MultiCommandList::refreshSorted(lang_t lang)
{
    // Put everything back in alphabetical (sometimes) order.
    sortedNames[lang].clear();
    for (auto &alias2name: names[lang])
        sortedNames[lang].push_back(alias2name.first);

    // Get numeric priority of this command's alias.
    auto find_my_prio = [&](const DLString &alias) {
        auto &prio = priorities[lang];

        // First look for exactly this alias in the priorities file.
        auto p = prio.find(alias);
        if (p != prio.end())
            return p->second;

        // If not found, see if priorities file had an entry for the main command name.
        const DLString &cmdName = names[lang][alias];
        auto p_cmd = prio.find(cmdName);
        if (p_cmd != prio.end())
            return p_cmd->second;

        // Assume lowest priority possible.
        return 1000;
    };

    // Define comparator for the two command aliases according to their priorities.
    auto compare_by_prio = [&](const DLString &a, const DLString &b) {
        int prio_a = find_my_prio(a);
        int prio_b = find_my_prio(b);
        return prio_a < prio_b;
    };

    // Launch the sorting.
    sortedNames[lang].sort(compare_by_prio);
}

void MultiCommandList::addCommand(Command::Pointer &cmd)
{
    // Sanity checks
    if (masterMap.find(cmd->getName()) != masterMap.end())
        throw Exception("Command " + cmd->getName() + " already registered");

    // Put the command into the master storage.
    masterMap[cmd->getName()] = cmd;

    // For all command names and aliases, reference the master entry from the 'names' map.
    for (int i = LANG_MIN; i < LANG_MAX; i++) {
        lang_t lang = (lang_t)i;

        if (cmd->name[lang].empty())
            continue;

        names[lang][cmd->name[lang]] = cmd->getName();

        for (auto &alias: cmd->aliases[lang].split(" ")) {
            names[lang][alias] = cmd->getName();
        }

        // Re-populate sorted lists after adding this command.
        refreshSorted(lang);
    }
}

void MultiCommandList::removeCommand(Command::Pointer &cmd)
{
    for (int i = LANG_MIN; i < LANG_MAX; i++) {
        lang_t lang = (lang_t)i;

        if (cmd->name[lang].empty())
            continue;

        auto it = names[lang].find(cmd->name[lang]);
        if (it != names[lang].end())
            names[lang].erase(it);

        sortedNames[lang].remove(cmd->name[lang]);

        for (auto &alias: cmd->aliases[lang].split(" ")) {
            auto it = names[lang].find(alias);
            if (it != names[lang].end())
                names[lang].erase(it);

            sortedNames[lang].remove(alias);
        }
    }
   
    auto it = masterMap.find(cmd->getName());
    if (it == masterMap.end())
        throw Exception("Command " + cmd->getName() + " already de-registered");

    masterMap.erase(it);
}

Command::Pointer MultiCommandList::sortedLookup(Character *ch, const DLString &input, lang_t lang)
{
    auto &sortedList = sortedNames[lang];

    for (auto &alias: sortedList) {
        if (input.strPrefix(alias)) {
            // Resolve real command name from its alias.
            const DLString &cmdName = names[lang][alias];

            // Check availability and return a match.
            Command::Pointer cmd = masterMap[cmdName];
            if (cmd->available(ch))
                return cmd;
        }
    }

    return Command::Pointer();
}

Command::Pointer MultiCommandList::chooseCommand(Character *ch, const DLString &input)
{
    lang_t guess_1, guess_2;

    if (!String::hasCyrillic(input)) {
        // No cyrillic letters - look for EN command, regardless of config.
        guess_1 = guess_2 = EN;

    } else {
        // TODO override default order according to config.lang settings
        guess_1 = RU;
        guess_2 = UA;

        // If a language-specific symbol gives the lang away, only look for commands in this language.
        if (String::hasUaSymbol(input))
            guess_1 = guess_2 = UA;
        else if (String::hasRuSymbol(input))
            guess_1 = guess_2 = RU;
    }

    // Do a command lookup for both guessed languages.
    Command::Pointer cmd = sortedLookup(ch, input, guess_1);
    if (cmd)
        return cmd;

    if (guess_1 == guess_2)
        return cmd;

    return sortedLookup(ch, input, guess_2);
}

static void record_distance(const DLString &cmd, const DLString &kuzdn, const DLString &candidate, InterpretArguments &iargs)
{
    DLString string2;
 
    if (cmd.size() < candidate.size())
        string2 = candidate.substr(0, cmd.size());
    else
        string2 = candidate;

    int distance = levenshtein(cmd, string2, 1, 2, 1, 1);
    if (distance <= 1)
        iargs.hints1.push_back(candidate);
    else if (distance == 2)
        iargs.hints2.push_back(candidate);

    if (kuzdn.strPrefix(candidate))
        iargs.translit.push_back(candidate);
}


void MultiCommandList::gatherHints(InterpretArguments &iargs)
{
    const DLString &cmdName = iargs.cmdName;
    DLString kuzdn = translit(cmdName);

    for (auto &it: masterMap) {
        Command::Pointer cmd = it.second;

        if (cmd->visible(iargs.ch)) {

            for (int i = LANG_MIN; i < LANG_MAX; i++) {
                lang_t lang = (lang_t)i;

                if (cmd->name[lang].empty())
                    continue;  

                record_distance(cmdName, kuzdn, cmd->name[lang], iargs);

                for (auto &alias: cmd->aliases[lang].split(" ")) 
                    record_distance(cmdName, kuzdn, alias, iargs);
            }
        }
    }
}


































/*-----------------------------------------------------------------------
 * CommandList
 *-----------------------------------------------------------------------*/
Command::Pointer CommandList::findExact( const DLString& name ) const
{
    list<Command::Pointer>::const_iterator ipos;

    if (name.isCyrillic( )) {
        for (ipos = commands_ru.begin( ); ipos != commands_ru.end( ); ipos++) {
            if ((*ipos)->getRussianName( ) == name) 
                return *ipos;

            for (auto &alias: (*ipos)->aliases.get(RU).split(" "))
                if (name == alias)
                    return *ipos;
        }

        return Command::Pointer( );
    }

    for (ipos = commands.begin( ); ipos != commands.end( ); ipos++) {
        if ((*ipos)->getName( ) == name) 
            return *ipos;

        for (auto &alias: (*ipos)->aliases.get(EN).split(" "))
            if (name == alias)
                return *ipos;
    }

    return Command::Pointer( );
}


Command::Pointer CommandList::findUnstrict(const DLString& name) const
{
    if (name.empty())
        return Command::Pointer();

    if (name.isCyrillic( )) {
        for (auto &cmd: commands_ru) {
            if (name.strPrefix(cmd->getRussianName()))
                return *cmd;

            for (auto &alias: cmd->aliases.get(RU).split(" "))
                if (name.strPrefix(alias))
                    return cmd;
        }

        return Command::Pointer();
    }

    for (auto &cmd: commands) {
        if (name.strPrefix(cmd->getName()))
            return *cmd;

        for (auto &alias: cmd->aliases.get(EN).split(" "))
            if (name.strPrefix(alias))
                return cmd;
    }

    return Command::Pointer();


}

void CommandList::gatherHints(InterpretArguments &iargs) const
{
    list<Command::Pointer>::const_iterator c;
    const DLString &cmd = iargs.cmdName;
    DLString kuzdn = translit(cmd);

    for (c = commands.begin(); c != commands.end(); c++) {
        if ((*c)->visible(iargs.ch)) {
            record_distance(cmd, kuzdn, (*c)->getName(), iargs);

            for (auto &a: (*c)->aliases.get(EN).split(" ")) 
                record_distance(cmd, kuzdn, a, iargs);

            record_distance(cmd, kuzdn, (*c)->getRussianName(), iargs);

            for (auto &a: (*c)->aliases.get(RU).split(" ")) 
                record_distance(cmd, kuzdn, a, iargs);
        }
    }
}

Command::Pointer CommandList::chooseCommand( Character *ch, const DLString &name ) const
{
    list<Command::Pointer>::const_iterator i;
    const list<Command::Pointer> &mylist = name.isCyrillic( ) ? commands_ru : commands;

    for (i = mylist.begin( ); i != mylist.end( ); i++) {
        Command::Pointer pCommand = *i;

        if (pCommand->available( ch ) && pCommand->matches( name ))  {
            return pCommand;
        }
    }

    for (i = mylist.begin( ); i != mylist.end( ); i++) {
        Command::Pointer pCommand = *i;

        if (pCommand->available( ch ) && pCommand->matchesAlias( name ))  {
            return pCommand;
        }
    }

    return Command::Pointer( );
}

static bool compare( Command::Pointer a, Command::Pointer b )
{
    return commandManager->compare( **a, **b, false );
}

static bool compare_ru( Command::Pointer a, Command::Pointer b )
{
    return commandManager->compare( **a, **b, true );
}

void CommandList::add( Command::Pointer &cmd )
{
    commands.push_back( cmd ); 
    commands.sort( compare );
    
    if (!cmd->getRussianName( ).empty( )) {
        commands_ru.push_back( cmd );
        commands_ru.sort( compare_ru );
    }
}

void CommandList::remove( Command::Pointer &cmd )
{
    commands.remove( cmd );
    commands_ru.remove( cmd );
}

/*-----------------------------------------------------------------------
 * CommandManager
 *-----------------------------------------------------------------------*/
const DLString CommandManager::PRIO_FILE_EN = "cmdpriority.xml";
const DLString CommandManager::PRIO_FILE_RU = "cmdpriority_ru.xml";
CommandManager* commandManager = NULL;

CommandManager::CommandManager( ) 
{
    checkDuplicate( commandManager );
    commandManager = this;
}

CommandManager::~CommandManager( )
{
    commandManager = NULL;
}

void CommandManager::initialization( )
{
    loadPriorities( );
    InterpretLayer::initialization( );
}

DLString CommandManager::getPrioritiesFolder() const
{
    return dreamland->getTablePath() + "/commands";
}

void CommandManager::destruction( )
{
    InterpretLayer::destruction( );
}

void CommandManager::registrate( Command::Pointer command )
{
    commands.add( command );
    multiCommands.addCommand(command);

    if (command->getHelp( ))
        command->getHelp( )->setCommand( command );
}

void CommandManager::unregistrate( Command::Pointer command )
{
    if (command->getHelp( ))
        command->getHelp( )->unsetCommand( );
    
    commands.remove( command );
    multiCommands.removeCommand(command);
}

void CommandManager::loadPriorities( )
{
    XMLVectorBase<XMLString> v, rv;                                                 
    DLFile prioFileEN(getPrioritiesFolder(), PRIO_FILE_EN);
    DLFile prioFileRU(getPrioritiesFolder(), PRIO_FILE_RU);

    if (!XMLFile(prioFileEN, "", &v).load()) {
        LogStream::sendError( ) << "Command priorities file not found!" << endl;
        return;
    }

    XMLFile(prioFileRU, "", &rv).load();

    for (unsigned int i = 0; i < v.size( ); i++)
        priorities_en[v[i].getValue( )] = i;

    for (unsigned int i = 0; i < rv.size( ); i++)
        priorities_ru[rv[i].getValue( )] = i;

    LogStream::sendNotice( ) 
        << "Loaded " << priorities_en.size( ) << " command priorities" << endl;
}

void CommandManager::putInto( )
{
    interp->put( this, CMDP_FIND, 10 );        
}

bool CommandManager::process( InterpretArguments &iargs )
{
    iargs.pCommand = multiCommands.chooseCommand( iargs.ch, iargs.cmdName );

    if (iargs.pCommand) 
        iargs.advance();
    else if (iargs.cmdName.size() >= 3) 
        multiCommands.gatherHints(iargs);

    return true;
}

Command::Pointer CommandManager::find(const DLString &cmdName) const
{
    for (auto &c: commands.getCommands())
        if (c->getName() == cmdName)
            return *c;
            
    return Command::Pointer();
}

Command::Pointer CommandManager::findExact( const DLString &cmdName ) const
{
    return commands.findExact( cmdName );
}

Command::Pointer CommandManager::findUnstrict( const DLString &cmdName ) const
{
    return commands.findUnstrict( cmdName );
}

CommandManager::CategoryMap CommandManager::getCategorizedCommands( ) const
{
    CategoryMap cats;

    list<Command::Pointer>::const_iterator cmd;
    for (cmd = commands.getCommands( ).begin( ); cmd != commands.getCommands( ).end( ); cmd++) 
        cats[(*cmd)->getCommandCategory()].push_back(*cmd);

    return cats;
}

bool CommandManager::compare( const Command &a, const Command &b, bool fRussian ) const
{       
    Priorities::const_iterator i_a, i_b, i_end;
    const Priorities &prio = fRussian ? priorities_ru : priorities_en;

    i_a = prio.find( fRussian ? a.getRussianName( ) : a.getName( ) );
    i_b = prio.find( fRussian ? b.getRussianName( ) : b.getName( ) );
    i_end = prio.end( );

    if (i_a != i_end && i_b != i_end)
        return (i_a->second < i_b->second);

    return (i_a != i_end);  
}
