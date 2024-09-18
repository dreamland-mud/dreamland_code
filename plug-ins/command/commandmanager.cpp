/* $Id: commandmanager.cpp,v 1.1.6.4.6.5 2011-04-19 01:25:37 rufina Exp $
 *
 * ruffina, 2004
 * based on CommandManager by NoFate
 */
#include <algorithm>

#include "commandmanager.h"
#include "commandinterpreter.h"
#include "commandhelp.h"

#include "levenshtein.h"
#include "translit.h"
#include "dl_ctype.h"
#include "xmlfile.h"
#include "logstream.h"
#include "xmlvector.h"
#include "dbio.h"
#include "dreamland.h"
#include "character.h"

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

            const XMLStringList &aliases = (*ipos)->getRussianAliases( );
            if (find(aliases.begin( ), aliases.end( ), name) != aliases.end( ))
                return *ipos;
        }

        return Command::Pointer( );
    }

    for (ipos = commands.begin( ); ipos != commands.end( ); ipos++) {
        if ((*ipos)->getName( ) == name) 
            return *ipos;

        const XMLStringList &aliases = (*ipos)->getAliases( );
        if (find(aliases.begin( ), aliases.end( ), name) != aliases.end( ))
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

            for (auto &alias: cmd->getRussianAliases())
                if (name.strPrefix(alias))
                    return cmd;
        }

        return Command::Pointer();
    }

    for (auto &cmd: commands) {
        if (name.strPrefix(cmd->getName()))
            return *cmd;

        for (auto &alias: cmd->getAliases())
            if (name.strPrefix(alias))
                return cmd;
    }

    return Command::Pointer();


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

void CommandList::gatherHints(InterpretArguments &iargs) const
{
    list<Command::Pointer>::const_iterator c;
    XMLStringList::const_iterator a;
    const DLString &cmd = iargs.cmdName;
    DLString kuzdn = translit(cmd);

    for (c = commands.begin(); c != commands.end(); c++) {
        if ((*c)->visible(iargs.ch)) {
            record_distance(cmd, kuzdn, (*c)->getName(), iargs);

            for (a = (*c)->getAliases().begin(); a != (*c)->getAliases().end(); a++) 
                record_distance(cmd, kuzdn, *a, iargs);
            
            for (a = (*c)->getRussianAliases().begin(); a != (*c)->getRussianAliases().end(); a++) 
                record_distance(cmd, kuzdn, *a, iargs);
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

    if (command->getHelp( ))
        command->getHelp( )->setCommand( command );
}

void CommandManager::unregistrate( Command::Pointer command )
{
    if (command->getHelp( ))
        command->getHelp( )->unsetCommand( );
    
    commands.remove( command );
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
    iargs.pCommand = commands.chooseCommand( iargs.ch, iargs.cmdName );

    if (iargs.pCommand) 
        iargs.advance();
    else if (iargs.cmdName.size() >= 3) 
        commands.gatherHints(iargs);

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
