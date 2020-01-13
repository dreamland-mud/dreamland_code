/* $Id: pluginmanager.h,v 1.25.2.6.6.3 2009/10/11 18:35:37 rufina Exp $
 * 
 * ruffina, Dream Land, 2005
 * based on idea by NoFate, 2002
 */
#ifndef __PLUGINMANAGER_H__
#define __PLUGINMANAGER_H__

#include <map>

#include "exception.h"
#include "oneallocate.h"
#include "xmlloader.h"

#include "sharedobject.h"

class PluginReloadRequest;

class PluginManager : public map<DLString, XMLSharedObject>,
                      public OneAllocate, 
                      public virtual XMLLoader
{
friend class XMLSharedObject;
public:

    PluginManager( );
    virtual ~PluginManager( );

    void unloadAll( );
    void loadAll( );

    void reload( const DLString &n );
    void reloadAll( );
    void reloadChanged( );

    void setReloadAllRequest( );
    void setReloadChangedRequest( );
    void setReloadOneRequest( const DLString &, int w = 0);
    void checkReloadRequest( );

    bool isAvailable( const DLString& ) const;

    virtual DLString getTableName( ) const;
    virtual DLString getNodeName( ) const;

    inline static PluginManager* getThis( );
    
    XMLSharedObject &load( const DLString &name );
    void unload( const DLString &name );

private:
    ::Pointer<PluginReloadRequest> request;
    static PluginManager* thisClass;
    
    static const DLString PROFILE_NAME;
    static const DLString TABLE_NAME;
    static const DLString NODE_NAME;
};


class PluginException : public Exception {
public:
        inline PluginException( const DLString &pluginName, const DLString &error ) throw()
                : Exception( "Error while loading plugin " + pluginName + ": " + error )
        {
        }
};

inline PluginManager* PluginManager::getThis( )
{
    return thisClass;
}


#endif
