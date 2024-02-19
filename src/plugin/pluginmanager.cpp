/* $Id: pluginmanager.cpp,v 1.32.2.9.6.4 2009/10/11 18:35:37 rufina Exp $
 * 
 * ruffina, Dream Land, 2005
 */
#include <fstream>
#include <errno.h>

#include "xmllist.h"
#include "logstream.h"
#include "xmlfile.h"

#include "pluginmanager.h"
#include "plugin.h"
#include <string.h>

/*--------------------------------------------------------------------------
 * Plugin
 *--------------------------------------------------------------------------*/
bool Plugin::isCritical() const 
{
    return false;
}

/*--------------------------------------------------------------------------
 * PluginManager 
 *--------------------------------------------------------------------------*/
 
const DLString PluginManager::TABLE_NAME = "plugins";
const DLString PluginManager::NODE_NAME = "so";
const DLString PluginManager::PROFILE_NAME = "plugin.xml";

PluginManager* PluginManager::thisClass = 0;

class PluginReloadRequest : public virtual DLObject {
public:
    virtual void process( ) = 0;
};

PluginManager::PluginManager( ) 
{
    checkDuplicate( thisClass );
    thisClass = this;
}

PluginManager::~PluginManager( )
{
    unloadAll( );
    thisClass = 0;
}

XMLSharedObject &PluginManager::load( const DLString &name ) 
{
    XMLSharedObject &so = (*this)[name];
    
    if (!so.isLoaded( )) {
        so.setName( name );
        loadXML( &so, DLString( "lib" ) + name );
        so.load( );
    }

    return so;
}

void PluginManager::unload( const DLString &name ) 
{
    iterator i;

    i = find( name );

    if (i != end( )) {
        i->second.unload( );
        erase( i );
    }
}

void PluginManager::unloadAll( ) 
{
    while (!empty( ))
        unload( begin( )->first );
}

void PluginManager::loadAll( ) 
{
    XMLListBase<XMLString> autoexec;
    XMLFile file( getTablePath( ) + '/' + PROFILE_NAME, "", &autoexec);
    
    if (!file.load( ))
        throw PluginException( "autoexec", strerror( errno ) );
    
    while (!autoexec.empty( )) {
        load( autoexec.front( ) );
        autoexec.pop_front( );
    }
}


void PluginManager::reload( const DLString &n ) 
{
    unload( n );
    loadAll( );
}

void PluginManager::reloadAll( ) 
{
    unloadAll( );
    loadAll( );
}

void PluginManager::reloadNonCritical( ) 
{
    list<DLString> todo;
    iterator i;

    for (i = begin( ); i != end( ); i++)
        if (!i->second.isCritical())
            todo.push_back( i->first );

    while (!todo.empty( )) {
        unload( todo.front( ) );
        todo.pop_front( );
    }

    loadAll( );
}

void PluginManager::reloadChanged( ) 
{
    list<DLString> todo;
    iterator i;

    for (i = begin( ); i != end( ); i++)
        if (i->second.isChanged( ))
            todo.push_back( i->first );

    while (!todo.empty( )) {
        unload( todo.front( ) );
        todo.pop_front( );
    }

    loadAll( );
}

class PluginReloadAllRequest : public PluginReloadRequest {
public:
    virtual void process( ) 
    {
        PluginManager::getThis( )->reloadAll( );
    }
};

class PluginReloadNonCriticalRequest : public PluginReloadRequest {
public:
    virtual void process( ) 
    {
        PluginManager::getThis( )->reloadNonCritical( );
    }
};

class PluginReloadChangedRequest : public PluginReloadRequest {
public:
    virtual void process( ) 
    {
        PluginManager::getThis( )->reloadChanged( );
    }
};

class PluginReloadOneRequest : public PluginReloadRequest {
public:
    PluginReloadOneRequest( const DLString &n, int w ) : name( n ), what(w) { }
    
    virtual void process( ) 
    {
        switch(what) {
        case 1:
            PluginManager::getThis( )->load( name );
            break;
        case 2:
            PluginManager::getThis( )->unload( name );
            break;
        default:
            PluginManager::getThis( )->reload( name );
        }
    }

    DLString name;
    int what;
};

void PluginManager::setReloadAllRequest( )
{
    request = ::Pointer<PluginReloadAllRequest>( NEW );
}
void PluginManager::setReloadNonCriticalRequest( )
{
    request = ::Pointer<PluginReloadNonCriticalRequest>( NEW );
}
void PluginManager::setReloadChangedRequest( )
{
    request = ::Pointer<PluginReloadChangedRequest>( NEW );
}
void PluginManager::setReloadOneRequest(const DLString &n, int w)
{
    request = ::Pointer<PluginReloadOneRequest>(NEW, n, w);
}

void PluginManager::checkReloadRequest( ) 
{
    try {
        if (request) 
            request->process( );
    }
    catch (const PluginException &e) {
        LogStream::sendError( ) << e.what( ) << endl;
    }

    request.clear( );
}

bool PluginManager::isAvailable( const DLString &name ) const
{
    const_iterator i = find( name );

    return i != end( );
}

DLString PluginManager::getTableName( ) const
{
    return TABLE_NAME;
}

DLString PluginManager::getNodeName( ) const
{
    return NODE_NAME;
}

