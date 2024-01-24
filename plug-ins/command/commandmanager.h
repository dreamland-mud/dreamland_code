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
#include "dlxmlloader.h"
#include "interpretlayer.h"
#include "commandplugin.h"

class Character;

class CommandList {
public:
        Command::Pointer findExact( const DLString & ) const;
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

class CommandLoader : public virtual Plugin, public DLXMLLoader {
public:    
        virtual void loadCommand( CommandPlugin::Pointer );
        virtual void saveCommand( CommandPlugin::Pointer );

        virtual DLString getNodeName( ) const;
protected:        
        virtual void initialization( ) { }
        virtual void destruction( ) { }
        static const DLString NODE_NAME;
};

class CommandManager : public InterpretLayer,
                       public OneAllocate, 
                       public CommandLoader 
{
public:
        typedef ::Pointer<CommandManager> Pointer;
        typedef std::map<DLString, int> Priorities;
        typedef std::list<Command::Pointer> CategoryCommands;
        typedef std::map<bitnumber_t, CategoryCommands> CategoryMap;

public:
        CommandManager( );
        virtual ~CommandManager( );
        
        void load( CommandPlugin::Pointer );
        void save( CommandPlugin::Pointer );

        void registrate( Command::Pointer );
        void unregistrate( Command::Pointer );
        
        Command::Pointer findExact( const DLString & ) const;
        inline const Priorities & getPriorities( ) const;
        inline const Priorities & getPrioritiesRU( ) const;
        inline CommandList & getCommands( );
        inline const CommandList & getCommands( ) const;
        CategoryMap getCategorizedCommands() const;
        bool compare( const Command &, const Command &, bool fRussian ) const;

        virtual bool process( InterpretArguments & );

        virtual DLString getTableName( ) const;

protected:
        virtual void initialization( );
        virtual void destruction( );
        virtual void putInto( );
        
private:
        void loadPriorities( );
        void savePrioritiesRU( );

        static const DLString TABLE_NAME;
        static const DLString PRIO_FILE;
        static const DLString PRIO_FILE_RU;

        Priorities priorities;
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

inline const CommandManager::Priorities & CommandManager::getPriorities( ) const
{
    return priorities;
}

inline const CommandManager::Priorities & CommandManager::getPrioritiesRU( ) const
{
    return priorities_ru;
}

extern CommandManager *commandManager;

#define INITPRIO_CMDLOADERS 49

template <const char *&tn>
class CommandLoaderTemplate : public CommandLoader {
public:
    typedef ::Pointer<CommandLoaderTemplate> Pointer;
    
    CommandLoaderTemplate( ) {
        thisClass = this;
    }
    virtual ~CommandLoaderTemplate( ) {
        thisClass = NULL;
    }
    
    virtual DLString getTableName( ) const { 
        return tableName;
    }
    static inline CommandLoaderTemplate * getThis( ) { 
        return thisClass;
    }
private:
    static const char *tableName;
    static CommandLoaderTemplate * thisClass;
};

#define CMDLOADER_DUMMY(x)         dummyCmdLoader_ ##x## _TypeName
#define CMDLOADER(x) CommandLoaderTemplate<CMDLOADER_DUMMY(x)>

#define CMDLOADER_EXTERN(x) \
extern const char *CMDLOADER_DUMMY(x);

#define CMDLOADER_DECL(x) \
const char *CMDLOADER_DUMMY(x) = "CMDLOADER(" #x ")"; \
template<> const char *CMDLOADER(x)::tableName = "commands/" #x; \
template<> CMDLOADER(x) * CMDLOADER(x)::thisClass = NULL; \
PluginInitializer<CMDLOADER(x)> dummyCmdLoader_ ##x## _init(INITPRIO_CMDLOADERS);


#endif 
