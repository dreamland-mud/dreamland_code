/* $Id$
 *
 * ruffina, 2018
 */
#ifndef WEBPROMPT_H 
#define WEBPROMPT_H 

#include <list>

#include "oneallocate.h"
#include "plugin.h"

class WebPromptListener;
class Descriptor;
class Character;
namespace Json {
    class Value;
}

class WebPromptManager : public Plugin, public OneAllocate {
public:        
    typedef ::Pointer<WebPromptManager> Pointer;
    typedef ::Pointer<WebPromptListener> WebPromptListenerPointer;
    typedef std::list<WebPromptListenerPointer> Listeners;
    
    WebPromptManager( );
    virtual ~WebPromptManager( );

    virtual void initialization( );
    virtual void destruction( );

    void registrate( WebPromptListenerPointer );
    void unregistrate( WebPromptListenerPointer );
    void handle( Descriptor *, Character *, Json::Value &json );
    
    inline static WebPromptManager * getThis( );
    
private:
    Listeners listeners;
    static WebPromptManager *thisClass;
};

inline WebPromptManager * WebPromptManager::getThis( )
{
    return thisClass;
}

class WebPromptListener : public virtual Plugin {
public:
        typedef ::Pointer<WebPromptListener> Pointer;

        virtual void initialization( );
        virtual void destruction( );
        
        virtual void run( Descriptor *, Character *, Json::Value &json ) = 0;
};

#endif
