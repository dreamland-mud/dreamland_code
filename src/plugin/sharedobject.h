/* $Id: sharedobject.h,v 1.1.4.2.8.3 2009/10/11 18:35:37 rufina Exp $
 * 
 * ruffina, Dream Land, 2005
 */
#ifndef __SHAREDOBJECT_H__
#define __SHAREDOBJECT_H__

#include <set>

#include "date.h"
#include "xmlvariablecontainer.h"
#include "xmllist.h"
#include "xmlstring.h"

#ifndef __MINGW32__

#else
#include <windows.h>
#endif

class Plugin;
class Initializer;

struct InitializerCmp {
    bool operator( ) (const Initializer *left, const Initializer *right) const;
};

class SharedObject {
public:
    typedef list<Pointer<Plugin> > PluginList;

    SharedObject( );
    
    inline bool isLoaded( ) const;
    bool isChanged( ) const;
    inline const Date getLoadTime( ) const;
    DLString getFileName( ) const;

    inline const DLString &getName( ) const;
    inline void setName( const DLString & );

    PluginList plugins;
protected:
    friend class Initializer;
    typedef set<Initializer *, InitializerCmp> InitSet;
    
    void addInit(Initializer *);
    void delInit(Initializer *);
    static SharedObject *current;

private:
    InitSet initializers;

protected:
    void loadSO( );
    void unloadSO( );

    DLString name;
    time_t loadTime;

#ifndef __MINGW32__
    void* handle;
#else
    HMODULE handle;
#endif
};

class XMLSharedObject : public XMLVariableContainer, public SharedObject {
XML_OBJECT    
public:
    typedef XMLListBase<XMLString> DependList;
    typedef list<DLString> UsedByList;
    
    virtual ~XMLSharedObject( );

    void load( );
    void unload( );

private:
    void use( XMLSharedObject &so );
    
    XML_VARIABLE DependList depends;
    UsedByList usedBy;
};

inline bool SharedObject::isLoaded( ) const 
{
    return handle != NULL;
}

inline const DLString & SharedObject::getName( ) const
{
    return name;
}

inline void SharedObject::setName( const DLString &n )
{
    name = n;
}

inline const Date SharedObject::getLoadTime( ) const
{
    return loadTime;
}
#endif
