/* $Id: schedulerlist.cpp,v 1.1.2.1.28.4 2009/10/11 18:35:37 rufina Exp $
 *
 * ruffina, Dream Land, 2003
 * based on idea by NoFate, 2001
 */
#include <algorithm>
#include <functional>
#include <typeinfo>

#include "schedulerlist.h"
#include "schedulertask.h"

SchedulerList::SchedulerList()
{
}

SchedulerList::~SchedulerList()
{
}

void SchedulerList::put( SchedulerTaskPointer pointer )
{
        push_back( pointer );
}

inline static bool __slay__( const SchedulerTask::Pointer task1, SchedulerTask::Pointer* task2 )
{
    return typeid( *task1.getPointer( ) ) == typeid( *task2->getPointer( ) );
}

inline static bool __slay_instance__( const SchedulerTask::Pointer task1, SchedulerTask::Pointer* task2 )
{
    return task1 == *task2;
}


void SchedulerList::tick( )
{
    for (iterator i = begin( ); i != end( ); i++)
        (*i)->before( );
    
    for (iterator i = begin( ); i != end( ); i++)
        (*i)->run( );

    for (iterator i = begin( ); i != end( ); i++)
        (*i)->after( );
}

void SchedulerList::slay( SchedulerTaskPointer& task )
{
    iterator pos = std::remove_if( 
                              begin( ), 
                            end( ), 
                            std::bind2nd( std::ptr_fun( __slay__ ), &task ) );
    erase( pos, end( ) );
}

void SchedulerList::slayInstance( SchedulerTaskPointer& task )
{
    iterator pos = std::remove_if( 
                              begin( ), 
                            end( ), 
                            std::bind2nd( std::ptr_fun( __slay_instance__ ), &task ) );
    erase( pos, end( ) );
}
