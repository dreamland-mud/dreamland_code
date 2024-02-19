/* $Id: sharedobject.cpp,v 1.1.4.4.6.4 2009/11/02 13:48:11 rufina Exp $
 * 
 * ruffina, Dream Land, 2005
 */
#include <dlfcn.h>
#include <sys/stat.h>
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#include "logstream.h"

#include "initializer.h"
#include "sharedobject.h"
#include "plugin.h"
#include "pluginmanager.h"
#include <string.h>

//#define LD_VERBOSE 1

/*--------------------------------------------------------------------------
 * Initializer list comparator
 *--------------------------------------------------------------------------*/
bool 
InitializerCmp::operator( ) (const Initializer *left, const Initializer *right) const
{
    if(left->getPriority( ) < right->getPriority( ))
        return true;
    
    if(left < right)
        return true;

    return false;
}

/*--------------------------------------------------------------------------
 * SharedObject
 *--------------------------------------------------------------------------*/
SharedObject *SharedObject::current = 0;

SharedObject::SharedObject( ) : loadTime( 0 ), handle( NULL )  
{
}

void
SharedObject::addInit(Initializer *i)
{
    initializers.insert(i);
}

void
SharedObject::delInit(Initializer *i)
{
    initializers.erase(i);
}

bool SharedObject::isChanged( ) const 
{
    struct stat sb;
    
    if (stat( getFileName( ).c_str( ), &sb) < 0)
        throw PluginException( name, strerror(errno) );

    return loadTime < sb.st_mtime;
}

bool SharedObject::isCritical() const
{
    for (auto &p: plugins) {
        if (p->isCritical())
            return true;
    }

    return false;
}

DLString SharedObject::getFileName( ) const
{
    return PluginManager::getThis( )->getTablePath( ) + '/' +
           PluginManager::getThis( )->getTableName( ) + "/lib" +
           getName( ) +
           ".so";
}

    
void SharedObject::loadSO( )
{
    PluginList::iterator i;

    loadTime = time( NULL );
    DLString initName = "initialize_" + getName( );
    typedef PluginList InitType( );
    
#if LD_VERBOSE
    LogStream::sendNotice( ) << "opening " << getFileName( ) << "..." << endl;
#endif

    if(current)
        abort();

    current = this;
    
    handle = dlopen( getFileName( ).c_str( ), RTLD_NOW /*| RTLD_GLOBAL */);
    
    if (!handle) {
        current = 0;
        throw PluginException( name, dlerror( ) );
    }
    
    InitType* functionInit = (InitType*)dlsym( handle, initName.c_str( ) );
    if(!current)
        abort();

#if LD_VERBOSE
    LogStream::sendNotice( ) << "calling initializers" << endl;
#endif

    current = 0;
    
    set<Initializer *>::iterator it;

    for(it = initializers.begin(); it != initializers.end( ); it++)
        (*it)->init(this);
    
    /*legacy initialization*/
    if(functionInit) {
#if LD_VERBOSE
        LogStream::sendNotice( ) << "calling legacy init " << initName << "..." << endl;
#endif
        PluginList pll = (*functionInit)( );
        plugins.splice(plugins.end(), pll);
    }
    
    for (i = plugins.begin( ); i != plugins.end( ); i++)
        (*i)->initialization( );
    
    LogStream::sendNotice( ) << "load [" << name << "] so" << endl;
}

void SharedObject::unloadSO( )
{
    PluginList::iterator i;

    for (i = plugins.begin( ); i != plugins.end( ); i++)
        (*i)->destruction( );
        
    plugins.clear( );

    if(current)
        abort();

    current = this;
    
    if (handle) {
        dlclose( handle );
        handle = NULL;
    }

    if(!current)
        abort();

    current = 0;

    loadTime = 0;
    LogStream::sendNotice( ) << "unload [" << name << "] so" << endl;
}

/*--------------------------------------------------------------------------
 * XMLSharedObject
 *--------------------------------------------------------------------------*/

XMLSharedObject::~XMLSharedObject( )
{
}

void XMLSharedObject::load( ) 
{
    DependList::iterator i;

    for (i = depends.begin( ); i != depends.end( ); i++)
        use( PluginManager::getThis( )->load( i->getValue( ) ) );

    try {
        loadSO( );
    } catch (const std::exception &ex) {
        LogStream::sendFatal() << "Exception loading [" << name << "]: " << ex.what() << endl;
        throw ex;
    }
}

void XMLSharedObject::unload( ) 
{
    while ( !usedBy.empty( ) ) {
        const DLString &so = usedBy.front( );
        
        PluginManager::getThis( )->unload( so );
        usedBy.pop_front( );
    }

    try {
        unloadSO( );
    } catch (const std::exception &ex) {
        LogStream::sendFatal() << "Exception unloading [" << name << "]: " << ex.what() << endl;
        throw ex;
    }
}

void XMLSharedObject::use( XMLSharedObject &so ) 
{
    so.usedBy.push_back( getName( ) );
}


