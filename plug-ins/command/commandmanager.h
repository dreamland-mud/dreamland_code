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

        // Find a command by its exact name or alias, in any lang.
        Command::Pointer findExact(const DLString &cmdName);

        // Find a command by its name or alias prefix, in any lang.
        Command::Pointer findUnstrict( const DLString &cmdName);

        list<Command::Pointer> getSortedCommands();

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

/** Keeps track of all the interpretable commands in the world, processes user input. */
class CommandManager : public InterpretLayer,
                       public OneAllocate 
{
public:
        typedef ::Pointer<CommandManager> Pointer;
        typedef std::list<Command::Pointer> CategoryCommands;
        typedef std::map<bitnumber_t, CategoryCommands> CategoryMap;

public:
        CommandManager( );
        virtual ~CommandManager( );
        
        void registrate( Command::Pointer );
        void unregistrate( Command::Pointer );
        
        Command::Pointer findExact( const DLString & );
        Command::Pointer findUnstrict( const DLString & );
        void refresh(lang_t lang);        
        list<Command::Pointer> getCommands();
        CategoryMap getCategorizedCommands() const;

        virtual bool process( InterpretArguments & );

protected:
        virtual void putInto( );
        
        MultiCommandList multiCommands;
};


extern CommandManager *commandManager;


#endif 
