#ifndef COMMANDLOADER_H
#define COMMANDLOADER_H

#include "dlxmlloader.h"
#include "plugin.h"
#include "command.h"

/** Class responsible for loading/saving a single command's profile from disk. 
 *
 */
class CommandLoader : public virtual Plugin, public DLXMLLoader {
public:    
        virtual bool loadCommand( Command::Pointer );
        virtual bool saveCommand( Command::Pointer );

        virtual DLString getNodeName( ) const;
protected:        
        virtual void initialization( ) { }
        virtual void destruction( ) { }
        static const DLString NODE_NAME;
};

/** Sets priority inside SharedObject::initializers at which a plugin will be instantiated (and initialized). 
 * Slightly less than for commands.
 */
#define INITPRIO_CMDLOADERS 49

/** Template for a loader that reads commands from a custom folder */
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