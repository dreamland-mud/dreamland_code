/* $Id$
 *
 * ruffina, 2004
 */
#ifndef DEATHMANAGER_H
#define DEATHMANAGER_H

#include <list>
#include <map>
#include "plugin.h"
#include "oneallocate.h"

using namespace std;
class Character;

class DeathHandler : public virtual Plugin {
public:
    typedef ::Pointer<DeathHandler> Pointer;

    virtual void initialization( );
    virtual void destruction( );
    virtual bool handleDeath( Character *, Character * ) const = 0;
    virtual int getPriority( ) const = 0;
};

class DeathManager : public Plugin, public OneAllocate {
public:
    typedef list<DeathHandler::Pointer> HandlersList;
    typedef map<int, HandlersList> HandlersPriority;
    
    DeathManager( );
    virtual ~DeathManager( );

    virtual void initialization( );
    virtual void destruction( );
    void handleDeath( Character *, Character * );
    void registrate( DeathHandler::Pointer );
    void unregistrate( DeathHandler::Pointer );
    static inline DeathManager * getThis( );

protected:
    HandlersPriority handlers; 
private:
    static DeathManager *thisClass;
};

inline DeathManager * DeathManager::getThis( )
{
    return thisClass;
}

#endif
