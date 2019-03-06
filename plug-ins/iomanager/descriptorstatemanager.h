/* $Id: descriptorstatemanager.h,v 1.1.4.2 2005/09/07 19:49:31 rufina Exp $
 *
 * ruffina, 2003
 */
#ifndef DESCRIPTORSTATEMANAGER_H
#define DESCRIPTORSTATEMANAGER_H

#include <list>

#include "oneallocate.h"
#include "plugin.h"

class DescriptorStateListener;
class Descriptor;

class DescriptorStateManager : public Plugin, public OneAllocate {
public:        
    typedef ::Pointer<DescriptorStateManager> Pointer;
    typedef ::Pointer<DescriptorStateListener> DescriptorStateListenerPointer;
    typedef std::list<DescriptorStateListenerPointer> Listeners;
    
    DescriptorStateManager( );
    virtual ~DescriptorStateManager( );

    virtual void initialization( );
    virtual void destruction( );

    void registrate( DescriptorStateListenerPointer );
    void unregistrate( DescriptorStateListenerPointer );
    void handle( int, int, Descriptor * );
    
    inline static DescriptorStateManager * getThis( );
    
private:
    Listeners listeners;
    static DescriptorStateManager *thisClass;
};

inline DescriptorStateManager * DescriptorStateManager::getThis( )
{
    return thisClass;
}

#endif
