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
