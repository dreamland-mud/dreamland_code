/* $Id: commandmanager.h,v 1.1.6.4.6.4 2011-04-19 01:25:37 rufina Exp $
 *
 * ruffina, 2004
 * based on CommandManager by NoFate
 */

#ifndef COMMANDMANAGER_H
#define COMMANDMANAGER_H

#include <map>
#include <list>

#include "oneallocate.h"
#include "interpretlayer.h"
#include "commandloader.h"
#include "command.h"

class Character;

// A command storage with support for multi-language input.
struct MultiCommandList {
        // Picks an available command for this character's input.
        Command::Pointer chooseCommand(Character *ch, const DLString &input);

        // Guess what the player wanted to type from existing input.
        void gatherHints(InterpretArguments &iargs);

        // Put a new command into storage, populate maps, re-sort the name lists.
        void addCommand(Command::Pointer &cmd);

        // Remove entries for a command from everywhere.
        void removeCommand(Command::Pointer &cmd);

        // Main command storage. Maps command name (EN) to the command instance.
        map<DLString, Command::Pointer> masterMap;        

        // For each lang, maps from a command name or alias to corresponding EN command name inside masterMap.
        map<lang_t, map<DLString, DLString> > names;

        // For each lang, keeps a sorted list of names&aliases from the 'names' maps. Sorting is done based on priorities files.
        map<lang_t, list<DLString> > sortedNames;

        // Look through sorted list of command and aliases names for the given language.
        Command::Pointer sortedLookup(Character *ch, const DLString &input, lang_t lang);

        // Re-populate sorted list of names and aliases for this lang.
        void refreshSorted(lang_t lang);
};

class CommandList {
public:
        Command::Pointer findExact( const DLString & ) const;
        Command::Pointer findUnstrict( const DLString & ) const;
        Command::Pointer chooseCommand( Character *, const DLString & ) const;
        void gatherHints(InterpretArguments &iargs) const;

        void add( Command::Pointer & );
        void remove( Command::Pointer & );
        inline const list<Command::Pointer> &getCommands( ) const;
        inline const list<Command::Pointer> &getCommandsRU( ) const;

protected:        
        list<Command::Pointer> commands;
        list<Command::Pointer> commands_ru;
};

inline const list<Command::Pointer> &CommandList::getCommands( ) const
{
    return commands;
}

inline const list<Command::Pointer> &CommandList::getCommandsRU( ) const
{
    return commands_ru;
}

/** Keeps track of all the interpretable commands in the world, processes user input. */
class CommandManager : public InterpretLayer,
                       public OneAllocate 
{
public:
        typedef ::Pointer<CommandManager> Pointer;
        typedef std::map<DLString, int> Priorities;
        typedef std::list<Command::Pointer> CategoryCommands;
        typedef std::map<bitnumber_t, CategoryCommands> CategoryMap;

public:
        CommandManager( );
        virtual ~CommandManager( );
        
        void registrate( Command::Pointer );
        void unregistrate( Command::Pointer );
        
        Command::Pointer findExact( const DLString & ) const;
        Command::Pointer findUnstrict( const DLString & ) const;
        Command::Pointer find(const DLString &cmdName) const;
        inline CommandList & getCommands( );
        inline const CommandList & getCommands( ) const;
        CategoryMap getCategorizedCommands() const;
        bool compare( const Command &, const Command &, bool fRussian ) const;

        virtual bool process( InterpretArguments & );

protected:
        virtual void initialization( );
        virtual void destruction( );
        virtual void putInto( );
        
private:
        void loadPriorities( );
        DLString getPrioritiesFolder() const;

        static const DLString PRIO_FILE_EN;
        static const DLString PRIO_FILE_RU;

        Priorities priorities_en;
        Priorities priorities_ru;
        CommandList commands;

public:
        MultiCommandList multiCommands;
};

inline CommandList & CommandManager::getCommands( )
{
    return commands;
}

inline const CommandList & CommandManager::getCommands( ) const
{
    return commands;
}


extern CommandManager *commandManager;


#endif 
